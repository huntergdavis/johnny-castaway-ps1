---
title: "From 87 to 99.5: the post-validation performance loop"
eyebrow: Lab · Methodology
subtitle: How the project closed roughly 12 percentage points of target-speed gap after every scene was already signed off — by treating performance as a separate ledger, not a refactor.
description: A retrospective on the headless-perf battle card after complete scene validation — the methodology, the accepted optimizations (FGP3, scene-local prefetch relief, stream-window retuning, padded residual packs, scoped read groups), the rejected ones (-O2, naive read-group probes), and the discipline of preserving the experiment log.
date: 2026-05-06
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

When the project hit 63 / 63 scenes signed off at `v0.7.0-ps1`, the visible work was done. Every scene the original game routed plays pixel-perfect on a PS1 with synced SFX across every applicable variant. The disc plays. That was the point.

The compact full-matrix baseline at the same time read **+17.4% over target / 87.1% target speed** across 120 timing-bearing scene/tide rows. That's a different kind of done. A scene that overruns its frame budget by a sixth still looks correct — pixels and audio match the host capture — but the playback rate is wrong, and on real hardware "wrong rate" eventually shows up as audio drift or a stretched walk. Validation said *the bar was met for visuals*. Performance said *most rows have more work to do*.

Between then and `v0.8.0-ps1` (the release that promoted the
headless optimization methodology to a baseline), the
headless-perf battle card moved to **+0.9% over target / 99.5%
target speed** across the same 120 rows. Roughly
**16.5 percentage points** of over-target gap closed; about
**12.4 target-speed points** added. ACTIVITY 9 — the last
validated scene and the widest one — graduated from "validated"
to "optimized validated outlier."
[`v0.8.1-ps1`]({{ '/lab/v081-mary4-freeze/' | relative_url }})
followed as a clean-rect pressure stability fix that left the
matrix mean untouched. The current post-`v0.8.1-ps1` performance branch is now
at **+0.6% over target / 99.7% target speed**, with roughly **16.8 percentage
points** of over-target gap closed and about **12.6 target-speed points**
added. This article is what that loop actually looked like.

## Two ledgers, on purpose

The project keeps two acceptance bars and never lets them merge:

- **Visual signoff** — pixel-perfect against host capture, plus SFX cues on the same engine ticks, signed off across every variant flag. Human review. The [scene ledger]({{ '/scenes/' | relative_url }}) tracks this in the top table.
- **Headless performance** — automated DuckStation timing in capture mode. Measures `loop_vb`, `target_vb`, `over_target`, `blocking_vb`, `prefetch_hits`, byte counts. The live battle card is at [/perf/]({{ '/perf/' | relative_url }}) — sortable headers, color-coded Target Speed cells.

A scene can clear the visual bar and still be slow. A scene can be fast and still be wrong. The ledgers stay separate because the failure modes are uncorrelated. Optimizing for speed without re-running the visual signoff would be how regressions ship.

The v0.8.0 invariant is that **63 / 63 scenes stayed green on the visual bar through every accepted perf change**. That's not aspirational — it's how the loop is wired.

## The headless harness

`scripts/run-regtest.sh` boots a deterministic DuckStation in `perf-log` mode against a `BOOTMODE.TXT` that names the scene, tide, and seed. The PS1 build runs the scene to a fixed end frame, dumps `perf-log.txt`, and exits. The matrix runner walks every (scene, tide) row, records the values, and writes a CSV row keyed by run ID and date.

Two numbers from the log do most of the work:

- `loop_vb` — vblanks the scene actually took.
- `target_vb` — vblanks the scene *should* have taken, computed from the host capture's frame count at native rate.

Their ratio is the row's `target_speed_pct`. Their difference is `over_target_pct`. Anything above zero means the row missed; the matrix mean is what the home-page status pill reads from.

The harness writes one JSONL line per run into a scratch directory and one row into the long-form table. The scratch directory is local-only — it's the experimentation log nobody else reads. The long-form table lives at `docs/ps1/performance-experiment-log.md` and is the decision record that survives branch rebases. Every accepted experiment, every rejected one, gets a row.

## Accepted experiments

These are the changes that landed against the matrix from the compact baseline through `v0.8.0-ps1`. None of them changed pixels.

### The single biggest unlock: clean-memory-relief drop-prefetch

Most of the matrix-wide gain in the last 24 hours came from one mechanism. Many scenes shared a diagnostic shape: **compact packs + a large clean snapshot ⇒ `policy=none` ⇒ every payload due-misses**. The runtime's prefetch buffer was sitting on memory the clean-rect path needed, so the streamer fell back to no-prefetch and missed every read.

