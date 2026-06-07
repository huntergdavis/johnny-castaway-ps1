---
layout: page
title: Dirty region template
eyebrow: Reference
subtitle: The renderer's bookkeeping cheat-sheet. For each TTM, which rectangles get touched, which need a clean-background restore, and which scenes are clear-screen heavy.
description: Reference for the dirty-region template — the offline JSON artifact the host build derives from TTM bytecode, telling the PS1 dirty-row renderer which rectangles to track per scene. Per-pack and per-scene template shape, opcode coverage, the grRestoreBgTiles bug that surfaced prevDirty, and where the runtime consumes it.
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

A labor of love by Hunter Davis. The PS1 framebuffer is small, and a full
clear-and-redraw of all 320×200 pixels every frame is too expensive to
sustain — the GPU can do it, but the dirty-row tile-upload path the
project uses for everything else cannot, because it is competing with
sprite uploads for VRAM bandwidth. So the renderer instead tracks a
*dirty region*: the set of rectangles it actually needs to redraw this
frame. Anything outside that region keeps its previous pixels.

This works as long as the renderer knows which rectangles a scene
touches. For some scenes — `FISHING1`, the simple animated activities —
the rectangles are obvious from the captured frames. For others — scenes
that use `COPY_ZONE_TO_BG` to commit ephemeral state into the background,
or that issue full `CLEAR_SCREEN` opcodes mid-animation — the rectangles
have to be derived from the TTM bytecode itself, statically, before the
PS1 ever runs the scene.

That derivation is what the *dirty-region template* is. It is an offline
JSON artifact, produced from scene-pack manifests plus extracted TTM
bytecode, listing every rectangle every TTM in the pack ever touches and
flagging which ones look like restore candidates. The PS1 build reads
the template at scene activation and seeds its dirty-rect bookkeeping
from it.

The format is described in the
[source schema]({{ site.github_url }}/blob/main/docs/ps1/research/DIRTY_REGION_TEMPLATE_SCHEMA.md):

> Describe the offline template artifact produced from scene-pack
> manifests plus extracted TTM bytecode. This is a planning/build
> artifact. It does not change runtime behavior by itself.

That last sentence is important. The template *informs* the runtime. It
does not *force* the runtime to do anything. Runtime consumption is
gated until per-scene restore policy is validated.

If you paid for this, you were cheated. Open source and free.

## How it gets generated

One Python script:
[`scripts/extract-dirty-region-templates.py`]({{ site.github_url }}/blob/main/scripts/extract-dirty-region-templates.py).
It reads each pack manifest, walks every TTM the manifest references,
disassembles the bytecode, and tracks which opcodes write into the
framebuffer and where. The current default output directory is
`docs/ps1/research/generated/dirty_region_templates_2026-03-18`.

The opcodes the extractor currently treats as dirty-region signals:

| Opcode             | Meaning                                       |
|--------------------|-----------------------------------------------|
| `TTM_UNKNOWN_1`    | Unknown opcode that empirically writes pixels. |
| `COPY_ZONE_TO_BG`  | Commit a region into the clean-background restore baseline. |
| `SAVE_IMAGE1`      | Same shape as `COPY_ZONE_TO_BG` per current PS1 routing. |
| `SAVE_ZONE`        | Track one active zone for later restore. |
| `RESTORE_ZONE`     | Restore from the saved zone. |
| `CLEAR_SCREEN`     | Full-screen clear; flagged separately from per-rect dirty. |

If a scene's TTM emits any of these, the rectangle the opcode targets
gets recorded.

## Per-pack artifact

Each pack JSON contains:

| Field                | Notes |
|----------------------|-------|
| `schema_version`     | Currently 1. |
| `artifact_kind`      | `"dirty_region_template_pack"`. |
| `pack_id`            | Matches the pack manifest. |
| `ads_names`          | Source ADS file(s). |
| `scene_indices`      | Story-order scene indices the pack covers. |
| `summary`            | Aggregate counts. See *summary* below. |
| `ttm_templates`      | One entry per referenced TTM. |
| `scene_templates`    | One entry per scene index. |

