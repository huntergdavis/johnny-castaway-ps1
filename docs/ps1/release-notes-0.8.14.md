# v0.8.14-ps1 Release Notes

**Date:** 2026-05-15
**Tag:** `v0.8.14-ps1`
**Theme:** JOHNNY1 local-LZ green promotion

`v0.8.14-ps1` is a performance point release collecting the JOHNNY1 local-LZ
full-frame payload promotion. It keeps all 63 scenes validated, keeps all 126
scene/tide rows routed and timing-bearing, and moves JOHNNY1 high/low from the
yellow band into green without changing the fixed pack footprint or PS-EXE
bucket.

## Headline

- **JOHNNY1 high/low advance to `johnny1-local-lz-v932`.** Entries `1` and
  `50` in both `JOHNNY1.FG2` and `JOHN1LOW.FG2` now store a scene-local
  copy/literal stream behind a payload sentinel.
- **The fixed layout is preserved.** Both packs stay at `448370` bytes, high
  remains LBA/sectors `13983/219`, low remains `14202/219`, and the PS-EXE
  bucket remains `217088` bytes.
- **Both tides move into green.** JOHNNY1 high/low improve from `1973/1945` to
  `1948/1945`, overrun `28 -> 3`, blocking/refill `25 -> 5`, loop read time
  `58 -> 37`, due misses `0`, and target speed `98.56% -> 99.85%`.
- **The remaining under-99 set is smaller.** JOHNNY1 high/low leave the yellow
  band; the remaining seven yellow rows are WALKSTUF1 low/high, BUILDING2
  high/low, VISITOR3 high/low, and BUILDING4 low.

## Current Battle Card

- **Public rollup:** `+0.2492%` over target / `99.7548%` target speed.
- **Raw signed rollup:** `-0.5179%` over target / `100.5371%` target speed.
- **Methodology total:** about `17.15` public over-target points removed and
  `+12.65` public target-speed points gained since the compact full-matrix
  baseline.
- **Bands:** `119` green, `7` yellow, `0` orange, `0` red.

## Verification

- JOHNNY1 high v932 proof:
  `scratch/ps1-perf-iterate/johnny1-local-lz-v932-high-rerun/20260515-101641-4098077/summary.json`.
- JOHNNY1 low v932 proof:
  `scratch/ps1-perf-iterate/johnny1-local-lz-v932-low-3600/20260515-102102-4124867/summary.json`.
- Transform summaries:
  `scratch/johnny1-local-lz-v932/JOHNNY1.summary.json` and
  `scratch/johnny1-local-lz-v932/JOHN1LOW.summary.json`.
- `./scripts/build-ps1.sh` and `git diff --check` passed before release prep.
  The release script rebuilds the PS1 executable, CD image, and static website
  again.
