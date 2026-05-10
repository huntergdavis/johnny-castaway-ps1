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
  rollup is `+0.3426%` over target / `99.6651%` target speed.
- **Raw optimization headroom remains visible.** The uncapped signed rollup is
  `-0.4259%` over target / `100.4489%` target speed, so future optimization
  work can still see when a scene runs faster than target.
- **Methodology total since the compact full-matrix baseline:** about `17.06`
  public over-target points removed and `12.57` public target-speed points
  gained.
- **Missing-scene confusion is closed.** MARY1/2/3 and SUZY1/2 are measured
  and green in the matrix; `suzy3` is not a standalone Johnny Castaway scene
  route, only an asset/reference naming source.

## Current Battle Card

- **Public rollup:** `+0.3426%` over target / `99.6651%` target speed.
- **Raw signed rollup:** `-0.4259%` over target / `100.4489%` target speed.
- **Bands:** `111` green, `15` orange, `0` yellow, `0` red.
- **Under-99 focus set:** WALKSTUF1 high/low, VISITOR3 high/low, BUILDING2
  high/low, VISITOR5 low, and JOHNNY1 high.
- **VISITOR3 baseline:** high `1075/1037` with `59` blocking VBlanks, `11`
  loop reads, and `11` due misses; low `1075/1039` with `69` blocking
  VBlanks, `16` loop reads, and `12` due misses.
- **WALKSTUF1 remains the largest outlier.** Recent cleanup-gap/core/edge
  probes lowered visible timing in places but were committed as log-only
  misses because hidden refill regressions made them unsafe to promote.

## Verification

- Static website/docs regenerated from the release metadata and source docs.
- Scene validation scope remains unchanged: 63 / 63 scenes are signed off
  under the visual + audible bar.
- Performance CSV and battle-card docs use the current 126 / 126 timing-bearing
  matrix as the release baseline.