### `summary`

```json
"summary": {
  "ttm_count": 12,
  "restore_candidate_ttm_count": 4,
  "scene_template_count": 3,
  "candidate_scene_indices": [0, 1, 2]
}
```

| Field                          | Meaning |
|--------------------------------|---------|
| `ttm_count`                    | Number of TTMs analyzed. |
| `restore_candidate_ttm_count`  | TTMs flagged as needing offline restore policy. |
| `scene_template_count`         | Scene rows present in `scene_templates`. |
| `candidate_scene_indices`      | Scenes the planner thinks should consume the template. |

### `ttm_templates`

Each TTM row carries:

| Field                    | Meaning |
|--------------------------|---------|
| `ttm_name`               | Source TTM file name. |
| `byte_size`              | Disassembled bytecode size. |
| `op_counts`              | Map of opcode name → count. |
| `unique_rect_count`      | Number of distinct rectangles ever touched. |
| `unique_rects`           | The rectangles themselves. |
| `union_rect`             | Bounding rectangle covering all `unique_rects`. |
| `clear_heavy`            | Boolean — does this TTM call `CLEAR_SCREEN` often. |
| `restore_candidate`      | Boolean — does the runtime need offline restore for this TTM. |
| `tag_templates`          | Per-tag (per scene-id) breakdown of the same data. |
| `source_scene_indices`   | Which scenes use this TTM. |
| `slot_ids`               | TTM slot identifiers the runtime sees. |

`union_rect` is the most-used field at runtime: when a TTM activates, the
runtime can size its dirty-rect bookkeeping with one read of `union_rect`
and never touch the per-frame `unique_rects` array unless it needs to.

`restore_candidate = true` is what tells the runtime the TTM cannot rely
on the default "redraw the dirty rects" path. It needs an explicit
restore from clean-background tiles, because the TTM has done a
`COPY_ZONE_TO_BG` that committed transient state into the baseline.

### `scene_templates`

Each scene row is a first-pass union over all referenced TTM templates:

| Field                | Meaning |
|----------------------|---------|
| `scene_index`        | Story-order scene index. |
| `ttm_names`          | TTMs this scene activates. |
| `unique_rects`       | Union of `unique_rects` across those TTMs. |
| `unique_rect_count`  | Cardinality. |
| `union_rect`         | Bounding rectangle of the union. |
| `clear_screen_count` | Sum of `CLEAR_SCREEN` calls across the TTMs. |
| `restore_candidate`  | Logical-OR of TTM-level `restore_candidate`. |

The scene row is what the runtime keys on. When scene N starts, the
runtime looks up the scene template, takes `union_rect` as the dirty-rect
seed, and treats `restore_candidate` as a flag controlling whether the
default path or the offline-restore path runs.

## Aggregate summary file

`summary.json` sits beside the per-pack files:

| Field                      | Meaning |
|----------------------------|---------|
| `schema_version`           | Currently 1. |
| `artifact_kind`            | `"dirty_region_template_summary"`. |
| `pack_count`               | Number of pack templates in this batch. |
| `candidate_pack_ids`       | Packs flagged as needing template-driven runtime work. |
| `candidate_scene_total`    | Sum of `candidate_scene_indices` across packs. |
| `packs`                    | Array of `{pack_id, summary}` rollups. |

It exists so a pack-planning pass over hundreds of scenes does not have
to re-open every per-pack JSON to find the candidates.

## How the runtime consumes it

The PS1 build does not parse JSON, and dirty-region template consumption
is still gated. Today the templates are *available* in the build tree
and are checked into the pack manifest's
`dirty_region_templates` field as a sidecar reference, but the renderer
does not yet flip its restore policy off them on every scene. The
schema doc says so explicitly:

