# v0.8.3-ps1 Release Notes

**Date:** 2026-05-08
**Tag:** `v0.8.3-ps1`
**Theme:** WALKSTUF1 compact foreground performance plus VISITOR3 motion payloads

`v0.8.3-ps1` is a performance point release after `v0.8.2-ps1`. All 63
scenes remain validated, all 126 high/low scene variants remain routed through
the headless perf matrix, and all 126 timing-bearing rows now average
`+0.3962%` public over target / `99.6166%` public target speed after the MARY3,
BUILDING1, VISITOR5 high, BUILDING2 low, WALKSTUF3 high, BUILDING6 compact,
ACTIVITY9 high compact, WALKSTUF3 low compact, JOHNNY1 compact, ACTIVITY9 low
compact, and VISITOR3 motion-copy/code-headroom/CD-pressure follow-ups through
v205. The raw signed optimization matrix is `-0.3723%` over target /
`100.4004%` target speed.

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
  methodology has removed about `17.00` public over-target points and added
  about `12.52` public target-speed points.
- **MARY3 is now green.** The follow-up guarded prefetch-preserve pass moves
  MARY3 high/low to `2296/2294` and `2297/2295`, cuts blocking
  `690/693 -> 53/51`, and keeps `prefetch_overrun_vb=0`.
- **BUILDING6 moved to the bottom of orange.** The compact-FGP3/v4 follow-up
  keeps both `1444370` byte footprints and fixed LBAs while moving high
  `2520/2442 -> 2482/2457`, low `2515/2437 -> 2485/2456`, and due misses
  `1/2 -> 0/0`.
- **WALKSTUF3 low is now green.** The latest compact-FGP3/v4 follow-up keeps the
  `994669` byte footprint and LBA fixed while moving low `2321/2293 ->
  2310/2295`, cutting blocking `41 -> 26`, loop reads `72 -> 29`, and due
  misses `5 -> 2`.
- **JOHNNY1 read pressure is lower.** The latest compact-FGP3/v4 follow-up keeps
  both `448370` byte footprints and fixed LBAs while moving high/low
  `1977/1943 -> 1974/1945`, cutting blocking `31 -> 26`, loop reads `16 -> 7`,
  and loop-read time `95 -> 56`.
- **ACTIVITY9 low is now green.** The latest compact-FGP3/v4 follow-up keeps the
  `1745484` byte footprint and LBA fixed while moving low `2085/2058 ->
  2075/2061`, cutting blocking `29 -> 17`, loop reads `59 -> 47`, loop-read
  time `289 -> 232`, and due misses `3 -> 1`.
- **VISITOR3 motion-copy and setup-segment payloads promoted.** The latest follow-up keeps the
  v181 frames `119..123` motion-copy payloads in both tides, adds high-tide
  frame `115`, shared frame `124`, shared frame `118`, and high-only frame
  `117` plus high-only re-anchored frame `127` motion-copy payloads, then adds
  high/low persistent setup segments for sectors `277..293` and `281..305`.
  Both packs
  remain `1555450` bytes with fixed LBAs `22472/23232` and the `215040` byte
  PS-EXE bucket. High moves
  `1118/1028 -> 1101/1030`, cuts blocking `150 -> 108`, loop reads `27 -> 20`,
  and due misses `26 -> 20`; low moves `1126/1025 -> 1102/1032`, cuts blocking
  `170 -> 124`, loop reads `31 -> 23`, and due misses `29 -> 22`.

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

The follow-up `visitor3-motion-x-v181` did promote. It keeps the same VISITOR3
CD footprint and executable sector bucket, but makes the late motion cluster
intrinsically cheaper to stream and compose by moving already-composited
background pixels in place before applying the small residual foreground
payload. The focused promotion gate and broad no-regression gate keep
non-VISITOR controls on their accepted profiles; an unrelated WALKSTUF1 high
drift was reproduced on the pre-change HEAD control and is not attributed to
this payload format.

