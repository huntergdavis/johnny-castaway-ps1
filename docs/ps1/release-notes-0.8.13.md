# v0.8.13-ps1 Release Notes

**Date:** 2026-05-15
**Tag:** `v0.8.13-ps1`
**Theme:** Under-99 payload-work checkpoint

`v0.8.13-ps1` is a performance point release collecting the post-v0.8.12
headless optimization work. It keeps all 63 scenes validated, keeps all 126
scene/tide rows routed and timing-bearing, and preserves the current
public-capped battle card while banking lower foreground payload, CD-work, and
documented miss data for the remaining yellow rows.

## Headline

- **WALKSTUF1 low advances to `walkstuf1-low-frame106-inplace-v910`.** The
  low row holds `1769/1477/1432`, overrun `45`, blocking/refill `65/20`, loop
  reads/read VBlanks `58/259`, and due misses `11`, while the in-place
  no-shift payload lane cuts active payload to `790208` bytes.
- **BUILDING2 high advances to `building2-high-frame173-inplace-v914`.** The
  row remains exact-flat at `1602/1351/1311`, overrun `40`,
  blocking/refill `54/18`, reads/read time `58/257`, and due `7`, while active
  payload drops to `669488` bytes through preserve-offset trims.
- **BUILDING4 low advances to `building4-low-frame283-inplace-v913`.** The row
  stays exact-flat at `2853/2816`, overrun `37`, blocking/refill `40/34`, and
  reads/read time `30/215`, while active payload drops to `808412` bytes.
- **Recent misses are now logged and closed.** JOHNNY1 black-clear primitive
  paths, BUILDING2 high `17..33` scalar rows, and WALKSTUF1 low frame132
  sector-boundary trimming are recorded as non-promotable on this baseline.

## Current Battle Card

- **Public rollup:** `+0.2697%` over target / `99.7347%` target speed.
- **Raw signed rollup:** `-0.4975%` over target / `100.5171%` target speed.
- **Methodology total:** about `17.13` public over-target points removed and
  `+12.63` public target-speed points gained since the compact full-matrix
  baseline.
- **Bands:** `117` green, `9` yellow, `0` orange, `0` red.
- **Remaining yellow rows:** WALKSTUF1 low/high, BUILDING2 high/low,
  VISITOR3 high/low, JOHNNY1 high/low, and BUILDING4 low.

## Verification

- WALKSTUF1 low v910 work gate:
  `scratch/ps1-perf-iterate/walkstuf1-low-frame106-inplace-v910-workreduction/20260515-062538-2758861/summary.json`.
- BUILDING2 high v914 work gate:
  `scratch/ps1-perf-iterate/building2-high-frame173-inplace-v914-workreduction/20260515-070731-3000534/summary.json`.
- BUILDING4 low v913 work gate:
  `scratch/ps1-perf-iterate/building4-low-frame283-inplace-v913-workreduction/20260515-065458-2928182/summary.json`.
- `./scripts/build-ps1.sh` and `git diff --check` passed before release prep.
  The release script rebuilds the PS1 executable, CD image, and static website
  again.
