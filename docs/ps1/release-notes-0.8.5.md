# v0.8.5-ps1 Release Notes

**Date:** 2026-05-09
**Tag:** `v0.8.5-ps1`
**Theme:** Full headless performance matrix baseline

`v0.8.5-ps1` is a performance/methodology point release after
`v0.8.4-ps1`. It keeps v0.8.4's custom Scene Explorer thumbnails and
reconciled scene metadata, then promotes the current full 126-row headless
matrix as the public performance baseline.

## Headline

- **126 / 126 scene/tide rows are timing-bearing.** Every routed high/low
  scene variant now contributes active-loop timing to the matrix.
- **Public performance is effectively at native speed.** The public-capped
  rollup is `+0.3215%` over target / `99.6847%` target speed.
- **Raw optimization headroom remains visible.** The uncapped signed rollup is
  `-0.4470%` over target / `100.4685%` target speed, so future optimization
  work can still see when a scene runs faster than target.
- **Methodology total since the compact full-matrix baseline:** about `17.08`
  public over-target points removed and `12.58` public target-speed points
  gained.
- **Missing-scene confusion is closed.** MARY1/2/3 and SUZY1/2 are measured
  and green in the matrix; `suzy3` is not a standalone Johnny Castaway scene
  route, only an asset/reference naming source.

## Current Battle Card

- **Public rollup:** `+0.3215%` over target / `99.6847%` target speed.
- **Raw signed rollup:** `-0.4470%` over target / `100.4685%` target speed.
- **Bands:** `111` green, `15` orange, `0` yellow, `0` red.
- **Under-99 focus set:** WALKSTUF1 low/high, VISITOR3 high/low, BUILDING2
  high/low, VISITOR5 high/low, JOHNNY1 high/low, BUILDING4 low, BUILDING6
  high/low, and JOHNNY6 high/low.
- **VISITOR3 baseline:** high `1070/1039` with `49` blocking VBlanks, `9`
  loop reads, and `9` due misses after the v299 frame-131 resident-alias
  promotion; low is `visitor3-low-alias-noop114117-v292` at `1075/1039` with `67`
  blocking VBlanks, `12` loop reads, and `12` due misses.
- **WALKSTUF1 low moved again.** The v289 gap1 prefix pack moves low to
  `1478/1428` (96.6% speed) after the v288 high gap1/window-prefetch guard
  moved high to `1477/1431` (96.9% speed).

## Verification

- Static website/docs regenerated from the release metadata and source docs.
- Scene validation scope remains unchanged: 63 / 63 scenes are signed off
  under the visual + audible bar.
- Performance CSV and battle-card docs use the current 126 / 126 timing-bearing
  matrix as the release baseline.
