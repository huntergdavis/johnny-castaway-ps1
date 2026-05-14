# v0.8.12-ps1 Release Notes

**Date:** 2026-05-14
**Tag:** `v0.8.12-ps1`
**Theme:** WALKSTUF1 low frame77/frame130 payload trims

`v0.8.12-ps1` is a performance point release after the v0.8.11 lazy-stream
regression fix. It keeps the restored allocation baseline and promotes two more
same-speed WALKSTUF1 low no-shift payload reductions.

## Headline

- **WALKSTUF1 low advances to `walkstuf1-low-frame130-inplace-v795`.**
  Frames `77` and `130` now shrink in-place on top of the v791 release
  baseline without moving pack offsets, pack LBA/sectors, or the PS-EXE bucket.
- **The focused W1-low gate remains exact-flat.** Scene/loop/target stay
  `1770/1478/1431`, overrun `47`, blocking/refill `64/20`, loop reads/read
  VBlanks `60/272`, and due misses `11`.
- **Active payload drops again.** The no-shift lane now cuts WALKSTUF1 low
  active payload from `879801 -> 799694`; the latest two promoted entries cut
  frame `77` `4762 -> 4033` and frame `130` `4593 -> 3913`, with both entries
  dropping modeled sector coverage `3 -> 2`.

## Current Battle Card

- **Public rollup:** `+0.2708%` over target / `99.7337%` target speed.
- **Raw signed rollup:** `-0.4963%` over target / `100.5160%` target speed.
- **Methodology total:** about `17.13` public over-target points removed and
  `+12.63` public target-speed points gained since the compact full-matrix
  baseline.
- **Bands:** `117` green, `9` yellow, `0` orange, `0` red.

## Verification

- v794 accepted gate:
  `scratch/ps1-perf-iterate/walkstuf1-low-frame77-inplace-v794-workreduction/20260514-152935-1787930/summary.json`.
- v795 accepted gate:
  `scratch/ps1-perf-iterate/walkstuf1-low-frame130-inplace-v795-workreduction/20260514-153635-1826834/summary.json`.
- `./scripts/site-build-static-root.sh` and `git diff --check` passed before
  release prep. The release script rebuilds the PS1 executable, CD image, and
  static website again.
