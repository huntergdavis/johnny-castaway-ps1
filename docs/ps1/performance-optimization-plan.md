# PS1 Scene Playback Performance Optimization Plan

Date: 2026-04-25

Baseline commit: `f704312c ps1: keep fg2 overlays scene-relative to island`

Scope: active PS1 scene playback runtime only. The target is faster playback
without frame dropping, skipped art, timing lies, or regression away from the
pixel-perfect FG2 methodology.

## Executive Summary

The active path is not slow because the current fishing FG2 packs are huge per
frame. The six routed fishing packs read small frame payloads, usually under
10 KB. The larger problem is that the runtime repeats expensive restore,
compose, and VRAM upload work for unchanged held VBlanks.

Top likely wins, in order:

| Rank | Optimization | Expected impact | Reason |
|---|---|---|---|
| 1 | Held-entry no-work path | Very high | 90-96% of current fishing FG2 entries hold for more than one VBlank, but the runtime redraws every VBlank. |
| 2 | Row/X-aware dirty restore and upload | Very high | Current rect-mode can upload 216-358 KB per VBlank for fishing scenes; exact row extents average 21-32 KB. |
| 3 | Specialized PAL4 FG2 compositor | High | Current code does per-pixel tile selection and transparency checks even though FG2 spans are already nonzero runs. |
| 4 | FG2 stream window / chunk cache | Medium to high | Current code issues a synchronous CD read per new FG2 entry. A 32-64 KB window can cover multiple entries. |
| 5 | Pack-time precomputed draw commands | Medium to high | The pack compiler can emit PS1-ready per-tile row commands, reducing runtime parsing and clipping. |

Non-goals:

| Non-goal | Reason |
|---|---|
| Frame dropping | Violates pixel-perfect playback. |
| Timing compression before throughput work | If the PS1 cannot process frames faster, shorter timing files only expose the same bottleneck. |
| Reintroducing FG1, ADS, or TTM runtime paths | Those were retired from the active public path. |
| Fixed island assumptions | The runtime must randomly place the island, so all optimizations must preserve scene-relative FG2 placement. |
| Direct framebuffer or progressive-mode experiments as first moves | Prior history says these were unstable. Exhaust stable scene playback first. |

## Current Runtime Shape

Active runtime files:

| File | Relevant role |
|---|---|
| `src/foreground_pilot.c` | FG2 selection, header/table/palette loading, frame timing, per-frame CD read, sound-event firing, scene loop. |
| `src/graphics_ps1.c` | RAM background tiles, clean-rect restore, PAL4/indexed8 FG2 span compositing, dirty-row upload, `LoadImage` calls. |
| `src/cdrom_ps1.c` | Sector reads, `CdSearchFile`, buffered read-into APIs, pack/resource loading. |
| `scripts/build-scene-foreground-pack.py` | FG2 pack compiler, base-diff crop, palette selection, row-span encoding, sound events. |
| `config/ps1/cd_layout.xml` | Routed FG2 files and active SCR/PSB/SND payloads. |

Current frame loop in `fgPlayOceanRuntimeScene`:

```c
while (foregroundPilotRuntimeActive()) {
    grBeginFrame();
    grRestoreBgFromRects();
    if (!fgRuntimeUsesBaseDiffBackdrop())
        fgBackdropTickBackgroundWaves();
    grUpdateDisplay(NULL, NULL, NULL);
    foregroundPilotRuntimeAdvance();
}
```

For base-diff FG2 packs, `fgBackdropTickBackgroundWaves()` is skipped, but
`grRestoreBgFromRects()`, `foregroundPilotRuntimeCompose()`, and
`grDrawBackground()` still run every displayed VBlank.

## Measured Pack Evidence

These figures were computed from `generated/ps1/foreground/*.FG2` at baseline
commit `f704312c`.

Active routed packs:

| Pack set | Count | Total size | Max pack | Max frame payload | Encoding |
|---|---:|---:|---:|---:|---|
| Fishing routed high/low packs | 6 | 6.08 MB | 1.75 MB (`FISHING3.FG2`) | 9.1 KB | PAL4 |

All generated scene corpus:

| Pack set | Count | Total size | Max pack | Max frame payload | Encoding mix |
|---|---:|---:|---:|---:|---|
| All high/low FG2 packs | 126 | 342.75 MB | 32.93 MB (`MARY3LOW.FG2`) | 189.8 KB | 122 PAL4, 4 indexed8 |

