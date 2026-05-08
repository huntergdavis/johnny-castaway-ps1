# PS1 Performance Next 100

Date: 2026-05-08

Current accepted fishing1 high-tide canary baseline:

| Metric | Value |
|---|---:|
| `loop_vb` | `1068` |
| `target_vb` | `1074` |
| `remaining_overrun_vb` | `0` |
| `remaining_over_target` | `0.0% public` (`-0.56%` raw signed) |
| `blocking_vb` | `2` |
| `prefetch_overrun_vb` | `2` |
| `loop_reads` | `20` |
| `upload_bytes` | `10638080` |
| `restore_bytes` | `251144` |
| `prefetch_buffer` | `137048` bytes for current fishing1 high-tide FGP3 playback |
| `jcreborn.exe` | `215040` bytes |
| `jcreborn.elf` | `951708` bytes |

Goal: keep the FISHING1 canary at the public 100% cap or better while reducing the remaining
matrix-wide gaps without changing pixels, sound event timing, scene identity,
or long-run heap stability. The previous MARY2 checkpoint was `0.8228%` over
target / `99.4872%` target speed across `120` timing-bearing rows after the
`mary2-prefetch-relief-v081` refresh.

Current all-scene rollup after the WALKSTUF1 compact FGP3/v4 pass:
`+0.5576%` public average over target / `99.4669%` public target speed across
`120` timing-bearing rows. The raw signed optimization matrix remains
`-0.2497%` / `100.2899%`. Since the compact full-matrix baseline was about
`17.4%` over target / `87.1%` target speed, the headless methodology has
removed about `16.84` public over-target points and added about `12.37`
public target-speed points.

Latest promoted WALKSTUF1 compact FGP3/v4 baseline: convert both WALKSTUF1
PAL4/FGP2 packs to padded compact FGP3/v4 restore-minus-current packs inside
the original `1535263` byte footprints. The compaction chain trims active
candidate bytes to `923959` while preserving pack LBAs `24744/25494` and the
`215040` byte PS-EXE bucket. High improves `loop_vb 1592 -> 1491`,
`target_vb 1406 -> 1426`, `overrun_vb 186 -> 65`, `blocking_vb 275 -> 85`,
`prefetch_overrun_vb 51 -> 32`, `loop_reads 134 -> 69`, `loop_read_vb
586 -> 300`, and `due_misses 55 -> 13`. Low improves `1604 -> 1489`,
`1407 -> 1427`, `197 -> 62`, `270 -> 86`, `55 -> 27`, `132 -> 69`,
`604 -> 305`, and `50 -> 12`. The broad non-WALKSTUF controls stayed
exact-flat except VISITOR3 high, and that VISITOR3 drift reproduced with the
original WALKSTUF1 FGP2 packs restored, so it is not attributed to the compact
candidate. The next top rows are VISITOR3 low/high, BUILDING2 low, WALKSTUF1
high/low, and BUILDING6 high/low.

Latest promoted VISITOR3 tail-trim stageguard baseline: trim all-zero draw
tails inside the existing VISITOR3 FGP3/v4 payloads, raise the high-tide
setup-prime resident budget to `232 KiB`, and skip VISITOR3 hidden large-stage
reads when held slack is below the fallthrough minimum. The pass preserves both
`1555450` byte pack footprints, foreground LBAs, and the `215040` byte PS-EXE
bucket. VISITOR3 high improves `loop_vb 1137 -> 1118`, `target_vb 1024 ->
1028`, `overrun_vb 113 -> 90`, `blocking_vb 190 -> 150`, `loop_reads 33 ->
27`, and `loop_read_vb 200 -> 153`; VISITOR3 low improves `1135 -> 1126`,
`1024 -> 1025`, `111 -> 101`, `184 -> 170`, `33 -> 31`, and `194 -> 179`.
Both rows keep `prefetch_overrun_vb=0`, and BUILDING2 high/low, BUILDING4
high/low, ACTIVITY9 low, and FISHING1 high stayed exact-flat.

Prior promoted VISITOR3 low code-shape baseline: compile the non-FGP3
`grCompositeToBackground()` and `grCompositeToBackgroundFlip()` helpers with
scoped `-Os`. The change shrinks `jcreborn.elf 960556 -> 951708` while keeping
the `215040` byte PS-EXE bucket, foreground pack LBAs, and the active FGP3
packed-span compositor fixed. VISITOR3 low improves `loop_vb 1138 -> 1135`,
`overrun_vb 114 -> 111`, `blocking_vb 191 -> 184`, and `loop_read_vb
200 -> 194`. VISITOR3 high, BUILDING2 high/low, BUILDING4 high/low, ACTIVITY9
low, and FISHING1 high stayed exact-flat.

Recent promoted BUILDING2 low read-group baseline: add a low-tide-only retained
stream group for file sectors `365..381`. The source-table change grows
`foregroundPilotPlay` by `8` bytes versus v109 but keeps the `215040` byte
PS-EXE bucket, foreground pack LBA `6817`, and all broad-control LBAs fixed.
BUILDING2 low improves `loop_vb 1385 -> 1383`, `target_vb 1303 -> 1304`,
`overrun_vb 82 -> 79`, `blocking_vb 121 -> 118`, `prefetch_overrun_vb 8 -> 5`,
`loop_reads 57 -> 55`, and `due_misses 23 -> 22`. VISITOR3 high/low, BUILDING2
high, BUILDING4 high/low, ACTIVITY9 low, and FISHING1 high stayed exact-flat.

Recent promoted BUILDING2 high read-group baseline: add a high-tide-only
retained stream group for file sectors `60..72`, after the v109 read matrix
ranked it as the only current `scheduler-or-guarded-probe` row with zero
overread, no partial touches, and medium visible gaps. The source-table change
grows `foregroundPilotPlay` by `12` bytes but keeps the `215040` byte PS-EXE
bucket, foreground pack LBA `6180`, and all broad-control LBAs fixed. BUILDING2
high improves `loop_vb 1353 -> 1349`, `target_vb 1311 -> 1316`,
`overrun_vb 42 -> 33`, `blocking_vb 56 -> 48`, `prefetch_overrun_vb 20 -> 12`,
and `loop_reads 62 -> 61`. VISITOR3 high/low, BUILDING2 low, BUILDING4
high/low, ACTIVITY9 low, and FISHING1 high stayed exact-flat.

Latest promoted BUILDING2 high restore-minus-current baseline: apply the
same cleanup-minus-current-draw pack transform only to high-tide
`BUILDING2.FG2`, preserving the `1303332` byte CD footprint, pack LBA `6180`,
and the `215040` byte PS-EXE bucket. High active payload drops
`789906 -> 674798`, modeled restore bytes drop `937272 -> 474572`, and the
gate improves high `loop_vb 1394 -> 1353`, `target_vb 1301 -> 1311`,
`overrun_vb 93 -> 42`, `blocking_vb 138 -> 56`, and `loop_reads 68 -> 62`.
Low is intentionally not transformed: the both-tide variant improved low
visible timing but regressed hidden prefetch overrun `8 -> 13`, so low stays
exact-flat at `1385/1303`, `blocking_vb=121`, `prefetch_overrun_vb=8`, and
`loop_reads=57`. That low row later moved to `1383/1304`, `blocking_vb=118`,
`prefetch_overrun_vb=5`, and `loop_reads=55` through the low-tide `365..381`
grouped-read pass above. VISITOR3 high/low, BUILDING4 high/low, ACTIVITY9 low,
and FISHING1 high controls stayed exact-flat.

Latest promoted VISITOR3 low exit-right offscreen draw-clip baseline: apply
the data-size-preserving offscreen draw trim only to low-tide entries
`139..143` in `VIST3LOW.FG2`. The pack remains `1555450` bytes, all entry
sizes and offsets are preserved, pack LBA stays `23232`, and the `215040` byte
PS-EXE bucket is unchanged. Low improves `loop_vb 1140 -> 1138`,
`overrun_vb 116 -> 114`, and `blocking_vb 194 -> 191`; high stays exact-flat
at `1137/1024`, and BUILDING2, BUILDING4, ACTIVITY9, and FISHING1 controls
stay exact-flat. The low `ship-left` subset (`113..117`) and combined
`ship-and-exit` subset are rejected because they reproduce the bad
`1151/1024` low-tide cadence; early-left is exact-flat and not promoted.

Latest promoted VISITOR3 high offscreen draw-clip baseline: apply the
data-size-preserving offscreen draw trim only to `VISITOR3.FG2` and leave
unproven low-side ranges untouched. The pack remains `1555450` bytes, all
entry sizes and offsets are preserved, pack LBA stays `22472`, and the `215040` byte
PS-EXE bucket is unchanged. High improves `loop_vb 1139 -> 1137`,
`overrun_vb 115 -> 113`, and `blocking_vb 191 -> 190`. This is now the
VISITOR3 high baseline.

Prior promoted WALKSTUF1 high setup-prime baseline: give high tide its own
`144 KiB` setup-prime resident cap instead of falling back to the global
foreground setup-prime cap. The retune increases high-tide setup coverage to
sectors `2..74` without moving the `1535263` byte FGP2/PAL4 pack, pack LBA
`24744`, or the `215040` byte PS-EXE bucket. High keeps full scene time flat
at `1880`, improves `loop_vb 1595 -> 1592`, `target_vb 1402 -> 1406`,
`overrun_vb 193 -> 186`, `blocking_vb 277 -> 275`, `loop_reads 136 -> 134`,
`loop_read_vb 602 -> 586`, and `due_misses 56 -> 55`. Low stays exact-flat at
`1895`, `1604/1407`, `blocking_vb=271`, `prefetch_overrun_vb=54`, and
`loop_reads=132`. VISITOR3 high/low, BUILDING2 high/low, BUILDING4 high/low,
ACTIVITY9 low, and FISHING1 high controls stayed exact-flat. This baseline is
now superseded by `walkstuf1-compact-fgp3-v141`.

Latest rejected WALKSTUF1 direct-stage threshold sweep: the current scalar
`8 KiB` direct-stage cap remains the measured knee. A `6 KiB` global cap fixed
layout and reduced active blocking, but moved too much work into hidden refill:
high regressed `1592 -> 1594` with `prefetch_overrun_vb 51 -> 75`, and low
regressed `1604 -> 1607` with `54 -> 81`. A `7 KiB` cap kept loop time and
hidden overrun flat, but only reduced blocking by `1/4` VBlanks and regressed
target-relative overrun by one VBlank on both tides. Close scalar direct-stage
caps for WALKSTUF1; retry only with generated scheduler ownership,
frame/range-specific direct-stage policy, or a pack/data-shape reduction that
lowers refill cost first.

Current WALKSTUF1 preprocess footprint gate: the default selective x-band
upload-ready model now has compact-pack zero-tail budget, but it remains unsafe
as a raw append. The old PAL4/FGP2 packs had only `1` byte of zero-tail slack;
the compact FGP3/v4 packs now expose `611305` zero-tail bytes per tide, and the
budgeted selective model fits `609192 / 611305` bytes while selecting `39`
frames, saving `1991904` modeled upload bytes from `2600320` selected runtime
bytes (`76.6%` saved), and retaining `59.45%` of the default selected savings.
Restore-minus-current cleanup is exhausted for this pack shape
(`restore_runtime_bytes == restore_minus_current_exact_bytes == 525826`).
Raw foreground-only upload payloads are still unsafe: selected draw-covered
x-band bytes and all-draw-covered selected frames are both `0`. Keep raw
same-footprint WALKSTUF1 upload-ready append work closed until there is a safe
background-owned/precomposed pixel source, ownership metadata, generated
scheduler support, or MoveImage-safe motion data; use the compact pack as the
new baseline for any future WALKSTUF1 preprocessing lane.

Current VISITOR3 preprocess safety gate: the same-footprint budgeted
upload-ready target remains a useful byte ceiling, but raw foreground-only
pack-emitted upload pixels are not safe under the current FGP3 data. The
analyzer now reports draw-covered x-band feasibility; both VISITOR3 tides show
`0` selected draw-covered bytes for the default `96`-frame selective plan and
the `74`-frame budgeted plan. The modeled `3858104` byte win depends on
restored background/cleanup pixels, which are dynamic at runtime. Do not build
that as a raw append; continue with a safe pixel-source/data-shape change,
compression plus ownership, or generated scheduler metadata.

Current VISITOR3 v140 upload/read-plan closure: after the v127 tail-trim
stageguard pass, the same analyzer finds more padded payload budget but the
safety constraint is unchanged. The default selective x-band model selects
`117` frames and needs `3394200` bytes for `10602536` modeled upload bytes
saved. The current same-footprint budgeted model selects high `75` frames
using `888880 / 891012` bytes for `6290232` saved bytes, and low `74` frames
using `853848 / 854114` bytes for `6166528` saved bytes, but both tides still
have `0` draw-covered selected x-band bytes. Runtime dirty-upload narrowing is
not a substitute: exact interval upload would create about `131996` loop rects,
and prior scratch-packed x-band probes already proved the copy/code-size cost
is worse than full-width tile bands. The refreshed v140 read-plan also found no
candidate that is append-start fireable, current-window-sized, and low-risk;
the fireable current-fit rows are the already-closed late tight clusters. Close
VISITOR3 runtime dirty-upload and hand read-table work until generated
scheduler ownership or safe background-owned/precomposed upload data exists.

Current VISITOR3 low setup-prime gate: the accepted `208 KiB` low-tide cap is
still the measured knee after the v127 tail-trim stageguard pass. Retesting
`216 KiB` preserved high tide but regressed low `1126 -> 1127` and blocking
`170 -> 173`; retesting `200 KiB` regressed low to `1152/1024`, blocking
`191`, and hidden refill `3`. Do not retry scalar low-prime sizes around this
point; VISITOR3 needs generated scheduler ownership or a pack/data-shape change
to reduce the remaining `150/170` visible blocking.

Current VISITOR3 fallthrough guard gate: lowering
`FG_PREFETCH_FALLTHROUGH_MIN_SLACK_VBLANKS` from `6` to `5` after the
WALKSTUF1 compact pass is rejected as exact-flat. The fresh control and
candidate both kept high at `1422`, `1118/1028`, `blocking_vb=150`,
`prefetch_overrun_vb=0`, and `loop_reads=27`; low stayed `1426`,
`1126/1025`, `blocking_vb=170`, `prefetch_overrun_vb=0`, and
`loop_reads=31`. Keep the guard at `6` and do not spend more VISITOR3 cycles
on local threshold-only fallthrough probes.

Current VISITOR3 retained-window slack gate: a VISITOR3-only `20 KiB`
compact-residual window with a `12` VBlank refill slack guard is rejected. It
kept layout and hidden prefetch overrun fixed, but moved more direct-stage work
into active visible CD: high regressed `loop_vb 1118 -> 1131`, overrun
`90 -> 102`, blocking `150 -> 210`, and reads `27 -> 39`; low regressed
`1126 -> 1139`, overrun `101 -> 106`, blocking `170 -> 212`, and reads
`31 -> 41`. Do not retry scalar VISITOR3 window/slack retunes; the next
VISITOR3 path needs generated scheduler ownership or pack/data-shape work.

Current BUILDING2 low restore-minus-current gate: the low-tide pack transform
is still too hidden-refill expensive under the current scheduler, even though
the visible signal is strong. A size-preserving `BUIL2LOW.FG2` cleanup-minus-
current candidate cuts active payload `789906 -> 674798` and modeled restore
bytes `937272 -> 474572`; low improves to `1346/1311`, overrun `35`, blocking
`50`, and `loop_reads=52`, with high tide flat. Strict promotion is blocked by
hidden `prefetch_overrun_vb 5 -> 13`. The temporary `144 KiB` setup-prime and
BUILDING2-low stage-guard salvages did not reduce that hidden debt. A later
BUILDING2-low window-refill slack guard did remove hidden refill (`5 -> 0`)
while keeping layout fixed, but it pushed too much work into visible cadence:
low moved `1383/1304 -> 1360/1313` with blocking `118 -> 180` and due misses
`22 -> 43`. Treat this as a generated scheduler/refill-ownership target, not a
pack-only or local slack-guard promotion.

The VISITOR3 no-op empty-hold recast is also closed under the current packs.
`scripts/compact-fgp3-zero-noop-entries.py` found `0` high-tide and `0`
low-tide FGP3/v4 entries whose cleanup and draw pixel counts are both zero;
active payload stayed `737600 -> 737600` for both packs. That means the earlier
entry-prune speed signal cannot be made cadence-preserving by simply replacing
payloads with empty holds.

The VISITOR3 zero-runtime-code entry-origin shift gate is closed too. Re-centering
each FGP3/v4 entry and subtracting the shift from compact cleanup/draw
coordinates saves `0` bytes on both current tides (`737600 -> 737600`), so
there is no pack payload or CD-duration win to benchmark.

The VISITOR3 duplicate-payload table-reuse gate is closed under the current
scheduler. Exact duplicate FGP3/v4 bodies exist, but the phase-preserving
variant removed only `5006` active bytes and regressed high/low to `1140/1024`
and `1163/1024`; the full variant removed `144068` active bytes but regressed
to `1158/1024` and `1165/1024`. Do not retry table-offset reuse without a
planner that explicitly preserves or reschedules CD phase.

The VISITOR3 offscreen-clip lane is closed as a broad both-tide standalone
pack transform, but tide-specific data-size-preserving subsets are promoted.
Clipping compact residual spans to the screen reduced logical payload
`737600 -> 655911` per tide and visibly improved high tide, but the
size-shrinking variant converted the win into hidden refill debt
(`prefetch_overrun_vb 0 -> 72/77`). Keeping entry sizes fixed avoided hidden
overrun and improved high `1139 -> 1137`; splitting low tide then proved
`ship-left` (`113..117`) is the bad phase while exit-right (`139..143`) is
safe and improves low `1140 -> 1138`. Runtime clipping already discards these
pixels visually; keep the promoted high and low exit-right trims, but do not
retry low `ship-left` without a scheduler-costed pack planner that preserves
low-tide phase.

Fresh low-tide scalar cap retry: `168 KiB` is not promotable. It preserves
layout but stays exact-flat at `1895`, `1604/1407`, `blocking_vb=271`,
`prefetch_overrun_vb=54`, and `loop_reads=132`, so the current WALKSTUF1
setup-prime knees are `160 KiB` low and `144 KiB` high. Do not keep raising
contiguous WALKSTUF1 caps without a generated segmented plan or a pack-shape
change.

Fresh low-tide scheduler signal: excluding WALKSTUF1 from the tight-slack
direct-stage shortcut is not promotable as source, but it proves the remaining
low-tide gap is scheduler-owned. It improves `loop_vb 1604 -> 1601`,
`blocking_vb 271 -> 214`, `loop_reads 132 -> 129`, and `due_misses 50 -> 34`,
but regresses hidden prefetch overrun `54 -> 79` and crosses the PS-EXE bucket.
Retry this only as a narrower/code-size-neutral guard or generated scheduler
metadata that avoids converting visible blocking into hidden overrun.

The narrower scalar follow-up, lowering `FG_PREFETCH_DIRECT_STAGE_MAX_BYTES`
from `8 KiB` to `4 KiB`, is also rejected. It keeps layout fixed and improves
blocking `271 -> 233`, but worsens `scene_vb 1895 -> 1898`, `loop_vb
1604 -> 1607`, target accounting `1407 -> 1405`, and hidden prefetch overrun
`54 -> 81`. Treat the direct-stage byte threshold as closed under the current
scheduler.

Recent promoted BUILDING4 restore-minus-current baseline: subtract current
PAL4 draw spans from each frame's compact cleanup prefix in both BUILDING4
FGP3/v4 packs while preserving the padded `1714154` byte CD footprint, pack
LBAs, and the `215040` byte PS-EXE bucket. Active payload drops
`1032442 -> 855284`; runtime restore bytes drop `1229878 -> 546950`.
High improves `loop_vb 2939 -> 2844`, `target_vb 2786 -> 2816`,
`overrun_vb 153 -> 28`, `blocking_vb 240 -> 37`, and loop reads
`81 -> 49`. Low improves `2945 -> 2855`, `2798 -> 2815`,
`147 -> 40`, `117 -> 46`, and loop reads `39 -> 30`. VISITOR3 high/low,
BUILDING2 high/low, ACTIVITY9 low, and FISHING1 high controls stayed on their
accepted profiles. This checkpoint later moved to `-0.0694%` over target /
`100.1388%` target speed after the VISITOR3 low read-table prune, WALKSTUF1
high setup-prime cap retune, VISITOR3 high/low offscreen draw clips, BUILDING2
high restore-minus-current cleanup, BUILDING2 high/low grouped-read passes, and
the VISITOR3 low scoped composite-helper pass, to `-0.0968%` over target /
`100.1613%` target speed after the VISITOR3 tail-trim stageguard pass, and to
`-0.2497%` over target / `100.2899%` target speed after the WALKSTUF1 compact
FGP3/v4 pass. The current rollup is tracked above.

Earlier promoted BUILDING4 cleanup-compact baseline: keep the cleanup-metadata-only FGP3/v3
format for both validated packs and retune the scene-local stream windows to
the new smaller payload cadence (`20 KiB` high, `32 KiB` low). It preserves
the `1714154`-byte CD footprint and PS-EXE bucket while shrinking active
payload `1705426 -> 1370198`. High improves `loop_vb 2985 -> 2939`,
`overrun_vb 211 -> 153`, `blocking_vb 285 -> 240`, and
`prefetch_overrun_vb 51 -> 27`; low improves `2981 -> 2945`, `197 -> 147`,
`199 -> 117`, and `119 -> 114`. FISHING1, VISITOR3 high/low, WALKSTUF1 high,
BUILDING2 high/low, and ACTIVITY9 high/low canaries stayed on their accepted
profiles. This checkpoint later moved to `0.4096%` over target /
`99.7860%` target speed after the FGP3/v4 compact draw metadata promotion and
compact metadata decoder inline follow-up; the current rollup is tracked above.

Latest promoted BUILDING2 baseline: keep the cleanup-metadata-only FGP3/v3
format for both validated packs. It preserves the `1303332`-byte CD footprint
and PS-EXE bucket while shrinking active payload `1296388 -> 1044638`. High
improves `loop_vb 1468 -> 1430`, `overrun_vb 183 -> 141`,
`blocking_vb 301 -> 212`, `prefetch_overrun_vb 56 -> 20`, and loop reads
`96 -> 82`; low improves `1465 -> 1429`, `189 -> 143`, `334 -> 193`, keeps
prefetch overrun flat at `35`, and cuts loop reads `87 -> 68`. FISHING1,
VISITOR3 high/low, WALKSTUF1 high, and BUILDING4 high canaries stayed flat.
This checkpoint later moved to `0.4096%` over target / `99.7860%` target
speed after the ACTIVITY9 low cleanup-metadata compaction, JOHNNY2
clean-pressure relief, selector cleanups, FGP3/v4 compact draw metadata, and
compact metadata decoder inline follow-up; the current rollup is tracked at
the top of this file.

Earlier promoted VISITOR3 cleanup baseline: keep the cleanup-metadata-only FGP3/v3
format for both validated packs. It preserves the `1555450`-byte CD footprint
and PAL4 draw payloads while shrinking active payload `1552446 -> 1265930`.
High improves `loop_vb 1450 -> 1406`, `overrun_vb 435 -> 387`,
`blocking_vb 355 -> 296`, and `prefetch_overrun_vb 14 -> 7`; low improves
`1452 -> 1405`, `440 -> 390`, `361 -> 301`, and `19 -> 8`. The largest
remaining absolute gaps are now VISITOR3, WALKSTUF1, BUILDING2, BUILDING4,
BUILDING6, and
generated selective preprocessing, not FISHING1.

