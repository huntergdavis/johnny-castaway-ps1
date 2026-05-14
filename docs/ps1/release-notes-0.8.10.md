# v0.8.10-ps1 Release Notes

**Date:** 2026-05-14
**Tag:** `v0.8.10-ps1`
**Theme:** WALKSTUF1 low no-shift payload follow-through

`v0.8.10-ps1` is a performance point release after `v0.8.9-ps1`. It keeps the
VISITOR5, VISITOR3, BUILDING2, BUILDING4, and WALKSTUF1 high wins from the
current release line, then promotes the current WALKSTUF1 low no-shift payload
baseline through `walkstuf1-low-frame76-inplace-v791`.

## Headline

- **WALKSTUF1 low keeps exact timing while dropping more payload work.** Frames
  `51`, `49`, `47`, `61`, `62`, `58`, `45`, `37`, `35`, `43`, `41`, `57`,
  `33`, `67`, `68`, `69`, `32`, `133`, `5`, `141`, `70`, `30`, `6`, `71`,
  `72`, `142`, `73`, `131`, `74`, `19`, `28`, `138`, `145`, `75`, and `76`
  now shrink in-place while preserving pack size, LBA/sectors, and the PS-EXE
  bucket.
- **The W1-low work-volume baseline is smaller.** Timing stays exact-flat at
  `1478/1431`, overrun `47`, blocking/refill `64/20`, read time `60/272`, and
  due misses `11`, while active payload drops `879801 -> 801103`.
- **The tactical queue is now explicit.** The battle card remains `117` green,
  `9` yellow, `0` orange, `0` red; the remaining yellow rows are WALKSTUF1
  low/high, BUILDING2 high/low, VISITOR3 high/low, JOHNNY1 high/low, and
  BUILDING4 low.

## Current Battle Card

- **Public rollup:** `+0.2708%` over target / `99.7337%` target speed.
- **Raw signed rollup:** `-0.4963%` over target / `100.5160%` target speed.
- **Methodology total:** about `17.13` public over-target points removed and
  `+12.63` public target-speed points gained since the compact full-matrix
  baseline.
- **Bands:** `117` green, `9` yellow, `0` orange, `0` red.

## Verification

- Latest focused gate:
  `scratch/ps1-perf-iterate/walkstuf1-low-frame76-inplace-v791-workreduction/20260514-150237-1630977/summary.json`.
- Latest matrix row:
  `docs/ps1/performance-scene-matrix.csv` at `2026-05-14T15:02:37` with stats
  version `walkstuf1-low-frame76-inplace-v791`.
- `origin/main` was merged at `13587be5b`, then `./scripts/build-ps1.sh`
  passed after the upstream PS1 runtime changes.
- The release image is produced by `./scripts/release.sh --version 0.8.10`.