The fix is a per-scene opt-in: when the clean snapshot is large, drop the prefetch buffer instead of starving on it. Scenes opt in by joining the *clean-memory-relief / large-clean drop-prefetch exception list*. Twelve scenes were added in the v0.8.0 push (2026-05-06): `JOHNNY1`, `ACTIVITY9`, `MARY1`, `ACTIVITY11`, `ACTIVITY12`, `BUILDING4`, `BUILDING6`, `JOHNNY6`, `ACTIVITY4`, `FISHING4` — plus `WALKSTUF1`, `VISITOR3`, `VISITOR5`, `ACTIVITY10`, `JOHNNY3` already on it from a prior pass. The numbers, straight from the experiment log:

- **ACTIVITY9 high**: `blocking_vb 884 → 139`, `loop_reads 251 → 116`, `due_misses 251 → 25`.
- **ACTIVITY9 low**: `871 → 175`, `251 → 166`, `251 → 48`.
- **BUILDING4 high**: `loop_vb 3286 → 2985`, `blocking 1519 → 285`, `loop_reads 427 → 93`, `due 427 → 40`.
- **BUILDING4 low**: `3294 → 2981`, `1510 → 199`, `427 → 62`, `427 → 14`.
- **BUILDING6 high**: `2642 → 2520`, `blocking 1035 → 62`, `due 306 → 1`.

Cuts measured in *thousands* of blocking vblanks per variant. Most of the ~12-percentage-point matrix-mean move came from this one mechanism. The fix is simple — release a buffer when memory pressure is high — and the discipline was the per-scene measured opt-in instead of a global runtime change.

### Stale-row baseline refreshes

Bracketing Stage 1, before the relief work landed, was a wave of `*-current-refresh` batches that re-ran 50+ scenes against current packs to evict stale April matrix rows. Promoted refreshes: `mismatch-top-v072-current-refresh`, `stale-top-v072b`, `stale-zero-v072b`, `stale-zero2-v072b`, `stale-pressure2-v072c`, `stale-layout-v072c`, `stale-next-v072c`. Not a code change. Hygiene. But it surfaced the rows that were actually slow against the current build and stopped the optimizer queue from chasing ghosts. Maybe two to three percentage points came from this alone.

### FGP3 pack format conversions

The original FG2 pack format carried per-frame foreground deltas as a sparse rect-and-pixel stream. FGP3 is a denser variant: the same frame deltas, but compressed with a smaller header and a residual cleanup table that replaces the runtime's "did I miss a pixel" rebuild. Most scenes' high-tide and low-tide packs got rebuilt as FGP3. The win is per-frame upload bytes, which on a 2× CD pipeline is the biggest single bottleneck after raw playback.

### Scene-local prefetch relief

Prefetch budgets used to be global — every scene drew from the same window cache. The relief pass made the prefetch window scene-local, so a scene that needed less buffer didn't pay for the window the next scene wanted. Smaller per-scene prefetch buffers, fewer evictions during the window where scene N+1 was loading.

### Stream-window retuning

The CD streaming code had a stage-1 window of 32 KB by default. That's a sensible default and the wrong number for most scenes. A scene-by-scene retune (some up, some down) reduced blocking vblanks across the matrix by about a third. The number that mattered wasn't the window size — it was that the window had been *one number* before, and was now a per-scene setting backed by measurement.

### Padded residual packs (ACTIVITY 9)

ACTIVITY 9 is the wide-boat scene. Its source sprite extends past the legacy 640px scene clip, which caused the residual cleanup pass to miss bow / stern pixels at the clip boundary. The first fix was a per-scene patch script that filled the missing pixels from the decoded source. The padded-residual fix re-encoded the FGP3 pack with the residual cleanup table padded to cover the full sprite footprint, so the runtime gets the right cleanup data without a special case in the playback engine.

### Scoped low-tide read group (ACTIVITY 9)

ACTIVITY 9's low-tide variant has different shoreline geometry from its high-tide variant. The default read group bundled both tides into one CD layout group, which overrode the prefetch window in the wrong direction during low-tide playback. Scoping the low-tide read group separately let the streaming code make a tide-specific prefetch decision. The win was specific to ACTIVITY 9, but the *technique* — read groups can be scope-narrowed without breaking the streaming contract — generalizes.

## Rejected experiments

These are the changes that didn't land. Naming them out loud is the point of the experiment log.

### `-O2` compiler flag

`-O2` is the obvious first thing to try and the obvious first thing to break. The build at `-O2` produced a binary that loaded, ran, and broke FISHING 1's caption rendering after about 30 seconds of soak testing — the optimizer reordered something inside the caption stamp path that worked at `-O0`. The captions docs already noted that `FntFlush` is empirically broken in this scene-runtime context; `-O2` exposed an analogous fragility in the working caption renderer. The decision was to stay at `-O0` plus targeted hand optimizations, not lift the whole runtime to `-O2` and then chase ghosts.