Latest promoted compact-residual baseline: convert every current FGP3/v3
compact PAL4 residual pack to FGP3/v4 compact draw metadata while preserving
the padded CD footprint and pack LBAs. VISITOR3 active payload drops
`1265930 -> 981514` per tide and moves high/low from `1406/1019` and
`1405/1015` to `1369/1023` and `1376/1023`; BUILDING2 high/low move
`1430/1289 -> 1405/1298` and `1429/1286 -> 1395/1294`; ACTIVITY9 low moves
`2087/2056 -> 2085/2058`. The compact decoder inline follow-up then moves
VISITOR3 high/low to `1357/1023` and `1361/1023`, BUILDING2 high/low to
`1394/1301` and `1385/1303`, and leaves ACTIVITY9 low plus FISHING1 high
inside the broad stability gate. The follow-up VISITOR3 high read-group prune
is a code-headroom promotion: it keeps that same rollup while shrinking
`foregroundPilotPlay` by `48` bytes. The later VISITOR3 restore-minus-current
pack pass moved the all-scene battle card to `0.0502%` over target /
`100.0292%` target speed, and the later BUILDING4 restore-minus-current pack
pass moves it to `-0.0193%` over target / `100.0943%` target speed across
`120` timing-bearing rows. The later VISITOR3 high-only offscreen draw clip
moves it to `-0.0249%` over target / `100.0985%` target speed, and the
VISITOR3 low exit-right offscreen draw clip moves it to `-0.0266%` over target
/ `100.0998%` target speed. The later BUILDING2 high restore-minus-current
cleanup plus BUILDING2 high/low grouped-read passes move it to `-0.0670%` over
target / `100.1368%` target speed. The later VISITOR3 low scoped
composite-helper pass moves it to `-0.0694%` over target / `100.1388%` target
speed, the later VISITOR3 tail-trim stageguard pass moves it to `-0.0968%`
over target / `100.1613%` target speed, and the WALKSTUF1 compact FGP3/v4
pass moves it to `-0.2497%` over target / `100.2899%` target speed.

Latest promoted VISITOR3 scheduler pass: the old high-tide guarded generated
window `138..162` and later `72..84` cleanup proved VISITOR3 groups need
scheduler/slack ownership and coverage checks, not blind hand tables. After
compact-u16 inline, the remaining high-tide local table (`138..162`,
`170..186`, `230..242`) is now exhausted and was removed. Broad canaries stayed
exact-flat on VISITOR3 high/low, BUILDING2 high/low, ACTIVITY9 low, and
FISHING1 high with fixed foreground LBAs and the accepted `215040` byte PS-EXE
sector bucket, while `foregroundPilotPlay` shrank by `48` bytes. Treat that as
code headroom for generated scheduler/data-shape work, not a VBlank speed win.
A current-baseline prepared-present threshold retune
from held slack `4` to `5` stayed exact-flat on VISITOR3 high and low, so do
not spend more local threshold-only probes here without a new scheduler budget.
Pack-only trailing-zero compaction is also rejected for VISITOR3: it saved
`573032` bytes across the two packs but worsened both tides because it changed
CD phase/adjacency without reducing active payload.

Latest promoted read-group selector cleanup: choose scene-local stream read
groups into local variables and assign `gFgRuntime.streamReadGroups` /
`streamReadGroupCount` once after policy selection. The 13-case canary set
stayed exact-flat on timing, pack LBAs, and the `215040` byte PS-EXE bucket,
while `foregroundPilotPlay` shrank `11408 -> 11372` (`-36`). Treat this as
code-headroom for generated scheduler/read-metadata probes, not a VBlank speed
win.

Latest promoted dead read-group cleanup: remove the stale ACTIVITY9 low
FGP3/v1 `{624,636}` selector/table. Current `ACTV9LOW.FG2` is FGP3/v3 compact
and the current baseline policy was already `none`, so the branch could never
produce a read win. The broad 13-case canary stayed exact-flat on timing, pack
LBAs, and the `215040` byte PS-EXE bucket while `foregroundPilotPlay` shrank
`11372 -> 11356` (`-16`). Artifact:
`scratch/ps1-perf-iterate/activity9-dead-readgroup-prune-v082-canaries/20260507-132441-4070377/summary.json`.

Latest promoted FGP3/v4 draw-metadata compaction: keep the same padded file
sizes and LBAs for `ACTV9LOW`, `BUILDING2`/`BUIL2LOW`,
`BUILDING4`/`BUIL4LOW`, and `VISITOR3`/`VIST3LOW`, but compact the PAL4 draw
row/span metadata after the compact cleanup tail. Broad canaries passed for
VISITOR3 high/low, BUILDING2 high/low, ACTIVITY9 low, and FISHING1 high.
Artifact:
`scratch/ps1-perf-iterate/fgp3v4-drawcompact-all-v082-broad/20260507-143445-284827/summary.json`.

Latest promoted compact decoder inline pass: change `grReadCompactSpanU16`
from a noinline `-Os` helper to default inline code so FGP3/v4 compact span
metadata reads avoid per-field call overhead. Focused promotion passed
VISITOR3 high/low and BUILDING2 high/low; broad stability also passed
ACTIVITY9 low and FISHING1 high with fixed pack LBAs and the `215040` byte
PS-EXE bucket. Artifact:
`scratch/ps1-perf-iterate/compact-u16-inline-v083-broad-stability/20260507-153511-639350/summary.json`.

Latest promoted code-headroom pass: remove the exhausted VISITOR3 high local
retained-read table and selector branch. Focused VISITOR3 high/low and broad
VISITOR3 high/low, BUILDING2 high/low, ACTIVITY9 low, and FISHING1 high gates
were exact-flat against the compact-u16 inline baseline; `foregroundPilotPlay`
shrinks by `48` bytes and the public rollup at that checkpoint remains
`0.4096%` over target / `99.7860%` target speed. Artifact:
`scratch/ps1-perf-iterate/visitor3-high-readgroup-prune-v084-broad/20260507-163049-965277/summary.json`.

Latest promoted VISITOR3 restore-minus-current pack pass: remove FGP3/v4
cleanup intervals that are fully redrawn by the same current PAL4 draw frame,
without changing draw pixels, padded file sizes, foreground LBAs, or the
`215040` byte PS-EXE bucket. VISITOR3 high improves `1357/1023 -> 1139/1024`,
`overrun_vb 334 -> 115`, `blocking_vb 246 -> 191`, `prefetch_overrun_vb 1 -> 0`,
and `loop_reads 35 -> 33`; low improves `1361/1023 -> 1140/1024`,
`338 -> 116`, `250 -> 194`, `2 -> 0`, and `35 -> 33`. Active payload drops
`981514 -> 737600` per tide and runtime `restore_bytes` drops
`973290 -> 498676`. BUILDING2 high/low, ACTIVITY9 low, and FISHING1 high
controls stayed exact-flat in the broad gate. The public rollup is now
`0.0502%` over target / `100.0292%` target speed at that checkpoint; the later
BUILDING4 restore-minus-current pass moves it to `-0.0193%` / `100.0943%`.
Artifact:
`scratch/ps1-perf-iterate/visitor3-restore-minus-current-v086-broad/20260507-171251-1205609/summary.json`.

Latest promoted MARY2 baseline: keep the same-layout padded FGP3 temporal
residual conversion for both validated packs. High tide improves the
same-commit current active loop `2385 -> 2330`, lowers overrun `135 -> 78`,
and keeps the pack LBA plus PS-EXE bucket fixed; low tide improves `2384 ->
2327`, lowers overrun `134 -> 75`, and keeps the same fixed-layout shape.
The row also replaces stale v0.6.8 timing, so the public rollup moves backward
even though the current pack is faster than its same-commit FGP2 control.

Latest promoted JOHNNY2 baseline: keep the same-layout padded FGP3 temporal
residual conversion and the JOHNNY2-local clean-pressure prefetch relief for
both validated packs. The FGP3 step improves the same-commit current active
loop `1833 -> 1801/1800`; the clean-pressure relief then moves both tides to
`1741/1751`, lowers blocking `369/377 -> 0`, drops due misses `144 -> 0`, and
cuts loop reads `144 -> 8` while keeping pack LBAs and the PS-EXE bucket fixed.

Latest promoted MARY5 baseline: keep the same-layout padded FGP3 temporal
residual conversion for both validated packs. High tide improves active loop
`1591 -> 1581`, eliminates overrun `9 -> 0`, lowers blocking `8 -> 5`, lowers
prefetch overrun `8 -> 0`, and cuts loop reads `49 -> 42`; low tide improves
active loop `1592 -> 1581`, eliminates overrun `11 -> 0`, lowers blocking
`10 -> 6`, lowers prefetch overrun `10 -> 2`, and cuts loop reads `49 -> 42`.
Both LBAs and the PS-EXE bucket stay fixed.

Latest promoted ACTIVITY11 baseline: keep the same-layout padded FGP3 temporal
residual conversion for both validated packs. High tide improves active loop
`1729 -> 1715`, eliminates overrun `9 -> 0`, lowers blocking `10 -> 2`,
lowers prefetch overrun `4 -> 2`, and cuts loop reads `29 -> 11`; low tide
improves active loop `1729 -> 1717`, eliminates overrun `12 -> 0`, lowers
blocking `14 -> 4`, keeps prefetch overrun to `4`, and cuts loop reads
`29 -> 11`. Setup cost rises by `8/11` VBlanks because FGP3 primes more before
playback, so treat this as an accepted active-loop/data-shape win rather than a
full-scene setup win.

Latest promoted VISITOR3 baseline: keep the scene-local `192 KiB` setup-prime
resident cap with the global setup-prime cap still at `128 KiB`, plus the
accepted low-tide `170..186` grouped append. It improves
VISITOR3 high from `1455/1010` to `1450/1015`, lowers blocking `361 -> 355`,
lowers prefetch overrun `21 -> 14`, and lowers loop reads `52 -> 45`. It also
improves VISITOR3 low from `1453/1009` to `1452/1012`, lowers blocking
`365 -> 361`, lowers prefetch overrun `23 -> 19`, and lowers loop reads
`51 -> 49`. Do not generalize this cap or group before canaries prove the
memory residency and append cadence are safe for other scenes.

Rejected cap edge: `196 KiB` regressed high-tide loop `1450 -> 1454`, overrun
`435 -> 440`, blocking `355 -> 356`, and prefetch overrun `14 -> 17`; `200 KiB`
regressed loop `1450 -> 1452` and overrun `435 -> 437` despite lowering
blocking `355 -> 353`; `208 KiB` regressed overrun `435 -> 436`, blocking
`355 -> 359`, and loop reads `45 -> 49` despite lowering prefetch overrun
`14 -> 13`. A later current-baseline `224 KiB` retry after cleanup compaction
kept layout fixed and lowered reads, but regressed high `1406 -> 1410` and low
`1405 -> 1413` with worse visible blocking. Treat `192 KiB` as the closed
accepted cap. The opposite cap-down retry at `184 KiB` also failed, regressing
high to `1408` and low to `1419`, so `192 KiB` is the measured knee rather
than just the largest safe point. The next VISITOR3 work should be
scheduler-owned CD timing or selective pack/data-shape preprocessing.

Rejected post-cap standalone group: `{158,170}` saved one nominal VISITOR3 high
read (`45 -> 44`) but regressed loop `1450 -> 1453`, overrun `435 -> 438`,
blocking `355 -> 357`, and prefetch overrun `14 -> 16`. Do not retry more
current-fit VISITOR3 groups as source tables; the planner's scheduler-owned
classification is now confirmed after setup-prime.

Rejected low-tide small group: `{126,132}` stayed exact-flat after the accepted
`170..186` append (`1452/1012`, blocking `361`, prefetch overrun `19`, loop
reads `49`). This closes the smallest fireable tight-gap low-tide probe too;
VISITOR3 should now move away from raw standalone append tables.

Latest promoted WALKSTUF1 baseline: keep the PAL4/FGP2 setup-prime policy that
applies the existing `88 KiB` setup-prime base to WALKSTUF1 even though the
validated packs are not indexed8 residual packs. High tide improves from
`1640/1406` to `1595/1403`, lowers overrun `234 -> 192`, blocking
`318 -> 278`, prefetch overrun `67 -> 50`, loop reads `144 -> 136`, and
loop read VBlanks `663 -> 601`. Low tide improves from `1631/1414` to
`1614/1397`, lowers blocking `296 -> 276`, prefetch overrun `60 -> 59`,
loop reads `147 -> 134`, loop read VBlanks `660 -> 605`, and due misses
`55 -> 49`; target accounting moved with the loop, so overrun stayed `217`.
The later low-tide-only cap retune raises the resident setup-prime cap to
`160 KiB` for low tide, keeps high tide exact-flat, and improves low again to
`1604/1407`, overrun `217 -> 197`, blocking `276 -> 270`, prefetch overrun
`59 -> 55`, and loop reads `134 -> 132`. The current high-tide-only cap retune
then raises high setup residency to `144 KiB` and improves high to
`1592/1406`, overrun `193 -> 186`, blocking `277 -> 275`, loop reads
`136 -> 134`, and due misses `56 -> 55` while keeping low exact-flat.
Canaries stayed exact-flat for FISHING1, VISITOR3 high/low, WALKSTUF1 high/low,
BUILDING2 high/low, BUILDING4 high/low, BUILDING6 high/low, and ACTIVITY9
high/low. Read-plan setup coverage now reports `auto:walkstuf1-low` as
sectors `2..78` and `auto:walkstuf1-high` as sectors `2..74`.

Rejected setup-owned hot segment: priming VISITOR3 high sectors `158..170`
during setup and retaining that segment for multiple entry copies still
regressed loop `1450 -> 1451`, overrun `435 -> 438`, and prefetch overrun
`14 -> 19` with blocking flat at `355`. Do not retry setup segments as a local
VISITOR3 fix; the extra resident bytes/read cadence still perturb the active
schedule. The next VISITOR3 work should be generated scheduler metadata or a
pack/data-shape change that reduces upload/CD work rather than relocating the
same sector cluster into setup.

Rejected post-cap larger group: VISITOR3 high `{146,170}` proved that simply
using the larger retained setup-prime buffer as group capacity is not enough.
It saved three reads (`45 -> 42`) but regressed loop `1450 -> 1456`, blocking
`355 -> 358`, and prefetch overrun `14 -> 18`. Do not retry 24-sector
VISITOR3 high groups through the current append path; the scheduler needs a
first-class budget/ownership change before larger generated groups are viable.

Rejected local direct-stage cap: VISITOR3 high with a scene-local `16 KiB`
tight-slack direct-stage cap stayed exact-flat against the accepted 192 KiB
setup-prime baseline (`scene_vb=1746`, `loop_vb=1450`, `blocking_vb=355`,
`prefetch_overrun_vb=14`, `loop_reads=45`) while shifting hot foreground code.
Do not retry VISITOR3 direct-stage caps without generated coverage metadata.

Rejected BUILDING2 high setup-prime: a high-only `64 KiB` contiguous setup
window improved raw active-loop time (`loop_vb 1468 -> 1457`) and prefetch
overrun (`56 -> 42`), but failed the full gate by regressing overrun
`183 -> 186`, visible blocking `301 -> 324`, and target accounting
`1285 -> 1271`. Do not retry contiguous BUILDING2 high setup-prime sizing as
a local source knob; the next BUILDING2 high path should be scheduler-owned
refill timing, segmented/prepared coverage, or data-shape preprocessing.

Rejected WALKSTUF3 low padded FGP3: the size gate looked good
(`986873 -> 811444` payload bytes inside the same `994669` byte file), and
runtime CD pressure improved (`loop_reads 72 -> 61`, `blocking_vb 45 -> 43`,
`prefetch_overrun_vb 25 -> 19`), but full timing regressed by one VBlank
(`scene_vb 2583 -> 2584`, `loop_vb 2320 -> 2321`). Do not promote one-sided
WALKSTUF3 low FGP3 conversion; retry only if paired high/low conversion,
setup residency, or scheduler ownership preserves total timing.

## 2026-04-30 ASM And Toolchain Feasibility Intake

Source: `/home/hunter/workspace/jc_ps1_sandbox/docs/ps1/hand-rolled-asm-feasibility.md`.

The feasibility pass says hand-written MIPS is viable, but it should not be the
first move. The cheaper, higher-confidence ladder is: make `-O2` tests the top
priority across the whole build, use word-stride C in restore/compose loops,
try scratchpad palette storage, then write small assembly only for a measured
hot loop. Current `CMakeLists.txt` already leaves `src/graphics_ps1.c` out of
the whole-TU `-Os` list, so the immediate `-O2` idea becomes a build-wide
audit/gate: confirm which files/helpers are actually compiled at default
`-O2`, then test every accepted `-Os` override back at `-O2` one at a time
before trying harder code changes. Do not retry broad `-O3` for
graphics/foreground/CD; those failures are still valid.

`-O2`-first sweep order:

| Order | Target | Why first |
|---:|---|---|
| 1 | Verify build flags and map output for all hot/cold TUs. | Avoid guessing. The default SDK build is already `-O2`; the test is identifying where accepted `-Os` overrides are now costing speed. |
| 2 | Scoped graphics helpers currently forced to `-Os`. | Graphics/restore/compose are the closest thing to free performance if the smaller helper shape is slower. |
| 3 | Hot or semi-hot whole TUs currently forced to `-Os`: `foreground_pilot.c`, `sound_ps1.c`, `events_ps1.c`, `resource.c`, and `jc_reborn.c`. | These can affect active playback, setup, input, or sound cadence; test individually with exact visual/audio/perf gates. |
| 4 | Cold/default-off TUs currently forced to `-Os`: pause/menu/captions/memcard/holidays/debug/stubs/utils/island. | Lower probability of speed wins, but they can still perturb hot phase or shrink/grow the executable; record outcomes. |
| 5 | Only after the `-O2` sweep: word-stride C restore/compose, scratchpad palette, then switchable hand-written MIPS. | Keeps the high-risk work behind the low-risk compiler flag matrix. |

Prioritized intake from that research:

| Priority | Experiment | Acceptance signal |
|---:|---|---|
| 1 | Build-wide `-O2` audit and sweep: prove defaults, then test every currently accepted `-Os` TU/helper as `-O2`, one target per experiment. | Same pixels/sound, no PS-EXE bucket regression unless explicitly accepted, and either lower loop/render/setup counters or a documented no-promotion result. |
| 2 | Graphics-specific `-O2` pass: prove `graphics_ps1.c` is default `-O2`, dump per-function sizes, then test scoped `-O2` versus accepted scoped `-Os` only on the graphics helpers that currently override codegen. | Same pixels/sound, no PS-EXE bucket regression, and either lower loop/render counters or a smaller hot symbol with flat timing. |
| 3 | Word-stride C restore copy for `grCleanRectCopyIn`: halfword edges, word-body loop, bounded row widths. | Restore detail counters or total loop improve without CD pressure or stale pixels. |
| 4 | Word-stride C compose helpers: decode 8 PAL4/indexed pixels per `uint32` load and store aligned 16bpp pairs as `uint32` where opaque/aligned. | Compose detail counters improve on WALKSTUF1/VISITOR3 or another sprite-heavy route with exact visual output. |
| 5 | Scratchpad palette copy for compose: copy the active 16-color palette to `0x1F800000` for the compose pass. | Compose counters improve without memory ownership conflicts; scratchpad layout is documented before promotion. |
| 6 | Hand-written MIPS only after the C/data-placement versions leave a measured gap. Start with the restore row copy before transparent compose loops. | ASM and C implementations remain switchable for A/B runs; accepted only with visual hashes/human signoff plus perf improvement. |

Red-team caveat: setup-prime passes are active-loop wins, not end-to-end
scene-time wins. The latest fishing2/fishing3 budgets move `128-352 KB` of
foreground reads into setup so active playback can reduce visible CD pressure
and spend more catch-up. Future work should hide these primes during
inter-scene/loading time or generate scene/tide-specific segmented coverage,
rather than treating setup time as free.

