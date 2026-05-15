# v0.8.15-ps1 Release Notes

**Date:** 2026-05-15
**Tag:** `v0.8.15-ps1`
**Theme:** WALKSTUF1 high setup-resident CD promotion

`v0.8.15-ps1` is a performance point release collecting the WALKSTUF1 high
setup-segment promotion. It keeps all 63 scenes validated, keeps all 126
scene/tide rows routed and timing-bearing, and reduces the worst W1-high CD
pressure without changing the foreground pack footprint.

## Headline

- **WALKSTUF1 high advances to `walkstuf1-high-setupseg242-388-v998`.**
  High tide now primes relative sectors `242..388` during setup.
- **The active loop improves.** Against the v949 current control, W1-high moves
  from `1481/1428` to `1476/1441`, overrun `53 -> 35`, blocking/refill
  `88/24 -> 49/17`, reads/read time `67/287 -> 37/182`, and due misses
  `15 -> 7`.
- **The setup trade is explicit.** Scene/setup cost rises `1770 -> 1822`, and
  setup bytes rise `413236 -> 712244`, accepted under the material setup
  regression gate.
- **The remaining under-99 set is unchanged but tighter.** The yellow band
  remains seven rows, with WALKSTUF1 high now at `97.629%` target speed.

## Current Battle Card

- **Public rollup:** `+0.2285%` over target / `99.7746%` target speed.
- **Raw signed rollup:** `-0.5387%` over target / `100.5570%` target speed.
- **Methodology total:** about `17.17` public over-target points removed and
  `+12.67` public target-speed points gained since the compact full-matrix
  baseline.
- **Bands:** `119` green, `7` yellow, `0` orange, `0` red.

## Verification

- Accepted material proof:
  `scratch/ps1-perf-iterate/walkstuf1-high-setupseg242-388-v998-material/20260515-141421-1292648/summary.json`.
- Boundary probes: `228..388` and split `252..388 + 228..252` crossed the
  structural cliff; `252..388`, `244..388`, `240..388`, and `241..388` were
  safe but slower or worse-speed boundaries.
- `./scripts/build-ps1.sh`, `./scripts/site-build-static-root.sh`, and
  `git diff --check` are rerun before release prep. The release script rebuilds
  the PS1 executable, CD image, and static website again.