The follow-up `visitor3-high-f115-motion-x-v182` also promoted. It extends the
motion-copy shape to high-tide frame `115` only, improving high from
`1105/1031` to `1104/1030`, cutting blocking `129 -> 128`, and cutting
loop-read time `132 -> 130`. Low remains on the v181 row after larger both-tide
and high-only windows regressed held slack or hidden refill.

The follow-up `visitor3-f124-sparse-motion-v188` promoted as an
offset-preserving work-reduction pass: frame `124` in both tides became a
sparse-in-place state-hull motion-copy payload, cutting another `14592` active
bytes per tide and reducing blocking to `126/142` without changing loop timing.

The follow-up `visitor3-f118-hull-sparse-v189` then promoted the same
offset-preserving lane for frame `118`. It keeps fixed LBAs and the `215040`
byte PS-EXE bucket, trims another `13901` active bytes per tide, nudges high to
`1104/1031`, keeps low at `1108/1028`, and cuts blocking to `120/139`.

The follow-up `visitor3-high-f117-target-hull-v193` promotes the high-only
subset of frame `117` after the both-tide and low-only sparse-hull variants
failed. It uses a target-hull payload that preserves cleanup geometry, trims
another `11149` high active bytes, improves high to `1101/1030`, cuts high
blocking to `116`, cuts high loop-read time to `118`, and leaves low exact-flat
on the v189 pack.

The follow-up `visitor3-low-segment281-305-v204` promotes a scheduler-owned
low-tide setup segment for sectors `281..305`. It accepts setup cost
(`scene_vb 1408 -> 1415`) because the active loop improves `1108/1028 ->
1102/1032`, overrun drops `80 -> 70`, blocking drops `139 -> 124`, loop reads
drop `27 -> 23`, loop-read time drops `148 -> 131`, and due misses drop
`25 -> 22`.

The follow-up `visitor3-high-segment277-293-v205` promotes the matching
high-tide setup segment for sectors `277..293`. It keeps the active loop flat
at `1101/1030` while cutting blocking `114 -> 108`, loop reads `22 -> 20`,
loop-read time `116 -> 108`, and due misses `21 -> 20`; setup cost raises high
total `scene_vb 1405 -> 1414`. Wider high segments `261..285` and `269..285`
cut more active CD work but were rejected because both introduced hidden refill
debt (`prefetch_overrun_vb 0 -> 3`).

## Verification

- Focused WALKSTUF1 high gate:
  `scratch/ps1-perf-iterate/walkstuf1-compact-fgp3-v141-high/20260508-063526-1596231/summary.json`.
- Focused WALKSTUF1 low gate:
  `scratch/ps1-perf-iterate/walkstuf1-compact-fgp3-v141-low/20260508-063632-1602886/summary.json`.
- Broad canary gate:
  `scratch/ps1-perf-iterate/walkstuf1-compact-fgp3-v141-broad/20260508-063809-1612125/summary.json`.
- VISITOR3 fallthrough non-promotion:
  `scratch/ps1-perf-iterate/visitor3-fallthrough5-v142-focused/20260508-074006-1958740/summary.json`.
- VISITOR3 motion-copy promotion:
  `scratch/ps1-perf-iterate/visitor3-motion-x-v181-focused/20260508-202531-2127593/summary.json`.
- VISITOR3 motion-copy broad no-regression gate:
  `scratch/ps1-perf-iterate/visitor3-motion-x-v181-broad-norequire/20260508-203050-2158325/summary.json`.
- VISITOR3 high frame-115 broad no-regression gate:
  `scratch/ps1-perf-iterate/visitor3-motion-x-v182-high-f115-broad/20260508-213318-2508409/summary.json`.
- VISITOR3 high frame-117 target-hull broad no-regression gate:
  `scratch/ps1-perf-iterate/visitor3-high-f117-target-hull-v193-broad/20260509-014222-3940374/summary.json`.
- VISITOR3 high setup-segment broad no-regression gate:
  `scratch/ps1-perf-iterate/visitor3-high-segment277-293-v205-broad-norequire-rerun/20260509-061624-1296310/summary.json`.
- A visible DuckStation run was manually checked before the release merge-down
  and looked good.

The release remains a performance point release. No scene validation scope
changed: 63 / 63 scenes remain signed off.
