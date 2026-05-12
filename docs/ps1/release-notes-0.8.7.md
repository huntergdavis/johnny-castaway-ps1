# v0.8.7-ps1 Release Notes

**Date:** 2026-05-11
**Tag:** `v0.8.7-ps1`
**Theme:** Deterministic BOOTMODE scene selection + Scene Explorer preview stability

`v0.8.7-ps1` is a stability and operability point release after
`v0.8.6-ps1`. It does not promote a new performance matrix row. Instead, it
hardens the machinery we use to prove what the PS1 runtime actually booted,
fixes a Suzy backdrop state leak, and makes Scene Explorer preview loading
independent of a large paused-menu heap allocation.

## Headline

- **Direct scene booting is now auditable.** The PS1 runtime prints one early
  `JCBOOT` line immediately after BOOTMODE parsing, before title/resources/sound
  side effects. The line includes the boot source, explicit scene, seed, loop
  mode, tide/night/holiday/raft state, island position, and normalized boot
  text.
- **Headless performance runs now reject wrong-scene measurements.** The perf
  iterator derives the expected scene from `fgpilot <slug>` and fails the run if
  `JCPERF2 scene` reports a different scene or if the explicit scene falls
  through to `JCPICK`.
- **Suzy backdrop cleanup is fixed.** Scene-specific backdrop state now clears
  clean-bg black mode and frees clean-bg rect state on entry/cleanup. That
  prevents a black Suzy scene from leaking backdrop state into later scenes and
  causing island/tree layers to disappear.
- **Scene Explorer previews no longer allocate a 153 KB buffer.** Thumbnail SCRs
  stream in 16-row chunks through a static buffer directly to the centered
  framebuffer rect, so the chapter-select screen can load previews even after
  long, fragmented sessions.

## Current Battle Card

- **Public rollup:** `+0.3156%` over target / `99.6902%` target speed.
- **Raw signed rollup:** `-0.4529%` over target / `100.4740%` target speed.
- **Methodology total:** about `17.08` public over-target points removed and
  `+12.59` public target-speed points gained since the compact full-matrix
  baseline.
- **Bands:** `111` green, `15` yellow, `0` orange, `0` red.
- **Under-99 focus set:** WALKSTUF1 high/low, VISITOR3 high/low, BUILDING2
  high/low, VISITOR5 low, JOHNNY1 high, and the remaining yellow rows. This
  release does not change the performance row membership.

## Verification

- `./scripts/build-ps1.sh` passed after merging the current `origin/main`.
- A per-scene Suzy disc was built and inspected; the disc BOOTMODE contained
  `fgpilot suzy1 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 seed 1`.
- A visible DuckStation run logged `JCBOOT source=file fgpilot=1 scene=suzy1`
  and loaded `SUZY1.FG2`; the watched scene was Suzy 1.
- Scene validation scope remains unchanged: 63 / 63 scenes are signed off under
  the visual + audible bar.
- Performance scope remains unchanged: 126 / 126 scene/tide rows are routed and
  timing-bearing.