Current note: the final-frame-hold correction is the new broad timing baseline.
It removes the artificial post-final `150` VBlank scene-pack tail while still
displaying the captured final frame for its own hold. The refreshed 11-case
canary set saves `1455` active-loop VBlanks total and moves FISHING1 high under
target (`1207/1074 -> 1067/1074`). The older fishing1 high-tide tail read group
`396..406` remains a work-reduction checkpoint, not a VBlank speed win. It kept
that era's remaining VBlank gap unchanged while dropping `loop_reads 68 -> 67`,
`setloc 74 -> 73`, `loop_read_vb 284 -> 283`, and `seek_back 5 -> 4`; the
follow-up retained-capacity pass kept that saved read while reducing the
runtime prefetch buffer `31760 -> 29712` bytes. The first two narrow cold-TU
compiler probes are also accepted: `ps1_captions.c -Os` and `memcard.c -Os`
kept timing and layout flat while shrinking `jcreborn.elf 741076 -> 740196`.
Recent flat graphics cleanups compile only `grDrawBackground()` and
`grUpdateDisplay()` with `-Os`, keeping all VBlank/CD/work metrics and
upload/dirty counters unchanged while shrinking the ELF before the dirty-row
and CD-helper wave.
The `grRestoreBgFromRects()` function-scoped `-Os` retry is rejected despite a
local function shrink, because total ELF grew to `714132` bytes with no VBlank
movement.
The PAL4 compositor function-scoped `-Os` retry is also rejected: it shrank the
loaded executable, but still regressed to `blocking_vb=26` even when a temporary
CD pad preserved `FISHING1.FG2` at LBA `396`.
The single-band narrow-upload scratch path is rejected too: fishing1 did not
hit a useful single-band case, so upload bytes stayed flat while code grew.
The latest accepted cleanup clears only touched current dirty rows before each
frame restore, preserving all timing/work counters while shrinking the ELF to
`712692` bytes.
The latest speed win also promotes only touched dirty-row ranges from current
to previous dirty state, improving `loop_vb 1221 -> 1219` and
`overrun_vb 150 -> 147` with CD pressure and graphics work stable.
A direct PAL4 row dirty-marking probe after that baseline is rejected even
though the formal gate passed: `loop_vb` stayed flat and the apparent overrun
gain came only from a `target_vb` shift while the compositor grew.
Skipping previous dirty-row clears for rows overwritten by current dirty rows
is also rejected as an isolated change: it stayed exact-flat but grew the ELF.
Post-dirty raw stream-window retuning is rejected: `20 KB` regressed visible
CD pressure, while `18 KB`/`17 KB` hit a structural invalid-read failure before
metrics.
The aligned CD read helper now caches its file LBA once per read, keeping exact
cadence flat while shrinking the hot helper by 8 bytes and ELF by 84 bytes.
A single-chunk branch inside that helper is rejected: the common-case branch
duplicated too much error/read code and grew the hot helper by 104 bytes with
no timing movement.
Function-scoped `-Os` on the aligned CD read helper is accepted: it keeps
exact cadence flat while shrinking the public aligned-read wrapper to 8 bytes
and ELF to `712556`.
The current v0.7.2 default-`O2` retest of that same aligned helper is rejected:
exact canaries stayed key-flat except one BUILDING2 blocking-only VBlank in the
partial run, while `ps1_streamReadAlignedIntoFile` grew by `+524` bytes and the
ELF grew `952312 -> 952488`. Keep the helper at function-scoped `-Os` until CD
helper splitting, async/scheduler ownership, or generated read metadata changes
the active helper shape.
Function-scoped `-Os` on the unbuffered stream-read helper is accepted too,
shrinking that setup-facing helper by 56 bytes and ELF to `712524` with exact
playback identity.
The current v0.7.2 default-`O2` retest of the unbuffered helper is rejected:
FISHING1 regressed `loop_vb 1068 -> 1069`, `blocking_vb 4 -> 9`,
`loop_reads 20 -> 21`, and `due_misses 0 -> 1`, while the helper grew
`592 -> 660` bytes and ELF grew `952312 -> 952548`. Keep the helper at
function-scoped `-Os` until setup/direct-read splitting or scheduler metadata
changes the phase.
The current v0.7.2 default-`O2` retest of `src/ps1_stubs.c` is also rejected:
FISHING1, VISITOR3, WALKSTUF1, BUILDING2, and ACTIVITY9 exact canaries stayed
fully flat while ELF grew `952312 -> 952360` and the stubs object grew
`16640 -> 16680`. Keep the translation unit at `-Os` and do not put remaining
cold/default-off `-O2` probes ahead of generated metadata without a stronger
phase hypothesis.
The current v0.7.2 default-`O2` retest of `src/pause_menu.c` is rejected:
it grew the PS-EXE bucket by `2048` bytes, shifted foreground LBAs by `+1`,
and regressed FISHING1, WALKSTUF1, BUILDING2, and ACTIVITY9 canaries. Keep the
translation unit at `-Os`; do not retry pause-menu `-O2` unless link-phase
padding or pause/menu source shape changes materially.
The current v0.7.2 default-`O2` retest of `src/ps1_captions.c` is rejected:
FISHING1 stayed exact-flat and the PS-EXE bucket stayed fixed, but the ELF and
captions object both grew with no speed or work-volume improvement. Keep the
translation unit at `-Os`.
The current v0.7.2 default-`O2` retest of `src/memcard.c` is rejected:
FISHING1 regressed `loop_vb 1068 -> 1069`, visible blocking `4 -> 5`, and
prefetch overrun `4 -> 6` while the ELF and memcard object grew. Keep the
translation unit at `-Os`.
The current v0.7.2 default-`O2` retest of `src/holidays.c` is rejected:
FISHING1 showed the same visible regression pattern while the ELF grew by
`3472` bytes and the holidays object grew `25440 -> 30008`. Keep the
translation unit at `-Os`.
The current v0.8.0 default-`O2` retest of `src/ps1_debug.c` is rejected:
against the refreshed `v080-current-fishing1-baseline`, FISHING1 stayed
exact-flat at `1069/1073`, but ELF grew `954192 -> 954632` with no timing or
work-volume win. Keep the translation unit at `-Os`.
The current v0.8.0 default-`O2` retest of `src/utils.c` is rejected:
FISHING1 stayed exact-flat at `1069/1073`, but ELF grew `954192 -> 955436` and
tracked hot symbols shifted by `+20` bytes. Keep the translation unit at
`-Os`.
The current v0.8.0 default-`O2` retest of `src/island.c` is rejected:
FISHING1 stayed exact-flat at `1069/1073`, but ELF grew `954192 -> 954492` and
tracked graphics/CD symbols shifted by `+48` bytes. Keep the translation unit
at `-Os`; the normal cold compiler-flag queue is exhausted, leaving only
review-only/default-off surfaces unless the link layout changes materially.
The current v0.8.0 default-`O2` retest of `src/ps1_pad_script.c` is rejected:
FISHING1 stayed exact-flat at `1069/1073`, but ELF grew `954192 -> 954248` and
tracked CD helper symbols shifted by `+36` bytes. Keep the translation unit at
`-Os`.
The current v0.8.0 default-`O2` retest of `src/scene_freeplay.c` is rejected:
it grew the PS-EXE bucket `215040 -> 217088`, shifted `FISHING1.FG2` LBA
`434 -> 435`, and grew ELF `954192 -> 960236` with no timing win. Keep the
translation unit at `-Os`.
The current v0.8.0 default-`O2` retest of `src/scene_picker.c` is rejected:
FISHING1 stayed exact-flat with fixed tracked hot symbols, but ELF grew
`954192 -> 955976` with no timing or work-volume win. Keep the translation unit
at `-Os`; the `-O2` audit queue is exhausted.
The same unbuffered helper now also caches its file LBA once, shrinking it by
another 32 bytes and ELF to `712332` with exact playback identity.
Function-scoped `-Os` on `fgRuntimeFillWindowForEntry()` is rejected as an
exact no-op: the accepted foreground TU codegen already emits the same helper
shape.
The first prepared-visual decoupling pass is rejected but informative:
metadata-only decoupling was flat and code-heavy, stage-next decoupling reduced
`loop_read_vb` by 3 without moving `loop_vb`, and preparing earlier at `>=4`
failed structurally. The next retry needs a real scheduler budget or separate
prepared visual storage, not another local threshold tweak.
A positive-slack-only stage-next retry reproduced the same flat/key-metric
result, so local prepared-payload decoupling is exhausted for this baseline.
The `102..110` FG2 read-group probe is rejected even though it saved one
transaction: visible CD pressure rose from `5` to `8` VBlanks, confirming that
new groups need a read-duration/slack cost model before promotion.
The first C restore-row word-copy probe is also rejected. The compatible
alignment helper kept FISHING1/FISHING2/FISHING3 timing exact-flat, but grew the
ELF and map with no restore, loop, CD, or work-counter benefit. Do not retry this
local `grCleanRectCopyIn()` row-copy shape unless finer counters prove a
sub-VBlank restore bottleneck; prefer generated restore bands or switchable ASM
after data-shape changes.
The first PAL4 compose-width probe is rejected too. An aligned 32-bit pair-store
branch inside `grCompositePacked4OpaqueRun()` shrank ELF by `444` bytes, but
BUILDING4 high regressed `loop_vb 3071 -> 3077` and `blocking_vb 234 -> 237`
with unchanged read/work counts. Do not retry local PAL4 pair stores without
generated aligned command classes or a pack/runtime-owned pair LUT.
The upload path now reuses one stack `RECT` for immediate `LoadImage()` calls,
shrinking `grDrawBackground` by 8 bytes and ELF to `712272` while keeping every
timing, CD, upload, and correctness counter exact.
The indexed8 compositor now treats generated indexed8 spans as opaque visible
runs and selects the RAM background tile once per left/right run. This is
accepted for the WALKSTUF1 indexed8 rows: high improves `2013 -> 2002` and low
improves `2028 -> 2014`, with PAL4 canaries exact-flat. The blocking increase
means follow-up work should target scheduler/CD ownership or host-side
preprocessing, not another local indexed8 pixel-loop branch.
A 4-pixel manual unroll of that indexed8 opaque helper is rejected: it grew the
ELF, shifted hot symbols by `+296` bytes, left loop time flat, and traded small
blocking improvements for refill/overrun regressions on WALKSTUF1. Do not spend
more time on local indexed8 loop unrolls; the next indexed8 wins need data-shape
or scheduler changes.
A same-index run detector inside the indexed8 opaque helper is also rejected.
It helped WALKSTUF1 high by `5` VBlanks but made low tide `16` VBlanks slower,
matching the host-side warning that average byte runs are only about `1.8`.
Do not spend more time on runtime indexed8 branch tricks; use pack-time
direct16/upload-ready data or generated CD metadata instead.
The accepted indexed8 row-local dirty pass is the safe boundary of this local
compositor lane: WALKSTUF1 high moves `2002 -> 1971`, low moves `2014 -> 1958`,
blocking and due misses improve, and PAL4 canaries stay exact-flat. The
tradeoff is refill overrun (`101 -> 139` high, `95 -> 140` low), so the next
indexed8 work should be scheduler/CD ownership, generated read metadata, or
pack-time direct16/upload-ready spans rather than more pixel-loop branching.
The first post-row-local WALKSTUF1 low read-group probe `{180,204}` is rejected:
it lowered hidden refill overrun but regressed visible loop and blocking
(`1958 -> 1998`, `452 -> 477`). Do not spend more time on raw saved-read
groups for WALKSTUF1 low unless the generator can model visible cadence; move
that lane toward scheduler ownership or upload-ready/direct16 pack data.
The naive WALKSTUF1 low direct16 FGP3/v3 probe is also rejected: removing
palette lookups did not matter enough to offset the pack growth
(`2.16 MB -> 2.92 MB`) and extra CD pressure (`loop_reads 62 -> 85`,
`blocking_vb 452 -> 712`). Future upload-ready work must be selective or
compressed, not whole-payload 16bpp expansion.
The current validated WALKSTUF1 pal4 packs also reject direct padded-FGP3
conversion before runtime measurement. `build-fg3-temporal-residual-pack.py`
expands both high and low payloads `1530775 -> 1712687`, so keeping the old
`1535263` byte CD footprint would truncate real data. Do not retry pal4 padded
FGP3 for WALKSTUF1 unless the encoder changes enough to prove the pack fits, or
the experiment explicitly accepts a layout-moving pack and runs broad canaries.
A VISITOR3-only `192 KiB` setup-prime cap is rejected. High tide traded lower
blocking for worse loop/refill, and low tide regressed across all key metrics.
Do not retry larger contiguous setup residency for VISITOR3; any preload work
there needs segmented/costed ownership rather than a bigger startup window.
BUILDING2 low is the next accepted setup-prime proof point. A shared high/low
`128 KiB` window failed because high tide regressed, but narrowing the policy
to low tide only improves `loop_vb 1559 -> 1542`, `overrun_vb 262 -> 252`,
`blocking_vb 150 -> 139`, and `prefetch_overrun_vb 44 -> 32` while high tide
stays exact-flat. Continue BUILDING-family work as per-tide policy, not broad
scene-family defaults.
BUILDING2 low also has one accepted grouped-read follow-up at `318..330`.
The group keeps low-tide `loop_vb=1542` while reducing `overrun_vb`,
`blocking_vb`, `prefetch_overrun_vb`, and `loop_reads`; high tide improves
under the same fixed source layout and the broad FISHING/VISITOR/WALKSTUF
canary set stays exact-flat. Treat this as a layout-sensitive read-metadata
win, not proof that arbitrary BUILDING groups are safe.
BUILDING4 low now has its own accepted stream-window knee at `36 KiB`.
It improves `loop_vb 3083 -> 3068`, `blocking_vb 180 -> 96`,
`loop_reads 51 -> 40`, and `due_misses 20 -> 2`, while raising hidden refill
overrun `60 -> 90`. BUILDING4 high, BUILDING6 high/low, and the FISHING,
VISITOR3, and WALKSTUF1 canaries stayed stable. Treat this as the current
model for BUILDING-family work: scene/tide-specific, fresh-baseline only, and
accepted only when visible loop/blocking wins justify any refill tradeoff.
Broad BUILDING4/6 setup-prime at `64 KiB` or `128 KiB` and family-wide
window retunes are rejected; retry preload for these scenes only with generated
segment coverage or scheduler-owned setup/preload budgeting.
Setup-time first-frame prerendering is rejected. The clock-reset variant left
STAND1 exact-flat, and the no-clock variant regressed FISHING1 visible CD
pressure. Treat zero-CD overrun as distributed per-frame render/present/upload
cost, not first-frame placement; future work should use upload-ready pack data
or a first-class present/upload scheduler.
The per-compose PAL4 scratchpad palette probe is rejected: it shrank the
compositor and ELF but did not move loop time safely, and VISITOR3 low regressed
hard. Do not spend more time on local scratchpad palette copies; scratchpad work
needs a generated or assembly compositor that owns layout and proves cross-scene
cadence.
The one-off 16-byte alignment probe for `fgRuntimeFillWindowForEntry()` is also
rejected: it kept the PS-EXE bucket stable but left every key seven-case metric
flat while growing ELF. Future address-bucket work should be scripted across
multiple functions and alignments so it can find a real phase knee instead of
testing one helper by hand.
The SDK-supported `NOGPREL` / `-G0` executable mode is rejected: it grew the
PS-EXE by two sectors, shifted FG pack LBAs, and regressed WALKSTUF1 high despite
a small VISITOR3-high blocking-only improvement. Keep `GPREL` / `-G8`; `-G4` or
`-G16` would require custom toolchain plumbing and should wait behind generated
read metadata and upload-ready/direct16 work.
The VISITOR3 high `72..84` 12-sector group is accepted after fixing the
read-plan tool to select the intended case from multi-case summaries. It is a
small but clean CD-pressure win against the fresh current baseline:
`loop_vb 1505 -> 1503`, `overrun_vb 500 -> 496`, `blocking_vb 357 -> 350`,
`prefetch_overrun_vb 109 -> 103`, `loop_reads 61 -> 58`, and
`due_misses 25 -> 24`, with the six canary rows exact-flat. Continue VISITOR3
through generated/costed groups such as `110..122` or `110..126`, not scalar
window changes.
The first `110..122` hand-coded follow-up is rejected: the seven-case gate was
exact-flat and VISITOR3 high reported `group_hits=0`. This does not disprove
the sector cluster; it says the runtime did not request the append start the
manual table expected. The next grouped-read attempt needs append-start
simulation or generated metadata, not another raw table entry from the same
candidate list.
The start-aligned `112..124` retry is also rejected: it changed the cadence but
regressed `loop_vb`, `blocking_vb`, `loop_reads`, and `due_misses` while only
reducing `prefetch_overrun_vb`. The next CD grouping path must score visible
blocking cost, not just saved transactions.
The later VISITOR3 high `230..242` group is accepted: it keeps the same PS-EXE
bucket and pack LBA, improves `loop_vb 1503 -> 1496`, `blocking_vb 350 -> 345`,
`prefetch_overrun_vb 103 -> 102`, `loop_reads 58 -> 57`, and `loop_read_vb 468 -> 458`,
while the six canary rows stay exact-flat. Treat this as evidence that selective
late groups can pay when they are both start-aligned and slack-safe; do not
generalize it to tight early clusters without a cost model.
The adjacent `242..254` group is rejected: despite matching an actual read
start, it regressed VISITOR3 high to `1507/1007`, raised `blocking_vb 345 -> 357`,
and raised `prefetch_overrun_vb 102 -> 109` with no read-count win. Adjacent
post-win grouping needs per-read cost modeling before more hand-coded trials.
The tail `738..749` group is also rejected: it reduced `prefetch_overrun_vb`
but regressed `loop_vb 1496 -> 1507`, `blocking_vb 345 -> 356`, and
`due_misses 24 -> 26`. Stop hand-coding VISITOR3 groups after `230..242` until
the planner can score visible blocking and due-frame risk, not just read count.
Function-scoped `-Os` on the buffered CD helper is rejected: it kept timing
flat but grew the ELF and did not shrink the helper.
Retesting the staged-copy fallthrough guard at `5` held VBlanks is rejected:
it doubled visible CD pressure after the CD-helper cleanup, so the current
`6` VBlank guard remains the local knee.
Raising that guard to `7` is also rejected as a structural failure before
metrics. Local fallthrough-threshold probing is exhausted for this baseline.
The latest harness pass adds host-side CD-summary comparison, so future
`blocking_reads 4 -> 5` regressions can be localized to FG2 file sectors
without adding PS1-side metrics that change the speed binary.
The latest runtime pass adds `JCPERF2 sched` ownership counters. It keeps
fishing1 exact-flat (`loop_vb=1219`, `blocking_vb=5`, `prefetch_overrun_vb=5`,
`FISHING1.FG2 LBA=396`, PS-EXE `143360`) and reports `present=72`,
`cd_stage=108`, `cd_window=54`, `visual_prepare=72`, `wait=574`,
`cd_reserved=28`, `prep_blocked_cd=13`, `prepared_ready=72`,
`prepared_used=72`, and `prepared_wasted=0`. The first owned-idle catch-up
prototype did not fire usefully (`catchup_idle=0`) and moved layout, so it was
reverted; the next scheduler attempt needs per-frame ownership analysis, not
another threshold-only catch-up.
The first host-side preprocessing pass is now analysis-first:
`scripts/analyze-fg2-preprocess-plans.py` exactly reproduces the accepted
fishing1 runtime graphics counters (`restore_bytes=2510092`,
`upload_bytes=16281600`, `upload_rects=502`) from the pack alone. It shows
that upload-ready x-bands could cut upload bytes by about `49.61%`, but only
by carrying about `8.2 MB` of aligned frame-band payload for fishing1. Exact
interval upload is a stronger byte floor (`86.83%` reduction) but explodes to
`95259` rects. Restore-skip metadata is the safer next pack-format experiment,
but only with coalescing: exact restore skip predicts `52.41%` lower restore
bytes but raises restore intervals from `24300` to `73417`; the current
`min8px_max4pieces` profile still saves `26.35%` with `36450` intervals.
The runtime prototype that parsed current PAL4 spans before restore confirmed
the byte savings but failed as an implementation path: the best variant reduced
`restore_bytes` to `2222854`, yet regressed `loop_vb 1219 -> 1221`, visible CD
pressure `5 -> 6`, and moved `FISHING1.FG2` from LBA `396` to `397`. Treat
restore-skip as an FGP3/side-metadata problem, not a runtime reparse problem.
The all-scene preprocessing opportunity matrix is now the pack-format targeting
surface. It parses current `FGP2`/`FGP3` packs and ranks selective upload-ready
or cleanup-metadata experiments against the measured battle card. The first
pass originally pointed at WALKSTUF1 low/high, VISITOR3 low/high, BUILDING4
high/low, BUILDING6 high/low, and BUILDING2 high/low. After the WALKSTUF1
compact pass, the current top generated graphics/scheduler targets are
VISITOR3 low/high, BUILDING2 low, WALKSTUF1 high/low, and BUILDING6 high/low.
Do not retry whole-payload direct16; use selective/compressed bands or
setup-resident upload-ready slices.
Two more hard-coded read-group probes are now rejected: `384..396` never fired
under the retained 11-sector capacity, and `307..317` kept every timing/read
counter exact while growing `foregroundPilotPlay` by `432` bytes. The direct
stage-into-window cache variant is also rejected: it removed the scratch-window
seed copy in theory, but exposed one extra visible CD VBlank. Finally,
single-TU `foreground_pilot.c -O3` is rejected as a no-win size/layout loss
(`jcreborn.exe 143360 -> 149504`, `FISHING1.FG2 LBA 396 -> 399`). The practical
conclusion is sharper: no more blind hard-coded groups or foreground-wide
compiler flags; the next CD win needs generated/costed group metadata or a
trace-backed scheduler budget.
The latest setup-prime wave proves a narrow exception: preloading enough FG2
coverage can make a previously unsafe threshold-`4` catch-up profitable, but
only when the catch-up is gated on a successful prime. The promoted `320 KB`
fishing1 high-tide prime improves `loop_vb 1219 -> 1215`, `overrun_vb
147 -> 140`, `blocking_vb 5 -> 1`, and `loop_reads 67 -> 43`; smaller
`192 KB`/`256 KB` versions either lost or still raised visible CD pressure.
The active-region/clean-rect follow-up found one safe narrow win: the static
backdrop has already been presented when FG2 clean rects are saved, so the
first forced upload no longer dirties all four screen tiles. Scoping that first
upload to the saved clean-rect Y band improves `loop_vb 1215 -> 1213`,
`overrun_vb 140 -> 138`, `max_upload_bytes 614400 -> 221440`, and
`upload_bytes 16281600 -> 15888640` without changing layout, restore bytes,
CD pressure, or correctness.
The I-B motion-comp analyzer changes the next pack-format order. Fishing1 has
large frame-to-frame reuse, but it is zero-shift temporal residual reuse rather
than translated motion: `151/154` candidate pairs, `71.16%` estimated payload
savings, and `0` nonzero-shift candidates. Walking scenes are the true
translation target (`WALKSTUF1` has `85` nonzero candidates; `WALK1LOW` has
`53`). Therefore the next fishing1-safe FGP3 experiment should be zero-shift
residual encoding first; GPU move/residual should wait for a walking-scene
validation path and a RAM-mirror/dirty-cleanup design.
The zero-shift runtime model is strong enough to promote to implementation
planning: fishing1 predicts compose payload `823277 -> 228087` (`72.30%`
saved), full-width dirty upload `15667200 -> 6576000` (`58.03%` saved), and
cleanup restore of only `136552` bytes. The hard invariant is that FGP3 must
carry full-current dirty metadata, because unchanged foreground pixels remain
in the RAM mirror and still need to be restorable on later frames.
The first FGP3 zero-shift temporal-residual pack is now promoted for fishing1
high tide. It converts `FISHING1.FG2` to `fgp3_pal4_residual`, improves
`loop_vb 1213 -> 1207`, `overrun_vb 138 -> 131`, and clears the last visible
high-tide CD pressure (`blocking_vb/prefetch_overrun_vb 1 -> 0`). Work volume
drops to `restore_bytes=251144`, `upload_bytes=6690560`, `upload_rects=290`,
and `loop_reads=6`. This accepted format change intentionally moves layout
(`FISHING1.FG2 LBA 396 -> 397`, PS-EXE `143360 -> 145408`), so future FGP3
work should claw back the executable cost and then fold residual generation
into the normal batch pack builder.
The follow-up red-team pass after FGP3 tested several local retries and found
the new bottleneck. Detail-tier attribution for the accepted canary reports
`present_wait_vb=155`, `compose_vb=2`, `restore_vb=0`, `upload_vb=0`,
`blocking_vb=0`, and `prefetch_overrun_vb=0`. Threshold-only prepared-present
changes, due-frame precompose, previous-dirty discard, FGP3 helper `-Os`, and
no-holiday call-site guarding all failed or stayed exact-flat. The next
high-impact path is no longer local restore/CD cleanup for fishing1 high tide;
it is a first-class present scheduler, a release/perf-log split, or a broader
pack/runtime architecture that can hide the mandatory VSync ownership without
dropping frames or weakening pause input.
The same FGP3 zero-shift format is now promoted for fishing1 low tide as well.
`FISH1LOW.FG2` converts to `fgp3_pal4_residual`, shrinks `426082 -> 303083`
bytes, and improves the low-tide gate `loop_vb 1215 -> 1209`,
`overrun_vb 142 -> 135`, `blocking_vb 5 -> 4`, `prefetch_overrun_vb 5 -> 4`,
and `loop_reads 31 -> 22`. High tide remained exact-flat after the low-tide
pack change. This makes generated all-scene FGP3 rollout a practical next
path, while low tide still has enough CD/refill pressure to justify targeted
pack-group or setup-prime policy work.
Fishing1 low tide now also uses the existing `320 KB` setup-prime policy.
Because the FGP3 low-tide pack fits inside the prime window, active-loop reads
fall `22 -> 0`, `blocking_vb 4 -> 0`, and `loop_vb 1209 -> 1207`; overrun
falls to the same `131` VBlank gap as high tide. This is intentionally logged
as an active-loop win with setup-cost trade (`setup_vb 182 -> 238`,
`scene_vb 1391 -> 1445`), so the next real global win is hiding that prime via
inter-scene preload or generated prime budgets.
FGP3 is now validated on fishing2 high tide as well. `FISHING2.FG2` shrinks
`1595559 -> 542743` bytes, clears due misses (`2 -> 0`), and improves
`loop_vb 1928 -> 1903`, `overrun_vb 190 -> 139`, `blocking_vb 50 -> 8`,
`prefetch_overrun_vb 44 -> 8`, and `loop_reads 134 -> 40`. This proves the
FGP3 residual approach scales past fishing1, but fishing2 still has enough
active-loop CD pressure to make setup-prime or generated pack-read groups the
next likely scene-specific win.
Fishing2 high tide now has a scene/tide-specific setup-prime budget. A
`352 KB` prime is the largest promoted point: it improves `loop_vb 1903 ->
1898`, `overrun_vb 139 -> 133`, `blocking_vb/prefetch_overrun_vb 8 -> 2`,
and `loop_reads 40 -> 14`. Larger contiguous primes are unsafe for this scene's
heap shape: `384 KB` and full-pack `544 KB` failed before loop start, while
`368 KB` hit the log cap/regtest `137`. The remaining two blocking VBlanks
should be attacked with generated read groups or inter-scene preload, not by
blindly growing the setup window.
A manual fishing2 high read-group probe for relative sectors `178..191` is
rejected. It moved the executable into the next sector bucket and shifted
`FISHING2.FG2 LBA 740 -> 741`, while regressing `loop_vb 1898 -> 1899` and
`blocking_vb/prefetch_overrun_vb 2 -> 3`. Any retry needs generated metadata
plus layout control, not another local source-table group.
Fishing2 low tide now also uses FGP3. `FISH2LOW.FG2` shrinks `784126 ->
385436` bytes, improves `loop_vb 1912 -> 1900`, `overrun_vb 157 -> 136`,
`blocking_vb/prefetch_overrun_vb 20 -> 5`, and `loop_reads 58 -> 27`, while
fishing2 high stays exact-flat. Remaining low-tide CD pressure is now small
enough to test generated setup-prime sizing or segmented prime coverage.
Fishing2 low tide now has a `256 KB` setup-prime budget. It improves
`loop_vb 1900 -> 1898`, `overrun_vb 136 -> 131`,
`blocking_vb/prefetch_overrun_vb 5 -> 0`, and `loop_reads 27 -> 10`, with
stable `FISH2LOW.FG2` LBA and PS-EXE bucket after cold diagnostic strings were
shortened. The rejected `320 KB` probe hit the log cap/regtest `137`, so low
tide should not grow a contiguous setup prime past `256 KB` without a new heap
or segmented-prime design.
Fishing3 high tide now uses FGP3 as the first larger next-scene proof point.
`FISHING3.FG2` shrinks `1831749 -> 724829` bytes and improves `loop_vb 2123 ->
2099`, `overrun_vb 189 -> 149`, `blocking_vb 87 -> 24`,
`prefetch_overrun_vb 39 -> 21`, and `due_misses 11 -> 1`. Its high-pack LBA and
PS-EXE bucket stay fixed; fishing3 low smoke still passes after the downstream
LBA shift. Next likely wins: FISH3LOW FGP3, then scene-specific or segmented
prime budgets.
Fishing3 low tide now also uses FGP3. `FISH3LOW.FG2` shrinks `906053 ->
549622` bytes and improves `loop_vb 2110 -> 2098`, `overrun_vb 156 -> 138`,
`blocking_vb 21 -> 8`, `prefetch_overrun_vb 21 -> 9`, and `loop_reads 65 ->
42`. Fishing3 now needs setup-prime or segmented preload work, not more format
conversion.
Contiguous fishing3 high setup-prime is rejected for now. `320 KB` failed
before playback; `256 KB` completed but kept `blocking_vb=24`, worsened
`due_misses 1 -> 2`, and moved both PS-EXE and `FISHING3.FG2` LBA. Retry this
scene only with segmented/generated prime coverage or inter-scene preload.
Fishing3 low tide now uses a `288 KB` contiguous setup-prime budget. The first
`256 KB` pass improved `loop_vb 2098 -> 2091` and `blocking_vb 8 -> 7`; the
retune keeps `loop_vb=2091` but lowers overrun/CD pressure to the fishing1-class
gap: `overrun_vb 134 -> 131`, `blocking_vb 7 -> 4`,
`prefetch_overrun_vb 7 -> 4`, and `loop_reads 24 -> 21`. Fishing3 high stays
exact-flat. The larger `320 KB` low-tide probe still remains rejected, so the
next larger target should be generated/segmented prime coverage, not another
blind contiguous read.
The `304 KB` low-tide retest confirms the knee: it kept layout stable but
regressed `blocking_vb 4 -> 5` and `overrun_vb 131 -> 132`. Keep `288 KB`
until segmented prime coverage can preload later ranges without shifting the
current CD phase.
Fishing3 high tide now has a smaller `128 KB` setup-prime budget. This is the
safe version of the earlier failed high-tide contiguous-prime idea: it improves
`loop_vb 2099 -> 2094`, `overrun_vb 149 -> 139`, `blocking_vb 24 -> 16`,
`prefetch_overrun_vb 21 -> 11`, and `loop_reads 52 -> 44` with stable layout.
Larger high-tide contiguous windows remain rejected; the next high-tide step
should be another measured small knee or segmented prime coverage, not a jump
back to `256 KB`.
The `160 KB` high-tide retest confirms the current knee: layout stayed fixed,
but active timing regressed to `loop_vb 2100`, `blocking_vb 29`, and
`due_misses 3`. Keep `128 KB` until the next test can preload later ranges
without using one larger contiguous read.
The `144 KB` midpoint is also rejected. It looked better only while moving the
executable/pack LBA; after recovering layout with cold string shrink, it
regressed `loop_vb 2094 -> 2096` and `blocking_vb 16 -> 21`. Treat high-tide
contiguous budget probing as exhausted at `128 KB`.
The planner-targeted `140 KB` point also failed with stable layout, regressing
`loop_vb 2094 -> 2095` and `blocking_vb 16 -> 19`. This confirms that the next
FISHING3 high win is not another contiguous prime size; it needs segmented
coverage or scheduler changes.
The scene-specific `2` VBlank refill guard is rejected too: it increased
visible CD pressure to `blocking_vb=23` and moved layout. Short-slack reads
remain unsafe without a real ownership budget.
Forcing fishing3 high back to catch-up threshold `5` is rejected as well:
blocking stayed flat while loop/refill/layout worsened. Keep setup-primed
threshold `4` until a scheduler can account for CD and catch-up ownership
together.
A fishing3 high read-group retry for relative sectors `223..234` is rejected:
the host CD log made it look like the safest local group, but the source-table
change moved PS-EXE/LBA and regressed `loop_vb 2099 -> 2103` plus
`blocking_vb 24 -> 28`. Future read groups must be generated and layout-held;
one-off hot source tables are exhausted for fishing3.
The first segmented setup-prime probe is accepted, but only as a narrow proof:
FISHING3 high relative sectors `67..73` read into scratch during setup moves
`loop_vb 2094 -> 2093` with stable layout and low tide exact-flat. It does not
lower `blocking_vb` yet, and it costs one setup read, so the next version needs
generated segment metadata or inter-scene preload that can target multiple
ranges without adding more hard-coded hot source logic.
Fishing3 low tide now has a second promoted retained read group, `{163,175}`,
after the accepted `{159,171}` group. It improves `loop_vb 2093 -> 2092`,
`target_vb 1952 -> 1954`, `overrun_vb 141 -> 138`,
`blocking_vb/prefetch_overrun_vb 11 -> 7`, and `loop_reads 33 -> 32` with
stable PS-EXE bucket and pack LBA. This confirms medium-later, start-aligned,
slack-safe read groups can still pay. Next FISHING3 low group candidates should
come from the cap-aware planner and avoid tight early clusters such as
`174..186` unless the generated cost model predicts lower loop/blocking, not
just fewer reads.
The wider FISHING3 low `{253,269}` follow-up is rejected even though it saved
two reads (`loop_reads 32 -> 30`): it regressed `loop_vb 2092 -> 2093` and
`blocking_vb/prefetch_overrun_vb 7 -> 8` with layout fixed. Keep this exact
range off the queue. If the `253` cluster is retried, test the narrower
`253..265` shape or generated metadata with a visible-cost model, not another
larger read-count-only group.
The narrower `{253,265}` retry is accepted. It keeps `loop_vb=2092` and
`overrun_vb=138` while improving `blocking_vb/prefetch_overrun_vb 7 -> 6` and
`loop_reads 32 -> 31`; all seven canaries stay flat. This is a useful pattern:
when a broad group saves more reads but adds visible pressure, split it down to
the smallest range that removes one read cleanly before trying any adjacent
cluster.
The FISHING3 high `{79,95}` medium-phase candidate is rejected as exact-flat:
it preserved `2096/1952`, `blocking_vb=18`, `prefetch_overrun_vb=13`, and
`loop_reads=41` while shifting hot symbols by `+4` bytes. Do not keep
hand-coding high-tide groups from estimated slack alone; this route now needs a
generated cost model that predicts an actual key-metric change.
A current-baseline retry of the older FISHING3 high `{246,258}` candidate is
also rejected. Under `compact-fgp3-v65-building4low-window36` it still saved
one nominal read (`loop_reads 41 -> 40`) but regressed all key pressure metrics:
`loop_vb 2096 -> 2100`, `blocking_vb 18 -> 19`, and
`prefetch_overrun_vb 13 -> 14`. Treat this exact range as a do-not-retry
standalone sector group; it belongs only in a generated setup/preload or
scheduler-owned experiment that changes cadence more deeply.
VISITOR3 low `{182,194}` is rejected for the same reason at larger scale. It
lowered hidden refill (`prefetch_overrun_vb 83 -> 77`) and one loop read, but
regressed active playback (`loop_vb 1532 -> 1539`, `blocking_vb 314 -> 324`)
and added one due miss. VISITOR3 low should move to generated cost/coverage
metadata or scheduler ownership before more one-off group appends.

