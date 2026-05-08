# v0.8.3-ps1 Release Notes

**Date:** 2026-05-08
**Tag:** `v0.8.3-ps1`
**Theme:** WALKSTUF1 compact foreground performance

`v0.8.3-ps1` is a performance point release after `v0.8.2-ps1`. All 63
scenes remain validated, all 126 high/low scene variants remain routed through
the headless perf matrix, and all 126 timing-bearing rows now average
`+0.4527%` public over target / `99.5662%` public target speed after the MARY3,
BUILDING1, VISITOR5 high, BUILDING2 low, WALKSTUF3 high, and BUILDING6 compact
follow-ups. The raw signed optimization matrix is `-0.3158%` over target /
`100.3500%` target speed.

## Headline

- **WALKSTUF1 compact packs promoted.** Both WALKSTUF1 tides now use padded
  compact FGP3/v4 restore-minus-current packs instead of the older PAL4/FGP2
  packs.
- **CD layout stays stable.** The high/low packs remain `1535263` bytes, LBAs
  stay `24744/25494`, and the PS-EXE remains in the `215040` byte bucket.
- **Visible loop pressure drops.** High improves `loop_vb 1592 -> 1491`,
  `target_vb 1406 -> 1426`, `overrun_vb 186 -> 65`, `blocking_vb 275 -> 85`,
  `loop_reads 134 -> 69`, and `loop_read_vb 586 -> 300`.
- **Low tide improves in parallel.** Low improves `1604 -> 1489`, `1407 ->
  1427`, `197 -> 62`, `270 -> 86`, `132 -> 69`, and `604 -> 305`.
- **Total methodology gain increased.** Since the compact full-matrix baseline
  was about `17.4%` over target / `87.1%` target speed, the headless
  methodology has removed about `16.95` public over-target points and added
  about `12.47` public target-speed points.
- **MARY3 is now green.** The follow-up guarded prefetch-preserve pass moves
  MARY3 high/low to `2296/2294` and `2297/2295`, cuts blocking
  `690/693 -> 53/51`, and keeps `prefetch_overrun_vb=0`.
- **BUILDING6 moved to the bottom of orange.** The compact-FGP3/v4 follow-up
  keeps both `1444370` byte footprints and fixed LBAs while moving high
  `2520/2442 -> 2482/2457`, low `2515/2437 -> 2485/2456`, and due misses
  `1/2 -> 0/0`.

## Follow-Up Closure

The next VISITOR3 candidate, `visitor3-fallthrough5-v142`, did not promote.
Lowering `FG_PREFETCH_FALLTHROUGH_MIN_SLACK_VBLANKS` from `6` to `5` kept both
tides exact-flat against a fresh current control:

- High stayed `scene_vb=1422`, `loop_vb=1118`, `target_vb=1028`,
  `overrun_vb=90`, `blocking_vb=150`, `prefetch_overrun_vb=0`, and
  `loop_reads=27`.
- Low stayed `scene_vb=1426`, `loop_vb=1126`, `target_vb=1025`,
  `overrun_vb=101`, `blocking_vb=170`, `prefetch_overrun_vb=0`, and
  `loop_reads=31`.

The runtime keeps the accepted `6` VBlank guard. Remaining VISITOR3 work stays
in generated scheduler ownership, safe upload/precomposed payloads, or another
pack/data-shape reduction.

## Verification

- Focused WALKSTUF1 high gate:
  `scratch/ps1-perf-iterate/walkstuf1-compact-fgp3-v141-high/20260508-063526-1596231/summary.json`.
- Focused WALKSTUF1 low gate:
  `scratch/ps1-perf-iterate/walkstuf1-compact-fgp3-v141-low/20260508-063632-1602886/summary.json`.
- Broad canary gate:
  `scratch/ps1-perf-iterate/walkstuf1-compact-fgp3-v141-broad/20260508-063809-1612125/summary.json`.
- VISITOR3 fallthrough non-promotion:
  `scratch/ps1-perf-iterate/visitor3-fallthrough5-v142-focused/20260508-074006-1958740/summary.json`.
- A visible DuckStation run was manually checked before the release merge-down
  and looked good.

The release remains a performance point release. No scene validation scope
changed: 63 / 63 scenes remain signed off.
