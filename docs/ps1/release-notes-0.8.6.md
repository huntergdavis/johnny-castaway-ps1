# v0.8.6-ps1 Release Notes

**Date:** 2026-05-10
**Tag:** `v0.8.6-ps1`
**Theme:** WALKSTUF1 / VISITOR3 setup-segment compaction follow-through

`v0.8.6-ps1` is a performance point release after `v0.8.5-ps1`. It carries
the public 126-row headless matrix baseline forward with the WALKSTUF1 low
prefix gap-compression + slack-guard promotion, the WALKSTUF1 high
window-prefetch guard on the gap-compressed pack, and the VISITOR3 high/low
setup-segment resident-copy promotions for frames `131` and `128`.

## Headline

- **126 / 126 scene/tide rows remain timing-bearing.** No row regressed.
- **Public performance.** The public-capped rollup is `+0.3157%` over target
  / `99.6902%` target speed, down from v0.8.5's `+0.3215%` / `99.6847%`.
- **Raw optimization headroom.** The uncapped signed rollup is `-0.4529%`
  over target / `100.4740%` target speed, versus v0.8.5's `-0.4470%` /
  `100.4685%`.
- **Bands unchanged at the band edges.** `111` green, `15` orange, `0`
  yellow, `0` red — same shape as v0.8.5; the headroom showed up inside the
  green band rather than promoting another row across the `99%` line.
- **Methodology delta vs v0.8.5.** About `0.0058` public over-target points
  removed and `0.0055` public target-speed points gained.

## What moved

- **WALKSTUF1 low** — gap6-prefix + slack-guard promotion
  (`walkstuf1-low-prefix-gap6-slackguard-v305`).
- **WALKSTUF1 high** — gap1 / window-prefetch / slack4 guard on the
  gap-compressed pack (`walkstuf1-high-gap1-windowprefetch-slack4-v288`).
- **VISITOR3 high** — frame `131` setup-segment resident alias of frames
  `121`/`123` (`visitor3-high-f131-resident-alias121123-v299`); the high
  setup-segment now also keeps a frame-`140` tail copy
  (`visitor3-high-f140-segment-copy-v291`).
- **VISITOR3 low** — frame `128` setup-segment resident copy
  (`visitor3-low-f128-resident-seg27-v302`) on top of the v292 noop-alias
  baseline.

## Current Battle Card

- **Public rollup:** `+0.3157%` over target / `99.6902%` target speed.
- **Raw signed rollup:** `-0.4529%` over target / `100.4740%` target speed.
- **Bands:** `111` green, `15` orange, `0` yellow, `0` red.
- **Under-99 focus set:** WALKSTUF1 low/high, VISITOR3 high/low, BUILDING2
  high/low, VISITOR5 high/low, JOHNNY1 high/low, BUILDING4 low, BUILDING6
  high/low, and JOHNNY6 high/low. (Same scenes as v0.8.5; the orange band
  membership did not change in this release.)
- **VISITOR3 baseline:** high `1070/1039` with `49` blocking VBlanks, `9`
  loop reads, and `9` due misses after the v299 frame-131 resident-alias
  promotion; low `1071/1039` with `63` blocking VBlanks, `11` loop reads,
  and `11` due misses after the v302 frame-128 resident copy.
- **WALKSTUF1 baseline:** low `1478/1428` (`96.6%` target speed) after the
  v305 gap6-prefix plus slack-guard promotion; high `1477/1431` (`96.9%`
  target speed) after the v288 window-prefetch / slack4 guard.

## Verification

- Static website/docs regenerated from the release metadata and source docs.
- Scene validation scope remains unchanged: 63 / 63 scenes are signed off
  under the visual + audible bar.
- Performance CSV and battle-card docs use the current 126 / 126
  timing-bearing matrix as the release baseline. No row changed status
  band; `loop_vb` improvements are within the green band.