Active pack details:

| Pack | Frames | Payload | Max frame | Avg visible pixels | Avg spans | Sound events |
|---|---:|---:|---:|---:|---:|---:|
| `FISHING1.FG2` | 156 | 807.2 KB | 7.0 KB | 4535 | 593 | 9 |
| `FISH1LOW.FG2` | 137 | 413.3 KB | 4.7 KB | 2445 | 336 | 9 |
| `FISHING2.FG2` | 295 | 1552.3 KB | 7.0 KB | 4708 | 597 | 9 |
| `FISH2LOW.FG2` | 247 | 760.8 KB | 4.8 KB | 2587 | 338 | 9 |
| `FISHING3.FG2` | 329 | 1782.2 KB | 9.1 KB | 4943 | 610 | 21 |
| `FISH3LOW.FG2` | 269 | 879.4 KB | 7.0 KB | 2853 | 352 | 21 |

Timing hold evidence:

| Pack | Entries | Target VBlanks | Avg hold | Max hold | Entries with hold > 1 |
|---|---:|---:|---:|---:|---:|
| `FISHING1.FG2` | 156 | 897 | 5.75 | 8 | 94.2% |
| `FISH1LOW.FG2` | 137 | 897 | 6.55 | 16 | 95.6% |
| `FISHING2.FG2` | 295 | 1473 | 4.99 | 8 | 90.8% |
| `FISH2LOW.FG2` | 247 | 1473 | 5.96 | 16 | 92.7% |
| `FISHING3.FG2` | 329 | 1636 | 4.97 | 11 | 94.2% |
| `FISH3LOW.FG2` | 269 | 1636 | 6.08 | 16 | 95.2% |

Interpretation: if a frame is held for 5 VBlanks, the current loop redraws the
same pixels 5 times. For base-diff FG2, the framebuffer can simply remain on
screen for the unchanged VBlanks.

## Dirty Restore And Upload Evidence

Current rect-mode backs up a scene-sized clean region, restores it every
VBlank, then marks the entire rect dirty. `grDrawBackground()` tracks dirty
state at tile row granularity, so a narrow dirty band can still become a full
320-pixel-wide row upload per tile.

Estimated current restore/upload cost under legal `VARPOS_OK` island positions:

| Pack | Current upload best | Current upload worst | Restore best | Restore worst |
|---|---:|---:|---:|---:|
| `FISHING1.FG2` | 216 KB | 315 KB | 195 KB | 288 KB |
| `FISH1LOW.FG2` | 216 KB | 315 KB | 205 KB | 307 KB |
| `FISHING2.FG2` | 216 KB | 315 KB | 183 KB | 276 KB |
| `FISH2LOW.FG2` | 216 KB | 315 KB | 194 KB | 290 KB |
| `FISHING3.FG2` | 263 KB | 358 KB | 239 KB | 334 KB |
| `FISH3LOW.FG2` | 263 KB | 358 KB | 239 KB | 334 KB |

Estimated exact per-frame row extent upload from the current FG2 span data:

| Pack | Avg exact row bytes | Max exact row bytes | Avg row rects |
|---|---:|---:|---:|
| `FISHING1.FG2` | 23.6 KB | 41.0 KB | 179 |
| `FISH1LOW.FG2` | 21.3 KB | 37.7 KB | 160 |
| `FISHING2.FG2` | 31.9 KB | 41.8 KB | 185 |
| `FISH2LOW.FG2` | 29.3 KB | 38.5 KB | 168 |
| `FISHING3.FG2` | 29.6 KB | 63.7 KB | 174 |
| `FISH3LOW.FG2` | 26.0 KB | 57.3 KB | 160 |

Interpretation: after held-frame no-work, row/X-aware restore and upload is
the next order-of-magnitude candidate. The challenge is batching row extents
into a safe number of `LoadImage` rectangles.

## Guardrails