Acceptance rule: use the exact fishing1 headless gate first. Promote only if a
key VBlank metric improves without regressing `blocking_vb`,
`prefetch_overrun_vb`, work identity, or correctness. Layout identity remains
mandatory for code-only experiments; deliberate pack-format experiments may use
`--allow-layout-change` only when the layout movement is documented and the
speed/work win is otherwise clean. Flat timing plus meaningful code-size/work
reduction is acceptable, but must not be counted as a speed win.

## Highest-Leverage Thesis

The remaining gap is not one single bottleneck. The current loop has visible
CD mostly contained to one VBlank on the setup-primed high-tide path, but that
came by moving read work into setup. The best path is parallel pressure on six
fronts:

| Front | Why It Can Still Move |
|---|---|
| Setup-prime/inter-scene preloading | `320 KB` priming proves residency can unlock catch-up; the setup cost must be hidden or generated per scene/tide. |
| CD grouping and read-cost prediction | Raw window sizes failed, but the CD log now shows zero-extra-sector group candidates. |
| Explicit render/CD slack scheduler | Several rejected variants were nominally faster but stole the slack hiding CD work. |
| Pack-emitted render/upload metadata | Runtime dirty/upload heuristics are locally exhausted; generated plans can remove branches. |
| Toolchain and layout control | Many valid cleanups regressed only because code/CD phase shifted. That is a solvable build problem. |
| Separate release/perf-log baselines | Perf logging is now part of the optimized path; release-speed measurements may expose free headroom. |

## Fresh Targets From The Latest Misses

The late 2026-04-26 wave ruled out more blind whole-TU compiler probing. Both
hot `-O3` attempts expanded executable layout, moved FG2 placement, and raised
visible CD pressure. The next useful tests should control phase first, then
retry promising source/toolchain ideas inside that controlled envelope.

The current speed binary reports FISHING1 high under target with only `2`
visible CD/refill VBlanks. Historical detail/trace builds showed large
present-wait ownership, but those counters are not compiled into the accepted
speed binary. The next major win now needs to reduce the matrix-wide offenders:
VISITOR3 CD/render ownership, BUILDING2 residual CD/due pressure, WALKSTUF1
validated-pack CD pressure, ACTIVITY9, and WALKSTUF3. Do this without early
display, tearing, frame drops, or weakened pause input.

| # | Target | Test Shape | Expected Signal |
|---:|---|---|---|
| 101 | Pure CD phase sweep | Insert `0..8` dummy sectors after `JCREBORN.EXE` with no source change. | Partially tested at `+1`, `+2`, `+3`, `+4`, and `+8`: exact timing-flat, so FG2 LBA alone is not the current speed lever. |
| 102 | Pure executable bucket sweep | Add inert text/data padding to keep FG2 LBA fixed while changing EXE bucket. | `+2 KB` was exact flat timing; use padding as a control tool, not a standalone speed target. |
| 103 | Hot-symbol address sweep | Pad before/after `foreground_pilot.o`, `cdrom_ps1.o`, and `graphics_ps1.o` independently. | Single-function `fgRuntimeFillWindowForEntry` positive shifts were flat; broader object/order sweeps are still untested. |
| 104 | Link-order sweep | Move `cdrom_ps1.o` before and after foreground/graphics without changing code. | Tests instruction locality and branch/cache phase as a first-class variable. |
| 105 | Function alignment sweep | Try 4/8/16/32-byte alignment for only CD/foreground hot functions. | Finds low-cost address buckets without whole-TU codegen changes. |
| 106 | Cold-section ballast | Keep cold `-Os` size wins but add deterministic padding to preserve the accepted EXE sector bucket. | Unlocks prior size wins without changing playback cadence. |
| 107 | Release-libs phase harness | Retest Release SDK libraries across CD and text phase pads. | Determines if the `1220` loop-VBlank signal can survive `blocking_vb<=5`. |
| 108 | Fifth-read locator | Host-side script maps the extra blocking read to LBA/file-sector timing candidates and covered entries. | Done for file-sector/CD-log comparison; frame/slack ownership still needs a trace binary or generated scheduler metadata. |
| 109 | Per-read slack class report | Bucket each read by held VBlanks available, sectors, preserved bytes, and overrun. | Delivered-sector parsing is done host-side; remaining gap is runtime slack ownership without perturbing the speed binary. |
| 110 | Group-fire trace build | Partly done host-side: read plans now mark whether each group start matches an observed runtime append start. A trace binary is still useful only if scheduler ownership changes. | Blocks no-op source probes like BUILDING4 `821..837` before emulator time. |
| 111 | Generated group metadata v2 | Emit group candidates beside FG2 entries without moving payload offsets. | Replaces hard-coded one-off group tables. |
| 112 | Selective two-group tail retry | Retry `384..396` only after group-fire tracing proves the append point. | Avoids increasing buffer capacity for groups that cannot execute. |
| 113 | Prepared-state detail counters | Done in Summary as `JCPERF2 sched`: prepared-ready/used/wasted plus CD-blocked prep and held-slice owners. | Fishing1 shows `72 ready / 72 used / 0 wasted`, so duplicate prepared-frame waste is not the current big win. |
| 114 | CD-first scheduler prototype | Refine from the first no-win ownership pass into a per-frame budget: read deadline, then precompose, then idle wait. | Prevents render prep from stealing CD slack while identifying which of the `574` wait slots and `28` CD-reserved slots can become useful work. |
| 115 | Read-deadline reservation | Reserve a minimum hidden-read budget before any speculative render prep. | Retests prepared-present ideas without creating extra visible reads. |
| 116 | No-source layout canary | Nightly/headless run checks that a rebuild of unchanged source preserves cadence. | Detects toolchain/container nondeterminism before optimization tests. |
| 117 | Perf-log off baseline | Capture release-speed metrics with logging disabled or minimized. | Quantifies how much of the remaining gap is diagnostic overhead. |
| 118 | Trace-binary split | Build a separate diagnostic executable so counters never perturb accepted speed binaries. | Allows high-detail metrics without invalidating timing. |
| 119 | Pack-local upload plans | Emit dirty/upload bands at pack generation time. | Removes runtime scan/merge logic instead of tuning it further. |
| 120 | Pack-local restore plans | Emit previous-frame restore bands and full-cover row masks. | Reduces restore work without runtime intersection checks. |
| 121 | Generated compositor classes | Group spans by alignment/length class offline. | Enables branch-light PAL4 composition without whole-TU `-O3`. |
| 122 | CD helper assembly microbench | Hand-code only the sector math/copy inner helper, preserving C call shape. | Tests runtime benefit without compiler expanding the whole TU. |
| 123 | Controller-poll release probe | Measure pause/input polling cost only in release/perf-off mode. | Avoids optimizing pad paths around perf-log noise. |
| 124 | ISO ordering probe | Move inactive resource/SND trees after active FG packs in a scratch layout. | Tests whether active scene adjacency can lower seek/read variability. |
| 125 | Cross-scene phase sample | Run the phase winner against fishing2/fishing3 before promotion. | Prevents a fishing1-only CD layout win from hurting the next validated scenes. |
| 126 | Present wait map | Emit trace-only frame classes: normal render, prepared-present, crossed-restore, crossed-compose, crossed-upload. | Shows which frames actually pay the full one-VBlank present wait. |
| 127 | Prepared upload feasibility proof | Analyze VRAM layout and active display area to prove whether any offscreen partial-buffer strategy can exist. | Blocks unsafe "upload early" ideas unless VRAM memory proves them possible. |
| 128 | Dirty-band offscreen staging | Prototype staging only dirty upload bands into unused VRAM, then copy during VBlank. | Could trade CPU/VRAM for shorter visible upload work. |
| 129 | Per-band VBlank deadline ordering | Sort upload bands by scanline/display risk instead of current tile order. | May reduce visible risk if any partial pre-VBlank upload is considered. |
| 130 | Prepared frame dual-RAM background | Keep two RAM composited backgrounds for current and prepared frames if heap allows. | Removes restore/compose from due frame without reusing mutable current state. |
| 131 | Dirty-row copy-on-write prepared RAM | Store only prepared dirty rows instead of full second background. | Lower memory version of dual-RAM background. |
| 132 | Prepared dirty-band delta buffer | Encode prepared frame as dirty row deltas to apply quickly at due time. | Moves compose cost out of present path without full-frame RAM. |
| 133 | VBlank upload budget counter | Trace how many upload bytes/rects fit inside one VBlank on target. | Separates mandatory wait from upload overrun. |
| 134 | Present wait skip proof gate | Add a diagnostic-only guard that proves a VBlank was already reached before upload. | Prevents unsafe skip experiments from being promoted blindly. |
| 135 | Display-page feasibility | Audit whether any lower-resolution/dithered page flip mode can preserve pixels. | Likely no, but it must be proven before dismissing page flipping. |
| 136 | Interlaced-field split upload | Test whether top/bottom field timing can safely split uploads. | Could reduce full-frame present wait if field safety is exploitable. |
| 137 | Pre-VBlank restore scheduling | Move only RAM restore earlier under a CD-first budget. | Reduces due-frame CPU before VSync without touching display early. |
| 138 | Pre-VBlank compose scheduling | Move only PAL4 compose earlier after restore has proven safe. | Extends prepared work while watching duplicate prep and CD starvation. |
| 139 | Prepared upload no-op class | Identify frames where next upload bands are identical to current framebuffer. | Those frames might advance without an upload. |
| 140 | Host timing hold rebalance | Recompute hold distribution to absorb known one-VBlank present latency without dropping entries. | More principled version of long-hold catch-up. |
| 141 | Sound-safe timing rebalance | Verify any hold rebalance against sound event cursor and late counters. | Prevents speed wins from desyncing the now-working sound path. |
| 142 | Per-scene present budget metadata | Emit expected present cost per frame in FG2 metadata. | Lets the scheduler choose where to spend catch-up safely. |
| 143 | Frame-class-specific catch-up | Apply catch-up only after expensive rendered frames and only when next CD is resident. | More targeted than threshold-only catch-up. |
| 144 | Present/input split | Poll Start on held frames and after rendered frames through an explicit cadence table. | Allows input cleanup without removing necessary polling. |
| 145 | Prepared-present state machine v3 | Make prepared visual, staged payload, and future window ownership explicit states. | Required before retrying decoupled prepared frames. |
| 146 | VRAM copy primitive benchmark | Measure `MoveImage`/GPU copy cost versus `LoadImage` for dirty bands. | Determines whether offscreen staging can be cheaper than CPU upload. |
| 147 | Upload command prebuild | Precompute `RECT`/pointer command data for prepared dirty bands. | Removes setup overhead on the due-frame upload path. |
| 148 | Pack-emitted present bands | Emit exactly the bands needed for prepared upload at pack time. | Replaces runtime dirty scan in the present path. |
| 149 | Cross-scene present histogram | Run detail attribution on fishing2/fishing3 and later all scenes. | Confirms whether present wait dominates beyond fishing1. |
| 150 | Visual signoff harness for present experiments | Capture stills/video around any present-wait change before promotion to main. | Present optimizations can pass counters while tearing visually, so they need extra signoff. |
| 151 | Generated setup-prime planner | Analyze every scene/tide pack for the smallest setup-prime byte window that covers useful early payloads. | Generalizes the fishing1 `320 KB` win without hard-coding one scene. |
| 152 | Inter-scene prime handoff | Start reading the next scene's prime window during the previous scene's dead/held tail when scene selection is known. | Converts the current setup-cost trade into a real end-to-end speed win. |
| 153 | Segmented prime window | Prime only the hot early and late FG2 spans instead of one contiguous first `N` bytes. | May reduce setup bytes while preserving the catch-up-safe coverage boundary. |
| 154 | Setup-prime low-tide sweep | Test low-tide fishing1 with generated prime sizes and keep threshold `5` unless coverage proves threshold `4` safe. | Prevents high-tide assumptions from leaking into the low-tide path. |
| 155 | Prime-size heap budget table | Emit per-scene largest-safe prime size after clean-rect allocation and runtime buffers. | Avoids memory regressions before trying all 63 scenes. |
| 156 | Prime-aware catch-up table | Emit frame ranges where threshold `4` is safe because all needed payload bytes are already resident. | Replaces the current scene-global setup-primed catch-up with frame-level proof. |
| 157 | Setup-cost gate | Add a harness mode that reports active-loop win, setup cost, and net scene cost separately. | Stops future preloading wins from accidentally hiding startup regressions. |
| 158 | Prime prefetch during title/menu | Investigate whether menu/transition time can warm the first scene's FG2 window before playback starts. | Converts cold-start setup reads into user-invisible work. |
| 159 | Cross-scene setup-prime matrix | Run fishing1/fishing2/fishing3 high/low with generated prime settings before main promotion. | Ensures the policy is not a fishing1-only trick. |
| 160 | Prime-plus-present scheduler | Use primed coverage to retry present/pipeline scheduling only inside proven-resident frame ranges. | Combines the current CD residency win with the remaining present-wait target. |
| 161 | FGP3 zero-shift residual pack | Encode fishing1 frames as previous-frame residuals where `dx=0,dy=0`. | Analyzer predicts this is the canary-safe temporal reuse path, unlike true translation. |
| 162 | FGP3 move/residual pack | Encode nonzero translation candidates only for walking scenes. | Analyzer proves walking packs, not fishing1, are the first real MoveImage targets. |
| 163 | Motion cleanup masks | Emit old-position cleanup bands for move/residual frames. | GPU move is unsafe unless old pixels are restored and dirty state remains exact. |
| 164 | RAM mirror motion proof | Prototype host-side replay that keeps RAM mirror and displayed image identical after move/residual frames. | Blocks runtime MoveImage until the mirror invariant is solved. |
| 165 | VISITOR3 precomposed x-band payload | Generate upload-ready bands from the final clean-plus-foreground composite, not foreground-only spans. | The v140 budget fits `74-75` frames but raw pixels are unsafe because background-owned pixels are required. |
| 166 | VISITOR3 background ownership mask | Emit per-band ownership bits that prove which pixels come from clean background, cleanup restore, or current draw. | Lets upload-ready data include only pixels that are deterministic for tide/night/holiday state. |
| 167 | VISITOR3 x-band compression probe | Compress selected precomposed bands with row-local RLE or nibble-delta coding and size-gate against current slack. | Current uncompressed budget barely fits only because payload was trimmed; compression can leave metadata safety margin. |
| 168 | VISITOR3 cap-hit frame split | Keep cap-hit frames on full-width upload and emit precomposed bands only for non-cap selected ranges. | Avoids repeating rect-cap failures around frames `128..130` and `141..142`. |
| 169 | VISITOR3 late-cluster scheduler sidecar | Generate a tiny per-entry read-deadline sidecar for the late `315..331` / `333..349` class instead of C tables. | The ranges are useful only when the scheduler owns timing and can avoid visible-gap theft. |
| 170 | VISITOR3 hidden-budget simulator | Replay held-frame slack, staged reads, and prepared-present work from perf logs to choose generated groups before a PS1 run. | Blocks source probes where saved reads convert directly into hidden refill debt. |
| 171 | VISITOR3 layout-moving upload experiment | Deliberately allow the upload-ready append to grow the pack and run full canaries with fixed documented LBA movement. | Same-footprint constraints may be more expensive than a measured layout-moving experiment. |
| 172 | VISITOR3 compact upload rect table | Store upload rect metadata as per-frame deltas and shared band templates. | Rect metadata, not just pixels, consumes the tight same-footprint budget. |
| 173 | VISITOR3 frame-127 tail isolation | Test a targeted precomposed or scheduler sidecar for the high-value frame-127/126/125 upload hotspot only. | The top few frames carry disproportionate modeled upload savings and may fit as a smaller proof. |
| 174 | VISITOR3 background-state keying | Key precomposed payloads by tide/night/holiday/island state and prove VISITOR3 only uses the matching background state. | Prevents baking dynamic ocean or holiday pixels into an unsafe foreground pack. |
| 175 | VISITOR3 setup-prime segmented coverage | Prime selected late sectors rather than raising contiguous high/low caps. | `256 KiB` high regressed despite saving a read, so contiguous prime phase is the problem. |
| 176 | VISITOR3 tail-cluster deferral | For late tight clusters, test extending the previous long hold only when the next payload is already resident. | Attempts to absorb read timing without changing scene cadence globally. |
| 177 | VISITOR3 payload order planner | Reorder payload bodies inside the existing padded pack while preserving entry offsets through an indirection table. | Duplicate read clusters may be layout-driven; current direct offset order is fragile. |
| 178 | VISITOR3 safe no-op cadence replacement | Replace removed visual-work entries with explicit hold metadata plus generated refill reservations. | The old no-op prune speed signal failed because cadence shortened and stole refill slack. |
| 179 | VISITOR3 compact parser split | Restore the accepted packed compositor byte-for-byte and add any new compact/upload parser in a separate cold path. | Prior compact data wins failed when shared PAL4 hot code moved. |
| 180 | WALKSTUF1 safe precomposed encoder | Build a background-owned/precomposed payload source before upload-ready append work. | The compact pack now has `611305` bytes of slack, but raw foreground-only upload bytes are still unsafe. |
| 181 | WALKSTUF1 direct-stage frame policy | Generate per-frame direct-stage caps instead of changing the global `8 KiB` threshold. | Scalar caps showed the right blocking signal but too much hidden refill debt. |
| 182 | WALKSTUF1 late-cluster sidecar groups | Emit generated scheduler metadata for the top low/high clusters without adding hot C table branches. | The guarded hand table did not fire and still shifted phase. |
| 183 | WALKSTUF1 explicit layout-moving run | Permit a controlled pack-size/layout change for a precomposed or motion-data WALKSTUF1 variant. | Same-footprint budget exists after compaction, but safety metadata or compression may still need a measured LBA move. |
| 184 | BUILDING2 low hidden-refill owner trace | Add trace-only ownership for the hidden `5 -> 13` refill debt in the rejected low restore-minus-current transform. | The visible win is huge; the only blocker is hidden scheduler ownership. |
| 185 | BUILDING2 low dual-pass scheduler | First apply restore-minus-current, then generated refill reservations around the shortened render cadence. | The transform makes due frames shorter; the scheduler must move hidden reads into that new space. |
| 186 | BUILDING6 FGP2 zero-shift residual | Implement a PAL4 residual/keyframe encoder for BUILDING6 instead of converting wholesale to FGP3. | v146 motion analysis predicts `680717` compose bytes and `11701120` upload bytes saved in the zero-shift runtime model, while direct FGP3 expands. |
| 187 | BUILDING6 keyframe cadence sweep | Choose every-N full frames plus residuals from motion analysis and size-gate against the current pack. | v146 reports `242 / 305` candidate pairs and `64.85%` modeled pair-payload savings, enough to justify a generated format. |
| 188 | BUILDING6 generated window ownership | Use read-plan metadata to schedule early `15..39` coverage without a scalar `48 KiB` window. | The raw larger window saved reads but paid them visibly. |
| 189 | Cross-outlier safe-pixel analyzer | Extend draw-covered accounting to background-owned/precomposed feasibility across VISITOR3, WALKSTUF1, and BUILDING6. | The same unsafe-raw-upload lesson now blocks multiple top outliers. |
| 190 | Pack-side payload entropy matrix | Report per-scene compressibility for active payload, upload bands, and rect metadata. | Decides whether compression, layout movement, or scheduler metadata is the best next path per outlier. |
| 191 | Hot-code budget ledger | Track remaining bytes in the `215040` PS-EXE bucket and which cold removals can fund each new parser. | Many valid ideas fail simply by crossing a sector bucket or moving hot symbols. |
| 192 | Layout-pinned failure replay | For each major rejected code-shape win, rerun with explicit EXE padding and FG LBA pinning before closing permanently. | Separates true codegen regressions from CD-layout phase regressions. |
| 193 | Long-run memory-pressure telemetry | Log heap/free-largest/scene index to `scratch` during 1+ hour scene cycling. | Recent long runs likely die after `10-15` scenes from memory pressure; the optimization loop needs durable crash evidence. |
| 194 | Outlier rotation gate | When VISITOR3 has no safe immediate lane, automatically rotate to WALKSTUF1, BUILDING2 low, and BUILDING6 while preserving VISITOR3 ideas. | Prevents the headless path from stalling on one scene after scalar/source lanes are exhausted. |
| 195 | Generated FG scheduler sidecar v1 | Emit a compact cold sidecar of per-entry read deadlines, slack class, and max safe append size. | Replaces hot C range tables with data that can be packed or compressed without crossing the PS-EXE bucket. |
| 196 | Deadline-aware append simulator | Replay `JCPERF2` held-loop, due-miss, and blocking counters from summaries before a runtime probe. | VISITOR3 and BUILDING6 both show saved reads can still worsen visible cadence. |
| 197 | Scene-local refill reservation map | Reserve VBlank ranges for CD refill before shortening compose work with data-shape transforms. | BUILDING2 low restore-minus-current failed because the faster renderer starved presentation/refill timing. |
| 198 | Precomposed band ownership builder | Generate upload bands from final clean background plus foreground, with tide/night/holiday keying. | VISITOR3 and WALKSTUF1 raw foreground-only bands fail because selected pixels are background-owned. |
| 199 | Background-state hash gate | Store a hash/key for any precomposed payload and reject it at runtime if scene state differs. | Prevents baking ocean/holiday/night pixels into reusable foreground packs without proof. |
| 200 | Compact ownership bitmap RLE | Encode per-band ownership as row-local runs rather than per-pixel masks. | Safe precomposed upload needs ownership proof without consuming the entire zero-tail budget. |
| 201 | Upload-band entropy codec matrix | Compare row RLE, x-delta, nibble plane, and LZ-style codecs on VISITOR3/WALKSTUF1/BUILDING6 selected bands. | The next payload format should be chosen from measured compressed sizes, not intuition. |
| 202 | Rect-template dictionary | Deduplicate repeated upload rect layouts across selected frames. | VISITOR3 and WALKSTUF1 rect metadata consumes meaningful same-footprint budget. |
| 203 | Per-frame rect cap planner | Generate frame-specific upload rect caps and fallback full-width rows for cap-hit frames. | Avoids global dirty rect cap tweaks that were exact-flat or too costly. |
| 204 | CD-layout padded sandbox | Add an automated branch mode that pads the PS-EXE and foreground files to hold LBAs fixed while testing code-shape ideas. | Many probes failed because code size shifted layout before the algorithm could be judged. |
| 205 | Hot-symbol drift budget report | Emit a per-run diff of hot function sizes and addresses into the summary JSON. | Makes code-shape failures actionable without manually checking maps after every probe. |
| 206 | Scratch log compactor | Summarize each long run's scene index, RSS, heap, largest free block, and last completed frame into a CSV. | Long-run crash diagnosis needs parseable breadcrumbs, not only terminal output. |
| 207 | FG pack zero-tail ledger | Track zero-tail slack, active payload, and last nonzero byte for every foreground pack after each accepted commit. | Prevents repeating stale slack assumptions like the pre-compact WALKSTUF1 `1` byte result. |
| 208 | Motion mirror invariant harness | Host-replay MoveImage/residual candidates against the RAM background mirror and dirty state. | BUILDING6 and WALKSTUF1 motion signals are unusable until mirror correctness is proved. |
| 209 | Zero-shift residual FGP2 extension | Add a minimal PAL4 residual entry type for zero-shift frames before true translation. | Zero-shift avoids old/new position cleanup complexity while capturing a large share of motion savings. |
| 210 | Nonzero motion cleanup prototype | Emit old-position cleanup plus new residual upload for a tiny BUILDING6 candidate range. | Tests the hardest MoveImage invariant on a bounded high-value pair before full format work. |
| 211 | Motion keyframe interval search | Choose keyframes by payload, upload rows, and due-miss risk instead of fixed every-N cadence. | The best motion frames are clustered; fixed cadence may waste pack budget. |
| 212 | BUILDING6 explicit layout-moving motion pack | Allow a measured pack growth for one BUILDING6 tide if zero-shift residual cannot fit in-place. | The v146 signal is large enough that a controlled LBA move may still net positive. |
| 213 | BUILDING2 low refill-placement sidecar | Generate a tide-specific refill schedule for the restore-minus-current candidate, then replay it before emulator. | This targets the exact `blocking_vb 118 -> 180` / `due_misses 22 -> 43` failure. |
| 214 | BUILDING2 restore-window hybrid | Combine restore-minus-current with smaller staged chunks rather than delaying all window work with a slack guard. | v144 proved a local guard fixes hidden debt but starves visible cadence. |
| 215 | VISITOR3 yacht-frame micro-target | Build a one- or two-frame precomposed/scheduler sidecar around the highest upload/read hotspots only. | A tiny proof may promote where full selective upload is too format-heavy. |
| 216 | VISITOR3 read-cluster cold metadata | Move `315..331` / `333..349` class data into a packed cold blob decoded only at scene start. | Keeps generated scheduling without growing hot `foregroundPilotPlay` branches. |
| 217 | VISITOR3 late-loop hold retimer | Test generated hold retiming only when the next payload is already resident. | Attempts to absorb late clusters without shortening cadence or increasing due misses. |
| 218 | WALKSTUF1 precomposed safety micro-target | Pick the top few budgeted v145 frames and generate owned/precomposed bands only for those frames. | Proves the safety path before consuming the full `609192` byte budget. |
| 219 | WALKSTUF1 motion-vs-upload comparator | Compare compact precomposed upload, zero-shift residual, and scheduler sidecar on the same top frames. | Prevents building the wrong generated format for a scene with multiple signals. |
| 220 | Cross-scene format selector | Classify each outlier as scheduler-owned, precomposed-upload-owned, zero-shift-motion-owned, or layout-moving. | The remaining outliers no longer share one scalar fix lane. |
| 221 | Generated metadata compression fuzzer | Randomize sidecar encodings and score bytes, decode cost, and hot-symbol impact. | Keeps metadata out of the PS-EXE bucket while preserving runtime simplicity. |
| 222 | Broad-canary auto-minimizer | When a candidate fails broad controls, bisect canaries and metrics to isolate layout drift vs true runtime coupling. | Speeds up log-only determinations and avoids rerunning irrelevant scenes. |
| 223 | Baseline promotion ledger script | Automatically update README/site/CSV rollups and total improvement text from the accepted summary. | Reduces manual drift after every successful headless commit. |
| 224 | No-runtime determination template | Generate experiment-log rows from scratch artifacts for failed size/safety gates. | Makes headless logging faster when a candidate is blocked before emulator time. |