### Naive read-group probes

The first read-group experiment tried to bundle every scene's high-tide and low-tide variants into one CD layout group on the assumption that two tides of the same scene "always go together." They don't — the screensaver loop picks tides independently — and bundling them slowed the prefetch window's first-tide read. The probe ran for two days, every measurement said the same thing, the experiment log got a row, the bundling came back out.

The full list of rejected probes lives in `docs/ps1/performance-experiment-log.md`. Future passes start from those rows so nobody re-tries the no-op tests.

## What's left

The current matrix mean is `99.6%` target speed. The remaining `0.4%`
lives in a small number of high-leverage rows. As of `{{ site.release.tag }}`
the only two red rows on the [battle card]({{ '/perf/' | relative_url }})
are `VISITOR 3` high (`72.5%`) and `VISITOR 3` low (`72.2%`) after
cleanup-metadata compaction plus guarded high-tide generated-window read
grouping. That scene's wide multi-view stitch hits the prefetch window the
hardest and is the largest single optimization target left. The yellow cluster
(twenty rows between
`80%` and `99%`) is the rest of the wide-action surface plus the
`BUILDING 2` and `BUILDING 4` clean-rect heavy frames, plus a couple of
other composite-frame rows. The optimization plan at
`docs/ps1/performance-optimization-plan.md` § 7 and § 8 lists about
thirty named experiments still on the bench. Some will land, some
will join the rejected log.

The home-page status strip carries the current target-speed pill (`{{ site.release.perf_target_speed_pct }}%` as of `{{ site.release.tag }}`); the live battle card is at [/perf/]({{ '/perf/' | relative_url }}).

## What the loop made obvious

A few things this work clarified that the validation grind didn't:

- **Two ledgers really are two ledgers.** The visual signoff loop and the headless perf loop have different cadences, different acceptance gates, and different failure modes. Mixing them produces neither.
- **`-O2` is not the answer to "make the runtime faster."** It's the answer to "go find a different bug in your runtime." Hand optimizations against measurement keep the failure surface small.
- **An experiment log is not paperwork.** The rejected rows pay for themselves the first time someone (often the same author, two months later) is about to try the same thing again.
- **A canary scene is the right unit of measurement.** FISHING 1's high-tide canary stayed on every release run as the baseline tracker. The matrix moved; the canary was the load-bearing reference frame.

The disc plays. That was always the point. It also plays at near native rate now, which means it plays *the way the original looked*, on the hardware nobody thought it would. That's the point too.

## Cross-links

- [/perf/]({{ '/perf/' | relative_url }}) — the live battle card this article is the back-story for.
- [/scenes/]({{ '/scenes/' | relative_url }}) — the visual signoff ledger; the parallel acceptance bar.
- [/docs/performance/]({{ '/docs/performance/' | relative_url }}) — reference manual for what each column on the battle card means.
- [/lab/the-63-scene-grind/]({{ '/lab/the-63-scene-grind/' | relative_url }}) — the prequel essay; the daily loop that closed the visual ledger and made this performance retrospective possible.
- [/lab/v081-mary4-freeze/]({{ '/lab/v081-mary4-freeze/' | relative_url }}) — the stability follow-on that left this article's matrix mean untouched.
- [/scenes/visitor3/]({{ '/scenes/visitor3/' | relative_url }}) — the slowest two rows; the largest single optimization target left.
- [/scenes/activity9/]({{ '/scenes/activity9/' | relative_url }}) — wide-boat scene; padded FGP3 + scoped low-tide read group case study.
- [/scenes/building4/]({{ '/scenes/building4/' | relative_url }}) — clean-rect-heavy variant; representative of the drop-prefetch unlock.
- [/scenes/fishing1/]({{ '/scenes/fishing1/' | relative_url }}) — the canary scene the matrix tracks against every release.
- [Glossary: FGP3]({{ '/docs/glossary/#fgp3' | relative_url }}) — the pack format most rebuilds converged on.
- [Glossary: prefetch window]({{ '/docs/glossary/#prefetch-window' | relative_url }}) — the buffer the drop-prefetch mechanism releases under clean-rect pressure.
- [Glossary: read group]({{ '/docs/glossary/#read-group' | relative_url }}) — the CD layout primitive the ACTIVITY 9 scope-narrowing exploited.
- [Glossary: experiment log]({{ '/docs/glossary/#experiment-log' | relative_url }}) — the long-form table of every accepted and rejected probe.
- [Glossary: canary scene]({{ '/docs/glossary/#canary' | relative_url }}) — the FISHING 1 high-tide load-bearing reference frame.