| Guardrail | Practical rule |
|---|---|
| Pixel-perfect means no skipped entries | Every FG2 entry must still be presented in order. |
| Faster means less work, not shorter truth | Timing can only be tightened after throughput improves. |
| One optimization at a time | Each change gets a clean build, DuckStation run, and human visual/audio check. |
| Scene-relative is mandatory | Island `xPos/yPos` must remain applied to FG2 overlays and backdrop sprites. |
| No per-frame allocation | All new buffers must be scene-persistent or static with bounded size. |
| No per-frame text I/O | `printf()` is available for gated setup/teardown probes, but not inside timing-critical playback loops. |
| Keep rollback easy | Each optimization should be a small commit with a single exit hatch. |

## Phase 0: Measurement Before More Tuning

Goal: make performance visible without changing playback semantics. Use overlay
or fixed counters for measurement; reserve `printf()` for rare, gated
diagnostic breadcrumbs.

| ID | Task | Result |
|---|---|---|
| `P0-01` | Add optional performance counters for restore, compose, upload, CD read, and advance. | Know which budget is actually dominant per scene. |
| `P0-02` | Count bytes restored, bytes uploaded, FG2 bytes read, spans composited, visible pixels, and `LoadImage` calls. | Avoid judging speed by feel only. |
| `P0-03` | Display counters in an existing on-screen overlay field or a tiny dedicated overlay panel. | Safe on hardware/emulator; no log dependency. |
| `P0-04` | Store counters in a fixed in-memory struct with `volatile` fields and bounded integer widths. | Avoid heap churn and compiler elision. |
| `P0-05` | Add a `perf 1` boot token or compile-time flag that only toggles overlay rendering and counter collection. | Keep normal releases clean. |
| `P0-06` | Capture baseline counters by screenshot/manual readout for `fishing1`, `fishing1 lowtide`, `fishing2`, `fishing3`. | Establish before/after comparisons without depending on log output. |
| `P0-07` | If persistent records are needed later, write them into a bounded RAM ring buffer inspectable via overlay pages. | Avoid CD writes and high-volume console output. |

Instrumentation rules:

| Rule | Reason |
|---|---|
| Do not add `printf()`, `fprintf()`, `ps1DebugPrint()`, or other console text emission to the per-frame perf path. | Text I/O is noisy and can alter timing. |
| Do not use DuckStation console logs as primary validation data. | Logs are emulator-only, high volume, and not representative of real hardware. |
| Do not allocate per-frame strings or format counters per frame. | Formatting cost would contaminate the measurement. |
| Prefer raw integer counters rendered by a minimal overlay. | Keeps measurement close to the real workload. |
| Gate all instrumentation. | Release playback must not pay the measurement cost. |

## Phase 1: Held-Entry No-Work Path

Goal: render each FG2 entry once, then hold the framebuffer unchanged for the
remaining VBlanks.

| ID | Task | Rationale |
|---|---|---|
| `P1-01` | Add a base-diff-only no-redraw hold path in `fgPlayOceanRuntimeScene`. | Base-diff packs already include visible background changes, and runtime waves are disabled. |
| `P1-02` | Track whether the current FG2 entry has already been rendered. | Prevent repeated restore/compose/upload for held VBlanks. |
| `P1-03` | Add a display-only wait path that handles `VSync` and controller polling without `LoadImage`. | Preserve timing and input while doing no visual work. |
| `P1-04` | Compare frame index, source frame, and sound-event timing before/after. | Avoid subtle SFX drift. |
| `P1-05` | Validate with `fishing1` high, low, holiday, night, raft, and forced island positions. | First acceptance scene must stay perfect. |
| `P1-06` | Validate with `fishing2` and `fishing3`. | Ensure longer scenes and larger payloads still advance correctly. |

Expected impact: if restore/compose/upload dominates, this can cut visual work
by roughly 80% on current fishing scenes. It does not skip any FG2 entry.

Red-team risks:

| Risk | Mitigation |
|---|---|
| Sound fires one hold too early or late | Keep `fgRuntimeLoadSceneFrame` timing unchanged; verify splash/catch cues. |
| Pause/debug input feels less responsive | Call the input wait path on every hold VBlank. |
| Non-base-diff scenes need animated runtime background | Gate the no-work path to base-diff FG2 only. |
| Last-frame hold/end behavior changes | Test scene completion and screensaver loop repeat. |

## Phase 2: Row/X-Aware Dirty Restore And Upload

Goal: stop restoring and uploading full scene-sized clean rects when the actual
FG2 spans touch a much smaller row/X set.