## Impact-Prioritized Order

| Priority | Area | Tests | Why First |
|---:|---|---|---|
| 1 | Setup-prime and inter-scene preload | `151-160` | The latest accepted win proves residency unlocks threshold-`4` catch-up; the next step is making the prime free or generated, not hard-coded. |
| 2 | Temporal/motion FGP3 | `161-164` | The I-B analyzer shows fishing1 has a large zero-shift residual path, while true nonzero motion belongs to walking scenes. |
| 3 | Pack-emitted/read-costed CD groups | `11-18`, `34-35` | The hard-coded tail group proves selective grouping can remove reads, while the broad 12-sector import proves a cost predictor is mandatory. |
| 4 | Pack-emitted render/upload metadata | `61-64`, `81-90` | Runtime upload/compositor heuristics are locally exhausted; generated metadata can remove branches and preserve deterministic work identity. |
| 5 | Explicit scheduler/CD budget | `41-50` | Many near-misses were nominal wins that stole CD slack. A CD-first budget is the gate for retrying them safely. |
| 6 | Toolchain/layout control | `91-100` | Valid code-size cleanups still perturb hot phase. Layout control can unlock old no-promotion wins without changing pixels. |

## Next 100 Tests

| # | Area | Test | Why Now | Promote If |
|---:|---|---|---|---|
| 1 | Baseline | Add a release-speed run without `perf-log` and compare against the perf-log baseline. | We may be optimizing the logging build instead of the shipped runtime. | Release run is faster and remains visually/sound identical, creating a separate target baseline. |
| 2 | Baseline | Add a dual-baseline policy: diagnostic baseline and release baseline. | Future changes should be judged against the right runtime mode. | Harness records both without weakening the strict perf-log gate. |
| 3 | Harness | Add a non-promotable trace binary for per-read frame/slack class logging. | Inline CD histograms perturb the speed binary. | Trace build explains the remaining visible read and setup-prime ownership without changing the accepted binary. |
| 4 | Harness | Add host-side correlation from DuckStation read timestamps to frame indices. | Current CD log has LBAs but not the exact runtime frame owner. | We can prove the FGP3 zero-pressure path stays zero and name any setup-prime or cross-scene read that reintroduces pressure. |
| 5 | Harness | Add optional full-frame hashes to reject sequential-CD false positives. | Skipping `Setloc` once looked fast but collapsed visual work. | The gate can safely retest lower-level CD continuation ideas. |
| 6 | Harness | Gate `scene_vb` alongside `loop_vb` for setup-shift experiments. | Done: `scripts/ps1-perf-iterate.sh` now baseline-gates `timing.scene_vb` and prints it in `vs baseline`. | Future setup shifts cannot fake active-loop wins; setup-prime/preload probes must preserve full-scene cost as well as active-loop cost. |
| 7 | Harness | Add map-address tolerance bands for hot functions. | We now know fixed EXE/LBA can still regress from code-address phase. | We can identify which address shifts are dangerous. |
| 8 | Harness | Add automated compiler-flag matrix runner that logs no-promotion outcomes. | Toolchain work needs lots of controlled probes. | Matrix results are searchable and never dirty the accepted branch on failure. |
| 9 | Harness | Add cross-scene smoke subset after every accepted fishing1 speed win. | Fishing1 knees may not hold for fishing2/fishing3. | Fishing2/fishing3 exact cases stay within agreed pressure gates. |
| 10 | Harness | Add a "fifth visible read" summary to `ps1-perf-cdlog-summary.py`. | Current failures often move `blocking_reads 4 -> 5`. | Done: `--compare` reports `JCPERF2` deltas and file-sector-normalized timing candidates without touching the PS1 binary. |
| 11 | CD | Implement runtime lookup for 12-sector zero-extra-sector read groups. | Broad import failed, but the tail-only group `396..406` is accepted; next step is costed/selective groups, not blanket planner import. | `loop_reads`, `loop_read_vb`, or `blocking_vb` falls with no new visible read. |
| 12 | CD | Implement 16-sector group lookup behind a stricter slack guard. | Planner shows `69 -> 29`; raw 24 KB failed, but exact groups may not. | Read count falls and `blocking_vb` stays `<=5`. |
| 13 | CD | Implement 24-sector group lookup only for proven late long-hold regions. | Planner shows `69 -> 20`; broad high-slack reads failed. | Late-sequence reads drop without `prefetch_overrun_vb` growth. |
| 14 | CD | Target the remaining late groups around LBAs `748`, `755`, `762`, `769`, `801`. | The tail group already removed one late read; continue only where the cost model says the append fits held slack. | That cluster loses at least one more read or hidden VBlank. |
| 15 | CD | Target only group `file_sector 22..34` from the 12-sector plan. | Early hard-coded group `106..117` did not fire, so new groups need append-start proof first. | No timing regression and read count drops by one. |
| 16 | CD | Add append-cost predictor using `appendBytes`, `preserveBytes`, delivered sector count, and slack. | Host-side delivered-sector parsing now gives actual read spans; runtime slack ownership is the missing input. | Predictor blocks variants that would create the fifth visible read. |
| 17 | CD | Add group-cost predictor from measured host read durations. | Same sector count can have different elapsed cost. | Group selection correlates with lower `loop_read_vb`. |
| 18 | CD | Prefer group reads only when current window tail preservation is zero-copy. | `memmove` is useful but may be expensive at the wrong time. | Fewer reads without larger `used_vb`. |
| 19 | CD | Retry smaller raw windows after group metadata exists. | Old `14/15 KB` windows starved due frames because coverage was blind. | `prefetch_overrun_vb` falls without due misses. |
| 20 | CD | Retry `direct-stage` caps after group metadata exists. | `8 KB` is the current knee, but groups can change coverage cost. | A lower cap reduces visible pressure or a higher cap reduces loop time safely. |
| 21 | CD | Implement group-fed second stage slot. | Isolated second slot caused due misses; grouped coverage may fix starvation. | Stage hits increase and due misses stay zero. |
| 22 | CD | Implement group-fed prepared-wait prefetch. | Raw prepared-wait prefetch was only safe after enough scheduler cleanup. | Duplicate prep stays low and CD pressure stays flat or improves. |
| 23 | CD | Retry direct-stage read into window with group/tail preservation. | The prior `8 KB` version was two VBlanks faster but added a visible read. | Keeps `blocking_reads=4` while preserving the loop win. |
| 24 | CD | Add append-preserving direct-stage seed v2. | Scratch seeding is accepted; smarter merge may save the copy without churn. | `loop_vb` or `loop_read_vb` falls with no extra seek-back. |
| 25 | CD | Test window-only path after grouped reads. | `no-stage1` failed structurally before the current simpler pipeline. | Stage buffer can be removed or bypassed without due misses. |
| 26 | CD | Test stage-only path after grouped reads. | Window logic may now be overkill for small frames. | Smaller binary/work with exact timing or better. |
| 27 | CD | Test dual-window ping-pong refill. | Single-window append is useful but serializes preservation and read. | Hidden reads increase without heap or due-frame regressions. |
| 28 | CD | Retry true async refill with first-class state ownership. | Naive async failed because it was inline and under-instrumented. | Async reduces blocking without controller-state or correctness failures. |
| 29 | CD | Test `CdReadSync(0)` completion polling during held waits. | Could hide async completion without spin-waiting. | No visible pressure increase and no missed reads. |
| 30 | CD | Retest sequential read continuation with frame hashes and stronger work gates. | Prior result was invalid but the idea is high upside. | Setloc drops without any frame/work/hash mismatch. |
| 31 | CD | Test lower-level CD continuation only for already sequential aligned reads. | Current active sequence is mostly forward. | `setloc` falls and visual identity remains exact. |
| 32 | CD | Cache CD `CdlLOC` for next sequential sector. | Avoid repeated `CdIntToPos`/`CdPosToInt` cost in hot reads. | Helper symbol/time shrinks with flat or improved timing. |
| 33 | CD | Precompute pack file LBA once into runtime state. | Isolated helper cache was no-op, but combined CD rewrite may use it. | Smaller CD helper or lower `loop_read_vb`. |
| 34 | CD | Emit FG2 group sidecar metadata without moving payload offsets. | Sound-event prefix shift proved payload offsets are fragile. | Metadata loads in setup and active playback remains phase-identical or faster. |
| 35 | CD | Emit FGP3 grouped chunks for fishing1 only. | A one-scene experimental format can prove value before all scenes. | Read count drops without pack-size explosion or pixel change. |
| 36 | CD | Test group padding that preserves current payload sector crossings. | Global payload alignment doubled blocking. | Padding improves specific group boundaries without shifting hot payload phase. |
| 37 | CD | Test CD layout ordering around current fishing1 pack and active SCR/PSB files. | Removing an unused file regressed setup/CD phase even with same FG LBA. | Setup and active loop both stay flat or improve. |
| 38 | CD | Build a CD-phase pad searcher over 0..8 sectors after EXE. | Several size wins failed from phase. | Finds a phase bucket that lets valid size cleanups pass. |
| 39 | CD | Test exact `FISHING1.FG2` LBA shifts around `399`. | We know `398` is bad; other phases may be better. | A deliberate phase improves loop/CD metrics without source changes. |
| 40 | CD | Test sector-aligned metadata prefix sizes around 6 KB, 8 KB, 10 KB. | `4 KB` regressed active phase, but nearby sizes may improve setup safely. | Setup drops and active loop stays flat or improves. |
| 41 | Scheduler | Build explicit held-slice budget: CD first, render prep second, wait last. | Many render-prep wins stole CD slack. | Prepared work falls without `blocking_vb` growth. |
| 42 | Scheduler | Track prepared-frame "used vs wasted" counters in a trace build. | Duplicate prep currently acts as ballast. | We can delete only truly wasted prep. |
| 43 | Scheduler | Retry prepared-present threshold `3` with CD-first budget. | Prior threshold improved nominal target but hurt CD pressure. | `target_vb` improves and `blocking_vb` stays flat. |
| 44 | Scheduler | Retry prepared-present threshold `5` with group prefetch. | Threshold-only saved prep but regressed pressure. | Prep drops and groups preserve CD coverage. |
| 45 | Scheduler | Retry exact-use prepared-current RAM reuse. | It removed duplicate work but changed CD phase. | Work drops and timing stays flat or improves. |
| 46 | Scheduler | Retry prepared-buffer release with explicit frame cursor state. | Prior version had suspect final cursor and due misses. | Buffer ownership simplifies without due misses. |
| 47 | Scheduler | Retry staged-frame prep at `3` VBlanks after group predictor. | It improved nominal overrun but exposed one CD VBlank. | Actual `loop_vb` or target improves without visible CD pressure. |
| 48 | Scheduler | Retry long-hold catch-up threshold `4` after group predictor. | Threshold `4` improved some accounting but regressed pressure. | `overrun_vb` falls and `blocking_vb` stays flat. |
| 49 | Scheduler | Test short-hold catch-up only when next group is resident. | Coverage alone was insufficient before groups. | `target_vb` improves without read pressure. |
| 50 | Scheduler | Add scanline-safe present scheduler prototype. | Skipping pre-upload wait looked fast but unproven. | Reduces present wait with frame-hash visual proof. |
| 51 | Scheduler | Compose staged frame before VSync only when no CD read is eligible. | Prior compose-before-wait stole prefetch cadence. | `restore/compose` phase moves earlier without CD regression. |
| 52 | Scheduler | Split `eventsWaitTick(0)` into pause-poll-only and full event paths. | Pause-poll-only regressed as a local change; full scheduler may use it. | Held-loop wait cost falls with stable pause behavior. |
| 53 | Scheduler | Poll Start only on held waits with remaining slack above threshold. | Input polling cost is small but constant. | No pause regression and VBlank/work metrics improve. |
| 54 | Scheduler | Test controller poll cadence in release baseline, not perf-log baseline. | Pad/SPI diagnostics once hid real cost. | Release run improves without missed Start input. |
| 55 | Scheduler | Move sound-event firing to prepared/present boundary only. | Sound is correct now, but event timing may add hot-path work. | Sound counters stay exact and loop work falls. |
| 56 | Scheduler | Consume leading empty artifact with a stricter visual/hash proof. | It was accepted, but follow-up setup render only moved accounting. | Further empty handling reduces `scene_vb`, not just `loop_vb`. |
| 57 | Scheduler | Test first-frame setup render plus explicit setup/loop phase barrier. | Prior version kept `scene_vb` flat. | Full scene time improves and CD pressure stays flat. |
| 58 | Scheduler | Replace repeated `foregroundPilotRuntimeAdvance()` checks with state-specific loop bodies. | Hot loop still branches through several modes. | Code shape shrinks or `advances` cost falls. |
| 59 | Scheduler | Split FG2 scene loop from legacy/testcard loop entirely. | Legacy diagnostic scenes are compiled out but mode checks remain. | Hot loop/code shrinks without phase regression. |
| 60 | Scheduler | Generate a fishing1-specific loop policy table at startup. | Branch decisions are deterministic for a scene. | Replaces runtime conditionals with table lookups and passes exact gate. |
| 61 | Graphics | Emit pack-time dirty/upload bands per frame. | Runtime upload heuristics are locally exhausted. | `upload_rects` or CPU work falls without wider bytes. |
| 62 | Graphics | Emit pack-time restore bands per previous frame. | Restore scans still cost `2.51 MB`. | `restore_bytes` or restore work falls with no stale pixels. |
| 63 | Graphics | Emit full-cover row masks to skip restore for rows overwritten by current frame. | Some rows may be fully replaced by FG2 spans. | Restore calls/bytes drop and visual hash stays exact. |
| 64 | Graphics | Retry clean-rect intersection skip using pack-time masks. | Runtime intersection screen cost more than it saved. | Pack-driven skip avoids hot runtime math. |
| 65 | Graphics | Replace dirty row min/max clear loops with packed bitset generations. | `memset` regressed because layout changed, not necessarily because clearing is optimal. | Clear work shrinks and CD phase stays flat. |
| 66 | Graphics | Write a custom MIPS dirty-row clear fill. | libc `memset` changed code shape and regressed. | Same layout or better timing with smaller clear cost. |
| 67 | Graphics | Isolate `grDrawBackground()` upload path into its own TU. | Narrow locals shrank ELF but regressed scheduler shape. | Upload changes become testable without disturbing FG2/CD code. |
| 68 | Graphics | Retry narrow upload locals after TU isolation. | Prior miss may be register/layout coupling. | ELF/stack shrinks and timing stays flat. |
| 69 | Graphics | Test 16-pixel X-aware upload with one final `DrawSync`. | Per-strip sync failed structurally. | Upload bytes fall and scene reaches `JCPERF2`. |
| 70 | Graphics | Test bounded scratch arena for X-aware upload strips. | Previous scratch attempts exploded rects or syncs. | Bytes fall with bounded rect count and no heap leak. |
| 71 | Graphics | Test exact-width multi-row bands only above a width-savings threshold. | Single-row exact-width was no-op in fishing1. | Upload bytes fall without rect pressure crossing the zero-gap failure. |
| 72 | Graphics | Sweep upload gap `0/1/2/3` after each accepted scheduler/CD change. | The current 1-row knee may move. | Either bytes or rects improve with flat timing. |
| 73 | Graphics | Add deterministic rect-widening cap instead of full fallback. | Pixel-perfect requires no fallback, but widening is deterministic. | Cap hits are explainable and `full_fallbacks=0`. |
| 74 | Graphics | Sort upload bands to reduce GPU command overhead. | Current order is likely row order, not necessarily command-optimal. | `upload_vb`/loop falls without pixel difference. |
| 75 | Graphics | Batch `LoadImage` rect setup data in a persistent small array. | Rect count is high at `502`. | Stack/code shrinks or upload work falls. |
| 76 | Graphics | Test max upload rect cap `7` after cross-scene proof. | Fishing1 max is `6`; cap `6` had no win but no headroom. | Cap `7` shrinks or remains safe across fishing scenes. |
| 77 | Graphics | Cache clean-rect row source pointers. | Restore bytes are stable but pointer math may be hot. | Restore helper shrinks or `restore_vb` drops in detail runs. |
| 78 | Toolchain | Done: add the build-wide `-O2` audit report and candidate CSV. | `scripts/ps1-o2-audit.py` now identifies every current `-Os` override from `compile_commands.json`, maps hot symbols from `jcreborn.map`, and writes `performance-o2-audit.md/.csv`. | Next concrete probe is test `92`: graphics scoped-helper `-O2`, one helper at a time. |
| 79 | Graphics | Done/no promotion: tested halfword-edge plus word-body restore copy in C for `grCleanRectCopyIn`. | The helper kept timing exact-flat but grew `jcreborn.elf 791808 -> 793372` and `jcreborn.map 45253 -> 45327`. | Do not retry this exact local C row-copy shape; use generated restore metadata, finer restore counters, or switchable ASM after data-shape changes. |
| 80 | Graphics | Test persistent clean-rect buffer reuse across same tide/background. | Memory leak fixes release per scene, but some buffers may be safely persistent. | Setup/restore improves without long-run heap drift. |
| 81 | Compose | Generate pack-time tile-split spans. | Runtime cross-tile splitting regressed; pack-time can remove hot branches. | Compose work falls and spans remain exact. |
| 82 | Compose | Generate per-row destination offsets in FG2 payload. | Runtime computes tile/row address repeatedly. | Compositor code/time shrinks. |
| 83 | Compose | Generate span command classes by alignment and length. | Dynamic aligned pair store regressed from branching. | Fast path uses branch-free classes. |
| 84 | Compose | Partial/no promotion: PAL4 local aligned pair-store branch regressed BUILDING4 high; continue only with generated opaque/aligned command classes, pack/runtime-owned pair LUTs, or isolated indexed8 tests. | Feasibility research still points at data width and branch removal, but runtime branch selection alone hurt cadence. | Compose detail improves without CD pressure. |
| 85 | Compose | Test scratchpad palette for compose, then hand-written MIPS PAL4/transparent loops only if measured compose cost remains high. | Scratchpad can remove palette-load stalls with less risk than inline ASM; ASM stays last because visual failure surface is large. | Compose detail improves and exact visual output survives cross-scene validation. |
| 86 | Compose | Test PAL4 direct16 pack option for fishing1. | Pack size may grow, but CPU could fall sharply. | Loop improves enough to justify pack-size budget. |
| 87 | Compose | Test direct16 only for hot/large frames. | Avoid full pack-size explosion. | Worst frames get faster with bounded pack growth. |
| 88 | Compose | Test per-scene generated fishing1 compositor. | One validated scene can justify bespoke codegen. | Fishing1 loop improves and generated output remains auditable. |
| 89 | Compose | Test LUT-per-palette direct two-pixel writes in generated code. | Runtime LUT attempt was not enough. | Compose detail improves with no branch growth. |
| 90 | Compose | Test row-level span coalescing at pack time. | PAL4 four-pixel unroll was no-op alone. | Fewer commands/spans or lower compose detail. |
| 91 | Toolchain | Done/no promotion: tested `-O2` on hot/semi-hot currently `-Os` source files; do not retry whole-TU `foreground_pilot.c`, `jc_reborn.c`, `resource.c`, `sound_ps1.c`, or `events_ps1.c`. | `foreground_pilot.c -O2` failed structurally, while `jc_reborn.c`, `resource.c`, `sound_ps1.c`, and `events_ps1.c -O2` stayed exact-flat while growing ELF. | Retry only as function-scoped, split-TU, or layout-padded shapes; move the active queue back to C restore/compose and generated metadata. |
| 92 | Toolchain | Done/no promotion: confirm graphics whole-TU `-O2` is already active, then test scoped `-O2` on graphics helpers that are currently forced to `-Os`. | `grDrawBackground()` and `grUpdateDisplay()` scoped `-O2` both stayed exact-flat while growing code. Keep the scoped `-Os` attributes. | Searchable no-promotion evidence is recorded; continue the `-O2` sweep on hot/semi-hot `-Os` translation units. |
| 93 | Toolchain | Do not retry whole-TU `-O3` on `foreground_pilot.c`; only test function-scoped/generated shapes with map padding. | Prior foreground `-O3` grew `foregroundPilotPlay` by about `5 KB`, moved pack layout, and did not improve key timing. | Accepted only if loop improves without CD pressure and hot symbol growth is explained. |
| 94 | Toolchain | Do not retry whole-TU `-O3` on `cdrom_ps1.c`; use helper-scoped `-O2`/`-Os`, generated read metadata, or assembly only under map/layout gates. | CD helper `-O3` grew the executable and worsened visible CD pressure; the feasibility note supports narrower targets, not broad flags. | `loop_read_vb` or helper size improves without layout/cadence regression. |
| 95 | Toolchain | Run per-file `-Os` only on `foreground_pilot.c`. | Done: exact-flat timing/work with PS-EXE `149504 -> 145408` and ELF `739900 -> 727716`. | Keep as a size/code-shape win; do not count as VBlank speed. |
| 96 | Toolchain | Done/no promotion for SDK-supported `-G0`: keep `GPREL` / `-G8`. Only test `-G4` or `-G16` after adding a custom build harness. | `NOGPREL` / `-G0` grew the PS-EXE by `4096` bytes and regressed WALKSTUF1 high. | Lower priority than generated read metadata and upload-ready/direct16 work. |
| 97 | Toolchain | Sweep hot-function alignment: default, 4, 8, 16, 32 bytes. | A one-off 16-byte alignment of `fgRuntimeFillWindowForEntry()` was exact-flat, so any retry must be scripted across several hot foreground/CD functions. | A phase bucket improves loop or CD pressure with map-address evidence. |
| 98 | Toolchain | Link hot FG2/CD sections first. | Keep scheduler/CD code contiguous and stable while cold code changes. | Hot symbol addresses stabilize and timing improves or becomes less fragile. |
| 99 | Toolchain | Split cold menu/debug/caption code into a cold archive or section. | Valid cold-code cleanup currently perturbs hot phase. | Cold shrink passes exact gate. |
| 100 | Toolchain | Add deterministic text padding around hot functions, then retry failed valid cleanups. | Active-guard removals and diagnostic gates were semantically valid but phase-sensitive. | At least one old size win becomes timing-flat or faster. |