> dirty-region templates are compiler sidecars only; runtime consumption
> remains intentionally gated until per-scene restore policy is validated.

The current consumers are:

- the pack compiler, which copies the matching template into the
  compiled-pack directory as `dirty_region_templates.json`
- the regtest harness, which reads templates to know which scenes have
  large `union_rect`s and may need extra frames of capture latency
- the runtime, on the pilot routes only — `FISHING1` and similar — where
  `restore_candidate` is honored as a hint to call
  `grForceFullRedrawNextFrame` after the next `grRestoreBgTiles`.

## The grRestoreBgTiles / prevDirty bug

The renderer assumed that `currDirty` (the rect set for the current
frame) was always what it needed to repaint. Then `grRestoreBgTiles`
came along — when the runtime resumes a scene from a saved background,
`grRestoreBgTiles` *wipes* `currDirty`, because the saved background
already has clean pixels. After the wipe, the next frame would compute
its dirty rect from a zeroed `currDirty` and fail to repaint regions
that *had* been dirty before the resume.

The fix was to keep `prevDirty` alongside `currDirty` and force a full
redraw next frame when a resume happens — `grForceFullRedrawNextFrame`.
The dirty-region template's `restore_candidate` flag is the input to
that decision: when the renderer sees a TTM marked `restore_candidate =
true`, it pre-arms the full-redraw path so the next `grRestoreBgTiles`
does not lose pixels.

This is now a baked-in habit in the project's dirty-rect bookkeeping
and is documented at the head of `graphics_ps1.c`. The relevant
takeaway for *this* doc: the dirty-region template is the static-side
input to a decision the runtime would otherwise have to make
dynamically — and getting that decision wrong loses pixels in a way
that is invisible until you screenshot a comparison frame.

## Current limitations

Direct from the schema doc:

- Templates are derived from static TTM bytecode only.
- No runtime validation is implied yet.
- Candidate selection is heuristic; dynamic overlap scenes may still
  require replay continuity.

"Replay continuity" is the legacy actor-recovery code in `ads.c`. As
long as some scenes still depend on it for visual correctness, the
template alone is not enough — the runtime needs both. See the
[SDL compat lite contract]({{ '/docs/formats/sdl-compat-lite/' | relative_url }})
for the long-term direction (replay continuity is on the kill list).

## Related references

- [FG2 pack payload]({{ '/docs/formats/pack-payload/' | relative_url }}) —
  the binary the template's `union_rect` helps the runtime size.
- [FG2 pack manifest]({{ '/docs/formats/pack-manifest/' | relative_url }}) —
  carries the sidecar reference to the template file.
- [SDL compat lite]({{ '/docs/formats/sdl-compat-lite/' | relative_url }}) —
  the contract that makes dirty-rect bookkeeping an implementation detail
  rather than a gameplay-visible behavior.

## Source on GitHub

- [`docs/ps1/research/DIRTY_REGION_TEMPLATE_SCHEMA.md`]({{ site.github_url }}/blob/main/docs/ps1/research/DIRTY_REGION_TEMPLATE_SCHEMA.md)
  — canonical schema; the "Current limitations" section above quotes
  it verbatim.
- [`scripts/extract-dirty-region-templates.py`]({{ site.github_url }}/blob/main/scripts/extract-dirty-region-templates.py)
  — offline extractor that derives templates from TTM bytecode.
- [`src/graphics_ps1/graphics_ps1.c`]({{ site.github_url }}/blob/main/src/graphics_ps1/graphics_ps1.c)
  — runtime consumer; `grRestoreBgTiles`, `currDirty` /
  `prevDirty`, and `grForceFullRedrawNextFrame`. The fix for the
  `prevDirty` bug above is documented at the head of this file.
- [`src/ads/ads.c`]({{ site.github_url }}/blob/main/src/ads/ads.c)
  — the legacy actor-recovery / "replay continuity" code the
  Current limitations section names. On the SDL compat lite kill
  list as the long-term direction.