| ID | Task | Rationale |
|---|---|---|
| `P2-01` | Replace per-tile `minY/maxY` dirty state with per-row `minX/maxX` state. | Preserve horizontal precision from FG2 spans. |
| `P2-02` | Keep `curr` and `prev` row extents per tile. | Restore previous pixels and upload union of previous/current changes. |
| `P2-03` | Change `grMarkRectDirty` to update row X extents, not just row bands. | Existing callers get better precision automatically. |
| `P2-04` | Add a clean-rect restore function that copies only previous row X extents. | Avoid 200-350 KB per-frame clean copies. |
| `P2-05` | Batch row extents into a bounded list of `LoadImage` rectangles. | Avoid one `LoadImage` per row. |
| `P2-06` | Use 8- or 16-pixel X bucket rounding for better batching. | Trade tiny extra upload for far fewer rectangles. |
| `P2-07` | Add a fallback merge policy when rect count exceeds a cap. | Avoid command overhead spikes on dense frames. |
| `P2-08` | Record bytes actually uploaded in telemetry. | Confirm real win. |

Expected impact: current active fishing scenes often upload 216-358 KB per
VBlank. Exact row extents average 21-32 KB. A practical batching strategy
should land between those numbers and much closer to the exact side.

Red-team risks:

| Risk | Mitigation |
|---|---|
| Stale pixels from under-restoring | Restore the union of previous frame row extents, not just current frame. |
| `LoadImage` command overhead beats byte savings | Use bucketed row merging and a rectangle cap. |
| Clean source rect does not cover a dirty row | Assert or telemetry-flag misses, then fall back to full rect for that frame. |
| Random island placement moves bounds negative or across tile splits | Clamp after applying `islandState.xPos/yPos`, and test extreme legal positions. |

## Phase 3: Specialized FG2 Compositors

Goal: make the compositor match the FG2 contract instead of treating every
span like an arbitrary transparent sprite.

| ID | Task | Rationale |
|---|---|---|
| `P3-01` | Split PAL4 span compositing by tile once per span, not once per pixel. | Remove thousands of repeated tile tests. |
| `P3-02` | Remove transparent-index checks inside FG2 spans. | Pack compiler only emits nonzero contiguous spans. |
| `P3-03` | Add a 256-entry PAL4 byte-to-two-pixels LUT per FG2 palette. | Convert two pixels per byte with one lookup. |
| `P3-04` | Use aligned 32-bit stores for even destination X and even pixel count. | Halve store count on the common path. |
| `P3-05` | Keep odd-left and odd-right edge handlers simple. | Preserve exact pixels around unaligned spans. |
| `P3-06` | Add an indexed8 fast path with direct palette lookup and no transparent checks. | Future scenes include indexed8 FG2 packs. |
| `P3-07` | Consider pack-time direct16 row commands for high-cost scenes only. | Removes palette work at runtime but increases pack size. |
| `P3-08` | Keep the old compositor behind a debug token until the new path is validated. | Fast rollback. |

Expected impact: active frames average only 2.4K-4.9K visible pixels, so this
is likely behind held-frame and dirty-upload fixes for fishing. It becomes more
important for scenes like Suzy and Mary with 100K+ visible pixels per frame.

## Phase 4: FG2 Streaming And CD Access

Goal: reduce synchronous CD operations without reintroducing the fragile
read-ahead behavior called out in the historical timing plan.

| ID | Task | Rationale |
|---|---|---|
| `P4-01` | Add a scene-local stream window over the current FG2 file. | Entries are stored sequentially, so one read can cover multiple frames. |
| `P4-02` | Start with a 32 KB or 64 KB blocking window after Phase 1 and Phase 2. | Lower risk than async prefetch. |
| `P4-03` | Serve current entry data directly from the window when possible. | Avoid `CdControl` and `CdReadSync` per entry. |
| `P4-04` | Refill the window at entry boundaries only. | Keep frame identity exact. |
| `P4-05` | Add adaptive window sizing by max frame payload. | Fishing can use small windows; large Mary/Suzy frames may need different policy. |
| `P4-06` | Align FG2 data chunks to sector boundaries as an optional pack mode. | Enables fewer scratch copies for large frames. |
| `P4-07` | Test true async prefetch only after blocking window is proven. | Prior naive read-ahead failed; do not start there. |
| `P4-08` | Keep one persistent scratch buffer per scene. | Preserve the fishing3 memory-leak fix. |