## First Execution Order

Run these first because they unlock many later tests or target known high-upside
near misses:

| Order | Test # | Reason |
|---:|---:|---|
| 1 | 78 | Done: `scripts/ps1-o2-audit.py` emits the current `-O2` candidate queue and map/symbol evidence. Re-run it after each build-system or optimization-flag change. |
| 2 | 92 | Done/no promotion: both scoped graphics helper `-O2` probes were measured and rejected. |
| 3 | 91 | Done/no promotion: the hot/semi-hot whole-TU `-O2` sweep is now recorded and should not be retried blindly. |
| 4 | 79 | Done/no promotion: C word-stride restore-row copy stayed timing-flat and grew artifacts. |
| 5 | 84 | PAL4 local pair-store branch is rejected. Continue the compose-width lane only through generated classes, pack/runtime-owned pair LUTs, or indexed8-only probes before MIPS ASM. |
| 6 | 85 | Done/no promotion: the per-compose scratchpad palette probe shrank code but regressed VISITOR3 low, so retry scratchpad only inside generated/assembly compositor ownership. |
| 7 | 10 | Done; use the host-side comparison output to target sector-specific CD/read-cost work. |
| 8 | 16 | Cost predictor needed before any more grouped-window or raw window-size probes. |
| 9 | 11 | Continue grouped-read runtime only through selective/costed boundaries; broad 12-sector import already failed, tail `396..406` and VISITOR3 high `72..84` are accepted. |
| 10 | 38 | Find a safe CD/code phase bucket for valid size cleanups. |

## 2026-05-06 Post-O2 Generated Optimization Queue

The current `-O2` audit queue is exhausted under the v0.8.0 FISHING1 canary.
The remaining useful work is generated metadata, data-shape changes, and
scheduler-owned timing. Use the refreshed baseline artifact
`scratch/ps1-perf-iterate/v080-current-fishing1-baseline/20260506-120547-3872759/summary.json`
for FISHING1 spot gates; do not compare new probes to the stale `1068/1074`
pre-v0.8.0 row.

| Priority | Experiment | Acceptance Signal |
|---:|---|---|
| 1 | Done first pass: every perf case records a current-baseline fingerprint with loop/target/blocking/prefetch, PS-EXE bucket, ELF bytes, FG pack LBA, and hot symbol sizes. | Baseline comparisons now label missing/dirty/different-commit baseline metadata before a false rejection/acceptance can be logged. |
| 2 | Done first pass: read-plan candidate rows now include concrete source, touched, and append-start read segments from actual CD logs, not just pack-sector overlap. | Proposed read groups can prove `append_start_fireable=true`, actual runtime ownership, and expected `group_hits>0` before source tables are touched. |
| 3 | Done first pass: read-plan output now classifies whether a candidate fits the current grouped-read runtime capacity or requires scheduler/larger-window metadata; the read-candidate matrix also closes rejected ranges using every current `scene_slug` from the validated scene matrix. | Runtime code size stops growing per experiment; groups can be enabled/disabled from pack metadata with exact FISHING1 canary layout, and stale direct probes cannot reappear just because a scene was missing from a hard-coded host-tool list. |
| 4 | Done first pass: read-plan output now assigns a visible-CD cost class from first-gap slack, internal-gap slack, overread sectors, partial-touch count, and seek direction. It also emits `scheduler_retry_class`, separating direct standalone candidates from rows that need scheduler-owned refill handling first. | Raw saved-read groups are sorted by visible-risk, preventing repeats of BUILDING4/WALKSTUF1 exact-flat or regressing hand groups. BUILDING2 low `538..550` is now correctly routed as `scheduler-owned-candidate`, and rejected FISHING3 low `253..269` is closed; the refreshed read-candidate matrix has zero standalone probes. |
| 5 | Next VISITOR3 pass: build scheduler-owned/generated append timing for additional balanced 16/24-sector candidates without more hot hand tables, or move to selective x-band/upload-ready preprocessing. | A read-group win must lower loop or blocking without increasing visible refill; scheduler counters explain why it fired. The current cleanup-compact plus guarded-group canary is `loop_vb=1406/1405`, `blocking_vb=293/301`, and `loop_read_vb=332/341`; high `111..127` is closed, low `106..122` is safe-but-inert, high `72..84` was removed as setup-covered dead weight, and remaining VISITOR3 local read-table probes should be treated as exhausted unless generated metadata can add them size-neutrally. |
| 6 | Generate per-scene/tide setup-prime segments from the current read-plan, capped by heap and active-loop payoff. | Setup primes are segmented and scene-local; no global cap raise or raw broad segment can regress VISITOR3/FISHING1 cadence. |
| 7 | Add selective upload-ready x-band preprocessing for VISITOR3 first, using the matrix's high score plus rect/cap pressure columns and the per-frame hotspot report. | Upload bytes and loop time drop without shifting the FG pack LBA, increasing loop reads, or triggering the current `3` x-band cap-hit warning; cap-hit frames `134..136` should stay on the current full-width path unless the runtime format can avoid the cap. Runtime scratch-packing is rejected; x-band rows need to be pre-contiguous in generated pack data. The default threshold plan is too large for same-footprint append, but the budgeted analyzer target selects `74 / 96` frames, uses `814184` of `814847` bytes, and retains `3858104` modeled upload bytes saved. Implement that smaller subset first, then try compression or deliberate layout movement only if needed. |
| 8 | Store upload-ready bands only when per-frame payload growth is under a generated threshold. | The direct16 lane avoids WALKSTUF1-style CD pressure where whole-pack expansion cancels compositor savings. |
| 9 | Compress upload-ready bands with a tiny pack-time RLE or residual opcode class. | VISITOR3/BUILDING2/BUILDING4 upload-byte savings survive without large sector growth. |
| 10 | Generate exact restore bands for dirty backdrop repair, separate from upload bands. | Restore bytes fall without hot runtime overlap tests or branch-heavy row walkers. |
| 11 | Done first pass: add rect-count and cap-pressure columns to the preprocessing matrix, not just byte volume. | Candidate upload plans now expose total x-band rects, cap hits, max rects, rects per frame, and exact interval counts before a runtime pack-format probe. |
| 12 | Done first pass: `scripts/analyze-fg2-preprocess-plans.py` now parses FGP3 cleanup/draw payloads and emits per-frame cap/saving hotspot reports, CSV export, pack zero-tail slack, and an exact budgeted subset for a payload ceiling. | VISITOR3 detail shows cap-hit frames `134..136` save `0%` under blanket x-band. The default CSV threshold plan selects `96 / 144` frames and estimates `6114568` selected-subset upload bytes saved, but the v093 footprint gate proves its `2462072` payload+rect bytes do not fit the current `814847` bytes of per-pack zero-tail slack. The v094 budgeted subset keeps the same-footprint path viable with `74` frames, `814184` payload+rect bytes, and `3858104` modeled upload bytes saved. |
| 13 | Try a pack/runtime-owned aligned PAL4 pair command class, replacing the failed runtime pair-store branch. | The hot compositor receives pre-aligned spans and avoids local branch/packing growth. |
| 14 | Generate same-palette direct16 only for indexed8 spans whose palette lookup cost dominates and whose CD expansion stays local. | WALKSTUF1-like indexed8 rows improve without whole-pack direct16 expansion. |
| 15 | Add a selective/keyframed FGP3 residual encoder for BUILDING6 and WALKSTUF1. | Current direct residual expansion becomes a shrinking or layout-stable data shape before benchmarking. |
| 16 | Build a per-scene memory-residency estimator for setup primes, preprocessed bands, and retained read windows. | Experiments fail fast when combined heap pressure would recreate scene-loader BSODs or retained-window regressions. |
| 17 | Add an automated all-canary baseline refresh command that records the artifact path into the experiment log. | Done: `scripts/ps1-perf-canary-baseline.sh` runs the standard canary set and writes `scratch/ps1-perf-iterate/latest-canary-baseline.txt` for same-commit probe gates. |
| 18 | Split cold boot/menu/debug functions into a layout-isolated section only after proving foreground LBAs stay fixed. | Valid size cleanups stop perturbing hot CD cadence or foreground pack placement. |
| 19 | Pad or lock foreground pack LBAs for code-shape experiments that are supposed to test CPU only. | Compiler/source experiments can separate code speed from CD layout movement. |
| 20 | Add per-scene sound-event timing deltas to the automatic promotion gate. | Speed wins remain safe for the validated scene corpus, not just pixels and loop VBlanks. |
| 21 | Add a generated hold-frame budget report so long holds can be used for safe setup/read work. | The scheduler spends known slack at scorecards/final holds without changing joke timing. |
| 22 | Add a "no-stitch/static scene" capture classification to avoid expensive generated stitching where host frames never leave the island. | Host generation time drops without changing validated scene assets. |
| 23 | Generate host-capture duration from observed scene loops instead of fixed overcapture multipliers. | Multi-loop scenes like ACTIVITY1 are complete without creating 4x redundant host packs. |
| 24 | Add pack-death/incomplete-generation detection to host capture and pack preprocessing. | A failed generation cannot silently produce truncated frames that later pass source build. |
| 25 | Add a random-run long-soak perf mode that samples scene IDs, loop metrics, heap, and BSOD signatures. | Performance commits get a quick broad stability gate before release merge-down. |
| 26 | Add a generated per-family optimization matrix: visitors, buildings, activities, fishing, walkstuff, stands. | Similar scenes share data-shape probes while still preserving scene-specific flags such as raft, tide, and holiday state. |
| 27 | Re-run the preprocessing opportunity matrix after every promoted runtime/data win. | Scores stay tied to the new baseline instead of chasing stale top candidates. |
| 28 | Add a "rejected but informative" artifact index keyed by experiment ID and source files touched. | The next agent can avoid repeating failed local threshold/branch/source-shape probes. |
| 29 | Promote host-only tooling improvements separately from runtime speed wins. | Better measurement and generation workflow can land without pretending to improve VBlank metrics. |
| 30 | When a binary choice is cheap, run both variants against the same fresh baseline before logging either. | The headless loop preserves momentum while still making a defensible binary decision. |

## Retest Rules For Old Failures

