# v0.8.8-ps1 Release Notes

**Date:** 2026-05-13
**Tag:** `v0.8.8-ps1`
**Theme:** VISITOR5 high retained-read promotion

`v0.8.8-ps1` is a performance point release after `v0.8.7-ps1`. It keeps the
deterministic boot-selection and Scene Explorer stability fixes from v0.8.7,
then promotes the verified VISITOR5 high-tide retained-read row.

## Headline

- **VISITOR5 high moves into green.** The high-tide runtime now uses a
  `30..46` retained-read group while the low-tide row keeps its existing
  `23..47` group.
- **Visible CD pressure drops.** VISITOR5 high improves from `1107/1090` to
  `1101/1096`; overrun falls `17 -> 5`, blocking/refill falls `16 -> 5`,
  loop reads fall `19 -> 18`, and due misses remain `0`.
- **The public battle card improves again.** The 126 timing-bearing rows now
  average `+0.2867%` over target / `99.7183%` target speed. Raw signed rollup
  is `-0.4805%` / `100.5006%`.
- **Band shape is cleaner.** The matrix now has `116` green rows, `10` yellow
  rows, `0` orange, and `0` red.

## Current Battle Card

- **Public rollup:** `+0.2867%` over target / `99.7183%` target speed.
- **Raw signed rollup:** `-0.4805%` over target / `100.5006%` target speed.
- **Methodology total:** about `17.11` public over-target points removed and
  `+12.62` public target-speed points gained since the compact full-matrix
  baseline.
- **Bands:** `116` green, `10` yellow, `0` orange, `0` red.
- **Under-99 focus set:** WALKSTUF1 low, BUILDING2 high, WALKSTUF1 high,
  VISITOR3 high/low, BUILDING2 low, JOHNNY1 high/low, BUILDING4 low, and
  VISITOR5 low.

## Verification

- Focused gate:
  `scratch/ps1-perf-iterate/visitor5-high-rg30-46-v496-focused/20260513-072408-2952191/summary.json`.
- Broad canaries:
  `scratch/ps1-perf-iterate/visitor5-high-rg30-46-v496-canaries/20260513-072519-2959484/summary.json`.
- Visual DuckStation run was watched on VISITOR5 high and accepted.
- `./scripts/build-ps1.sh` passed before the promotion commit.
- `./scripts/build-ps1.sh clean` and `./scripts/make-cd-image.sh` passed for
  the release image.