Red-team risks:

| Risk | Mitigation |
|---|---|
| Window refill causes visible hitch | Measure with counters first; then consider async only for refill. |
| Large scenes exceed memory budget | Bound window size and fall back to direct buffered read. |
| Off-by-one frame data after refill | Validate with checksums or entry index telemetry in debug builds. |
| CD controller state regressions | Keep `CdSearchFile` amortized, avoid new per-frame searches, and test title boot every time. |

## Phase 5: Pack Format Improvements

Goal: move repeatable parsing and clipping work out of the PS1 runtime.

| ID | Task | Rationale |
|---|---|---|
| `P5-01` | Emit per-frame row extents in the entry table or a side table. | Dirty restore/upload can avoid parsing spans twice. |
| `P5-02` | Emit per-tile command streams. | Runtime no longer splits spans around x=320/y=240. |
| `P5-03` | Emit row offsets for each encoded frame. | Runtime can jump rows without scanning from the beginning. |
| `P5-04` | Add a versioned `FGP3` format for optimized commands. | Keep `FGP2` available during migration. |
| `P5-05` | Use `uint8` relative row deltas where legal. | Shrink command overhead for small bboxes. |
| `P5-06` | Use `uint16` or varint payload sizes per row. | Reduce overhead without complex decompression. |
| `P5-07` | Pre-bucket X extents to the upload batching granularity. | Runtime can directly merge rows. |
| `P5-08` | Add per-scene capability flags: pal4-pair-lut, row-extents, tile-split, sector-aligned. | Runtime can select fast paths safely. |
| `P5-09` | Emit pack-stat JSON alongside every FG2/FGP3. | Make routing decisions data-driven. |
| `P5-10` | Add a corpus scanner that flags scenes with full-screen diffs. | Plan special handling for Suzy, Mary3, Activity9. |

## Phase 6: Scene Startup And Backdrop Cost

Goal: keep per-scene setup from stealing heap and causing long transitions.

| ID | Task | Rationale |
|---|---|---|
| `P6-01` | Keep `BACKGRND.PSB` persistent across scene loops. | Already done; preserve it. |
| `P6-02` | Keep `MRAFT.PSB` persistent when raft stage remains active. | Avoid repeat CD load on screensaver loops. |
| `P6-03` | Consider keeping `HOLIDAY.PSB` persistent across loops. | Avoid reload when random holiday repeats. |
| `P6-04` | Remove `ISLETEMP.SCR` from active CD layout if no active path uses it. | Smaller CD and fewer accidental fixed-island regressions. |
| `P6-05` | Build a minimal palette/metadata table to replace `RESOURCE.001` dependence. | Potential CD size and startup simplification. |
| `P6-06` | Track scene-start heap largest allocation in debug overlay. | Catch fragmentation before long soaks. |
| `P6-07` | Validate all scene cleanup paths with overnight loop. | Screensaver stability remains non-negotiable. |

## Phase 7: Build And Binary-Level Optimizations

Goal: keep the executable small and hot code friendly.

| ID | Task | Rationale |
|---|---|---|
| `P7-01` | Keep `-O2`, `-G8`, and function/data sections as the baseline. | Current flags are already sensible. |
| `P7-02` | Test `-O3` only on hot compositor files. | Whole-program `-O3` may grow code and hurt I-cache. |
| `P7-03` | Split hot compositor code into a small translation unit if needed. | Easier to test flags per file. |
| `P7-04` | Consider MIPS assembly only after C fast paths are measured. | Assembly should target proven hot loops. |
| `P7-05` | Remove or compile-gate unused debug/text formatting in release builds. | `vsnprintf` and debug paths are visible in the map. |
| `P7-06` | Skip `ClearOTagR` in pure FG2 software-background frames if safe. | Active playback often does not emit GPU primitives. |
| `P7-07` | Keep the primitive path available for debug/other scenes. | Avoid breaking future sandbox work. |

## Phase 8: Large-Scene Strategy

Goal: avoid optimizing only fishing and then failing on Mary/Suzy/Activity.

