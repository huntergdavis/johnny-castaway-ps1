# v0.8.11-ps1 Release Notes

**Date:** 2026-05-14
**Tag:** `v0.8.11-ps1`
**Theme:** Lazy stream-buffer release-regression fix

`v0.8.11-ps1` is a corrective point release after `v0.8.10-ps1`. It keeps the
WALKSTUF1 low no-shift payload baseline from v791, but rolls back the
post-release heap-fragmentation experiment that pinned too much memory before
large clean-rect scenes could allocate their snapshots.

## Headline

- **WALKSTUF1 low renders and measures correctly again after the release
  merge.** The merged heap-fragmentation experiment pinned a `256 KB` CD sector
  pool plus boot-time FG stream buffers. The first W1-low post-release check
  skipped after `JCSKIP clean-rect-alloc-failed`, reporting only one entry and
  unusable `loop_vb=0` metrics.
- **Lazy CD/FG stream allocation is restored.** CD sector staging is back to
  per-read `malloc/free`, FG frame/scratch buffers stay lazy at boot, and the
  prefetch pre-prime branch is removed.
- **The accepted focused gate is exact-flat to v791.** W1-low returns to
  scene/loop/target `1770/1478/1431`, overrun `47`, blocking/refill `64/20`,
  loop reads/read VBlanks `60/272`, and due misses `11`.

## Current Battle Card

- **Public rollup:** `+0.2708%` over target / `99.7337%` target speed.
- **Raw signed rollup:** `-0.4963%` over target / `100.5160%` target speed.
- **Methodology total:** about `17.13` public over-target points removed and
  `+12.63` public target-speed points gained since the compact full-matrix
  baseline.
- **Bands:** `117` green, `9` yellow, `0` orange, `0` red.

## Verification

- Accepted rollback gate:
  `scratch/ps1-perf-iterate/walkstuf1-low-restore-lazy-cd-buffers-v793/20260514-152341-1753206/summary.json`.
- Failed post-release baseline checks:
  `scratch/ps1-perf-iterate/walkstuf1-low-frame77-inplace-v792-workreduction/20260514-151829-1722776/summary.json`
  and
  `scratch/ps1-perf-iterate/walkstuf1-low-v791-postrelease-baseline-check/20260514-151958-1731631/summary.json`.
- Partial rollback check:
  `scratch/ps1-perf-iterate/walkstuf1-low-lazy-streams-postrelease-fix/20260514-152141-1741701/summary.json`
  proved boot-lazy FG streams fixed rendering but still regressed timing until
  the static CD sector pool was also removed.
- `./scripts/build-ps1.sh`, `./scripts/site-build-static-root.sh`, and
  `git diff --check` passed before release.