| Old Failure Class | Retry Only After |
|---|---|
| Raw larger windows | Group metadata plus cost predictor exists. |
| VISITOR3 raw stream windows and standalone groups | Do not retry scalar window sizes; fresh-baseline high/low sweeps failed. The old VISITOR3 high `72..84` row is now removed because setup-prime coverage already owns sectors `1..97`; use coverage checks before adding or keeping local rows. The post-recovery generated groups high `163..175` and low `158..170` both reported `group_hits=0`, with low regressing visible timing. The later high `170..186` larger-group probe is accepted against same-source canaries, but the `144..156` follow-up regressed `loop_vb 1455 -> 1459` and `blocking_vb 361 -> 367` despite saving one nominal read, and `102..118` regressed `1455 -> 1457` / `blocking_vb 361 -> 366` despite saving two nominal reads. After the accepted low `170..186` append, low `{126,132}` stayed exact-flat and low `{122,138}` regressed total loop despite saving reads, so tight low-tide standalone tables are closed too. The v0.8.1 larger-window extension of accepted high `{170,186}` to `{170,194}` also failed: high stayed `loop_vb=1450` while `blocking_vb 355 -> 356` and `prefetch_overrun_vb 14 -> 16`; do not extend that accepted table under the current runtime. The contained-large-entry staging probe proved duplicate VISITOR3 CD work exists (`loop_reads -1`, `due_misses -7` on both tides) but failed promotion because `loop_vb` stayed flat and hidden refill jumped to `prefetch_overrun_vb 75/79`; do not retry large resident-window staging without explicit scheduler budgeting. The VISITOR3-only strict x-band upload retry also regressed (`1455 -> 1461`) and grew `grDrawBackground` by `1084` bytes, so do not retry runtime scratch-packed x-aware upload. The promoted cleanup-metadata-only FGP3/v3 data shape improves both tides without moving LBAs or PS-EXE bucket, while the fuller compact draw+cleanup variant is rejected because the extra compositor code disturbed non-VISITOR canaries. The guarded high `144..160` append was superseded by `138..162` and removed; do not turn either shape into an unguarded standalone table. The low `{105,121}` guard with `minSlackVBlanks=6` and current low `{106,122}` guard with `minSlackVBlanks=8` both stayed exact-flat, while high `{104,120}` with `minSlackVBlanks=4` regressed `loop_vb 1406 -> 1410` and `blocking_vb 294 -> 300` despite saving two nominal reads, so all are closed as local guarded tables. Duplicate-payload table reuse is closed: one-read/byte wins regressed high/low cadence even with fixed layout. Broad offscreen clipping is closed as a both-tide transform: shrinking payload creates hidden refill overrun. The high-only data-size-preserving draw clip is accepted, the low-side retry proves only exit-right `139..143` is safe, the later scoped composite-helper `-Os` pass moves low to `1135/1024`, and the v4 draw-tail trim plus VISITOR3 stage guard is now promoted while standalone v7/runtime packed-draw shapes remain rejected. Continue VISITOR3 from the new `1118/1028` high and `1126/1025` low baseline with scene-local/generated metadata, pack data-shape preprocessing without runtime scratch packing, or prepared visual ownership; current-fit append ownership alone is not enough. |
| BUILDING2 raw stream windows | Do not retry scalar window sizes. High regressed all tested sizes, and low's parameter-only `32 KiB` win failed as compiled default source. The current low `603..619` group saved three nominal reads but regressed loop timing `1465 -> 1469` with fixed layout, so standalone tight clusters are closed. The cleaner low `538..550` group improved loop/blocking (`1465 -> 1461`, `334 -> 328`) but failed strict promotion because refill overrun regressed `35 -> 40`; keep it as a scheduler-owned retry candidate, not a standalone table. The cleanup-metadata FGP3/v3 data shape is now accepted for BUILDING2, so future BUILDING2 work should start from `1430/1289` high and `1429/1286` low, not the older FGP3/v1 baseline. Use scheduler-owned grouping or selective preprocessing instead. |
| BUILDING2 local min-slack grouped appends | Do not retry `{538,550}` with a local `minSlackVBlanks=6` guard. It was safe but exact-flat (`1465/1276`, `blocking_vb=334`, `prefetch_overrun_vb=35`) and only grew hot code. Scheduler ownership must be first-class enough to move cadence, not just a per-table guard around the current append path. |
| BUILDING2 high standalone visible-cost groups | Do not retry `96..104`, `66..78`, `325..331`, `538..550`, or adjacent one-off hand tables. `96..104` saved one nominal read but worsened visible blocking/due misses, `66..78` was loop/blocking/read-flat with only a one-VBlank hidden-refill change, and the fresh tail candidate `538..550` stayed exact-flat while growing hot code by `44` bytes. BUILDING2 high needs scheduler-owned timing or selective upload-ready/preprocessed pack data. |
| BUILDING4 high standalone visible-cost groups | Do not retry `{49,65}` as a high-tide one-off source table. It stayed exact-flat at `2985/2774`, `blocking_vb=285`, and `prefetch_overrun_vb=51` while growing/shifting `foregroundPilotPlay` by `+540` bytes. BUILDING4 needs generated scheduler/larger-window ownership or selective preprocessing, not isolated current-window tables. |
| BUILDING5 raw stream windows | Do not retry scalar window sizes. High and low both regressed total loop despite lower read counts; use generated grouping or preprocessing instead. |
| BUILDING-family raw stream windows | BUILDING4 low `36 KiB` is accepted; BUILDING6 `20/28 KiB`, BUILDING4 high `20/28 KiB`, and broad setup-prime are rejected. Retry only scene/tide-locally with fresh baselines and bounded visible-CD/refill tradeoff rules. |
| BUILDING6 high group `505..517` | Do not retry as a one-off hard-coded group. It fit the existing window and stayed exact-flat, so read-plan rank alone is insufficient for BUILDING6; require generated visible-cost metadata or scheduler-owned grouping first. |
| BUILDING6 `48 KiB` window plus `15..39` group | Do not retry as a larger scalar retained-window probe. The fresh v136 test saved many reads but regressed both visible cadence and hidden refill on both tides, so current BUILDING6 refill placement is scheduler-owned rather than capacity-owned. |
| BUILDING6 pal4 padded FGP3 | Do not retry as a same-layout padded conversion under current validated packs. The v0.7.2 high/low packs expand `1444370 -> 1601445` bytes (`-10.87%` saved), so this lane needs selective/keyframed residual encoding or an explicit layout-changing experiment. |
| BUILDING6 selective upload-ready append | Do not promote or retry as a raw same-footprint append. v146 shows both current packs leave only `1` byte of zero-tail slack, while the default selected x-band plan needs `3354208` payload-plus-rect bytes for `92` frames and `5708192` modeled upload bytes saved; raw foreground-only safety still reports `0` selected draw-covered bytes and `0` all-draw-covered selected frames. |
| BUILDING6 motion-comp host signal | Do not implement direct runtime MoveImage from the current analyzer alone. v146 reports a strong host-only signal (`242 / 305` candidate pairs, `64.85%` pair-payload savings; zero-shift runtime model saves `680717` compose bytes and `11701120` upload bytes), but runtime promotion needs a new pack format that preserves old-position cleanup, RAM mirror correctness, dirty tracking, and CD/layout budget. |
| PAL4 aligned pair stores | Do not retry as a local branch in `grCompositePacked4OpaqueRun()`. It shrank ELF but regressed BUILDING4 high cadence; retry only after pack-generated aligned command classes or a pair LUT removes the hot-path branch/packing cost. |
| PAL4 pair-LUT halfword compositor | Do not retry as a local C inner-loop swap. Reusing `palLutSierra` improved VISITOR3 high blocking only (`191 -> 188`) with flat loop timing and regressed low tide `1140 -> 1142`; keep PAL4 compose work on generated aligned command classes, selective direct16/upload-ready data, or layout-neutral assembly/codegen. |
| Smaller windows | Group metadata preserves due-frame coverage. |
| Prepared-frame cleanup | Explicit render/CD budget exists. |
| Direct-stage read-into-window | Group/tail-preserving merge keeps `blocking_reads=4`. |
| ACTIVITY10 low contiguous setup-prime | Generated segmented coverage or inter-scene preload exists; `304 KiB`/`288 KiB` zero-loop and `256 KiB` regresses loop/canary timing. |
| Debug/code-size compile gates | Text/CD phase padding or hot/cold section isolation exists. |
| Audio TU `-Os` | Done under the foreground/resource-size baseline; keep accepted unless cross-scene sound validation regresses. |
| Resource TU `-Os` | Done under the foreground-size baseline; keep accepted unless a cross-scene setup regression appears. |
| Pause-menu TU `-Os` | Done under the foreground/resource/sound-size baseline; keep accepted if normal pause visual/input validation passes. |
| PS1 stubs TU `-Os` | Done under the pause-menu-size baseline; keep as cumulative ELF pressure only. |
| Events TU `-Os` | Done under the pause/stub-size baseline; keep accepted if normal pause/input validation passes. |
| Utils TU `-Os` | Done under the events-size baseline; keep as cumulative ELF pressure only. |
| Uncompress TU `-Os` | Do not retry as an isolated flag; it was exact no-op on runtime and size. |
| Island TU `-Os` | Done under the utils-size baseline; keep accepted if random-island visual validation stays clean. |
| Main TU `-Os` | Done under the island-size baseline; keep accepted if menu/pause visual validation stays clean. |
| CDROM TU `-Os` | Do not retry as a whole-TU flag; it crossed a smaller PS-EXE bucket and regressed `blocking_vb` to `10`. |
| Buffered read file-LBA cache | Do not retry alone; it was timing-flat but grew the helper and ELF. |
| Upload perf guard combine | Done; keep because it shrank `grDrawBackground` with exact timing/work identity. |
| Upload perf guard local cache | Do not retry alone; it shrank the function but grew total ELF with no speed movement. |
| Inline perf-detail check | Do not retry alone; it shrank two hot functions but grew total ELF with no speed movement. |
| Upload perf marker combine | Done; keep because it removed one rendered-frame marker call and preserved all upload/dirty counters. |
| `grDrawBackground()` function `-Os` | Done; keep because scoped upload-function codegen stayed exact-flat and shrank ELF. A current v0.7.2 retest of default `-O2` grew `grDrawBackground` by `504` bytes, left VISITOR3/WALKSTUF1/BUILDING2 key timing flat, and gave ACTIVITY9 only a one-VBlank blocking-only movement with no loop win. Retest only after generated upload-ready bands or a dirty-upload rewrite changes this hot path. |
| `grUpdateDisplay()` function `-Os` | Done; keep because scoped display-wrapper codegen stayed exact-flat and shrank ELF. A current v0.7.2 retest of default `-O2` grew the helper by `40` bytes, shifted CD helpers by `40` bytes, and left VISITOR3/WALKSTUF1/BUILDING2/ACTIVITY9 key timing flat. Retest only after display/present scheduling or upload ownership changes. |
| Reusable upload `RECT` stack storage | Done; keep because it removes unused per-call stack array storage with exact timing/upload identity. |
| Upload rect cap `6` after stack reuse | Do not promote from fishing1 alone; it was exact no-op on all tracked metrics and narrows cross-scene headroom. |
| Upload band byte arrays | Do not promote alone; it shrank `grDrawBackground` but grew the final ELF with no timing or work movement. |
| `grRestoreBgFromRects()` function `-Os` | Do not retry alone; it shrank the function but grew total ELF with no timing movement. |
| PAL4 compositor function-scoped `Os` | Do not retry; it shrank the compositor and loaded executable but regressed blocking even with foreground LBA restored. |
| Single-band narrow upload scratch | Do not retry as a special case; fishing1 upload bytes did not move and code grew. |
| Touched-only current dirty-row clearing | Done; keep because it removes per-frame row-table stores with exact timing/work identity and no new memory. |
| Touched-only dirty-row promotion | Done; keep because it removes full dirty-row table copies and produced a repeatable `2` VBlank loop win. |
| Direct PAL4 row dirty marking | Do not retry as written; it produced only target-accounting movement with flat loop speed and large compositor growth. |
| Dirty-row promotion overlap clear skip | Do not retry as an isolated branchy cleanup; exact-flat timing with ELF growth. |
| Post-dirty raw window retune | Do not retry raw `18-20 KB` windows blindly; `20 KB` regressed and the 9-sector rounded window shape crashed before metrics. Use generated group metadata/cost prediction first. |
| Aligned CD file-LBA cache | Done; keep because it shrank the active aligned read helper and ELF with exact timing/layout identity. |
| Aligned CD single-chunk fast path | Do not retry as a duplicated branch; it grew the helper by 104 bytes without moving timing. |
| Aligned CD helper function-scoped `Os` | Done; keep because it shrank the aligned-read path and ELF with exact timing/layout identity. The current v0.7.2 default-`O2` retest grew the helper by `+524` bytes and stayed key-flat, so do not retest without a changed CD helper/scheduler shape. |
| Buffered CD helper function-scoped `Os` | Do not retry alone; it grew the ELF and did not shrink the helper. |
| Fallthrough slack `5` after CD helper cleanup | Do not retry as a local guard change; it regressed `blocking_vb` and `prefetch_overrun_vb` to `10`. |
| Fallthrough slack `7` after CD helper cleanup | Do not retry as a local guard change; it failed before metrics with log overflow/regtest `137`. |
| Cap-aware foreground read plans | Done; use regenerated read-plan artifacts before choosing new groups or setup segments. Older plans overstated setup residency for FISHING1, VISITOR3, and WALKSTUF1. |
| Visible-cost foreground read plans | Done first pass plus append-start fireability scoring. Use `visible_candidate_sets` and require `append_start_fireable=true` before choosing new manual groups. Raw saved-read rank alone has selected exact-flat or regressing BUILDING/VISITOR/WALKSTUF candidates. |
| VISITOR3 low read group `182..194` | Do not promote or retry as a hand-coded group. It saved one read and lowered refill overrun, but regressed loop/blocking; require generated visible-cost scoring before more VISITOR3 groups. |
| VISITOR3 high `163..175` / low `158..170` groups | Do not retry as standalone source tables. The generated visible-cost candidates produced `group_hits=0`; high had no key timing improvement and low regressed loop/blocking/refill. Require append-start ownership metadata or scheduler-owned preload before more VISITOR3 grouping. |
| VISITOR3 high read group `170..186` with 16-sector retained capacity | Done; keep. The first stale-canary rejection was corrected with a clean same-source control. VISITOR3 high improves `1456/1010 -> 1455/1010`, `blocking_vb 363 -> 361`, and `loop_reads 53 -> 52`; VISITOR3 low, FISHING1, FISHING3 high/low, BUILDING2 low, ACTIVITY9 low, and WALKSTUF1 high canaries match the clean control on loop timing. Do not generalize this into more hot hand tables; next VISITOR3 work should use scheduler/generated metadata. |
| VISITOR3 low read group `170..186` with 16-sector retained capacity | Do not promote or retry as a hand-coded group. It saved one nominal read but regressed low tide `1455/1010 -> 1458/1010`, `blocking_vb 366 -> 369`, with fixed layout. The retained append cadence costs more than the saved read. |
| ACTIVITY9 high `434..450` / low `841..853` groups | Do not retry as standalone hand-coded groups. Under `activity9-window-v072c`, high fired but regressed `loop_vb 2185 -> 2209`, `blocking_vb 117 -> 123`, and `prefetch_overrun_vb 14 -> 17`; low stayed exact-flat and failed the improvement gate. ACTIVITY9 residual CD work needs scheduler-owned read timing, generated append-start ownership metadata, or selective preprocessing/upload-ready pack data. |
| WALKSTUF1 pal4 padded FGP3 | Do not retry direct pal4 temporal-residual conversion for the current validated packs. Both high and low expand `1530775 -> 1712687` payload bytes, so padding back to the old file size would corrupt the pack. The current high `429..441` read-group probe stayed exact-flat while shifting hot symbols, and the read plan classifies the remaining top candidates as tight-visible-gap. Retry only with a new shrinking encoder, scheduler-owned grouping, selective indexed8/data-shape preprocessing, or an explicit layout-moving pack experiment. |
| ACTIVITY9 pal4 padded FGP3 | Done; keep. Both validated wide-stitched ACTIVITY9 packs shrink as FGP3 temporal residuals and fit when padded back to the original `1745484` byte CD footprint. Runtime payload drops `1740180 -> 1453793`; high improves `2185/2049 -> 2101/2056`, low improves `2197/2054 -> 2103/2053`, and the exact matrix rollup moves to `0.8745%` over target / `99.4479%` target speed. |
| ACTIVITY9 low FGP3 read group `624..636` | Done; keep. Under the padded FGP3 data shape, the low-tide grouped append reduces visible CD pressure: `loop_vb 2103 -> 2093`, target accounting `2053 -> 2056`, `blocking_vb 60 -> 43`, `prefetch_overrun_vb 18 -> 14`, and `due_misses 7 -> 5`. That checkpoint later moved to `0.8309%` over target / `99.4779%` target speed after the VISITOR3/WALKSTUF1 setup-prime and VISITOR3 low-group promotions; the current rollup is tracked at the top of this file. |
| BUILDING5 pal4 padded FGP3 | Done; keep. Both current BUILDING5 validated packs shrink as FGP3 temporal residuals and fit when padded back to the original `818670` byte CD footprint. High improves `loop_vb 3359 -> 3343`, `overrun_vb 13 -> 0`, `blocking_vb 20 -> 5`, and `loop_reads 56 -> 41`; low improves `loop_vb 3357 -> 3345`, `overrun_vb 10 -> 0`, `blocking_vb 17 -> 8`, and `loop_reads 56 -> 41`. This checkpoint later moved to `0.8071%` over target / `99.5018%` target speed after the ACTIVITY11 padded-FGP3 promotion; the current rollup is tracked at the top of this file. |
| ACTIVITY11 pal4 padded FGP3 | Done; keep. Both current ACTIVITY11 validated packs shrink as FGP3 temporal residuals and fit when padded back to the original `433970` byte CD footprint. High improves `loop_vb 1729 -> 1715`, `overrun_vb 9 -> 0`, `blocking_vb 10 -> 2`, `prefetch_overrun_vb 4 -> 2`, and `loop_reads 29 -> 11`; low improves `loop_vb 1729 -> 1717`, `overrun_vb 12 -> 0`, `blocking_vb 14 -> 4`, `prefetch_overrun_vb 9 -> 4`, and `loop_reads 29 -> 11`. Full scene setup grows by `8/11` VBlanks, accepted because the active loop now lands under target. This checkpoint later moved to `0.7939%` over target / `99.5149%` target speed after the MARY5 padded-FGP3 promotion; the current rollup is tracked at the top of this file. |
| MARY5 pal4 padded FGP3 | Done; keep. Both current MARY5 validated packs shrink as FGP3 temporal residuals and fit when padded back to the original `646602` byte CD footprint. High improves `loop_vb 1591 -> 1581`, `overrun_vb 9 -> 0`, `blocking_vb 8 -> 5`, `prefetch_overrun_vb 8 -> 0`, and `loop_reads 49 -> 42`; low improves `loop_vb 1592 -> 1581`, `overrun_vb 11 -> 0`, `blocking_vb 10 -> 6`, `prefetch_overrun_vb 10 -> 2`, and `loop_reads 49 -> 42`. This checkpoint later moved to `0.8228%` over target / `99.4872%` target speed after the JOHNNY2/MARY2 stale-row refreshes and MARY2 prefetch relief; the current rollup is tracked at the top of this file. |
| JOHNNY2 pal4 padded FGP3 + clean-pressure relief | Done; keep. Both current JOHNNY2 validated packs shrink as FGP3 temporal residuals and fit when padded back to the original `288637` byte CD footprint. Against the same-commit FGP2 control, high improves `loop_vb 1833 -> 1801`, `overrun_vb 82 -> 50`, and `blocking_vb 401 -> 369`; low improves `1833 -> 1800`, `82 -> 49`, and `399 -> 377`. The follow-up JOHNNY2-local clean-pressure relief preserves `stage1_window` prefetch and moves both tides to `1741/1751`, with blocking `369/377 -> 0`, due misses `144 -> 0`, and loop reads `144 -> 8`. Current exact matrix rollup is `-0.2497%` over target / `100.2899%` target speed. |
| MARY2 pal4 padded FGP3 + prefetch relief | Done; keep. Both current MARY2 validated packs shrink as FGP3 temporal residuals and fit when padded back to the original `582189` byte CD footprint. Against the same-commit FGP2 control, the FGP3 step improves high `scene_vb 2638 -> 2583`, `loop_vb 2385 -> 2330`, and low `loop_vb 2384 -> 2327`. The follow-up MARY2-local clean-pressure relief restores `stage1_window` prefetch and moves high/low to `2241/2248` and `2242/2250`, with blocking `668/662 -> 2/2` and due misses `233 -> 0`. This checkpoint later moved to `0.7400%` over target / `99.5291%` target speed after the VISITOR3 cleanup-metadata compaction; the current rollup is tracked at the top of this file. |
| VISITOR3 FGP3/v3 cleanup-metadata compaction | Done; keep. Both current VISITOR3 validated packs keep their `1555450`-byte CD footprint and PAL4 draw payloads, but cleanup row/span metadata is compact-u16 encoded. Runtime active payload drops `1552446 -> 1265930`; high improves `1450/1015 -> 1406/1019`, `blocking_vb 355 -> 296`, `prefetch_overrun_vb 14 -> 7`, and `loop_reads 45 -> 40`; low improves `1452/1012 -> 1405/1015`, `blocking_vb 361 -> 301`, `prefetch_overrun_vb 19 -> 8`, and `loop_reads 49 -> 44`. Full compact draw metadata is rejected for now because the extra compositor code moved non-VISITOR canaries. This checkpoint later moved to `0.6781%` over target / `99.5777%` target speed after the BUILDING2 cleanup-metadata compaction; the current rollup is tracked at the top of this file. |
| VISITOR3 pack-only FGP3 padding trim | Do not promote. Trimming both current packs saved `573032` trailing zero bytes but regressed high `1406 -> 1409` and low `1405 -> 1412`; keep the same-layout padded files unless a format change reduces active runtime work. |
| BUILDING2 FGP3/v3 cleanup-metadata compaction | Done; keep. Both current BUILDING2 validated packs keep their `1303332`-byte CD footprint and use compact-u16 cleanup metadata while preserving existing PAL4 draw spans. Active payload drops `1296388 -> 1044638`; high improves `1468/1285 -> 1430/1289`, `blocking_vb 301 -> 212`, `prefetch_overrun_vb 56 -> 20`, and `loop_reads 96 -> 82`; low improves `1465/1276 -> 1429/1286`, `blocking_vb 334 -> 193`, keeps prefetch overrun flat, and cuts `loop_reads 87 -> 68`. This checkpoint later moved to `0.4096%` over target / `99.7860%` target speed after the BUILDING4, ACTIVITY9, JOHNNY2, WALKSTUF1 low-primecap, selector-cleanup, FGP3/v4 draw-metadata, and compact decoder inline promotions; the current rollup is tracked at the top of this file. |
| BUILDING4 FGP3/v3 cleanup-metadata compaction plus stream-window retune | Done; keep. Both current BUILDING4 validated packs keep their `1714154`-byte CD footprint and use compact-u16 cleanup metadata while preserving existing PAL4 draw spans. Active payload drops `1705426 -> 1370198`; high improves `2985/2774 -> 2939/2786`, `blocking_vb 285 -> 240`, `prefetch_overrun_vb 51 -> 27`, and `overrun_vb 211 -> 153`; low improves `2981/2784 -> 2945/2798`, `blocking_vb 199 -> 117`, `prefetch_overrun_vb 119 -> 114`, and `overrun_vb 197 -> 147`. The compact payload needed smaller scene-local windows (`24 -> 20 KiB` high, `36 -> 32 KiB` low) to keep hidden refill under the strict gate. This checkpoint later moved to `0.4096%` over target / `99.7860%` target speed after ACTIVITY9 low cleanup-metadata compaction, JOHNNY2 clean-pressure relief, WALKSTUF1 low-primecap, selector cleanup, FGP3/v4 draw-metadata compaction, and compact decoder inline promotion; the current rollup is tracked at the top of this file. |
| ACTIVITY9 low FGP3/v3 cleanup-metadata compaction | Done; keep as a tide-specific no-new-runtime-code pack-shape win. The paired high/low attempt is rejected because high regressed, but compacting only `ACTV9LOW.FG2` keeps the `1745484`-byte CD footprint, keeps ACTIVITY9 high flat at `2094/2056`, and improves low `2098/2056 -> 2087/2056`, `blocking_vb 47 -> 42`, `prefetch_overrun_vb 19 -> 12`, and active payload `1453793 -> 1196583`. Standard canaries across FISHING1, VISITOR3 high/low, WALKSTUF1 high/low, BUILDING2 high/low, BUILDING4 high/low, BUILDING6 high/low, and ACTIVITY9 high/low passed. This checkpoint later moved to `0.4096%` over target / `99.7860%` target speed after JOHNNY2 clean-pressure relief, WALKSTUF1 low-primecap, selector cleanup, FGP3/v4 draw-metadata compaction, and compact decoder inline promotion; the current rollup is tracked at the top of this file. |
| FGP3/v4 compact draw metadata for current compact residual packs | Done; keep. Current FGP3/v3 compact residual packs now use FGP3/v4 draw row/span metadata, preserving padded CD footprints and LBAs while reducing active metadata. VISITOR3 high/low improve to `1369/1023` and `1376/1023`, BUILDING2 high/low improve to `1405/1298` and `1395/1294`, ACTIVITY9 low improves to `2085/2058`, and the FISHING1 high control remains under target at `1068/1074`. This checkpoint later moved to `0.4096%` over target / `99.7860%` target speed after the compact decoder inline promotion; the current rollup is tracked at the top of this file. |
| Compact FGP3/v4 metadata reader inline | Done; keep. `grReadCompactSpanU16` is now a default inline helper instead of a noinline `-Os` helper. VISITOR3 high/low improve to `1357/1023` and `1361/1023`, BUILDING2 high/low improve to `1394/1301` and `1385/1303`, ACTIVITY9 low remains timing-flat at `2085/2058` with an accepted `blocking_vb 28 -> 29`, and FISHING1 high remains exact-flat under target at `1068/1074`. This checkpoint later moved to `-0.2497%` over target / `100.2899%` target speed after later VISITOR3, BUILDING4, BUILDING2, WALKSTUF1, and code-shape promotions. |
| VISITOR3 high read-group table prune | Done; keep as code headroom. The remaining high-tide local table (`138..162`, `170..186`, `230..242`) is exhausted after compact-u16 inline and no longer changes runtime metrics. Removing it keeps VISITOR3 high/low, BUILDING2 high/low, ACTIVITY9 low, and FISHING1 high exact-flat while shrinking `foregroundPilotPlay` by `48` bytes. This checkpoint later moved to `-0.2497%` over target / `100.2899%` target speed after later VISITOR3, BUILDING4, BUILDING2, WALKSTUF1, and code-shape promotions. |
| VISITOR3 restore-minus-current cleanup | Done; keep as pack-side data-shape win. FGP3/v4 cleanup spans now omit intervals redrawn by the same current frame while leaving draw pixels, padded pack size, pack LBAs, and PS-EXE size fixed. VISITOR3 high/low improve to `1139/1024` and `1140/1024`, blocking drops to `191/194`, prefetch overrun drops to `0/0`, active payload drops `981514 -> 737600`, and runtime restore bytes drop `973290 -> 498676`. This checkpoint later moved to `-0.0670%` over target / `100.1368%` target speed after later BUILDING4, BUILDING2, WALKSTUF1, and VISITOR3 draw-clip promotions. |
| BUILDING4 restore-minus-current cleanup | Done; keep as pack-side data-shape win. FGP3/v4 cleanup spans now omit intervals redrawn by the same current frame while leaving draw pixels, padded pack size, pack LBAs, and PS-EXE size fixed. BUILDING4 high/low improve to `2844/2816` and `2855/2815`, blocking drops to `37/46`, active payload drops `1032442 -> 855284`, and runtime restore bytes drop `1229878 -> 546950`. This checkpoint later moved to `-0.0670%` over target / `100.1368%` target speed after later VISITOR3, BUILDING2, and WALKSTUF1 promotions. |
| WALKSTUF1 high setup-prime cap 144 KiB | Superseded by the compact FGP3/v4 WALKSTUF1 baseline. Keep as a historical high-tide-only setup-prime retune, not a shared WALKSTUF1 policy. High improved `1595/1402 -> 1592/1406`, `overrun_vb 193 -> 186`, `blocking_vb 277 -> 275`, `loop_reads 136 -> 134`, and `due_misses 56 -> 55`; low stayed exact-flat at `1604/1407`. |
| VISITOR3 high-only offscreen draw clip | Done; keep as a high-tide-only pack-data trim. The transform preserves `VISITOR3.FG2` size, entry sizes, offsets, pack LBA `22472`, and the `215040` byte PS-EXE bucket, while zeroing clipped draw tails for `17` high-tide entries. High improves `1139/1024 -> 1137/1024`, `overrun_vb 115 -> 113`, and `blocking_vb 191 -> 190`; low and broad controls stay exact-flat. Current exact matrix rollup is `-0.2497%` over target / `100.2899%` target speed. |
| VISITOR3 low exit-right offscreen draw clip | Done; keep as a low-tide pack-data trim limited to entries `139..143`. The transform preserves `VIST3LOW.FG2` size, entry sizes, offsets, pack LBA `23232`, and the `215040` byte PS-EXE bucket, while zeroing clipped exit-right draw tails. Low improves `1140/1024 -> 1138/1024`, `overrun_vb 116 -> 114`, and `blocking_vb 194 -> 191`; high and broad controls stay exact-flat. Low `ship-left` and combined `ship-and-exit` subsets are rejected because they regress low to `1151/1024`. Current exact matrix rollup is `-0.2497%` over target / `100.2899%` target speed. |
| BUILDING2 high restore-minus-current cleanup | Done; keep as a high-tide-only pack-side data-shape win. FGP3/v4 cleanup spans now omit intervals redrawn by the same current frame only in `BUILDING2.FG2`, preserving the `1303332` byte CD footprint, pack LBA `6180`, and the `215040` byte PS-EXE bucket. High improves `1394/1301 -> 1353/1311`, `overrun_vb 93 -> 42`, `blocking_vb 138 -> 56`, and `loop_reads 68 -> 62`; low stays on the prior pack because the both-tide transform regressed hidden prefetch overrun `8 -> 13`. Current exact matrix rollup is `-0.2497%` over target / `100.2899%` target speed. |
| BUILDING2 high read group `60..72` | Done; keep as a high-tide-only retained stream group. The range was the only current scheduler-or-guarded matrix candidate with zero overread and medium visible gaps after the v108 pack pass. It grows `foregroundPilotPlay` by `12` bytes but keeps the `215040` byte PS-EXE bucket and all canary pack LBAs fixed. BUILDING2 high improves `1353/1311 -> 1349/1316`, `overrun_vb 42 -> 33`, `blocking_vb 56 -> 48`, `prefetch_overrun_vb 20 -> 12`, and `loop_reads 62 -> 61`; VISITOR3 high/low, BUILDING2 low, BUILDING4 high/low, ACTIVITY9 low, and FISHING1 high stay exact-flat. Current exact matrix rollup is `-0.2497%` over target / `100.2899%` target speed. |
| BUILDING2 low read group `365..381` | Done; keep as a low-tide-only retained stream group. The range was the top remaining BUILDING2 low row after v109 and passed focused plus broad strict gates despite its partial-overlap/overread risk. It grows `foregroundPilotPlay` by `8` bytes versus v109 but keeps the `215040` byte PS-EXE bucket and all canary pack LBAs fixed. BUILDING2 low improves `1385/1303 -> 1383/1304`, `overrun_vb 82 -> 79`, `blocking_vb 121 -> 118`, `prefetch_overrun_vb 8 -> 5`, `loop_reads 57 -> 55`, and `due_misses 23 -> 22`; VISITOR3 high/low, BUILDING2 high, BUILDING4 high/low, ACTIVITY9 low, and FISHING1 high stay exact-flat. Current exact matrix rollup is `-0.2497%` over target / `100.2899%` target speed. |
| BUILDING2 low restore-minus-current retry | Do not promote as a pack-only change. It improves low as far as `1383 -> 1346`, overrun `79 -> 35`, blocking `118 -> 50`, and loop reads `55 -> 52`, but hidden refill regresses `5 -> 13`; temporary setup-prime and stage-guard salvages did not fix that. Retry only with generated scheduler/refill ownership or a second data-shape change that reduces active CD pressure before shortening the render cadence. |
| BUILDING2 low restore-minus-current plus window slack `5` | Do not promote or retry as a local slack guard. It fixes the strict hidden-refill blocker (`prefetch_overrun_vb 5 -> 0`) and improves loop/overrun (`1383/1304 -> 1360/1313`, overrun `79 -> 47`), but it starves active presentation and regresses visible blocking `118 -> 180` plus due misses `22 -> 43`. This proves the low transform needs generated refill placement, not simply fewer low-slack window reads. |
| BUILDING6 `48 KiB` window plus `15..39` read group | Do not promote or retry as a scalar window/group change. It reduced loop reads from `74 -> 32` high and `73 -> 31` low, but regressed high `2520/2442 -> 2568/2443`, blocking `62 -> 115`, hidden refill `64 -> 117`, and low `2515/2437 -> 2565/2445`, blocking `70 -> 115`, hidden refill `66 -> 96`. BUILDING6 needs generated scheduler ownership or a shrinking/selective FGP2 data-shape encoder before another read-count group. |
| VISITOR3 low scoped composite-helper `-Os` | Done; keep as code-shape plus low-tide timing win. `grCompositeToBackground()` shrinks `0xbf4 -> 0x5b0`, `grCompositeToBackgroundFlip()` shrinks `0xc60 -> 0x63c`, and `jcreborn.elf` shrinks `960556 -> 951708` while the PS-EXE bucket remains `215040`. VISITOR3 low improves `1138/1024 -> 1135/1024`, `overrun_vb 114 -> 111`, `blocking_vb 191 -> 184`, and `loop_read_vb 200 -> 194`; VISITOR3 high, BUILDING2 high/low, BUILDING4 high/low, ACTIVITY9 low, and FISHING1 high stay exact-flat. Current exact matrix rollup is `-0.2497%` over target / `100.2899%` target speed. |
| VISITOR3 v4 draw-tail trim + stage guard | Done; keep as the current VISITOR3 baseline. FGP3/v4 zero draw-tail bytes are trimmed while both packs stay `1555450` bytes and LBAs stay fixed; high setup-prime residency rises to `232 KiB`, and a VISITOR3-only hidden large-stage guard prevents no-slack prefetch debt. VISITOR3 high improves `1137/1024 -> 1118/1028`, `overrun_vb 113 -> 90`, `blocking_vb 190 -> 150`, `loop_reads 33 -> 27`, and `loop_read_vb 200 -> 153`; low improves `1135/1024 -> 1126/1025`, `111 -> 101`, `184 -> 170`, `33 -> 31`, and `194 -> 179`. Both keep `prefetch_overrun_vb=0`, and BUILDING2 high/low, BUILDING4 high/low, ACTIVITY9 low, and FISHING1 high stay exact-flat. Current exact matrix rollup is `-0.2497%` over target / `100.2899%` target speed. |
| WALKSTUF1 compact FGP3/v4 restore-minus-current | Done; keep as the current WALKSTUF1 baseline. Both PAL4/FGP2 packs are compacted into FGP3/v4 restore-minus-current packs and padded back to the original `1535263` byte footprint, preserving pack LBAs and the `215040` byte PS-EXE bucket. High improves `1592/1406 -> 1491/1426`, `overrun_vb 186 -> 65`, `blocking_vb 275 -> 85`, `prefetch_overrun_vb 51 -> 32`, `loop_reads 134 -> 69`, and `due_misses 55 -> 13`; low improves `1604/1407 -> 1489/1427`, `197 -> 62`, `270 -> 86`, `55 -> 27`, `132 -> 69`, and `50 -> 12`. The broad non-WALKSTUF controls stayed exact-flat except VISITOR3 high; that same drift reproduced with original WALKSTUF1 FGP2 packs restored, so it is tracked as unrelated current-control drift. Current exact matrix rollup is `-0.2497%` over target / `100.2899%` target speed. |
| VISITOR3 cleanup-only offscreen clip | Do not promote. Clipping only offscreen FGP3/v4 cleanup spans preserves pack sizes, entry offsets, LBAs, and PS-EXE bytes while trimming `5526` bytes, `58513` cleanup pixels, and `1299` cleanup spans per tide, but both focused v106 gates are exact-flat: high remains `1137/1024` with `blocking_vb=190`, low remains `1138/1024` with `blocking_vb=191`. Treat this as safe but inert; future offscreen work needs draw/CD phase ownership or a different measured counter. |
| WALKSTUF1 high setup-prime cap 152 KiB | Do not promote. Raising the high-only cap from `144 KiB` to `152 KiB` extends setup coverage to sectors `2..76` and nominally cuts high loop reads `134 -> 133`, but regresses high `scene_vb 1880 -> 1884`, `loop_vb 1592 -> 1595`, `target_vb 1406 -> 1405`, `blocking_vb 275 -> 286`, and `due_misses 55 -> 57`. Treat `144 KiB` as the high-cap knee. |
| WALKSTUF1 low setup-prime cap 168 KiB | Do not promote. Raising the low-only cap from `160 KiB` to `168 KiB` preserves pack LBA and PS-EXE bucket but is exact-flat at `1895`, `1604/1407`, `overrun_vb=197`, `blocking_vb=271`, `prefetch_overrun_vb=54`, `loop_reads=132`, and `due_misses=50`. Treat `160 KiB` as the low-cap ceiling under the current contiguous setup-prime policy. |
| WALKSTUF1 low no-direct-stage branch | Do not promote as a broad scene-name check. It improves low visible metrics (`1604 -> 1601`, `blocking_vb 271 -> 214`, `due_misses 50 -> 34`) but regresses hidden refill (`prefetch_overrun_vb 54 -> 79`) and crosses the PS-EXE bucket. Keep the signal; retry only with a narrower direct-stage threshold or generated scheduler metadata. |
| Direct-stage cap 4 KiB | Do not promote. It preserves layout and lowers WALKSTUF1 low blocking, but regresses active timing (`1604 -> 1607`) and hidden refill (`54 -> 81`). Keep the global cap at `8 KiB`; frame/range-specific scheduling is required. |
| Direct-stage caps 6 KiB and 7 KiB | Do not promote or retry as scalar thresholds. `6 KiB` repeats the hidden-refill failure on both WALKSTUF1 tides despite visible blocking relief, and `7 KiB` is too small a blocking win with target-relative overrun regressions. Keep `8 KiB` until generated scheduler/read-cost metadata can choose frame/range-specific coverage. |
| WALKSTUF1 low read group `297..313` with `minSlack=8` | Do not promote or retry as a hand table. The safe slack guard prevented the group from firing (`group_hits=0`), while the source branch still shifted low target enough to regress overrun by one VBlank. WALKSTUF1 read clusters need generated scheduler metadata, not another hot source-table branch. |
| WALKSTUF1 selective upload-ready same-footprint append | Do not promote or retry as a raw same-footprint append. The current compact FGP3/v4 packs now expose `611305` zero-tail bytes and can fit a `609192` byte budgeted x-band subset for `39` frames with `1991904` modeled upload bytes saved, but raw foreground-only safety still reports `0` selected draw-covered x-band bytes and `0` all-draw-covered selected frames. Compact slack is a byte budget, not a safety proof; retry only with safe background-owned/precomposed pixels, ownership metadata, generated scheduler ownership, or MoveImage-safe motion data. |
| VISITOR3 default selective upload-ready append | Do not promote as a layout-neutral pack append. The current threshold plan selects `96 / 144` frames and estimates `6114568` selected upload bytes saved, but the upload-ready payload plus rect metadata needs `2462072` bytes per tide against only `814847` bytes of padded zero-tail slack. Retry only as a smaller budgeted subset, compressed upload payload, shrinking pack transform, or explicit layout-moving experiment. |
| VISITOR3 budgeted selective upload-ready target | Done as host-side implementation target, not runtime behavior. The current v140 analyzer exact-knapsacks the default-selected VISITOR3 rows against the post-tail-trim pack slack: high selects `75 / 117` default frames using `888880 / 891012` bytes and retaining `6290232` modeled upload bytes saved, while low selects `74 / 117` frames using `853848 / 854114` bytes and retaining `6166528` modeled bytes saved. Runtime promotion still needs a generated pack format with pre-contiguous rows, a safe background-owned/precomposed pixel source, and full VISITOR3/canary validation. |
| VISITOR3 runtime dirty-upload narrowing | Do not retry as a source-side optimization. The live uploader already has row-level dirty X metadata, but exact narrow intervals for current VISITOR3 would create about `131996` upload rects over the loop, and scratch-packed x-band variants have already failed from code-size, copy, and cadence cost. Upload-byte work must be pack-emitted or precomposed, not packed from tile rows during `grDrawBackground()`. |
| VISITOR3 v140 current-window read-plan refresh | Do not promote or retry another hand-authored source table. The refreshed read-plan from v127 found `0` candidates that are append-start fireable, current-window-sized, and low-risk. The rows that fit and fire are the late tight-cluster class, including the already-rejected high `315..331` and low `333..349` shapes, and remain `high-risk:scheduler-only`. |
| VISITOR3 `20 KiB` retained window with `12` VBlank slack | Do not promote or retry as scalar window/slack tuning. It improved total scene duration by shortening setup/load shape, but active loop regressed on both tides: high `1118 -> 1131`, blocking `150 -> 210`, reads `27 -> 39`; low `1126 -> 1139`, blocking `170 -> 212`, reads `31 -> 41`. Hidden refill stayed `0` and layout stayed fixed, so the failure is scheduler/CD ownership, not binary layout. |
| VISITOR3 low setup-prime `200 KiB` / `216 KiB` | Do not promote or retry as scalar low-prime tuning. `216 KiB` regressed low `1126 -> 1127` and blocking `170 -> 173`; `200 KiB` regressed low to `1152/1024`, blocking `191`, and hidden refill `3`. Keep the accepted `208 KiB` low cap. |
| VISITOR3 high setup-prime `256 KiB` after stage guard | Do not retry as scalar high-prime tuning. With the v127 stage guard active, `256 KiB` reduced high loop reads by one but regressed high to `1131/1027`, overrun `104`, blocking `155`, and hidden refill `3`. Keep high at `232 KiB`; larger residency is phase-negative under the current scheduler. |
| VISITOR3 no-op FGP3 entry prune | Do not promote. Removing the visually no-op entries reduced VISITOR3 high `loop_vb 1139 -> 1115`, `blocking_vb 191 -> 123`, `loop_reads 33 -> 29`, and active payload `737600 -> 659318`, but the shortened cadence created hidden refill debt: high `prefetch_overrun_vb 0 -> 56`, low `0 -> 17`. Treat this as evidence that VISITOR3 needs scheduler-owned prefetch placement or budgeted upload-ready data, not isolated entry-count pruning. |
| VISITOR3 no-op empty-hold recast | Do not promote. The pack-side scanner found `0` current VISITOR3 high/low FGP3/v4 payload entries with both cleanup and draw pixel counts at zero, so active payload stays `737600 -> 737600` and no binary runtime probe is available. The old prune win removed entries that still carry real cleanup/draw work under the current data shape. |
| VISITOR3 entry-origin recentering | Do not promote. A host-side size gate over current VISITOR3 high/low FGP3/v4 compact cleanup/draw streams found no legal origin shift that reduces compact-u16 metadata; both tides stay `active_payload 737600 -> 737600`, `saved=0`, `changed_entries=0`. |
| Dirty upload rect cap `8 -> 24` | Do not promote. VISITOR3 analyzer estimated only a tiny loop-level upload-byte reduction from eliminating the three cap-hit frames, and the runtime probe was exact-flat on VISITOR3 high/low (`1357/1023` and `1361/1023`) with no key metric improvement. Move upload work to generated selective upload-ready payloads or restore/compose coalescing rather than global rect-cap tweaks. |
| ACTIVITY9 high FGP3 read group `447..463` | Do not retry under the current data shape. It was tested with the low FGP3 group and stayed exact-flat on high tide, so only the low table was promoted. Revisit only after ACTIVITY9 high pack data, append-start ownership metadata, or scheduler timing changes. |
| ACTIVITY9 FGP3/v3 cleanup-metadata compaction | Do not promote as a paired high/low pack change under the current gate. It saved `257210` active payload bytes per tide and improved low `loop_vb 2098 -> 2087`, but high regressed `2094 -> 2099` with stable layout. Retry only with a high-tide window/cadence retune or explicit tide-specific promotion logic that keeps high flat. |
| BUILDING6 `48 KiB` window plus `15..39` group | Do not promote or retry as a scalar larger-window path. It saved reads but regressed high to `2568/2443` with hidden refill `117` and low to `2565/2445` with hidden refill `96`; require generated scheduler ownership or a shrinking/selective FGP2 encoder first. |
| BUILDING6 pal4 padded FGP3 | Do not benchmark direct pal4 temporal-residual conversion under the current validated packs. The size gate expands `1444370 -> 1601445`, so preserving CD layout would require truncation. Retry only with a shrinking encoder, selective residual/keyframe strategy, or explicit layout-moving experiment. |
| BUILDING6 v146 upload/motion refresh | Do not spend emulator time on raw BUILDING6 upload append. Same-footprint slack is still `1` byte and safe-pixel coverage is still `0`; the actionable path is generated zero-shift/motion residual format work or scheduler ownership, not another scalar source probe. |
| BUILDING4 high read group `537..561` | Do not promote or retry as a raw 24-sector hand-coded group. It saved two reads but regressed loop, blocking, and refill pressure; require generated scheduler/cost metadata before larger BUILDING4 high append groups. |
| BUILDING4 high read group `821..837` | Do not promote or retry as a standalone hand-coded group. A fresh current-code read-plan marked it low-visible-risk, but the corrected source probe under BUILDING4 high's `24 KiB` window plus `32 KiB` retained group capacity stayed exact-flat (`2985/2774`, `blocking_vb=285`, `prefetch_overrun_vb=51`, `loop_reads=93`, `due_misses=40`) and only grew/shifted code. BUILDING4 needs scheduler-owned append timing or generated ownership metadata, not more one-off range tables. |
| BUILDING4 high read group `57..69` | Do not promote or retry as a standalone hand-coded group. Append-start fireability was true, but BUILDING4 high stayed exact-flat while hot-code growth/phase made FISHING1, WALKSTUF1, and ACTIVITY9 canaries fail. Fireability is necessary but not sufficient; require scheduler-owned append timing or generated metadata before more BUILDING4 hand tables. |
| Setup segment persistence cleanup | Do not retry alone. It was memory-safe in theory but regressed VISITOR3 high through code layout/cadence while only improving refill overrun. |
| Unbuffered CD helper function-scoped `Os` | Done; keep because it shrank the setup-facing stream helper and ELF with exact playback identity. |
| Unbuffered CD helper default `O2` retest | Do not promote or retry under the current phase. It regressed FISHING1 loop/blocking/read/due-miss metrics, grew the helper `592 -> 660` bytes, and grew ELF `952312 -> 952548`. |
| `ps1_stubs.c` whole-TU `O2` | Do not promote or retry under the current phase. Five exact canaries stayed timing/work-flat while ELF and the stubs object grew. |
| `pause_menu.c` whole-TU `O2` | Do not promote or retry under the current phase. It grew the PS-EXE bucket `215040 -> 217088`, shifted foreground LBAs by `+1`, and regressed FISHING1, WALKSTUF1, BUILDING2, and ACTIVITY9 canaries. |
| `ps1_captions.c` whole-TU `O2` | Do not promote or retry under the current phase. FISHING1 stayed exact-flat and the PS-EXE bucket stayed fixed, but ELF grew `952268 -> 954084` and the captions object grew `28180 -> 31060` bytes. |
| Unbuffered CD file-LBA cache | Done; keep because it shrank the setup-facing stream helper and ELF with exact playback identity. |
| `fgRuntimeFillWindowForEntry()` function-scoped `Os` | Do not retry alone; it was exact no-op on timing, size, and tracked symbols. |
| Prepared visual metadata decoupling | Do not retry as metadata-only; it adds duplicate probes and code growth without staging farther ahead. |
| Prepared visual stage-next branch | Retry only with an explicit no-slack guard and scheduler budget; v2 was correctness-clean and lowered read/late counters but left `loop_vb` flat. |
| Prepared visual `>=4` threshold | Do not retry as a threshold-only tweak; it failed structurally before metrics. |
| Prepared visual positive-slack stage-next branch | Do not retry as a local guard; it reproduced v2's flat timing and code growth. |
| VISITOR3 prepared-visual `>=4` threshold after high `170..186` | Do not retry as a threshold-only tweak. The current-source probe stayed exact-flat on VISITOR3 high and low, so the remaining gap needs CD-first scheduling or generated append ownership rather than broader local prepare eligibility. |
| VISITOR3 prepared-present threshold `5` after cleanup compaction | Do not retry as a threshold-only tweak. It stayed exact-flat at the current `1406/1405` VISITOR3 high/low baseline, so the scheduler needs generated CD ownership or pack/data-shape work rather than another local slack constant. |
| VISITOR3 prepared-present threshold `3` or `5` after v127 | Do not retry as a local slack-constant tweak. Both sides of the accepted `4` threshold stayed exact-flat on the current v127 VISITOR3 baseline (`1118/1028` high and `1126/1025` low), with blocking, reads, hidden refill, pack LBAs, and PS-EXE bucket unchanged. Continue only through generated scheduler ownership or safe pack/data-shape work. |
| VISITOR3 high `144..156` current-window group after cleanup compaction | Do not retry as a source-table append. The fresh post-BUILDING4 baseline probe stayed exact-flat on VISITOR3 high/low (`1406/1019` and `1405/1015`), reported `group_hits=0`, and only shifted hot symbols by `+4` bytes. Retry this sector cluster only if scheduler-owned refill metadata changes when the append is attempted. |
| VISITOR3 resident-window staging reuse | Do not retry as a broad local staging eligibility change. It saved one VISITOR3 high duplicate read and reduced blocking, but turned hidden refill into `prefetch_overrun_vb 7 -> 40` and regressed low tide `loop_vb 1405 -> 1409`. Retry only with explicit generated refill budget/cadence ownership, not by treating every resident large entry as stageable. |
| VISITOR3 high read group `97..113` | Do not retry as a source-table group. It saved reads (`40 -> 38`) but regressed high tide `loop_vb 1406 -> 1421` and `blocking_vb 296 -> 309` with stable layout. Early VISITOR3 clusters are visible-gap-sensitive; use scheduler-owned refill timing or data-shape work instead. |
| VISITOR3 high `144..156` slack-gated group | Do not retry as a local min-slack table guard. Requiring `6` held VBlanks around the existing grouped append path stayed exact-flat (`1455/1010`, `blocking_vb=361`, `prefetch_overrun_vb=21`, `loop_reads=52`) while shifting hot foreground symbols. Retry only with generated scheduler ownership/refill budget metadata or a pack/data-shape change. |
| VISITOR3 direct-stage cap `16 KiB` | Do not retry as a local scene cap. It stayed exact-flat against the 192 KiB setup-prime high-tide baseline while shifting hot code; require generated coverage/read-cost metadata before changing direct-stage policy. |
| FG2 read group `102..110` | Do not retry as a raw hard-coded group; it saved one read but regressed visible CD pressure to `8` VBlanks. Retry only with group-cost prediction or CD-first slack ownership. |
| FG2 read groups `384..396` and `307..317` | Do not retry as raw hard-coded groups. `384..396` did not fit/fire; `307..317` was exact-flat with code growth. Generated metadata must prove append fit and read-count movement first. |
| Global setup-prime resident cap above `128 KiB` | Do not retry as a raw cap raise. The `192 KiB` probe saved FISHING1 reads but regressed visible loop/CD pressure and increased retained prefetch heap. |
| Direct stage into stream window | Do not retry as a local helper swap; it preserved reads but raised visible CD pressure by one VBlank. Retry only when group/tail metadata proves the direct-stage window stays hidden. |
| Hot whole-TU `-O3` | Function-scoped codegen or address padding preserves hot layout first. |
| Foreground pilot TU `-O3` | Do not retry as a whole-TU flag; it grew `foregroundPilotPlay` by about `5 KB`, moved the foreground pack three LBAs, and produced no key speed gain. |
| Graphics whole-TU `-O3` | Do not retry; `grDrawBackground`/restore code grew and cadence regressed to `blocking_vb=11`. |
| `grDrawBackground()` scoped `O2` | Do not promote alone; the four-case gate stayed exact-flat but the helper grew by `504` bytes and the ELF grew by `568` bytes. |
| `grUpdateDisplay()` scoped `O2` | Do not promote alone; the four-case gate stayed exact-flat but the helper grew by `40` bytes and the ELF grew by `168` bytes. |
| `foreground_pilot.c` whole-TU `O2` | Do not retry as a whole-TU flag. It grew the PS-EXE by one sector, grew `foregroundPilotPlay 0x252c -> 0x3114`, and failed before `JCPERF2` on repeated `fishing1-high` runs. Retry foreground codegen only as function-scoped, split-TU, or layout-padded probes. |
| `jc_reborn.c` whole-TU `O2` | Do not promote alone; the four-case gate stayed exact-flat, PS-EXE stayed flat, ELF grew by `4188` bytes, and hot symbols shifted by `+1300` bytes. Retry only through function-scoped or split cold/boot code shapes. |
| `resource.c` whole-TU `O2` | Do not promote alone; the four-case gate stayed exact-flat, PS-EXE stayed flat, ELF grew by `1584` bytes, and foreground hot symbols shifted by `+240` bytes. Retry only through function-scoped lookup helpers or cold/resource split shapes. |
| `sound_ps1.c` whole-TU `O2` | Do not promote alone; the four-case gate stayed exact-flat, PS-EXE stayed flat, ELF grew by `180` bytes, and CD helper symbols shifted by `+64` bytes. Retry only through function-scoped audio helpers or cold/audio split shapes. |
| `events_ps1.c` whole-TU `O2` | Do not promote alone; the four-case gate stayed exact-flat, PS-EXE stayed flat, ELF grew by `1240` bytes, and CD helper symbols shifted by `+208` bytes. Retry only through function-scoped event helpers or cold/input split shapes. |
| PAL4 compositor function-scoped `O3` | Do not retry; it shrank `grCompositePacked4SpansToBackground` by `28` bytes but still regressed cadence with `FISHING1.FG2` LBA restored. |
| Perf TU `-O3` | Do not retry as a whole-TU flag; it bloated `ps1PerfMarkCdReadDetailed` and regressed the exact gate. |
| Perf TU `-Os` | Do not promote just for ELF shrink; it left `jcreborn.exe` flat and grew hot perf functions. |
| Unused perf wrapper removals | Do not remove `ps1PerfMarkCdRead()` without padding/control; it is dead code but currently stabilizes perf/CD layout. |
| Pure FG2 LBA shifts | Do not retry as a standalone speed test; tested offsets up to `+8` sectors were timing-flat. |
| Hot-loop source cleanups | Preserve or deliberately sweep hot-symbol addresses first; the redundant prefetch pre-check removal failed even with FG2 LBA restored. |
| Whole-TU link-order moves | Do not retry the simple `cdrom_ps1.c`-next-to-foreground order; it was timing-flat despite large symbol movement. Use targeted hot-section padding or cold-section isolation instead. |
| Runtime dirty/upload heuristics | Pack-emitted masks or upload plans replace hot runtime checks. |
| Hard-coded read groups | Append-start trace or generated group metadata proves the group can fire; sectors `106..117` and `384..396` were exact no-ops even with extra retained capacity. |
| Upload coordinate static tables | Do not retry; static tables grew `grDrawBackground` and did not move timing. |
| Async CD | Async state ownership and polling metrics exist in a trace build. |
| `Setloc` skipping | Full frame hashes and work-identity gates prove every frame rendered. |
| Upload rect cap `6` | Cross-scene matrix proves `max_upload_rects <= 6`; current fishing1-only retest has no measured win. |
| Present-path polling cleanup | Replace only as part of a scheduler/input design; direct removal regressed pressure and weakens pause responsiveness. |
| 4-VBlank catch-up guards | Do not retry as another local threshold guard; the prepared-plus-window-resident form was exact no-op. Retry only as structural hold rebalance or a first-class prepared/dual-buffer scheduler. |