| ID | Task | Rationale |
|---|---|---|
| `P8-01` | Classify packs by max frame payload and average visible pixels. | Large scenes need different policies. |
| `P8-02` | Special-case full-screen packs for direct tile memcpy paths. | Span overhead is poor when almost every pixel changes. |
| `P8-03` | Consider scene-specific base selection for full-screen outliers. | Better base frames can reduce false diffs. |
| `P8-04` | Revisit capture masks for scenes that diff the entire screen. | Some full-screen diffs may be capture artifact, not real animation. |
| `P8-05` | Add a "must benchmark before routing" flag for packs over a threshold. | Prevent routing a 30 MB pack blindly. |
| `P8-06` | Keep CD image route scene-by-scene, not all 126 packs. | Disk budget and validation discipline. |

Large-scene outliers from the current corpus:

| Pack | Pack size | Max frame payload | Note |
|---|---:|---:|---|
| `MARY3LOW.FG2` | 32.93 MB | 189.8 KB | Indexed8, very large diffs. |
| `MARY3.FG2` | 32.35 MB | 186.7 KB | Indexed8, very large diffs. |
| `ACTIVITY9.FG2` | 28.36 MB | 111.8 KB | PAL4 but broad screen diffs. |
| `ACTV9LOW.FG2` | 28.07 MB | 114.7 KB | PAL4 but broad screen diffs. |
| `SUZY1.FG2` | 19.68 MB | 153.8 KB | Full-screen style diffs. |
| `SUZY2.FG2` | 13.07 MB | 153.8 KB | Full-screen style diffs. |

## Phase 9: Validation Protocol

Every performance change should use this checklist:

| Step | Check |
|---|---|
| 1 | `./scripts/build-ps1.sh clean` succeeds. |
| 2 | `./scripts/make-cd-image.sh` succeeds. |
| 3 | Run `fgpilot fishing1` default. |
| 4 | Run `fgpilot fishing1 lowtide 1`. |
| 5 | Run `fgpilot fishing1 night 1`. |
| 6 | Run `fgpilot fishing1 holiday 4`. |
| 7 | Run `fgpilot fishing1 raft-stage 5`. |
| 8 | Run forced island extremes from `VARPOS_OK`. |
| 9 | Run `fgpilot fishing2` and `fgpilot fishing3`. |
| 10 | Confirm sound timing, especially splash/catch events. |
| 11 | Let a loop run long enough to catch memory regressions. |
| 12 | Commit only after human visual/audio signoff. |
| 13 | Confirm the change added no per-frame `printf()`/console-output dependency. |

Forced island positions to keep using:

| Position | Why |
|---|---|
| `island-pos -222 -44` | Upper-left edge of first `VARPOS_OK` branch. |
| `island-pos -113 84` | Lower-right edge of first branch. |
| `island-pos -114 -14` | Second-branch boundary. |
| `island-pos 20 85` | Right/lower extreme. |
| `island-pos -114 -73` | Highest island case. |
| `island-pos 5 -13` | Third-branch right edge. |

## Recommended First Five Experiments

| Order | Experiment | Commit only if |
|---:|---|---|
| 1 | Add optional performance counters and overlay values with no per-frame `printf()` or log dependency. | Counters do not change playback and no visual regression. |
| 2 | Implement base-diff held-entry no-work path. | Fishing1 variants remain perfect and feel faster. |
| 3 | Add row/X dirty state and upload batching behind a debug flag. | Byte counters drop and no stale pixels appear. |
| 4 | Specialize PAL4 FG2 compositor with span-level tile split and pair LUT. | Same pixels, lower compose counters. |
| 5 | Add blocking FG2 stream window. | Fewer CD reads with no stutter or frame identity bugs. |

## Red-Team Conclusions

The safest near-term speedup is not a more aggressive timing file. The current
runtime appears to be doing too much work for identical held frames. That is a
semantic mismatch between the FG2 pack's timing model and the renderer loop,
not an artistic tradeoff.

The second major issue is dirty precision. The current clean-rect system solved
memory stability, but it throws away too much horizontal information. The FG2
pack already knows exact row spans. Preserving that information through restore
and upload is the best path toward an order-of-magnitude reduction without
changing pixels.

CD prefetching is still valuable, but it should come after the renderer stops
redrawing held frames and uploading huge row bands. Otherwise prefetching risks
masking the wrong bottleneck and reviving the fragile read-ahead failures from
the older foreground timing work.
