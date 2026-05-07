# v0.8.1-ps1 Release Notes

**Date:** 2026-05-06
**Tag:** `v0.8.1-ps1`
**Theme:** clean-rect pressure stability

`v0.8.1-ps1` is a stability point release on top of the complete-scene
`v0.8.0-ps1` baseline. All 63 scenes remain validated, the headless
performance matrix remains routed, and this release fixes a long-run
scene-load freeze found during randomized DuckStation soak testing.

## Headline

- **Scene-load freeze fixed.** Large ocean clean-rect snapshots now estimate
  the real restore footprint before allocating, including the animated ocean
  wave band and upper/lower split rects.
- **Pressure relief is generalized.** The fix is not MARY4-specific; it covers
  every random-position scene whose foreground bounds expand into a large split
  clean snapshot.
- **Long-run diagnostics are stronger.** The new DuckStation long-run helper
  records run output under `scratch/` so future soak failures can be traced
  after the fact.
- **Performance baseline stays compatible.** Same-commit VISITOR3 high/low
  refreshes stayed exact to the current matrix baseline after the clean-rect
  estimator landed.

## Root Cause

The long-run freeze reproduced after a random scene load selected `MARY 4`.
TTY logs reached the large split clean-rect save:

- `JCPICK ... mary4`
- `JCRECT 2-rect lower=(0,190,608,166) upper=(0,92,361,98)`

The runtime then stopped before the expected scene-playback lines. The pressure
estimator used `fgBoundsW * fgBoundsH * 2`, which ignored both the ocean wave
band expansion and the upper/lower split save. In the failing route, the real
clean background backup was over 256 KiB, but the estimator under-counted it,
so optional prefetch/walk memory was not released early enough.

## Fix

`foreground_pilot.c` now computes clean-rect pressure through one helper that
mirrors the actual clean-save path. The helper accounts for the pack bounds,
the ocean wave band, clamping, and the split at the low-tide/water boundary.
`fgPlayOceanRuntimeScene()` uses that estimate before saving clean backgrounds,
so large scenes can drop optional pressure before the heap gets fragmented.

The affected scan found 14 non-exempt random-position scenes covered by this
centralized path: `activity1`, `activity5`, `fishing1`, `fishing2`,
`fishing3`, `fishing7`, `fishing8`, `johnny2`, `johnny4`, `johnny5`, `mary4`,
`mary5`, `visitor4`, and `walkstuf3`.

## Verification

- `./scripts/build-ps1.sh` passed.
- Focused `MARY 4` low-pressure route completed with
  `JCMEM large-clean scene=mary4 bytes=265916 drop-prefetch`, `scene-end`, and
  `alloc_fail=0`.
- Representative `FISHING 1` low-pressure route completed with
  `JCMEM large-clean scene=fishing1 bytes=327816 drop-prefetch`, `scene-end`,
  and `alloc_fail=0`.
- VISITOR3 high/low same-commit refresh stayed at the current matrix baseline:
  high `loop_vb=1450`, low `loop_vb=1452`.

The release remains a bugfix/stability point release. Scene validation is still
63 / 63, and the speed battle card remains the `v0.8.0` performance baseline
plus the accepted post-baseline BUILDING5/WALKSTUF1/VISITOR3 improvements.
