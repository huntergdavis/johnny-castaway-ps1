# v0.8.16-ps1 Release Notes

**Date:** 2026-05-17
**Tag:** `v0.8.16-ps1`
**Theme:** memory-region allocator stability release

`v0.8.16-ps1` promotes the memory-region allocator work from `origin/main`.
This is a stability point release, not a speed-claim release: it attacks the
long-run heap-fragmentation failure mode by routing allocations through explicit
BOOT, CACHE, and TRANSIENT regions.

## Headline

- **Memory allocations now carry lifetime intent.** BOOT data seals after
  startup, CACHE data is reusable long-lived resource storage, and TRANSIENT
  scene data can be wiped between major scene loads.
- **The allocator has release gates.** Every `memAlloc` call site is required to
  carry a nearby `MEM_REGION_RATIONALE`, the documented region count is checked,
  synthetic halt bootmodes stay off by default, and generated pack metrics are
  freshness-checked.
- **Pack-header metrics are generated.** `scripts/generate-pack-metrics.py`
  writes `src/generated/pack_header_metrics.{c,h}` so foreground/CD decisions
  can use current pack shape without hand-maintained constants.
- **The full matrix survives the allocator merge.** The R34 full matrix in the
  allocator branch records `126/126` scene/tide rows passing with 0 BSODs.

## Current Battle Card

- **Public rollup:** `+0.5699%` over target / `99.4843%` target speed.
- **Raw signed rollup:** `-0.1470%` over target / `100.2147%` target speed.
- **Methodology total:** about `16.83` public over-target points removed and
  `+12.38` public target-speed points gained since the compact full-matrix
  baseline.
- **Bands:** `118` green, `4` yellow, `2` orange, `2` red.
- **Under-99 focus:** VISITOR3 high/low, WALKSTUF1 high/low, BUILDING2 high/low,
  and BUILDING4 high/low remain the active performance queue after the allocator
  baseline refresh.

## Verification

- `./scripts/check-mem-region-gates.sh` passes. Gate 3 still reports informational
  residual-string warnings for known comments/fallback text, but the script
  exits cleanly.
- `./scripts/build-ps1.sh` rebuilds `build-ps1/jcreborn.exe` successfully.
- `python3 scripts/check-mem-region-rationale.py` reports all `memAlloc` sites
  annotated.
- The release script rebuilds the PS1 executable, CD image, and static website
  again before tagging.
