# v0.8.3-ps1 Release Notes

**Date:** 2026-05-08
**Tag:** `v0.8.3-ps1`
**Theme:** WALKSTUF1 compact foreground performance plus VISITOR3 motion payloads

`v0.8.3-ps1` is a performance point release after `v0.8.2-ps1`. All 63
scenes remain validated, all 126 high/low scene variants remain routed through
the headless perf matrix, and all 126 timing-bearing rows now average
`+0.3697%` public over target / `99.6402%` public target speed after the MARY3,
BUILDING1, VISITOR5 high, BUILDING2 low, WALKSTUF3 high, BUILDING6 compact,
ACTIVITY9 high compact, WALKSTUF3 low compact, JOHNNY1 compact, ACTIVITY9 low
compact, and VISITOR3 motion-copy/code-headroom/CD-pressure follow-ups through
v237. The raw signed optimization matrix is `-0.3988%` over target /
`100.4240%` target speed.

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
  methodology has removed about `17.03` public over-target points and added
  about `12.54` public target-speed points.
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
- **VISITOR3 motion-copy, setup-segment, and resident payloads promoted.** The latest follow-up keeps the
  v181 frames `119..123` motion-copy payloads in both tides, adds high-tide
  frame `115`, shared frame `124`, shared frame `118`, and high-only frame
  `117` plus high-only re-anchored frames `127`, `126`, and `125`, then adds high/low
  persistent setup segments for sectors `277..293` and `281..305`, the high
  `320 KiB` setup-prime cap, the guarded low second segment `150..174`, and
  the low frame-125/frame-126 resident re-anchor plus the low frame-118
  resident-copy payload plus the low frame-127 resident-copy payload.
  Both packs
  remain `1555450` bytes with fixed LBAs `22472/23232` and the `215040` byte
  PS-EXE bucket. High moves
  `1118/1028 -> 1089/1035`, cuts blocking `150 -> 83`, loop reads `27 -> 15`,
  and due misses `26 -> 15`; low moves `1126/1025 -> 1088/1035`, cuts blocking
  `170 -> 95`, loop reads `31 -> 17`, and due misses `29 -> 16`.

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

The follow-up `visitor3-high-reanchor-f126-v206` promotes another high-only
re-anchor payload after v205. Frame `126`/source `172` widens from
`x=185,w=162` to `x=154,w=193` and becomes a sparse-in-place target-hull
motion-copy payload. It keeps high visible timing flat at `1414`, `1101/1030`
with `overrun_vb=71`, while cutting active payload `527206 -> 511690`,
blocking `108 -> 107`, and loop-read time `108 -> 107`. Low and the broad
canaries remain flat.

The follow-up `visitor3-high-reanchor-f125-v207` promotes the next high-only
re-anchor payload after v206. Frame `125`/source `171` widens from
`x=161,w=162` to `x=154,w=169` and becomes a sparse-in-place target-hull
motion-copy payload. It keeps high visible timing flat at `1414`, `1101/1030`
with `overrun_vb=71`, while cutting active payload `511690 -> 496661`,
blocking `107 -> 106`, and loop-read time `107 -> 106`. Low and the broad
controls remain flat; the broad gate was split because an inconclusive harness
`137` killed JOHNNY1 before metrics, then the tail rerun passed the remaining
JOHNNY1/ACTIVITY9/FISHING1 controls.

The follow-up `visitor3-high-setup-prime288-v213` expands only the high-tide
setup-prime cap from `232 KiB` to `288 KiB`. It trades high setup time
(`scene_vb 1414 -> 1423`) for active-loop timing relief:
`1101/1030 -> 1099/1032`, overrun `71 -> 67`, blocking `106 -> 96`,
loop reads `20 -> 17`, loop-read time `106 -> 96`, and due misses `20 -> 17`.
Low cap probes at `288/256/240 KiB` were rejected because they added hidden
refill or visible cadence debt, so low stayed on v204 until the guarded v216
second setup segment moved it to `1098/1034`.

The follow-up `visitor3-high-setup-prime320-v214` expands only the high-tide
setup-prime cap from `288 KiB` to `320 KiB`. It trades high setup time for
active-loop timing relief: `1099/1032 -> 1089/1035`, overrun `67 -> 54`,
blocking `96 -> 83`, loop reads `17 -> 15`, loop-read time `96 -> 83`, and due
misses `17 -> 15`.

The follow-up `visitor3-low-dual-segment150-174-slack4-v216` adds a guarded
second low-tide setup segment for sectors `150..174` and raises only VISITOR3
low refill eligibility to `4` VBlanks. It accepts low setup time
(`scene_vb 1415 -> 1424`) because the active loop improves `1102/1032 ->
1098/1034`, overrun drops `70 -> 64`, blocking drops `124 -> 112`, loop reads
drop `23 -> 20`, loop-read time drops `131 -> 119`, due misses drop `22 -> 19`,
and hidden refill remains `0`.

