---
layout: page
title: Transition prefetch schema
eyebrow: Reference
subtitle: The post-processed planner that says which pack to start streaming next, given the current pack and the analyzer's story-order graph.
description: Reference for the transition prefetch schema — the post-processed JSON artifact that ranks pack-to-pack transitions by added bytes and working set, drives the foreground pilot's prefetch decisions, and feeds the pack manifest's prefetch_hints. Inputs, outputs, field definitions, runtime behavior on a 2x CD drive, and what happens when prefetch misses.
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

A labor of love by Hunter Davis. The PS1 ships with a 2x CD-ROM drive.
Cold-seek latency is around 150 ms; sustained read throughput is roughly
300 KB/s. Both numbers are an order of magnitude worse than the original
DOS environment Sierra targeted. If the runtime waits until the player has
walked into a new scene before it asks the disc for that scene's pack,
the player sees a stall. If it asks early — before the new scene is
needed — the read can complete in the background and presentation never
notices.

Picking *which* pack to ask early for is the job of the transition
prefetch schema. It is a post-processing planner artifact, generated from
the scene analyzer's output, that ranks pack-to-pack edges by how much
new data they introduce. The runtime does not read this file directly —
its content is folded into the
[pack manifest's `prefetch_hints`]({{ '/docs/formats/pack-manifest/' | relative_url }})
section, and the foreground pilot reads that subset. The schema doc lives
at
[`TRANSITION_PREFETCH_SCHEMA.md`]({{ site.github_url }}/blob/main/docs/ps1/research/TRANSITION_PREFETCH_SCHEMA.md).

This file documents the post-processed planning artifacts generated from
[`scene_analysis_output_2026-03-17.json`]({{ site.github_url }}/blob/main/docs/ps1/research/generated/scene_analysis_output_2026-03-17.json). It exists so transition and
prefetch studies target the same field names the manifest uses.

If you paid for this, you were cheated. Open source and free.

## Inputs

The planner reads three things from analyzer output:

- `scenes[*]` — every scene the analyzer found, in story order
- per-scene resource bindings — which BMPs, SCRs, TTMs, ADSes each scene
  references
- per-scene peak memory estimates — the working-set cost for the scene,
  computed from the resource sizes

These three feed every output below. Story order is the analyzer's
heuristic — adjacency in `scenes[*]` — not an instrumented runtime trace.
The schema is explicit:

> Scene ordering is the analyzer's story order, not an instrumented
> runtime trace.

## Outputs

The planner emits five output sections:

| Output                  | Meaning |
|-------------------------|---------|
| `pack_candidates`       | ADS-family groups with union resource accounting; useful for pack-granularity decisions. |
| `pack_manifest_inputs`  | Direct fields that map into `PACK_MANIFEST_SCHEMA`. |
| `transition_edges`      | Adjacent story-order scene edges with added/shared/removed resource counts and bytes. |
| `top_prefetch_edges`    | The transition edges, ranked by added bytes and working set. |
| `top_pack_boundaries`   | Edges flagged as either large working-set crossings or cross-family transitions. |

`pack_candidates` is what the planner thinks the pack set should be.
`pack_manifest_inputs` is the format-converted version of the same data,
ready to drop into a `PACK_MANIFEST_SCHEMA` JSON. `transition_edges` is
the raw edge list. `top_prefetch_edges` is the ranked subset the runtime
prefetcher should care about. `top_pack_boundaries` is the subset the
pack-granularity planner should care about — they are different views of
the same edge data with different sort orders.

## Field definitions

Per-edge fields:

| Field                     | Meaning |
|---------------------------|---------|
| `added_bytes`             | Bytes that appear in the destination scene but not the source scene. |
| `shared_bytes`            | Bytes that exist in both scenes. |
| `working_set_bytes`       | Union of source and destination resource bytes. |
| `edge_class`              | Heuristic label derived from `added_bytes` and whether the ADS family changed. |
| `prefetch_hint.priority`  | `low`, `medium`, or `high`. Reflects relative post-processing priority, not a runtime guarantee. |

`added_bytes` is the most important number here. It is the lower bound on
how much data the prefetcher has to fetch from the CD before the
destination scene can present its first frame. A transition with
`added_bytes` of 200 KB on a 300 KB/s drive needs at minimum two thirds
of a second of warm-up, and that is before seek time. Transitions with
small `added_bytes` are cheap; large ones are budgets.

`edge_class` is a heuristic label — currently produced by a small
decision table based on `added_bytes` and ADS-family change. The schema
calls this out:

> The `edge_class` labels are intentionally simple and should be replaced
> if the project gains validated transition telemetry.

`prefetch_hint.priority` becomes the manifest's
`prefetch_hints.confidence` field after format conversion. It is *not* a
runtime guarantee — the runtime is free to ignore a `high` hint if it
has no spare bandwidth.

## How the runtime decides what to load

The PS1-side prefetcher is in
[`src/foreground_pilot/foreground_pilot.c`]({{ site.github_url }}/blob/main/src/foreground_pilot/foreground_pilot.c).
At a high level:

1. While playing a scene, the pilot maintains a `streamWindowBuffer`
   (default 16 KB; `FG_PREFETCH_DEFAULT_WINDOW_BYTES`) that holds the
   next frames' worth of data ahead of the playback cursor.
2. Stage-1 prefetch tops up the window between presented frames. The
   gating is done by `gFgPrefetchStage1Enabled` and a slack budget —
   `FG_PREFETCH_WINDOW_MIN_SLACK_VBLANKS = 3`. Below 3 VBlanks of slack,
   the pilot will not start a new window read because the read cannot
   complete before the next frame must present.
3. When the current scene is close to ending, the pilot consults the
   pack's `prefetch_hints` to know which pack(s) it should already be
   fetching. The decision keys off `likely_next_pack_ids` from
   `transition_hints` and `candidate_scene_indices` from `prefetch_hints`.
4. The CD drive runs at 2x. A cold-seek read costs ~150 ms; a sustained
   read inside the same track is much cheaper. The prefetcher prefers to
   keep the head close to the in-flight pack and reads ahead in the same
   sector spiral whenever possible.

At `v0.9.3-ps1`, the next-scene stage also has a non-blocking path. It starts
aligned async CD reads for the destination pack's metadata and initial payload
window before the current scene hands off, polls for completion during slack,
and drains the async path before any later blocking CD operation. Completed
metadata and payload windows can be parked in the SPU cold cache, which frees
the main `streamWindowBuffer` for the scene still on screen. The SPU copy is a
DMA-backed parking step, not direct CPU execution from SPU RAM.

`FG_PREFETCH_FALLTHROUGH_MIN_SLACK_VBLANKS = 6` is the slack threshold
above which the pilot will start a fall-through read into a different
pack. Below that, the fall-through is not worth the seek penalty.
`FG_PREFETCH_DIRECT_STAGE_MAX_BYTES` (8 KB) caps how much data a single
direct-stage prefetch will issue before yielding back to the present
loop.

These tuning knobs are still moving. Recent commits — `1d737913` ("ps1:
plan FG2 read groups from CD logs"), `6e766a4f` ("ps1: add perf CD log
summarizer"), `7a06d50f` ("docs: log high-slack window miss") — are all
part of an ongoing effort to *measure* the prefetcher's miss rate and
back-pressure it against actual disc behavior rather than estimated
numbers.

## What happens if prefetch does not land in time

Three failure modes, in order of severity:

**Frame drop.** The pilot finishes presenting frame N, looks for frame
N+1 in `streamWindowBuffer`, and finds the buffer empty. It blocks on
`CdRead` for the missing payload. The drive completes the read in the
order of one or two VBlanks, and the next frame presents late. The
viewer sees a brief stall but the scene resumes.

**Scene re-skip.** Some scenes have a strict timing relationship with
their sound events — fishing scenes time the splash sample to the
`sourceFrame` of the splash diff. If a frame is late enough that its
sound event misses, the pilot may opt to skip the late frame and resync
the next event window rather than play out-of-time audio. This is
visible as a small jump in the animation but is preferable to desynced
audio.

**Fallback to legacy ADS path.** If the pack file fails to load
entirely — wrong magic, truncated read, missing on disc — the runtime
gives up on the FG2 path and falls back to running the original ADS
script through the on-console interpreter. This is correct but slow,
and the fallback is logged with `JCFG2: fallback` to TTY so the
regtest harness flags it.

The first two are quality-of-experience problems. The third is a
correctness fallback: if the pack on disc disagrees with the pack the
runtime expected, the legacy interpreter is the only path that does not
silently render a wrong frame.

## Caveats

Direct from the schema doc:

- This is a post-processing planner, not a runtime transition validator.
- Scene ordering is the analyzer's story order, not an instrumented
  runtime trace.
- The `edge_class` labels are intentionally simple and should be
  replaced if the project gains validated transition telemetry.

The planner is informed by the analyzer's *static* understanding of the
scene graph. The runtime's actual transition behavior — which scene the
player walks into next — depends on Johnny's pathing, which the analyzer
does not simulate. So the planner is a starting point, not the ground
truth, and the prefetcher needs the slack budget above to absorb cases
where the planner guessed wrong.

## Related references

- [FG2 pack manifest]({{ '/docs/formats/pack-manifest/' | relative_url }}) —
  the consumer of this schema's output via `prefetch_hints`.
- [FG2 pack payload]({{ '/docs/formats/pack-payload/' | relative_url }}) —
  the binary the prefetcher actually reads.
- [Performance]({{ '/docs/performance/' | relative_url }}) — the CD log
  summarizer and current prefetch tuning state.

## Source on GitHub

- [`docs/ps1/research/TRANSITION_PREFETCH_SCHEMA.md`]({{ site.github_url }}/blob/main/docs/ps1/research/TRANSITION_PREFETCH_SCHEMA.md)
  — canonical schema doc; the field tables above are derived from it.
- [`docs/ps1/research/generated/scene_analysis_output_2026-03-17.json`]({{ site.github_url }}/blob/main/docs/ps1/research/generated/scene_analysis_output_2026-03-17.json)
  — input file the transition + prefetch planning artifacts above
  were generated from; named in the page intro.
- [`src/foreground_pilot/foreground_pilot.c`]({{ site.github_url }}/blob/main/src/foreground_pilot/foreground_pilot.c)
  — runtime consumer; the prefetch decision logic lives here.
