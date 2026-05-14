# v0.8.9-ps1 Release Notes

**Date:** 2026-05-14
**Tag:** `v0.8.9-ps1`
**Theme:** WALKSTUF1 low in-place payload reductions

`v0.8.9-ps1` is a performance point release after `v0.8.8-ps1`. It keeps the
VISITOR5 high retained-read promotion from v0.8.8, the deterministic
BOOTMODE/Scene Explorer stability fixes from v0.8.7, and promotes the current
mainline performance work through `walkstuf1-low-frame35-inplace-v759`.

## Headline

- **VISITOR5 low joins high in green.** The low-tide row now uses the matching
  `30..46` retained-read shape, improving from `1104/1092` to `1102/1097` and
  moving the row into the green band.
- **BUILDING2 low gets both speed and work-volume wins.** The `218..229`
  slack-8 row and v739 draw-tail trim improve it to `1339/1317`, overrun `22`,
  blocking/read time `53`/`150`, reads `37`, and due misses `12`.
- **WALKSTUF1 low carries the newest no-shift pack baseline.** Frames `51`,
  `49`, `47`, `61`, `62`, `58`, `45`, `37`, `35`, `43`, `41`, `57`, `33`, and `67` now shrink in-place while
  preserving pack size, LBA/sectors, and the PS-EXE bucket. Timing stays
  exact-flat at `1478/1431`, overrun `47`, blocking/refill `64/20`, reads/read
  time `60/272`, and due misses `11`, while active payload drops
  `879801 -> 829912`.
- **Other current mainline wins are included.** VISITOR3 high keeps its
  `277..293` tail-pack repack, BUILDING4 low keeps the offscreen draw-span clip
  plus frame291 in-place shrink, BUILDING2 high keeps exact-flat offscreen
  work-volume clips, and WALKSTUF1 high keeps the late-tail/offscreen clip
  baseline.

## Current Battle Card

- **Public rollup:** `+0.2708%` over target / `99.7337%` target speed.
- **Raw signed rollup:** `-0.4963%` over target / `100.5160%` target speed.
- **Methodology total:** about `17.13` public over-target points removed and
  `+12.63` public target-speed points gained since the compact full-matrix
  baseline.
- **Bands:** `117` green, `9` yellow, `0` orange, `0` red.
- **Under-99 focus set:** WALKSTUF1 low, BUILDING2 high, WALKSTUF1 high,
  VISITOR3 high/low, BUILDING2 low, and JOHNNY1 high/low.

## Verification

- Latest focused gate:
  `scratch/ps1-perf-iterate/walkstuf1-low-frame35-inplace-v759-focused/20260514-091313-3805611/summary.json`.
- Latest matrix row:
  `docs/ps1/performance-scene-matrix.csv` at `2026-05-14T09:13:13` with stats
  version `walkstuf1-low-frame35-inplace-v759`.
- `./scripts/build-ps1.sh` passed before merging `origin/main`.
- `origin/main` was merged, then `./scripts/build-ps1.sh` passed again after
  the upstream PS1 runtime changes.
- The release image is produced by `./scripts/release.sh --version 0.8.9`.