The follow-up `visitor3-low-f125126-resident-v227` promotes the low-side
frame-125/frame-126 resident re-anchor inside the accepted `150..174` setup
segment with no runtime-source change. Frame `125` drops `16828 -> 1799`
bytes, frame `126` drops `16828 -> 1312` bytes, active low payload drops by
`30545` bytes, and the pack footprint, LBA, sound offset, and `215040` byte
PS-EXE bucket stay fixed. The active loop improves `1098/1034 -> 1095/1035`,
overrun drops `64 -> 60`, blocking drops `112 -> 108`, loop reads drop
`20 -> 19`, loop-read time drops `119 -> 115`, due misses drop `19 -> 18`, and
hidden refill remains `0`.

The follow-up `visitor3-low-f118-resident-copy-v234` copies the existing
low-side frame-118 payload unchanged into the same accepted `150..174` setup
segment. This keeps cadence/render semantics intact while converting one more
active-loop read into resident data. The active loop improves
`1095/1035 -> 1091/1035`, overrun drops `60 -> 56`, blocking drops
`108 -> 103`, loop reads drop `19 -> 18`, loop-read time drops `115 -> 110`,
due misses drop `18 -> 17`, and hidden refill remains `0`.

The follow-up `visitor3-low-f127-resident-copy-v237` then compacts that same
resident segment by moving frames `125/126` earlier unchanged and copying the
existing frame-127 payload unchanged into the freed tail. That converts another
active-loop read without changing frame geometry or pack layout: the active
loop improves `1091/1035 -> 1088/1035`, overrun drops `56 -> 53`, blocking
drops `103 -> 95`, loop reads drop `18 -> 17`, loop-read time drops
`110 -> 102`, due misses drop `17 -> 16`, and hidden refill remains `0`.

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
- VISITOR3 high frame-126 re-anchor broad no-regression gate:
  `scratch/ps1-perf-iterate/visitor3-high-reanchor-f126-v206-broad-norequire/20260509-071828-1639276/summary.json`.
- VISITOR3 high frame-125 focused gate:
  `scratch/ps1-perf-iterate/visitor3-high-reanchor-f125-v207-focused/20260509-075102-1822459/summary.json`.
- VISITOR3 high frame-125 split broad controls:
  `scratch/ps1-perf-iterate/visitor3-high-reanchor-f125-v207-broad-norequire/20260509-075210-1829320/`
  plus `scratch/ps1-perf-iterate/visitor3-high-reanchor-f125-v207-tail-rerun/20260509-075859-1871481/summary.json`.
- VISITOR3 high setup-prime v213 broad no-regression gate:
  `scratch/ps1-perf-iterate/visitor3-high288-low208-v213-broad/20260509-093115-2397654/summary.json`.
- VISITOR3 low resident re-anchor focused gate:
  `scratch/ps1-perf-iterate/visitor3-low-f125126-resident-v227-focused/20260509-122159-3376401/summary.json`.
- VISITOR3 low resident re-anchor broad gate:
  `scratch/ps1-perf-iterate/visitor3-low-f125126-resident-v227-broad/20260509-122307-3383321/summary.json`.
- VISITOR3 low resident re-anchor current-control drift check:
  `scratch/ps1-perf-iterate/visitor3-low-f125126-resident-v227-current-control/20260509-123359-3447654/summary.json`.
- VISITOR3 low frame-118 resident-copy focused gate:
  `scratch/ps1-perf-iterate/visitor3-low-f118-resident-copy-v234-focused/20260509-131645-3689098/summary.json`.
- VISITOR3 low frame-118 resident-copy broad/control gate:
  `scratch/ps1-perf-iterate/visitor3-low-f118-resident-copy-v234-broad/20260509-131940-3705538/summary.json`.
- VISITOR3 low frame-127 resident-copy focused gate:
  `scratch/ps1-perf-iterate/visitor3-low-f127-resident-copy-v237-focused/20260509-140328-3954717/summary.json`.
- VISITOR3 low frame-127 resident-copy broad/control gate:
  `scratch/ps1-perf-iterate/visitor3-low-f127-resident-copy-v237-broad/20260509-140440-3961993/summary.json`.
- VISITOR3 low frame-127 resident-copy current-control drift check:
  `scratch/ps1-perf-iterate/visitor3-low-f127-resident-copy-v237-current-control/20260509-141713-4034325/summary.json`.
- A visible DuckStation run was manually checked before the release merge-down
  and looked good.

The release remains a performance point release. No scene validation scope
changed: 63 / 63 scenes remain signed off.
