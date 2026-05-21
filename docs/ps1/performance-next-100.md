# PS1 Performance Next 100

Date: 2026-05-21

Current accepted fishing1 high-tide canary baseline:

| Metric | Value |
|---|---:|
| `loop_vb` | `1068` |
| `target_vb` | `1072` |
| `remaining_overrun_vb` | `0` |
| `remaining_over_target` | `0.0% public` (`-0.37%` raw signed) |
| `blocking_vb` | `5` |
| `prefetch_overrun_vb` | `5` |
| `loop_reads` | `20` |
| `upload_bytes` | `10638080` |
| `restore_bytes` | `251144` |
| `prefetch_buffer` | `137048` bytes for current fishing1 high-tide FGP3 playback |
| `jcreborn.exe` | `217088` bytes |
| `jcreborn.elf` | `951708` bytes |

Goal: keep the FISHING1 canary at the public 100% cap or better while reducing the remaining
matrix-wide gaps without changing pixels, sound event timing, scene identity,
or long-run heap stability. The previous MARY2 checkpoint was `0.8228%` over
target / `99.4872%` target speed across `120` timing-bearing rows after the
`mary2-prefetch-relief-v081` refresh.

Current allocator-era rollup after the memory-region allocator refresh,
targeted W1/B2 setup-segment checkpoint, VISITOR3 stage1-under-clean-relief
checkpoint, VISITOR3 high/low clean-relief stream-window checkpoints, the
VISITOR3 high terminal-window trim, and the VISITOR3 high setup-segment
extension, the BUILDING4 high setup-segment promotion, the BUILDING2 high
`83..95` read-group retime, the VISITOR3 low third setup-segment promotion,
the VISITOR3 high frame139 raw-gap promotion, the VISITOR3 high third
setup-segment promotion, the WALKSTUF1 low `238..342` setup-segment
retarget, the WALKSTUF1 high `286..344` second setup-segment
retarget, followed by the BUILDING2 high guarded `271..287` visible-speed row,
the `315..327` same-loop CD-pressure row, the BUILDING4 low gap-8
dirty-upload band merge retune, the VISITOR3 low frame138 raw-gap
promotion, the WALKSTUF1 low clean-rect/setup-edge promotion, and the
WALKSTUF1 low `{91,107}` first-boundary read-group, the VISITOR3 high
frame56/57 raw-gap plus tight-refill, the BUILDING2 low `226..238` setup-segment
promotion, the VISITOR3 high tight56 retune, the BUILDING2 low
`226..262` + clean80 green promotion, the VISITOR3 high clean64 retune, the
BUILDING4 low 24 KiB stream-window green promotion, the WALKSTUF1 high
frame92 D4 promotion, the WALKSTUF1 low split `344..350` setup edge, the
WALKSTUF1 low frame132 payload trim, the WALKSTUF1 low `{378..390}`
read-group promotion, and the WALKSTUF1 high entry136/entry57 exact-flat
payload trims, followed by the WALKSTUF1 low `244..350`/`179..185`
setup-retarget plus `{113..129}` CD-pressure promotion, then the same-speed
`{355..371}` W1-low read-work promotion, then the same-speed W1-high
frame56 plus `{178..194}` CD-pressure promotion, the VISITOR3 high
80 KiB clean-relief stream-window promotion, and finally the same-speed
BUILDING2 high entries `92`/`94`/`95` payload trim plus `{185..197}` CD work,
followed by the WALKSTUF1 low entry65, entry39, entry55, entry56, entry59, entry63, entry66, and entry85 payload-only trims, then the
B2-high setup-resident duplicate alias for entries `141` and `142`,
W1-high prepare-before-window scheduler ownership, and the B2-high entry38
setup-edge duplicate alias, and the VISITOR3-high setup-edge `40..47`
speed row plus `42..49` same-speed CD-pressure slide, W1-low fresh-owner
`160..176`, the hot foreground scene-ID code-headroom pass, the W1-low
previous-visible late-cleanup compaction, and the BUILDING2 high
previous-visible cleanup-speed promotion, then the VISITOR3 high
previous-visible cleanup-headroom pass with entry `62` excluded, followed by
the VISITOR3 high offscreen cleanup screen-clip headroom pass, the BUILDING2
high preserve-entry-size screen-clip promotion, and the W1-low
preserve-entry-size screen-clip headroom promotion:
`+0.2498%` public average over target / `99.7539%` public target speed across
all `126` timing-bearing rows. The raw signed optimization matrix is about
`-0.4671%` / `100.4843%`. Since the compact full-matrix baseline was
about `17.4%` over target / `87.1%` target speed, the headless methodology has
removed about `17.15` public over-target points and added about `12.65` public
target-speed points. Bands are now `121` green, `5` yellow, `0` orange, and
`0` red. The latest W1-low preserve-entry-size screen-clip pass keeps pack
footprint, LBA, offsets, and table `dataSize` fixed while clipping `63`
entries, dropping logical active payload `755808 -> 712808`, removing `73798`
cleanup pixels and `39618` draw pixels, and improving hidden refill `4 -> 3`
while W1-low stays `1470/1446`, overrun `24`, and blocking `32`. The prior
BUILDING2 high preserve-entry-size screen-clip pass keeps
pack footprint, LBA, offsets, and table `dataSize` fixed while clipping `25`
entries, dropping logical active payload `574094 -> 520974`, removing `1982`
cleanup pixels and `40166` draw pixels, and improving B2-high
`1343/1311 -> 1343/1312`, overrun `32 -> 31`, blocking `51 -> 50`, and refill
overrun `18 -> 17`. The prior VISITOR3 high screen-clip headroom pass keeps the
five-yellow canary exact-flat while changing entries `101` and `116`, dropping
active high-pack payload `437785 -> 436469`, and removing `7393` offscreen
cleanup pixels with fixed pack footprint/LBA. The prior VISITOR3 high
cleanup-headroom pass keeps timing flat at `1082/1045`, overrun `37`, and
blocking `34` while changing `106` entries,
dropping active payload `461631 -> 437785`, cleanup restore bytes
`542088 -> 36092`, runtime restore bytes `471382 -> 56312`, and upload bytes
`18785280 -> 18038400`; entry `62` is closed as phase-negative because the
sector reduction regressed the row to `1083/1044`, overrun `39`, blocking
`35`. The latest promoted speed checkpoint keeps VISITOR3's tiny stage1 prefetch
frame buffer alive under clean-memory relief, keeps the bounded clean-relief
stream windows, trims high-tide terminal reads before resident setup data, and
extends the high-tide second setup segment through relative sector `229`, then
adds a third low-tide retained setup segment at relative sectors `206..230` and
relocates high frame `139`'s raw payload into the already retained `203..229`
gap, then adds a third high-tide retained setup segment at relative sectors
`228..262`, then moves high frames `56` and `57` raw into that paid gap, merges high setup coverage into `203..262`, pays the early retained setup edge `40..47`, slides it to `42..49` as same-speed CD-pressure work, and
caps tight-slack speculative refills at `56 KiB`:
high improves `1232/1033 -> 1070/1046`, blocking `478 -> 35`, reads
`137 -> 4`, and due `137 -> 2`; low improves `1231/1040 -> 1065/1039`,
blocking `438 -> 75`, reads `126 -> 18`, and due `126 -> 14`. WALKSTUF1 low
then replaces its split `197..243` plus `410..434` setup residency with one
retained `238..344` segment after low-only 48 KiB clean-rect chunking, then
adds the `{91,107}` first-boundary read group. The combined allocator-era W1-low
track improves `1479/1435 -> 1470/1445`, overrun `44 -> 25`,
blocking/read time `65/230 -> 35/163`, refill `18 -> 7`,
loop reads `50 -> 31`, and due `10 -> 4`; the newest `{378..390}` read group
then moves the current row to `1470/1446`, overrun `24`,
blocking/refill/read time `34/6/159`, and loop reads `30`. The latest
setup-retarget plus `{113..129}` owner keeps the same `1470/1446` target speed
but reduces scene `1812 -> 1809`, blocking/refill `34/6 -> 33/5`, and loop
reads/read time `30/159 -> 26/150`; the later `{355..371}` work-volume row
keeps speed and pressure exact-flat while reducing loop reads/read time again
to the current `24/146`. It becomes the new W1-low CD-pressure baseline for the next
candidate sweep. The
WALKSTUF1 high follow-up keeps the `198..244` setup slice, retargets the
second retained slice from `411..435` to `286..344`, adds `{149,165}`, and now
encodes frame `92` as previous-frame D4, improving
`1475/1433 -> 1471/1440`, overrun `42 -> 31`, blocking `76 -> 57`, prefetch
overrun `15 -> 13`, and due `15 -> 10`; the D4 step trades reads/read time
`44/199 -> 45/209` while still cutting visible loop time. The later entry136
and entry57 preserve-offset trims remove `4255` bytes and two payload sectors
without changing timing, pack footprint, pack LBA/sectors, or the PS-EXE
bucket. The newest frame56 preserve-offset trim removes another `3101` bytes
and one payload sector; paired with `{178..194}`, it keeps W1-high speed flat
at `1471/1440` while improving blocking `57 -> 56` and loop reads/read time
`45/209 -> 43/207`; the latest `{423..439}` retained-read row keeps the same
timing flat and lowers loop reads/read time again `43/207 -> 42/205`; the
follow-up `{404..416}` row stays exact-flat and lowers read work again
`42/205 -> 41/200`; the current prepare-first scheduler row moves W1-high to
`1472/1441`, keeps overrun/refill flat at `31/13`, and improves blocking/due
`56/10 -> 43/7`. The follow-up `365..389` direct-read row is closed on this
baseline: raw and slack4 variants reduced loop reads `44 -> 41` but regressed
W1-high to `1812/1476/1443`, overrun `33`, blocking `46`, and refill `16`;
slack8 was safe but exact-flat at `1808/1472/1441`, blocking/refill `43/13`.
The next broad `379..403` row repeats that result: raw reduces loop reads
`44 -> 40` but regresses to `1814/1478/1441`, overrun `37`, blocking `50`,
and refill `20`; slack8 is safe but exact-flat. The smaller `379..395` slice
gets closer but still fails: raw/slack4 reduce reads `44 -> 42` with overrun
flat at `31`, but blocking/refill regress `43/13 -> 44/14`; slack8 skips to
exact-flat. The next smaller `395..411` row is promoted as same-speed
CD-pressure work: W1-high remains exact-flat at `1808/1472/1441`, overrun
`31`, blocking/refill `43/13`, and due `7`, while loop reads/read time improve
`44/204 -> 42/201`; the five-yellow canary stays exact-flat. The `92..116`
family is now closed as a hand-authored table: raw reduced reads/read time
`42/201 -> 39/199` but regressed W1-high to `1810/1474/1438`, overrun `36`,
blocking/refill `47/15`, while slack1, slack2, and slack4 stayed exact-flat
and inert at `1808/1472/1441` with reads `42`. The late `411..423` row is now
promoted in layout-neutral replacement form: replacing the older high-tide
`{422..434}` tail row with `{411..423}` keeps W1-high exact-flat at
`1808/1472/1441`, overrun `31`, blocking/refill `43/13`, and due `7`, while
lowering loop reads/read time `42/201 -> 41/198`. The refreshed direct
read-table queue has no standalone/guarded safe promotion left; the top
remaining W1-high scalar item is `379..391`, scheduler-owned-only, so larger
W1-high work now needs generated ownership or paired render/work reduction.
The
latest VISITOR3 high clean-relief window retune then widens the high-tide
stream window from `68 KiB` to `80 KiB` while keeping the `56 KiB`
tight-refill cap, improving `1075/1044 -> 1071/1045`, overrun `31 -> 26`,
blocking `45 -> 35`, hidden refill `3 -> 1`, and due `3 -> 2`;
the rejected `96 KiB` variants saved a read but regressed loop/target cadence.
The VISITOR3 low direct grouped-append lane is closed on the current baseline.
Adding low-tide-only `{9..33}` was source-shape negative with no read gain
(`1065 -> 1067`, blocking `75 -> 76`); `{86..98}` fired and saved one read
but regressed to `1070/1041`, overrun `29`, blocking `79`; `{92..108}` also
saved one read but still regressed to `1067/1041`, blocking `76`. VISITOR3 low
needs generated deadline/refill ownership or pack/render byte reduction, not a
hand-authored low-tide append table.
The latest VISITOR3-low frame133 D4 data-shape swing is closed as well.
Encoding frame `133` against decoded frame `132` shrank the payload
`17069 -> 14188` bytes and `9 -> 7` sectors, but the normal staged path exposed
that staged previous-frame deltas are consumed without decode, so the first
forms tripped at frame133. The code-correct staged-decode debug form proved
correctness but regressed low to `1070/1040`, blocking `79`, reads `19`, and
crossed the PS-EXE bucket. The smaller binary alternative, forcing frames
`132/133` through on-demand decode, stayed in the `233472` byte bucket and
passed correctness but still failed timing at `1372/1068/1039`, overrun `29`,
blocking `77`, reads `19`, with `foregroundPilotPlay +76`. Close frame133 D4
unless a no-hot-C predecoded/staged-delta path or a row-reference codec removes
the decode/scheduler cost.
Frame `134` is closed in the same family. Encoding it against raw frame `133`
shrinks `17001 -> 14202` bytes and keeps the pack LBA plus the `233472` byte
PS-EXE bucket fixed, but forcing only frame `134` through on-demand decode still
regresses VISITOR3 low to `1372/1068/1039`, overrun `29`, blocking `78`, reads
`19`, and grows `foregroundPilotPlay` by `36` bytes. The isolated terminal D4
byte savings are not enough without a scheduler-neutral decode path.
The latest BUILDING2 high payload trim then cuts entries `92`, `94`, and `95`
from `8834 -> 6370`, `8873 -> 6939`, and `10247 -> 8827` bytes, reducing
active payload `669408 -> 663590` while the five-yellow canary stays exact-flat
at B2-high `1347/1313`, VISITOR3 high/low `1070/1046` and `1065/1039`, and
the previous WALKSTUF1 high/low baseline `1471/1440` and `1470/1446`. The follow-up `{185..197}`
B2-high read group is another same-speed CD-pressure row, and the current
`{158..174}` retry now lands cleanly on top of it: B2-high remains
`1621/1347/1313`, overrun `34`, blocking/refill `39/16`, and due `5`, while
loop reads/read time drop first `45/199 -> 43/197` and then `43/197 -> 40/189`.
The large setup-residency ladder is now closed on this baseline. Expanding the
second retained segment to `86..242`, `90..242`, `104..242`, `135..242`, or
`151..242` exhausted CACHE during clean-rect allocation; moving `104..242` to
TRANSIENT exhausted TRANSIENT/libc; the memory-safe `185..242` form reached
counters and cut loop reads `40 -> 38` but regressed B2-high to
`1630/1350/1311`, overrun `39`, blocking `45`, and prefetch overrun `19`.
Do not retry additive B2-high retained setup without first reducing clean
snapshot/cache residency or adding generated deadline/refill ownership.
The allocator-split retry for the same `122..146` early window is closed too.
All-in-TRANSIENT setup residency failed before `JCPERF2` with
`JCBSOD-FATAL TRANSIENT region+libc both exhausted: req=196608 have=165016`.
Keeping the accepted `3..35` plus `202..242` setup slices in CACHE and adding
only `122..146` as a TRANSIENT-owned third slice completed, but it saved reads
`40 -> 35` while regressing B2-high `1621/1347/1313 -> 1636/1350/1312`,
overrun `34 -> 38`, blocking `39 -> 44`, prefetch overrun `16 -> 18`, and
grew `foregroundPilotPlay` by `72` bytes. Close additive B2-high `122..146`
residency in both CACHE and split allocator forms; future work needs generated
deadline/refill ownership or render/pack work reduction before this cluster is
worth retrying.
The guarded `249..261` follow-up is closed too: slack9, slack10, and slack12
were all exact-flat at `1621/1347/1313`, overrun `34`, blocking/refill `39/16`,
and reads `40`; lower guards already regress. This confirms the safe/firing
threshold split for that row and keeps B2-high focused on generated ownership
or prior data-shape reduction. A prefetch-only generated-owner probe for the
same row is also closed: it regressed B2-high to `1631/1357/1314`, overrun
`43`, blocking `55`, due `8`, and reads `42` while only improving prefetch
overrun `16 -> 14`.
The broad B2-high group-slack gate is closed as well. Raising all current
B2-high read groups to `minSlackVBlanks=6` kept the visible timing exact-flat
at `1621/1347/1313`, but regressed pressure instead of relieving it:
blocking `39 -> 41`, loop reads `40 -> 42`, and due `5 -> 6`, with hidden
refill still `16`. The accepted read groups are not the remaining refill owner
by themselves; B2-high now needs generated frame/deadline ownership or a prior
data-shape/render-byte reduction before more scalar slack gating.
The nearby `{145..161}` scalar row is also closed. Although the refreshed read
plan modeled two saved reads and high visible safety, the focused proof
regressed B2-high to `1626/1353/1311`, overrun `42`, blocking `45`, and
hidden refill `21`, with loop reads still `40`. Do not retry the `145..161`
cluster as another static row; use generated per-frame ownership or reduce
pack/render work first.
The follow-up W1-low entry65/source frame96 preserve-offset trim is payload-only:
W1-low remains `1809/1470/1446`, overrun `24`, blocking/refill `33/5`, reads/due
`24/4`, and pack LBA/PS-EXE bucket stay fixed, while active low-pack payload
drops `788773 -> 785809` (`2964` bytes).
The next W1-low entry39/source frame49 preserve-offset trim is also payload-only:
the same five-yellow canary stays exact-flat, while active low-pack payload
drops again `785809 -> 782698` (`3111` more bytes). This reopens and supersedes
the older v878 entry39 rejection because the current setup/read baseline has a
different active-read shape.
The current W1-low entry55/source frame65 preserve-offset trim continues the
same lane: W1-low stays exact-flat at `1809/1470/1446`, overrun `24`,
blocking/refill `33/5`, reads/due `24/4`, and pack LBA/PS-EXE bucket stay
fixed, while active low-pack payload drops `782698 -> 777984` (`4714` more
bytes).
The follow-up W1-low entry56/source frame67 preserve-offset trim keeps the
same five-yellow canary exact-flat and drops active low-pack payload again
`777984 -> 774883` (`3101` more bytes), bringing the current W1-low payload
lane to `13890` bytes removed from the pre-entry65 baseline.
The follow-up W1-low entry59/source frame78 preserve-offset trim continues the
same lane: the five-yellow canary stays exact-flat at W1-low `1809/1470/1446`,
overrun `24`, blocking/refill `33/5`, reads/due `24/4`, fixed pack LBA, and
the same `233472` byte PS-EXE bucket while active low-pack payload drops again
`774883 -> 771139` (`3744` more bytes). The current W1-low payload lane now
has `17634` bytes removed from the pre-entry65 baseline.
The follow-up W1-low entry63/source frame91 preserve-offset trim keeps the
same five-yellow canary exact-flat and drops active low-pack payload again
`771139 -> 767740` (`3399` more bytes), bringing the current W1-low payload
lane to `21033` bytes removed from the pre-entry65 baseline.
The follow-up W1-low entry66/source frame100 preserve-offset trim keeps the
same five-yellow canary exact-flat and drops active low-pack payload again
`767740 -> 765161` (`2579` more bytes), bringing the current W1-low payload
lane to `23612` bytes removed from the pre-entry65 baseline.
The follow-up W1-low entry85/source frame148 preserve-offset trim keeps the
same five-yellow canary exact-flat and drops active low-pack payload again
`765161 -> 764658` (`503` more bytes), bringing the current W1-low payload
lane to `24115` bytes removed from the pre-entry65 baseline.
The promoted previous-visible late-cleanup subset then trims entries
`194/196/198/200/202/206/208/210` without moving the pack footprint or LBA:
active payload drops `764658 -> 755808`, cleanup spans `2611 -> 173`,
cleanup pixels `24946 -> 287`, and cleanup restore bytes `49892 -> 574`.
The refreshed five-yellow canary stays exact-flat at W1-low `1809/1470/1446`,
overrun `24`, blocking/refill `32/4`, loop reads/read time `24/146`, and due
`4`.
The prior BUILDING2 high previous-visible cleanup promotion applies the same
compaction family to the whole high-tide pack after the W1-low subset proved
the tool path. Unlike the first broad probe, that baseline accepted it
under the speed-prioritized gate: B2-high improves `1357/1307 -> 1343/1311`,
overrun `50 -> 32`, and target speed `96.315% -> 97.617%` while active
payload drops `663590 -> 574094`, cleanup restore bytes `439186 -> 80522`,
runtime restore bytes `438988 -> 116648`, and upload bytes
`26753280 -> 24341120`. The tradeoff is small CD pressure growth
(`blocking/refill 49/26 -> 51/18`, loop reads `40 -> 44`, due `5 -> 7`);
VISITOR3 high/low and WALKSTUF1 high/low stay exact-flat in the canonical
five-yellow canary. Artifact:
`scratch/ps1-perf-iterate/building2-high-prev-visible-cleanup-promote-five-yellow/20260521-000235-1299970/summary.json`.
The current BUILDING2 high preserve-entry-size screen-clip promotion keeps the
previous-visible cleanup row's table `dataSize`, file size, LBA, and CD phase
fixed while clipping screen-invisible compact spans inside the payload. It
changes `25` entries, removes `40166` draw pixels and `1982` cleanup pixels,
and improves B2-high `1343/1311 -> 1343/1312`, overrun `32 -> 31`,
blocking/refill `51/18 -> 50/17`, and target speed `97.617% -> 97.692%`.
Artifact:
`scratch/ps1-perf-iterate/b2high-screen-clip-preserve-entry-five-yellow-current/20260521-014550-1889928/summary.json`.
The first broad W1-high preserve-entry-size retry is closed as log-only. It
reduced logical active payload `848331 -> 777722` and improved visible
scene/loop `1808/1472 -> 1806/1470`, but failed the no-regression gate on
blocking/refill `43/13 -> 44/14`; paired `{372..388}` and `{272..284}` read
groups did not recover the debt. Reopen as narrower frame subsets or with
generated deadline/refill ownership, not as another broad clip.
The current broad W1-low preserve-entry-size screen-clip pass is promoted as
work/refill headroom. It changes `63` entries with fixed physical layout,
reduces logical active payload `755808 -> 712808`, removes `73798` cleanup
pixels and `39618` draw pixels, and keeps W1-low `1470/1446`, overrun `24`,
blocking `32`, and due `4` while improving hidden refill `4 -> 3`. Artifact:
`scratch/ps1-perf-iterate/w1low-screen-clip-preserve-entry-five-yellow-current/20260521-021156-2038863/summary.json`.
The next larger W1-low entry60/source frame81 preserve-offset trim is closed
as log-only on this baseline: it improved visible scene/loop/blocking
(`1809/1470 -> 1807/1468`, blocking `33 -> 28`) but regressed hidden refill
`5 -> 10` and reads `24 -> 27`; paired scalar `{199..211}` and `{204..220}`
owners did not recover the refill debt. Reopen only with generated
refill-budget ownership.
The current follow-up closes the first refill-recovery ladder for that same
entry60 signal. A low-only 4-VBlank scheduler guard fixed hidden refill
(`5 -> 2`) but regressed W1-low to `1811/1472/1446`, blocking `40`, and due
`6`; pairing entry60 with `{229..241}` left the bare hidden-refill failure
intact; guard plus `{229..241}` held loop/target flat but regressed blocking
`33 -> 39`; guard plus `{160..176}` was worse at `1817/1478/1447`,
blocking `46`. Entry60 now needs a generated per-frame refill/visible-budget
owner, not another scalar slack guard or static read row.
The window-size recovery ladder for W1-low entry60 is also closed. Reapplying
the entry60/source frame81 trim and widening only
`FG_WALKSTUF1_LOW_RESIDUAL_WINDOW_BYTES` to `48 KiB`, `56 KiB`, and `64 KiB`
all produced the same visible-positive/refill-negative profile:
`1809/1470/1446 -> 1807/1468/1445`, overrun `24 -> 23`, blocking `33 -> 28`,
due `4 -> 3`, but hidden refill `5 -> 10` and loop reads `24 -> 27`. The
`{204..210}` scalar row and an added 4-VBlank low guard were inert under the
48 KiB window. Do not retry scalar W1-low resident-window growth for entry60;
the remaining path is generated deadline/refill ownership or a different
pack/render work reduction.
The prepared-frame/refill-slack recovery ladder for entry60 is closed as well.
Allowing `walkstuf1` to prepare staged visual work at slack `5` preserved the
visible win but kept the same hidden-refill failure (`5 -> 10`, reads
`24 -> 27`). Restoring normal prepare timing and raising W1-low refill
minimum slack to `4` fixed hidden refill (`5 -> 2`) but traded it into visible
blocking and due misses (`1809/1470/1446 -> 1811/1472/1446`, blocking
`33 -> 40`, due `4 -> 6`). Entry60 should not be retried with scalar prepare
or min-slack controls; it needs generated frame/deadline/refill ownership or a
new render/data-shape reduction that creates slack first.
The preserve-offset restore-minus-current rescan is now closed for the current
five-yellow set. After hardening the transformer to copy unparseable payloads
and emit JSON summaries, the scan found zero same-offset shrinkable entries in
`BUILDING2`, `VISITOR3` high/low, or `WALKSTUF1` high/low; remaining candidates
were already unparseable/no-op or would grow. Keep the hardened scanner for
future data-shape work, but do not spend emulator time on another current-pack
restore-minus-current pass without a new transform family.
The broad previous-visible cleanup family is no longer closed globally. The
B2-high whole-pack form is promoted on the current baseline because it converts
large cleanup/upload work into a real loop/target/overrun win, while VISITOR3
high/low and W1-high broad forms remain phase-negative and W1-low remains
limited to the accepted late subset. Future previous-visible retries should be
scene-specific canaries with explicit loop/target/restore accounting, not blind
whole-pack sweeps.
The local-LZ sector-collapse swing is also closed as a standalone yellow-row
speed path. W1-high entry `55` saved two sectors but regressed loop/blocking and
hidden refill; B2-high entries `89..91` saved six modeled sectors but regressed
loop by `12` VBlanks; W1-low entry `60` reproduced the known visible-positive
but refill-negative entry60 profile (`1470/1446 -> 1468/1445`, blocking
`33 -> 28`, refill `5 -> 10`). Local-LZ may still be useful on rows with more
slack, but these current yellow candidates need generated deadline/refill
ownership or a preceding render/restore reduction before retrying.
The allocator-era W1-low entry60 local-LZ retest is now closed under the newer
`160..176` fresh-owner baseline. The raw `L4` frame60 form and the strongest
modeled `{204..220}` static recovery row produced the same profile:
`1809/1470/1446 -> 1807/1468/1443`, overrun `24 -> 25`, blocking
`32 -> 27`, refill `4 -> 13`, reads `24 -> 28`, and due `4 -> 2`.
This preserves the visible/blocking signal but makes the target/refill trade
worse than the current baseline, so entry60 remains generated-deadline-only.
The priority tooling now marks all `46` current direct-read candidates as
phase traps and surfaces the intended non-scalar lanes in both the under-green
CSV and read-candidate matrix. The refreshed queue is therefore ordered around
custom VISITOR3 terminal data shape / generated deadlines, BUILDING2
frame-deadline data-shape or render reduction, and WALKSTUF1 no-decode
canonicalization or generated owner work before any more scalar range retries.
The latest VISITOR3-low retained-setup tail swing is also closed. Extending the
accepted `206..232` segment to `206..241` would cover the frame133 terminal
payload but exhausts TRANSIENT before `JCPERF2` (`req=176128`, `have=168956`).
The memory-neutral swap to `215..241` completed but regressed low tide from
`1369/1065/1039` to `1371/1067/1038`, overrun `26 -> 29`, blocking
`75 -> 79`, and did not improve any key metric. Keep the accepted `206..232`
coverage; VISITOR3 low now needs generated deadline/refill ownership or a
custom terminal row-reference shape, not another retained setup slide.
The latest B2-high setup-resident duplicate alias points entries `141` and `142`
at already-retained duplicate payloads for entries `116` and `118`. The
five-yellow canary stays exact-flat at B2-high `1621/1347/1313`, overrun `34`,
blocking/refill `39/16`, reads/due `43/5`, fixed pack LBA, and the same
`233472` byte PS-EXE bucket, while uncovered active ownership drops
`286/519400 -> 284/518994`. This is banked as same-speed payload ownership, not
a VBlank speed win.
The follow-up B2-high entry38/source frame45 setup-edge duplicate alias is
banked as a layout-neutral cleanup row. It points entry `38` at duplicate entry
`35`'s payload offset `73149` (`2743` bytes), keeps file size/LBA and the
`233472` byte PS-EXE bucket fixed, and keeps both the focused B2-high proof and
canonical five-yellow canary exact-flat: B2-high remains `1621/1347/1313`,
overrun `34`, blocking/refill `39/16`, reads/due `40/5`, while VISITOR3
high/low and WALKSTUF1 high/low also stay exact-flat. Focused artifact:
`scratch/ps1-perf-iterate/building2-high-setup-alias38-current/20260520-144112-2313137/summary.json`;
canary artifact:
`scratch/ps1-perf-iterate/building2-high-setup-alias38-canary/20260520-144415-2330463/summary.json`.
The broader pair alias for entries `38` and `41` is closed: it preserved layout
but regressed B2-high to `1630/1357/1311`, overrun `46`, blocking/refill
`52/19`, reads `42`, and due `6`.
The follow-up B2-high entry33/source frame40 setup alias is closed. Although it
duplicates setup-resident entry30/source frame37 and preserved pack LBA plus the
PS-EXE bucket, it regressed B2-high to `1622/1349/1309`, overrun `40`,
blocking/refill `46/19`; this duplicate is phase-bearing across the setup-edge
boundary, so it should not be retried without generated deadline ownership.
The follow-up B2-high entries `59`/`60` forward alias into entries `62`/`63`
is also closed. It preserved pack LBA and the PS-EXE bucket, but moved identical
payloads across an early grouped-read boundary and regressed B2-high to
`1625/1351/1311`, overrun `40`, blocking/refill `44/19`, and reads `45`.
The B2-high hot active duplicate alias is closed too: pointing entries `137`,
`156`, `162`, and `164` backward to identical active-cluster payloads preserved
pack size/LBA and source layout but regressed B2-high to `1659/1370/1311`,
overrun `59`, blocking/refill `76/17`, reads `50`, and due misses `11`.
Duplicate-payload relocation now needs setup residency, forward-order physical
ownership, or generated deadline/refill ownership; backward aliases inside the
hot `248..261` cluster are explicitly off the path.
The B2-high first setup-edge extension is closed as well. Extending setup
coverage from `3..35` to `3..43` made the boundary payloads resident and cut
uncovered active entries `284 -> 276`, but it regressed to `1641/1348/1310`,
overrun `38`, blocking `44`; setup-edge residency is not free enough without a
clean/allocator byte reduction or generated cadence proof.
Future B2-high relocation should only reuse setup-resident ownership, preserve
forward active order, or use generated deadline/refill metadata that proves the
evicted/shifted boundary is harmless.
The restore-span tile-hoist source variant is closed after exact-flat W1-high
and VISITOR3-high gates. It stayed in the same PS-EXE bucket but only shifted
hot symbol addresses by about `-8` bytes and did not move loop, overrun,
blocking, or refill metrics. Restore-side wins now need generated/pack-owned
metadata or a bigger renderer redesign that changes measured restore/upload
work, not local loop hoisting.
The current-baseline VISITOR3-low window-size retest is closed: `20 KiB`
reduced reads/due but regressed loop and overrun, while `12 KiB` preserved
overrun but regressed blocking/due. Keep the accepted `16 KiB` low-tide
clean-relief window until generated ownership or pack shape changes the due
cluster.
The
under-green canary refresh now stamps W1 high/low at `1472/1441` and
`1470/1446`, B2 high/low at `1343/1312` and `1327/1318`, and B4 high/low at
`2843/2816` and `2847/2820`; BUILDING4 high and low are now green at `99.05%`.
BUILDING2 high's latest guarded `271..287` read-group promotion improves
loop/overrun/blocking/read-time/due to `1347/34/41/203/6` with the accepted
hidden-refill tradeoff `14 -> 16`. The follow-up guarded `315..327` row is a
same-loop pressure win: B2-high stays `1347/1313`, overrun `34`, and refill
`16`, while blocking drops `41 -> 39`, loop reads `47 -> 45`, read time
`203 -> 199`, and due `6 -> 5`. The later `{185..197}` row keeps loop speed,
blocking, refill, and due exact-flat while reducing loop reads/read time
`45/199 -> 43/197`; the newest `{158..174}` row keeps the same timing while
cutting CD pressure again to `40/189`; the entries `141/142` setup duplicate alias keeps timing
flat while reducing active uncovered owners/payload `286/519400 -> 284/518994`.
`{287..311}` was exact-flat with no key win, and `{249..261}` regressed
blocking and loop reads. The newest gap-8 upload retune improves
BUILDING4 low `2853/2816 -> 2849/2816`, overrun `37 -> 33`,
blocking/refill `42/35 -> 38/31`, and read time `223 -> 222`; gap `11`
was rejected as a one-VBlank regression against gap `8`.
The newest VISITOR3 low raw-gap promotion moves frame `138`'s raw payload into
the paid gap at offset `457772` and extends the third low setup segment from
`206..230` to `206..232`, improving low `1074/1039 -> 1065/1039`, overrun
`35 -> 26`, blocking/read time `85/103 -> 75/97`, reads `19 -> 18`, and due
`15 -> 14` while keeping hidden refill `0`.
The newest W1-low frame132 payload trim removes one compacted low-pack draw
tail without changing pack size/LBA and improves low again
`1473/1447 -> 1470/1445`, overrun `26 -> 25`, blocking/read time
`41/186 -> 35/163`, loop reads `36 -> 31`, due `5 -> 4`, and target speed
`98.235% -> 98.299%` while hidden refill stays `7`.
The newest W1-low `{378..390}` read-group promotion keeps loop flat and turns
that payload baseline into `1470/1446`, overrun `24`, blocking/refill
`34/6`, loop reads/read time `30/159`, due `4`, and target speed
`98.367%`. The rejected wider batch `{113,129}`/`{136,160}`/`{160,184}` plus
`{378,390}` saved reads but regressed hidden refill `7 -> 15`, so only
`{378,390}` is retained.
The newest W1-low setup/owner promotion revisits `{113,129}` only after moving
setup coverage to `244..350` plus a split `179..185` setup edge. That narrower
shape passes the focused and five-yellow gates at the same `1470/1446` target
speed while improving scene `1812 -> 1809`, blocking/refill `34/6 -> 33/5`,
and loop reads/read time `30/159 -> 26/150`. The newest `{355..371}`
retained-group promotion is a same-speed work-volume win: W1-low remains
`1809/1470/1446`, overrun `24`, blocking/refill `33/5`, and due `4`, while
loop reads/read time drop `26/150 -> 24/147`. The later entry65/source frame96,
entry39/source frame49, entry55/source frame65, entry56/source frame67,
entry59/source frame78, entry63/source frame91, entry66/source frame100, and
entry85/source frame148
preserve-offset trims keep
every speed-bearing metric exact-flat while reducing active payload another
`24115` bytes to `764658`; the later previous-visible cleanup subset reduces
it again to `755808`, so they are banked as payload/restore work only and do
not change speed totals. The follow-up raw/min-slack
`160..176` static owner probe is closed: raw and slack8 moved loop/blocking but
regressed hidden refill, while slack32 was exact-flat/inert. The fresh owner
form is now promoted only for the narrower `160..176` cluster: W1-low stays
`1809/1470/1446`, overrun `24`, loop reads/read time `24/146`, and due `4`,
while blocking improves `33 -> 32` and refill improves `5 -> 4`. The broad
fresh-owner `129..153` lane is closed; it improved blocking/due only by taking
target/refill debt (`1446 -> 1444`, refill `5 -> 14`), and slack8/slack12 were
worse. The follow-up direct-stage owner lane also closed `129..153`, prior
`160..176` forms, and `355..379`. The distinct retained-group `{129..153}` form is now closed too:
raw and slack8 saved reads but regressed target/refill/visible blocking, while
slack32 was exact-flat/inert. The follow-up `153..177` retained-group probe is
also closed:
raw and slack8 improved loop/blocking but regressed hidden refill and target,
while slack32 was exact-flat/inert. The follow-up `153..177` fresh metadata
owner after the scene-ID code-headroom pass is closed too: it improved loop
`1470 -> 1468` but tightened target `1446 -> 1444`, regressed blocking
`32 -> 33`, regressed refill `4 -> 8`, and grew `fgRuntimeFillWindowForEntry`
by `40` bytes. The smaller `147..171`, `153..169`,
`142..154`, `136..148`, and `147..163` rows are now closed too. Raw/static
forms either regressed target/refill/visible blocking or went exact-flat under
high slack guards, so the W1-low static retained-read table lane is exhausted.
Remaining W1-low work should be treated as generated-ownership/data-shape
leads, not direct table rows.
The latest W1-high `345..361` sweep closes the last open scheduler-owned row
from that refresh: raw grouping saved reads but regressed visible cadence,
slack8 was inert, and direct-stage ownership regressed blocking/due with hot
code growth.
The prior W1-low first-boundary read-group promotion reopens the previously
rejected `{91,107}` range on the newer clean-rect/setup-edge baseline and
improves low again `1475/1443 -> 1473/1444`, overrun `32 -> 29`,
blocking/read time `48/200 -> 43/195`, refill `12 -> 11`, reads `39 -> 36`,
and due `6 -> 5`.

Latest promoted VISITOR3 high same-speed setup-edge slide: slide the early
retained setup edge from `40..47` to `42..49` after the setup-edge merge.
Focused proof stays exact-flat at `1391/1070/1046`, overrun `24`, blocking
`35`, hidden refill `0`, and due `2`, while reducing loop reads/read time
`5/61 -> 4/59` and hidden CD VBlanks `26 -> 24`. The five-yellow canary at
`scratch/ps1-perf-iterate/visitor3-high-setupseg42-49-canary/20260520-171420-3186250/summary.json`
keeps VISITOR3 low, BUILDING2 high, and WALKSTUF1 high/low exact-flat. The
left neighbor `41..48` is timing-flat but no better than the prior read count,
and the right neighbor `43..50` regresses to `1398/1077/1042`, overrun `35`,
blocking `44`, and refill `10`. This is a CD-pressure baseline, not a VBlank
speed win; public/raw rollups and bands remain unchanged.

Prior promoted VISITOR3 high setup-edge merge: merge terminal coverage into
retained `203..262` and spend the largest allocator-safe spare retained edge on
`40..47`. Focused proof improves `1388/1071/1045 -> 1391/1070/1046`, overrun
`26 -> 24`, hidden refill `1 -> 0`, loop reads `7 -> 5`, and due stays `2`
while blocking stays `35`; the five-yellow canary leaves VISITOR3 low,
BUILDING2 high, and WALKSTUF1 high/low exact-flat. The `36..48` shape is
closed as structural allocator pressure (`req=178176 have=168956`), and
`40..46` passed but was inferior (`1071/1044`, overrun `27`, reads `4`).

Current allocator-era big-swing queue after closing the scalar grouped-read,
single-frame D4, duplicate-alias, isolated trim, and sequential-Setloc lanes:

1. **BUILDING2 frame-deadline data-shape / render-reduction pass.** The
   current read matrix has no safe standalone direct-read rows; B2-high's top
   remaining candidates (`310..322`, `135..159`, `318..334`, `310..326`) are
   scheduler-owned only. The next big swing should keep the cleanup/render win
   and remove either frame-deadline restore/upload work or create generated
   ownership that cannot fire into visible blocking.
2. **Generated deadline/refill owner metadata sidecar.** Build a no-hot-C or
   code-size-neutral generator that emits per-scene append-start, deadline, and
   refill-budget metadata, then gates reads by frame/deadline ownership instead
   of static sector ranges. First targets: B2-high `249..261` / `287..311`,
   VISITOR3-low terminal/early clusters, VISITOR3-high `103..127`, W1-high
   late `365..389`, and W1-low `142..160` / `153..177` follow-ups around the
   promoted `160..176` owner pocket. Static ranges repeatedly saved reads but
   moved cost into visible blocking/refill, so the next promotion must prove
   ownership before firing.
3. **VISITOR3 custom terminal data shape.** Replace more scalar V3-low/high
   retries with a pixel-perfect row-reference or setup-dictionary codec for the
   terminal frames that still drive due misses. The simple `134..136` sector
   alignment and early static read groups are closed; the next swing must reduce
   terminal bytes or reads without changing physical cadence blindly.
4. **WALKSTUF1 no-decode pack canonicalization.** Attack W1-high and W1-low
   with pack-side row-span/offset canonicalization that preserves pixels and
   avoids D4 runtime decode. W1-high D4 frames `181/183/185/187` saved bytes but
   moved cost into visible blocking; W1-low isolated preserve-offset trims are
   exact-flat but too small. Favor sector-boundary changes only when a canary
   proves no target/refill debt.
5. **BUILDING2 high frame/deadline-owned data-shape.** Keep B2-high focused on
   generated ownership plus selective no-decode relocation. Blanket duplicate
   aliasing and isolated entries `89..91` regressed despite byte savings, while
   held-slack and prefetch-only gates proved too coarse.
6. **Render/restore work reduction with static pack ownership.** Revisit clean
   restore/upload cuts only when the data is generated or pack-owned, not a hot
   runtime cache. The remaining yellow rows still have enough restore/upload
   work that a real static-background or sprite-local restore reduction could
   pay off without touching CD phase.
7. **Code-headroom/source-headroom promotions.** Promote exact-flat hot-code
   shrink and work-volume rows even without speed wins when they keep pack LBAs
   fixed and do not change timing. This compounds by making future generated
   owner/data-shape code less likely to cross the PS-EXE bucket.
8. **Allocator-aware sidecar validation.** Every sidecar or setup-residency
   retry must pass the current allocator-era clean-rect/CACHE/TRANSIENT shape.
   Additive retained setup failed structurally in W1-low and B2-high, so memory
   viability is now a first-class gate, not a follow-up check.
8. **Measured attribution before more scalar guesses.** Add or reuse per-row
   attribution for dirty upload, restore, frame index, CD wait, and group-hit
   identity before spending emulator time on another local range. Hand-authored
   tables are demoted unless this attribution proves a new ownership class.

Latest promoted code-headroom row: caching the active foreground scene ID
removes repeated hot-path scene-name compares from the scheduler while keeping
the five-yellow canary exact-flat. The proof
`scratch/ps1-perf-iterate/hot-scene-id-five-yellow-current/20260520-215337-573156/summary.json`
keeps B2-high, VISITOR3 high/low, and WALKSTUF1 high/low timing, CD pressure,
pack LBAs, and the `233472` byte PS-EXE bucket fixed, while shrinking
`foregroundPilotPlay` by `84` bytes, `fgRuntimeLoadSceneFrame` by `52`,
`fgRuntimeFillWindowForEntry` by `24`, and `fgRuntimeTryPrefetchWindow` by
`12`. This is not a speed win, but it is now the hot-code headroom baseline for
generated-owner and custom data-shape work.

Prior promoted code-headroom row: lowering `GR_UPLOAD_BAND_MERGE_GAP` from
`8` to `0` keeps the focused W1-high/low proof and five-yellow canary
exact-flat while shrinking `grDrawBackground` by `36` bytes and leaving the
PS-EXE bucket and pack LBAs fixed. This is not counted as a speed win, but it
is now the default source-headroom baseline for the generated-owner and
render/data-shape swings above. Artifacts:
`scratch/ps1-perf-iterate/upload-band-gap0-walkstuf1-current/20260520-192341-3919459/summary.json`
and
`scratch/ps1-perf-iterate/upload-band-gap0-five-yellow-current/20260520-192708-3939015/summary.json`.

Latest promoted W1-low fresh owner pocket: the generated-owner lane now keeps
only the `160..176` fresh-window cluster, gated to real prefetch slack and
frames `101..111`. It keeps W1-low at `1809/1470/1446` and the same
`24/146` loop reads/read time, while cutting blocking/refill `33/5 -> 32/4`.
The five-yellow canary shows VISITOR3 high/low, B2-high, and W1-high
exact-flat; pack LBAs and the `233472` byte PS-EXE bucket stay fixed.
Artifacts:
`scratch/ps1-perf-iterate/walkstuf1-low-owner160-176-fresh-current/20260520-200456-4153242/summary.json`
and
`scratch/ps1-perf-iterate/walkstuf1-low-owner160-176-fresh-five-yellow-current/20260520-200639-4163023/summary.json`.

Latest rejected narrow BUILDING2 high generated-owner retry: a fresh-window
owner for the modeled low-risk `249..261` cluster was inert because the
opportunity is an append seam, not a fresh read. Moving the same owner into the
append-extension path fired but failed: B2-high regressed
`1621/1347/1313 -> 1625/1352/1309`, overrun `34 -> 43`, blocking
`39 -> 45`, hidden refill `16 -> 20`, and loop reads stayed `40`; the
perf-embedded build also crossed the PS-EXE sector bucket
`233472 -> 235520`. Artifacts:
`scratch/ps1-perf-iterate/building2-high-fresh249-261-current/20260520-202900-94475/summary.json`
and
`scratch/ps1-perf-iterate/building2-high-owner249-261-current/20260520-203213-113320/summary.json`.
Close B2-high `249..261` hot scheduler ownership on this baseline. The next
B2-high swing should reduce pack/render work or use no-hot-C generated
deadline/refill metadata, not another C-side append owner.

Latest rejected BUILDING2 high prepared-idle generated-owner probe: a
B2-high-only owner table for `{95..119}`, `{122..146}`, `{249..265}`,
`{255..271}`, and `{287..311}` fired only while a prepared frame was already
waiting and no immediate payload read was needed. This was intentionally
different from static groups and visible due fills, but it still failed the
focused gate: B2-high regressed `1621/1347/1313 -> 1625/1352/1309`, overrun
`34 -> 43`, blocking `39 -> 45`, hidden refill `16 -> 20`, while loop reads
stayed `40`. The helper also shifted `BUILDING2.FG2` LBA `6189 -> 6190` and
crossed the PS-EXE sector bucket `233472 -> 235520`. Artifact:
`scratch/ps1-perf-iterate/building2-high-owned-append-current/20260520-173726-3315756/summary.json`.
Close prepared-frame-idle runtime ownership for B2-high. The remaining
generated path must be frame-index/deadline/refill-budget metadata emitted
outside the hot foreground scheduler, or B2-high must first get a pack/render
reduction that creates real slack before the owner row fires.

Latest rejected BUILDING2 high entry89 trim: the isolated preserve-offset
draw-tail trim for entry `89` / source frame `107` removed `4510` active bytes
and collapsed the entry from `5 -> 3` sectors while keeping file size, pack LBA,
runtime source, and the `233472` byte PS-EXE bucket fixed. It still failed the
focused gate: B2-high regressed `1621/1347/1313 -> 1622/1349/1312`, overrun
`34 -> 37`, blocking `39 -> 42`, hidden refill stayed `16`, and the only win
was loop reads `40 -> 39`. Artifact:
`scratch/ps1-perf-iterate/building2-high-trim-entry89-current/20260520-174210-3342501/summary.json`.
Close entry89/source frame107 as a standalone trim; entry90 remains the only
untested isolated member of the current `89..91` sector-collapse split.

Latest rejected BUILDING2 high entry90 trim: the remaining isolated
preserve-offset draw-tail trim in the `89..91` split removed `4080` active
bytes from entry `90` / source frame `109` and collapsed the entry from `5 -> 3`
sectors with file size, pack LBA, runtime source, and the `233472` byte PS-EXE
bucket fixed. It reproduced the entry89 failure profile: B2-high regressed
`1621/1347/1313 -> 1622/1349/1312`, overrun `34 -> 37`, blocking `39 -> 42`,
hidden refill stayed `16`, and loop reads improved only `40 -> 39`. Artifact:
`scratch/ps1-perf-iterate/building2-high-trim-entry90-current/20260520-174621-3366150/summary.json`.
Close the current `89..91` draw-tail sector-collapse split under strict B2-high
gates; the next B2 data-shape path needs generated deadline/refill ownership or
non-CD render/upload reduction first.

Latest rejected VISITOR3 low read-group probe: the current read-plan's only
low-risk saved-read candidate, `{86..98}`, was current-window-compatible and
modeled as a scheduler-owned candidate with one saved read. The focused source
probe did save that read (`18 -> 17`) but regressed VISITOR3 low
`1369/1065/1039 -> 1374/1070/1041`, overrun `26 -> 29`, blocking `75 -> 79`,
and due stayed `14`, with hot foreground symbols shifted by `+60` bytes.
Artifact:
`scratch/ps1-perf-iterate/visitor3-low-rg86-98-current/20260520-175126-3395181/summary.json`.
Close `{86..98}` as a hand-authored read group. VISITOR3 low still needs
frame/deadline/refill ownership or pack/render-byte slack before grouped reads
are viable.

Latest rejected WALKSTUF1 high D4 payload swing: frames `181`, `183`, `185`,
and `187` delta-compress against the prior frame and save `11465` active bytes
with fixed pack size, pack LBA, and PS-EXE bucket. The focused gate still
regressed W1-high `1808/1472/1441 -> 1809/1473/1441`, overrun `31 -> 32`,
blocking `43 -> 56`, and loop reads `41 -> 42`, while hidden refill improved
only `13 -> 10`. Artifact:
`scratch/ps1-perf-iterate/walkstuf1-high-d4-181-187-current/20260520-180155-3453957/summary.json`.
Close this combined D4 cluster as a standalone speed path; retry only as
single-frame canaries or with deadline-owned reads that keep the delta decode
off the visible blocking path.

The first binary split is also closed: frame `187` alone shrinks `4764 -> 731`
bytes, saving `4033` active bytes, but the focused gate stayed exact-flat at
W1-high `1808/1472/1441`, overrun `31`, blocking/refill `43/13`, reads/due
`41/7`, so `--require-improvement` rejected it. Artifact:
`scratch/ps1-perf-iterate/walkstuf1-high-d4-frame187-current/20260520-180605-3477710/summary.json`.
Frame `181` is also closed: it shrinks `4708 -> 1484` bytes and improves
scene/loop/overrun/refill `1808/1472/31/13 -> 1807/1471/29/9`, but regresses
blocking `43 -> 54`, loop reads `41 -> 43`, and due misses `7 -> 10`. Artifact:
`scratch/ps1-perf-iterate/walkstuf1-high-d4-frame181-current/20260520-181012-3501246/summary.json`.
Frame `183` is closed too: it shrinks `4602 -> 2251` bytes, but regresses
W1-high `1808/1472/1441 -> 1813/1477/1441`, overrun `31 -> 36`, blocking
`43 -> 47`, reads/due `41/7 -> 42/8`, with only refill improving `13 -> 11`.
Artifact:
`scratch/ps1-perf-iterate/walkstuf1-high-d4-frame183-current/20260520-181306-3518065/summary.json`.
Frame `185` closes the split: it shrinks `4483 -> 2626` bytes, but regresses
W1-high `1808/1472/1441 -> 1811/1475/1441`, overrun `31 -> 34`, blocking
`43 -> 45`, reads/due `41/7 -> 42/8`, with only refill improving `13 -> 11`.
Artifact:
`scratch/ps1-perf-iterate/walkstuf1-high-d4-frame185-current/20260520-181557-3534550/summary.json`.
Close hand-added W1-high D4 for this cluster; move to no-decode payload shrink,
clean/upload/restore reduction, or generated deadline ownership.

Latest promoted VISITOR3 high window-size retune: the 80 KiB clean-relief
stream window passes the focused and five-yellow gates, moving V3 high from
`1075/1044` to `1071/1045` with overrun `31 -> 26`, blocking `45 -> 35`,
refill `3 -> 1`, and due `3 -> 2`. Keep the 80 KiB relief window and the
56 KiB tight cap as the new V3-high baseline. The 96 KiB variants are closed:
they reduced loop reads but tightened target cadence enough to regress strict
promotion gates.
Latest rejected VISITOR3 high prepare-before-window owner probe: mirroring the
accepted W1-low scheduler priority onto high-tide VISITOR3 was exact-flat
against the current five-yellow baseline. Focused V3 high stayed
`1388/1071/1045`, overrun `26`, blocking/refill `35/1`, reads/due `7/2`, and
all scheduler counters matched baseline while `foregroundPilotPlay` grew by
`68` bytes. Artifact:
`scratch/ps1-perf-iterate/visitor3-high-prepare-first-current/20260520-015421-2155850/summary.json`.
Close this local priority inversion for V3 high; the remaining early
`40..64` family still needs generated append/deadline ownership or pack/render
data-shape work, not another prepare-first branch.
Latest rejected BUILDING2 high stream-window-size swing: high-tide-only
`24 KiB`, `20 KiB`, and `12 KiB` windows were tested while retaining accepted
B2 read groups. The larger windows reduced active reads but regressed cadence
badly (`24 KiB` to `1631/1357/1312`, blocking/refill `55/28`; `20 KiB` to
`1637/1363/1311`, blocking/refill `66/26`). The smaller `12 KiB` shape improved
hidden refill `16 -> 13` but still regressed loop/blocking to
`1628/1355/1311`, blocking `52`, reads `57`, due `10`. Artifacts:
`scratch/ps1-perf-iterate/building2-high-window24-current/20260520-021817-2290475/summary.json`,
`scratch/ps1-perf-iterate/building2-high-window20-current/20260520-022006-2301046/summary.json`,
and `scratch/ps1-perf-iterate/building2-high-window12-current/20260520-022150-2310843/summary.json`.
Close scalar B2-high stream-window-size retuning; future B2-high work needs
generated deadline/refill-budget ownership, upload/restore reduction, or
no-decode relocation with harmless eviction.
Latest rejected clean-rect row-owner source swing: a conservative
`gGrCleanRectRowOwner[480]` cache let unique clean-rect rows skip the per-span
scan over split clean rectangles, but the source/helper growth crossed the
PS-EXE bucket `233472 -> 235520`, shifted `BUILDING2.FG2` LBA `6189 -> 6190`,
and regressed B2-high from `1621/1347/1313` to `1625/1351/1311`, overrun
`34 -> 40`, blocking/refill `39/16 -> 45/20`, with reads/due still `43/5`.
Artifact:
`scratch/ps1-perf-iterate/clean-rect-row-owner-b2-high-current/20260520-022946-2355589/summary.json`.
Close runtime clean-rect row-owner caching as a source-side fix; keep the
restore-scan idea only as generated metadata, pack-owned static bands, or a
layout-pinned graphics split.
Latest rejected global dirty-upload band swing: reducing the renderer's
dirty-row merge gap from `8` to `4` and then `0` kept B2-high and the full
five-yellow canary exact-flat on timing, layout, and the `233472` byte PS-EXE
bucket. Gap `0` saved B2-high upload bytes `21836800 -> 21205760`, W1-high
`17241600 -> 17103360`, and W1-low `17987840 -> 17881600`, but increased
VISITOR3 high/low upload bytes and did not move any loop/target/blocking/refill
metric. Artifacts:
`scratch/ps1-perf-iterate/building2-high-upload-gap4-current/20260520-031639-2620742/summary.json`,
`scratch/ps1-perf-iterate/building2-high-upload-gap0-current/20260520-031829-2630931/summary.json`,
and
`scratch/ps1-perf-iterate/upload-gap0-five-yellow-canary/20260520-032111-2646392/summary.json`.
Retest after the B2-high `{158..174}` promotion repeated the result: gap `0`
and gap `4` stayed exact-flat on all five yellow rows, with B2-high still
`1621/1347/1313`, VISITOR3 high/low still `1388/1071/1045` and
`1369/1065/1039`, and W1 high/low still `1808/1472/1441` and
`1809/1470/1446`. Gap `4` mildly reduced VISITOR3-low/B2/W1 upload bytes but
still did not move any timing metric; gap `0` still increased VISITOR3 upload
bytes. Retest artifacts:
`scratch/ps1-perf-iterate/upload-gap0-five-yellow-current/20260520-115957-1396578/summary.json`
and
`scratch/ps1-perf-iterate/upload-gap4-five-yellow-current/20260520-120905-1449371/summary.json`.
Close global upload-gap retuning as a 99% speed path; any retry should be
scene-local/generated and paired with CD/deadline ownership rather than a
global constant.
Latest rejected code-phase/link-order swing: moving `src/cdrom_ps1.c` before
`src/foreground_pilot.c` shifted foreground hot symbols by about `+10984` bytes
while keeping the PS-EXE bucket at `233472`, but the current five-yellow canary
was exact-flat on every speed-bearing metric. Artifact:
`scratch/ps1-perf-iterate/linkorder-cd-before-fg-canary/20260520-045809-3193268/summary.json`.
Close this coarse link-order bucket; future code-phase work needs narrower
function alignment/object padding controls or should be deferred behind
generated scheduler and pack-side work.
Latest rejected VISITOR3 high final hand-table row: the refreshed matrix left
only `{44..50}` unclosed. A high-tide-only route under the current `80 KiB`
clean-relief stream window stayed exact-flat at `1388/1071/1045`, overrun `26`,
blocking/refill `35/1`, reads/due `7/2`, with fixed pack LBA `22619` and
PS-EXE bucket `233472`, while foreground hot code grew/shuffled by `24` bytes.
Artifact:
`scratch/ps1-perf-iterate/visitor3-high-rg44-50-current/20260520-023650-2395484/summary.json`.
Close `{44..50}` as a static source row; the current yellow hand-table matrix
is now exhausted and read work should move to generated deadline/refill
ownership only.
Latest rejected W1-high preserve-offset pack swing: entry `55` / source frame
`65` was the only remaining W1-high draw-tail candidate in the current scan and
shrunk `4716 -> 2` bytes (`4 -> 1` sectors) with fixed pack size/LBA and no
source changes, but it regressed W1-high from `1807/1471/1440` to
`1811/1475/1438`, overrun `31 -> 37`, blocking/refill `56/13 -> 64/18`.
Artifact:
`scratch/ps1-perf-iterate/walkstuf1-high-trim-entry55-current/20260520-024222-2426984/summary.json`.
Close early W1-high entry55/source frame65 sector-collapse trimming; these
visually empty tail bytes are phase-bearing unless a generated owner can absorb
the shifted cadence.
Latest rejected WALKSTUF1 early-prepare scheduler swing: allowing `walkstuf1`
to prepare staged visual work at slack `5` as well as slack `4` kept both
remaining W1 yellow rows exact-flat (`1808/1472/1441` high and
`1809/1470/1446` low) while growing `foregroundPilotPlay` by `276` bytes and
shifting downstream hot symbols. Artifact:
`scratch/ps1-perf-iterate/walkstuf1-prepare-slack5-current/20260520-114109-1289612/summary.json`.
Keep the accepted slack-4 prepare point; W1 needs per-cluster deadline/refill
metadata or pack/render data-shape work, not the same visual prepare one held
VBlank earlier.
Latest rejected W1-low no-direct-stage retest: disabling the small-payload
direct-stage shortcut for low-tide `walkstuf1` on the current payload baseline
again improved due ownership (`due 4 -> 2`) but regressed target/refill:
`1809/1470/1446 -> 1811/1472/1444`, overrun `24 -> 28`, hidden refill
`5 -> 14`, and loop reads `24 -> 25`. Artifact:
`scratch/ps1-perf-iterate/walkstuf1-low-no-direct-stage-current/20260520-114656-1322913/summary.json`.
Keep direct-stage enabled until a generated owner can make this decision per
frame with an explicit refill budget.
Latest rejected VISITOR3 high active-window setup swing: adding `103..127` on
top of the accepted `203..262` and `277..293` setup coverage in CACHE exceeded
the allocator/clean-rect budget before `JCPERF2`; dropping the high-tide
clean-relief stream window made the run complete but regressed the row to
stage1-only behavior (`loop_reads 4 -> 114`, blocking `45 -> 315`). Keep V3
high on the accepted stream-window plus retained-segment shape. Reopen the
`103..127` active window only after reducing clean-rect/CACHE pressure or with
generated deadline ownership that does not sacrifice the stream window.
The follow-up V3-high clean-rect chunk sweep closes pure split-size retuning:
`96 KiB` and `80 KiB` regressed target/blocking, while `56 KiB` was exact-flat
with no material work-volume win. The accepted `64 KiB` cap remains the
baseline until clean area/bytes can be reduced rather than just split
differently.
The V3-high setup-prime window lane is also closed. `128 KiB` and `96 KiB`
reopened the CACHE/clean-rect allocator cliff, while `80 KiB` and the pure
no-extra-memory `68 KiB` form both completed but moved active CD phase
backward. Do not re-enable clean-relief setup priming unless it is paired with
real clean-area byte reduction or generated deadline ownership.

Pre-allocator historical all-scene rollup after the VISITOR3 high-only sparse frame-117
target-hull timing promotion, the v202/v206/v207 high re-anchor CD-pressure
promotions, the v204 low persistent setup segment, the v205 high persistent
setup segment, the v213/v214 high setup-prime expansions, the v216 guarded low
second setup segment, the v227 low frame-125/frame-126 resident re-anchor, the
v234 low frame-118 resident-copy follow-up, the v237 low frame-127
resident-copy follow-up, the v238 high frame-127/frame-130 resident-copy
follow-up, the v248 low frame-114/frame-117 no-op residual compaction
follow-up, the v249 low frame-113 no-op residual follow-up, the v277
WALKSTUF1 high sector `201..213` read-group follow-up, the v288 WALKSTUF1
high gap1/window-prefetch guard, the v289 WALKSTUF1 low gap1 prefix pack, the
v291 VISITOR3 high frame-140/tail setup-segment copy, the v292 VISITOR3
low no-op payload alias, the v299 VISITOR3 high frame-131 resident-alias
setup-prime copy, the v302 VISITOR3 low frame-128 resident segment copy, the
v305 WALKSTUF1 low gap6-prefix plus slack-guard promotion, the v316 WALKSTUF1
high `213..229` read-group/slack4 promotion, the v327 VISITOR3 low
frame128/frame129 resident-slot swap, the v331 WALKSTUF1 low
staged-prepare scheduler fallback, the v338 VISITOR3 low tail pack-only
compaction, the v340 WALKSTUF1 high `344..360` read-group promotion, the
v347 BUILDING2 high `249..257` read-group pressure reduction, the v354
JOHNNY6 compact-FGP3 metadata/restore-minus-current promotion, the v364
BUILDING6 scene-local slack4 window-refill guard, the v379 BUILDING2 high
`226..242` read-group promotion, the v383/v384 WALKSTUF1 high
`422..434` / `444..456` same-speed CD-work reductions, the v401 VISITOR5 high
current-layout refresh, the v428 WALKSTUF1 shared `443..455` / `444..456`
dual-tail CD-work reduction, the v441 BUILDING2 high `206..230` plus
24-sector grouped-read capacity promotion, the v445 BUILDING2 low `238..250`
retained-read promotion, the v451 VISITOR5 low compact FGP3/v4 plus
`23..47` retained-read promotion, the v452 VISITOR3 low frame129
custom D4 delta promotion, the v454 BUILDING2 low frame71/frame77
previous-frame D4 delta promotion, and the v458 WALKSTUF1 high current-control
refresh, the v460 VISITOR3 high frame137 previous-frame D4 CD-pressure
promotion, and the v462 VISITOR3 high frame132 D4 plus slack4 CD-pressure
promotion, the v464 VISITOR3 high one-sector frame132 setup-segment, the
v470 VISITOR3 low frame132 previous-frame D4 CD-pressure promotion, the
v474 WALKSTUF1 low `78..91` first post-prime boundary read-group promotion,
the v477 VISITOR3 low frame132 setup-prime gap relocation, the v496
VISITOR5 high `30..46` retained-read promotion, the v501 VISITOR3 high
frame137 sector-203 setup relocation, the v510 VISITOR3 low frame137
setup-prime gap relocation, the v526 VISITOR5 low `30..46` retained-read
promotion, the v626 BUILDING2 low `218..229` slack-8 retained-read promotion,
the v629 VISITOR3 high `277..293` tail-pack repack, the v652 BUILDING4 low
offscreen draw-span clipping pack pass, the v746 BUILDING4 low frame291
in-place work-volume shrink, the v653/v654 WALKSTUF1 high/low
late-tail work-volume clips, the v657 WALKSTUF1 high late-tail physical
compaction, the v660 BUILDING2 low offscreen work-volume clip, the v664/v698/v700/v701/v702/v703/v877/v879/v880/v887/v888/v889/v890/v891/v892/v896
BUILDING2 high offscreen work-volume clips, the v665/v666/v668/v669/v672/v673/v674/v675/v678/v680/v684/v685/v686/v687/v688/v689/v690/v691/v692/v693/v694/v695/v696/v716/v717/v718/v719/v720/v721/v722/v723/v724/v725/v726
WALKSTUF1 low isolated mid/left/pre-tail/mid-right/pre-left-edge/post-left/late-left2/frame65/post-left-singleton/mid-right-ad/ae/af/frame1/post-mid/frame3/frame140/frame61/frame60/frame62/frame59/frame58/frame63/frame133/frame132/frame5/frame141/frame131/frame19/frame6/frame142/frame130/frame145/frame129 offscreen work-volume clips, the v747..v910 WALKSTUF1 low in-place work-volume shrinks through frame106, the v760 bounded CD fast-poll recovery, the v705 WALKSTUF1 low late-tail subset physical compaction, plus the v932 JOHNNY1 local-LZ full-frame payload compression, v935 WALKSTUF1 low `209..225` retained-read row, and v954 BUILDING4 low entry30/entry33 local-LZ payload baseline:
`+0.2523%` public average over target / `99.7519%` public target speed across
all `126` timing-bearing rows. The raw signed optimization matrix is
about `-0.5148%` / `100.5342%`. Since the compact full-matrix baseline was about
`17.4%` over target / `87.1%` target speed, the headless methodology has
removed about `17.15` public over-target points and added about `12.65`
public target-speed points. At that pre-allocator checkpoint, green rows were
`119 / 126`, with `7` yellow rows remaining and no orange/red rows.

Latest promoted WALKSTUF1 high preserve-offset payload baseline:
`walkstuf1-high-frame51-inplace-v839`, `walkstuf1-high-frame49-inplace-v841`,
`walkstuf1-high-frame47-inplace-v842`, `walkstuf1-high-frame45-inplace-v843`,
`walkstuf1-high-frame43-inplace-v844`,
`walkstuf1-high-frame138-inplace-v882`, `walkstuf1-high-frame135-inplace-v884`, and `walkstuf1-high-frame139-inplace-v927` shrink entries `51` / source frame
`61`, `49` / source frame `59`, `47` / source frame `57`, `45` / source frame
`55`, `43` / source frame `53`, `138` / source frame `246`, `135` / source frame `243`, and `139` / source frame `247` inside
`WALKSTUF1.FG2` without moving any
payload offsets. Current allocator-era follow-ups also trim entry `136` /
source frame `244` from `3762 -> 2596` bytes and entry `57` / source frame
`70` from `3409 -> 320` bytes, removing two more payload sectors while keeping
W1 high exact-flat at `1471/1440`, overrun `31`, blocking/refill `57/13`, and
reads/due `45/10`. Entry `51` drops `5588 -> 770` bytes, entry `49` drops
`5269 -> 770` bytes, entry `47` drops `7399 -> 3158` bytes, entry `45` drops
`5276 -> 1657` bytes, entry `43` drops `5555 -> 2211` bytes, entry `138` drops
`4716 -> 3804` bytes, entry `135` drops `4415 -> 3850` bytes, entry `139` drops
`4011 -> 3668` bytes, and active payload drops `882007 -> 859666`; file size
stays `1535263`, pack LBA/sectors stay `24883/750`, and the PS-EXE bucket
stays `217088`. The strict speed gates correctly found no timing
improvement, while the no-regression work gates passed exact-flat at scene
`1764`, loop/target `1476/1434`, overrun `42`, blocking/refill `81/23`,
loop reads/read VBlanks `63/276`, and due misses `16`. Treat v839/v841/v842/v843/v844/v882/v884/v927 as
safe same-speed payload-work baselines, not VBlank speed wins; future W1-high
preserve-offset trims need the same exact-flat gate, while green conversion
still needs generated deadline/read ownership or upload/restore work.

Latest rejected WALKSTUF1 high preserve-offset payload probe:
`walkstuf1-high-frame55-inplace-v840` tried the largest remaining same-lane
entry after v839. The candidate shrank entry `55` / source frame `65`
`4716 -> 2` bytes and active payload `877189 -> 872475`, but the focused gate
regressed scene/loop/target `1764/1476/1434 -> 1780/1492/1422`, overrun
`42 -> 70`, blocking `81 -> 113`, refill `23 -> 27`, and due misses
`16 -> 18`, with loop reads still `63`. The pack was restored and rebuilt.
Close frame55 for now; size-only tail removals are not automatically
cadence-neutral once they alter frame deadlines/read ownership. The later
v927 frame139/source247 rerun resolved the earlier v885 structural uncertainty
and promotes as the current same-speed high payload baseline.
Do not retry remaining W1-high preserve-offset payload trims without a full
correctness summary.

Latest promoted WALKSTUF1 low payload/speed baseline:
v747/v749/v750/v751/v753/v755/v756/v757/v759/v762/v763/v766/v767/v769/v770/v771/v772/v773/v774/v775/v776/v777/v779/v780/v781/v782/v783/v784/v785/v786/v787/v788/v789/v790/v791/v794/v795/v797/v798/v800/v801/v802/v846/v847/v849/v852/v853/v855/v859/v860/v861/v862/v863/v864/v865/v866/v867/v868/v869/v870/v871/v872/v873/v874/v875/v876/v908/v909/v910
kept the current timing profile exact-flat through v855, then v859 converts a
fixed-sector shrink into a small speed win and v860/v861/v862/v863/v864/v865/v866/v867/v868/v869/v870/v871/v872/v873/v874/v875/v876/v908/v909/v910 stay exact-flat while shrinking frames `51`, `49`,
`47`, `61`, `62`, `58`, `45`, `37`, `35`, `43`, `41`, `57`, `33`, `67`,
`68`, `69`, `32`, `133`, `5`, `141`, `70`, `30`, `6`, `71`, `72`, `142`,
`73`, `131`, `74`, `19`, `28`, `138`, `145`, `75`, `76`, `77`, `130`,
`135`, `1`, `88`, `90`, `3`, `53`, `136`, `79`, `81`, `129`, `139`, `87`, `89`, `98`, `27`, `101`, `93`, `94`, `97`, `99`, `100`, `134`, `91`, `92`, `95`, `140`, `108`, `109`, `107`, `110`, `111`, and `106` in-place, preserving every payload
offset and the `1535263` byte pack footprint. Active payload drops
`879801 -> 790208`; the latest non-preserve frame132 trim then shrinks the
current compacted low pack to `788773` active payload while preserving the
`1535263` byte file footprint and pack LBA/sectors. The accepted `{378..390}`
read group then keeps the pack fixed while moving the current speed row to
`1470/1446`, overrun `24`, blocking/refill `34/6`, reads/read time `30/159`,
and due `4`; the current `{355..371}` follow-up keeps timing flat and lowers
reads/read time to `24/146`. The latest v910 frame
`106` / source frame `209` entry shrinks `944 -> 919` bytes and sector
coverage stays `2 -> 2`.
Scene/loop remain `1769/1477`, target improves `1431 -> 1432`, overrun
improves `46 -> 45`, blocking/refill move `64/20 -> 65/20`, loop reads stay
`58`, loop read VBlanks improve `266 -> 259`, and due misses stay `11`.
Treat v859 as the first speed-bearing fixed-sector payload win after the
v747..v855 same-speed lane, `walkstuf1-low-frame132-nonpreserve-current` as
the current payload-speed baseline, and `walkstuf1-low-rg378-390-current` as
the current read-pressure baseline; remaining W1-low speed conversion likely
needs more targeted boundary trims, generated deadline/read ownership, or a
pack/data-shape change.

Latest rejected WALKSTUF1 low no-sector payload probe:
`walkstuf1-low-frame85-inplace-v854` tested entry `85` / source frame `148`
after v853, shrinking `4854 -> 4351` bytes and active payload
`793537 -> 793034` while keeping modeled sector coverage `3 -> 3`.
The fixed-sector trim was still phase-negative: scene/loop/target regressed
`1769/1477/1431 -> 1773/1481/1428`, overrun `46 -> 53`,
blocking/refill `64/20 -> 86/28`, loop reads `58 -> 61`, and due
`11 -> 13`. The pack was restored and rebuilt. Close frame85 on the v853
baseline; remaining W1-low no-shift trims must keep strict timing gates, and
speed conversion still needs generated deadline/read ownership or a
pack/data-shape change.

Latest rejected WALKSTUF1 low near-empty/sector-collapse payload probes:
`walkstuf1-low-frame55-inplace-v856` tested the largest remaining
preserve-offset shrink after v855. Entry `55` / source frame `65` shrank
`4716 -> 2` bytes, active payload `793194 -> 788480`, and modeled sector
coverage `4 -> 1`, but the focused gate regressed scene/loop/target
`1769/1477/1431 -> 1775/1483/1430`, overrun `46 -> 53`,
blocking/refill `64/20 -> 73/21`, and due `11 -> 12` while loop reads stayed
`58`. `walkstuf1-low-frame60-inplace-v857` then tested the next largest
candidate, entry `60` / source frame `81`, shrinking `4432 -> 476` bytes,
active payload `793194 -> 789238`, and sector coverage `3 -> 1`; it regressed
scene/loop/target `1769/1477/1431 -> 1773/1481/1427`, overrun `46 -> 54`,
blocking/refill `64/20 -> 85/25`, loop reads `58 -> 60`, and due `11 -> 13`.
Both packs were restored and rebuilt. Close frame55 and frame60 on the current
W1-low baseline; combined with the W1-high frame55/frame56 misses, near-empty
or multi-sector-collapse preserve-offset rows should not be retried without
generated deadline/read ownership. Continue with fixed-sector/non-empty
exact-flat probes only if they are cheap, otherwise pivot to generated
ownership or another under-99 scene.

Latest rejected WALKSTUF1 low sector-boundary payload probes:
`walkstuf1-low-frame96-inplace-v850` tested entry `96` / source frame `176`
after v849, shrinking `4649 -> 4491` bytes and active payload
`794755 -> 794597` while changing modeled sector coverage `4 -> 3`.
That late-cluster sector-boundary trim was phase-negative: scene/loop/target
regressed `1769/1477/1431 -> 1775/1483/1431`, overrun `46 -> 52`, and
blocking/refill `64/20 -> 71/25`, while reads/due stayed `58/11`. After v855,
`walkstuf1-low-frame39-inplace-v858` tested a larger early one-sector shrink:
entry `39` / source frame `49` shrank `7835 -> 4724` bytes, active payload
`793194 -> 790083`, and sector coverage `4 -> 3`, but regressed
scene/loop/target `1769/1477/1431 -> 1783/1491/1427`, overrun `46 -> 64`,
blocking/refill `64/20 -> 79/30`, and loop reads `58 -> 64`; due improved
`11 -> 10`, but the visible timing regression dominated. The v916
`walkstuf1-low-frame132-inplace-v916` follow-up tested a later one-sector
shrink, entry `132` / source frame `240` (`4402 -> 2967`, sector coverage
`4 -> 3`), and kept scene/loop/target flat at `1769/1477/1432` but regressed
blocking/refill `65/20 -> 66/21` with no key improvement. All packs were
restored and rebuilt. Close frame96, frame39, and frame132 on their tested
baselines;
remaining W1-low no-shift trims should be fixed-sector exact-flat work-volume
checks, not speed candidates, unless generated deadline/read ownership changes
first.

Latest rejected WALKSTUF1 low scalar window-size probe:
`walkstuf1-low-window-scan-v897` / `v897b` tried prefetch windows `20 KB`,
`24 KB`, `28 KB`, `32 KB`, `48 KB`, and `64 KB` against the v876 baseline.
Every larger-than-default window reduced some loop reads but moved visible
cadence negative: baseline `1769/1477/1432`, overrun `45`, blocking/refill
`65/20`, reads/read time `58/259`, due `11`; the closest misses were
`20 KB` and `24 KB` at `1783/1489/1431`, overrun `58`, with blocking/refill
`78/29` and `64/28`. Wider windows regressed harder through `64 KB` at
`1836/1542/1443`, overrun `99`, blocking/refill `100/73`, reads/read time
`15/183`, due `2`. Close scalar W1-low window growth on the current baseline;
future W1-low green conversion needs generated read/deadline ownership or
selective upload/static work, not a larger raw resident window.

Latest rejected WALKSTUF1 low targeted read-group probe:
`walkstuf1-low-rg279-311-v900` added the largest refreshed modeled cluster as a
local `{279,311,0}` table row after scalar window growth closed. It reduced
loop reads `58 -> 51` and due misses `11 -> 10`, but the row did not produce a
group hit and shifted scheduling/layout negative: scene/loop/target regressed
`1769/1477/1432 -> 1784/1492/1431`, overrun `45 -> 61`,
blocking/refill `65/20 -> 77/31`, and loop-read VBlanks stayed effectively
flat `259 -> 255`. Close this region as a hand-authored local table insertion;
W1-low needs generated deadline/refill ownership or non-CD static work before
retrying large mid-tail read clusters.

Latest promoted BUILDING4 high setup-segment baseline: caching relative
sectors `264..288` during setup is the first allocator-era BUILDING4 high
promotion to cross 99%. The accepted canary
`scratch/ps1-perf-iterate/building4-high-setupseg264-288-canaries-pass/20260518-023050-2754222/summary.json`
improves active loop/target `2847/2816 -> 2843/2816`, overrun `31 -> 27`,
blocking/refill `36/32 -> 34/30`, and loop reads/read VBlanks
`49/256 -> 47/251`; due stays `1`. B4 low, W1 high/low, and B2 high/low
stayed exact-flat. The accepted setup trade is scene setup
`3115 -> 3121` (+0.19%), inside the 0.25% canary allowance. This removes
BUILDING4 high from the under-99 queue and leaves BUILDING4 low as the only
B4 yellow row.

Latest promoted BUILDING4 low speed baseline: v652 applies the successful part
of the offscreen draw-span lane directly to `BUIL4LOW.FG2`, preserving every
entry offset/size, the `1714154` byte pack footprint, LBA `9118`, and the
`217088` byte PS-EXE bucket. It changes `30` frames, removes `65111`
offscreen PAL4 draw pixels, drops `11871` spans and `471` draw rows, and
reduces logical draw bytes by `61047`. The same-commit gate improves scene
`3131 -> 3128`, active loop/target `2856/2816 -> 2853/2816`, overrun
`40 -> 37`, blocking `44 -> 40`, prefetch overrun `37 -> 34`, and loop-read
VBlanks `223 -> 215`; loop reads and due misses remain `30` and `1`. This
adds about `0.0008` public over-target points removed and `0.0009`
target-speed points, leaving BUILDING4 low yellow at `98.70%` with `37`
VBlanks of remaining gap.

Latest promoted BUILDING4 low work-volume baseline: v746/v806/v807/v808/v811/
v813/v814/v815/v816/v827/v893/v895/v903/v911/v912/v913/v924/v928 keep the v652 timing profile exact-flat while
shrinking frames `291`, `34`, `31`, `290`, `32`, `35`, `36`, `37`, `288`,
`287`, `39`, `286`, `292`, `426`, `41`, `293`, `284`, `283`, `40`, and `425` in-place instead of physically moving downstream
entries. v954 then compresses entries `30` / source frame `54` and `33` /
source frame `59` with local-LZ (`4650 -> 679` and `6656 -> 2761` bytes,
modeled sectors `3 -> 1` and `4 -> 2`) and cuts active payload
`807263 -> 799397` while staying exact-flat to current control at
`3129/2853/2815`, overrun `38`, blocking/refill `42/36`, read time `223`, and
due misses `1`. Treat this as a safe payload-work lane, not a speed win; green
conversion now needs generated deadline/static-upload ownership rather than
more ordinary no-shift tail trimming.

Latest rejected BUILDING4 low bulk no-shift trim: v899 applied every remaining
mechanically trim-capable `BUIL4LOW.FG2` entry after v895 as one preserve-offset
pack probe. It preserved the `1714154` byte footprint, LBA/sectors `9118/837`,
and `217088` byte PS-EXE bucket while shrinking active payload
`809364 -> 794233` across `18` entries, but valid timing regressed:
scene/loop/target `3128/2853/2816 -> 3133/2858/2817`, overrun `37 -> 41`,
blocking/refill `40/34 -> 46/31`, loop-read VBlanks `215 -> 219`, and due
misses `1 -> 2`. Close bulk catch-up trimming for B4 low; the known-bad
sector-collapse entries still steal phase even when batched with smaller tails.
Green conversion should now favor generated static/upload ownership, or isolated
fixed-sector checks only when they prove exact-flat.

Earlier rejected BUILDING4 high mirror pass: v715 applied the same
preserve-offset offscreen draw-span clip to `BUILDING4.FG2` that v652 promoted
for low tide. The transform is structurally safe and trims the same host-side
work volume (`30` frames, `65111` pixels, `11871` spans, `471` draw rows, and
`61047` logical draw bytes), but the fresh current-code baseline measured
`2847/2815`, worse than the published stale matrix row `2844/2816`. The matched
gate only improved target-relative overrun `32 -> 31` with loop/blocking/refill
flat, so the pack was restored and rebuilt. The later setup-segment promotion
crosses 99%, so future BUILDING4 high work should be measured as green-margin
polish, not as the primary under-99 lane.

Latest promoted WALKSTUF1 work baselines: v653/v654 narrow the rejected
offscreen clipping lane to late-tail frames only, preserving every entry
offset/size, the `1535263` byte pack footprints, fixed LBAs (`25633` low,
`24883` high), and the `217088` byte PS-EXE bucket. They are not VBlank speed
wins. High v654 stays exact-flat at scene `1764`, active loop/target
`1476/1434`, overrun `42`, blocking/refill `81/23`, reads/due `65/16`, while
draw pixels drop `824527 -> 776856`, spans `139288 -> 134136`, and draw rows
`17611 -> 17296`. v657 then physically compacts the already-clipped high
late-tail payloads while preserving the file byte count, LBA/sectors, and
PS-EXE bucket; timing stays exact-flat, active payload drops
`918345 -> 882007`, CD sectors `605 -> 586`, loop reads/read VBlanks
`65/282 -> 63/275`, and due misses stay `16`. Low v653 stays exact-flat at scene `1770`,
`1478/1431`, overrun `47`, blocking/refill `64/20`, loop reads/read VBlanks
`62/281`, and due `11`, while draw pixels drop `824527 -> 785455`, spans
`139288 -> 135025`, draw rows `17611 -> 17298`, dirty rows `27900 -> 27821`,
upload rects `667 -> 666`, and upload bytes `17856000 -> 17805440`.
v665 then clips only the isolated low mid offscreen cluster `133..139`; timing
and CD stay exact-flat, and runtime frame rows/spans/pixels drop
`17298/135025/785455 -> 17292/134774/780557`. v666 extends the isolated safe
subset to the left offscreen cluster `43..57`, still exact-flat, and drops
runtime rows/spans/pixels again to `16838/130637/737371`.
The adjacent v667 `58..74` attempt is rejected: it removed another `48096`
host pixels and `8216` spans, but regressed scene/loop `1770/1478 ->
1782/1490`, blocking `64 -> 97`, refill `20 -> 28`, loop reads `62 -> 63`,
and due misses `11 -> 15`. Do not treat the safe W1 low offscreen region as
contiguous.
v668 adds the separate pre-left cluster `30..41`, stays exact-flat, and drops
runtime rows/spans/pixels to `16680/127950/712324`. v669 clips the separate
pre-tail cluster `194..201`, stays exact-flat, and drops runtime rows/spans/pixels
to `16678/127061/703725`. The broad v670 mid-right `85..101` clip is rejected:
it removed `2966` pixels and `980` spans but regressed blocking `64 -> 65`; split
that region before retrying. The v671 lower split `85..92` reproduced the same
blocking regression after removing `1702` pixels and `595` spans, so the next
binary probe is `93..101`. v672 promotes that upper split exact-flat, removing
`1264` pixels and `385` spans and dropping runtime rows/spans/pixels to
`16678/126676/702461`. v673 clips frames `27..28`, stays exact-flat, removes
`1668` pixels and `122` spans, and drops runtime rows/spans/pixels to
`16678/126554/700793`. v674 clips the separate `75..77` post-left slice, stays
exact-flat, removes `1879` pixels and `628` spans, and drops runtime rows/spans/pixels
to `16678/125926/698914`. v675 clips `66..74`, proving the old `58..74` failure
was not contiguous, and stays exact-flat while removing `15524` pixels, `3364`
spans, `11` rows, and dropping runtime rows/spans/pixels to `16667/122562/683390`.
v678 clips frame `65` alone after the v676/v677 adjacent splits failed, stays
exact-flat while removing `3439` pixels, `527` spans, `4` rows, and dropping
runtime rows/spans/pixels to `16663/122035/679951`.
v680 clips post-left singleton frames `79`, `81`, and `83`, stays exact-flat
while removing `1538` pixels and `540` spans, and drops runtime rows/spans/pixels
to `16663/121495/678413`.
v684 clips mid-right frames `87..88` after the lower splits failed, stays
exact-flat while removing `426` pixels and `154` spans, and drops runtime
rows/spans/pixels to `16663/121341/677987`.
v685 clips mid-right frames `89..92`, stays exact-flat while removing `569`
pixels and `207` spans, and drops runtime rows/spans/pixels to
`16663/121134/677418`.
v686 clips frame `85`, stays exact-flat while removing `388` pixels and `129`
spans, and drops runtime rows/spans/pixels to `16663/121005/677030`.
v687 clips frame `1`, stays exact-flat while removing `269` pixels and `118`
spans, and drops runtime rows/spans/pixels to `16663/120887/676761`.
v688 clips frames `106..112`, stays exact-flat while removing `218` pixels
and `104` spans, and drops runtime rows/spans/pixels to `16663/120783/676543`.
v689 clips frame `3`, stays exact-flat while removing `93` pixels and `45`
spans, and drops runtime rows/spans/pixels to `16663/120738/676450`.
v690 clips frame `140`, stays exact-flat while removing `50` pixels and `28`
spans, and drops runtime rows/spans/pixels to `16663/120710/676400`.
v691 clips frame `61`, stays exact-flat while removing `4495` pixels, `684`
spans, and `79` rows, and drops runtime rows/spans/pixels to
`16584/120026/671905`. This proves at least one frame in the previously risky
`58..63` left2 remainder can promote as a singleton; continue binary there
instead of retrying broad `58..64` direct clipping.
v692 clips frame `60`, stays exact-flat while removing `4431` pixels, `662`
spans, and `80` rows, and drops runtime rows/spans/pixels to
`16504/119364/667474`. The safe singleton island now includes `60..61`;
continue with `62`, `59`, `58`, or `63` individually before retesting pairs.
v693 clips frame `62`, stays exact-flat while removing `4510` pixels, `645`
spans, and `58` rows, and drops runtime rows/spans/pixels to
`16446/118719/662964`. The safe singleton island now includes `60..62`;
continue with `59`, `58`, or `63` individually before retesting adjacent pairs.
v694 clips frame `59`, stays exact-flat while removing `4191` pixels, `633`
spans, and `72` rows, and drops runtime rows/spans/pixels to
`16374/118086/658773`. The safe singleton island now includes `59..62`;
continue with `58` or `63` individually before retesting adjacent pairs.
v695 clips frame `58`, stays exact-flat while removing `4167` pixels, `596`
spans, and `72` rows, and drops runtime rows/spans/pixels to
`16302/117490/654606`. The safe singleton island now includes `58..62`;
continue with `63` individually before retesting adjacent pairs.
v696 clips frame `63`, stays exact-flat while removing `3983` pixels, `578`
spans, and `30` rows, and drops runtime rows/spans/pixels to
`16272/116912/650623`. The safe singleton island now includes `58..63`;
frame `64` remains the direct-clip boundary trigger.
The v697 padded full-pack draw-tail compaction retry is rejected even after the
v696 singleton island: it drops active payload `916139 -> 773574` and loop reads
`62 -> 56`, but regresses scene/loop `1770/1478 -> 1776/1484`, target
`1431 -> 1428`, overrun `47 -> 56`, blocking `64 -> 73`, and refill
`20 -> 29`. Do not retry whole-pack W1-low physical compaction as a single
pass; any compaction retry needs a smaller phase-preserving subset or scheduler
ownership for the new read cadence.
v705 proves the smaller subset form for the already-clipped late tail:
compacting only entries `194..210` preserves file size/LBA/sectors, the
PS-EXE bucket, scene/loop/target `1770/1478/1431`, overrun `47`,
blocking/refill `64/20`, and due misses `11`, while active payload drops
`916139 -> 879801` and loop reads/read VBlanks improve to `60/274` on the
clean current baseline.
v716 then clips frame `133` as a singleton, stays exact-flat, removes `2937`
pixels, `188` spans, and `4` rows, and drops runtime rows/spans/pixels to
`16268/116724/647686`. v717 clips frame `132` the same way, removes another
`2131` pixels, `142` spans, and `1` row, and drops runtime rows/spans/pixels
to `16267/116582/645555`. v718 clips early frame `5`, removes another `1864`
pixels and `352` spans, and drops runtime rows/spans/pixels to
`16267/116230/643691`. v719 clips frame `141`, removes another `1695` pixels
and `358` spans, and drops runtime rows/spans/pixels to `16267/115872/641996`.
v720 clips frame `131`, removes another `1673` pixels, `103` spans, and `1`
row, and drops runtime rows/spans/pixels to `16266/115769/640323`. v721 clips
frame `19`, removes another `1320` pixels, `70` spans, and `9` rows, and drops
runtime rows/spans/pixels to `16257/115699/639003`. v722 clips frame `6`,
removes another `1317` pixels and `307` spans, and drops runtime
rows/spans/pixels to `16257/115392/637686`. v723 clips frame `142`, removes
another `1097` pixels and `294` spans, and drops runtime rows/spans/pixels to
`16257/115098/636589`. v724 clips frame `130`, removes another `1015` pixels
and `57` spans, and drops runtime rows/spans/pixels to
`16257/115041/635574`. v725 clips frame `145`, removes another `916` pixels
and `187` spans, and drops runtime rows/spans/pixels to `16257/114854/634658`.
v726 clips frame `129`, removes another `782` pixels and `56` spans, and drops
runtime rows/spans/pixels to `16257/114798/633876`. Keep these as same-speed CD/work
baselines, not public
VBlank speed wins. The remaining W1-low compaction risk is specifically
early/mid payload displacement, not late-tail physical compaction; direct
offscreen work should continue only as singleton/subset gates that preserve
reads, blocking, refill, and due misses.
The v706 early-left subset compaction confirms that risk: compacting only
entries `43..57` trims active payload `879801 -> 846946`, but regresses
scene/loop `1770/1478 -> 1778/1486`, target `1431 -> 1430`, overrun
`47 -> 56`, blocking `64 -> 72`, refill `20 -> 25`, and loop reads
`60 -> 61`. Close early-left physical displacement under the current scheduler;
future byte removal there needs generated deadline ownership or a no-shift
encoding.
The v707 later-left split confirms the same branch is not just an early-left
problem: compacting entries `66..77` trims active payload `879801 -> 861691`,
but regresses scene/loop `1770/1478 -> 1784/1492`, target `1431 -> 1425`,
overrun `47 -> 67`, blocking `64 -> 92`, refill `20 -> 29`, and due misses
`11 -> 14`. Close broad left-side physical compaction without generated
ownership.
The v708 small mid-pack split also fails: entries `133..140` trim only
`3239` bytes and improve loop `1478 -> 1477`, but target drops
`1431 -> 1428`, overrun regresses `47 -> 49`, blocking jumps `64 -> 79`,
and due misses `11 -> 14`. Non-tail same-order physical compaction should now
be considered closed unless a generated planner owns the new deadlines.
The v676 `62..65` split is rejected even though it is smaller than the old
`58..74` miss: it removed `15288` pixels, `2277` spans, and `103` rows, but
regressed scene/loop `1770/1478 -> 1782/1490`, blocking `64 -> 97`, refill
`20 -> 28`, loop reads `62 -> 63`, and due misses `11 -> 15`. Treat
`62..65` as a direct-clip phase cliff; any remaining `58..65` retry should be
one/two-frame or use a non-shrinking data-shape/hole-fill approach.
The v677 `64..65` bisection also failed, despite only removing `6795` pixels,
`1054` spans, and `15` rows, with the same `1782/1490`, blocking `97`, refill
`28`, reads `63`, and due `15` profile. Frame `65` is now proven safe, making
frame `64` the current direct adjacent boundary trigger.
The v679 frame `64` single-frame confirmation also failed with the same
`1782/1490`, blocking `97`, refill `28`, reads `63`, and due `15` profile
after removing only `3356` pixels, `527` spans, and `11` rows. Treat `64` as
closed for direct clipping; remaining `58..63` needs non-shrinking/hole-fill or
scheduler-safe treatment before another attempt.
The v681 `85..88` mid-right split also reproduces the smaller blocking cliff
from the old `85..92` bundle: it removes `1133` pixels and `388` spans, keeps
scene/loop flat, improves overrun `47 -> 46`, but regresses blocking `64 -> 65`.
Split `85..88` again before closing those individual frames.
The v682 `85..86` pair repeats the same profile after removing `707` pixels
and `234` spans; isolate `86` or `85` before closing the lower mid-right pair.
The v683 frame `86` single-frame probe still regresses blocking `64 -> 65`
after only `319` pixels and `105` spans, so close frame `86` for direct
clipping and move to `87..88` or smaller non-risk candidates.

Latest promoted VISITOR5 low speed baseline: reuse the accepted high-tide
`30..46` retained-read group shape for low tide instead of the older `23..47`
low-side group. The binary sweep also checked `30..54` and `9..33`; both were
exact-flat at `1104/1092`, blocking/refill `11`, and loop reads `19`. The
promoted v526 gate improves scene `1363 -> 1361`, active loop/target
`1104/1092 -> 1102/1097`, overrun `12 -> 5`, blocking/refill `11 -> 5`, loop
reads `19 -> 18`, and due misses stay `0`; pack LBA/sectors stay
`24394/173`, and the PS-EXE bucket stays `217088`. VISITOR5 high, VISITOR3
high/low, BUILDING2 high/low, and WALKSTUF1 high/low canaries stayed
exact-flat. This moves VISITOR5 low into green; after the later JOHNNY1 v932
promotion, the under-99 focus set is WALKSTUF1 low/high, BUILDING2 high/low,
VISITOR3 high/low, and BUILDING4 low.

Latest promoted VISITOR3 allocator-era speed baseline: the old force-relief
path dropped both the large setup-prime/window buffers and the tiny stage1
prefetch frame buffer. Full setup-prime/window restore was rejected because
VISITOR3 BSODed during clean-rect allocation, and the first stage1-only
promotion proved the small stage buffer could survive clean relief. The current
promotion keeps clean-memory relief enabled, preserves the small stage1 buffer
for both tides, allows a high-tide `68 KiB` stream window, allows low tide to
keep a smaller `16 KiB` stream window only when at least 5 VBlanks of slack are
available, trims high-tide terminal reads before already-resident
setup-segment data, extends the high-tide second setup segment through
relative sector `229`, relocates high frame `139`'s raw payload into that
already retained gap, adds a third high-tide retained setup segment for
relative sectors `228..262`, adds a third low-tide retained setup segment for
relative sectors `206..230`, then extends that low segment to `206..232`
so frame `138` can be moved raw into the paid gap, moves high frames `56`
and `57` raw into the retained `228..262` gap with an initial `64 KiB`
tight-refill cap, tightens that cap to `56 KiB`, and finally caps high-only
clean strips at `64 KiB`. The accepted high setup-segment canary
`scratch/ps1-perf-iterate/visitor3-high-seg203-229-payloadtrim-canaries-pass/20260518-011814-2337084/summary.json`
improves high from `1232/1033` to `1096/1040`, overrun `199 -> 56`,
blocking `478 -> 71`, reads `137 -> 8`, due `137 -> 5`, and target speed
`83.847% -> 94.891%`. The accepted high raw-gap canary
`scratch/ps1-perf-iterate/visitor3-high-frame139-rawgap-canaries-pass/20260518-050438-3630173/summary.json`
improves high further to `1096/1041`, overrun `55`, blocking `67`, reads `7`,
due `4`, and target speed `94.982%`. The accepted high third-segment canary
`scratch/ps1-perf-iterate/visitor3-high-thirdseg228-262-canaries-pass/20260518-053640-3809465/summary.json`
improves high again to `1082/1042`, overrun `40`, blocking `50`, reads `6`,
due `3`, and target speed `96.303%`. The accepted high frame56/57 raw-gap
tight-refill gate
`scratch/ps1-perf-iterate/visitor3-high-frame56-57-rawgap-tight64-currentbaseline/20260518-174143-3725529/summary.json`
improves high to `1079/1043`, overrun `36`, blocking/read time `49/57`,
reads `4`, due `3`, and target speed `96.664%`. The accepted tight56 cap canary
`scratch/ps1-perf-iterate/visitor3-high-tight56-canaries/20260518-213723-865271/summary.json`
improves high again to `1075/1043`, overrun `32`, blocking/read time `46/54`,
reads `4`, due `3`, hidden refill `3`, and target speed `97.023%`. The accepted low third-segment canary
`scratch/ps1-perf-iterate/visitor3-low-thirdseg206-230-visitor-canaries-pass/20260518-043607-3468206/summary.json`
improves low from `1231/1040` to `1074/1039`, overrun `191 -> 35`,
blocking `438 -> 85`, reads `126 -> 19`, due `126 -> 15`, and target speed
`84.484% -> 96.741%`. The accepted frame138 raw-gap follow-up
`scratch/ps1-perf-iterate/visitor3-low-frame138-rawgap-seg206-232-vnext/20260518-124907-2068964/summary.json`
improves low again to `1065/1039`, overrun `26`, blocking/read time `75/97`,
reads `18`, due `14`, and target speed `97.559%`. Use this as the allocator-era VISITOR3
runtime baseline; future VISITOR3 work still needs data-shape or scheduler
ownership to cross 99%.

Latest promoted VISITOR3 high clean-strip retune: capping high-tide clean
strips at `64 KiB` after the raw-gap tight56 baseline improves the row without
moving pack layout. Focused proof:
`scratch/ps1-perf-iterate/visitor3-high-clean64-vnext/20260519-010801-2078106/summary.json`;
seven-case canary:
`scratch/ps1-perf-iterate/visitor3-high-clean64-canaries/20260519-010953-2088970/summary.json`.
The accepted cap improves VISITOR3 high `1075/1043 -> 1075/1044`, overrun
`32 -> 31`, blocking/read time `46/54 -> 45/53`, and target speed
`97.023% -> 97.116%`, while hidden refill stays `3` and reads/due stay `4/3`.
BUILDING2 high/low, VISITOR3 low, WALKSTUF1 high/low, and BUILDING4 low stayed
stable. Neighboring caps are closed: `80 KiB` did not improve and regressed
overrun to `33`; `48 KiB` missed `JCPERF2`.

Latest promoted WALKSTUF1 high read group: `{149,165}` follows the accepted
`{78,91}` row and converts the top regenerated W1-high scheduler-owned
candidate into a same-overrun CD-pressure win. Focused artifact:
`scratch/ps1-perf-iterate/walkstuf1-high-rg149-165-vnext/20260518-202745-469634/summary.json`;
under-green canary artifact:
`scratch/ps1-perf-iterate/walkstuf1-high-rg149-165-canaries/20260518-202932-479774/summary.json`.
The canary keeps W1 high at `1475/1441`, overrun `34`, refill `13`, and due
`10`, while improving blocking/read time `59/212 -> 57/199` and loop reads
`46 -> 44`; W1 low, VISITOR3 high/low, BUILDING2 high/low, and BUILDING4 low
stay exact-flat. The later VISITOR3 high tight56 cap and BUILDING2 low
`226..262` + clean80 promotion move the public rollup to
`+0.2487%` over target / `99.7549%` target speed after the later VISITOR3 high
clean64 retune.

Prior WALKSTUF1 high setup-segment retarget: high keeps the `198..244`
setup slice and extends the second retained slice to the allocator-safe
`286..344` edge. Focused artifact:
`scratch/ps1-perf-iterate/walkstuf1-high-setupseg286-344-current/20260518-111109-1509132/summary.json`;
under-green canary artifact:
`scratch/ps1-perf-iterate/walkstuf1-high-setupseg286-344-canaries/20260518-111308-1520614/summary.json`.
The focused and under-green canaries promote the shape as a same-overrun
pressure win: WALKSTUF1 high moves to `1475/1441`, overrun stays `34`, while
blocking/read time improves `60/228 -> 59/212`, prefetch overrun `15 -> 13`,
loop reads `48 -> 46`, and due misses stay `10`; W1 low, VISITOR3 high/low,
BUILDING2 high/low, and BUILDING4 low stay stable. After the later VISITOR3
low frame138 raw-gap promotion, W1-low clean-rect/setup-edge promotion, and
W1-low `{91,107}` first-boundary read group, VISITOR3 high frame56/57
raw-gap plus tight-refill promotion, BUILDING2 low `226..238` setup-segment
promotion, the later VISITOR3 high tight56 cap, and the BUILDING2 low
`226..262` + clean80 promotion, plus the later VISITOR3 high clean64 retune,
public rollup is `+0.2487%` over target / `99.7549%` target speed;
raw signed is about `-0.4683%` / `100.4852%`, and bands are `120` green, `6` yellow, `0` orange,
`0` red. The wider `286..346` edge regressed overrun to `35`, while the older
`286..350` variant crossed the allocator clean-rect cliff; keep `286..344` as
the current second-slice boundary.

Latest rejected WALKSTUF1 high read-group sweep: after the W1-low `{91,107}`
promotion, raw W1-high scalar rows still trade modeled read wins for visible
phase debt. `{84,108,0}` improved blocking `59 -> 56`, refill `13 -> 12`,
reads `46 -> 42`, and due `10 -> 8`, but regressed loop/overrun
`1475/34 -> 1477/36`; adding slack `4` made the row exact-flat with no key
metric win. `{80,92,0}` regressed target/overrun/blocking/refill to
`1438/37/66/18`, and `{164,188,0}` reduced reads to `44` but regressed
target/overrun/blocking to `1440/35/66`. The follow-up `{156,180,0}` retest
also reduced reads `46 -> 44` and due `10 -> 9`, but regressed
target/overrun/blocking/refill to `1438/37/62/22`. Artifacts:
`scratch/ps1-perf-iterate/walkstuf1-high-rg84-108-vnext/20260518-160749-3189948/summary.json`,
`scratch/ps1-perf-iterate/walkstuf1-high-rg84-108-slack4-vnext/20260518-160943-3200969/summary.json`,
`scratch/ps1-perf-iterate/walkstuf1-high-rg80-92-vnext/20260518-161134-3211699/summary.json`,
and
`scratch/ps1-perf-iterate/walkstuf1-high-rg164-188-vnext/20260518-161329-3222854/summary.json`,
plus `scratch/ps1-perf-iterate/walkstuf1-high-rg156-180-vnext/20260518-161741-3246714/summary.json`.
Do not retry W1-high scalar grouped appends in this lane without generated
deadline/refill ownership or a render/upload work reduction that changes the
phase budget.

Latest rejected BUILDING2 high top early read-group probes: after the W1-high
scalar closures, `{122,146,0}` was the largest simple B2-high candidate left in
the refreshed matrix, and `{95,119,0}` tested whether chaining directly after
the accepted `{83,95}` row would create runtime group hits. `{255,271,0}` then
tested the late cluster adjacent to the accepted `{271,287,5}` row. All three
focused gates stayed exact-flat at `1621/1347/1313`, overrun `34`,
blocking/refill `39/16`, loop reads/read time `45/199`, and due `5`, with
`group_hits=0`. Artifacts:
`scratch/ps1-perf-iterate/building2-high-rg122-146-vnext/20260518-162313-3278027/summary.json`
and
`scratch/ps1-perf-iterate/building2-high-rg95-119-vnext/20260518-162616-3295536/summary.json`,
plus
`scratch/ps1-perf-iterate/building2-high-rg255-271-vnext/20260518-162927-3313663/summary.json`.
Close `122..146`, `95..119`, and `255..271` as inert scalar appends on this
baseline. B2-high clusters now need generated append-start/deadline ownership or
pack/upload work, not another blind hand table row.

Latest rejected BUILDING2 high setup-window swing: retargeting the accepted
second setup segment away from `202..242` to `122..162`, `95..135`, or
`140..180` all regressed timing, proving the late cluster is essential.
Preserving `202..242` and adding `122..146`/`122..145`/`122..144` hit the
CACHE clean-rect cliff before `JCPERF2`; clean-cap pairings at `80 KiB` and
`88 KiB` ran but regressed loop/blocking/overrun. Smaller additive windows
`122..138` and `95..111` fit but were phase-negative, reducing reads while
raising blocking to `48` and overrun to `44`/`43`. Artifacts are recorded under
`building2-high-seg2-*` and `building2-high-add-seg3-*` in
`scratch/ps1-perf-iterate/`. Keep B2-high setup coverage at `3..35` plus
`202..242`; do not retry simple retained setup residency for the exposed early
clusters without generated deadline/refill ownership or a clean/allocator byte
reduction.

Latest rejected BUILDING2 high runtime-owner swing: the prepare-before-window
branch regressed B2-high to `1351/1311`, overrun `40`, blocking `45`, and
refill `19`; hand-coded direct-stage ownership for `249..261` and `185..197`
was exact-flat/inert while growing `foregroundPilotPlay`; and runtime staged
x-band upload grew the PS-EXE bucket `233472 -> 235520` without producing a
valid `JCPERF2` gate. These are now logged under
`building2-high-prepare-before-window-current`,
`building2-high-directstage-current`, and
`building2-high-xband-upload-staged-current`. A later generic scratch x-band
retry with `4`, `8`, and `16` pixel alignments plus a transfer-alignment safety
gate also failed before `JCPERF2`; the best variant reached BUILDING2 frame
`132/334`, still emitted transfer-rounding warnings, and grew the PS-EXE bucket
`233472 -> 235520`. A refreshed medium-gap `{249,257}` grouped append then
regressed `1347/1313 -> 1349/1313`, blocking `39 -> 40`, with `group_hits=0`;
the one-read movement was phase drift, not a controlled append win. Close
hand-coded B2-high runtime ownership, local read-table additions, and
hot-renderer scratch-copy upload staging; reopen through generated metadata or
pack-emitted/precomposed data only.

Latest rejected BUILDING4 low read-group route: enabling a low-tide `building4`
read-group table and adding `{274,298,0}` did not move timing. The focused gate
stayed exact-flat at `3121/2849/2816`, overrun `33`, blocking/refill `38/31`,
loop reads/read time `30/222`, and due `1`, with `group_hits=0`; it grew
`foregroundPilotPlay` by `20` bytes. Artifact:
`scratch/ps1-perf-iterate/building4-low-rg274-298-vnext/20260518-163348-3338542/summary.json`.
Close B4-low hand-authored group routing for `274..298`; remaining B4-low work
should target static upload/restore elimination or generated append ownership.

Latest rejected clean first-frame upload dirty-mark probe: VISITOR3 high's
remaining max-upload row looked like a possible first-frame full-refresh cost,
so the binary check removed the clean-rect full-dirty mark first, then removed
the clean-tile full-dirty mark too. Both focused gates stayed exact-flat at
`1395/1082/1042`, overrun `40`, blocking/refill `50/5`, loop reads `6`, due
`3`, and max upload `614400`; the second probe only shifted code addresses.
Artifacts:
`scratch/ps1-perf-iterate/cleanrect-no-full-first-upload-visitor3-high-vnext/20260518-163900-3367927/summary.json`
and
`scratch/ps1-perf-iterate/clean-no-full-first-upload-visitor3-high-vnext/20260518-164046-3378263/summary.json`.
Keep the clean baseline first-frame dirty invariant; VISITOR3 high's remaining
work is driven by another upload producer or by CD/window phase, not by these
two clean-save marks.

Latest corrected VISITOR3 clean-relief read-plan and rejected low read-group
sweep: `scripts/ps1-foreground-read-plan.py` now mirrors runtime clean-memory
relief by reporting `setup_prime_bytes=0` for VISITOR3 while preserving the
targeted setup segments. The regenerated read matrix now shows real early
VISITOR3 high/low active-read pressure instead of treating the disabled
setup-prime window as resident coverage. The first three VISITOR3-low grouped
append probes are still rejected: `{9,33,0}` regressed `1369/1065/75` to
`1371/1067/76`, `{9,25,0}` regressed to `1374/1070/1039` with overrun `31`
and blocking `79`, and `{92,108,0}` regressed to `1371/1067/1041` with
blocking `76`. Artifacts:
`scratch/ps1-perf-iterate/visitor3-low-rg9-33-vnext/20260518-164901-3424623/summary.json`,
`scratch/ps1-perf-iterate/visitor3-low-rg9-25-vnext/20260518-165046-3434768/summary.json`,
and
`scratch/ps1-perf-iterate/visitor3-low-rg92-108-vnext/20260518-165221-3444117/summary.json`.
Keep the analysis fix, but close VISITOR3-low blind grouped appends in this
lane; next low work needs data-shape/setup relocation that changes where the
reads land, not merely how adjacent reads are grouped.

Latest VISITOR3 high frame56/57 raw-gap plus tight-refill promotion: the
earlier raw-gap probe had the right visible shape but carried one extra hidden
refill VBlank. Keeping frames `56` and `57` raw in the retained high-tide
`228..262` gap and capping tight-slack speculative window refills first at `64 KiB`,
then at `56 KiB`, turns that near miss into the current strict-safe baseline. Focused artifact:
`scratch/ps1-perf-iterate/visitor3-high-frame56-57-rawgap-tight64-vnext/20260518-172551-3635538/summary.json`;
current-baseline no-regress artifact:
`scratch/ps1-perf-iterate/visitor3-high-frame56-57-rawgap-tight64-currentbaseline/20260518-174143-3725529/summary.json`;
current tight56 canary artifact:
`scratch/ps1-perf-iterate/visitor3-high-tight56-canaries/20260518-213723-865271/summary.json`;
stale-baseline eight-case guard artifact:
`scratch/ps1-perf-iterate/visitor3-high-frame56-57-rawgap-tight64-canaries/20260518-172833-3650847/summary.json`;
transform summary:
`scratch/visitor3-high-frame57-rawgap-current/summary-frame56-57.json`.
The promoted raw-gap row improves VISITOR3 high `1082/1042 -> 1079/1043`,
overrun `40 -> 36`, blocking/read time `50/62 -> 49/57`, loop reads `6 -> 4`,
and due stays `3`; hidden refill stays `5`. The later accepted cap sweep keeps
the frame56/57 raw-gap layout and lowers the cap to `56 KiB`, improving
`1079/1043 -> 1075/1043`, overrun `36 -> 32`, blocking/read time `49/57 -> 46/54`,
and hidden refill `5 -> 3`. BUILDING2 high/low, BUILDING4 low, WALKSTUF1
high/low, and VISITOR3 low stayed exact-flat against the current baseline.
Slack-only retunes `6`, `7`, and `8` plus `32`, `48`, and `52 KiB` caps were
rejected; keep the accepted `56 KiB` cap and the frame56/57 raw-gap layout.

Latest WALKSTUF1 low first-boundary read-group promotion: the earlier
`{91,107}` scalar row failed on the older `238..342` retained setup baseline,
but passes after the low-only 48 KiB clean-rect cap and `238..344` setup edge.
Focused artifact:
`scratch/ps1-perf-iterate/walkstuf1-low-rg91-107-post-clean48-vnext/20260518-150229-2825275/summary.json`;
under-green canary artifact:
`scratch/ps1-perf-iterate/walkstuf1-low-rg91-107-post-clean48-canaries-noregress/20260518-151229-2883104/summary.json`;
W1-high canary:
`scratch/ps1-perf-iterate/walkstuf1-low-rg91-107-w1high-canary/20260518-152051-2931418/summary.json`;
VISITOR3-low canary:
`scratch/ps1-perf-iterate/walkstuf1-low-rg91-107-visitor3low-canary/20260518-152240-2942030/summary.json`.
The promoted row improves W1-low `1475/1443 -> 1473/1444`, overrun
`32 -> 29`, blocking/read time `48/200 -> 43/195`, refill `12 -> 11`,
loop reads `39 -> 36`, and due `6 -> 5`; W1 high, VISITOR3 high/low,
BUILDING2 high/low, and BUILDING4 low stay stable. Keep `{91,107,0}` after
the existing `{78,91,0}` first-boundary row.

Latest WALKSTUF1 low clean-rect/setup-edge retarget: low-only 48 KiB
clean-rect chunking lets the allocator keep the low retained setup segment
at relative sectors `238..344` without destabilizing W1-high or other canaries.
Focused artifact:
`scratch/ps1-perf-iterate/w1low-clean48-runtimecap-setup238-344-focused/20260518-134826-2404755/summary.json`;
under-green canary artifact:
`scratch/ps1-perf-iterate/w1low-clean48-runtimecap-setup238-344-canaries/20260518-135202-2425616/summary.json`;
W1-high canary:
`scratch/ps1-perf-iterate/w1low-clean48-runtimecap-setup238-344-w1high-canary/20260518-135014-2415120/summary.json`;
VISITOR3-low canary:
`scratch/ps1-perf-iterate/w1low-clean48-runtimecap-setup238-344-visitor3low-canary/20260518-140437-2497177/summary.json`.
The promoted shape improves W1-low `1480/1442 -> 1475/1443`, overrun
`38 -> 32`, blocking/read time `55/211 -> 48/200`, refill `15 -> 12`,
and keeps due misses at `6`; W1-high, VISITOR3 high/low, BUILDING2 high/low,
and BUILDING4 low stay stable.

Prior WALKSTUF1 low setup-segment retarget: the allocator-era split low
setup slices (`197..243` plus `410..434`) were replaced with one larger
retained setup segment at relative sectors `238..342`, keeping the second
slot disabled to stay under the CACHE clean-rect cliff. Focused artifact:
`scratch/ps1-perf-iterate/walkstuf1-low-setupseg238-342-replace-vnext/20260518-065116-38864/summary.json`;
canary artifact:
`scratch/ps1-perf-iterate/walkstuf1-low-setupseg238-342-replace-canaries/20260518-065731-74208/summary.json`.
The earlier `238..344` edge probe failed before the clean-rect cap was made
low-specific: allocating the 106-sector W1-low setup buffer (`217088` bytes)
exhausted CACHE before the `97280` byte clean-rect allocation, so it produced
no `JCPERF2` metrics. Artifact:
`scratch/ps1-perf-iterate/walkstuf1-low-setupseg238-344-current/20260518-113619-1651972/summary.json`.
That failure is now superseded by the promoted low-only 48 KiB clean-rect
chunking path above; keep `238..344` as the current CACHE-backed W1-low
boundary.
A follow-up `648 KiB CACHE / 760 KiB TRANSIENT` rebalance is also closed:
`238..344` became measurable and improved visible overrun/blocking, but
regressed hidden refill `15 -> 20`; `238..343` regressed visible timing and
refill. Artifacts:
`scratch/ps1-perf-iterate/cache648-w1low-setupseg238-344-focused/20260518-114116-1679988/summary.json`
and
`scratch/ps1-perf-iterate/cache648-w1low-setupseg238-343-focused/20260518-114558-1706973/summary.json`.
The focused and under-green canaries promote the shape: WALKSTUF1 low moves
`1479/1435 -> 1480/1442`, overrun `44 -> 38`, blocking/read time
`65/230 -> 55/211`, refill overrun `18 -> 15`, loop reads `50 -> 38`, and
due misses `10 -> 6`; W1 high, VISITOR3 high/low, BUILDING2 high/low, and
BUILDING4 high/low stay stable. Public rollup becomes `+0.2846%` over target /
`99.7207%` target speed; raw signed becomes `-0.4323%` / `100.4510%`, and
bands stay `119` green, `7` yellow, `0` orange, `0` red.

Prior VISITOR3 high third-segment pass: the remaining uncovered high-tide
active read was the `228..262` sector range covering frames `135`, `136`, and
`138`. Adding that range as a third high retained setup segment removes all
uncovered active reads in the planner while preserving clean-relief operation.
Focused artifact:
`scratch/ps1-perf-iterate/visitor3-high-thirdseg228-262-vnext/20260518-053501-3800130/summary.json`;
canary artifact:
`scratch/ps1-perf-iterate/visitor3-high-thirdseg228-262-canaries-pass/20260518-053640-3809465/summary.json`.
The row improves `1096/1041 -> 1082/1042`, overrun `55 -> 40`,
blocking/read time `67/79 -> 50/58`, loop reads/due `7/4 -> 6/3`, and keeps
hidden refill at `5`. Public rollup becomes `+0.2880%` over target /
`99.7174%` target speed; raw signed becomes `-0.4289%` / `100.4478%`, and
bands move to `119` green, `7` yellow, `0` orange, `0` red.

Latest VISITOR3 high frame139 raw-gap pass: moving the raw `15679` byte frame
`139` payload to offset `417308` inside the already retained `203..229` setup
segment gap removes one active read without enabling the rejected D4 decode
path. Focused artifact:
`scratch/ps1-perf-iterate/visitor3-high-frame139-rawgap-vnext/20260518-050255-3620479/summary.json`;
canary artifact:
`scratch/ps1-perf-iterate/visitor3-high-frame139-rawgap-canaries-pass/20260518-050438-3630173/summary.json`.
The row improves `1096/1040 -> 1096/1041`, overrun `56 -> 55`,
blocking/read time `71/83 -> 67/79`, loop reads/due `8/5 -> 7/4`, and keeps
hidden/prefetch at `12/5`. Public rollup becomes `+0.2994%` over target /
`99.7069%` target speed; raw signed becomes `-0.4175%` / `100.4373%`, and
bands stay `119` green, `6` yellow, `1` orange, `0` red.

Latest VISITOR3 high setup-segment sweep: after the terminal read trim,
extending setup segment2 from only sector `203` to sectors `203..229` is the
current clean knee. `18` sectors passed but was smaller (`1102/1042`, blocking
`74`); `26` sectors passed best (`1096/1040`, overrun `56`, blocking/read time
`71/83`); `27`, `30`, and `34` sectors failed by raising hidden refill to `6`;
and `28` sectors passed but was weaker (`1097/1041`, blocking `73`). The
promoted row improves the active baseline `1106/1042 -> 1096/1040`, overrun
`64 -> 56`, blocking/read time `76/88 -> 71/83`, with refill/due flat at
`5/5`.

Latest VISITOR3 high terminal-window trim: the active window refill was reading
through the last non-resident high-tide payloads and into relative sectors that
were already resident from setup. The accepted runtime trim stops that terminal
read at the last needed payload boundary before the resident setup segment,
improving the then-current row `1113/1042 -> 1106/1042`, overrun `71 -> 64`,
blocking/read time `83/95 -> 76/88`, and keeping refill/due flat at `5/5`.
That trim enabled the later setup-segment2 `203..229` sweep above; the broader
`203..245` swing reached `1088/1043` but stayed rejected because it raised
hidden refill from `5` to `6`.

Latest VISITOR3 high window-size sweep: `68 KiB` is the current allocator-safe
knee. `72 KiB` passed but regressed to `1121/1042`, `70 KiB` passed but
regressed versus the knee to `1115/1041`, and `67 KiB` / `69 KiB` were
phase-negative (`1123/1041` and `1124/1041`). Keep `68 KiB` until a data-shape
or clean-rect pressure change shifts the knee.

Latest VISITOR3 low clean-relief window sweep: restoring a low-tide
`16 KiB` stream window under clean relief is the new allocator-safe knee when
paired with the VISITOR3 low slack guard at `5`. Slack `4` was faster locally
but introduced `prefetch_overrun_vb=3`; slack `5` kept hidden refill clean and
improved low from the stage1/window64 baseline `1107/1042` to `1088/1038`,
overrun `65 -> 50`, blocking `347 -> 104`, reads/read time `124/426 -> 21/123`,
and due `98 -> 17`. `20 KiB` regressed versus the knee to `1092/1039`, `24 KiB`
passed but was slower at `1090/1039`, and slack `6` was exact-flat to slack `5`.

Latest VISITOR3 low third-segment pass: read groups `{206,230,0}` and
`{248,264,0}` were inert (`group_hits=0`), and replacing the accepted
`150..177` segment with `206..233` regressed to `1097/1035`, blocking `123`.
Adding `206..230` as a third retained setup segment instead passed:
`1088/1038 -> 1074/1039`, overrun `50 -> 35`, blocking/read time
`104/123 -> 85/103`, reads `21 -> 19`, due `17 -> 15`, hidden refill `0`.
VISITOR3 high and the B2/B4/W1 non-visitor canaries stayed exact-flat.

Pre-allocator promoted VISITOR3 high speed baseline: keep the v299 resident
frame-131 placement, the v291 frame-140/tail setup-segment placement, store
frame `137` as a 503-byte D4 previous-frame delta against frame `136`, and add
frame `132` as a 768-byte D4 delta against frame `131`. v462 proved the
frame132 data shape but needed a slack4 guard to avoid hidden refill; v464
preloaded frame132 through a one-sector high setup segment at sector `203`;
v501 then moved the already-small frame137 payload into the same paid sector
before frame132. The v629 tail-pack follow-up reuses the already-proven
VISITOR3 low compact cleanup payloads for high frames `143` and `144`, then
packs frames `141/140/142/143/144` plus sound events into the existing
sector `277..293` setup segment so frame `141` no longer starts at sector
`275`. The promoted v629 gate improves scene `1400 -> 1398`, active
loop/target `1065/1039 -> 1063/1040`, overrun `26 -> 23`, visible blocking
`41 -> 35`, loop-read VBlanks `41 -> 35`, loop reads `7 -> 6`, and due misses
`7 -> 6`; hidden refill and prefetch overrun stay `0`, pack LBA/sectors stay
`22611/760`, the pack footprint stays `1555450` bytes, and the PS-EXE bucket
stays `217088`. This promotion adds about `0.0023` public over-target points
removed and `0.0022` public target-speed points gained, moving VISITOR3 high
to `97.84%` target speed.

Latest rejected VISITOR3 high frame133 D4 attempt: v484 stored frame `133` as
a 14288-byte chained previous-frame D4 delta against the already-decoded frame
`132`, shrinking that payload from `17069` bytes and sector span `211..221`
to `211..219`. Local decode validation proved every candidate frame matched
the accepted baseline byte-for-byte, but runtime ownership was unsafe: the
initial staged candidate emitted incomplete correctness (`trip=1`,
`last_frame=132`, `sound_events=7/8`), the staged-decode repair stalled
without `JCPERF2`, and the due-load-only/no-stage variant was killed before
metrics. Restore source and `VISITOR3.FG2`; do not retry frame133 chained D4
without first designing explicit staged/prepared ownership for previous-frame
delta payloads.
The 2026-05-14 source-only v727 follow-up confirms the broad hook is unsafe:
decoding every staged previous-frame D4 before `fgRuntimeSetStagedFrame()`
reached the scene rect marker but timed out without `JCPERF2` even with the
accepted current pack and no frame133 payload change. Future retries must be
frame-specific and generated-owned, not a global staged-D4 behavior change.

Latest rejected VISITOR3 high terminal read-group attempt: v487 added a
high-only grouped append for sectors `228..252`, targeting the current
terminal cluster for frames `135..137`. The focused gate completed with valid
`JCPERF2` metrics but stayed exact-flat at scene `1402`, active loop/target
`1067/1039`, overrun `28`, blocking `45`, loop reads/due `8/8`, and hidden
refill `0`; `group_hits` stayed `0` while foreground hot code shifted
(`foregroundPilotPlay +52` bytes). Restore the source and close scalar
VISITOR3 high terminal read-group tables; this cluster needs generated
scheduler ownership or pack-side byte relocation, not another hand table.

Latest rejected VISITOR3 high setup-prime frame133 relocation: v488 compressed
high frame `129` to its known 609-byte D4 payload, compacted frames `130` and
`131` earlier inside the setup-prime window, and moved full frame `133` into
the freed setup-prime gap at sectors `150..160`. The shape preserved the
`1555450` byte pack footprint and fixed LBA/sectors, but it could not produce
a valid gate: normal, skip-build, and case-local-CD focused runs were all
killed with exit `137` before `JCPERF2`; the orphaned local-CD emulator
produced no late metrics after an additional wait. Restore source and
`VISITOR3.FG2`; do not promote this residency layout without first proving the
frame129 D4/prime-compaction ownership path in a deterministic local validator
or with a stable non-orphaning harness.

Latest rejected VISITOR3 high frame134 D4 attempt: v489 encoded frame `134`
in place as a previous-frame D4 delta against full frame `133`, shrinking the
payload `17001 -> 14202` bytes without changing pack footprint or LBA. The
focused gate was killed with exit `137` before `JCPERF2`, and the orphaned
emulator produced no late metrics after a one-minute wait. Restore source and
`VISITOR3.FG2`; avoid further high-tide D4 additions with large command counts
until a deterministic D4 ownership validator explains the pre-metric hang.

Latest rejected WALKSTUF1 low physical-reorder swing: v511 made the tight
frame `146..154` cluster setup-prime resident without source code by reordering
valid `WALK1LOW.FG2` payload bytes. It preserved the `1535263` byte pack,
`data_offset=4392`, `sound_offset=920531`, LBA/sectors, and the PS-EXE bucket,
but displacing the early ramp was worse than the resident-cluster win: focused
low regressed scene `1770 -> 1795`, active loop/target `1478/1431 ->
1503/1424`, overrun `47 -> 79`, blocking `64 -> 120`, loop reads `64 -> 71`,
and due misses `11 -> 19`. Close broad setup-prime physical reordering for
WALKSTUF1 low unless the swap does not push the startup ramp out of resident
coverage.

Latest rejected WALKSTUF1 low near-equal resident-slot swap: v512 swapped setup
frames `26..36` with hot frames `148..157`, a range-size delta of only `-3`
bytes. The hot cluster became setup-prime resident in sectors `47..71`, entries
`37..147` shifted by only `-3` bytes, and the pack/LBA/PS-EXE identity stayed
fixed. It still failed harder than v511: scene `1770 -> 1798`, active
loop/target `1478/1431 -> 1506/1424`, overrun `47 -> 82`, blocking `64 ->
131`, hidden refill `20 -> 24`, loop reads `64 -> 70`, and due misses `11 ->
22`. Close early-ramp physical swaps entirely. The next viable WALKSTUF1 low
lanes are generated deadline ownership, custom payload shrinking/aliasing
before reordering, or physical relocation only after creating true in-window
slack that leaves startup entries resident.

Latest rejected WALKSTUF1 low standalone D4: v513 tested the largest unclosed
tight-cluster D4 byte signal, frame `158`, which shrinks `5232 -> 3181` bytes
against previous frame `157`. The focused gate showed the familiar byte trap:
scene `1770 -> 1778`, active loop/target `1478/1431 -> 1486/1429`, overrun
`47 -> 57`, blocking `64 -> 80`, hidden refill `20 -> 21`, loop reads stayed
`64`, and due misses worsened `11 -> 14`. Close standalone WALKSTUF1 low D4
for frame `158`; together with the earlier frame `59`, `92`, `181`, `187`,
and `189` failures, previous-frame deltas need generated deadline ownership or
a code-neutral resident-slot plan before they are worth another runtime gate.

Latest rejected WALKSTUF1 low resident-hole D4 swing: v522 created a valid
setup-prime in-data hole without evicting startup frames by encoding frame `28`
as D4 (`6764 -> 3098` bytes), then moved frame `158`'s D4 payload plus tiny
full frames `137` and `78` into that hole. The pack footprint, LBA, and
`217088` byte PS-EXE bucket stayed fixed, but the focused gate regressed hard:
scene `1770 -> 1792`, active loop/target `1478/1431 -> 1500/1427`, overrun
`47 -> 73`, blocking `64 -> 104`, hidden refill `20 -> 28`, loop reads
`64 -> 69`, and due misses `11 -> 15`. Close frame28-created resident holes
paired with frame158 D4; the startup decode and hot predicate/code-shape debt
outweigh resident ownership. Future WALKSTUF1 low hole work needs either
zero-decode alias/slack, a generated low-CPU representation, or explicit
deadline ownership before moving hot payloads into setup-prime holes.

Latest rejected WALKSTUF1 low same-order D4 compaction: v523 encoded frame
`100` as the previously exact-flat D4 delta (`4683 -> 586` bytes), then
compacted every later payload earlier by `4097` bytes while keeping the sound
table offset, total pack size, LBA, sectors, and PS-EXE bucket fixed. This
avoided reverse seeks, but the phase shift was still negative: focused low
regressed scene `1770 -> 1784`, active loop/target `1478/1431 ->
1492/1426`, overrun `47 -> 66`, blocking `64 -> 99`, hidden refill
`20 -> 29`, and due misses `11 -> 15`; loop reads stayed `64`. Close
same-order D4 tail compaction for WALKSTUF1 low unless a generated planner can
also retarget the retained-read sectors and deadlines.

Latest promoted VISITOR3 low baseline: keep the v338 tail compaction, move
frame `128` into the accepted resident slot, store frame `129` as a 609-byte
custom D4 delta against that resident payload, and store frame `132` as a
768-byte D4 delta against frame `131`. The v452 focused gate improves scene
`1401 -> 1397`, active loop/target `1072/1040 -> 1068/1041`, overrun
`32 -> 27`, blocking `58 -> 51`, loop reads `10 -> 9`, loop-read time
`58 -> 51`, and due misses `10 -> 9`; the v470 follow-up keeps scene,
loop/target, overrun, reads/due, hidden refill, pack LBA/sectors, and the
`217088` byte PS-EXE bucket fixed while cutting blocking and loop-read time
`51 -> 50`. The v477 pack-only follow-up relocates that existing 768-byte
frame `132` payload into the unused setup-prime gap at offset `203181`
/ sector `99`, keeping VIST3LOW bytes, LBA/sectors, and the `217088` byte
PS-EXE bucket fixed while improving scene `1397 -> 1394`, active loop/target
`1068/1041 -> 1065/1041`, overrun `27 -> 24`, blocking and loop-read time
`50 -> 45`, loop reads `9 -> 8`, and due misses `9 -> 8`. The v510 pack-only
follow-up relocates the existing 503-byte frame `137` D4 payload into the
same setup-prime in-data gap family at offset `203949` / sector `99`,
keeping VIST3LOW bytes, LBA/sectors, and the `217088` byte PS-EXE bucket
fixed while improving scene `1394 -> 1391`, active loop/target
`1065/1041 -> 1062/1040`, overrun `24 -> 22`, blocking and loop-read time
`45 -> 42`, loop reads `8 -> 7`, and due misses `8 -> 7`. VISITOR3 high,
BUILDING2 high/low, and WALKSTUF1 high/low stayed exact-flat as canaries.
Use this as the current VISITOR3 low baseline; future VISITOR3 work should
prefer pack-only/setup-owned relocation before adding hot decoder code.

Latest rejected VISITOR3 low terminal compaction: v824 tried to avoid the
closed D4/decode path by moving existing terminal payloads byte-for-byte into
the large gap after frame `129`, keeping `VIST3LOW.FG2` size, LBA/sectors,
payload bytes, source code, and the PS-EXE bucket fixed. The binary split
showed no promotable form: frame `130` alone regressed to `1395/1066`,
`130,131` was exact-flat at `1391/1062/1040`, `130,131,133,134` regressed to
`1394/1065/1041`, and the all-terminal move increased blocking `42 -> 46`
plus reads `7 -> 8`. Close byte-for-byte terminal relocation after frame
`129`; VISITOR3 low needs generated scheduler ownership, custom compression
with timing proof, or newly paid residency rather than same-payload compaction.

Latest rejected VISITOR3 low resident-frame attempt: v476 tried to combine a
frame `128` D4 rewrite with moving frame `132` into a low setup segment. The
focused low run showed the same speed shape later captured by v477, but the
BUILDING2 low canary failed to emit complete `JCPERF2` metrics with that
source/pack shape. Restoring source and the pack made BUILDING2 low pass
exact-flat, so v476 is closed. The durable lesson is to keep VISITOR3 low
frame132 ownership pack-only when an unused setup-prime gap can hold the
existing 768-byte D4 payload.

Latest promoted WALKSTUF1 baseline: both tides now share the accepted
`201..213`, `213..229`, `344..360`, `422..434`, `443..455`, and `444..456`
read groups, and low tide adds the first post-prime boundary extension
`78..91`. The v340 strict gate moved high target `1429 -> 1432`, overrun
`51 -> 48`, blocking `85 -> 83`, loop reads `69 -> 67`, and loop-read time
`301 -> 292` while keeping scene/loop flat at `1768/1480`; the v383/v384
follow-ups keep all high timing metrics flat and lower loop reads `67 -> 65`
plus loop-read time `292 -> 284`. The v458 current-control refresh records the
current accepted high row at `1476/1434`, overrun `42`, blocking `81`, hidden
refill `23`, loop reads `65`, loop-read time `282`, and due misses `16`; v657
keeps those timing counters flat while physically compacting the clipped
late-tail payloads, reducing loop reads/read time to `63/275`. The
v474 low boundary fix extends the failed `78..90` row to `78..91` so it owns
the overlapping sector-89 follow-up read; low improves scene `1776 -> 1770`,
active loop/target `1484/1431 -> 1478/1431`, overrun `53 -> 47`, blocking
`72 -> 64`, hidden refill `22 -> 20`, loop reads `66 -> 64`, loop-read time
`287 -> 286`, and due misses `12 -> 11`. WALKSTUF1 high, BUILDING2 high/low,
and VISITOR3 high/low canaries stayed on accepted profiles with fixed pack
LBA/sectors and PS-EXE bucket. The v598 same-speed follow-up adds shared
`427..443`, keeping low timing exact-flat at `1770`, `1478/1431`, overrun
`47`, blocking/refill `64/20`, and due `11` while cutting loop reads
`64 -> 62` and loop-read time `286 -> 281`; WALKSTUF1 high plus VISITOR3
high/low and BUILDING2 high/low stayed exact-flat. The v474 promotion adds
about `0.0033` public over-target points removed and `0.0031` public
target-speed points gained; v598 is a CD-work baseline improvement only.

Latest rejected WALKSTUF1 low scheduler-guard batch: v580-v585 tested whether
the fresh scheduler-owned matrix rows could be isolated with low-tide-only
`minSlack` thresholds instead of plain retained rows. `{242,266,4}`,
`{285,309,5}`, and `{308,332,4}` were exact-flat at the accepted `1770`,
`1478/1431`, overrun `47`, blocking/refill `64/20`, reads/due `64/11`.
`{285,309,4}` fired but regressed scene `1770 -> 1776`, active loop
`1478 -> 1484`, overrun `47 -> 51`, blocking `64 -> 70`, refill `20 -> 25`,
while reads/due improved `64/11 -> 60/10`. `{141,165,0}` and `{141,165,4}`
both improved blocking `64 -> 63` and reads/due `64/11 -> 62/10`, but still
regressed scene `1770 -> 1776`, active loop `1478 -> 1484`, overrun
`47 -> 52`, and refill `20 -> 25`. Close scalar/scheduler-threshold ownership
for `242..266`, `285..309`, `308..332`, and `141..165`; the useful CD wins
sit behind a binary cliff where the firing threshold still steals visible or
refill cadence. The next WALKSTUF1 low path should be generated frame-deadline
ownership, pack-side byte reduction before those clusters, or upload/restore
work removal rather than more hand-authored read-group thresholds.

Latest rejected WALKSTUF1 low scalar-closure follow-up: v586-v590 moved
through the next fresh rows after the scheduler-guard batch. `{305,329,0}` was
exact-flat at the accepted `1770`, `1478/1431`, overrun `47`,
blocking/refill `64/20`, reads/due `64/11`. `{148,172,0}` and `{148,172,4}`
were both destructive, regressing active loop/target to `1490/1429`, overrun
`61`, blocking `88`, refill `32`, and due misses `13` while only cutting loop
reads to `62`. `{364,388,0}` and `{364,388,4}` were the only mildly useful
signal, cutting reads `64 -> 61`, but still regressing active loop/target to
`1481/1433`, overrun `48`, blocking `68`, and refill `24`. Close `305..329`,
`148..172`, and `364..388` for scalar guarded forms. This leaves the remaining
read-table queue mostly as cleanup; the larger WALKSTUF1 low gain needs a
generated frame-deadline owner or a data/upload reduction before the CD
clusters rather than more hand-authored thresholds.
The 2026-05-14 v728/v729 follow-up tested the current lower-risk tail hints
after the W1-low singleton pass was exhausted. `{371,387,0}` cut reads
`60 -> 58` but regressed scene/loop `1770/1478 -> 1772/1480`, blocking/refill
`64/20 -> 67/23`; the adjacent `{387,399,0}` split cut reads `60 -> 59` but
still regressed to `1771/1479`, blocking/refill `66/22`. Close `371..399` for
direct retained rows; this confirms the remaining W1-low CD work requires
generated deadline ownership or pack/upload reduction before timing.

Latest rejected WALKSTUF1 low post-prepare refill sweep: v609-v612 tested
whether low tide could keep the accepted visual prepare-before-window policy
and then reclaim CD window work from leftover held slack. Slack6 did not fire
and stayed exact-flat at `1770`, `1478/1431`, overrun `47`,
blocking/refill `64/20`, reads `62`. Any-slack, slack2, and slack4 fired but
all regressed to `1776`, `1484/1428`, overrun `56`, blocking/refill `81/23`,
reads `66`, and due misses `14`. Close this branch as binary no-op/regress;
future W1 low work needs generated frame-deadline ownership or pack/upload
work reduction before scheduling, not a second speculative window after
prepare.

Latest rejected WALKSTUF1 low two-phase boundary retry: v630-v632 kept the
accepted `78..91` row and added a separate `91..98` grouped append with
slack gates `6`, `5`, and `4`. All feasible thresholds were exact-flat at
scene `1770`, `1478/1431`, overrun `47`, blocking/refill `64/20`, reads
`62`, loop-read time `281`, due misses `11`, and `group_hits=0`; slack `3`
cannot fire because W1 low rejects window prefetch at `<=3` slack. Close
two-phase `91..98` scalar ownership for this baseline. The next W1-low work
should target generated deadline ownership for the `285..321` cluster or a
custom pack representation that reduces bytes/spans before scheduling.

Latest rejected WALKSTUF1 low late-cluster scalar retry: v633-v634 tested the
read-plan's largest current-fit row, `297..321`, at slack `5` and `4`. Both
runs stayed exact-flat at `1770`, `1478/1431`, overrun `47`,
blocking/refill `64/20`, reads `62`, loop-read time `281`, due misses `11`,
and `group_hits=0`; slack below `4` is unreachable under the W1-low window
guard. Close plain `297..321` scalar ownership. The range is a real cluster,
but the current runtime append boundary does not line up with it, so future
generated ownership must use observed append starts or a pack reorder that
changes the boundary.

Latest rejected WALKSTUF1 compact-origin rebase: v616 dry-ran the VISITOR3
origin-rebase compactor over both accepted W1 packs (`WALK1LOW.FG2` and
`WALKSTUF1.FG2`) for frames `0..215`. Both packs returned `total_saved=0`,
`applied_saved=0`, and `frames_rewritten=0`, so no pack was modified and no
PS1 timing gate was warranted. Close full-payload origin rebasing for current
W1 data; the next pack-side path needs smaller row-span/header
canonicalization, a scene-local dictionary, generated frame-deadline ownership,
or upload/restore work reduction rather than another whole-payload rebase.

Latest rejected WALKSTUF1 preserve-offset draw-gap coalescing: v617-v619
replayed known palette-index foreground pixels and merged only draw-span gaps
that were already known foreground. The transform produced large byte/span
savings (`77..94 KiB` and `40k..53k` merged gaps depending on tide/gap cap),
but timing moved the wrong way: low gap4 cut reads `62 -> 59` while regressing
to `1772`, `1480/1426`, overrun `54`, blocking/refill `75/29`; low gap1 was
scene/loop-flat but target/refill-negative at `1478/1424`, overrun `54`,
blocking/refill `77/27`; high gap1 regressed to `1774`, `1486/1424`,
overrun `62`, blocking/refill `90/38`, reads `74`. Close preserve-offset
draw-gap coalescing for W1: savings that add pixels are not enough. The next
pack-side lane needs semantic/precomposed strip or row-template compression
that reduces spans without increasing draw pixels, or generated frame-deadline
ownership/upload reduction.

Latest rejected WALKSTUF1 low window resweep: v620 retested explicit low-tide
stream windows after the shared `427..443` row changed the read shape. Current
default `40 KiB` remains best at `1770`, `1478/1431`, overrun `47`,
blocking/refill `64/20`, reads `62`, due `11`. Explicit `36`, `38`, `42`,
`44`, and `48 KiB` all cut reads (`22..31`) but regressed loop to
`1521..1533`, overrun to `85..97`, blocking to `96..109`, and refill to
`41..55`. Close low-window retuning: W1 low is not solved by read count alone;
the next attempt must reduce visual/upload work or use generated deadlines
that avoid visible/refill ownership.

Latest promoted BUILDING2 low scalar salvage: v626 narrows the one-refill
`218..230` near-miss to `218..229` with a slack-8 guard. It keeps hidden
refill at `0` while preserving the useful visible win: scene/loop
`1619/1349 -> 1614/1344`, target `1320 -> 1318`, overrun `29 -> 26`,
blocking `70 -> 61`, loop reads `52 -> 50`, loop-read time `221 -> 218`,
and due misses `15 -> 14`. VISITOR3 high/low, BUILDING2 high, and WALKSTUF1
high/low controls stayed exact-flat; pack LBA/sectors and the `217088` byte
PS-EXE bucket stayed fixed. Treat this as the current BUILDING2 low baseline.

Latest rejected BUILDING2 low current-baseline scalar retries: v900 retested
the wider `{218,230,8}` row after the v739 pack-speed baseline. It stayed
timing-flat at `1603`, `1339/1317`, overrun `22`, and refill `0`, but regressed
blocking `53 -> 54` and produced no key improvement, so `{218,229,8}` remains
the accepted edge. v900b then tested refreshed planner row `{96,112,0}`; it
saved reads `37 -> 34`, but regressed scene/loop/target `1603/1339/1317 ->
1608/1344/1313`, overrun `22 -> 31`, blocking/refill `53/0 -> 57/2`, and only
improved due `12 -> 11`. Close B2-low `218..230` and `96..112` as current
hand-table retries; future low work must use generated refill/deadline
ownership, upload/restore reduction, or pack-side byte changes that keep
`prefetch_overrun_vb=0`.

Latest rejected BUILDING2 low D4-hole physical compaction: v613-v615 removed
the two interior gaps left by tiny D4 frames before the `218..230` near-miss
cluster, first together and then independently. Removing both gaps shifted
payloads earlier by `14562` bytes but regressed to `1627`, `1357/1318`,
overrun `39`, blocking/refill `73/2`, reads `55`, due `17`. Removing only the
later gap was worse on CD (`blocking/refill 87/3`, reads `58`, due `19`);
removing only the earlier gap still failed at `1623`, `1353/1317`, overrun
`36`, blocking/refill `76/2`. Close broad physical gap compaction: the holes
are phase-bearing padding, and the BUILDING2 low near-miss still needs a
targeted one-sector byte reduction near `218..230`, generated deadline
ownership, or upload/restore work reduction.

Latest promoted WALKSTUF1 low shared-tail CD-work reduction: v598 promoted
the only safe result from the v591-v597 tail closure as a shared table row
instead of a low-only branch. Low timing stays exact-flat at scene `1770`,
active loop/target `1478/1431`, overrun `47`, blocking/refill `64/20`, and
due misses `11`, while loop reads drop `64 -> 62` and loop-read VBlanks
`286 -> 281`. WALKSTUF1 high stays exact-flat at `1764`, `1476/1434`,
overrun `42`, blocking/refill `81/23`, reads/due `65/16`; VISITOR3 high/low
and BUILDING2 high/low also stayed exact-flat with fixed pack LBAs and the
`217088` byte PS-EXE bucket. Treat this as a lower-CD-pressure baseline, not a
target-speed win. The closed tail rows are `303..319`, `155..171`, `91..107`,
`405..421`, and `378..394`; `405..421` remains the best near-miss because it
lowered scene/loop by one but cost one blocking/refill VBlank.

Latest promoted WALKSTUF1 high work-volume baseline: v731/v732/v733/v734/v830/v831/v832/v833/v834/v835/v836/v837
continue the safe singleton offscreen lane after the v654 late-tail clip,
adding frames `55`, `138`, `51`, `49`, `47`, `45`, `43`, `56`, `57`, `136`, `135`, and `139` while preserving file size,
pack LBA/sectors, and the PS-EXE bucket. Timing/CD stay at scene `1764`,
active loop/target `1476/1434`, overrun `42`, blocking/refill `81/23`, reads
`63`, and due `16`; runtime rows/spans/pixels now drop `17296/134136/776856 ->
16859/129919/731016`. This is not a speed win, but it is the current
WALKSTUF1 high work-volume baseline before generated deadline/upload work. v882
then shrinks the already-clipped frame `138` payload in place, v884 follows
with frame `135`, and v927 follows with frame `139`, keeping this row exact-flat
while reducing active payload to `859666`.

Latest rejected WALKSTUF1 high scalar-closure batch: v599-v608 tested the
remaining high-ranked shared retained-read rows after the v598 baseline. The
exact-flat/no-op rows were `{295,319,0}`, `{238,262,5}`, and `{306,330,0}` at
the accepted high timing `1764`, `1476/1434`, overrun `42`,
blocking/refill `81/23`, reads `65`. `{84,108,0}`, `{164,188,0}`,
`{156,180,0}`, and the lower-risk `{268,280,0}` were visibly phase-negative,
with `{268,280,0}` still regressing to `1772`, `1484/1424`, overrun `60`,
blocking/refill `98/27`. `{238,262,0}` / `{238,262,4}` and `{360,384,0}`
saved reads or blocking but paid it back as scene/loop/refill debt. Close
WALKSTUF1 high shared scalar appends for this phase; remaining high-side gain
needs generated deadline ownership, pack-side byte/phase reduction before the
cluster, or upload/restore work removal.

Latest rejected WALKSTUF1 low boundary-extension retry: v475 replaced the
accepted `78..91` first post-prime retained-read group with `78..98` so the
same 3.21s gap could also cover the next `91..98` read while still fitting the
24-sector grouped window. The extra sectors were phase-negative: low regressed
scene `1770 -> 1783`, active loop/target `1478/1431 -> 1491/1424`, overrun
`47 -> 67`, blocking `64 -> 101`, hidden refill `20 -> 26`, loop reads
`64 -> 65`, loop-read time `286 -> 303`, and due misses `11 -> 15`. Keep
`78..91` as the accepted endpoint; with older `78..94` already closed and
`78..98` worse, do not hand-extend the early WALKSTUF1 low group past sector
`91`. The next low work needs generated deadline ownership, cold sidecar
residency, or a custom data shape that does not pull more sectors into this
visible phase.

Latest rejected WALKSTUF1 low mid-row retained-read family: v502 split the
WALKSTUF1 table by tide and tested `{196,208}`, `{190,202}`, `{209,225}`, and
`{225,241}` as low-only rows. `{196,208}` and `{225,241}` were exact-flat at
`1770`, `1478/1431`, overrun `47`, blocking/refill `64/20`, and reads/due
`64/11`, with added hot code shape. `{190,202}` regressed to `1490/1428`,
overrun `62`, blocking `96`, refill `27`, due `15`; `{209,225}` saved one loop
read (`64 -> 63`) but still regressed to `1490/1431`, overrun `59`, blocking
`93`, refill `30`, due `14`. This closes the current low-only scalar mid-row
table family. The older v497 guarded `{273,285,8}` row also remains closed
because it saved one read but regressed blocking/refill `64/20 -> 66/21`.
Continue with generated deadline ownership, source-neutral pack relocation, or
upload/restore work reduction instead of more hand-authored retained rows.

Latest rejected WALKSTUF1 low tight-cluster retest: v504 split the shared
table by tide and added `{285,297}` only to low, after quick pack probes showed
no cheap existing cleanup transform (`0` no-op entries, `0` draw-tail bytes,
and restore-minus-current grew active payload). The focused gate saved one read
(`64 -> 63`) but regressed scene/loop `1770/1478 -> 1778/1486`, overrun
`47 -> 55`, blocking `64 -> 75`, and hidden refill `20 -> 26`; due stayed
`11`. Close `285..297` on the current baseline too. The late low cluster needs
generated deadline ownership or a custom data/upload representation, not a
low-only hand table.

Latest rejected WALKSTUF1 low v817 tight-cluster follow-up: v828 tried the
next high-upside direct table row `{297,309}` after v817 and the closed
`410..422`, `190..202`, `273..285`, and `285..297` probes. It saved one loop
read (`58 -> 57`) but regressed scene/loop/target `1769/1477/1431 ->
1776/1484/1430`, overrun `46 -> 54`, blocking/refill `64/20 -> 74/26`, and
left due at `11`. Close hand-authored W1-low tight-cluster read groups on the
v817 baseline; the next W1-low attempt needs generated deadline ownership or
pack/upload work reduction with a host proof before changing source.

Latest rejected WALKSTUF1 low v847 `410..422` retest: v848 rechecked the
direct low-only `{410,422}` row after the v846/v847 preserve-offset payload
shrinks made v847 the new runtime baseline. It again saved one loop read and
one read VBlank (`58/266 -> 57/265`) but regressed scene/loop/overrun
`1769/1477/46 -> 1770/1478/47`, with target, blocking/refill, and due unchanged
at `1431`, `64/20`, and `11`. Keep `410..422` closed under the v847 baseline
too; remaining low retained windows need generated ownership or a different
pack/data shape, not another scalar table retry.

Latest rejected WALKSTUF1 low setup-prime prefix-hole fill: v509 moved frame
`78` / source `137` and frame `80` / source `141` into the already-covered
`296`-byte prefix hole before `data_offset=4392`, using `247` bytes while
keeping pack LBA, file size, source, and PS-EXE bucket fixed. The focused gate
failed structurally with missing `JCPERF2` and invalid byte reads around
`0x4209FC4D..0x4209FC79` at PC `0x80034D20`, then hit the clean-retry prefetch
drop path. Close prefix metadata/setup holes as FGP3 payload targets: even when
the sector is setup-prime resident, payload entries must stay in the data area
or in a mechanically proven in-data hole. Future WALKSTUF1 low relocation work
needs a valid in-data hole, generated/code-neutral residency, or a custom
upload/data representation.

Latest rejected WALKSTUF1 low D4 probes: v453 encoded frame `181` as a
previous-frame delta against frame `180`, shrinking that payload
`4708 -> 1484` bytes and reducing its modeled sector span from four to three.
Both runtime shapes preserved pack LBA and the `217088` byte PS-EXE bucket and
improved public cadence (`1484/1431 -> 1481/1433`, overrun `53 -> 48`), but
visible blocking regressed `72 -> 76` and due misses regressed `12 -> 14`.
v455 then tried the safer `190..202` neighborhood by encoding only frame `100`
against frame `99`, shrinking `4683 -> 586` bytes and reducing its span from
three sectors to one, but the focused gate was exact-flat at `1776`,
`1484/1431`, overrun `53`, blocking `72`, refill `22`, reads/due `66/12`.
v456 then tried late-tail frame `187` against frame `186`, shrinking
`4764 -> 731` bytes and reducing its modeled span from roughly four sectors to
one. The first run died before metrics, and the clean rerun was exact-flat at
`1776`, `1484/1431`, overrun `53`, blocking `72`, refill `22`, reads/due
`66/12`. v461 tested an earlier lower-command-count candidate, frame `92`
against frame `91`, shrinking `6035 -> 2056` bytes and reducing the modeled
span by two sectors, but it regressed hard: scene `1776 -> 1790`,
active loop/target `1484/1431 -> 1498/1421`, overrun `53 -> 77`, blocking
`72 -> 114`, refill `22 -> 35`, reads `66 -> 70`, and due misses
`12 -> 17`. v465 tried frame `189` against frame `188`, shrinking
`5382 -> 1444` bytes and saving one loop read, but still regressed active
loop/target `1484/1431 -> 1487/1429`, overrun `53 -> 58`, blocking `72 -> 80`,
refill `22 -> 23`, and due `12 -> 13`. v466 then combined frame `28` D4
hole-fill with setup-prime sidecar relocation for late frames `181`, `187`,
and `189`; it preserved pack LBA and saved one loop read, hidden refill
`22 -> 19`, but regressed overrun `53 -> 55`, blocking `72 -> 82`, and due
`12 -> 15` while moving hot foreground symbols. Close frames `181`, `100`,
`187`, `189`, `92`, and setup-hole D4 relocation as standalone/hand-coded D4
lanes. v471 then tried the untested earlier frame `59`, shrinking
`4200 -> 1452` bytes and collapsing sectors `109..111` to one sector, but it
stayed exact-flat at `1776`, `1484/1431`, overrun `53`, blocking `72`, refill
`22`, reads/due `66/12`, while growing/shifting hot foreground symbols.
WALKSTUF1 low needs generated scheduler/deadline ownership, a cold
sidecar that does not grow hot loop code, or a different custom data shape if
this mechanism is retried.

Latest rejected WALKSTUF1 low sector-boundary D4 retry: v480 encoded frame
`140` against frame `139`, shrinking `4296 -> 1968` bytes in the lower-risk
`273..285` cluster without moving later payload offsets. The focused pass was
exact-flat at scene `1770`, active loop/target `1478/1431`, overrun `47`,
blocking `64`, refill `20`, reads/due `64/11`. v481 then batched non-adjacent
hot-cluster frames `140`, `145`, `152`, `155`, `158`, and `161`, saving
`10925` payload bytes and cutting reads `64 -> 62` plus refill `20 -> 19`, but
regressed scene `1770 -> 1798`, active loop/target `1478/1431 -> 1506/1423`,
overrun `47 -> 83`, blocking `64 -> 90`, and due misses `11 -> 15`. Close
multi-frame D4 sector-boundary compression for WALKSTUF1 low under the current
decoder/cadence; future byte cuts need a lower-CPU representation or generated
CD ownership.

Latest promoted WALKSTUF1 high D4 probe: the current allocator-era baseline
reopened previous-frame D4 with a fresh same-source gate. Encoding high frame
`92` against frame `91` shrinks `6035 -> 2056` bytes with `26` commands and
promotes because the focused row improves scene/loop/target
`1811/1475/1441 -> 1807/1471/1440`, overrun `34 -> 31`, and target speed
`97.695% -> 97.893%`; blocking/refill stay `57/13`, due stays `10`, and
VISITOR3 high/low, BUILDING2 high/low, WALKSTUF1 low, and BUILDING4 low stayed
flat. Keep this as a narrow exception to the old D4 closure: frame `100` was a
stale-baseline exact-flat miss, frame `175` failed, and VISITOR3/B2 large-frame
D4 still fails unless the candidate proves a current-baseline visible win.

Latest promoted BUILDING2 high baseline: keep accepted `60..72`, `226..242`,
and `249..257`, add `206..230`, and raise grouped-read capacity to 24 sectors.
The v441 current-control row improves scene `1603 -> 1602`, active loop/target
`1352/1311 -> 1351/1311`, overrun `41 -> 40`, blocking `55 -> 54`, hidden
refill `19 -> 18`, loop reads `61 -> 58`, and loop-read time `262 -> 257`;
due misses stay `7`. BUILDING2 low, WALKSTUF1 high/low, and VISITOR3 low
canaries stayed flat. The v664 late-only work-volume follow-up clips high
frames `168..177` after broad/hot offscreen clipping proved phase-negative;
v698 then extends the safe same-speed subset to frames `94..104`, and v700
adds the adjacent boundary singleton frame `92`; v701/v702/v703 add adjacent
frames `91`, `90`, and `89` from the same safe side. v877/v879/v880/v887/v888/v889/v890/v891/v892/v896/v914/v926 then trim
entries `172` / source frame `231`, `171` / source frame `228`, `96` /
source frame `119`, `170` / source frame `226`, `97` / source frame `121`, `98` / source frame `123`, `174` / source frame `239`, `99` / source frame `126`, `168` / source frame `219`, `169` / source frame `223`, `173` / source frame `235`, and `100` / source frame `129`
in place with preserved offsets, shrinking `1831 -> 851`, `1980 -> 1025`,
`8781 -> 7944`, `1683 -> 1186`, `8718 -> 8258`, `8876 -> 8637`, and
`1625 -> 1460`, `8843 -> 8728`, `1372 -> 1266`, `1820 -> 1495`, `1765 -> 1134`, and `8701 -> 8621` bytes. The current follow-up trims entries `92`, `94`, and `95` (`8834 -> 6370`, `8873 -> 6939`, `10247 -> 8827`) while active payload drops `674798 -> 663590`. All stay exact-flat at scene/loop/target
`1602/1351/1311`, overrun `40`, blocking/refill `54/18`, reads/read time
`58/257`, and due `7`, and drop runtime frame rows/spans/pixels
`18144/110717/468636 -> 18030/105645/446246`. Use v926 as the current
same-speed BUILDING2 high work baseline plus the entries `92`/`94`/`95` trim
as the current payload baseline for future comparisons. The current read-work
rows add `{185..197}` and `{158..174}` and keep B2-high speed exact-flat while
reducing loop reads/read time `45/199 -> 40/189`; `{287..311}` was exact-flat
without a key win and `{249..261}` regressed visible CD pressure.
The allocator-era speed row now layers targeted setup slices with `83..95`,
`{158..174}`, guarded `271..287`, `315..327`, `{185..197}`, and the
previous-visible cleanup plus preserve-entry-size screen-clip promotions,
moving B2-high to `1343/1312` while cutting restore/upload work and leaving
current active CD pressure at blocking/refill `50/17`, reads/read time
`44/196`, and due `7`.
The v704 frame `88` follow-up was a host no-op (`0` frames changed), so the
direct boundary lane now stops at frame `89` unless a different transform
finds new removable work. The later v894 preserve-offset entry-size retry for
frame `90` (`8849 -> 4769`, sector coverage `5 -> 3`) also failed: it saved one
loop read but regressed scene/loop/target `1602/1351/1311 -> 1605/1354/1310`,
overrun `40 -> 44`, and refill `18 -> 20`, so do not retry post-hot
sector-changing entry shrinks without generated deadline ownership.

Latest rejected BUILDING2 high draw-tail sector-collapse swing: the current
preserve-offset trim scanner still finds `12084` removable bytes across 11
entries, mostly entries `89..91`, but the broad trim regressed B2-high from
`1621/1347/1313`, overrun `34`, blocking/refill `39/16`, reads `43`, to
`1622/1349/1313`, overrun `36`, blocking/refill `41/15`, reads `41`.
Isolating entry `91` still regressed the same strict counters while reducing
only one read. Artifacts:
`scratch/ps1-perf-iterate/building2-high-trimdrawtails-current/20260520-010146-1859565/summary.json`
and
`scratch/ps1-perf-iterate/building2-high-trimdrawtails-entry91-current/20260520-010330-1869410/summary.json`.
Close B2-high entries `89..91` under draw-tail sector-collapse; the next
B2-high path needs generated deadline ownership or upload/restore elimination,
not another no-decode tail shrink that changes cadence.

Latest rejected BUILDING2 high frame71 setup-gap D4 swing: frame `71` can be
encoded as a previous-frame D4 payload (`6656 -> 13` bytes) and physically
moved into the already-paid setup gap at offset `6144`, but enabling that
high-tide D4 path still regressed B2-high from `1621/1347/1313` to
`1626/1353/1310`. Overrun moved `34 -> 43`, blocking/refill `39/16 -> 43/20`,
and reads moved `43 -> 44` with fixed pack LBA and fixed `233472` byte PS-EXE
bucket. Artifact:
`scratch/ps1-perf-iterate/building2-high-frame71-primegap-current/20260520-013252-2034671/summary.json`.
Close B2-high frame71 D4 and local decode variants even when payload ownership
is setup-side; remaining B2-high work should be generated deadline ownership,
upload/restore elimination, or no-decode cadence-preserving data.

Latest rejected BUILDING2 high clean-safe residency swap: entries `90..95`
were moved from the hot `141..164` physical span into the already retained
second setup segment at sectors `202..223`, while evicting setup-resident
entries `105..109` into the old hole at sectors `141..162`. The source-neutral
pack kept `BUILDING2.FG2` at `1303332` bytes and pack LBA fixed, but the
focused gate still moved B2-high from `1621/1347/1313` to `1621/1347/1311`,
overrun `34 -> 36`, blocking/refill `39/16 -> 40/18`, and reads stayed `43`.
Adding a temporary `{149..165}` grouped read over the evicted cluster reduced
reads `43 -> 40` but regressed scene/loop/target to `1625/1351/1311`, overrun
`40`, blocking `45`, and refill `20`. Artifacts:
`scratch/ps1-perf-iterate/building2-high-setup-swap90-95-current/20260520-014411-2098182/summary.json`
and
`scratch/ps1-perf-iterate/building2-high-setup-swap90-95-rg149-165-current/20260520-014616-2110053/summary.json`.
Close this same-footprint swap shape: moving active pressure into setup only
helps if the evicted setup bytes do not become a new tight active cluster.

Latest promoted BUILDING2 low baseline: keep accepted `238..250`, `318..330`,
and `365..381`, keep the v454 frame `71` / `77` previous-frame D4 deltas, and
add the v626 slack-8 `218..229` retained read ahead of them. The current
focused row improves scene/loop `1619/1349 -> 1614/1344`, target
`1320 -> 1318`, overrun `29 -> 26`, blocking `70 -> 61`, prefetch overrun
stays `0`, loop reads `52 -> 50`, loop-read time `221 -> 218`, and due
misses `15 -> 14`; pack LBA/sectors and the `217088` byte PS-EXE bucket stay
fixed. VISITOR3 high/low, BUILDING2 high, and WALKSTUF1 high/low controls
stayed flat. Use this as the current baseline for future BUILDING2 low
comparisons.

Latest rejected BUILDING2 low D4-hole repack: v479 reran the pack compactor
after teaching the restore-minus-current tool to skip existing D4 payloads.
The transform found no further cleanup-minus-current savings, then repacked
payloads after the accepted frame `71`/`77` D4 holes while preserving total
file size and LBA. The focused gate regressed low from `1619`, `1349/1320`,
overrun `29`, blocking `70`, refill `0`, reads/due `52/15` to `1627`,
`1357/1318`, overrun `39`, blocking `73`, refill `2`, reads/due `55/17`.
Close hole-closing repacks after v454; future BUILDING2 low pack work must
preserve CD phase explicitly or reduce real work without moving later payloads.

Latest rejected BUILDING2 low middle-row replacement: v505 replaced the
accepted `{318,330}` row with the planner-ranked wider `{314,338}` span while
keeping `{238,250}` and `{365,381}` unchanged. The replacement did reduce CD
pressure (`loop_reads 52 -> 50`, loop-read time `221 -> 217`, blocking
`70 -> 66`, due misses `15 -> 13`) and kept hidden refill at `0`, but strict
public cadence regressed: scene `1619 -> 1623`, active loop/target
`1349/1320 -> 1353/1321`, and overrun `29 -> 32`. Keep `{318,330}` as the
accepted middle row. Future BUILDING2 low read work needs generated
deadline/refill ownership or pack-side work reduction before replacing
accepted rows.

Latest promoted BUILDING6 slack baseline: keep the compact-FGP3/v4 pack shape
and add a scene-local `4` VBlank window-refill minimum. Fresh current-layout
baselines were high `2744`, `2479/2453`, overrun `26`, blocking `26`, hidden
refill `29`, and low `2748`, `2482/2456`, overrun `26`, blocking `28`, hidden
refill `30`. The v364 focused gates move high to `2736`, `2471/2456`, overrun
`15`, blocking `20`, hidden refill `16`; low moves to `2740`, `2474/2455`,
overrun `19`, blocking `24`, hidden refill `20`. Both tides accept one due
miss and keep `loop_reads=42`. BUILDING2 high/low, WALKSTUF1 high/low,
FISHING1 high, and ACTIVITY9 low canaries stayed on accepted profiles, while
VISITOR3 high still hits the known missing-`JCPERF2` measurement defect. This
moved BUILDING6 high/low into green and set the then-current rollup to
`+0.3159%` / `99.6903%` public and `-0.4511%` / `100.4726%` raw signed.

Latest rejected WALKSTUF1 low setup-owned cluster: the current read-plan's top
`291..307` signal was tested as reusable setup residency in v365 instead of an
active-loop read group. It reduced loop `1484 -> 1479`, overrun `53 -> 50`,
hidden refill `22 -> 21`, and loop reads `67 -> 65`, but the setup/CD phase
made visible blocking worse (`72 -> 92`), raised due misses (`12 -> 16`),
lowered target (`1431 -> 1429`), and regressed scene time (`1776 -> 1783`).
Close additive setup segments around this cluster; the low row now needs
phase-preserving pack shrink/relocation or generated deadline ownership, not
another fixed setup read.

Latest rejected WALKSTUF1 low pack-transform retry: v366 ran the existing
restore-minus-current transformer on the current low pack. Host-side it only
saved about `2 KB` of restore work while growing active payload by about
`1.2 KB`, and the focused gate confirmed the trade is wrong: loop regressed
`1484 -> 1493`, target fell `1431 -> 1426`, overrun rose `53 -> 67`,
blocking rose `72 -> 93`, hidden refill rose `22 -> 24`, and reads rose
`67 -> 69`. Close second-pass restore-minus-current for WALKSTUF1 low as well
as high.

Latest rejected WALKSTUF1 low contiguous-prime retest: v367 raised the
low-only setup-prime cap from `160 KiB` to `176 KiB` on the current baseline.
The focused gate was exact-flat (`1776`, `1484/1431`, overrun `53`, blocking
`72`, hidden refill `22`, reads `67`, due `12`), and the emitted read plan
still reported setup coverage `prime=[2, 78]`. This closes scalar cap growth
above `160 KiB`; the next WALKSTUF1 low work must be noncontiguous/generated
residency, phase-preserving pack relocation/shrink, or scheduler ownership.

Latest rejected WALKSTUF1 low offscreen/hole-fill lane: v378 tried the
size-preserving version of offscreen PAL4 draw clipping first, then narrowed to
setup-prime tail hole fills. Broad draw clipping trimmed `179604` offscreen
draw pixels and `23012` spans across `91` frames but regressed low to
`1780`, `1488/1426`, overrun `62`, blocking `86`, hidden refill `30`, reads
`71`, due `14`. The three-clone in-prime variant was worse (`1791`,
`1499/1421`, blocking `113`, hidden `37`, due `16`), and the minimal `35:190`
clone still regressed (`1780`, `1488/1427`, blocking `91`, due `14`). Close
naive draw clipping and tail-hole payload clones for WALKSTUF1 low; the next
low work must preserve frame/CD phase explicitly with generated reservations,
sector-boundary frame splits, or a custom format that changes work without
retiming the scheduler.

Latest rejected WALKSTUF1 low full-cluster retest: v380 retried the late
`285..309` retained-read group on the current baseline after v379 showed
expanded groups can turn a failed narrow range into a win. The group fired and
cut reads `67 -> 63` plus due misses `12 -> 11`, but it still regressed low to
scene `1782`, active loop/target `1490/1433`, overrun `57`, blocking `76`, and
hidden refill `26`. Close this retest as another phase-negative hand table;
future low work needs generated deadline ownership, sector-boundary planning,
or a custom format that changes work without moving the visible-loop cadence.
The v494 low-only narrower `285..297` table confirms the same class under the
current v474 baseline: reads improve `64 -> 63`, but scene/loop regress
`1770/1478 -> 1778/1486`, overrun `47 -> 55`, blocking `64 -> 75`, and hidden
refill `20 -> 26`. The v495 safer tail `387..399` table is also phase-negative:
reads improve `64 -> 63`, but scene/loop regress `1770/1478 -> 1771/1479`,
overrun `47 -> 48`, blocking `64 -> 66`, and hidden refill `20 -> 22`.

Latest rejected WALKSTUF1 low low-risk direct group: v381 tested `190..202`,
the safest remaining current-fit retained-read row from the v331 plan. It had
previously failed as setup-owned residency, but the direct table was worse than
baseline too: scene `1776 -> 1784`, active loop/target `1484/1431 -> 1492/1423`,
overrun `53 -> 69`, blocking `72 -> 90`, hidden refill `22 -> 34`, reads
`67 -> 70`, and due `12 -> 13`. Close the direct `190..202` lane; remaining
WALKSTUF1 low work should not spend more time on plain retained tables.

Latest rejected WALKSTUF1 low low-risk tail direct group: v486 tested the
current visible-cost planner's unclosed `371..387` retained-read candidate,
which was distinct from the older setup-owned `371..387` segment failure. The
normal focused gate was host-killed before `JCPERF2`; the clean case-local-CD
retry ran past the baseline finish window and was still issuing dense tail
sector reads around `44s` with no `JCPERF2` before the wrapper was killed with
exit `137`. No metrics were accepted and the source table was restored. Treat
`371..387` direct grouping as phase-negative/inconclusive under the current
layout; future work needs generated deadline ownership or pack-side data shape,
not another scalar retained-read row. The v493 low-only table retest completed
after stopping the external long-run relauncher and confirms the rejection:
reads improved `64 -> 62`, but scene/loop regressed `1770/1478 -> 1774/1482`,
overrun `47 -> 50`, blocking `64 -> 67`, and hidden refill `20 -> 23`.

Latest rejected WALKSTUF1 low shared-table split: v491 separated the shared
WALKSTUF1 retained-read table so low tide kept only the low-promoted `78..91`
and `443..455` groups while high retained its full group set. The focused low
harness exited `137` before `JCPERF2`; the orphaned emulator only reached
early boot/asset loads after another minute and produced no scene-complete
metrics. After stopping the external long-run relauncher, the v492 clean
rerun completed with valid metrics and was exact-flat at scene `1770`, active
loop/target `1478/1431`, overrun `47`, blocking `64`, hidden refill `20`, and
reads/due `64/11` while only shifting hot symbols. Restore the shared table;
low needs generated scheduler/data-shape ownership rather than manual removal
of high-only groups.

Latest rejected WALKSTUF1 low first post-setup retained group: v472 tested
`78..90`, the current read-plan's long-first-gap post-prime candidate. It did
reduce due misses `12 -> 11`, but it regressed scene `1776 -> 1780`, active
loop/target `1484/1431 -> 1488/1428`, overrun `53 -> 60`, blocking `72 -> 79`,
hidden refill `22 -> 27`, and loop reads `66 -> 68`. Close the narrowed
`78..90` form specifically; v474's promoted `78..91` boundary-fixed form owns
the overlapping sector-89 follow-up and is now the accepted low baseline.

Latest rejected runtime-shape cache: v473 cached scene/tide policy bits and
the window-slack threshold at scene start, replacing hot-loop scene-name
checks in D4 gating, direct-stage, visitor fallthrough, WALKSTUF1 window
gating, and the prepare-before-window branch. WALKSTUF1 low stayed exact-flat
at `1776`, `1484/1431`, overrun `53`, blocking `72`, refill `22`, reads/due
`66/12`; BUILDING2 high also stayed exact-flat at `1602`, `1351/1311`,
overrun `40`, blocking `54`, refill `18`, reads/due `58/7`. The PS-EXE bucket
stayed `217088`, but BUILDING2 high moved/got larger hot symbols
(`fgRuntimeLoadSceneFrame +284`, `foregroundPilotPlay +168`). Close naive
scene-policy caching as a speed lane; retry only as a broader cold-policy
refactor with a hot-symbol budget and same-layout proof.

Latest rejected WALKSTUF1 high direct group: v368 appended the current
read-plan `360..376` candidate after the accepted `344..360` group. It cut
reads `67 -> 65` and loop-read VBlanks `292 -> 289`, but regressed scene
`1768 -> 1774`, loop/target `1480/1432 -> 1486/1431`, overrun `48 -> 55`,
blocking `83 -> 87`, and hidden refill `26 -> 28`. Close direct high-table
follow-ups in this neighborhood; the next high work needs generated scheduler
ownership, setup-safe noncontiguous residency, or pack-side byte removal.

Latest rejected WALKSTUF1 high direct-group retry: v405/v406 tested the
unclosed `178..194` retained-read candidate from the accepted v384 plan, first
ungated and then with `minSlack=4`. Both variants fired the same bad schedule:
loop reads improved `65 -> 64`, but scene regressed `1768 -> 1774`, active
loop/target `1480/1432 -> 1486/1427`, overrun `48 -> 59`, blocking
`83 -> 98`, and due misses `16 -> 17`; hidden refill improved only
`26 -> 23`. Close the `178..194` hand-table lane. The remaining high-side
mid-pack rows are not missing a simple slack threshold; they need generated
deadline ownership, sector-boundary pack shaping, or actual payload reduction.
The v901 current-baseline `156..164` follow-up is also closed: it was the
refreshed low-visible-risk one-read W1-high candidate with zero overread, but
the table insertion regressed scene/loop/target `1764/1476/1434 ->
1774/1486/1427`, overrun `42 -> 59`, blocking `81 -> 102`, and due
`16 -> 17` while loop reads stayed `63`. Do not retry W1-high mid-pack
hand-authored read rows without generated scheduler/refill ownership.
The v902 `238..246` companion row closes the other low-visible-risk refreshed
high candidate: it regressed scene/loop/target `1764/1476/1434 ->
1778/1490/1420`, overrun `42 -> 70`, blocking `81 -> 111`, hidden refill
`23 -> 32`, due `16 -> 19`, and loop reads `63 -> 66`. Treat remaining
W1-high table rows as generated-metadata-only work, not scalar source probes.

Latest rejected WALKSTUF1 high staged-prepare ownership probe: v483 removed the
low-tide guard from the accepted WALKSTUF1 prepare-before-window scheduler
branch so high tide would prepare the staged visual frame before speculative
window refill. Low's rule does not transfer: focused high regressed scene
`1764 -> 1787`, active loop/target `1476/1434 -> 1499/1425`, overrun
`42 -> 74`, blocking `81 -> 103`, refill `23 -> 32`, and reads `65 -> 67`.
Keep the staged-prepare-before-window rule low-only; high still needs explicit
generated ownership or data-shape reduction, not the low visual-work heuristic.

Latest rejected WALKSTUF1 high adjacent-group merge: v506 replaced the accepted
split rows `{201,213}` and `{213,229}` with one 24-sector `{201,225}` row. The
candidate tested whether a simpler table shape could keep the modeled `201..225`
read-plan saving without adding hot code. It was phase-negative: high regressed
scene `1764 -> 1782`, active loop/target `1476/1434 -> 1494/1427`, overrun
`42 -> 67`, blocking `81 -> 108`, and due misses `16 -> 18`, while loop reads
stayed `65`. Keep the accepted split rows. Future high work needs generated
deadline ownership or pack-side payload/work reduction, not adjacent-row merging.

Latest rejected WALKSTUF1 high window retune: v382 rechecked the old adjacent
window-size signal as parameter-only probes. Current `54 KiB` remains the knee:
`53 KiB` cut reads `67 -> 22` but regressed to `1820`, `1531/1434`, overrun
`97`, blocking `115`, hidden refill `46`; `55 KiB` cut reads to `19` but
regressed to `1828`, `1539/1439`, overrun `100`, blocking `108`, hidden refill
`49`. Do not source-promote adjacent high-window sizes under this layout.

Latest rejected BUILDING2 high early group: v369 inserted `17..33` before the
accepted high groups. It stayed exact-flat against v347 (`1603`,
`1352/1311`, overrun `41`, blocking `55`, hidden refill `19`, reads `61`,
due `7`) while only shifting hot-symbol addresses by `+32` bytes. v881 retested
the same row after the v880 payload baseline; it saved reads `58 -> 55` and
blocking `54 -> 53`, but regressed scene/loop `1602/1351 -> 1607/1356` and
overrun `40 -> 45`. Close this early scalar row; BUILDING2 high now needs
generated ownership or data-shape work rather than more hand table rows.

Latest rejected BUILDING2 high direct-stage ownership probe: v370 kept
`17..33` and denied exact-min direct-stage only for BUILDING2 high sector
`17`. It was still exact-flat on all key metrics while growing
`foregroundPilotPlay` by `64` bytes. Close hand-coded sector exceptions; the
next BUILDING2 high work should be generated metadata or pack/data-shape work.

Latest rejected BUILDING2 high middle-cluster retest: v419 tried the refreshed
`210..222` row after the accepted `226..242` promotion, avoiding the overlap
that made the older `210..226` attempt inert. The group fired and reduced loop
reads `61 -> 59`, but it regressed high from `1602`, `1351/1313`, overrun
`38`, blocking `50`, hidden refill `17` to `1607`, `1356/1311`, overrun `45`,
blocking `57`, hidden refill `23`. Close `210..222` as another local table;
the middle cluster needs generated phase ownership or real byte reduction.

Latest rejected BUILDING2 high replacement group: v420 replaced the accepted
`226..242` group with adjacent `222..238` to test whether the middle cluster
could be shifted earlier without adding another table. It kept reads/due misses
at `61/7`, but lost the v379 win: scene `1602 -> 1603`, active loop/target
`1351/1313 -> 1352/1311`, overrun `38 -> 41`, blocking `50 -> 55`, and hidden
refill `17 -> 19`. Keep `226..242`; close adjacent replacement hand nudges
unless generated deadline ownership or pack-side byte reduction changes phase.

Latest rejected BUILDING2 high reusable setup segment: v459 retained the early
`11..27` sector cluster through the existing setup-segment buffer instead of
raising a contiguous setup-prime window. It cut loop reads `58 -> 55` and
loop-read VBlanks `257 -> 250`, but regressed scene `1602 -> 1619`,
active loop/target `1351/1311 -> 1359/1310`, overrun `40 -> 49`, blocking
`54 -> 62`, and hidden refill `18 -> 26`, while growing hot foreground code by
`192` bytes. Close hand-authored BUILDING2 high setup segments; retry this
class only as generated ownership metadata, pack-side byte reduction, or
selective/preprocessed render data.

Latest rejected BUILDING2 high D4 split: v463 isolated the only meaningful
high-tide previous-frame delta candidates from the current scan. Frame `77`
shrinks `8828 -> 909` bytes but repeats the high-side phase failure: loop stays
`1351`, target drops `1311 -> 1310`, blocking regresses `54 -> 56`, hidden
refill regresses `18 -> 19`, and overrun moves `40 -> 41` while reads drop
`58 -> 57`. Frame `71` shrinks `6656 -> 13` bytes but is exact-flat at
`1602`, `1351/1311`, overrun `40`, blocking `54`, refill `18`, reads/due
`58/7`. Together with the earlier paired high probe, close BUILDING2 high
frames `71` and `77` as standalone D4 deltas; high still needs generated
deadline ownership or a different pack shape. The v492 current-baseline
frame71 retest reconfirmed this closure: `6656 -> 13` bytes, fixed pack
footprint/LBA, valid `JCPERF2`, and exact-flat timing/pressure at `1602`,
`1351/1311`, overrun `40`, blocking `54`, hidden refill `18`, reads/due
`58/7`.

Latest rejected BUILDING2 high aligned middle group: v467 replaced the accepted
`206..230` read group with the current read-plan's largest `202..226` signal,
leaving `226..242` in place as the follow-up. The rerun was exact-flat at
scene `1602`, active loop/target `1351/1311`, overrun `40`, blocking `54`,
hidden refill `18`, reads/due `58/7`, while shifting hot foreground symbols
and growing `foregroundPilotPlay` by `156` bytes. Keep `206..230`; close
middle-cluster hand-table alignment. BUILDING2 high now needs generated
deadline ownership or pack-side byte/work reduction.

Latest rejected BUILDING2 high micro D4 sector-boundary probe: v468 encoded
frame `91` as a previous-frame D4 delta against frame `90`, shrinking
`8867 -> 8766` bytes and moving the payload end from sector `150` to `149`.
It cut visible blocking `54 -> 53`, but the decode/code-shape cost regressed
scene `1602 -> 1631`, active loop/target `1351/1311 -> 1380/1310`, and
overrun `40 -> 70`; hidden refill, reads, and due misses stayed `18`, `58`,
and `7`. Close small BUILDING2 high D4 sector-boundary deltas; remaining
adjacent-frame savings are too small for hot runtime decode.

Latest rejected BUILDING2 high duplicate-alias micro probe: v507 rewrote only
high frame `71`'s entry table to reuse byte-identical frame `70` payload
sectors `72..75` instead of its duplicate sectors `75..78`, preserving source,
file size, pack LBA, and the PS-EXE bucket. It stayed exact-flat at scene
`1602`, active loop/target `1351/1311`, overrun `40`, blocking/refill `54/18`,
loop reads `58`, loop-read VBlanks `257`, and due misses `7`. Close this
adjacent alias shape; duplicate-payload wins need a later due-path payload with
proven forward/current-window residency, not an already-covered adjacent entry.

Latest rejected BUILDING2 high narrow early group: v478 added the current
read-plan-compatible `23..29` retained-read row ahead of the accepted high
groups. The focused pass was exact-flat at scene `1602`, active loop/target
`1351/1311`, overrun `40`, blocking `54`, hidden refill `18`, loop reads
`58`, due misses `7`, and `group_hits=0`, while growing/shifting hot
foreground symbols. Close narrow early scalar groups in this cluster; the
next BUILDING2 high work needs generated ownership or pack-side work reduction.

Latest rejected BUILDING2 high accepted-row extension: v498 extended the
promoted `{60,72}` row to `{60,76}` after the current read plan ranked it as
a balanced low-visible-risk 16-sector candidate. The extra sectors saved one
loop read (`58 -> 57`) but regressed visible timing and refill: scene/loop
`1602/1351 -> 1608/1357`, target `1311 -> 1309`, overrun `40 -> 48`,
blocking `54 -> 60`, and hidden refill `18 -> 26`. Keep `{60,72}` as the
early boundary; further BUILDING2 high progress needs generated deadline
ownership or pack-side work reduction rather than wider hand-authored rows.

Latest rejected BUILDING2 high late low-risk row: v499 appended the current
read-plan's largest unclosed low-visible-risk candidate, `{312,336}`, after the
accepted high rows. The focused gate completed but stayed exact-flat at scene
`1602`, active loop/target `1351/1311`, overrun `40`, blocking `54`, hidden
refill `18`, reads/due `58/7`, loop-read VBlanks `257`, and `group_hits=0`.
Close late scalar row `312..336`; the modeled tail savings are not exposed by
the current append state. Remaining BUILDING2 high work needs generated
deadline/append-start ownership or pack-side byte/work reduction, not another
plain retained table.

Latest rejected BUILDING2 high preserve-offset draw-gap merge: v635 used the
same known-foreground draw-gap merger tested on W1, but on `BUILDING2.FG2`.
The conservative gap1 transform changed `231` frames, saved `54093` payload
bytes, merged `29058` gaps, and cut spans `104351 -> 75293`, all with fixed
pack size/LBA and no source code. Timing regressed anyway: scene `1602 ->
1607`, active loop/target `1351/1311 -> 1356/1312`, overrun `40 -> 44`,
blocking `54 -> 57`, and refill `18 -> 22`. Close preserve-offset draw-gap
merging for BUILDING2 high; future pack work must reduce bytes/spans without
adding draw pixels or must move sector boundaries without changing decode
work.

Latest rejected BUILDING2 high offscreen draw-span clipping: v655 tested the
BUILDING4-style zero-runtime clip on `BUILDING2.FG2` after a host scan found
`132298` offscreen pixels, `27787` spans, `1769` draw rows, and `134358`
logical draw bytes outside the viewport. The broad clip and the hot
`72..92` split both preserved pack size/LBA and the `217088` byte PS-EXE
bucket, but both reproduced the same phase regression: scene `1602 -> 1605`,
active loop/target `1351/1311 -> 1354/1309`, overrun `40 -> 45`, blocking
`54 -> 57`, and refill `18 -> 21`; reads/due stayed `58/7`. Close B2-high
offscreen clipping as a broad speed lane. The later v664/v698/v700/v701/v702/v703 subsets are safe
only as same-speed work-volume cleanup, while v699 closes the untried early
`67,69..71` subset after it reproduced the same phase regression. Remaining
B2-high speed work needs generated deadline ownership, sector-boundary byte
removal, or precomposed upload/restore work that does not shorten visual work
into a worse CD phase.

Latest rejected BUILDING2 high safe-subset physical compaction: v709 repacked
the accepted offscreen-clip subsets `89..104` and `168..177`, preserving file
size/LBA/PS-EXE bucket while reducing active payload `674798 -> 651508`
(`23290` bytes). The focused gate regressed scene/loop `1602/1351 ->
1609/1358`, target `1311 -> 1309`, overrun `40 -> 49`, blocking `54 -> 60`,
refill `18 -> 26`, and loop reads `58 -> 59`; due stayed `7`. Close combined
same-order physical compaction for B2-high safe subsets. Any retry must split
post-hot `89..104` from late `168..177` or be scheduler-owned.

Latest rejected BUILDING2 high post-hot physical compaction: v713 split v709
and repacked only `89..104`, trimming `19616` active bytes. Scene/loop/blocking
stayed flat at `1602/1351/54`, but target fell `1311 -> 1310`, overrun
regressed `40 -> 41`, refill worsened `18 -> 24`, and reads worsened `58 ->
59`. Close post-hot physical compaction; only the late-only `168..177` split
remains unresolved, and it is same-speed work-reduction at best.

Latest rejected BUILDING2 high late physical compaction: v714 split v709 the
other way and repacked only `168..177`, trimming `3674` active bytes. It
regressed scene/loop `1602/1351 -> 1617/1366`, target `1311 -> 1305`, overrun
`40 -> 61`, blocking `54 -> 67`, and refill `18 -> 30` while reads/due stayed
`58/7`. This closes same-order physical compaction of B2-high accepted
offscreen-clip subsets; remaining B2-high speed needs scheduler-owned metadata,
no-shift encoding, or a different upload/restore representation.

Latest rejected BUILDING2 high whole-pack draw-tail trim: v829 applied the
same `compact-fgp3-trim-draw-tails.py` transform that promoted BUILDING2 low in
v739, preserving file size/LBA/source while reducing active payload `674798 ->
651506` across `26` entries. High tide regressed anyway: scene/loop/target
`1602/1351/1311 -> 1609/1358/1309`, overrun `40 -> 49`, blocking `54 -> 60`,
refill `18 -> 26`, and loop reads `58 -> 59`. Close B2-high full tail trimming
under the v703 baseline; the removed high-tide bytes are cadence padding, not
free payload. The later v877 single-entry preserve-offset/fixed-sector trim is
accepted only because it preserves the established read/refill phase and stays
exact-flat; v879/v880 extend that same-speed payload baseline, but v883 closes
entry173/source235 after the supposedly fixed-sector trim exited `137` twice
before `JCPERF2` correctness.

Latest rejected BUILDING2 high early-wide row: v500 inserted `{53,77}` before
the accepted high rows to test whether starting before the failed `60..76`
extension could use an earlier slack gap and save two modeled reads. The gate
was exact-flat at scene `1602`, active loop/target `1351/1311`, overrun `40`,
blocking `54`, hidden refill `18`, reads/due `58/7`, and loop-read VBlanks
`257`. Close `53..77` as another scalar row; the early cluster is not missing
just a wider or earlier table entry.

Latest rejected BUILDING2 high tail-adjacent row: v503 appended `{298,322}`
after the accepted high rows to test the current read plan's medium-risk
two-read tail-adjacent candidate. The focused gate stayed exact-flat at scene
`1602`, active loop/target `1351/1311`, overrun `40`, blocking `54`, hidden
refill `18`, reads/due `58/7`, while hot foreground symbols shifted and
`foregroundPilotPlay` grew by `164` bytes. Close `298..322` as another scalar
retained row; BUILDING2 high now needs generated deadline/append-start
ownership or pack-side upload/cleanup reduction.

Latest rejected BUILDING2 high setup-owned middle cluster: v508 added a
high-tide-only reusable setup segment for sectors `202..226`, the same middle
cluster that read-plan ranking keeps surfacing after direct tables closed. It
did lower loop reads `58 -> 55`, but regressed scene `1602 -> 1619`, active
loop/target `1351/1311 -> 1358/1308`, overrun `40 -> 50`, blocking
`54 -> 61`, and hidden refill `18 -> 25`, with hot foreground symbols shifted
by `+172` bytes. Close local setup-owned `202..226`; the cluster needs
code-neutral generated ownership or pack-side visual-work reduction, not a new
source setup segment.

Latest rejected BUILDING2 high D4 retry: v514 applied the accepted low-tide
frame `71`/`77` previous-frame D4 transforms to high tide. The byte win was
real (`6656 -> 13` and `8828 -> 909`) and saved one loop read (`58 -> 57`),
but high regressed strict counters: scene stayed `1602`, active loop stayed
`1351`, target `1311 -> 1310`, overrun `40 -> 41`, blocking `54 -> 56`,
hidden refill `18 -> 19`, and due misses stayed `7`. Keep the low-tide D4
promotion only; do not retry high frame71/frame77 D4 without a code-neutral
decode predicate or generated deadline ownership.

Latest rejected BUILDING2 high full early-cluster group: v485 inserted the
read-plan top-ranked `3..27` retained-read row before the accepted high groups.
The probe could not produce a valid gate: the normal focused run and
case-local-CD retry were both host-killed with exit `137` before `JCPERF2`, and
the intermediate skip-build retry only reached the sound preload before the
emulator was orphaned. No runtime metrics were accepted and the source table was
restored. Do not promote `3..27` as a scalar row from these artifacts; retry
early-cluster ownership only after the harness is stable or through generated
metadata/pack-side work reduction.

Latest rejected VISITOR3 high current-layout frame129 D4 retest: v469 reran
the large `17025 -> 609` byte frame `129` delta after v464 gave frame `132`
setup-segment ownership. The run emitted complete metrics but failed strict
baseline matching because the accepted v464 artifact uses an older case label;
the counters were exact-flat at scene `1402`, active loop/target `1067/1039`,
overrun `28`, blocking `45`, hidden refill `0`, reads/due `8/8`. Close
standalone VISITOR3 high frame `129`; it remains inert on the current layout.

Latest rejected VISITOR3 low grouped-append ownership: v421-v423 tested the
remaining tight `206..230` low cluster without adding setup residency. A
low-only 24-sector grouped append with `minSlack=4`, the same group unguarded,
and the inferred append-edge `218..230` all stayed exact-flat at scene `1401`,
active loop/target `1072/1040`, overrun `32`, blocking `58`, hidden refill
`0`, and reads/due `10/10`. Close this local table path; the cluster needs
existing-resident-byte compression/aliasing, deeper generated deadline
metadata, or a custom pack representation.

Latest rejected BUILDING2 low scheduler ownership probe: v388 reran a fresh
low baseline at `1619`, `1349/1318`, overrun `31`, blocking `81`, hidden
refill `1`, reads `55`, and due `18`, then temporarily gave low-tide
BUILDING2 the same staged-prepare-before-window priority that is accepted for
WALKSTUF1 low. The result was exact-flat on every key metric, so the row is
not blocked by that local priority inversion. Close BUILDING2 low
prepare-before-window; future low work needs generated refill ownership,
pack-side byte/cleanup reduction, or selective preprocessing.

Latest rejected runtime-shape probe: v389 cached hot scene-policy bits at
runtime start and replaced repeated scene-name checks in the scheduler/window
helpers. BUILDING2 low, VISITOR3 low, and WALKSTUF1 low were exact-flat;
VISITOR3 high hit missing `JCPERF2` under the shifted layout; and the source
did not create code-size or work-volume savings. Close manual cached policy
flags as a speed lane. Keep policy bits only if they are generated with a
larger scheduler-ownership table.

Latest rejected no-holiday hot-path probe: v390 guarded the per-frame
`fgBackdropStampHoliday()` call in foreground composition because all current
perf boots use `holiday 0`. BUILDING2 low was exact-flat and shifted hot
symbols by `+40` bytes; the WALKSTUF1 low sample was killed before `JCPERF2`.
Close the standalone holiday-stamp guard. It is too small to move timing and
should only return as part of a larger backdrop/holiday cleanup with stable
layout padding.

Latest rejected VISITOR5 low scalar group: v372 added the low-tide `9..25`
read group after a v371 rerun confirmed the public row is current. It stayed
exact-flat (`1371`, `1112/1090`, overrun `22`, blocking/refill `12`, reads
`25`, due `0`) while adding code shape. Do not continue VISITOR5 low hand
tables unless generated ownership changes the scheduling model.

Latest rejected VISITOR5 high scalar group and baseline refresh: v401 reran the
accepted current-layout high row at `1107/1090`, overrun `17`,
blocking/refill `16`, reads `19`, and due `0`, then tested the read plan's
top `99..111` group. The group saved one read but regressed timing to
`1112/1091`, overrun `21`, blocking/refill `18`. The public CSV now stamps
VISITOR5 high as `visitor5-high-current-v401`; rollup is `+0.3178%` over
target / `99.6884%` target speed at that point. Current rollup after the v458
WALKSTUF1 high current-control refresh, v464 VISITOR3 high setup-segment
promotion, v474 WALKSTUF1 low boundary read group, and v477 VISITOR3 low
frame132 setup-prime relocation is `+0.2954%` / `99.7097%`. Close direct
VISITOR5 high hand tables
unless a generated scheduler or pack-side data-shape pass changes ownership.

Follow-up v490 tested the read-plan's lower visible-risk high-tide row
`30..46` after correcting the source gate so the high branch actually ran.
The focused harness reached the VISITOR5 FG stream but was killed with exit
`137` before `JCPERF2`; the orphaned DuckStation process continued issuing
small VISITOR5 reads for another minute and still emitted no scene-complete
metrics. Restore the source table and keep direct high-tide scalar groups
closed unless a clean rerun proves the row with no external relauncher noise.

Latest promoted VISITOR5 high baseline: v496 reran that same lower-risk
`30..46` high-tide row after stopping the external long-run relauncher that
had been corrupting focused perf runs. The focused gate passed at
`scratch/ps1-perf-iterate/visitor5-high-rg30-46-v496-focused/20260513-072408-2952191/summary.json`,
and broad canaries passed at
`scratch/ps1-perf-iterate/visitor5-high-rg30-46-v496-canaries/20260513-072519-2959484/summary.json`.
VISITOR5 high improves scene `1365 -> 1359`, active loop/target
`1107/1090 -> 1101/1096`, overrun `17 -> 5`, blocking/refill `16 -> 5`,
loop reads `19 -> 18`, and loop-read VBlanks `99 -> 84`; due misses remain
`0`, pack LBA/sectors stay `24221/173`, and the PS-EXE bucket stays `217088`.
VISITOR5 low, WALKSTUF1 high/low, BUILDING2 high/low, VISITOR3 high/low, and
FISHING1 high canaries stayed on accepted profiles. This moves VISITOR5 high
to `99.55%` target speed and into green, adding about `0.0088` public
over-target points removed and `0.0086` public target-speed points gained.

Latest rejected WALKSTUF1 low direct-stage ownership lane: v402 tested both
sides of the late `297..321` exact-read/window split. Denying the direct-stage
shortcut for that sector band saved reads (`67 -> 65`) but regressed target
`1431 -> 1429`, blocking `72 -> 76`, and hidden refill `22 -> 27`. Forcing
direct-stage ownership for the same band kept reads at `67` and scene/loop
flat, but still regressed target `1431 -> 1429`, blocking `72 -> 73`, and
hidden refill `22 -> 24`. Close hand-coded direct-stage/window toggles for this
cluster; the next WALKSTUF1 low attempt needs pack-side sector-boundary changes,
a generated reservation map with exact phase ownership, or a custom data shape
rather than another scene-specific C branch.

Latest rejected WALKSTUF1 low sector-aligned hot-cluster pack: v403 rebuilt the
low pack in the same `1535263` byte footprint while sector-aligning entries
`145..159` and shifting the sound table into zero-tail slack. This did reduce
one read (`67 -> 66`), but it regressed scene/loop `1776/1484 -> 1780/1488`,
target `1431 -> 1427`, overrun `53 -> 61`, blocking `72 -> 102`, and due
misses `12 -> 18`. Close brute sector-align padding for the late yacht cluster;
the row needs a planner that removes bytes or changes ownership without pushing
the CD head farther forward.

Latest rejected WALKSTUF1 low mid-cluster direct-stage deny: v404 tried the
smaller `285..297` medium-risk ownership split after the late `297..321` branch
closed. Forcing that band through full window refill saved reads (`67 -> 65`)
but regressed scene/loop `1776/1484 -> 1784/1492`, target `1431 -> 1430`,
overrun `53 -> 62`, blocking `72 -> 81`, and hidden refill `22 -> 28`. Close
manual direct-stage-deny branches for WALKSTUF1 low clusters; they reduce
transactions but move the cost into visible cadence.

Latest rejected WALKSTUF1 low boundary-frame prefix-gap pack: v417 tested the
narrowest remaining prefix-gap data-shape swing by applying the accepted
`gap6` cleanup merge only to frames `38` and `39`, moving the sound table
`920531 -> 920087` while preserving the `1535263` byte pack footprint and pack
LBA. The focused gate still regressed low from `1776`, `1484/1431`, overrun
`53`, blocking `72`, hidden refill `22`, reads/due `67/12` to `1780`,
`1488/1429`, overrun `59`, blocking `78`, hidden refill `29`, reads/due
`67/11`. Close boundary-frame cleanup-gap widening; WALKSTUF1 low now needs
generated deadline ownership, phase-fixed byte removal, or a custom
representation that reduces render/upload work without moving CD cadence.

Latest rejected WALKSTUF1 low phase-fixed cleanup compaction: v442 kept every
payload offset and the sound-event table fixed, then shrank cleanup payload
sizes in place. The broad `frames 38..215` pass cut `5292` encoded bytes and
collapsed cleanup spans `39188 -> 36798`, but regressed pressure despite fixed
CD phase: scene/loop stayed `1776/1484`, while target fell `1431 -> 1430`,
overrun rose `53 -> 54`, blocking rose `72 -> 74`, and hidden refill rose
`22 -> 25`. The frame-210-only binary variant was exact-flat at `1776`,
`1484/1431`, blocking `72`, hidden refill `22`, reads/due `66/12`. Close
preserve-offset cleanup-gap compaction for WALKSTUF1 low; the safe form is
inert and the broad form adds cleanup pressure. The next low-side swing needs
generated deadline/refill ownership, sector-read-changing byte removal, or a
custom render/upload representation that does not add cleanup pixels.

Latest rejected WALKSTUF1 low prepare-threshold sweep: v446/v447 tried moving
the accepted staged-prepare priority from the normal `4` VBlank slack point to
the two adjacent binary choices, `5` and `3`, without changing packs or layout.
Both directions regressed the row to scene `1782`, active loop `1490`, blocking
`84`, and larger hidden refill (`24` / `31`) while moving no key metric in the
right direction. Close scalar prepare-threshold tuning; WALKSTUF1 low still
needs generated deadline ownership or pack/data-shape work, not a different
single slack constant.

Latest rejected runtime policy-cache probe: v625 retested cached scene-policy
flags plus cached window minimum slack on the current v598 WALKSTUF1 low
baseline. The PS-EXE bucket stayed `217088`, but the row stayed flat at scene
`1770`, active loop/target `1478/1431`, overrun `47`, blocking `64`, reads/due
`62/11`, while hidden refill/prefetch overrun regressed `20 -> 21` and
`foregroundPilotPlay` grew by `80` bytes. This supersedes the earlier v448/v473
policy-cache misses: close standalone runtime policy caching unless it is part
of generated scheduler metadata with a hot-symbol budget.

Latest rejected VISITOR3 low window-slack sweep: v449/v450 lowered the
low-tide dual-segment window-refill guard from the accepted `4` VBlanks to `3`
and then `2`. Both variants stayed exact-flat at scene `1401`, active
loop/target `1072/1040`, overrun `32`, blocking `58`, hidden refill `0`,
loop reads `10`, and due misses `10`, while shifting hot symbols and growing
`foregroundPilotPlay` by `8` bytes. Close scalar VISITOR3 low window-slack
guard tuning; v452 proved the remaining low cluster responds to compression
inside existing resident bytes, not scalar slack-threshold changes.

Latest rejected WALKSTUF1 low runtime narrow upload: v418 tested a
scene-gated dirty X-band uploader that copied narrow row bands into a 64 KiB
scratch buffer before `LoadImage`, hoping to turn the large host-side upload
byte signal into held-frame slack. It was rejected structurally: the source
grew the PS-EXE bucket `217088 -> 219136`, and even with layout changes
allowed the run timed out before `JCPERF2` after progressing much more slowly
than the accepted baseline. Close scratch-copy exact uploads; if upload-byte
reduction returns, it needs pack-authored/upload-ready rectangles or a
code-size-neutral DMA path that does not copy during playback.

Latest rejected JOHNNY1 scalar group: v373 added shared `131..147` after both
tides ranked it as their top low-risk grouped-window candidate. High stayed
exact-flat at `2069`, `1973/1945`, overrun `28`, blocking/refill `25`, reads
`7`, and due `0`, while code grew. Skip low and close direct JOHNNY1 grouped
rows until generated ownership or pack shape changes.

Post-v373 candidate deck after scalar read-table exhaustion:

| # | Lane | Idea | Why it is different from the misses |
| --- | --- | --- | --- |
| 1 | Generated scheduler | Emit per-entry direct-stage-deny bits from the read-plan instead of scene/sector C branches. | Avoids hot-path code growth and can test whole candidate sets without address drift. |
| 2 | Generated scheduler | Add per-read ownership class: setup-owned, hidden-prefetch-owned, visible-due-owned, or never-group. | Prevents candidates that save reads but steal visible cadence from being emitted. |
| 3 | Generated scheduler | Generate a compact per-scene append table sorted by actual observed append starts, not sector ranges guessed by hand. | Current hand tables often do not fire even when the planner says fireable. |
| 4 | Generated scheduler | Add a direct-stage grouped-window mode that is only enabled by generated metadata and only when first-gap slack exceeds a threshold. | Tests the early-cluster problem without permanent scene-specific C checks. |
| 5 | Generated scheduler | Emit min/max slack bands per group from the trace, not one scalar `minSlackVBlanks`. | Many misses need a middle slack window: enough to group, not enough to starve present. |
| 6 | Generated scheduler | Add a no-code runtime table blob in the pack metadata tail for read groups. | Lets pack transforms change scheduler behavior without growing `foregroundPilotPlay`. |
| 7 | Pack shape | Try forward-order duplicate repacking with physical payload copies sorted by playback, then table offsets updated forward-only. | BUILDING2 aliasing failed because backward references caused seek churn. |
| 8 | Pack shape | For WALKSTUF1, search for resident-slot swaps that keep evicted frames before the old sector, never into padded tail. | v342 proved tail eviction is worse than the removed late read. |
| 9 | Pack shape | Narrowed by v375: true in-prime payload clones require a prior shrink; zero-tail clones plus a small setup segment are rejected. | WALK1LOW sectors `2..78` have only `296` unused bytes, and cloning entries `148..149` to tail sector `450` regressed low hard. |
| 10 | Pack shape | Add a frame-local dictionary for repeated cleanup row headers in WALKSTUF1 v4 packs. | Existing exact duplicate payload scan found no full-frame duplicates; smaller metadata repetition may still exist. |
| 11 | Pack shape | Closed by v374: generic draw-tail trim with copy-through unparsable entries. | `--copy-unparseable` let the scan copy WALKSTUF1 entries `54` and `137`, but all parseable entries trimmed `0` bytes in both tides; no PS1 gate was warranted. |
| 12 | Pack shape | Test PAL4 draw-row delta coding for WALKSTUF1 only, preserving v4 decode path behind a scene flag. | Targets byte pressure without changing cleanup geometry. |
| 13 | Pack shape | Generate "no-op alias with local forward copy" entries for VISITOR3/WALKSTUF only when source frame remains ahead of current CD head. | Keeps cadence entries while avoiding reverse seeks. |
| 14 | Pack shape | Repack BUILDING2 high/low by loop order while padding to original file size and keeping pack LBA fixed. | The current logical order is not necessarily CD-optimal after compaction. |
| 15 | Render data | Emit precomposed upload bands only for frames with background ownership proven by scene/tide/night state. | Raw upload-ready failed because pixels depended on cleanup/background state. |
| 16 | Render data | Add a sidecar ownership mask for VISITOR3 and WALKSTUF cleanup/background/current pixels. | Converts unsafe upload-ready modeling into a provable pixel source. |
| 17 | Render data | Compress upload-ready bands with row-RLE plus rect table, budgeted to zero-tail slack. | The budgeted VISITOR3 plan had bytes but no safe pixel source; compression may make safer subsets fit. |
| 18 | Render data | Precompose only fully opaque sprite interior rows, leaving edges on current residual compositor. | Reduces upload/compose work without needing cleanup/background ownership at boundaries. |
| 19 | Render data | Add per-frame dirty-rect coalescing metadata generated offline for JOHNNY1 and VISITOR5. | Their CD reads are already low; render/upload work may be the remaining fixed overhead. |
| 20 | Runtime shape | Split `fgRuntimeTryStageNextFrame()` into cold scene-policy setup and a smaller hot path. | Many exact-flat probes only shift hot code; shrinking the hot path may expose real wins. |
| 21 | Runtime shape | Closed by v886 as a broad hot-path source rewrite: caching scene-name policy checks grew `foregroundPilotPlay` and regressed W1-low cadence. | Reopen only as cold setup metadata or no-code pack policy that proves no hot-symbol growth before timing. |
| 22 | Runtime shape | Replace table/count setup branches with a generated scene policy enum. | Keeps new generated ownership from growing the branch cascade. |
| 23 | Validation | Done in v407: `--require-improvement` now fails if the baseline file has no matching case label. | Prevents false PASS like the v368/v385 label-mismatch cases while preserving exploratory warning-only comparisons. |
| 24 | Validation | Add group-hit/read-delta expectations for read-group probes. | Exact-flat groups should fail immediately as "did not fire" instead of consuming full analysis. |
| 25 | Validation | Add a stale-row verifier that reruns only matrix rows whose stats version does not match the current source/pack promotion. | Avoids optimizing against stale battle-card entries. |
| 26 | Validation | Record direct-stage vs window-stage read counts in the candidate matrix. | Distinguishes candidates that need generated direct-stage ownership from append-table rows. |
| 27 | Validation | Add a hot-symbol drift budget to exploratory source probes by default. | Several misses only shifted addresses; catching that earlier reduces noise. |
| 28 | Validation | Generate "next best non-scalar lane" recommendations after three same-family misses. | Keeps the headless optimizer from repeatedly trying inert table variants. |

Post-v570 generated/data-shape queue after scalar read-table exhaustion:
the refreshed read-candidate matrix now has `0` standalone, `0` guarded, `0`
scheduler-owned-only rows, and `52` closed exact ranges. The remaining under-99
scenes are close enough that the next wins should come from generated
ownership, pack-side byte shape, and code-neutral scene-specific compression
rather than more hand-coded retained-read tables.

Latest rejected runtime-shape probe: v886 cached scene family/policy bits at
runtime start and replaced hot `gFgRuntime.sceneName` comparisons with byte
checks. The focused W1-low gate regressed from `1769/1477/1432`, overrun `45`,
blocking/refill `65/20`, reads/due `58/11` to `1776/1484/1429`, overrun `55`,
blocking/refill `77/25`, reads/due `59/12`, while `foregroundPilotPlay` grew
by `252` bytes. Close broad cached-scene-kind rewrites for now; generated
ownership needs no-code pack metadata, cold setup-only policy tables, or a
strict hot-symbol budget.

| # | Target | Idea | First gate |
| --- | --- | --- | --- |
| 1 | BUILDING2 low | Closed by v626 as a narrowed scalar row: `218..229` with slack8 keeps hidden refill at zero and promotes the useful near-miss win. Do not retry the wider `218..230` hand row unless the pack/source shape changes again. | New baseline is `1614/1344`, target `1318`, overrun `26`, blocking/refill `61/0`, reads/due `50/14`. |
| 2 | BUILDING2 low | Closed in v622 as a simple split: `218..224 + 224..230` kept refill at zero but regressed scene/loop/overrun/blocking and saved only one read. Do not retry as scalar subgroups. | Next retry of this cluster must be pack-side byte reduction, upload/restore work reduction, or generated deadline ownership that preserves the raw `218..230` visible-blocking win without the refill VBlank. |
| 3 | BUILDING2 low | v660 added the safe pack-side work-volume follow-up after v626: offscreen draw-span clipping stayed exact-flat at `1344/1318`, overrun `26`, blocking/refill `61/0`, reads/due `50/14`, while removing `120179` offscreen draw pixels, `25136` spans, and `1537` frame rows. v739 now builds on that pack work and moves timing to `1339/1317`, overrun `22`, blocking/refill `53/0`, reads/read time `37/150`, and due `12`. | Keep v739 as the current low speed baseline. The remaining path to green still needs generated ownership around the current `218..229` scheduler shape, upload/restore work reduction, or another sector-boundary byte change that preserves `prefetch_overrun_vb=0`. |
| 3a | BUILDING2 low | Superseded by v739 after a clean baseline retry: the same active-payload trim `660236 -> 538534` now passes focused and canary gates, improving low to `1339/1317`, overrun `22`, blocking/refill `53/0`, reads/read time `37/150`, and due `12`. The old v710 structural failure is retained in the experiment log as a dirty-path/allocator warning, not the current baseline truth. | Keep v739 as the speed-bearing B2-low pack baseline. Future low work should build on this pack and avoid broad scheduler-slot stealing; remaining path to green likely needs generated ownership or upload/restore elimination. |
| 3b | BUILDING2 low | v711 narrows compaction to the largest hot cluster `80..86`, trimming `41614` bytes and reducing blocking `61 -> 59`, but it regresses scene/loop `1614/1344 -> 1617/1349` and overrun `26 -> 31`. | Do not retry this hot cluster as same-order displacement. Any B2-low compaction must preserve loop timing, not just visible blocking. |
| 4 | BUILDING2 low | Narrowed by v645: the downstream `250..262` row is still unsafe after the v626 `218..229` promotion, regressing to `1649/1358`, overrun `41`, blocking `64`, and refill `4` despite reads `50 -> 48`. Generate an append-start table from observed CD log starts, not sector windows, and blacklist starts that previously caused hidden refill. | Add a planner report showing which start fired, then fail fast if `group_hits=0` or refill rises. Do not retry `250..262` as an adjacent scalar follow-up on the v626 baseline. |
| 5 | BUILDING2 high | Closed for current scalar/setup forms: preserve the accepted `206..230` overread and do not retry the separate `185..197` cluster as a hand table or reusable setup segment. v640 showed the eight-VBlank hand-table guard still fired and regressed refill/blocking; v851 moved `185..197` into setup-owned residency and still regressed active loop `1351 -> 1357`, overrun `40 -> 48`, blocking `54 -> 63`, refill `18 -> 26`, and due `7 -> 8` despite reads `58 -> 55`. v838 also closes the late `{319,335}` hand row after it saved one read but regressed loop/blocking. v916 closes early `17..33` as a hand row after unguarded regressed, slack-4 was inert, and slack-2 failed before complete `JCPERF2`. v926 extends the earlier same-speed payload baseline through frame100. The allocator-era broad setup sweep now also closes moving the second segment earlier: `86..242`, `176..242`, and `179..242` failed allocator headroom; feasible `184..242` and `194..242` reduced reads but regressed loop/overrun/blocking/refill. | Only reopen B2-high scalar/setup clusters with generated planner metadata that proves earlier non-visible ownership, fixed hot code, `blocking_vb <= 39`, `refill <= 16`, and no due-miss increase before a PS1 gate. Larger setup residency is closed unless a separate clean-rect/allocator work reduction creates enough headroom and a planner predicts visible-cadence neutrality. |
| 6 | BUILDING2 high | Closed by v704 as safe subsets only: frames `168..177`, `94..104`, and `89..92` offscreen clipping remove `22390` pixels, `5072` spans, and `114` frame rows while staying exact-flat; frame `88` is a host no-op, and broad/hot v655 plus early `67,69..71` v699 clipping regressed phase. | Do not retry broad/hot/early offscreen clipping for speed. Reopen only for generated scheduler-coupled clipping or upload/restore data that proves fewer rows/spans without changing CD/refill cadence. |
| 7 | BUILDING2 high | Generate "accepted-row tail ownership" metadata for `206..230` so shorter rows can be simulated without replacing cadence-critical overread. v829 confirms whole-pack draw-tail trimming removes cadence padding and regresses high tide; v877/v879/v880/v887/v888/v889/v890/v891/v892/v896/v914/v926 prove only selected no-shift fixed-sector single-entry trims are currently safe, and still only as same-speed work reduction. | Planner must report same due-read order as accepted plus fewer visible blocking VBlanks before a PS1 gate. |
| 8 | WALKSTUF1 low | Narrowed by v644 and v828: simply giving the late `285..321` cluster direct CD-window ownership is phase-negative. The latest `{297,309}` probe saved one read but regressed low to `1776/1484`, overrun `54`, blocking/refill `74/26`, with due still `11`. | Next probe must be metadata/planner-first and should not touch `foregroundPilotPlay` unless it proves an earlier non-visible slot; fail if due misses rise above `11` or blocking rises above `64`. |
| 9 | WALKSTUF1 low | Create a resident mini-pack for one late cluster using no-decode aliasing, not frame28 D4 holes. The D4 hole created bytes but added startup/hot decode debt; a true alias must move bytes without extra runtime work. | Host validator must prove zero new D4 gates and fixed setup-prime startup residency before PS1 timing. |
| 10 | WALKSTUF1 low | Closed by v638: exact cleanup-row and draw-metadata dictionaries across frames `146..158` grow the hot cluster before runtime overhead. | Reopen only for semantic/near-duplicate row-template compression or generated ownership; exact row dictionaries are not a sector-saving lane on this baseline. |
| 11 | WALKSTUF1 low | Closed by v630-v632: two-phase `91..98` after accepted `78..91` is inert at slack `6/5/4`, and slack `3` is unreachable under the W1-low guard. | Do not retry as a scalar row unless generated deadline ownership can prove nonzero hits before timing. |
| 12 | WALKSTUF1 low | Pack-order "same-order, no tail displacement" compaction with generated read retargeting. v523 moved bytes earlier but did not retarget deadlines, so the phase shift became negative. | Run planner first; only gate if predicted read starts stay forward and accepted early ramp remains resident. |
| 12a | WALKSTUF1 low | v712 closes physical same-order compaction for the preserve-offset direct-clip-safe island `58..63`: trimming `22639` bytes regressed scene/loop `1770/1478 -> 1784/1492`, blocking `64 -> 97`, refill `20 -> 31`, and due `11 -> 15`. | Treat W1-low non-tail physical compaction as closed across left, mid, and safe-island subsets. Only late-tail v705 remains safe without generated scheduler ownership. |
| 13 | WALKSTUF1 high | Add high-tide generated prepare-before-window ownership for the `298..322` suffix family. Low has a prepare fallback; high scalar rows are exhausted and need the same ownership class without hand branches. | High-only metadata probe, zero source-code growth in `foregroundPilotPlay`, require `blocking_vb < 81`. |
| 14 | WALKSTUF1 high | Split the late suffix into "safe setup tail" and "visible suffix" groups with per-read ownership. Exact-flat `306..322` and negative `298..310` suggest the useful boundary is not the sector range itself. | Planner must show at least one saved read and no visible due takeover before PS1. |
| 15 | WALKSTUF1 high | No-decode payload shrink for high frames around the `298..322` cluster using row-span canonicalization, not previous-frame D4. Prior high D4 sector-boundary deltas saved bytes but did not pay back CPU/cadence. | Host transform must save >=1 sector and leave D4 gate table unchanged. |
| 15a | WALKSTUF1 high | Closed late local-LZ cluster `133/140..144/192` as a standalone sector-collapse path. The batch saved seven modeled sectors but regressed W1-high to `1855/1519/1432`, overrun `87`, blocking/refill `64/16`, reads/due `44/11`; the largest single split, entry140/source248, still regressed to `1817/1481/1439`, overrun `42`, blocking/refill `48/15`, reads/due `42/8`. | Do not retry late W1-high L4 shrinkage without generated deadline/refill ownership or a no-decode fixed-sector format. |
| 16 | VISITOR3 high | Closed by v641 for the scalar retest: the post-v629 read plan still lists `211..235`, but the matching-label gate is exact-flat and only adds code drift. | Future VISITOR3 high work should skip scalar `211..235` and move to custom compression, payload placement inside existing paid segments, or generated ownership with a non-flat work metric. |
| 17 | VISITOR3 high | Deterministic staged-D4 validator for frame133: prove previous-frame ownership, decode budget, and staged-present timing locally before any emulator run. v484 failed because the chained D4 path reached frame132 correctness but not full scene correctness. | Host validator must replay frames `131..134` with exact pixels and report a schedule with no missing SFX/frame trip before PS1. |
| 18 | VISITOR3 high | Custom one-scene compression for frames `133..135` that stores changed sprite spans against a setup-owned base, not chained previous-frame D4. This targets the remaining terminal bytes without the D4 command-count debt. | Encode one frame, run offline pixel validator, then require fixed pack footprint and `foregroundPilotPlay` code-size neutrality. |
| 19 | VISITOR3 high | Move one more terminal payload into the existing sector-203 setup gap family by shrinking headers around frame133, not by broad setup-prime relocation. v488's broad relocation could not produce deterministic metrics. | Host packing proof must show no setup-prime eviction and no new source code before timing. |
| 20 | VISITOR3 high | Generate a CD-deadline sidecar for terminal frames `135..137` that explicitly schedules prefetch during long non-visible gaps rather than relying on append grouping or scalar slack retunes. Hand groups did not fire under current pack state, and the allocator-era min-slack/prepare-first sweep only traded visible blocking for hidden refill or exact-flat code drift. | Require planner-visible saved read, nonzero generated ownership hit count in logs, `prefetch_overrun_vb <= 5`, no hot-code growth in `foregroundPilotPlay`, and no due-miss increase before a PS1 gate. |
| 21 | VISITOR3 low | Closed by v637 and v824: no remaining due frame produced a previous-frame D4 payload small enough for the `10588` paid setup-prime gap, the best same-offset D4 probe regressed timing, and byte-for-byte terminal relocation after frame `129` was either exact-flat or phase-negative. | Reopen only for a different custom compression family, generated deadline ownership, or newly paid residency; do not retry ordinary previous-frame D4 for frames `130`, `131`, `133`, `134`, `135`, `136`, or `138`, and do not retry raw terminal compaction after frame `129`. |
| 22 | VISITOR3 both | Generate a timeline cost model that estimates CD saved, decode cost, upload cost, and hot-code drift before a PS1 run. VISITOR3 has too many byte wins that are phase-negative unless ownership is right. | Reject candidates whose model predicts any hidden refill or PS-EXE bucket movement. |
| 23 | JOHNNY1 high/low | Closed by v932 for the current under-99 target: local-LZ sentinel payloads compress full-frame entries `1` and `50` in both fixed-footprint packs, cutting active payload `316608 -> 112093` and moving both tides to green at `1948/1945`, overrun `3`, blocking/refill `5`, and read time `37`. | Keep v932 as the baseline. Do not retry per-span black GPU clears, clean-span compiler toggles, or same-frame/no-op aliasing for speed on this target. |
| 24 | JOHNNY1 high/low | Residual black-backdrop upload/coalescing work is no longer an under-99 blocker after v932. The prior v915 signal still reduces upload bytes but was not a speed win. | Reopen only as broader headroom/footprint cleanup, not ahead of the remaining under-99 rows. |
| 25 | JOHNNY1 high/low | Exact no-op aliasing and cleanup adjacency scans remain closed: the current pack has no duplicate payload groups, repeated offset runs, or exact-adjacent cleanup spans to remove. | Future JOHNNY1 work should be whole-disc footprint or code-headroom work, not a priority speed lane. |
| 26 | BUILDING4 low | Closed by v652 for the first classifier pass: same-commit offscreen draw-span clipping is a real pack-only win, but it only moves low from `2856/2816` to `2853/2816`. Remaining debt is still mixed CD/upload work, not a plain retained-read row. | Continue from the v652 baseline; require fixed LBA/EXE bucket and a work-counter drop before timing. |
| 27 | BUILDING4 low | Scene-specific static-mask precomposition for the large island/tree/background area, leaving only moving foreground rows dirty. BUILDING4 low remains near target with high restore/upload pressure. | Offline mask validator first; gate only if visual status stays green and upload/restore bytes drop. |
| 28 | BUILDING4 low | Extend v652 into a smaller occlusion/static-mask pass, not broader clipping. The accepted offscreen draw clip proved pack-authored work reduction can pay here, while W1 proved broad clipping can destabilize CD phase. | Host analyzer must prove exact static ownership and preserve entry offsets/file size before PS1. |
| 29 | Cross-scene | Add a hot-symbol drift budget to the perf harness and reject source probes that grow or shift `foregroundPilotPlay` unless a work metric already improved. Many v548-v570 misses were pure code-shape noise. | Implement as a gate option and run it against one known exact-flat stale row to prove it fails early. |
| 30 | Cross-scene | Auto-generate fresh same-commit baselines for any row whose candidate root predates the latest accepted promotion. This prevents stale `saved=1` rows from wasting runs after pack/data-shape changes. | Extend the matrix script to mark stale-root rows as "refresh-required" instead of scheduler-owned. |
| 31 | Cross-scene | Add binary sweep automation for threshold experiments: try both sides, keep only the better, and log both when neither clears strict gates. This matches the headless methodology and avoids manual scalar bias. | Use BUILDING2 low slack8/slack9 as the first automated regression test. |
| 32 | Cross-scene | Scene-end heap arena reset for foreground scratch allocations, preserving background audio. This is not a speed win, but it protects long-run validation once more pack-side sidecars are added. | Add instrumentation first: prove no live foreground pointer crosses major-scene teardown before enabling reset. |

Post-v645 non-scalar idea batch after the latest scalar/deadline misses:
the main lesson is that the remaining yellow rows are phase traps. Rows that
save reads at the deadline usually increase visible blocking, hidden refill,
or target cadence. New work should either remove payload/restore/upload work
before the scheduler sees it or generate explicit earlier ownership with a
host proof before any emulator time.

| # | Target | Idea | First gate |
| --- | --- | --- | --- |
| 33 | WALKSTUF1 low | Build an offline frame-deadline planner that emits the exact earliest safe CD slot for each late-cluster read, then replay it against the CD log without changing C first. | Planner must prove a sector `297..321` read starts before the current prepare slot and does not overlap prepared-frame ownership. |
| 34 | WALKSTUF1 low | Try semantic row-template compression for frames `146..158`, grouping near-duplicate rows by x-shift and short literal edge patches instead of exact row dictionaries. | Host pixel replay must be exact and save at least one sector before a decoder is considered. |
| 35 | WALKSTUF1 low | Generate a no-runtime payload reorder that preserves the accepted early ramp and moves only frames whose new sector is still forward from the previous read head. | Reject if any moved frame creates a backward seek or changes setup-prime coverage. |
| 36 | WALKSTUF1 low | Split late-cluster frames into tiny header/metadata entries plus bulk row data already resident in a setup-owned sidecar. | Host pack proof must show no hot decode branch and no PS-EXE growth. |
| 37 | WALKSTUF1 high | Port the low-tide planner from idea 33 but target the high `295..319` suffix and accepted high read-group split points. | Planner-only proof first; reject if predicted blocking stays above `81`. |
| 38 | WALKSTUF1 high | Try no-decode row-span canonicalization on the `84..108` and `238..262` high clusters, but forbid adding draw pixels. | Host transform must reduce spans/bytes while preserving exact pixels and D4 gates. |
| 39 | BUILDING2 high | Search for duplicate payload groups that can be physically copied forward into playback order without aliases or backward seeks. | Host CD-head replay must show fewer reverse seeks before PS1 timing. |
| 40 | BUILDING2 high | Build a frame-local row coalescer for frames `101..109` that removes headers without drawing extra pixels, unlike gap1. | Gate only if it saves a sector and keeps cleanup/draw pixel counts unchanged. |
| 41 | BUILDING2 high | Generate deadline ownership for `202..226`, but only from a pre-scene metadata blob so `foregroundPilotPlay` does not grow. | Metadata hit count must be nonzero in logs and blocking must stay `<=54`. |
| 42 | BUILDING2 high | Try selective precomposed upload bands for static building/island rows on the hot `101..109` frames. | Offline mask must prove background ownership; PS1 gate requires restore/upload byte drop. |
| 43 | BUILDING2 low | Target the v626 row with pack-side byte reduction immediately before sector `229`, not another adjacent read row. | Host proof must keep refill predicted at zero and avoid shifting the D4 holes. |
| 44 | BUILDING2 low | Generate a refill-budget owner for the old `218..230` one-refill near-miss, reserving one earlier hidden slot explicitly. | Reject if prefetch overrun is nonzero in a dry-run log probe. |
| 45 | VISITOR3 high | Design a one-scene sprite-span delta against a setup-owned terminal base for frames `133..136`, avoiding chained previous-frame D4. | Host pixel replay exact; command count must be below the failed D4 frames. |
| 46 | VISITOR3 high | Pack terminal frames into an existing paid setup segment using micro-header savings, not broad relocation. | Host proof must keep sectors `203` and `277..293` resident without evictions. |
| 47 | VISITOR3 low | Search for two-stage custom compression where setup-prime holds a small dictionary and due frames hold only row references. | Must fit the remaining `10588` byte paid gap and require no per-pixel runtime loop. |
| 48 | VISITOR3 both | Add a host cost model that prices CD saved, decode commands, upload bytes, and hot-code drift before any PS1 run. | Model must reject known misses v637/v641/v644. |
| 49 | JOHNNY1 | Done by v932 for the 99% goal: compress the full-frame payloads instead of chasing black-clear code. | Keep as accepted local-LZ baseline; revisit black-clear only for future headroom cleanup. |
| 50 | JOHNNY1 | Superseded by v932. A pack-side black-clean row mask is no longer needed to get JOHNNY1 green. | Do not spend the next yellow-row cycle here. |
| 51 | JOHNNY1 | Superseded by v932. The data-only split idea is lower priority than W1/B2/VISITOR3/B4 remaining yellows. | Reopen only if future footprint goals require an even smaller JOHNNY1 pack. |
| 52 | BUILDING4 low | Closed by v652 for the first same-commit classifier: offscreen draw-span clipping lowered both draw work and visible CD cost, but BUILDING4 low is still below green. | Next pass should extend the static/occlusion proof, not retry scalar retained reads. |
| 53 | BUILDING4 low | Build a static island/tree/background ownership mask and precompose only rows that never overlap animated sprites. | Visual diff must cover low tide and wave frames before PS1 timing. |
| 54 | Cross-scene | Add a "phase-trap" tag in the candidate CSV when saved reads pair with tight internal gaps and prior scalar misses. | Candidate matrix should stop ranking those rows above pack/data-shape work. |
| 55 | Cross-scene | Add hot-symbol drift and `foregroundPilotPlay` growth gates to focused runs by default. | Known exact-flat source probes should fail before a full broad canary. |
| 56 | Cross-scene | Generate same-commit baselines automatically after each accepted promotion for all remaining yellows. | Avoid optimizing against stale v570 candidate rows after v626/v629-style phase changes. |
| 57 | Cross-scene | Add a scratch arena reset probe at major scene teardown with foreground pointers poisoned in debug builds. | Long-run validation must prove no live foreground allocation crosses scenes. |
| 58 | Cross-scene | Record per-read owner reason in perf logs: due path, staged prefetch, window append, setup segment, or generated owner. | New generated lanes must prove ownership changed before PS1 timing can promote. |

Post-2026-05-19 direct-read closure batch:
the refreshed read-candidate matrix has `0` standalone rows, `0`
scheduler-or-guarded rows, `0` scheduler-owned-only rows, and `52` closed
exact ranges after promoting W1-high `{423..439}` and `{404..416}` as same-speed CD work.
VISITOR3-low static appends `239..263`, `256..268`, and `256..272` all
produced the same phase-negative `+2` loop / `+1` blocking profile.
VISITOR3-high `{40..52}` stayed exact-flat/inert, W1-high `{352..368}`
regressed, and W1-high `{404..416}` promoted as a same-speed CD-work row.
WALKSTUF1-low static appends exhausted the mid, early, and late pockets: only
`167..183` showed useful speed (`1470 -> 1468`, blocking `34 -> 32`), but it
always raised hidden refill `6 -> 9`, even with min-slack guards and a `32 KiB`
window. The next queue therefore must stop treating saved reads as promotable
by itself.

| # | Target | Idea | First gate |
| --- | --- | --- | --- |
| 59 | WALKSTUF1 low | Convert the `167..183` near-miss into generated read ownership with an explicit refill budget, not a raw append row. | Generated hit count must be nonzero; accept only if loop `<=1468`, blocking `<=32`, and refill `<=6`. |
| 60 | WALKSTUF1 low | Closed by `walkstuf1-low-rg167-183-maxslack-current`: bounded max-slack gates `4`, `8`, `12`, `16`, and `32` were exact-flat, while the ungated control reproduced the visible win and the hidden refill debt. | Do not retry scalar min/max slack around this grouped append. Future `167..183` work needs explicit refill-budget ownership or a pack/data-shape change. |
| 61 | WALKSTUF1 low | Try a two-phase split for `167..183`: resident setup for the first touched frame, generated append for the second. | Setup increase must stay inside the current allocator headroom and active refill must not rise. |
| 62 | WALKSTUF1 low | Build a pack-side boundary trim around frames `109..115` that preserves file size but removes the need for the `167..183` append. | Host proof must save at least one read boundary without moving accepted setup sectors `238..350`. |
| 63 | WALKSTUF1 low | Add a generated "never append" blacklist for pockets proven phase-negative: `120..136`, `120..132`, `153..177`, `160..176`, `174..198`, `174..186`, `366..378`. | Candidate tooling should hide these from top recommendations unless pack layout changes. |
| 63a | WALKSTUF1 low | Treat `141..153` as generated-owner/clean-byte-dependent. The `141..153` setup retarget structurally exhausted CACHE, the `141..147` and `147..153` same-footprint swaps improved loop but regressed target/blocking/refill by displacing `179..185`, additive `179..185 + 141..147` hit the clean-rect CACHE cliff, and pairing additive `141..147` with a low-only `40 KiB` clean cap still exhausted CACHE. | Do not retry as a direct swap, additive retained setup slice, or scalar clean-cap pairing until a real clean-byte or pack/data-shape win creates headroom; preserve the accepted `179..185` slice. |
| 64 | WALKSTUF1 low | Search for no-decode payload relocation into the already retained `238..350` setup segment, but only for frames after the accepted early ramp. | Reject any candidate that creates a backward seek or changes setup-prime coverage. |
| 65 | WALKSTUF1 high | Reuse the W1-low generated-owner design for the remaining high `74..86`, `84..108`/`92..108`, `352..368`, and late suffix pockets instead of adding more hot C read rows. | Must keep `foregroundPilotPlay` size and addresses within the hot-symbol drift budget. |
| 66 | WALKSTUF1 high | Done by `walkstuf1-high-prepare-first-current`: selective prepare-before-window ownership for W1-high only, reusing the accepted W1-low ordering shape without applying it to B2/V3. | Promoted despite loop/target both shifting `1471/1440 -> 1472/1441` because overrun/refill stayed flat at `31/13`, blocking improved `56 -> 43`, due misses improved `10 -> 7`, and the five-yellow canary stayed flat outside W1-high. |
| 67 | WALKSTUF1 high | Pack-side no-decode trim around frame92's neighbors now that frame92 D4 is accepted. | Host proof must keep the D4 predicate table unchanged and save at least one sector boundary. |
| 68 | VISITOR3 low | Replace static low append rows with generated per-frame deadlines that preserve accepted setup clusters `150..177`, `206..232`, and `281..305`. Whole-pack frame-order relocation, `18..32 KiB` clean-relief window sweeps, and retained tail slides to `206..241` / `215..241` are now closed on the current baseline. | Dry-run log must show fewer due reads without changing setup residency, growing the clean-relief window, or adding hidden refill. |
| 69 | VISITOR3 low | Build a custom row-reference codec using the paid `206..232` setup segment as a dictionary for frames `134..136`. | Pixel replay must be exact and command count must be below failed D4 variants. |
| 70 | VISITOR3 low | Closed by `visitor3-low-align135-136-current` and `visitor3-low-align134-136-current`: simple no-decode terminal sector alignment changed modeled read shape but did not improve timing, and the wider alignment regressed loop/blocking. | Do not retry byte-for-byte terminal boundary moves around `248..272`; move this pocket to custom row-reference data shape or generated deadline ownership. |
| 71 | VISITOR3 high | Create a generated terminal-frame deadline sidecar for `133..139`; scalar high append rows are closed. | Require high blocking `<45`, refill `<=3`, and due `<=3` before canaries. |
| 72 | VISITOR3 high | Move a terminal high payload into the existing paid setup family via micro-header savings rather than broad relocation. | Host proof must keep all current resident setup sectors and pack LBA fixed. |
| 73 | VISITOR3 both | Add a motion/residual hybrid for terminal frames with static hull/background ownership masks. | Offline replay must validate cleanup/background ownership across high/low, night, and raft state. |
| 74 | BUILDING2 high | Generate a scheduler sidecar for the accepted B2-high read groups so future changes do not grow the branch cascade. | Sidecar build must be exact-flat to current B2-high before adding any new groups. |
| 75 | BUILDING2 high | Shift from read rows to upload/restore work: identify static building rows that can be precomposed or skipped. | First gate requires lower upload or restore bytes with loop/block/refill exact-flat. |
| 76 | BUILDING2 high | Try no-decode row-span canonicalization on hot frames `85..100`, but forbid added draw pixels and broad same-value gap filling. | Host transform must reduce payload sectors and preserve draw pixel count. |
| 77 | BUILDING2 high | Revisit frame-local compression only when it removes a whole read boundary without adding runtime D4/local-LZ decode. | Reject same-offset byte wins unless the CD-head model predicts a read drop. |
| 78 | Cross-scene | Teach `performance-under-green-attribution.csv` to mark experiment-log-closed ranges and phase traps. | The CSV must stop presenting closed direct rows as the top under-green work queue. |
| 79 | Cross-scene | Add candidate freshness metadata: root artifact commit, current source commit, and current pack hash. | Any stale-root row should be `refresh-required`, not `scheduler-owned`. |
| 80 | Cross-scene | Add a generated-owner hit counter to `JCPERF2` separate from ordinary window hits. | A generated scheduler probe must fail if owner hits are zero. |
| 81 | Cross-scene | Add a hard hot-symbol drift gate for promotion-style source probes. | Known exact-flat source rows should fail before canary if hot symbols shift without work savings. |
| 82 | Cross-scene | Add an automatic "same failure signature" closer for dominated exact subsets once a superset and one subset fail identically. | The tool should explain the dominance relation in the experiment log before closing. |
| 83 | Cross-scene | Generate a refill-debt report from failed runs that ranks candidates by useful loop gain minus hidden refill cost. | `167..183` should surface as a generated-ownership lead, not a scalar reject. |
| 84 | Cross-scene | Add binary-sweep manifests for window/slack/code-threshold probes so both sides are tested and logged together. | Produces one closure row per family instead of repeated manual commits. |
| 85 | Cross-scene | Add an allocator-headroom column to every generated sidecar idea. | Reject sidecars that cannot fit without reducing current CACHE/TRANSIENT safety margins. |
| 86 | Cross-scene | Build a "no hot C" metadata path for per-scene read groups in pack tails. | First proof is exact-flat current W1-low and B2-high with metadata replacing existing C rows. |
| 87 | Cross-scene | Add a visual/static ownership mask generator for precomposed upload rows. | Before timing, every row must prove pixel source: cleanup background, previous frame, or sprite payload. |
| 88 | Cross-scene | Add a long-run allocator validation canary after any metadata sidecar or pack-format change. | Must preserve 0 BSODs and prove scene-boundary transient wipes do not discard live sidecar data. |

Latest rejected W1-low generated-owner shape probe:
`walkstuf1-low-rg167-183-maxslack-current` added max-slack gating to grouped
read appends and swept `4`, `8`, `12`, `16`, and `32` against the current
five-yellow baseline. Every bounded gate skipped the candidate and stayed
exact-flat at `1812/1470/1446`, overrun `24`, blocking/refill `34/6`,
reads/due `30/4`. The ungated control still improved visible loop/blocking to
`1810/1468/1445` and `32`, but it also reproduced hidden refill `6 -> 9`.
Close scalar max-slack gating for `167..183`; the useful fire is tied to the
same high-slack slot that causes refill debt.

Latest promoted VISITOR3 motion-copy payload baseline: keep the v181
scene-specific FGP3 marker payload for yacht translation frames `119..123`,
then add the v182 high-tide frame `115` state-hull motion-copy payload, then
add the v188 sparse-in-place frame `124` state-hull motion-copy payload and
the v189 sparse-in-place frame `118` hull motion-copy payload to both tides,
then add the v193 high-only sparse-in-place frame `117` target-hull payload,
the v202 high-only frame `127` re-anchor payload, and the v204 low-tide
persistent setup segment for sectors `281..305`, followed by the v205 high-tide
persistent setup segment for sectors `277..293` and the v206 high-only frame
`126` re-anchor payload, then the v207 high-only frame `125` re-anchor payload,
the v213 high-only setup-prime expansion from `232 KiB` to `288 KiB`, the
v214 high-only setup-prime expansion from `288 KiB` to `320 KiB`, the
v216 guarded low second setup segment for sectors `150..174`, the v227
low frame-125/frame-126 resident re-anchor, the v234/v237 frame-118/frame-127
resident copies inside that accepted segment, and the v238 high
frame-127/frame-130 resident-copy compaction inside the existing high
setup-prime window, followed by the v248 low frame-114/frame-117 no-op residual
compaction, the v249 low frame-113 no-op residual, the v291 high
frame-140/tail setup-segment copy, the v299 high frame-131 resident-alias
setup-prime copy, and the v302 low frame-128 resident segment copy.
The v214 high baseline moves VISITOR3 high to `1089/1035`, cuts overrun
`67 -> 54`, blocking `96 -> 83`, loop reads `17 -> 15`,
loop-read time `96 -> 83`, and due misses `17 -> 15`; cumulative setup-prime
residency raises total high `scene_vb 1414 -> 1420`.
The v238 high compaction then moves high to `1075/1037`, cuts overrun
`54 -> 38`, blocking `83 -> 59`, loop reads `15 -> 11`,
loop-read time `83 -> 59`, and due misses `15 -> 11`; the v291 high
setup-segment copy moves high again to `1074/1038`, cuts overrun to `36`,
blocking to `58`, loop reads to `10`, loop-read time to `58`, and due misses
to `10`; the v299 resident-alias setup-prime copy aliases duplicate frames
`121`/`123` to frame `120`, fits frame `131` fully in setup-prime coverage,
and moves high again to `1070/1039`, cuts overrun to `31`, blocking to `49`,
loop reads to `9`, loop-read time to `49`, and due misses to `9`; high total
is now `scene_vb=1402`.
Both VISITOR3 tides preserve the original `1555450` byte CD footprint, fixed
later offsets, current LBAs `22473/23233`, and the `217088` byte PS-EXE bucket.
The v216 second setup segment moves low `1102/1032 -> 1098/1034`; the v227
resident re-anchor moves low again to `1095/1035`, the v234 resident copy moves
low to `1091/1035`, the v237 resident copy moves low to `1088/1035`, the
v248 no-op residual compaction moves low to `1086/1035`, and v249 no-ops
frame `113` to move low to `1075/1039`; the v291 paired current-layout low
control was `1079/1039`, v292 aliases frames `114..117` to frame `113`'s
resident no-op payload, and v302 aliases frame `123` to frame `121`, grows the
second setup segment from `24` to `27` sectors, and copies frame `128`
resident. The current low profile is `1071/1039`, overrun `32`, blocking `63`,
loop reads `11`, loop-read time `63`, and due misses `11`, with current scene
total at `scene_vb=1399`.
The v193 rewrite trims another `11149` high active bytes: high improves to
`scene_vb=1405`, `loop_vb=1101`, `target_vb=1030`, `overrun_vb=71`, and due
misses `21`, while `blocking_vb` improves `120 -> 116` and `loop_read_vb`
improves `122 -> 118`. The later v299 pass owns the current high profile:
`scene_vb=1402`, `loop_vb=1070`, `target_vb=1039`, `overrun_vb=31`,
`blocking_vb=49`, `loop_read_vb=49`, and due misses `9`. The latest low
resident-segment pass owns the current low profile:
`scene_vb=1399`, `loop_vb=1071`, `target_vb=1039`, `overrun_vb=32`,
`blocking_vb=63`, `loop_read_vb=63`, and due misses `11`. Hidden refill stays
`prefetch_overrun_vb=0` on both tides. The latest broad no-regression gate is
`scratch/ps1-perf-iterate/visitor3-low-f128-resident-seg27-v302-broad-norequire/20260510-042342-2826400/summary.json`;
the high focused gate is
`scratch/ps1-perf-iterate/visitor3-high-f131-resident-alias121123-v299-focused/20260510-033736-2567021/summary.json`.
The low focused gate is
`scratch/ps1-perf-iterate/visitor3-low-f128-resident-seg27-v302-focused/20260510-042223-2818332/summary.json`.
Rejected low cap probes at `288 KiB`, `256 KiB`, and `240 KiB` either added
hidden refill or regressed visible timing, so low stays capped at `208 KiB`.
The follow-up v208 frame `116` copy-only hull probe is rejected: it saved
`9102` high active bytes (`14226 -> 5124`) with fixed footprint/LBA, but
regressed high to `1427`, `1114/1029`, overrun `85`, and blocking `114`.
The follow-up v209 frame `114` copy-only hull probe is also rejected: it saved
`2567` high active bytes (`7171 -> 4604`) with fixed footprint/LBA, but
regressed high to `1418`, `1105/1029`, and overrun `76` with blocking flat at
`106`.

Latest rejected VISITOR3 v183-v192 probes: low-tide precursor motion-copy
expansion, C-side fastspan row copies, terminal zero trimming, compact-origin
rebasing, low precursor hull motion, terminal hand-authored read groups, and
compact motion-copy metadata plus frame `117` sparse-hull variants and generic
runtime narrow upload are closed for now. Low frames `114..118` saved
`59543` active payload bytes on paper, but regressed low from `1108/1028` to
`1129/1027` and added hidden
refill. Split-frame tests showed no single precursor frame promotes; sparse
frame `117` stayed negative, and the later v184 hull-mode retry still regressed
the closest low frame to `1110/1028` despite lowering blocking `143 -> 141`. A
runtime same-tile fast path for motion spans crossed the PS-EXE bucket
`215040 -> 217088` and did not improve key metrics. Terminal zero-run trimming
found `0` bytes, compact-origin rebasing found only a `12`-byte high frame `113`
host-side saving and no terminal saving, and v184 terminal `16`-sector read
groups reduced high reads/blocking but regressed high loop `1104 -> 1108` and
low loop `1108 -> 1112`. The v185 narrowed left-tile row-copy runtime path kept
layout stable at `215040` with fixed VISITOR3 LBAs, but both tides were
exact-flat against v182 (`1104/1030` high and `1108/1028` low). The v186
contiguous one-span copy metadata opcode saved `3906` high active bytes and
`3485` low active bytes, but compact repack regressed both tides and
sparse-in-place still regressed low to `1119/1027` with hidden refill. The v190
and v191 frame `117` sparse-hull variants saved bytes but regressed both/low
timing; only the later high-only target-hull shape promoted. The v187/v192
runtime narrow-upload paths crossed the PS-EXE bucket or failed before
`JCPERF2`, even though the host-side x-band model remains useful. The v208
frame `116` copy-only hull retry closes the largest remaining precursor hull
byte signal for high tide too: removing cleanup rows still converted bytes into
visible cadence debt. The v209 frame `114` retry repeats the pattern with a
smaller copy footprint, so adjacent precursor hull rewrites are closed unless
the payload format changes. The v215 unguarded low second-segment probe showed
the same segment can reduce reads but fail when hidden refill appears; v216
promoted only after the low refill slack guard made the segment deadline-aware.
The v222 guarded low `133..149` grouped append is also closed: it kept
VISITOR3 low exact-flat at `1098/1034`, reported `group_hits=0`, and grew
`foregroundPilotPlay` by `48` bytes, proving append-start fireability alone is
not enough under the current one-staged-frame/window-prefetch state machine.
The v223 fresh-window version of the same group is closed too: it also stayed
exact-flat at `1098/1034`, kept blocking at `112` and due misses at `19`, and
added `12` bytes to `fgRuntimeFillWindowForEntry` plus `48` bytes to
`foregroundPilotPlay`. Hand-authored `133..149` grouping is now exhausted; the
next VISITOR3 swing must either change the data shape enough to reduce both CD
and CPU, or add deadline-aware scheduling with a second retained window/sector
map rather than another scalar table row. The v224 low frame `123` original
payload rollback is closed as exact-flat too: it grew the frame `2616 -> 17570`
bytes, shifted the sound table by `+362`, but left low at `1098/1034`,
blocking `112`, and due misses `19`; single-frame rollback of the accepted
`119..123` motion cluster is not the missing win.

Latest rejected VISITOR3 low direct-stage micro-guard: v408 denied the
exact-min-slack direct-stage shortcut for low-tide source frame `117` only.
The current v338 row stayed exact-flat at scene `1401`, active loop/target
`1072/1040`, overrun `32`, blocking `58`, hidden refill `0`, loop reads `10`,
and due `10`, while `foregroundPilotPlay` grew by `68` bytes and hot symbols
shifted. Close frame-117 direct-stage denial; the next VISITOR3 work should use
trace/heatmap-guided payload changes or generated ownership, not another
single-frame branch.

## VISITOR3 white-whale backlog

These are intentionally deeper than the exhausted scalar/read-group lanes.
Rank order is current working priority, not guaranteed payoff.

| Rank | Idea | Why it could work | Main risk / gate |
|---:|---|---|---|
| 1 | Frame-local residual dictionary for terminal frames `139..144` | Tail duplication proved the late payload cluster is expensive, but moving it later is phase-negative. A same-offset dictionary inside each original payload could shrink bytes without changing pack LBA or read phase. | Needs a tiny decoder or generated FGP3 shape that stays inside the `215040` byte PS-EXE bucket. |
| 2 | VISITOR3-only precomposed background-owned strips | Upload-ready analysis failed because selected pixels were not foreground-owned. Precompose strips from the known background state at pack build time and mark them background-owned so runtime can blit safe pixels without cleanup reads. | Must prove no island/tide/night/holiday state dependency leaks into the strip pixels. |
| 3 | Motion opcode v2 with row-run headers shared across frames | Current motion-copy repeats row/span metadata per frame. Frames `119..123` and nearby yacht/state motion share geometry; a shared per-scene row-run table could shrink CD bytes without adding per-pixel work. | Runtime table lookup must be code-size-neutral or replace existing motion parser branches. |
| 4 | Motion opcode v2 block-copy rectangles | Low precursor motion loses CPU. Encoding a few rectangular copy regions instead of hundreds of row spans could cut CPU while keeping the byte win. | Rectangular copies may include pixels that should stay transparent unless masks are precise. |
| 5 | Motion opcode v2 masked strip copy | Store a bitmask per strip and copy only set pixels from old background. This may cut cleanup/residual bytes for yacht motion while reducing per-pixel span metadata. | Mask decode may cost more than it saves unless masks are highly compressible. |
| 6 | Per-frame choose-between-original-and-motion with CPU model | Low frames save bytes but lose cycles. Build a measured eligibility model using payload saved, copy pixels, cleanup pixels, and read timing, then test only frames predicted to be net-positive. | Model can overfit; strict gates remain the source of truth. |
| 7 | Same-offset payload packing for frame `117` only plus micro residual trim | Frame `117` was the closest low-tide precursor. Combine sparse-in-place motion with residual-tail trimming or cleanup subtraction to find a one-frame net win. | The known result is still +3 VBlanks with sparse-in-place, so the added trim must be real. |
| 8 | Terminal frame partial restore-minus-current v2 | Existing restore-minus-current already helped VISITOR3, but terminal frames may still restore rows immediately overwritten by motion/exposed cleanup. Re-run subtraction with motion-aware current intervals. | Must not disturb visual correctness around yacht wake/rope/water overlaps. |
| 9 | Frame-order-aware prefetch deadline sidecar | Local read groups do not fire; generated metadata could tell the scheduler exact frame deadlines and safe read groups for VISITOR3 without C table churn. | Requires runtime scheduling changes and proof that setup/held slack is sufficient. |
| 10 | Do-not-stage negative hints for late terminal reads | Tail atlas and taildup showed some movements are phase-negative. A sidecar can also forbid speculative staging that causes hidden refill debt near terminal frames. | Hard to distinguish helpful hidden work from harmful hidden debt without per-frame gates. |
| 11 | Per-pack sector clustering around existing hot offsets | Instead of moving data to zero-tail, repack only within the already-read sector neighborhood so hot frames share sector reads but keep phase. | Entry offsets/sound offsets must stay valid and file footprint fixed. |
| 12 | Duplicate only subpayload spans inside current sectors | If full entry duplication is phase-negative, duplicate only the spans that trigger extra sectors into slack bytes in the same sector group. | Needs pack surgery below entry granularity. |
| 13 | VISITOR3 terminal frame split: early core plus late tail | Split large entries into an early resident core and a late small residual, reducing blocking at due time without moving the whole payload. | Requires pack/runtime support for two payload sources per frame. |
| 14 | Palette-index RLE for repeated water/background strips | VISITOR3 late frames likely contain repeated water/background runs. A VISITOR3-specific RLE substream could be tiny and decode into PAL4 spans. | Decoder code size and branch cost. |
| 15 | Extend previous-frame D4 deltas beyond frames `129`/`132` | v452 proved a tiny prior-frame delta can remove one VISITOR3 low due read, and v470 proved frame `132` can shave one more blocking/read VBlank without moving pack LBA or growing the executable bucket. Re-rank frame `137` and the terminal cluster for the same mechanism, one frame at a time. | v484 proved high frame `133` chained D4 is byte-valid but runtime-unsafe with current staged/prepared ownership. Multi-frame delta probes need explicit ownership gates before focused plus broad timing gates. |
| 16 | Generated per-frame copy-previous-background mode | Motion-copy is one instance. Generalize to copy selected previous background regions plus smaller draw deltas for frames that are near-identical but not simple X translations. | Copy regions can propagate stale pixels if cleanup ownership is wrong. |
| 17 | Pre-baked clean-rect cache for VISITOR3 yacht region | Keep a small cached clean background rectangle for the yacht travel band to make cleanup cheaper than reading/restoring dynamic spans. | RAM pressure and correctness with island/water overlays. |
| 18 | Tide-specific opcode selection | High and low differ: high accepted frame `115`, low rejected every precursor. Generate independent opcode families and never assume paired eligibility. | More tooling complexity and broader visual verification burden. |
| 19 | Sound-table alignment preservation for all probes | Sparse-in-place showed offset churn was not the main issue for frame `117`, but future repacks should default to preserving sound and later entry offsets to isolate CPU/CD effects. | May leave less byte budget for dictionary payloads. |
| 20 | VISITOR3 frame-class heatmap | Build a per-frame heatmap of bytes, reads, blocking, copy pixels, cleanup pixels, upload bytes, and target slack to rank custom algorithms by measured bottleneck, not intuition. | Analysis time, but low implementation risk. |
| 21 | Reclaim code headroom before runtime opcode work | The fastspan idea failed mainly by crossing the EXE bucket. Prune or shrink cold foreground code first, then retry only the smallest runtime opcode variant. | Code-headroom commits must be flat across broad canaries. |
| 22 | Replace generic motion parser with VISITOR3-only parser | If motion remains VISITOR3-only, a hardwired parser may be smaller/faster than the generic marker/opcode path. | Less reusable and still must preserve v181/v182 behavior exactly. |
| 23 | Static frame `115/119..123` motion table in code | Move repeated motion metadata to code and keep pack payloads smaller. | Probably crosses PS-EXE bucket unless offset by code pruning; also moves data from CD to executable. |
| 24 | Pack-side row span macro table | Store common row span lists once in the pack and reference them by small IDs from several frames. | Runtime indirection and validation complexity. |
| 25 | VISITOR3 low alternate baseline without frame `119..123` motion | Since low precursor motion is CPU-sensitive, test whether the accepted low `119..123` set still dominates after v182 and current scheduler changes. | Could regress the best known low baseline; log-only if so. |
| 26 | Targeted due-miss reduction via frame deadline padding | Some remaining due misses may be caused by target accounting around tight frames. Slight timing-table adjustments that preserve visual cadence could reduce due pressure. | Must not lie about timing or speed; visual/audio cadence gate is strict. |
| 27 | Strip-level lossless equivalence search | Search alternate span partitions that render byte-identical but have lower metadata/read cost, especially for long horizontal water strips. | Tooling-heavy, but no runtime code if successful. |
| 28 | Re-run VISITOR3 read candidates after every data-shape change | Data-shape wins can make previously inert scheduler rows fire. Treat read candidates as invalidated after each accepted pack transform. | Prevents stale decisions, but can burn run time. |
| 29 | Scene-local LBA padding before VISITOR3 packs | If another accepted change shifts LBAs, deliberately pad earlier CD layout to keep VISITOR3 phase stable while testing pack mutations. | Must not shift other canaries or PS-EXE bucket. |
| 30 | VISITOR3 custom decompressor in overlay budget | If main EXE bucket cannot fit a decoder, test whether a scene-local overlay/decompressor loaded during setup can pay for itself without active-loop debt. | Setup-time and RAM pressure; likely only worth it for a large terminal-frame byte cut. |
| 31 | VISITOR3 low grouped-append state trace | Instrument why the fireable `133..149` row still reported `group_hits=0`. | Must be trace-only and keep the measured baseline flat. |
| 32 | VISITOR3 two-entry lookahead scheduler | Let VISITOR3 low reserve the frame after the staged frame when the staged frame is already resident. | Could starve present prep unless CD ownership is explicit. |
| 33 | VISITOR3 second stream window just for low `117..118` | Keep one normal window and one tiny retained append window so the useful cluster is not evicted by the staged payload path. | RAM pressure and code-size growth must stay below the current bucket. |
| 34 | VISITOR3 sector-resident bitmap | Track resident sectors independently from the contiguous stream window and setup segments. | Bitmap lookup and copy path can grow hot code. |
| 35 | VISITOR3 phase-local preload after frame `113` | Read the `133..149` cluster during the long pre-precursor slack window rather than at scene setup. | Needs proof that sound/source-frame deadlines still hold. |
| 36 | VISITOR3 low frame `117..118` micro-subpack | Split only the precursor cluster into a tiny subpack loaded at the phase transition. | Subpack lookup and file residency may cost more than one due read. |
| 37 | VISITOR3 same-sector residual compactor | Compress frame `117` enough that frames `117..118` fit one fewer sector without changing offsets after frame `118`. | Sparse in-place space is tight and visual equivalence must be byte-proven. |
| 38 | VISITOR3 payload-hole allocator | Reuse zeroed slack inside already-read sectors for duplicated hot spans. | Requires below-entry offset surgery and safety checks. |
| 39 | VISITOR3 late-cluster in-sector dictionary | Store repeated rows from frames `117..118` and `139..144` once inside their current sector neighborhoods. | Decoder path must be smaller than the saved CD/read pressure. |
| 40 | VISITOR3 low prepared-frame priority inversion | Prefer window prefetch over visual prepare only for the known low precursor cluster. | Could waste prepared-present opportunities and regress loop timing. |
| 41 | VISITOR3 low no-evict terminal segment policy | Keep `281..305` resident while testing extra low cluster ownership, because v221 proved stealing it regresses. | Needs a third residency owner or generated replacement. |
| 42 | VISITOR3 low direct-stage deny for frame `117` only | Closed by v408: exact-flat timing/read metrics with `foregroundPilotPlay` growth. | Do not retry as a hand-coded branch; use generated ownership or payload changes. |
| 43 | VISITOR3 per-source-frame slack ledger | Build a CSV of held slack, staged validity, resident window, and next read per source frame. | Host model must match PS1 counters before trusting it. |
| 44 | VISITOR3 scheduler simulator from JCPERF2 trace | Replay exact wait, prefetch, prepare, and advance decisions before writing C tables. | Requires richer trace export but avoids blind source probes. |
| 45 | VISITOR3 low frame `117` original-payload rollback test | Confirm whether accepted motion payloads still help low after v216, instead of assuming old wins remain additive. | Repacking original larger payloads may require offset movement or padding control. |
| 46 | VISITOR3 high and low divergent pack bodies | Stop forcing tides to share the same motion and residual choices where one tide is CPU-negative. | More generated artifacts and visual-signoff burden. |
| 47 | VISITOR3 copy-span SIMD-style unroll only for opcode marker | Speed the accepted motion-copy rows without touching normal FGP3 spans. | Previous generic copy paths were flat or code-size-negative. |
| 48 | VISITOR3 row-run bytecode replacement | Replace compact span parsing for VISITOR3 with a scene-private bytecode optimized for repeated horizontal rows. | New decoder must fit code headroom and pass exact visual gates. |
| 49 | VISITOR3 predecoded row templates in setup RAM | Decode repeated row layouts once during setup, then consume smaller payload references in the loop. | Setup RAM and lifetime ownership are risky. |
| 50 | VISITOR3 terminal frame still-sequence player | Treat frames `139..144` as a tiny precomputed visual sequence with fixed dirty bands. | Scene-private path must preserve sound and dynamic background state. |
| 51 | VISITOR3 clean-ocean strip atlas | Store deterministic clean water/background strips separately from foreground deltas for yacht frames. | Tide/night/holiday state keying must be airtight. |
| 52 | VISITOR3 direct16 strip micro-proof for frame `127` | Convert only the top modeled upload hotspot to ready-to-upload 16bpp rows. | Large payload bytes could worsen CD even if upload work falls. |
| 53 | VISITOR3 compressed direct16 strips | RLE or LZ encode precomposed 16bpp strips and decode into upload scratch. | Decode cost can erase the upload win. |
| 54 | VISITOR3 FGP3 span partition search | Search alternate byte-identical span cuts that reduce rect count, parser branches, or sector crossings. | Tooling-heavy and may find no feasible same-footprint move. |
| 55 | VISITOR3 frame `128..130` cap-hit bypass | Leave cap-hit rows on current path and optimize only surrounding non-cap rows. | Partial format can add branch cost without enough coverage. |
| 56 | VISITOR3 low due-miss cluster map | Rank remaining due misses by exact frame, read sector, and target slack rather than aggregate counters. | Needs detailed log extraction before coding. |
| 57 | VISITOR3 code-layout padded retry harness | Pad hot symbols and PS-EXE bucket deliberately before testing any new decoder or scheduler branch. | Padding can mask real code-size costs if not documented. |
| 58 | VISITOR3 resident-window replacement search | Automatically try small state-machine variants that choose stage, window, or prepare order per frame class. | Search space can explode; every winner still needs broad canaries. |
| 59 | VISITOR3 foreground-only custom mini-driver | Fork only VISITOR3 playback into a generated, scene-private driver using fixed metadata. | Maintenance burden and binary size, but this may be justified for the white whale. |
| 60 | VISITOR3 final 10-percent decision tree | Compare scheduler, compressed precompose, custom bytecode, and mini-driver paths after three more big swings. | Prevents endless micro-probes but requires disciplined stop criteria. |

Latest promoted ACTIVITY9 low compact-FGP3/v4 baseline: convert `ACTV9LOW.FG2`
to padded compact FGP3/v4 restore-minus-current data while preserving the
`1745484` byte CD footprint. The compaction chain trims active payload
`942219 -> 776893`, cleanup spans `112361 -> 36308`, cleanup pixels
`697279 -> 194715`, and modeled restore bytes `1394558 -> 389430`. Low
improves `scene_vb 2333 -> 2323`, `loop_vb 2085 -> 2075`, `target_vb 2058 ->
2061`, `overrun_vb 27 -> 14`, `blocking_vb 29 -> 17`, `loop_reads 59 -> 47`,
`loop_read_vb 289 -> 232`, and `due_misses 3 -> 1`; hidden refill stays
`prefetch_overrun_vb=12`. Pack LBA stays `3961`, sectors stay `853`, and the
PS-EXE bucket stays `215040`. VISITOR3, BUILDING2, BUILDING4, ACTIVITY9 high,
and FISHING1 broad canaries stayed on their accepted profiles.

Latest promoted JOHNNY1 baseline: keep the compact-FGP3/v4 restore-minus-current
packs and add v932 scene-local local-LZ sentinel payloads for full-frame entries
`1` and `50` while preserving the `448370` byte CD footprints, pack offsets,
high/low LBA/sectors `13983/219` and `14202/219`, and the `217088` byte PS-EXE
bucket. The compact chain had already trimmed active payload `446058 -> 316608`;
v932 cuts the active payload again to `112093`. Both tides now pass at scene
`2024`, loop/target `1948/1945`, overrun `3`, blocking/refill `5`, read time
`37`, due `0`, and target speed `99.846%`, moving JOHNNY1 high/low into green.

Latest JOHNNY1 current-fit read-table decision: do not promote or retry
hand-authored `131..147`, `145..161`, or `138..154` retained read groups.
The v177 probes kept both tides exact-flat at `2070` scene, `1974/1945`,
`overrun_vb=29`, `blocking_vb=26`, `prefetch_overrun_vb=26`, `loop_reads=7`,
`loop_read_vb=56`, and `due_misses=0`, with `group_hits=0` in every run.
These rows remain evidence for generated append ownership or a larger
retained-window design, not for another local table edit.

Latest JOHNNY1 window-slack decision: do not promote or retry a local
`4` VBlank window-refill minimum. The v363 fresh same-layout baselines put both
tides at `1973/1945`, `blocking_vb=25`, `prefetch_overrun_vb=25`,
`loop_reads=7`, and `due_misses=0`. Raising only JOHNNY1 to slack `4` improved
target and hidden refill by one VBlank, but both tides regressed visible
blocking `25 -> 28`, raised loop reads `7 -> 10`, and introduced
`due_misses=1`. The safe scalar knee stays at the global `3` VBlank default;
future JOHNNY1 work needs generated append ownership, a larger retained-window
design, or pack/data-shape reduction.

Latest VISITOR3 hot-atlas decision: do not promote or retry setup-owned tail
atlases under the current runtime. The v178 25-sector atlas duplicated frames
`139..144` into pack zero-tail space and removed real reads, but high regressed
`1118 -> 1141` and low failed the full-scene/hidden-refill gate despite an
active-loop improvement. The v179 7-sector terminal atlas for frames `142..144`
still regressed both tides (`1118 -> 1132` high, `1126 -> 1129` low) and added
low hidden refill debt. The pack file sizes, LBAs, and PS-EXE bucket stayed
fixed, so the closure is about setup-owned CD phase/lifetime, not broad layout.
The v180 no-source tail-duplication isolate also failed: repointing frames
`139..144` to zero-tail copies without any setup segment regressed high to
`1122/1027`, low to `1130/1024`, and added `prefetch_overrun_vb=3` on both
tides. The next VISITOR3 swing needs either data that is cheaper to present
(precomposed/background-owned upload with compression/ownership metadata) or a
real deadline-aware scheduler sidecar; another manual setup segment or
layout-only terminal tail repoint is closed.

Latest promoted WALKSTUF3 low compact-FGP3/v4 baseline: convert
`WALK3LOW.FG2` to padded compact FGP3/v4 restore-minus-current data while
preserving the `994669` byte CD footprint. The compaction chain trims active
payload `986873 -> 429436`, cleanup spans `62722 -> 35300`, cleanup pixels
`266273 -> 149620`, and modeled restore bytes `532546 -> 299240`. Low improves
`loop_vb 2321 -> 2310`, `target_vb 2293 -> 2295`, `overrun_vb 28 -> 15`,
`blocking_vb 41 -> 26`, `prefetch_overrun_vb 21 -> 17`, `loop_reads 72 -> 29`,
`loop_read_vb 359 -> 150`, and `due_misses 5 -> 2`. WALKSTUF3 high and broad
controls stayed exact-flat. Pack LBA stays `26906`, sectors stay `486`, and the
PS-EXE bucket stays `215040`.

Latest promoted BUILDING6 compact-FGP3/v4 baseline: convert both packs to
padded compact FGP3/v4 restore-minus-current data inside the original
`1444370` byte footprints. The chain first expanded raw FGP3 candidates to
`1601445` bytes, then compacted residual metadata, PAL4 draw metadata, and
restore-minus-current payloads back to `970725` bytes before padding. High
improves `loop_vb 2520 -> 2482`, `target_vb 2442 -> 2457`, `overrun_vb
78 -> 25`, `blocking_vb 62 -> 25`, `prefetch_overrun_vb 64 -> 27`,
`loop_reads 74 -> 42`, `loop_read_vb 423 -> 243`, and `due_misses 1 -> 0`.
Low improves `2515 -> 2485`, `2437 -> 2456`, `78 -> 29`, `70 -> 28`,
`66 -> 29`, `73 -> 42`, `420 -> 243`, and `2 -> 0`. Pack LBAs stay
`10754/11460`, sectors stay `706`, and the PS-EXE bucket stays `215040`.

Latest promoted WALKSTUF3 high compact-FGP3/v4 baseline: convert
`WALKSTUF3.FG2` to padded compact FGP3/v4 restore-minus-current data while
preserving the `1026922` byte CD footprint. The compaction chain trims active
payload `1017926 -> 538132` and modeled restore bytes `681994 -> 385772`.
Against current-layout control, high improves `loop_vb 2325 -> 2310`,
`target_vb 2282 -> 2290`, `overrun_vb 43 -> 20`, `blocking_vb 65 -> 47`,
`prefetch_overrun_vb 32 -> 18`, `loop_reads 74 -> 37`, and `loop_read_vb
378 -> 183`. Low tide remains on the unchanged pack and was refreshed as a
current-layout canary at `2321/2293`.

Latest promoted BUILDING2 low restore/read baseline: keep the padded `1303332`
byte `BUIL2LOW.FG2` footprint and fixed pack LBA while reducing active payload
`789906 -> 674798`. The scene-local low-tide window guard uses a `4` VBlank
held-slack threshold so the pack-side win promotes without hidden refill debt,
and the accepted low read groups are now `238..250`, `318..330`, and
`365..381`. The latest v445 row keeps scene/loop flat at `1619/1349`, improves
target `1318 -> 1319`, overrun `31 -> 30`, blocking `81 -> 80`, loop reads
`55 -> 53`, loop-read time `230 -> 227`, and due misses `18 -> 17`; hidden
refill stays `1`.

Recent promoted WALKSTUF1 compact FGP3/v4 baseline: convert both WALKSTUF1
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
candidate.

Latest promoted MARY3 clean-pressure baseline: preserve stage/window prefetch
for MARY3 under clean pressure, but require `8` VBlanks of window-refill slack
for this scene. High improves `loop_vb 2402 -> 2296`, `target_vb 2295 ->
2294`, `overrun_vb 107 -> 2`, `blocking_vb 690 -> 53`, `loop_reads 255 ->
44`, and `due_misses 255 -> 13`; low improves `2402 -> 2297`, `2296 -> 2295`,
`106 -> 2`, `693 -> 51`, `255 -> 44`, and `255 -> 13`. Both tides keep
`prefetch_overrun_vb=0`. A `7` VBlank slack retry was rejected because high
tide regressed against this baseline.

Latest promoted VISITOR5 high-only compact FGP3/no-autoprime baseline:
convert `VISITOR5.FG2` to a padded compact FGP3/v4 restore-minus-current pack
inside the original `354227` byte footprint, but leave `VIST5LOW.FG2` on the
prior FGP2 baseline because the paired low-tide compact candidate regressed CD
debt. VISITOR5 now skips automatic FGP3 setup-prime only when the active pack
is FGP3. High improves `scene_vb 1369 -> 1361`, `loop_vb 1111 -> 1104`,
`target_vb 1090 -> 1092`, `overrun_vb 21 -> 12`, `blocking_vb 12 -> 11`,
`prefetch_overrun_vb 12 -> 11`, `loop_reads 25 -> 19`, and `loop_read_vb
123 -> 91`; pack LBA stays fixed at `24082`, pack bytes stay `354227`, and
the PS-EXE bucket stays `215040`. Low tide was verified exact-flat when its
original FGP2 pack was restored. Broad VISITOR3, BUILDING2, BUILDING4,
ACTIVITY9, and FISHING1 canaries stayed on their accepted profiles. The next
top rows are WALKSTUF1 low/high, BUILDING2 high/low, VISITOR3 low/high,
BUILDING4 low, and the remaining under-99 rows.

Latest promoted VISITOR5 low compact FGP3/read-ownership baseline: convert
`VIST5LOW.FG2` to the same padded compact FGP3/v4 restore-minus-current pack
inside the original `354227` byte footprint, then add a low-tide compact-only
`23..47` retained read after the current HEAD raised grouped-read capacity to
24 sectors. Low improves scene `1371 -> 1363`, active loop/target
`1112/1090 -> 1104/1092`, overrun `22 -> 12`, `blocking_vb 12 -> 11`,
`prefetch_overrun_vb 12 -> 11`, `loop_reads 25 -> 19`, and `loop_read_vb
123 -> 91`; due misses stay `0`, pack LBA stays `24394`, pack sectors stay
`173`, and the PS-EXE bucket stays `217088`. VISITOR5 high, BUILDING2
high/low, WALKSTUF1 high/low, VISITOR3 low, and FISHING1 high canaries stayed
on their accepted profiles. At v451 the public rollup was `+0.3111%` over
target / `99.6949%` target speed, raw signed rollup was `-0.4559%` /
`100.4772%`; v452 supersedes that rollup with the VISITOR3 low frame129 delta
above. After later VISITOR5 and JOHNNY1 promotions, the next top rows are
WALKSTUF1 low/high, BUILDING2 high/low, VISITOR3 high/low, BUILDING4 low, and
the remaining under-99 tail.

Latest promoted BUILDING1 compact FGP3/no-autoprime baseline: convert
`BUILDING1.FG2` and `BUIL1LOW.FG2` to padded compact FGP3/v4
restore-minus-current packs inside the original `318131` byte footprints, but
skip auto-resident FGP3 setup-prime for this scene. The no-autoprime variant
avoids the rejected setup-only scene regression while keeping the active-loop
win: high improves `loop_vb 792 -> 784`, `target_vb 778 -> 782`,
`overrun_vb 14 -> 2`, `blocking_vb 17 -> 15`, `prefetch_overrun_vb 17 -> 9`,
and `loop_reads 23 -> 17`; low improves `794 -> 787`, `779 -> 782`,
`15 -> 5`, `21 -> 16`, `21 -> 14`, and `23 -> 17`. Full scene time also
improves `1028 -> 1020` high and `1032 -> 1025` low. Broad VISITOR3,
BUILDING2, BUILDING4, ACTIVITY9, and FISHING1 canaries stayed on their
accepted profiles.

Closed BUILDING6 direct-FGP3 decision: do not retry raw same-footprint or
layout-moving PAL4 temporal-residual conversion. The uncompressed conversion
expands both packs `1444370 -> 1601445` bytes and the explicit layout-moving
probe regressed high `2520/2442 -> 2618/2418`, `overrun_vb 78 -> 200`, and
`blocking_vb 62 -> 283`; low regressed `2515/2437 -> 2621/2419`,
`78 -> 202`, and `70 -> 292`. That lane is superseded by the compact
FGP3/v4 restore-minus-current promotion above; future BUILDING6 work should
start from the compact baseline, not the raw direct-FGP3 shape.

Latest VISITOR5 paired compact-FGP3 decision: do not promote low tide or the
paired high/low compact FGP3/v4 shape. Both packs fit inside the original
`354227` byte footprint as `279245` active-byte compact residual assets, and
active loops improve in the original setup-prime run (`1112 -> 1102` high,
`1111 -> 1104` low), but full scene time regresses `1369 -> 1382` high and
`1370 -> 1385` low. The no-autoprime paired retry fixes high but makes low CD
debt worse (`blocking_vb 7 -> 15`, `prefetch_overrun_vb 7 -> 15`). Keep the
promoted high-only no-autoprime variant and retry low only with generated
scheduler ownership or another data shape.

Closed BUILDING1 compact-FGP3 setup-prime variant: do not promote paired
compact FGP3/v4 under the current auto-resident setup-prime policy. The
resident setup-prime version made active loops green (`792 -> 781` high,
`794 -> 781` low; `loop_reads 23 -> 8`) but regressed full scene time
`1028 -> 1040` high and `1032 -> 1041` low. Keep the promoted no-autoprime
variant instead.

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

The first hand-coded frame/range follow-up is also closed. Forcing WALKSTUF1
high sector range `178..194` out of the exact-min-slack direct-stage shortcut
and into the retained-window path reduced hidden refill (`prefetch_overrun_vb
32 -> 26`) but regressed the user-facing loop: `1491 -> 1495`, target-relative
overrun `65 -> 68`, blocking `85 -> 95`, reads `69 -> 70`, and due misses
`13 -> 14`. Do not retry this as another local source deny-list; the next
range-specific attempt needs generated scheduler ownership/cost metadata that
can decide when the grouped read is actually hidden.

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
`0` selected draw-covered bytes for the default `117`-frame selective plan and
the current budgeted high `75` / low `74` frame plans. The modeled win depends on
restored background/cleanup pixels, which are dynamic at runtime. Do not build
that as a raw append; continue with a safe pixel-source/data-shape change,
compression plus ownership, or generated scheduler metadata.

Current VISITOR3 upload/read-plan closure: after the v181 motion-copy promotion,
the refreshed matrix still ranks VISITOR3 as the top preprocessing target, but
the v175 raw foreground-only append safety failure remains binding until a new
motion-aware analyzer proves ownership. Artifacts:
`scratch/visitor3-preprocess-safety-v175/visitor3-high.json` and
`scratch/visitor3-preprocess-safety-v175/visitor3-low.json`. The default
selective x-band model selects `117` frames and needs `3394200` bytes for
`10602536` modeled upload bytes saved. The current same-footprint budgeted
model selects high `75` frames using `888880 / 891012` bytes for `6290232`
saved bytes, and low `74` frames using `853848 / 854114` bytes for `6166528`
saved bytes, but both tides still have `0` draw-covered selected x-band bytes.
Runtime dirty-upload narrowing is not a substitute: exact interval upload would
create about `131996` loop rects, and prior scratch-packed x-band probes already
proved the copy/code-size cost is worse than full-width tile bands. The latest
generic BUILDING2-high scratch-pack retry confirms the same failure mode even
with `16`-pixel bands and transfer-alignment fallback: no valid `JCPERF2`, a
stall around frame `132`, and a one-sector PS-EXE growth. The current read-plan
matrix also found no candidate that is append-start fireable,
current-window-sized, and low-risk; the fireable current-fit rows are the
already-closed late tight clusters. Close VISITOR3 runtime dirty-upload and
hand read-table work until generated scheduler ownership or safe
background-owned/precomposed upload data exists.

Latest VISITOR3 v184 terminal read-group retry: after the v182 motion baseline,
hand-authored `16`-sector terminal groups did fire and reduce some read pressure,
but still hurt visible cadence. High terminal `{277,293}` lowered blocking
`128 -> 125`, loop reads `23 -> 21`, and due misses `22 -> 21`, yet regressed
high loop `1104 -> 1108`; low terminal `{297,313}` regressed low loop
`1108 -> 1112`, blocking `143 -> 144`, and due misses only `25 -> 24`. The
two-group variant was no better. Do not retry VISITOR3 terminal hand tables
without generated frame-deadline ownership or a data-shape change.

Current VISITOR3 v150 late-cluster setup/group closure: retesting the top
low-tide `333..349` cluster confirms the planner's `high-risk:scheduler-only`
classification. A low-tide retained read group saved reads (`31 -> 29`) but
regressed active loop `1126 -> 1130`, overrun `101 -> 105`, and blocking
`170 -> 171`. A one-shot setup segment for `159..171` stayed loop-flat while
adding setup cost (`scene_vb 1426 -> 1434`), and a persistent setup segment for
`333..349` regressed loop `1126 -> 1129`, blocking `170 -> 174`, and hidden
refill `0 -> 2`. Follow-up guarded retained groups at `333..349` with
`8` and `7` VBlank minimum slack were exact-flat (`1126/1025`,
`blocking_vb=170`, `loop_reads=31`) and failed `--require-improvement`.
Do not retry VISITOR3 late clusters through local source tables, guarded
source groups, or setup segments; the next attempt needs generated scheduler
ownership or safe precomposed/background-owned pack data.

Closed VISITOR3 low setup-prime gate: the accepted `208 KiB` low-tide cap was
the measured knee before the v181 motion-copy payload pass. Retesting
`216 KiB` preserved high tide but regressed low `1126 -> 1127` and blocking
`170 -> 173`; retesting `200 KiB` regressed low to `1152/1024`, blocking
`191`, and hidden refill `3`. Do not retry scalar low-prime sizes around this
point; VISITOR3 needs generated scheduler ownership or a pack/data-shape change
to reduce the remaining `129/143` visible blocking.

Closed VISITOR3 fallthrough guard gate: lowering
`FG_PREFETCH_FALLTHROUGH_MIN_SLACK_VBLANKS` from `6` to `5` after the
WALKSTUF1 compact pass is rejected as exact-flat. The fresh control and
candidate both kept high at `1422`, `1118/1028`, `blocking_vb=150`,
`prefetch_overrun_vb=0`, and `loop_reads=27`; low stayed `1426`,
`1126/1025`, `blocking_vb=170`, `prefetch_overrun_vb=0`, and
`loop_reads=31`. Keep the guard at `6` and do not spend more VISITOR3 cycles
on local threshold-only fallthrough probes.

Closed VISITOR3 retained-window slack gate: a VISITOR3-only `20 KiB`
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

Current BUILDING2 low chained tail-group gate: adding a follow-up retained
read group `381..397` after the accepted `365..381` group is rejected. It
reduced low-tide `loop_reads 55 -> 54`, but regressed `scene_vb 1657 -> 1661`,
`loop_vb 1383 -> 1387`, overrun `79 -> 83`, and blocking `118 -> 122` while
leaving hidden refill at `5`. Do not retry this tail group as a local source
table; saved reads after `365..381` still need scheduler-owned placement.

Current BUILDING2 low early group gate: `67..73` is closed. Unguarded grouping
cut blocking `81 -> 76` and loop reads `55 -> 54`, but regressed scene/loop,
target, public overrun, and hidden refill (`1619/1349/1318/31/1` to
`1621/1351/1316/35/3`). The slack-4 guarded variant stayed exact-flat, so this
range is not a useful local table.

Current BUILDING2 low mid-cluster group gate: the `204..220` group has the
same strict blocker as restore-minus-current. Guards at `8` and `7` VBlanks
were exact-flat (`1383/1304`, `blocking_vb=118`, `prefetch_overrun_vb=5`).
Unguarded grouping reduced visible blocking `118 -> 108`, `loop_reads 55 ->
53`, and due misses `22 -> 18`, but regressed target-relative overrun `79 ->
82` and hidden refill `5 -> 13`. Treat this as another generated scheduler
placement target, not a hand-authored read group.

Current BUILDING2 high follow-up group gate: the `226..242` row is now the
accepted retained-read group between accepted `60..72` and `249..257`, cutting
loop, overrun, blocking, hidden refill, and loop-read time. The earlier
`249..257` row remains a same-speed pressure reducer after `60..72`. The
older `210..226` row remains scheduler-owned in practice: adding it after the
accepted high `60..72` group kept layout fixed but produced an exact-flat
focused gate (`1599` scene, `1349/1316`, `overrun_vb=33`,
`blocking_vb=48`, `prefetch_overrun_vb=12`, `loop_reads=61`) with
`group_hits=0`, so do not retry `210..226` as a local hand table. The fresh
post-v347 `226..238` follow-up is also closed: it reduced loop reads `61 -> 59`
but worsened loop-read time `262 -> 263` and total read time `402 -> 403`
while every timing/blocking/refill key metric stayed flat.

The VISITOR3 no-op empty-hold recast is also closed under the current packs.
`scripts/compact-fgp3-zero-noop-entries.py` found `0` high-tide and `0`
low-tide FGP3/v4 entries whose cleanup and draw pixel counts are both zero;
active payload stayed `737600 -> 737600` for both packs. That means the earlier
entry-prune speed signal cannot be made cadence-preserving by simply replacing
payloads with empty holds.

The VISITOR3 zero-runtime-code entry-origin shift gate is closed too. Re-centering
each FGP3/v4 entry and subtracting the shift from compact cleanup/draw
coordinates saved only `12` high-tide bytes in the latest v184 current-baseline
scan, saved `0` low-tide bytes, and found no terminal-frame payload reduction,
so there is no meaningful pack payload or CD-duration win to benchmark.

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
FGP3/v4 pass. The missing-scene timing refresh and MARY3 guarded
prefetch-preserve follow-up then move the raw signed matrix to `-0.2159%` /
`100.2547%`, the BUILDING1 compact-FGP3/no-autoprime pass moves it to
`-0.2404%` / `100.2410%`, the VISITOR5 high-only compact-FGP3/no-autoprime pass
moves it to `-0.2471%` / `100.2478%`, and the BUILDING2 low
restore-minus-current/slack-4 pass moves it to `-0.2752%` / `100.2759%`. The
WALKSTUF3 high compact-FGP3/v4 pass moves it to `-0.2825%` / `100.2833%`, the
BUILDING6 compact-FGP3/v4 pass moves it to `-0.3158%` / `100.3500%`, the
ACTIVITY9 high compact-FGP3/v4 pass moves it to `-0.3228%` / `100.3568%`, the
WALKSTUF3 low compact-FGP3/v4 pass moves it to `-0.3273%` / `100.3612%`, and
the JOHNNY1 compact-FGP3/v4 pass moves it to `-0.3281%` / `100.3620%`. The
ACTIVITY9 low compact-FGP3/v4 pass moves it to `-0.3331%` / `100.3669%`, the
VISITOR3 v181 motion-copy payload pass moves it to `-0.3621%` /
`100.3916%`, the VISITOR3 high frame-115 motion-copy follow-up moves it to
`-0.3620%` / `100.3915%`, the VISITOR3 sparse frame-124 work-reduction
follow-up keeps it at `-0.3620%` / `100.3915%`, and the VISITOR3 sparse
frame-118 hull work-reduction follow-up moves it to `-0.3629%` / `100.3922%`.
The VISITOR3 high-only frame-117 target-hull follow-up moves it to
`-0.3644%` / `100.3935%`.
The current rollup is tracked above.

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
remaining absolute gaps are now VISITOR3, WALKSTUF1, BUILDING2, BUILDING4, and
generated selective preprocessing, not FISHING1 or JOHNNY1.

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
pass moves it to `-0.2497%` over target / `100.2899%` target speed. The
missing-scene timing refresh and MARY3 guarded prefetch-preserve follow-up move
the raw signed matrix to `-0.2159%` over target / `100.2547%` target speed,
the BUILDING1 compact-FGP3/no-autoprime pass moves it to `-0.2404%` /
`100.2410%`, the VISITOR5 high-only compact-FGP3/no-autoprime pass moves it to
`-0.2471%` over target / `100.2478%` target speed, and the BUILDING2 low
restore-minus-current/slack-4 pass moves the raw signed matrix to `-0.2752%`
over target / `100.2759%` target speed. The WALKSTUF3 high compact-FGP3/v4
pass then moves it to `-0.2825%` over target / `100.2833%` target speed, the
BUILDING6 compact-FGP3/v4 pass moves it to `-0.3158%` over target /
`100.3500%` target speed, the ACTIVITY9 high compact-FGP3/v4 pass moves it to
`-0.3228%` over target / `100.3568%` target speed, the WALKSTUF3 low
compact-FGP3/v4 pass moves the raw signed matrix to `-0.3273%` over target /
`100.3612%` target speed, and the JOHNNY1 compact-FGP3/v4 pass moves the
raw signed matrix to `-0.3281%` over target / `100.3620%` target speed. The
ACTIVITY9 low compact-FGP3/v4 pass moves the current raw signed matrix to
`-0.3331%` over target / `100.3669%` target speed, the VISITOR3 v181
motion-copy payload pass moves it to `-0.3621%` over target / `100.3916%`
target speed, the VISITOR3 high frame-115 motion-copy follow-up moves it to
`-0.3620%` over target / `100.3915%` target speed, and the VISITOR3 sparse
frame-124 work-reduction follow-up keeps the same raw signed rollup while
lowering VISITOR3 CD pressure. The VISITOR3 sparse frame-118 hull follow-up
then moves the raw signed matrix to `-0.3629%` over target / `100.3922%`
target speed while cutting VISITOR3 blocking to `120/139`. The VISITOR3
high-only frame-117 target-hull follow-up moves the raw signed matrix to
`-0.3644%` over target / `100.3935%` target speed while cutting VISITOR3
blocking to `116/139`; the VISITOR3 low persistent segment v204 then moves the
raw signed matrix to `-0.3723%` over target / `100.4004%` target speed while
cutting VISITOR3 low blocking to `124`. The VISITOR3 high setup-prime v213/v214
passes then move the raw signed matrix to `-0.3856%` over target / `100.4122%`
target speed while cutting VISITOR3 high to `1089/1035` and blocking `83`. The
v216 guarded low second-segment pass moves the raw signed matrix to `-0.3903%`
over target / `100.4164%` target speed while cutting VISITOR3 low to
`1098/1034` and blocking `112`. The v227 low resident re-anchor then moves the
raw signed matrix to `-0.3934%` over target / `100.4192%` target speed while
cutting VISITOR3 low to `1095/1035` and blocking `108`; the v234 low frame-118
resident copy moves it again to `-0.3965%` over target / `100.4219%` target
speed while cutting VISITOR3 low to `1091/1035` and blocking `103`; the v237
low frame-127 resident copy moves it to `-0.3988%` / `100.4240%` while cutting
VISITOR3 low to `1088/1035` and blocking `95`; the v238 high resident-copy
compaction moves it to `-0.4111%` / `100.4353%` while cutting VISITOR3 high to
`1075/1037` and blocking `59`; the v248 low no-op residual compaction moves it
to `-0.4126%` / `100.4367%` while cutting VISITOR3 low to `1086/1035` and
blocking `93`; the v249 low frame-113 no-op residual moves it again to
`-0.4226%` / `100.4459%` while cutting VISITOR3 low to `1075/1039` and
blocking `69`; the v277 WALKSTUF1 high sector `201..213` read-group follow-up
moves it to `-0.4259%` / `100.4489%` while cutting WALKSTUF1 high to
`1488/1426`, overrun `62`, and hidden refill `27`; the v288 gap1/window-
prefetch guard moves it again to `-0.4349%` / `100.4573%` while cutting
WALKSTUF1 high to `1477/1431`, overrun `46`, blocking `90`, and hidden refill
`19`; the v289 low-tide gap1 prefix then moves it to `-0.4416%` /
`100.4635%` while cutting WALKSTUF1 low to `1478/1428`, overrun `50`, blocking
`75`, and due misses `13`; the v291 VISITOR3 high frame-140/tail setup-segment
copy cuts VISITOR3 high to `1074/1038`, overrun `36`, blocking `58`, loop-read
time `58`, and due misses `10`; the v292 VISITOR3 low no-op alias moves the
rollup to `-0.4432%` / `100.4649%` while cutting VISITOR3 low to
`1075/1039`, overrun `36`, blocking `67`, loop-read time `67`, and due misses
`12`; the v299 VISITOR3 high frame-131 resident-alias copy moves the then-current
rollup to `-0.4470%` / `100.4685%` while cutting VISITOR3 high to
`1070/1039`, overrun `31`, blocking `49`, loop-read time `49`, and due misses
`9`; the v302 VISITOR3 low frame-128 resident segment copy moves the then-current
rollup to `-0.4501%` / `100.4714%` while cutting VISITOR3 low to `1071/1039`,
overrun `32`, blocking `63`, loop-read time `63`, and due misses `11`; the
v305 WALKSTUF1 low gap6-prefix plus slack-guard promotion moved the raw
rollup to `-0.4529%` / `100.4740%` while cutting WALKSTUF1 low to
`1475/1430`, overrun `45`, blocking `67`, hidden refill `21`, loop-read time
`282`, and due misses `12`; v316 refreshes the current raw rollup to
`-0.4344%` / `100.4566%` while cutting current-layout WALKSTUF1 high to
`1480/1429`, overrun `51`, blocking `85`, loop-read time `301`, and due
misses `16`; v327 then moves VISITOR3 low to `1072/1040`, overrun `32`,
blocking `64`, loop-read time `64`, and due misses `11`; v331 moves the raw
rollup to `-0.4403%` / `100.4621%` while cutting current-layout WALKSTUF1 low
public overrun to `53`, blocking to `72`, and prefetch overrun to `22`; v338
keeps VISITOR3 low timing flat while cutting blocking to `58`, loop-read time
to `58`, and due misses to `10`; v340 moves the raw rollup to `-0.4420%` /
`100.4637%` while cutting WALKSTUF1 high to `1480/1432`, overrun `48`,
blocking `83`, loop reads `67`, and loop-read time `292`; v347 keeps the
rollup flat while cutting BUILDING2 high blocking `56 -> 55`, hidden refill
`20 -> 19`, loop reads `62 -> 61`, and loop-read time `266 -> 262`.

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
The current compact-pack WALKSTUF1 `201..213` group is also rejected as a
local source table. It gives a real low-tide loop signal (`1489 -> 1486`,
overrun `62 -> 59`, reads `69 -> 67`) but regresses visible blocking
`86 -> 89` and hidden refill `27 -> 28`; guards at `8` and `12` VBlanks
produce the same profile. Keep this range for generated scheduler ownership,
not a hot C table.
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
high/low, BUILDING6 high/low, and BUILDING2 high/low. After the WALKSTUF1,
BUILDING6, ACTIVITY9 high, JOHNNY6, and BUILDING6 slack4 passes, the current
top generated graphics/scheduler targets are WALKSTUF1 low/high, BUILDING2
high/low, VISITOR3 low/high, and BUILDING4 low.
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
| 225 | VISITOR3 precomposed terminal-frame proof | Build a tiny precomposed upload payload only for frames `142..144`, keyed by tide/night/background state. | The setup atlas showed those terminal frames are a real read target; precomposed pixels attack presentation cost without setup CD debt. |
| 226 | VISITOR3 compressed terminal x-band proof | Encode frames `142..144` as row-local RLE/x-delta precomposed bands and size-gate before runtime code. | A 7-sector residency atlas was too expensive as CD setup work; compressed upload data may fit and run only on due frames. |
| 227 | VISITOR3 high-only late-sequence strategy | Treat high and low independently instead of requiring one symmetric atlas or format. | The 25-sector atlas helped low active loop but hurt high badly; tide-specific tactics are required. |
| 228 | VISITOR3 low-only late-sequence strategy | Build a low-only candidate that preserves `prefetch_overrun_vb=0` before testing high. | Low has the largest public gap and showed the strongest read-removal signal, but hidden refill must be owned explicitly. |
| 229 | VISITOR3 deadline sidecar for frames `139..144` | Generate per-frame read deadlines and slack reservations for the terminal yacht/photo cluster. | The atlas proved data residency can remove reads; scheduler ownership is the missing piece. |
| 230 | VISITOR3 held-slice replay simulator | Replay the v178/v179 logs to identify which held waits became setup/hidden debt. | Prevents another emulator run that only moves reads into scene-vb or `prefetch_overrun_vb`. |
| 231 | VISITOR3 primary-dataOffset atlas | Repoint `header.dataOffset` to a compact duplicate ordering that includes current setup-covered frames plus late hot frames. | Tests whether compacting the resident set inside the existing setup window can help without adding a second setup read. |
| 232 | VISITOR3 setup-prime resident-set knapsack | Select entries for a compact setup window by read cost and early-frame preservation, then duplicate only that set. | Avoids losing the early `1..116`/`1..112` coverage while making room for late hotspots. |
| 233 | VISITOR3 pack-order local search | Search payload order/duplication inside zero-tail while keeping file size/LBA fixed and scoring a read-plan model before emulator. | Manual atlas ordering is too crude for a fragile CD phase problem. |
| 234 | VISITOR3 frame `127..140` upload hotspot micro-pack | Target the highest modeled upload-savings frames separately from the terminal read cluster. | The upload model says frames `125..127` dominate byte savings; the read atlas targeted the wrong bottleneck for high tide. |
| 235 | VISITOR3 cap-hit fallback table | Leave cap-hit frames on current full-width upload while precomposing only non-cap rows. | Prevents a compact upload format from repeating old rect-cap and scratch-packing failures. |
| 236 | VISITOR3 background-owned ocean strip cache | Store deterministic ocean/background strips once per tide and compose foreground deltas against that cache. | Raw foreground x-bands are unsafe because selected pixels include restored background. |
| 237 | VISITOR3 clean-state hash verifier | Runtime-check a small background hash before accepting any precomposed strip. | Lets us bake background-owned pixels without risking wrong tide/night/holiday visuals. |
| 238 | VISITOR3 row-template dictionary | Deduplicate repeated dirty-row layouts and payload rows across the late yacht frames. | Rect and row metadata are large enough that dictionary reuse may be the only same-footprint path. |
| 239 | VISITOR3 direct16 strip format | Store ready-to-upload 16bpp strips for selected frames instead of PAL4 spans plus runtime compose. | Bypasses PAL4 compose and dirty-restore for the worst frames at the cost of pack bytes. |
| 240 | VISITOR3 PAL4-to-direct16 offline converter | Host-generate direct16 payloads from validated foreground/background states and compare exact pixels to runtime output. | Produces the source material needed for a safe precomposed strip experiment. |
| 241 | VISITOR3 frame-cluster subpack | Split the final yacht/photo cluster into a tiny secondary pack loaded only when entering that phase. | A smaller phase pack may make late data local without a scene-start setup read. |
| 242 | VISITOR3 phase-transition preload | Start reading the terminal subpack during the long pre-terminal hold instead of scene setup. | Moves the atlas idea to the point where spare scene time actually exists. |
| 243 | VISITOR3 second staged-payload slot | Add a scene-local two-entry stage queue only for the terminal cluster. | Current one-slot staging cannot get far enough ahead through tight late reads. |
| 244 | VISITOR3 associative sector cache | Keep a small sector cache for the terminal cluster without invalidating it on unrelated direct reads. | More precise than the failed single persistent segment and may avoid code-path lifetime side effects. |
| 245 | VISITOR3 no-code pack-tail duplication | Duplicate hot frames into tail positions but leave runtime setup untouched, measuring whether stream locality alone helps. | Separates data layout effects from the failed setup-segment code path. |
| 246 | VISITOR3 code-neutral parser funding | Reclaim or pad hot code before any new precomposed decoder so PS-EXE bucket and hot-symbol phase stay fixed. | Prior data-shape wins failed when decoder/code movement shifted other canaries. |
| 247 | VISITOR3 overlay/cold decoder probe | Put the precomposed-strip decoder in a cold overlay-like path or isolated TU with layout padding. | Allows a new format without growing `foregroundPilotPlay` in the hot loop. |
| 248 | VISITOR3 LZ4-style payload size gate | Test simple bytewise LZ/RLE on active FGP3 payload bodies and selected upload bands offline. | If payload compression saves enough sectors, it may beat scheduler work without unsafe pixels. |
| 249 | VISITOR3 per-frame read-vs-upload classifier | Label each bad frame as CD-read dominated, upload dominated, compose dominated, or mixed. | Prevents aiming read-residency at frames whose dominant cost is presentation. |
| 250 | VISITOR3 long-hold retime canary | Try a generated retime that only borrows from long holds after sound-safe proof and resident-next-frame proof. | The scene has long holds; controlled retiming may absorb unavoidable late reads without changing event order. |
| 251 | VISITOR3 sound-event timing proof | Build a host check that any retime preserves sound cursor/source-frame semantics. | Retiming is off-limits until audio correctness is mechanically proven. |
| 252 | VISITOR3 visual splice proof | Host-render the late cluster as a precomputed visual sequence and compare against frame captures. | If exact, a custom scene-private playback path becomes plausible for the white whale. |
| 253 | VISITOR3 custom terminal mini-player | Implement a branch just for the last yacht/photo phase using preloaded/precomposed frames. | Scene-specific code is acceptable if it is isolated, layout-funded, and measured. |
| 254 | VISITOR3 white-whale decision gate | After three more big swings, compare expected gain of scheduler/precomposed/custom-player paths and commit to one deeper implementation. | Prevents endless small probes while keeping VISITOR3 as the priority target. |
| 255 | WALKSTUF1 frame-phase heatmap | Build a per-frame table of payload bytes, sector span, copied pixels, cleanup pixels, hidden refill, and visible blocking contribution for high/low. | The v274-v276 frame-109 probe proved payload bytes alone are not predictive; cadence and CPU have to be scored together. |
| 256 | WALKSTUF1 zero-runtime precomposed frame 109 | Replace frame `109` with a compact precomposed final-pixel payload instead of a motion-copy runtime helper. | The frame has a clean byte signal, but runtime row motion is timing-flat or code-negative. |
| 257 | WALKSTUF1 multi-frame sector-boundary packer | Choose motion/precompose candidates by whether their combined savings eliminate or move full sectors, not by individual payload savings. | Sparse frame `109` saved `3139` bytes but left read sectors unchanged. |
| 258 | WALKSTUF1 no-LBA internal repack search | Brute-force payload ordering inside the padded pack while holding pack LBA and PS-EXE fixed, then run only layouts that reduce due reads. | v276 layout movement helped hidden refill but worsened public cadence; a constrained layout search may find a safe ordering. |
| 259 | WALKSTUF1 phase-safe non-sparse bisection | Try controlled non-sparse repacks for frames `89`, `109`, `167`, and `201` one at a time, scored by read-plan deltas before runtime. | The old all-four pass mixed a high signal with low-tide regressions; tide/frame isolation is required. |
| 260 | WALKSTUF1 copied-pixel CPU budget gate | Reject any motion candidate whose copied-pixel count exceeds the payload-sector savings unless a faster helper fits the current PS-EXE bucket. | Frame `109` copied `13808` pixels and stayed timing-flat despite large byte savings. |
| 261 | WALKSTUF1 direct precomposed strip codec | Encode only the changed yacht/body strips as final RGB555 or PAL4 rows with row-local RLE. | Avoids moving pixels from the background mirror and may cut compose/restore work directly. |
| 262 | WALKSTUF1 cleanup-minus-next-current | Omit cleanup spans that are immediately covered by the next frame, not just the current frame. | Current restore-minus-current helped; the hot clusters may still restore pixels that become hidden one frame later. |
| 263 | WALKSTUF1 restore deferral proof | Delay selected cleanup restores by one held frame when the exposed background is not visible yet. | This targets cleanup work without changing payload layout, but needs a strict visual-frame proof. |
| 264 | WALKSTUF1 retained-sector scratch sidecar | Keep one hot 12-sector cluster resident in a small scratch buffer loaded during existing setup reads instead of adding a new setup read. | v269 failed because the new setup segment cost more than it saved. |
| 265 | WALKSTUF1 setup-prime prefix-hole fill | Closed by v509 for prefix holes. Do not move FGP3 payload entries before `data_offset`, even if those sectors are covered by setup-prime. | v509 used `247` of `296` bytes in the prefix hole and failed structurally with missing `JCPERF2` plus invalid byte reads. Retry only with a proven in-data hole or generated/code-neutral residency. |
| 266 | WALKSTUF1 sector-aligned frame split | Split large hot payloads so the due portion sits before a sector boundary and cold tail can be prefetched later. | Several misses are sector-phase problems rather than raw byte problems. |
| 267 | WALKSTUF1 frame-hold retimer around `145..155` | Stretch a long-safe hold before the hot cluster only if the next sector window is resident, preserving total scene cadence. | Cleanup v264 proved the cluster can improve public timing but hidden refill must be owned. |
| 268 | WALKSTUF1 generated refill reservation map | Emit a tiny per-frame table that blocks window reads in frames that would steal visible cadence and forces them into known hidden slack. | Prepare-before-window v273 proved naive priority inversion hurts; generated reservations need exact phase ownership. |
| 269 | WALKSTUF1 dual-window prototype | Keep the normal stream window plus one tiny retained hot-sector window for `201..213` or `287..303`. | Hand groups either did not fire or converted reads to visible debt. |
| 270 | WALKSTUF1 cold decoded read plan | Decode scene-local scheduler metadata once at scene start from the pack, avoiding hot C table branches. | Source-table probes are now limited by hot-symbol drift as much as logic. |
| 271 | WALKSTUF1 per-tide divergent packs | Stop requiring high and low to share semantic transforms when only one tide has a safe phase. | Earlier all-tide motion copied low into a worse cadence while high had a partial signal. |
| 272 | WALKSTUF1 old-position cleanup atlas | Precompute clean-background strips for the exact exposed old-position pixels instead of restoring through generic rect lookup. | Generic cleanup spans add restore cost and dirty rows; an atlas may be smaller and faster for repeated yacht positions. |
| 273 | WALKSTUF1 row-template dictionary | Deduplicate repeated cleanup/draw row span patterns across frames `89`, `109`, `167`, `201`, and `145..155`. | Metadata repetition is large, but compacting it must not add hot parser cost. |
| 274 | WALKSTUF1 tiny assembly row mover | If motion stays viable, implement the row-copy primitive as a fixed MIPS helper and compare code size to C. | The C fast-copy helper crossed a PS-EXE sector; assembly may be small enough to retest. |
| 275 | WALKSTUF1 MoveImage hardware probe | Test GPU `MoveImage` for same-background-row motion while separately updating the RAM mirror. | Hardware copy may be faster, but mirror/dirty invariants must be proved before promotion. |
| 276 | WALKSTUF1 RAM-mirror row-copy batch | Batch all row moves for one frame, update mirror rows once, then mark dirty rects once per row. | The current opcode marks/copies per span and may burn the byte win in CPU overhead. |
| 277 | WALKSTUF1 dirty-rect coalescer by source frame | For motion/precompose frames, emit dirty rectangles directly instead of deriving them from copied/restore spans. | Dirty upload volume and rect count remain meaningful blockers in the current baseline. |
| 278 | WALKSTUF1 active-payload entropy codec | Run row-RLE, nibble-delta, LZSS, and per-frame Huffman size gates on the current high/low active payloads. | A custom compression path may remove sectors without semantic motion CPU. |
| 279 | WALKSTUF1 decode-cost simulator | Estimate decompression VBlanks from operation counts before building a runtime codec. | Compression only helps if decode work is cheaper than the CD/refill it removes. |
| 280 | WALKSTUF1 one-frame keyframe recast | Make a selected hot frame a full precomposed keyframe if it removes multiple later cleanup/restores. | A larger frame can still win if it improves subsequent cadence and sector phase. |
| 281 | WALKSTUF1 visual equivalence replay for custom codecs | Host-replay every candidate payload against clean background and compare frame hashes before emulator time. | Big custom formats need a cheap correctness gate or the iteration loop will be too slow. |
| 282 | WALKSTUF1 low-first independent pass | Retarget low tide with the same creative lanes after high has no safe immediate promotion. | Low is also under 99% and may have different phase knees than high. |
| 283 | WALKSTUF1 profiler trace build | Add frame-indexed compose/restore/CD/dirty counters for only WALKSTUF1 high to identify the true last 66 VBlanks. | Current aggregate counters hide which frames pay the remaining overrun. |
| 284 | WALKSTUF1 promotion batch minimizer | When a candidate improves high but hurts low or controls, automatically isolate source vs pack vs layout deltas and retest the smallest subset. | Earlier broad failures mixed real local wins with unrelated layout/control drift. |
| 285 | Under-green frame-phase trace gate | Generate a per-frame table for all six yellow rows with loop VBlank, read due slot, blocking, refill, upload bytes, restore bytes, and held-frame slack. | The remaining gaps are only 26-34 VBlanks; aggregate counters are too coarse for the final pass. |
| 286 | BUILDING2 high generated deadline owner | Replace hard-coded B2-high read groups with generated per-frame deadlines that only append when a known hidden slot exists. | Every scalar B2-high row saved reads but landed in visible cadence debt; deadline ownership is the missing variable. |
| 287 | BUILDING2 high clean-safe residency swap | Try a same-footprint setup residency swap that removes one active B2-high cluster while lowering clean-strip lifetime pressure. | B2-low proved residency can pay when paired with allocator shape; high needs the same memory-safe pairing. |
| 288 | BUILDING2 high residual dictionary gate | Offline-score repeated FGP3 residual rows in the B2-high hot clusters and only test dictionaries that remove a full sector. | Payload work must change sector phase now; byte-only savings have been timing-flat or phase-negative. |
| 289 | BUILDING2 high clean allocation relocation | Move only B2-high clean-rect temporary storage to a shorter-lived arena or later allocation point. | B2-low setup expansion failed until clean memory shape was handled; high may have the same hidden allocator blocker. |
| 290 | BUILDING2 high upload-vs-restore splitter | Classify B2-high overrun frames by upload bytes versus restore bytes before adding another CD change. | If the last 34 VBlanks are presentation-bound, more read groups will keep failing. |
| 291 | BUILDING4 low background-owned xband micro-proof | Precompute one or two selected B4-low dirty x-bands with background-owned pixels and host-compare against frame captures. | The x-band byte model is large, but foreground-only bands were unsafe; prove pixel ownership on a tiny slice first. |
| 292 | BUILDING4 low direct16 one-frame sidecar | Encode one B4-low hot frame as a direct RGB555 sidecar inside existing zero-tail slack and branch only for that frame. | A one-frame proof limits code and correctness risk while testing whether upload/compose work is the real blocker. |
| 293 | BUILDING4 low pack-emitted dirty-band plan | Store dirty-band rectangles in the pack so runtime skips hot clean-rect scanning and avoids generic band discovery. | Runtime clean-loop rewrites were flat; generated metadata can remove work without scanning new state. |
| 294 | BUILDING4 low setup/read co-schedule table | Generate a table that pairs existing setup coverage with allowed loop reads for the B4-low 30/1 read profile. | The row has both visible blocking and refill debt; a single-side scheduler change keeps moving debt around. |
| 295 | BUILDING4 low clean lifetime split | Split clean/background scratch lifetime so the scene can test larger residency without starving clean allocations. | Several promising residency forms fail structurally or flatline because allocator lifetime, not bytes alone, is binding. |
| 296 | VISITOR3 high terminal cluster sidecar | Build a scene-private sidecar for the late VISITOR3 high terminal frames and keep the parser layout-funded. | VISITOR3 is still the largest percentage miss; terminal work needs a custom format rather than more table rows. |
| 297 | VISITOR3 high raw-gap scheduler ownership | Generate ownership metadata specifically for the accepted frame56/57 raw-gap and tight-window cap. | The cap win proves this region is phase-sensitive; explicit ownership may harvest the remaining refill VBlanks. |
| 298 | VISITOR3 high clean-strip arena placement | Move high-only clean strips out of the contested cache lifetime while preserving the accepted 64 KiB cap. | Clean64 bought one target VBlank; allocator placement may unlock the next cap without the 48 KiB structural miss. |
| 299 | VISITOR3 low D4 chain extension | Test more D4 deltas adjacent to the accepted frame129/frame132/frame137 family, but require fixed LBA and zero hidden refill. | The current low baseline already accepts custom deltas; extending the family is more plausible than new residency. |
| 300 | VISITOR3 low shared terminal dictionary | Build a dictionary across terminal cleanup rows 137-144 and compare active sectors before emulator runs. | The terminal cluster has repeated cleanup structure and prior tail compaction worked without runtime code. |
| 301 | WALKSTUF1 high generated phase heatmap | Produce a high-tide frame heatmap from perf logs and pack metadata before selecting the next hot frame. | The remaining W1-high rows are close enough that blind frame choice is wasting emulator cycles. |
| 302 | WALKSTUF1 high cleanup-minus-next proof | Host-render cleanup deferral where the next frame immediately covers the restored pixels, then test only visually proven rows. | Current restore-minus-current was useful; next-frame coverage may remove work without changing CD reads. |
| 303 | WALKSTUF1 high precomposed strip sidecar | Encode the yacht/body hot strips as final pixels for one frame and use a sidecar decoder outside the generic compositor. | Motion-copy CPU erased byte wins; direct final strips test a different cost profile. |
| 304 | WALKSTUF1 low divergent scheduler pass | Generate low-only refill reservations instead of carrying high-side read groups into low tide. | Earlier high carryovers regressed low; tide divergence is necessary for the last few VBlanks. |
| 305 | WALKSTUF1 low retained hot-sector scratch | Fill a small scratch buffer from existing setup reads and use it for one low-tide hot cluster without adding setup bytes. | New setup segments cost too much, but the allocator may allow a short-lived scratch resident. |
| 306 | Yellow-scene scratch arena reset | Add an opt-in per-major-scene scratch arena reset for foreground temporaries while leaving background audio allocations intact. | The custom allocator opens a new way to prevent fragmentation and lower allocation search cost between heavy scenes. |
| 307 | FGP3 frame-local temporary pool | Allocate compositor clean/upload temporaries from a frame-local pool that is cleared after present. | Many remaining scenes churn similar buffers every frame; frame-local lifetime is stricter than heap lifetime. |
| 308 | Hot-code padding harness | Add a build-time padding knob that restores hot symbol addresses after source experiments. | Several real-looking wins were hidden by hot layout drift; padding separates algorithm wins from code placement noise. |
| 309 | Release-speed shadow baseline | Maintain a no-perf-log baseline beside the perf-log battle card for the final yellow rows. | If logging cadence is influencing phase, the final 1-3% should be checked against the shipped runtime too. |
| 310 | JCPERF2 frame correlator | Import JCPERF2 events into a frame-indexed CSV keyed by scene frame and pack entry. | This turns hidden refill and blocking counters into actionable frame targets. |
| 311 | Experiment canary minimizer | Auto-diff a promoted candidate into source, pack, layout, and doc pieces and retest the smallest speed-bearing subset. | Broad wins can include accidental layout movement; minimization keeps the new baseline stable. |
| 312 | Sector-phase payload simulator | Simulate how candidate payload movement changes sector due slots before any emulator run. | Many byte wins did not cross a sector or moved the sector into a worse phase. |
| 313 | Pack-authored scheduler metadata format | Emit small per-pack scheduler hints for safe append groups, refill reservations, and setup coverage. | Generated ownership is the common dependency across B2, W1, B4, and VISITOR3. |
| 314 | Visual-proof thumbnail gate | Store compact host-rendered before/after thumbnails or hashes for every custom codec candidate. | The next big swings alter pixel ownership; fast visual proof prevents wasting full gates on unsafe payloads. |

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
| 5 | Next VISITOR3 pass: build scheduler-owned/generated append timing for additional balanced 16/24-sector candidates without more hot hand tables, or move to generated motion/residual/precomposed pack data. | A read-group win must lower loop or blocking without increasing visible refill; scheduler counters explain why it fired. The current VISITOR3 baseline is high `1063/1040`, blocking `35`, loop-read time `35`, and low `1062/1040`, blocking `42`, loop-read time `42`; v204/v205/v213/v214/v216/v464/v477/v501/v510/v629 prove bounded setup residency and pack-only tail relocation can promote or cut same-loop CD pressure when hidden refill stays at zero, v206/v207/v227 prove generated re-anchors can still remove CD pressure after setup when the payload is placed inside accepted residency, v234/v237/v238/v291/v299/v302 prove unchanged payload relocation can win when semantic rewrites fail, and v248/v249/v292/v338 prove pack-only cleanup/no-op/data aliasing can still cut precursor CD pressure if source layout stays fixed. Setup debt must be justified by active-loop payoff. The wider v205 high probes `261..285` and `269..285`, low setup-prime caps above `208 KiB`, v215 unguarded low second segment, v290 scalar high cap growth, v297 partial frame-131 edge copy, low no-op/empty frame-118 variants, and the v228 frame-127 re-anchor either added hidden refill or regressed visible timing, so remaining VISITOR3 local read-table/setup-cap probes should be treated as exhausted unless generated metadata can add them size-neutrally and deadline-aware. |
| 6 | Generate per-scene/tide setup-prime segments from the current read-plan, capped by heap and active-loop payoff. | Setup primes are segmented and scene-local; no global cap raise or raw broad segment can regress VISITOR3/FISHING1 cadence. |
| 7 | Add selective upload-ready x-band preprocessing for VISITOR3 first, using the matrix's high score plus rect/cap pressure columns and the per-frame hotspot report. | Upload bytes and loop time drop without shifting the FG pack LBA, increasing loop reads, or triggering the current `6` high-tide x-band cap-hit warning; cap-hit frames `114`, `134..136`, and `141..142` should stay on the current full-width path unless the runtime format can avoid the cap. Runtime scratch-packing is rejected; x-band rows need to be pre-contiguous in generated pack data. The default threshold plan is too large for same-footprint append, but the budgeted analyzer target selects `78 / 92` high-tide frames, uses `968904` of `970076` bytes, and retains `4232112` modeled upload bytes saved. Implement that smaller subset first, then try compression or deliberate layout movement only if needed. |
| 8 | Store upload-ready bands only when per-frame payload growth is under a generated threshold. | The direct16 lane avoids WALKSTUF1-style CD pressure where whole-pack expansion cancels compositor savings. |
| 9 | Compress upload-ready bands with a tiny pack-time RLE or residual opcode class. | VISITOR3/BUILDING2/BUILDING4 upload-byte savings survive without large sector growth. |
| 10 | Generate exact restore bands for dirty backdrop repair, separate from upload bands. | Restore bytes fall without hot runtime overlap tests or branch-heavy row walkers. |
| 11 | Done first pass: add rect-count and cap-pressure columns to the preprocessing matrix, not just byte volume. | Candidate upload plans now expose total x-band rects, cap hits, max rects, rects per frame, and exact interval counts before a runtime pack-format probe. |
| 12 | Done first pass: `scripts/analyze-fg2-preprocess-plans.py` now parses FGP3 cleanup/draw payloads and emits per-frame cap/saving hotspot reports, CSV export, pack zero-tail slack, and an exact budgeted subset for a payload ceiling. | Current VISITOR3 high detail shows cap-hit frames `114`, `134..136`, and `141..142` save `0%` under blanket x-band. The default CSV threshold plan selects `92 / 144` frames and estimates `5730024` selected-subset upload bytes saved, but its `2111224` payload+rect bytes do not fit the current `970076` bytes of high-pack zero-tail slack. The current budgeted subset keeps the same-footprint path viable with `78` frames, `968904` payload+rect bytes, and `4232112` modeled upload bytes saved. |
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
| 31 | Extend VISITOR3 motion-copy to low frames `114..118` only. | The promoted `119..123` codec proves motion rows can reduce both reads and blocking; low still has larger residual blocking than high. |
| 32 | Extend VISITOR3 motion-copy to high precursor frames `117..118` only. | High may have one more safe translate cluster before the promoted motion span without touching the later tail phase. |
| 33 | Generate a VISITOR3 motion opcode emitter instead of hand-authored pack surgery. | The next motion experiment should be reproducible and should emit per-row copy, cleanup, and residual bytes from host frame diffs. |
| 34 | Build a VISITOR3 motion eligibility heatmap by frame, row, and signed X delta. | The decoder is only useful where row translation dominates; heatmap evidence should choose frames before emulator time. |
| 35 | Add a motion-copy visual fuzz gate that replays random frame start states. | In-place background moves are stateful, so promotion needs proof beyond the normal linear loop. |
| 36 | Add a VISITOR3 residual dictionary for repeated tiny draw spans after motion-copy. | v181 still carries residual draw bytes; a scene-local dictionary could remove repeated visitor edge/splash fragments. |
| 37 | Add a VISITOR3 cleanup-row coalescer inside motion payloads. | Exposed-background restore spans may be merged or represented as row deltas cheaper than full cleanup commands. |
| 38 | Try a VISITOR3 post-motion precomposed strip for the small exposed edge. | A packed clean strip could replace multiple cleanup spans when the exposed background is static for the cluster. |
| 39 | Split VISITOR3 motion decoder into a cold marker parser plus hot row-copy helper. | The current source stayed in the sector bucket by `noinline`; tighter isolation may reclaim code or speed. |
| 40 | Write a tiny MIPS row-copy helper for VISITOR3 motion spans. | Horizontal copies dominate v181 work and are safe to specialize if C pointer loops remain expensive. |
| 41 | Add a VISITOR3 background-mirror row cache for motion-copy source rows. | If row source lookup/tile checks dominate, per-row tile pointers can remove branchy pixel lookup. |
| 42 | Emit VISITOR3 motion rows sorted by direction and tile locality. | In-place overlapping copies require direction; grouping same direction may improve cache/code path predictability. |
| 43 | Test VISITOR3 frame `124` keyframe reshaping after motion-copy. | v181 shifts the terminal cluster boundary; one downstream keyframe may now be too large or badly phased. |
| 44 | Test VISITOR3 motion-copy plus one-sector early preload for the first residual after frame `123`. | The motion payload may create enough slack to preload the next unavoidable residual without tail-atlas phase debt. |
| 45 | Add a VISITOR3 no-regression control that pins WALKSTUF1 labels to same-commit controls. | The v181 broad run exposed stale-label drift; future gates should not waste time on unrelated baseline mismatch. |
| 46 | Try low-only VISITOR3 motion frame expansion under unchanged high pack. | The low row remains worse; splitting tides avoids high code/data interactions when chasing low-specific frames. |
| 47 | Try high-only VISITOR3 motion frame expansion under unchanged low pack. | High may tolerate one more translated frame and should not wait for low eligibility. |
| 48 | Add a motion-copy pack validator that checks source/destination row overlap legality. | A generated codec needs static guarantees that each copy direction preserves source pixels. |
| 49 | Add a motion-copy size predictor that includes decoder text growth. | v181 nearly crossed the PS-EXE bucket; future pack wins need code-size accounting before runtime gates. |
| 50 | Test direct dirty-row marking from motion-copy spans. | Moving background pixels dirties known rows; marking only moved/restored rows may cut later upload work. |
| 51 | Test VISITOR3 motion-copy with upload-ready x-band emission for the moved strip only. | Full x-band append is unsafe, but the motion delta may expose a smaller owned strip with deterministic pixels. |
| 52 | Build a VISITOR3 phase-aware residual layout within the existing footprint. | Tail duplication failed because it moved payload later; a layout optimizer should prefer earlier sectors for residuals that remain reads. |
| 53 | Add a VISITOR3 per-entry read deadline table generated from v181 CD logs. | Motion-copy changed which reads matter; deadline metadata can target the new blocking cluster rather than stale sectors. |
| 54 | Try a VISITOR3 two-keyframe mini-format for the walking visitor silhouette. | If motion-copy leaves repeated silhouette residuals, a scene-specific keyframe plus XOR/delta may beat PAL4 residual. |
| 55 | Test a VISITOR3 residual transparency mask run-length format. | The remaining residual likely has sparse transparent edges where PAL4 compact spans still carry metadata overhead. |
| 56 | Add a generated "motion plus erase" opcode for rows where the old sprite area is fully clean background. | Copying the moved pixels and erasing the trail can be encoded together instead of separate copy and cleanup commands. |
| 57 | Measure VISITOR3 compose/restore counters per v181 frame. | Frame-level detail should identify whether the next 10% is CD payload, row-copy CPU, cleanup restore, or upload. |
| 58 | Try VISITOR3 motion-copy after an artificial code-padding sweep. | If helper addresses are phase-sensitive, padding may make a larger decoder safe without regressing controls. |
| 59 | Build a cross-scene motion-copy detector but gate only VISITOR3 first. | Other scenes can validate the generator while VISITOR3 remains the only runtime promotion target. |
| 60 | Add a one-command VISITOR3 motion experiment runner. | Repeated pack surgery, visual verification, build bucket check, focused gate, broad gate, CSV refresh, and docs update should be scripted before the next swing. |

## Retest Rules For Old Failures

| Old Failure Class | Retry Only After |
|---|---|
| Raw larger windows | Group metadata plus cost predictor exists. |
| VISITOR3 raw stream windows and standalone groups | Do not retry scalar window sizes; fresh-baseline high/low sweeps failed. The old VISITOR3 high `72..84` row is now removed because setup-prime coverage already owns sectors `1..97`; use coverage checks before adding or keeping local rows. The post-recovery generated groups high `163..175` and low `158..170` both reported `group_hits=0`, with low regressing visible timing. The later high `170..186` larger-group probe is accepted against same-source canaries, but follow-ups around `144..156`, `102..118`, and tight low-tide standalone tables either stayed flat or regressed. The VISITOR3-only strict x-band upload retry also regressed and grew `grDrawBackground`, so runtime scratch-packed x-aware upload stays closed. Continue VISITOR3 from the current allocator-era `1075/1044` high and `1065/1039` low baseline with scene-local/generated metadata, motion-copy expansion, pack data-shape preprocessing without runtime scratch packing, prepared visual ownership, or more allocator-shape work. The v204/v205/v213/v214/v216/v464/v477/v501/v510/v629 setup residency plus the allocator-era low `206..230` third segment, the allocator-era high `228..262` third segment, v227/v234/v237/v238/v291/v299/v302 unchanged-payload relocations, the allocator-era high frame139 and frame56/frame57 no-decode raw-gap relocations, the high-only clean64 retune, and v248/v249/v292/v338 no-op/cleanup residuals/data aliasing show setup-owned residency, pack-only data reduction, and clean-strip shaping can work only when active-loop payoff exceeds setup debt and hidden refill stays flat. The rejected v205 wide high segments, low setup-prime caps above `208 KiB`, v215 unguarded low second segment, v228 frame-127 re-anchor, v230-v233 no-op frame-118 variants, v246/v247 third-segment attempts, v290 scalar high prime growth, v297 partial frame-131 edge copy, and clean-strip neighbors `80 KiB`/`48 KiB` show wider coverage, erased semantics, or over-tight allocator shaping can still lose by adding hidden refill, visible regression, setup cost, heap pressure, or failing before metrics. |
| BUILDING2 raw stream windows | Do not retry scalar window sizes. High regressed all tested sizes, and low's parameter-only `32 KiB` win failed as compiled default source. The current low `603..619` group saved three nominal reads but regressed loop timing `1465 -> 1469` with fixed layout, so standalone tight clusters are closed. The cleaner low `538..550` group improved loop/blocking (`1465 -> 1461`, `334 -> 328`) but failed strict promotion because refill overrun regressed `35 -> 40`; keep it as a scheduler-owned retry candidate, not a standalone table. The cleanup-metadata FGP3/v3 data shape is now accepted for BUILDING2, so future BUILDING2 work should start from `1430/1289` high and `1429/1286` low, not the older FGP3/v1 baseline. Use scheduler-owned grouping or selective preprocessing instead. |
| BUILDING2 local min-slack grouped appends | Do not retry `{538,550}` with a local `minSlackVBlanks=6` guard. It was safe but exact-flat (`1465/1276`, `blocking_vb=334`, `prefetch_overrun_vb=35`) and only grew hot code. Scheduler ownership must be first-class enough to move cadence, not just a per-table guard around the current append path. |
| BUILDING2 high standalone visible-cost groups | Do not retry `96..104`, `66..78`, `325..331`, `538..550`, or adjacent one-off hand tables. `96..104` saved one nominal read but worsened visible blocking/due misses, `66..78` was loop/blocking/read-flat with only a one-VBlank hidden-refill change, and the fresh tail candidate `538..550` stayed exact-flat while growing hot code by `44` bytes. BUILDING2 high needs scheduler-owned timing or selective upload-ready/preprocessed pack data. |
| BUILDING4 high standalone visible-cost groups | Do not retry `{49,65}` as a high-tide one-off source table. It stayed exact-flat at `2985/2774`, `blocking_vb=285`, and `prefetch_overrun_vb=51` while growing/shifting `foregroundPilotPlay` by `+540` bytes. BUILDING4 needs generated scheduler/larger-window ownership or selective preprocessing, not isolated current-window tables. |
| BUILDING5 raw stream windows | Do not retry scalar window sizes. High and low both regressed total loop despite lower read counts; use generated grouping or preprocessing instead. |
| BUILDING-family raw stream windows | BUILDING4 low `36 KiB` is accepted; BUILDING4 low `40/48 KiB`, BUILDING6 `20/28 KiB`, BUILDING4 high `20/28 KiB`, and broad setup-prime are rejected. The v387 low-tide rerun proved the larger fresh-fill shape cuts reads but converts them into visible debt: `48 KiB` drops reads `30 -> 18` while regressing loop `2856 -> 2897` and blocking `44 -> 85`; `40 KiB` drops reads `30 -> 23` while regressing loop `2856 -> 2881` and blocking `44 -> 68`. Retry only scene/tide-locally with generated deadline ownership, pack-side byte reduction, or bounded visible-CD/refill tradeoff rules. |
| BUILDING6 high group `505..517` | Do not retry as a one-off hard-coded group. It fit the existing window and stayed exact-flat, so read-plan rank alone is insufficient for BUILDING6; require generated visible-cost metadata or scheduler-owned grouping first. |
| BUILDING6 `48 KiB` window plus `15..39` group | Do not retry as a larger scalar retained-window probe. The fresh v136 test saved many reads but regressed both visible cadence and hidden refill on both tides, so current BUILDING6 refill placement is scheduler-owned rather than capacity-owned. |
| BUILDING6 pal4 padded/direct FGP3 | Do not retry as direct temporal-residual conversion under current validated packs, with or without layout movement. The size gate expands both packs `1444370 -> 1601445`, and the v154 explicit layout-moving probe regressed high `2520 -> 2618`, blocking `62 -> 283`, and low `2515 -> 2621`, blocking `70 -> 292`. Retry only with a genuinely shrinking selective/keyframed encoder, generated scheduler ownership, or a motion format that proves RAM-mirror/dirty-state safety. |
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
| VISITOR5 paired compact FGP3/v4 | Do not promote under the current setup-prime policy. Both packs fit inside the original `354227` byte footprint, and active loop improves (`1112 -> 1102` high, `1111 -> 1104` low), but full scene cost regresses (`1369 -> 1382`, `1370 -> 1385`) and low-tide CD/refill regresses (`blocking_vb 7 -> 10`, `prefetch_overrun_vb 7 -> 10`). Retry only high-tide-only, setup-prime capped/disabled, or with generated scheduler ownership. |
| BUILDING1 paired compact FGP3/v4 with auto-resident setup-prime | Do not promote. Both packs fit inside the original `318131` byte footprint, and active loops improve to under target (`792 -> 781` high, `794 -> 781` low; `loop_reads 23 -> 8`), but full scene cost regresses (`1028 -> 1040`, `1032 -> 1041`). The no-autoprime variant is promoted as `building1-compact-fgp3-noautoprime-v157`; do not retry the auto-resident version unless setup cost gets an explicit acceptance rule. |
| ACTIVITY9 pal4 padded FGP3 | Done; keep. Both validated wide-stitched ACTIVITY9 packs shrink as FGP3 temporal residuals and fit when padded back to the original `1745484` byte CD footprint. Runtime payload drops `1740180 -> 1453793`; high improves `2185/2049 -> 2101/2056`, low improves `2197/2054 -> 2103/2053`, and the exact matrix rollup moves to `0.8745%` over target / `99.4479%` target speed. |
| ACTIVITY9 low FGP3 read group `624..636` | Done; keep. Under the padded FGP3 data shape, the low-tide grouped append reduces visible CD pressure: `loop_vb 2103 -> 2093`, target accounting `2053 -> 2056`, `blocking_vb 60 -> 43`, `prefetch_overrun_vb 18 -> 14`, and `due_misses 7 -> 5`. That checkpoint later moved to `0.8309%` over target / `99.4779%` target speed after the VISITOR3/WALKSTUF1 setup-prime and VISITOR3 low-group promotions; the current rollup is tracked at the top of this file. |
| BUILDING5 pal4 padded FGP3 | Done; keep. Both current BUILDING5 validated packs shrink as FGP3 temporal residuals and fit when padded back to the original `818670` byte CD footprint. High improves `loop_vb 3359 -> 3343`, `overrun_vb 13 -> 0`, `blocking_vb 20 -> 5`, and `loop_reads 56 -> 41`; low improves `loop_vb 3357 -> 3345`, `overrun_vb 10 -> 0`, `blocking_vb 17 -> 8`, and `loop_reads 56 -> 41`. This checkpoint later moved to `0.8071%` over target / `99.5018%` target speed after the ACTIVITY11 padded-FGP3 promotion; the current rollup is tracked at the top of this file. |
| ACTIVITY11 pal4 padded FGP3 | Done; keep. Both current ACTIVITY11 validated packs shrink as FGP3 temporal residuals and fit when padded back to the original `433970` byte CD footprint. High improves `loop_vb 1729 -> 1715`, `overrun_vb 9 -> 0`, `blocking_vb 10 -> 2`, `prefetch_overrun_vb 4 -> 2`, and `loop_reads 29 -> 11`; low improves `loop_vb 1729 -> 1717`, `overrun_vb 12 -> 0`, `blocking_vb 14 -> 4`, `prefetch_overrun_vb 9 -> 4`, and `loop_reads 29 -> 11`. Full scene setup grows by `8/11` VBlanks, accepted because the active loop now lands under target. This checkpoint later moved to `0.7939%` over target / `99.5149%` target speed after the MARY5 padded-FGP3 promotion; the current rollup is tracked at the top of this file. |
| MARY5 pal4 padded FGP3 | Done; keep. Both current MARY5 validated packs shrink as FGP3 temporal residuals and fit when padded back to the original `646602` byte CD footprint. High improves `loop_vb 1591 -> 1581`, `overrun_vb 9 -> 0`, `blocking_vb 8 -> 5`, `prefetch_overrun_vb 8 -> 0`, and `loop_reads 49 -> 42`; low improves `loop_vb 1592 -> 1581`, `overrun_vb 11 -> 0`, `blocking_vb 10 -> 6`, `prefetch_overrun_vb 10 -> 2`, and `loop_reads 49 -> 42`. This checkpoint later moved to `0.8228%` over target / `99.4872%` target speed after the JOHNNY2/MARY2 stale-row refreshes and MARY2 prefetch relief; the current rollup is tracked at the top of this file. |
| JOHNNY2 pal4 padded FGP3 + clean-pressure relief | Done; keep. Both current JOHNNY2 validated packs shrink as FGP3 temporal residuals and fit when padded back to the original `288637` byte CD footprint. Against the same-commit FGP2 control, high improves `loop_vb 1833 -> 1801`, `overrun_vb 82 -> 50`, and `blocking_vb 401 -> 369`; low improves `1833 -> 1800`, `82 -> 49`, and `399 -> 377`. The follow-up JOHNNY2-local clean-pressure relief preserves `stage1_window` prefetch and moves both tides to `1741/1751`, with blocking `369/377 -> 0`, due misses `144 -> 0`, and loop reads `144 -> 8`. The current rollup is tracked at the top of this file. |
| MARY2 pal4 padded FGP3 + prefetch relief | Done; keep. Both current MARY2 validated packs shrink as FGP3 temporal residuals and fit when padded back to the original `582189` byte CD footprint. Against the same-commit FGP2 control, the FGP3 step improves high `scene_vb 2638 -> 2583`, `loop_vb 2385 -> 2330`, and low `loop_vb 2384 -> 2327`. The follow-up MARY2-local clean-pressure relief restores `stage1_window` prefetch and moves high/low to `2241/2248` and `2242/2250`, with blocking `668/662 -> 2/2` and due misses `233 -> 0`. This checkpoint later moved to `0.7400%` over target / `99.5291%` target speed after the VISITOR3 cleanup-metadata compaction; the current rollup is tracked at the top of this file. |
| VISITOR3 FGP3/v3 cleanup-metadata compaction | Done; keep. Both current VISITOR3 validated packs keep their `1555450`-byte CD footprint and PAL4 draw payloads, but cleanup row/span metadata is compact-u16 encoded. Runtime active payload drops `1552446 -> 1265930`; high improves `1450/1015 -> 1406/1019`, `blocking_vb 355 -> 296`, `prefetch_overrun_vb 14 -> 7`, and `loop_reads 45 -> 40`; low improves `1452/1012 -> 1405/1015`, `blocking_vb 361 -> 301`, `prefetch_overrun_vb 19 -> 8`, and `loop_reads 49 -> 44`. Full compact draw metadata is rejected for now because the extra compositor code moved non-VISITOR canaries. This checkpoint later moved to `0.6781%` over target / `99.5777%` target speed after the BUILDING2 cleanup-metadata compaction; the current rollup is tracked at the top of this file. |
| VISITOR3 pack-only FGP3 padding trim | Do not promote. Trimming both current packs saved `573032` trailing zero bytes but regressed high `1406 -> 1409` and low `1405 -> 1412`; keep the same-layout padded files unless a format change reduces active runtime work. |
| BUILDING2 FGP3/v3 cleanup-metadata compaction | Done; keep. Both current BUILDING2 validated packs keep their `1303332`-byte CD footprint and use compact-u16 cleanup metadata while preserving existing PAL4 draw spans. Active payload drops `1296388 -> 1044638`; high improves `1468/1285 -> 1430/1289`, `blocking_vb 301 -> 212`, `prefetch_overrun_vb 56 -> 20`, and `loop_reads 96 -> 82`; low improves `1465/1276 -> 1429/1286`, `blocking_vb 334 -> 193`, keeps prefetch overrun flat, and cuts `loop_reads 87 -> 68`. This checkpoint later moved to `0.4096%` over target / `99.7860%` target speed after the BUILDING4, ACTIVITY9, JOHNNY2, WALKSTUF1 low-primecap, selector-cleanup, FGP3/v4 draw-metadata, and compact decoder inline promotions; the current rollup is tracked at the top of this file. |
| BUILDING4 FGP3/v3 cleanup-metadata compaction plus stream-window retune | Done; keep. Both current BUILDING4 validated packs keep their `1714154`-byte CD footprint and use compact-u16 cleanup metadata while preserving existing PAL4 draw spans. Active payload drops `1705426 -> 1370198`; high improves `2985/2774 -> 2939/2786`, `blocking_vb 285 -> 240`, `prefetch_overrun_vb 51 -> 27`, and `overrun_vb 211 -> 153`; low improves `2981/2784 -> 2945/2798`, `blocking_vb 199 -> 117`, `prefetch_overrun_vb 119 -> 114`, and `overrun_vb 197 -> 147`. The compact payload needed smaller scene-local windows (`24 -> 20 KiB` high, `36 -> 32 KiB` low) to keep hidden refill under the strict gate. This checkpoint later moved to `0.4096%` over target / `99.7860%` target speed after ACTIVITY9 low cleanup-metadata compaction, JOHNNY2 clean-pressure relief, WALKSTUF1 low-primecap, selector cleanup, FGP3/v4 draw-metadata compaction, and compact decoder inline promotion; the current rollup is tracked at the top of this file. |
| ACTIVITY9 low FGP3/v3 cleanup-metadata compaction | Done; keep as a tide-specific no-new-runtime-code pack-shape win. The paired high/low attempt is rejected because high regressed, but compacting only `ACTV9LOW.FG2` keeps the `1745484`-byte CD footprint, keeps ACTIVITY9 high flat at `2094/2056`, and improves low `2098/2056 -> 2087/2056`, `blocking_vb 47 -> 42`, `prefetch_overrun_vb 19 -> 12`, and active payload `1453793 -> 1196583`. Standard canaries across FISHING1, VISITOR3 high/low, WALKSTUF1 high/low, BUILDING2 high/low, BUILDING4 high/low, BUILDING6 high/low, and ACTIVITY9 high/low passed. This checkpoint later moved to `0.4096%` over target / `99.7860%` target speed after JOHNNY2 clean-pressure relief, WALKSTUF1 low-primecap, selector cleanup, FGP3/v4 draw-metadata compaction, and compact decoder inline promotion; the current rollup is tracked at the top of this file. |
| FGP3/v4 compact draw metadata for current compact residual packs | Done; keep. Current FGP3/v3 compact residual packs now use FGP3/v4 draw row/span metadata, preserving padded CD footprints and LBAs while reducing active metadata. VISITOR3 high/low improve to `1369/1023` and `1376/1023`, BUILDING2 high/low improve to `1405/1298` and `1395/1294`, ACTIVITY9 low improves to `2085/2058`, and the FISHING1 high control remains under target at `1068/1074`. This checkpoint later moved to `0.4096%` over target / `99.7860%` target speed after the compact decoder inline promotion; the current rollup is tracked at the top of this file. |
| Compact FGP3/v4 metadata reader inline | Done; keep. `grReadCompactSpanU16` is now a default inline helper instead of a noinline `-Os` helper. VISITOR3 high/low improve to `1357/1023` and `1361/1023`, BUILDING2 high/low improve to `1394/1301` and `1385/1303`, ACTIVITY9 low remains timing-flat at `2085/2058` with an accepted `blocking_vb 28 -> 29`, and FISHING1 high remains exact-flat under target at `1068/1074`. The current rollup is tracked at the top of this file. |
| VISITOR3 high read-group table prune | Done; keep as code headroom. The remaining high-tide local table (`138..162`, `170..186`, `230..242`) is exhausted after compact-u16 inline and no longer changes runtime metrics. Removing it keeps VISITOR3 high/low, BUILDING2 high/low, ACTIVITY9 low, and FISHING1 high exact-flat while shrinking `foregroundPilotPlay` by `48` bytes. The current rollup is tracked at the top of this file. |
| VISITOR3 restore-minus-current cleanup | Done; keep as pack-side data-shape win. FGP3/v4 cleanup spans now omit intervals redrawn by the same current frame while leaving draw pixels, padded pack size, pack LBAs, and PS-EXE size fixed. VISITOR3 high/low improve to `1139/1024` and `1140/1024`, blocking drops to `191/194`, prefetch overrun drops to `0/0`, active payload drops `981514 -> 737600`, and runtime restore bytes drop `973290 -> 498676`. This checkpoint later moved to `-0.0670%` over target / `100.1368%` target speed after later BUILDING4, BUILDING2, WALKSTUF1, and VISITOR3 draw-clip promotions. |
| BUILDING4 restore-minus-current cleanup | Done; keep as pack-side data-shape win. FGP3/v4 cleanup spans now omit intervals redrawn by the same current frame while leaving draw pixels, padded pack size, pack LBAs, and PS-EXE size fixed. BUILDING4 high/low improve to `2844/2816` and `2855/2815`, blocking drops to `37/46`, active payload drops `1032442 -> 855284`, and runtime restore bytes drop `1229878 -> 546950`. This checkpoint later moved to `-0.0670%` over target / `100.1368%` target speed after later VISITOR3, BUILDING2, and WALKSTUF1 promotions. |
| WALKSTUF1 high setup-prime cap 144 KiB | Superseded by the compact FGP3/v4 WALKSTUF1 baseline. Keep as a historical high-tide-only setup-prime retune, not a shared WALKSTUF1 policy. High improved `1595/1402 -> 1592/1406`, `overrun_vb 193 -> 186`, `blocking_vb 277 -> 275`, `loop_reads 136 -> 134`, and `due_misses 56 -> 55`; low stayed exact-flat at `1604/1407`. |
| VISITOR3 high-only offscreen draw clip | Done; keep as a high-tide-only pack-data trim. The transform preserves `VISITOR3.FG2` size, entry sizes, offsets, pack LBA `22472`, and the `215040` byte PS-EXE bucket, while zeroing clipped draw tails for `17` high-tide entries. High improves `1139/1024 -> 1137/1024`, `overrun_vb 115 -> 113`, and `blocking_vb 191 -> 190`; low and broad controls stay exact-flat. The current rollup is tracked at the top of this file. |
| VISITOR3 low exit-right offscreen draw clip | Done; keep as a low-tide pack-data trim limited to entries `139..143`. The transform preserves `VIST3LOW.FG2` size, entry sizes, offsets, pack LBA `23232`, and the `215040` byte PS-EXE bucket, while zeroing clipped exit-right draw tails. Low improves `1140/1024 -> 1138/1024`, `overrun_vb 116 -> 114`, and `blocking_vb 194 -> 191`; high and broad controls stay exact-flat. Low `ship-left` and combined `ship-and-exit` subsets are rejected because they regress low to `1151/1024`. The current rollup is tracked at the top of this file. |
| BUILDING2 high restore-minus-current cleanup | Done; keep as a high-tide-only pack-side data-shape win. FGP3/v4 cleanup spans now omit intervals redrawn by the same current frame only in `BUILDING2.FG2`, preserving the `1303332` byte CD footprint, pack LBA `6180`, and the `215040` byte PS-EXE bucket. High improves `1394/1301 -> 1353/1311`, `overrun_vb 93 -> 42`, `blocking_vb 138 -> 56`, and `loop_reads 68 -> 62`; low stays on the prior pack because the both-tide transform regressed hidden prefetch overrun `8 -> 13`. The current rollup is tracked at the top of this file. |
| BUILDING2 high read group `60..72` | Done; keep as a high-tide-only retained stream group. The range was the only current scheduler-or-guarded matrix candidate with zero overread and medium visible gaps after the v108 pack pass. It grows `foregroundPilotPlay` by `12` bytes but keeps the `215040` byte PS-EXE bucket and all canary pack LBAs fixed. BUILDING2 high improves `1353/1311 -> 1349/1316`, `overrun_vb 42 -> 33`, `blocking_vb 56 -> 48`, `prefetch_overrun_vb 20 -> 12`, and `loop_reads 62 -> 61`; VISITOR3 high/low, BUILDING2 low, BUILDING4 high/low, ACTIVITY9 low, and FISHING1 high stay exact-flat. The current rollup is tracked at the top of this file. |
| BUILDING2 high read group `249..257` | Done; keep as a same-speed high-tide retained stream group. The v347 row adds `{249,257}` after the accepted `60..72` group, keeps scene/loop/target/overrun flat at `1603`, `1352/1311`, and `41`, and reduces blocking `56 -> 55`, hidden refill `20 -> 19`, loop reads `62 -> 61`, and loop-read VBlanks `266 -> 262`. BUILDING2 low, VISITOR3 low, FISHING1 high, and WALKSTUF1 high/low controls stayed exact-flat. The current rollup is tracked at the top of this file. |
| BUILDING2 high read group `226..242` | Done; keep as the accepted high-tide retained stream group between accepted `60..72` and `249..257`, but use the current-control row at the top of this file for future comparisons. The original v379 row improved scene `1603 -> 1602`, active loop `1352/1311 -> 1351/1313`, overrun `41 -> 38`, blocking `55 -> 50`, hidden refill `19 -> 17`, and loop-read VBlanks `262 -> 254` while preserving `loop_reads=61`, `due_misses=7`, pack LBA/sectors `6181/637`, and the `217088` byte PS-EXE bucket. Later layout changes no longer reproduce that exact `1351/1313` artifact; the current v441 baseline measures `1351/1311`, overrun `40`, blocking `54`, and hidden refill `18`. |
| BUILDING2 high read group `206..230` with 24-sector grouped capacity | Done; keep as the current high-tide retained stream capacity baseline. The first `{206,230}` probe under the old 16-sector capacity was exact-flat, then v441 raised the grouped-read capacity to 24 sectors and kept the row. Focused high improves scene `1603 -> 1602`, active loop `1352/1311 -> 1351/1311`, overrun `41 -> 40`, blocking `55 -> 54`, hidden refill `19 -> 18`, loop reads `61 -> 58`, and loop-read VBlanks `262 -> 257`; due stays `7`, pack LBA/sectors and the PS-EXE bucket stay fixed. BUILDING2 low, WALKSTUF1 high/low, and VISITOR3 low canaries stayed exact-flat. |
| BUILDING2 high read group `226..238` | Do not promote as a hand-authored group. The v348 probe fired and reduced loop reads `61 -> 59`, but all key metrics stayed exact-flat and read time worsened (`loop_read_vb 262 -> 263`, total read VBlanks `402 -> 403`, hidden read time `207 -> 208`). Transaction count alone is not progress for this row; retry only through generated deadline placement or pack-side byte reduction. |
| BUILDING2 high read group `298..322` | Do not promote or retry as a hand-authored table. The v503 focused gate completed with no key movement: scene `1602`, active loop/target `1351/1311`, overrun `40`, blocking `54`, hidden refill `18`, loop reads `58`, and due misses `7`, while hot foreground symbols shifted and `foregroundPilotPlay` grew by `164` bytes. |
| BUILDING2 high read group `319..335` | Do not promote or retry as a hand-authored table. The v838 focused gate fired and saved one read (`58 -> 57`) but regressed scene/loop `1602/1351 -> 1605/1354`, overrun `40 -> 43`, and blocking `54 -> 57`, with refill/due flat at `18/7` and hot symbols shifted by `+208`. The late append still needs generated deadline ownership or pack-side data reduction. |
| BUILDING2 high read group `17..25` | Do not promote as a hand-authored group. The v400 current read-plan refresh marked it medium risk, and the probe saved one read plus hidden refill `17 -> 16`, but it regressed high to scene `1603`, active loop/target `1352/1312`, overrun `40`, and blocking `51`. The early cluster is still phase-negative under the current grouped-append code shape. |
| BUILDING2 high read group `23..29` | Do not promote or retry as a narrower early hand table. The v434 focused gate stayed exact-flat at scene `1603`, `1352/1311`, overrun `41`, blocking `55`, hidden refill `19`, reads/due `61/7`, while shifting tracked hot symbols by `+4`. |
| BUILDING2 high read group `17..33` | Do not promote or retry as a local hand-authored group, including simple min-slack guards. The current v916 retest on the v914 payload baseline proved the row is not green-safe: unguarded saved reads `58 -> 55` and blocking `54 -> 53`, but regressed scene/loop `1602/1351 -> 1607/1356` and overrun `40 -> 45`; slack-4 was exact-flat/inert; slack-2 failed before complete `JCPERF2` at frame `131/334`. Reopen only with generated deadline ownership or code-neutral metadata that proves the read fires in a non-visible slot. |
| BUILDING2 duplicate-payload entry-table aliasing | Do not promote or retry as raw backreferences. Both high and low packs have exact duplicate residual payloads, but v359 full aliasing creates reverse/mixed seek debt and regresses high to `1454/1305` with `blocking_vb=195` and low to `1457/1314` with `blocking_vb=228`. v360's `16 KiB`-limited version still regresses high/low to `1368`/`1369` loop VBlanks, v361's adjacent-only alias is exact-flat, and v507's high-only frame71-to-frame70 adjacent alias is also exact-flat at the current `1602`, `1351/1311`, `blocking_vb=54` baseline. Duplicate compression needs generated forward-order copy/no-op semantics or an explicit runtime alias cache, not reused older offsets in the stream table. |
| BUILDING2 high setup segment `202..226` | Do not promote or retry as a local source segment. The v508 focused gate saved loop reads `58 -> 55`, but regressed to `1619`, `1358/1308`, overrun `50`, blocking `61`, and hidden refill `25`, with hot foreground symbols shifted by `+172` bytes. Setup ownership for the middle cluster needs generated/code-neutral metadata or pack-side work reduction first. |
| WALKSTUF1 low setup-prime prefix-hole payload relocation | Do not promote or retry for prefix holes. The v509 source-neutral pack rewrite moved frames `78` and `80` into the `296`-byte prefix before `data_offset=4392`, kept file size/LBA/source/PS-EXE fixed, but failed structurally with missing `JCPERF2`, invalid byte reads near `0x4209FC4D`, and a clean-retry prefetch drop. Prefix setup-prime sectors are metadata/setup territory, not safe FGP3 payload storage. |
| BUILDING2 low setup segment `67..73` | Do not promote or retry as an additive setup-prime edge segment. The v362 probe made the top low cluster reusable during setup, but it regressed low to `1630`, `1354/1314`, overrun `40`, blocking `132`, and due misses `29` while saving only one loop read. This confirms the `67..73` cluster is phase-sensitive: it cannot be solved by simply moving the read to setup or by the earlier scalar read group. |
| BUILDING2 high read group `83..95` | Do not promote or retry as a hand-authored group. The current v896 retest in v898b saved one read (`58 -> 57`) but regressed scene/loop/target `1602/1351/1311 -> 1609/1358/1310`, overrun `40 -> 48`, blocking `54 -> 63`, and hidden refill `18 -> 26`; pack LBA and the PS-EXE bucket stayed fixed, but the extra table row shifted tracked hot symbols by `+4`. This closes the lower-visible-risk table row on the current baseline. |
| BUILDING2 high read group `210..226` | Do not promote or retry as a hand-authored table, including guarded slack variants. The current v896 retest in v898 saved three loop reads (`58 -> 55`) and read time (`257 -> 250`), but regressed scene/loop `1602/1351 -> 1609/1358`, overrun `40 -> 47`, blocking `54 -> 62`, and hidden refill `18 -> 26`; pack LBA and the PS-EXE bucket stayed fixed, but the extra table row shifted tracked hot symbols by `+4`. The v905-v907 guarded sweep closed the binary alternatives: `minSlack=4` still fired and reproduced the same phase-negative regression, while `minSlack=6` and `8` were exact-flat/inert. The middle cluster needs generated deadline ownership or pack-side data/work reduction, not another local read-table row. |
| BUILDING2 high read group `210..222` | Do not promote or retry as a hand-authored table. The v419 current-baseline retest saved reads (`61 -> 59`) but regressed scene `1602 -> 1607`, active loop `1351 -> 1356`, target `1313 -> 1311`, overrun `38 -> 45`, blocking `50 -> 57`, and prefetch overrun `17 -> 23`. This confirms the middle cluster is phase-negative when forced locally. |
| BUILDING2 high read group `222..238` | Do not promote or retry as an adjacent replacement for the accepted middle group. The v420 probe replaced `{226,242}` with `{222,238}` and regressed high from scene `1602`, active loop/target `1351/1313`, overrun `38`, blocking `50`, hidden refill `17` to scene `1603`, `1352/1311`, overrun `41`, blocking `55`, hidden refill `19`, while reads/due stayed `61/7`. Keep `{226,242}` until a generated scheduler or byte-reduction pass changes the phase model. |
| BUILDING2 high cold pad ±1 sweep | Do not promote or retry as a one-word text phase nudge. The v432/v433 sweep changed the post-compact fgpilot pad from three NOPs to four and two NOPs. Both variants shifted tracked hot symbols by `+4` / `-4` bytes but stayed exact-flat at scene `1603`, `1352/1311`, overrun `41`, blocking `55`, hidden refill `19`, reads/due `61/7`. The lost v379 artifact is not recovered by this immediate pad. |
| BUILDING2 low read group `365..381` | Done; keep as a low-tide-only retained stream group. The range was the top remaining BUILDING2 low row after v109 and passed focused plus broad strict gates despite its partial-overlap/overread risk. It grows `foregroundPilotPlay` by `8` bytes versus v109 but keeps the `215040` byte PS-EXE bucket and all canary pack LBAs fixed. BUILDING2 low improves `1385/1303 -> 1383/1304`, `overrun_vb 82 -> 79`, `blocking_vb 121 -> 118`, `prefetch_overrun_vb 8 -> 5`, `loop_reads 57 -> 55`, and `due_misses 23 -> 22`; VISITOR3 high/low, BUILDING2 high, BUILDING4 high/low, ACTIVITY9 low, and FISHING1 high stay exact-flat. The current rollup is tracked at the top of this file. |
| BUILDING2 low read group `238..250` | Done; keep as the current low-tide retained stream group before accepted `318..330` / `365..381`. The v445 focused row keeps scene/loop flat at `1619/1349`, improves target `1318 -> 1319`, overrun `31 -> 30`, blocking `81 -> 80`, loop reads `55 -> 53`, loop-read VBlanks `230 -> 227`, and due misses `18 -> 17`; hidden refill stays `1`, pack LBA/sectors stay `6818/637`, and BUILDING2 high, WALKSTUF1 high/low, and VISITOR3 low canaries stayed flat. |
| BUILDING2 low read group `67..73` | Do not promote as a local read group. Unguarded reduced blocking `81 -> 76` and loop reads `55 -> 54`, but regressed scene `1619 -> 1621`, loop `1349 -> 1351`, target `1318 -> 1316`, overrun `31 -> 35`, and hidden refill `1 -> 3`; the slack-4 guarded variant was exact-flat and did not fire a useful append. |
| BUILDING2 low read group `67..91` with 24-sector capacity | Do not promote or retry as a hand-authored group. The v443 unguarded row proves the larger capacity can fire and cut loop reads `55 -> 51`, but it regresses scene `1619 -> 1630`, active loop/target `1349/1318 -> 1360/1315`, overrun `31 -> 45`, blocking `81 -> 110`, hidden refill `1 -> 4`, and due misses `18 -> 22`. The v444 `minSlack=4` binary is exact-flat, so the simple choices are phase-negative or inert. This early cluster needs generated refill/deadline ownership or a pack-side byte reduction before retrying. |
| BUILDING2 low read group `250..258` | Do not promote as a hand-authored group. The v400 current read-plan refresh marked it as a standalone long-gap candidate, but the focused probe regressed low to scene `1625`, active loop/target `1355/1317`, overrun `38`, blocking `88`, and hidden refill `2` while only reducing reads `55 -> 54`. This closes another direct low table; remaining low work needs generated refill ownership or pack-side data-shape reduction. |
| BUILDING2 low read group `243..259` | Do not promote or retry as a hand-authored grouped-append table. The v413 current-plan probe looked better than the closed `250..258` row because it covered reads `103..105` and modeled two saved reads, but it actually regressed low to scene `1625`, active loop/target `1355/1320`, overrun `35`, and loop reads `57` while only nudging blocking `81 -> 79` and due misses `18 -> 17`. The v414 `minSlack=4` scheduler guard made the row exact-flat with no work reduction. This cluster needs generated refill ownership or byte/cleanup removal, not another local table. |
| BUILDING2 low read group replacement `314..338` for accepted `318..330` | Do not promote or retry as a hand-authored replacement. The v505 focused gate saved real CD work (`loop_reads 52 -> 50`, loop-read time `221 -> 217`, blocking `70 -> 66`, due `15 -> 13`) but regressed public timing to scene `1623`, active loop/target `1353/1321`, overrun `32`. Keep `{318,330}` until generated deadline/refill ownership or pack-side work reduction changes the phase model. |
| Zero-byte restore perf marker prune | Do not promote as source cleanup. Guarding `ps1PerfMarkRestore(0)` at five residual/direct restore call sites kept VISITOR3 low exact-flat and shifted hot symbols (`foregroundPilotPlay +8`, graphics helpers +40), so it adds phase risk without speed. |
| VISITOR3 motion-aware existing transform scan | No remaining win in the old cleanup/draw-tail transform family. A scratch wrapper that skipped `0xffff` motion markers byte-identically found zero positive VISITOR3 high deltas, zero draw-tail low deltas, and only a low frame `143` restore-minus-current change that grew `2857 -> 2861` bytes. New VISITOR3 attempts need a generated data shape or scheduler-owned metadata, not another pass of these transformers. |
| WALKSTUF1 high second restore-minus-current pass | Do not promote as a pack-only change. The v351 probe reduced restore bytes `725714 -> 725550`, but grew active payload `918345 -> 918551` and regressed high from `1768`, `1480/1432`, overrun `48`, blocking `83`, hidden refill `26`, reads `67`, due `16` to `1784`, `1496/1416`, overrun `80`, blocking `120`, hidden refill `35`, reads `74`, due `19`. |
| BUILDING2 low restore-minus-current retry | Do not promote as a pack-only change. It improves low as far as `1383 -> 1346`, overrun `79 -> 35`, blocking `118 -> 50`, and loop reads `55 -> 52`, but hidden refill regresses `5 -> 13`; temporary setup-prime and stage-guard salvages did not fix that. Retry only with generated scheduler/refill ownership or a second data-shape change that reduces active CD pressure before shortening the render cadence. |
| BUILDING2 low restore-minus-current plus window slack `5` | Do not promote or retry as a local slack guard. It fixes the strict hidden-refill blocker (`prefetch_overrun_vb 5 -> 0`) and improves loop/overrun (`1383/1304 -> 1360/1313`, overrun `79 -> 47`), but it starves active presentation and regresses visible blocking `118 -> 180` plus due misses `22 -> 43`. This proves the low transform needs generated refill placement, not simply fewer low-slack window reads. |
| BUILDING6 `48 KiB` window plus `15..39` read group | Do not promote or retry as a scalar window/group change. It reduced loop reads from `74 -> 32` high and `73 -> 31` low, but regressed high `2520/2442 -> 2568/2443`, blocking `62 -> 115`, hidden refill `64 -> 117`, and low `2515/2437 -> 2565/2445`, blocking `70 -> 115`, hidden refill `66 -> 96`. BUILDING6 needs generated scheduler ownership or a shrinking/selective FGP2 data-shape encoder before another read-count group. |
| VISITOR3 low scoped composite-helper `-Os` | Done; keep as code-shape plus low-tide timing win. `grCompositeToBackground()` shrinks `0xbf4 -> 0x5b0`, `grCompositeToBackgroundFlip()` shrinks `0xc60 -> 0x63c`, and `jcreborn.elf` shrinks `960556 -> 951708` while the PS-EXE bucket remains `215040`. VISITOR3 low improves `1138/1024 -> 1135/1024`, `overrun_vb 114 -> 111`, `blocking_vb 191 -> 184`, and `loop_read_vb 200 -> 194`; VISITOR3 high, BUILDING2 high/low, BUILDING4 high/low, ACTIVITY9 low, and FISHING1 high stay exact-flat. The current rollup is tracked at the top of this file. |
| VISITOR3 v4 draw-tail trim + stage guard | Done; keep as the superseded VISITOR3 baseline that v181 built on. FGP3/v4 zero draw-tail bytes were trimmed while both packs stayed `1555450` bytes and LBAs stayed fixed; high setup-prime residency rose to `232 KiB`, and a VISITOR3-only hidden large-stage guard prevented no-slack prefetch debt. VISITOR3 high improved `1137/1024 -> 1118/1028`, `overrun_vb 113 -> 90`, `blocking_vb 190 -> 150`, `loop_reads 33 -> 27`, and `loop_read_vb 200 -> 153`; low improved `1135/1024 -> 1126/1025`, `111 -> 101`, `184 -> 170`, `33 -> 31`, and `194 -> 179`. Both kept `prefetch_overrun_vb=0`, and BUILDING2 high/low, BUILDING4 high/low, ACTIVITY9 low, and FISHING1 high stayed exact-flat. The current rollup is tracked at the top of this file. |
| WALKSTUF1 compact FGP3/v4 restore-minus-current | Done; keep as the current WALKSTUF1 baseline. Both PAL4/FGP2 packs are compacted into FGP3/v4 restore-minus-current packs and padded back to the original `1535263` byte footprint, preserving pack LBAs and the `215040` byte PS-EXE bucket. High improves `1592/1406 -> 1491/1426`, `overrun_vb 186 -> 65`, `blocking_vb 275 -> 85`, `prefetch_overrun_vb 51 -> 32`, `loop_reads 134 -> 69`, and `due_misses 55 -> 13`; low improves `1604/1407 -> 1489/1427`, `197 -> 62`, `270 -> 86`, `55 -> 27`, `132 -> 69`, and `50 -> 12`. The broad non-WALKSTUF controls stayed exact-flat except VISITOR3 high; that same drift reproduced with original WALKSTUF1 FGP2 packs restored, so it is tracked as unrelated current-control drift. The current rollup is tracked at the top of this file. |
| WALKSTUF1 high frame `109` motion-copy | Do not promote under the current opcode/helper shape. Sparse-in-place high-only motion saved `3139` active payload bytes but was exact-flat (`1779`, `1490/1424`, overrun `66`, blocking `92`, hidden `31`, reads `69`); the C row-copy helper crossed the PS-EXE bucket and shifted LBA; the non-sparse repack improved hidden refill `31 -> 26` but regressed public timing to `1784`, `1495/1427`, overrun `68`, and reads `73`. Retry only as zero-runtime precomposed data, generated phase-safe layout search, or a cost-modeled multi-frame codec. |
| WALKSTUF1 high read group `201..213` | Done; keep as the retained-read prerequisite for the current WALKSTUF1 high baseline. The high-tide-only retained stream read group promotes because it improves `scene_vb 1779 -> 1777`, active loop `1490/1424 -> 1488/1426`, overrun `66 -> 62`, hidden refill `31 -> 27`, and loop reads `69 -> 68` while keeping blocking `92`, pack LBA `24745`, pack sectors `750`, and the `217088` byte PS-EXE bucket fixed. Same-branch broad controls for WALKSTUF1 low, VISITOR3 high/low, BUILDING2 high/low, ACTIVITY9 low, FISHING1 high, JOHNNY1 high, and MARY3 high/low stayed exact-flat. |
| WALKSTUF1 high gap1 pack + 3-VBlank window-prefetch guard | Done; keep as the current WALKSTUF1 high baseline. The v288 promotion combines the gap1 early-prefix cleanup pack with the accepted `201..213` high read group and a high-tide-only guard that skips speculative window prefetch when held slack is only the default 3 VBlanks. It improves high `scene_vb 1777 -> 1766`, active loop `1488/1426 -> 1477/1431`, overrun `62 -> 46`, blocking `92 -> 90`, hidden refill `27 -> 19`, and loop-read time `309 -> 285`; due misses rise `14 -> 17`, but WALKSTUF1 low, VISITOR3 high/low, BUILDING2 high/low, ACTIVITY9 low, FISHING1 high, JOHNNY1 high, and MARY3 high/low stay flat in the broad gate. Public rollup moves to `+0.3336%` over target / `99.6734%` target speed. |
| WALKSTUF1 high prefix gap2 after v288 | Do not promote or retry as cleanup-gap threshold escalation. The v306 `max_gap=2` prefix pass saved only another `522` bytes after the accepted gap1 pack, but regressed high from `1766`, `1477/1431`, overrun `46`, blocking `90`, hidden refill `19`, loop-read VBlanks `285`, due `17` to `1783`, `1494/1418`, overrun `76`, blocking `98`, hidden refill `34`, loop-read VBlanks `326`, due `15`. Higher prefix gap thresholds are now closed for high tide; keep gap1 only with the v288 guard and use semantic placement, precomputed strip data, or generated scheduler ownership for the remaining read clusters. |
| WALKSTUF1 high prefix gap2 preserve-offsets | Do not promote as a work-reduction pack. The v309 preserve-offsets variant kept all CD placement fixed and avoided the v306 cadence regression, but stayed exact-flat on every key metric (`1766`, `1477/1431`, overrun `46`, blocking `90`, prefetch overrun `19`, reads `69`, due `17`). The extra `522` bytes and `162` spans saved are safe but not measurable. |
| WALKSTUF1 high active-loop gap preserve `140..170` | Do not promote as a hot-cluster cleanup-gap pass. The v310 preserve-offset probes on entries `140..170` saved `1400` bytes at gap1 and `2012` bytes at gap2 without moving pack offsets, but both produced the same public result: scene/loop stayed `1766/1477`, target fell `1431 -> 1430`, overrun worsened `46 -> 47`, blocking improved `90 -> 85`, due misses improved `17 -> 16`, and prefetch overrun worsened `19 -> 21`. The cluster needs semantic/precomposed data or generated deadline ownership, not more cleanup-span merging. |
| WALKSTUF1 high read group `329..345` after v288 | Do not promote or retry as a direct retained-read group. The v307 current-baseline retest cut CD pressure (`loop_reads 69 -> 66`, `blocking_vb 90 -> 84`, `due_misses 17 -> 15`) but did not improve loop time and worsened public cadence by dropping target `1431 -> 1428` and raising prefetch overrun `19 -> 22`; `minSlack=4` and `minSlack=8` both exact-flat/no-op. This sector neighborhood needs generated deadline ownership or pack/data-shape reduction, not another hand group. |
| WALKSTUF1 high read group `298..314` after v288 | Do not promote or retry as a direct retained-read group. The v311 no-threshold probe reduced reads `69 -> 66`, blocking `90 -> 83`, and due misses `17 -> 13`, but regressed high to `1774`, `1485/1426`, overrun `59`, and prefetch overrun `26`. `minSlack=3` and `minSlack=4` exact-flat/no-op, proving the only firing opportunities are low-slack and phase-negative. This closes the `286..322` hand-group lane unless generated deadline ownership can reserve refill time. |
| WALKSTUF1 high setup segment `75..92` / `74..98` | Do not promote or retry as transient setup-scratch residency. The v312 `75..92` segment reduced active reads `69 -> 67` and prefetch overrun `19 -> 16`, but regressed public timing: `scene_vb 1766 -> 1778`, loop `1477 -> 1479`, overrun `46 -> 48`, blocking `90 -> 97`, hits `199 -> 197`, and due misses `17 -> 19`. The wider `74..98` form failed before `JCPERF2`. Early-cluster residency needs inter-scene preload or offset-stable generated resident placement, not a larger transient scratch segment. |
| WALKSTUF1 high offscreen span clipping | Do not promote or retry as naive FGP3/v4 cleanup/draw clipping. The v313 full offset-preserved clip cut active payload `918345 -> 814617`, loop reads `69 -> 62`, and blocking `90 -> 83`, but regressed public timing to `1780`, `1478/1425`, overrun `53`, and prefetch overrun `22`. Draw-only, cleanup-only, active-cluster-only, and active-cluster plus `5`-VBlank slack variants all regressed; the slack pair merely traded refill debt for blocking (`90 -> 151`). Future offscreen-work removal needs generated phase ownership or setup/target-cadence preservation, not smaller entry sizes alone. |
| WALKSTUF1 high read group `213..229` plus high-tide slack4 | Done; keep as the current high baseline. The v316 promotion extends the accepted `{201,213}` group with `{213,229}` and uses a high-tide-only 4-VBlank no-slack guard. Against the current-layout no-candidate control it improves high `scene_vb 1770 -> 1768`, active loop `1482/1429 -> 1480/1429`, overrun `53 -> 51`, blocking `102 -> 85`, loop reads `70 -> 69`, and due misses `19 -> 16`; hidden refill regresses `16 -> 26` as an accepted tradeoff because public loop and blocking improve. The same-layout canaries stayed flat. |
| WALKSTUF1 low gap1 prefix pack | Done; keep as the prerequisite for the current WALKSTUF1 low baseline. The v289 pack-only promotion applies the same one-pixel cleanup-gap merge to low-tide frames `0..37`, changing `29` frames, saving `1126` active payload bytes, and preserving the `1535263` byte padded footprint. Focused and broad gates improve low `scene_vb 1779 -> 1770`, active loop `1487/1424 -> 1478/1428`, overrun `63 -> 50`, blocking `95 -> 75`, and due misses `15 -> 13`; hidden refill stays `25`, loop reads stay `69`, pack LBA stays `25495`, and WALKSTUF1 high plus VISITOR3 high/low, BUILDING2 high/low, ACTIVITY9 low, FISHING1 high, JOHNNY1 high, and MARY3 high/low stay flat. This is superseded in the rollup by the later WALKSTUF1, VISITOR3, BUILDING6, and BUILDING2 promotions; current rollup is tracked at the top of this file. |
| WALKSTUF1 low prefix gap6 compaction alone | Do not promote alone as a larger cleanup-gap threshold. Gap6 moved frame `39` fully into paid setup-prime coverage and improved CD pressure (`blocking_vb 75 -> 70`, `loop_reads 69 -> 67`, `due_misses 13 -> 11`), but without slack guarding the extra cleanup geometry regressed `scene_vb 1770 -> 1774`, active loop `1478/1428 -> 1482/1426`, overrun `50 -> 56`, and hidden refill `25 -> 31`. Keep only as the pack half of v305, where the low slack guard repays the hidden-refill debt. |
| WALKSTUF1 low prefix gap6 pack + scene-wide slack guard | Done; keep as the current WALKSTUF1 low baseline. The v305 promotion reuses the gap6 prefix pack from the failed v304 probe, then removes the high-tide-only condition from the WALKSTUF1 3-VBlank speculative window-prefetch guard so low tide also skips no-slack window attempts. Focused and broad gates improve low `scene_vb 1770 -> 1767`, active loop `1478/1428 -> 1475/1430`, overrun `50 -> 45`, blocking `75 -> 67`, hidden refill `25 -> 21`, loop reads `69 -> 66`, loop-read time `301 -> 282`, and due misses `13 -> 12`; WALKSTUF1 high, VISITOR3 high/low, BUILDING2 high/low, ACTIVITY9 low, FISHING1 high, JOHNNY1 high, and MARY3 high/low stay flat. Public rollup is superseded by later WALKSTUF1, VISITOR3, BUILDING6, and BUILDING2 work; current rollup is tracked at the top of this file. |
| WALKSTUF1 low speculative window slack `4` / `5` | Do not promote or retry as scalar wait-threshold tuning. The v410/v411 current-baseline probes raised low's guard from the accepted `3` VBlanks to `4` and `5`; both stayed exact-flat at `1776`, `1484/1431`, overrun `53`, blocking `72`, refill `22`, reads/due `67/12`, with only source/code-shape churn. Low needs generated deadline ownership or pack/data-shape movement before this guard becomes useful. |
| WALKSTUF1 low staged prepare before speculative window refill | Done; keep as the current WALKSTUF1 low scheduler baseline. The v331 source-only scheduler fallback lets low tide run `fgRuntimePrepareStagedFrameForPresent()` at the 4-VBlank held-frame slack point even when a speculative window prefetch would otherwise own the CD slot. Focused baseline and bounded controls improve low public overrun `54 -> 53`, blocking `74 -> 72`, prefetch overrun `24 -> 22`, and loop-read VBlanks `290 -> 288`; target moves `1429 -> 1431` while scene/loop move `1775/1483 -> 1776/1484`, so it is explicitly tracked as a bounded public-pressure tradeoff. Due misses stay `12`, the PS-EXE bucket stays `217088`, and WALKSTUF1 high, BUILDING2 high/low, VISITOR3 low, and FISHING1 high controls stay exact-flat. |
| WALKSTUF1 low offscreen draw clipping and setup-prime tail hole fill | Do not promote or retry as naive zero-runtime pack edits. The v378 broad size-preserving draw-clip pass removed `179604` offscreen pixels and `23012` draw spans but regressed low to `1780`, `1488/1426`, overrun `62`, blocking `86`, hidden refill `30`, reads `71`, and due `14`. A three-source in-prime clone (`37:57`, `35:190`, `33:215`) was worse (`1791`, `1499/1421`, blocking `113`, hidden `37`, due `16`), and the minimal `35:190` clone still regressed (`1780`, `1488/1427`, blocking `91`, due `14`). Future low work needs generated phase ownership, sector-boundary frame splitting, or a custom representation that preserves scheduler cadence. |
| WALKSTUF1 low read group `285..309` current-baseline retest | Do not promote or retry as a hand-authored retained table. The v380 focused retest fired and saved reads (`67 -> 63`) plus due misses (`12 -> 11`), but it regressed low from `1776`, `1484/1431`, overrun `53`, blocking `72`, hidden refill `22` to `1782`, `1490/1433`, overrun `57`, blocking `76`, hidden refill `26`. The late cluster remains phase-negative even on the newer baseline; use generated deadline ownership, sector-boundary frame planning, or a custom data format instead. |
| WALKSTUF1 low read group `285..297` after v428/current | Do not promote or retry as a shared/direct or low-only retained table. The v429 focused gate used the v428 shared dual-tail baseline and saved reads (`66 -> 64`), but regressed low to scene `1784`, active loop/target `1492/1430`, overrun `62`, blocking `81`, hidden refill `28`, and due misses `12`. The v504 current-baseline low-only split saved one read (`64 -> 63`) but still regressed to `1778`, `1486/1431`, overrun `55`, blocking `75`, hidden refill `26`. This closes the smaller mid-cluster form; the cluster now needs generated deadline ownership, byte-removing pack work, or a custom representation. |
| WALKSTUF1 low read group `291..297` after v428 | Do not promote or retry as a shared/direct retained table. The v430 focused gate narrowed v429 to the smallest fireable row, but stayed exact-flat against v428: `1776`, `1484/1431`, overrun `53`, blocking `72`, hidden refill `22`, reads `66`, and due misses `12`. It adds source/data shape without a measurable work win. |
| WALKSTUF1 low read group `303..309` after v428 | Do not promote or retry as a shared/direct retained table. The v431 focused gate tested the later mid-cluster fire point and also stayed exact-flat against v428: `1776`, `1484/1431`, overrun `53`, blocking `72`, hidden refill `22`, reads `66`, and due misses `12`. Direct rows in this subcluster are inert unless scheduler ownership changes. |
| WALKSTUF1 low read group `78..90` | Do not promote or retry as a narrowed early hand table. The v412/v472 current-baseline probes kept pack LBA plus EXE bucket fixed and reduced due misses, but regressed low timing and CD pressure. The accepted successor is the v474 boundary-fixed `78..91` form, which owns the overlapping sector-89 follow-up and is tracked at the top of this file. |
| WALKSTUF1 low read group `273..297` | Do not promote or retry as a direct retained table. The v416 current-baseline probe was the remaining large medium-risk low cluster and did save reads (`67 -> 64`), but it regressed scene `1776 -> 1782`, active loop `1484 -> 1490`, overrun `53 -> 59`, blocking `72 -> 80`, and prefetch overrun `22 -> 26` with fixed pack LBA and EXE bucket. This closes the last plausible large low hand table; future low work needs generated deadline ownership, byte-removing pack transforms, or a custom representation that preserves cadence. |
| WALKSTUF1 low boundary-frame `gap6` prefix pack | Do not promote or retry as a cleanup-gap pack move. The v417 f38/f39-only variant moved `sound_events_offset 920531 -> 920087` inside the fixed `1535263` byte pack and kept LBA stable, but it regressed low to scene `1780`, active loop/target `1488/1429`, overrun `59`, blocking `78`, and prefetch overrun `29` while reads stayed `67`. Narrow prefix cleanup is still phase-negative; future low pack work must remove bytes while preserving CD cadence or change the runtime representation. |
| WALKSTUF1 low scratch-copy narrow upload | Do not promote or retry as a runtime upload path. The v418 scene-gated X-band uploader grew the PS-EXE bucket by one sector and failed before `JCPERF2` even with layout changes allowed; the v623 generic staged x-band retry repeated the same structural failure at `217088 -> 219136` and also timed out before `JCPERF2`. The current BUILDING2-high generic scratch-pack retry repeated the same class at `233472 -> 235520`, reaching only frame `132` before missing `JCPERF2`. The copy-to-scratch cost overwhelms any narrower DMA benefit; upload-byte work must be prepacked/upload-ready or avoided before runtime. |
| WALKSTUF1 low read group `297..313` current-baseline retest | Do not promote or retry as a direct retained table. The v394 focused gate proved the high-upside cluster fires, cutting reads `67 -> 66` and due misses `12 -> 11`, but it regressed scene `1776 -> 1778`, active loop/target `1484/1431 -> 1486/1430`, overrun `53 -> 56`, blocking `72 -> 73`, and hidden refill `22 -> 25` with fixed LBA and fixed PS-EXE bucket. This reinforces that tight-gap low clusters need generated deadline ownership or pack/data-shape changes, not more source-table append groups. |
| WALKSTUF1 low read group `297..321` current-baseline retest | Do not promote or retry as a direct retained table. The v399 24-sector version saved more reads when unguarded (`67 -> 63`) but regressed scene/loop to `1782/1490`, overrun `59`, blocking `87`, hidden refill `23`, and due misses `13`. Slack6 and slack8 guards were exact-flat and produced no key work-volume win. This closes larger scalar grouped appends over the late `297..321` cluster; future work needs generated deadline ownership or pack-side byte removal. |
| WALKSTUF1 low late direct-stage/window ownership | Do not promote or retry as a hand-coded scene branch. The v402 direct-stage-deny branch saved reads `67 -> 65` but regressed target `1431 -> 1429`, blocking `72 -> 76`, and hidden refill `22 -> 27`; the binary opposite, forcing direct-stage for sectors `297..321`, kept scene/loop flat but still regressed target `1431 -> 1429`, blocking `72 -> 73`, and hidden refill `22 -> 24`. This closes manual direct-stage toggles for the late cluster; use generated reservation metadata or pack/data-shape work instead. |
| WALKSTUF1 low sector-aligned entries `145..159` | Do not promote or retry as brute padding. The v403 same-footprint pack layout inserted `18908` bytes of zero padding to sector-align hot entries and moved `sound_events_offset 920531 -> 939439`. It saved one loop read (`67 -> 66`) but regressed low to scene `1780`, active loop/target `1488/1427`, overrun `61`, blocking `102`, and due misses `18`. Padding the hot cluster improves neither phase nor CD pressure; future pack work must remove bytes or preserve CD-head locality. |
| WALKSTUF1 low direct-stage deny `285..297` | Do not promote or retry as a hand-coded ownership branch. The v404 smaller-window probe saved reads `67 -> 65`, but regressed low to scene `1784`, active loop/target `1492/1430`, overrun `62`, blocking `81`, and hidden refill `28`. This closes manual direct-stage-deny for the medium-risk mid cluster as well as the late cluster. |
| WALKSTUF1 low read group `190..202` | Do not promote or retry as a direct retained-read table. The v381 focused gate used the matching baseline label and still rejected it: low regressed from `1776`, `1484/1431`, overrun `53`, blocking `72`, hidden refill `22`, reads `67`, due `12` to `1784`, `1492/1423`, overrun `69`, blocking `90`, hidden refill `34`, reads `70`, due `13`. This closes the prior setup-segment signal in direct-table form too. |
| WALKSTUF1 high read group `344..360` after v316 | Done; keep as a prerequisite for the current WALKSTUF1 high baseline. The v340 source-only promotion adds `{344,360}` after the accepted `{201,213}` and `{213,229}` high groups. Focused gate keeps scene/loop flat at `1768/1480`, moves target `1429 -> 1432`, overrun `51 -> 48`, blocking `85 -> 83`, loop reads `69 -> 67`, loop-read time `301 -> 292`, due misses `16`, and prefetch overrun `26`; WALKSTUF1 low plus BUILDING2 high/low, VISITOR3 low, and FISHING1 high controls stayed flat. Later BUILDING6, BUILDING2, and v383 WALKSTUF1 work supersede its rollup; current rollup is tracked at the top of this file. |
| WALKSTUF1 high read group `422..434` after v340 | Done; keep as a prerequisite for the current WALKSTUF1 shared CD-work baseline. The v383 source-only promotion appends `{422,434}` after the accepted `{201,213}`, `{213,229}`, and `{344,360}` high groups. Focused gate keeps timing exact-flat at scene `1768`, active loop `1480/1432`, overrun `48`, blocking `83`, hidden refill `26`, and due misses `16`, while reducing loop reads `67 -> 66` and loop-read VBlanks `292 -> 286`. WALKSTUF1 low, BUILDING2 high, FISHING1 high, and VISITOR3 low controls stayed flat. Current rollup is tracked at the top of this file; this is a lower-CD-pressure baseline, not a VBlank speed win. |
| WALKSTUF1 high read group `156..164` | Do not promote or retry as a hand-authored table. The v901 current-baseline probe kept pack LBA and the PS-EXE bucket fixed but regressed scene/loop/target `1764/1476/1434 -> 1774/1486/1427`, overrun `42 -> 59`, blocking `81 -> 102`, and due `16 -> 17`; loop reads stayed `63`. The refreshed planner's low-visible-risk label is not sufficient without scheduler-owned refill/deadline metadata. |
| WALKSTUF1 high read group `238..246` | Do not promote or retry as a hand-authored table. The v902 companion probe kept pack LBA and the PS-EXE bucket fixed but regressed scene/loop/target `1764/1476/1434 -> 1778/1490/1420`, overrun `42 -> 70`, blocking `81 -> 111`, hidden refill `23 -> 32`, due `16 -> 19`, and loop reads `63 -> 66`. The remaining W1-high mid-pack read-count signal needs generated metadata or a data-shape change first. |
| WALKSTUF1 high read group `304..310` | Do not promote or retry as a hand-authored suffix table. The v904 current-baseline probe kept pack LBA and the PS-EXE bucket fixed but stayed exact-flat at `1764`, `1476/1434`, overrun `42`, blocking/refill `81/23`, reads/read VBlanks `63/276`, and due `16`, so the strict gate failed for no key improvement. This closes the remaining scalar suffix row; generated scheduler ownership or pack/render data changes are required before retrying this cluster. |
| WALKSTUF1 high read group `444..456` after v383 | Done; keep as a prerequisite for the current WALKSTUF1 shared CD-work baseline. The v384 source-only promotion appends `{444,456}` after the accepted high groups through `{422,434}`. Focused gate keeps timing exact-flat at scene `1768`, active loop `1480/1432`, overrun `48`, blocking `83`, hidden refill `26`, and due misses `16`, while reducing loop reads `66 -> 65`, loop-read VBlanks `286 -> 284`, and hidden read time `203 -> 201`. WALKSTUF1 low, BUILDING2 high, FISHING1 high, and VISITOR3 low controls stayed flat. Current rollup is tracked at the top of this file; this is another lower-CD-pressure baseline, not a VBlank speed win. |
| WALKSTUF1 shared low/high tail read groups `427..443` / `443..455` / `444..456` | Done; keep as the current WALKSTUF1 low CD-work baseline. The v428 promotion adds `{443,455}` beside the accepted high `{444,456}` in the shared WALKSTUF1 table; v598 adds the shared `{427,443}` row after low-only testing proved it was timing-flat. Current low timing stays `1770`, `1478/1431`, overrun `47`, blocking/refill `64/20`, and due misses `11`, while loop reads drop `64 -> 62` and loop-read VBlanks `286 -> 281`. High stays `1764`, `1476/1434`, overrun `42`, blocking/refill `81/23`, reads/due `65/16`; VISITOR3 high/low and BUILDING2 high/low controls stay flat. Current public rollup is tracked at the top of this file; this is a CD-work win, not a VBlank speed win. |
| WALKSTUF1 high adjacent read-group merge `{201,213}` + `{213,229}` -> `{201,225}` | Do not promote or retry as a table-shape simplification. The v506 focused gate regressed high from `1764`, `1476/1434`, overrun `42`, blocking `81`, refill `23`, reads/due `65/16` to `1782`, `1494/1427`, overrun `67`, blocking `108`, refill `23`, reads/due `65/18`. Keep the accepted split rows; merging loses the useful phase/coverage of the second row. |
| WALKSTUF1 high read group `268..280` after v384 | Do not promote or retry as a hand-authored retained table. The v385 focused probe looked clean in the read plan but regressed the current high baseline from scene `1768`, active loop `1480/1432`, overrun `48`, blocking `83`, hidden refill `26`, reads `65`, due `16` to scene `1772`, active loop `1484/1424`, overrun `60`, blocking `98`, hidden refill `27`, reads `66`, due `17`. This closes the remaining low-visible-risk mid-pack direct table; future high-side attempts need generated deadline ownership or pack-side byte/geometry removal. |
| WALKSTUF1 high read group `287..311` plus shifted follow-ups | Do not promote as a plain retained table. The v436 direct and slack4 variants saved CD work (`reads/due 65/16 -> 63/15`, refill `26 -> 24`) but regressed visible blocking `83 -> 84` with no public speed gain. The `309..325` replacement was exact-flat, and stacking `312..324` regressed to `1776`, `1488/1433`, overrun `55`, blocking `97`. This lane needs generated deadline ownership or data-shape removal before retrying. |
| WALKSTUF1 high read group `238..246` after v384 | Do not promote or retry as a hand-authored retained table. The v395 focused probe kept layout fixed but regressed high from scene `1768`, active loop `1480/1432`, overrun `48`, blocking `83`, hidden refill `26`, reads `65`, due `16` to scene `1778`, active loop `1490/1420`, overrun `70`, blocking `111`, hidden refill `32`, reads `68`, due `19`. Mid-pack high rows are now closed unless generated deadline ownership or pack-side data-shape changes alter the scheduler phase. |
| WALKSTUF1 high read group `80..92` after v428 | Do not promote or retry as an early direct table. The v435 focused gate regressed high from `1768`, `1480/1432`, overrun `48`, blocking `83`, refill `26`, reads/due `65/16` to `1788`, `1500/1425`, overrun `75`, blocking `105`, refill `26`, reads/due `66/15`. The early cluster needs generated deadline ownership or pack/data-shape work, not scalar grouping. |
| WALKSTUF1 high read group `360..376` after v340 | Do not promote or retry as a direct high table. The v368 follow-up reduced loop reads `67 -> 65` and loop-read VBlanks `292 -> 289`, but regressed scene `1768 -> 1774`, active loop `1480 -> 1486`, target `1432 -> 1431`, overrun `48 -> 55`, blocking `83 -> 87`, and hidden refill `26 -> 28`. The late read saving is real but costs more cadence than it removes. |
| WALKSTUF1 high window `53 KiB` / `55 KiB` current-baseline retune | Do not promote adjacent scalar window sizes. The v382 parameter-only probes showed both sides of the accepted `54 KiB` high window are phase-negative on the current layout: `53 KiB` regressed to `1820`, `1531/1434`, overrun `97`, blocking `115`, hidden refill `46`; `55 KiB` regressed to `1828`, `1539/1439`, overrun `100`, blocking `108`, hidden refill `49`. Lower read count alone is not progress when the active-loop cadence collapses. |
| WALKSTUF1 low window `36 KiB` / `44 KiB` current-baseline retune | Do not promote adjacent scalar window sizes. The v396 parameter-only retry shows the accepted `40 KiB` low window remains the current knee: `44 KiB` cut loop reads `67 -> 22` and due misses `12 -> 5`, but regressed scene `1776 -> 1827`, active loop/target `1484/1431 -> 1533/1436`, overrun `53 -> 97`, blocking `72 -> 106`, and hidden refill `22 -> 52`; `36 KiB` was killed before `JCPERF2` while still issuing CD reads. Current low-window work is closed unless generated scheduler ownership changes when larger windows are allowed to read. |
| BUILDING2 low read group `210..222` | Do not promote or retry as a hand-authored retained table. The v386 unguarded and `minSlack=4` probes both saved reads (`55 -> 53`) and one due miss, but regressed public timing from scene `1619`, active loop `1349/1318`, overrun `31`, blocking `81`, hidden refill `1` to scene `1621`, active loop `1351/1317`, overrun `34`, blocking `81`, hidden refill `3`. This closes another medium-risk direct-group row; BUILDING2 low needs generated refill ownership or pack-side byte/cleanup reduction. |
| WALKSTUF1 high read group `156..164` | Do not promote or retry as a hand-authored high table. The v346 focused gate regressed high from the v340 baseline: scene `1768 -> 1774`, active loop `1480 -> 1486`, target `1432 -> 1427`, overrun `48 -> 59`, blocking `83 -> 102`, and due misses `16 -> 17`; hidden refill improved only `26 -> 23`, and loop reads stayed `67`. This closes the highest-ranked remaining current-compatible scalar high row; future high work needs generated deadline ownership or pack-side payload removal. |
| WALKSTUF1 low read group `371..387` | Do not promote or retry as a hand table. The v341 focused gate proved the group fires and cuts loop reads `67 -> 65`, but it regresses public playback: scene `1776 -> 1780`, active loop `1484 -> 1488`, overrun `53 -> 56`, blocking `72 -> 75`, and hidden refill `22 -> 26`; due misses stay `12`. This closes the best remaining scalar low-table candidate after v340. WALKSTUF1 low needs generated deadline ownership or pack/data-shape work, not more hand-authored retained groups. |
| WALKSTUF1 low frame `188` resident slot swap through frame `39` | Do not promote or retry tail-evicting resident swaps. The v342 pack-only probe moved late frame `188` from sectors `383..387` into frame `39`'s setup-prime slot at `75..78`, then moved frame `39` to padded tail sectors `449..454`. Fixed pack bytes and LBA were preserved, but low regressed scene `1776 -> 1800`, active loop `1484 -> 1508`, overrun `53 -> 79`, blocking `72 -> 119`, hidden refill `22 -> 29`, loop reads `67 -> 68`, and due misses `12 -> 17`. Future resident work must shrink/alias enough bytes to keep evicted early frames local; padded-tail eviction creates a worse far-seek pattern than the late read it removes. |
| WALKSTUF1 low setup-segment sweep `371..387` / `190..202` | Do not promote or retry as hand-owned setup residency. The v343 original-order `371..387` segment improved loop/overrun `1484 -> 1480` and `53 -> 50`, but failed strict pressure by regressing blocking `72 -> 76`, hidden refill `22 -> 24`, setup cost, and due misses `12 -> 13`. The v344 before-window ordering regressed loop/overrun/blocking to `1485`, `56`, and `80` despite one fewer read, and the v345 `190..202` segment had no key improvement (`1486`, `57`, `79`, hidden `24`, due `13`). Future low residency must avoid extra setup reads and preserve early locality, or be generated deadline-owned rather than another scalar segment. |
| WALKSTUF1 low read groups `83..91`, `155..163`, and `237..245` | Do not promote or retry as hand-authored retained-read tables. The v355 combined table was the only apparent speed hint (`1776/1484/53 -> 1774/1482/51`), but it failed by adding pressure (`blocking_vb 72 -> 76`, `prefetch_overrun_vb 22 -> 24`, `loop_reads 67 -> 68`, `due_misses 12 -> 13`). The isolated v356-v358 probes regressed outright: `155..163` to `1783`, `1491/1423`, blocking `101`; `83..91` to `1784`, `1492/1429`, blocking `86`; and `237..245` to `1784`, `1492/1428`, blocking `80`. This closes current low-risk scalar grouping after v354; future WALKSTUF1 low work must change data shape, metadata ownership, or deadline phase. |
| WALKSTUF1 low read group `201..213` | Do not promote as a symmetric high/low table. The v278 focused low gate proved the candidate fires and reduces reads (`67 -> 65`), but public timing regresses `scene_vb 1779 -> 1781`, active loop `1487/1424 -> 1489/1425`, overrun `63 -> 64`, blocking `95 -> 97`, hidden refill `25 -> 27`, and due misses to `17`. Low needs generated phase ownership or pack/data-shape work, not the high-side read group copied across tides. |
| WALKSTUF1 high read group `178..194` | Historical direct-table forms remain closed. The v279 focused gate reduced loop reads `68 -> 67`, but regressed high `scene_vb 1777 -> 1781`, active loop `1488/1426 -> 1492/1423`, overrun `62 -> 69`, blocking `92 -> 99`, hidden refill `27 -> 28`, and due misses `14 -> 16` with fixed pack LBA and `217088` byte PS-EXE bucket. The allocator-era current-baseline retest was also phase-negative by itself. The 2026-05-19 promotion is the narrow paired form only: first trim frame56/source67 in place, then add `{178..194}`, yielding flat `1471/1440` timing with blocking `57 -> 56` and reads/read time `45/209 -> 43/207`. Do not retry this neighborhood as a standalone hand table; pair future rows with data-shape or generated ownership proof. |
| WALKSTUF1 high read group `423..439` after frame56/`178..194` | Done as same-speed CD work. The focused proof stayed flat at `1807/1471/1440`, overrun `31`, blocking/refill `56/13`, and due `10`; the five-yellow canary kept BUILDING2 high, VISITOR3 high/low, and WALKSTUF1 low exact-flat while W1-high loop reads/read time improved `43/207 -> 42/205`. Keep it as a prerequisite W1-high pressure baseline, not a VBlank speed win. |
| WALKSTUF1 high read group `404..416` after `{423..439}` | Done as same-speed CD work. The focused proof stayed flat at `1807/1471/1440`, overrun `31`, blocking/refill `56/13`, and due `10`, while loop reads/read time improved `42/205 -> 41/200`; the five-yellow canary kept BUILDING2 high, VISITOR3 high/low, and WALKSTUF1 low exact-flat. Keep it as the current W1-high pressure baseline, not a VBlank speed win. |
| WALKSTUF1 high current-plan read groups `372..388`, `365..389`, and `268..280` | Do not promote or retry as hand-authored tables on the allocator-era baseline. The current planner correctly predicted saved reads, but not safe cadence: `{372..388}` saved `41 -> 39` reads and regressed to `1813/1477/1442`, overrun `35`, blocking/refill `45/15`; `{365..389}` saved `41 -> 38` and regressed to `1813/1477/1443`, overrun `34`, blocking/refill `46/16`; `{268..280}` saved `41 -> 40` and regressed to `1815/1479/1440`, overrun `39`, blocking/refill `52/16`, due `9`. Remaining W1-high grouped-read work needs generated deadline/refill ownership or no-hot-C metadata, not another scalar row. |
| Packed PAL4 compositor direct single-column dirty marking | Do not promote in the current source shape. Replacing `grMarkRectDirty()` with `grMarkSingleColumnDirty()` inside `grCompositePacked4SpansToBackground()` crossed the PS-EXE bucket (`233472 -> 235520`), shifted W1-high LBA by one sector, and regressed W1-high to `1809/1473/1440`, overrun `33`, blocking/refill `45/17`. Retry only after source-headroom work or with a code-size-neutral row-dirty update that proves fixed LBA/EXE identity first. |
| WALKSTUF1 high read group `74..86` | Do not retry as a hand-authored source table. The current allocator-era probe fired and improved blocking/due (`56 -> 52`, `10 -> 7`), but regressed public speed (`1471/1440 -> 1477/1439`), overrun (`31 -> 38`), and hidden refill (`13 -> 15`). Early W1-high setup-edge work needs generated deadline/refill ownership or a paired data-shape reduction, not another static append row. |
| WALKSTUF1 high frame `37` setup-edge motion-copy | Do not promote under the current runtime motion helper. Sparse-in-place frame `37` motion-copy shrank the payload `6885 -> 4817` bytes and reduced the setup-prime spill from `2098` bytes to `30`, but the focused v280 gate regressed high to `1790`, `1501/1422`, overrun `79`, blocking `101`, and hidden refill `38`; only due misses improved `14 -> 13`. The signal should be retried only as no-runtime/precomputed data or generated scheduler ownership, not as the current compact motion marker dispatch. |
| WALKSTUF1 high early-prefix cleanup gap1 | Do not promote without a hidden-refill fix, but keep as the strongest current signal. The v283 pack-only cleanup-gap1 transform on frames `0..37` saved `1126` bytes and improved high from `1777`, `1488/1426`, overrun `62`, blocking `92` to `1770`, `1481/1425`, overrun `56`, blocking `81`; the sole gate blocker was hidden refill `27 -> 28`. Gap3 fully setup-covered entry `37` but regressed hidden refill harder, and a high-only slack-4 guard fixed hidden refill by starving visible CD (`blocking 132`). Retry with a targeted generated refill reservation, not a scalar slack guard. |
| WALKSTUF1 high entry136 preserve-offset trim | Done as work-volume, not a speed win. The isolated current-baseline entry `136` / source frame `244` trim shrinks `3762 -> 2596` bytes (`3 -> 2` sectors) with fixed `WALKSTUF1.FG2` footprint/LBA/sectors and exact-flat five-yellow timing. The full four-entry preserve-offset batch is rejected because the early-prefix trims regress W1 high to `1477/1433`, blocking/refill `71/24`; do not retry the early-prefix batch without generated deadline/refill ownership. |
| MARY3 guarded prefetch-preserve | Done; keep as the current MARY3 baseline. MARY3 now preserves stage/window prefetch under clean pressure, but uses an `8` VBlank scene-local window-refill guard to keep hidden refill debt at zero. High improves `2402/2295 -> 2296/2294`, `overrun_vb 107 -> 2`, `blocking_vb 690 -> 53`, `loop_reads 255 -> 44`, and `due_misses 255 -> 13`; low improves `2402/2296 -> 2297/2295`, `106 -> 2`, `693 -> 51`, `255 -> 44`, and `255 -> 13`. The `7` VBlank retry is rejected because high tide regressed against the slack-8 baseline. The current rollup is tracked at the top of this file. |
| VISITOR3 cleanup-only offscreen clip | Do not promote. Clipping only offscreen FGP3/v4 cleanup spans preserves pack sizes, entry offsets, LBAs, and PS-EXE bytes while trimming `5526` bytes, `58513` cleanup pixels, and `1299` cleanup spans per tide, but both focused v106 gates are exact-flat: high remains `1137/1024` with `blocking_vb=190`, low remains `1138/1024` with `blocking_vb=191`. Treat this as safe but inert; future offscreen work needs draw/CD phase ownership or a different measured counter. |
| WALKSTUF1 high setup-prime cap 152 KiB | Do not promote. Raising the high-only cap from `144 KiB` to `152 KiB` extends setup coverage to sectors `2..76` and nominally cuts high loop reads `134 -> 133`, but regresses high `scene_vb 1880 -> 1884`, `loop_vb 1592 -> 1595`, `target_vb 1406 -> 1405`, `blocking_vb 275 -> 286`, and `due_misses 55 -> 57`. Treat `144 KiB` as the high-cap knee. |
| WALKSTUF1 low setup-prime cap 168/176 KiB | Do not promote. The older `168 KiB` cap preserved pack LBA and PS-EXE bucket but was exact-flat at `1895`, `1604/1407`, `overrun_vb=197`, `blocking_vb=271`, `prefetch_overrun_vb=54`, `loop_reads=132`, and `due_misses=50`. The current-baseline v367 `176 KiB` retest is also exact-flat at `1776`, `1484/1431`, overrun `53`, blocking `72`, hidden refill `22`, reads `67`, and due `12`, with setup coverage still `prime=[2, 78]`. Treat `160 KiB` as the low-cap ceiling under the current contiguous setup-prime policy. |
| WALKSTUF1 low no-direct-stage branch | Do not promote as a broad scene-name check. It improves low visible metrics (`1604 -> 1601`, `blocking_vb 271 -> 214`, `due_misses 50 -> 34`) but regresses hidden refill (`prefetch_overrun_vb 54 -> 79`) and crosses the PS-EXE bucket. Keep the signal; retry only with a narrower direct-stage threshold or generated scheduler metadata. |
| Direct-stage cap 4 KiB | Do not promote. It preserves layout and lowers WALKSTUF1 low blocking, but regresses active timing (`1604 -> 1607`) and hidden refill (`54 -> 81`). Keep the global cap at `8 KiB`; frame/range-specific scheduling is required. |
| Direct-stage caps 6 KiB and 7 KiB | Do not promote or retry as scalar thresholds. `6 KiB` repeats the hidden-refill failure on both WALKSTUF1 tides despite visible blocking relief, and `7 KiB` is too small a blocking win with target-relative overrun regressions. Keep `8 KiB` until generated scheduler/read-cost metadata can choose frame/range-specific coverage. |
| WALKSTUF1 high direct-stage deny range `178..194` | Do not promote or retry as a hand-coded range policy. It improved hidden refill (`32 -> 26`) but regressed high `loop_vb 1491 -> 1495`, overrun `65 -> 68`, blocking `85 -> 95`, loop reads `69 -> 70`, and due misses `13 -> 14`. Treat this as proof that local range deny-lists still need generated scheduler ownership, not another scalar or source-table tweak. |
| WALKSTUF1 low read group `297..313` with `minSlack=8` | Do not promote or retry as a hand table. The safe slack guard prevented the group from firing (`group_hits=0`), while the source branch still shifted low target enough to regress overrun by one VBlank. WALKSTUF1 read clusters need generated scheduler metadata, not another hot source-table branch. |
| WALKSTUF1 low setup segment `329..345` | Do not promote or retry as a retained setup segment. The v303 focused probe converted real CD pressure (`blocking_vb 75 -> 67`, `prefetch_overrun_vb 25 -> 16`, `loop_reads 69 -> 66`, `due_misses 13 -> 12`), but setup cost and code growth regressed public timing (`scene_vb 1770 -> 1784`, active loop `1478/1428 -> 1479/1432`) and grew `foregroundPilotPlay` by `180` bytes. Future WALKSTUF1 low cluster work needs generated scheduler-owned append timing or pack-side relocation, not extra setup residency. |
| WALKSTUF1 selective upload-ready same-footprint append | Do not promote or retry as a raw same-footprint append. The current compact FGP3/v4 packs now expose `611305` zero-tail bytes and can fit a `609192` byte budgeted x-band subset for `39` frames with `1991904` modeled upload bytes saved, but raw foreground-only safety still reports `0` selected draw-covered x-band bytes and `0` all-draw-covered selected frames. Compact slack is a byte budget, not a safety proof; retry only with safe background-owned/precomposed pixels, ownership metadata, generated scheduler ownership, or MoveImage-safe motion data. |
| VISITOR3 default selective upload-ready append | Do not promote as a layout-neutral pack append. The current high-tide threshold plan selects `92 / 144` frames and estimates `5730024` selected upload bytes saved, but the upload-ready payload plus rect metadata needs `2111224` bytes against only `970076` bytes of padded zero-tail slack. Retry only as a smaller budgeted subset, compressed upload payload, shrinking pack transform, or explicit layout-moving experiment. |
| VISITOR3 budgeted selective upload-ready target | Done as host-side implementation target, not runtime behavior. The current v140 analyzer exact-knapsacks the default-selected VISITOR3 rows against the post-tail-trim pack slack: high selects `75 / 117` default frames using `888880 / 891012` bytes and retaining `6290232` modeled upload bytes saved, while low selects `74 / 117` frames using `853848 / 854114` bytes and retaining `6166528` modeled bytes saved. Runtime promotion still needs a generated pack format with pre-contiguous rows, a safe background-owned/precomposed pixel source, and full VISITOR3/canary validation. |
| VISITOR3 runtime dirty-upload narrowing | Do not retry as a source-side optimization. The live uploader already has row-level dirty X metadata, but exact narrow intervals for current VISITOR3 would create about `131996` upload rects over the loop, and scratch-packed x-band variants have already failed from code-size, copy, and cadence cost. Upload-byte work must be pack-emitted or precomposed, not packed from tile rows during `grDrawBackground()`. |
| VISITOR3 v140 current-window read-plan refresh | Do not promote or retry another hand-authored source table. The refreshed read-plan from v127 found `0` candidates that are append-start fireable, current-window-sized, and low-risk. The rows that fit and fire are the late tight-cluster class, including the already-rejected high `315..331` and low `333..349` shapes, and remain `high-risk:scheduler-only`. |
| VISITOR3 setup-owned tail atlas | Do not retry manual zero-tail atlases through setup segments. The v178 25-sector atlas removed reads but regressed high and failed low full-scene/hidden-refill gates; the v179 7-sector terminal atlas still regressed both tides. Tail residency needs phase-transition preload, a real scheduler sidecar, or precomposed data, not another scene-start setup segment. |
| VISITOR3 pack-only tail duplication | Do not retry as a layout-only tail move. The v180 no-source duplication of frames `139..144` kept pack sizes/LBAs fixed but regressed high/low to `1122/1027` and `1130/1024`, added `prefetch_overrun_vb=3` on both tides, and raised blocking by `+4` on both. The terminal payload phase is negative unless paired with scheduler ownership or precomposed data. |
| VISITOR3 low frame-128 resident segment copy | Done; keep as the prerequisite for the current VISITOR3 low slot-swap baseline. The v302 pack/source promotion aliases duplicate low frame `123` to frame `121`, compacts frames `118..128` into the second setup segment, grows that segment from `24` to `27` sectors, and copies frame `128` resident without changing the `1555450` byte pack footprint. Focused and broad gates improve low `scene_vb 1402 -> 1399`, active loop `1075/1039 -> 1071/1039`, overrun `36 -> 32`, blocking `67 -> 63`, loop reads `12 -> 11`, loop-read VBlanks `67 -> 63`, and due misses `12 -> 11`; hidden refill stays `0`, and VISITOR3 high plus controls stay flat. The v327 frame128/frame129 resident-slot swap and v338 tail compaction supersede this row by moving low to `1072/1040`, overrun `32`, blocking `58`, loop-read time `58`, and due misses `10`. Current rollup is tracked at the top of this file. |
| VISITOR3 low frame128/frame129 resident-slot swap | Done; keep as the prerequisite for the current VISITOR3 low tail baseline. The v327 pack-only promotion copies frame `129` into frame `128`'s already paid segment2 resident slot and points frame `128` back to its original cold payload. Pack size `1555450`, low LBA `23371`, sectors `760`, source code, loop reads, due misses, and hidden refill stay fixed. Focused and broad gates improve low `scene_vb 1408 -> 1401`, active loop `1079/1040 -> 1072/1040`, overrun `39 -> 32`, blocking `67 -> 64`, and loop-read time `67 -> 64`; VISITOR3 high, WALKSTUF1 high/low, BUILDING2 high/low, FISHING1 high, JOHNNY1 high, and the longer Activity9/Mary3 canaries stay flat. The v338 tail pack-only compaction supersedes this row's CD-pressure counters while keeping the same public timing. |
| VISITOR3 low tail pack-only compaction | Done; keep as the prerequisite for the current VISITOR3 low frame129-delta baseline. The v338 pack-only pass rewrites only `VIST3LOW.FG2`: frame `143` cleanup becomes one bounded compact span per active row (`3497 -> 2857` bytes) and frame `144` terminal cleanup becomes one compact full-width span per row (`3744 -> 1908` bytes), moving frame `144` from exclusive sector `306` to `305` inside the already paid tail setup window. Focused gate improves low CD pressure with public timing flat: `1072/1040`, overrun `32`, blocking `64 -> 58`, loop reads `11 -> 10`, loop-read time `64 -> 58`, and due misses `11 -> 10`; hidden refill remains `0`, pack footprint/LBA/sectors stay `1555450`/`23371`/`760`, and no source changes are needed. The v452 frame129 delta supersedes this row's public timing and CD-pressure counters. |
| VISITOR3 low frame129/frame132/frame137 D4 plus prime-gap relocation | Done; keep as the current VISITOR3 low baseline. The v452 pack/runtime promotion moves frame `128` into the accepted resident slot and stores frame `129` as a 609-byte custom D4 delta against frame `128` inside freed payload space. v470 stores frame `132` as a 768-byte D4 delta, v477 moves that existing payload into the unused setup-prime gap at offset `203181` / sector `99`, and v510 moves frame `137`'s existing 503-byte D4 payload into the same setup-prime in-data gap family at offset `203949` / sector `99`. Current low is scene `1391`, active loop/target `1062/1040`, overrun `22`, blocking `42`, loop reads/due `7/7`, and loop-read time `42`; hidden refill stays `0`, pack footprint/LBA/sectors stay `1555450`/`23371`/`760`, and the PS-EXE bucket stays `217088`. VISITOR3 high, WALKSTUF1 high/low, BUILDING2 high/low, VISITOR5 high/low, and FISHING1 high canaries stayed on accepted profiles across the prerequisite passes; v510's VISITOR3 high, BUILDING2 high/low, and WALKSTUF1 high/low canaries stayed exact-flat. |
| VISITOR3 low extra setup residency after v338 | Do not retry as another retained segment. The v397/v398 segment sweep proved all direct forms are closed on the current baseline: full `206..305` residency disabled effective prefetch and regressed to `1119/1034`, blocking `475`, reads/due `143`; same-footprint `206..230` swapping lost the accepted tail and regressed to `1073/1038`, overrun `35`, blocking `67`, reads/due `12`; additive third-segment `206..230` plus accepted `150..177` and `281..305` failed structurally before `JCPERF2`. Keep the current v338 resident budget; remaining VISITOR3 low work must compress/alias one payload inside existing resident bytes or use generated deadline ownership without another setup buffer. |
| VISITOR3 low tiny setup segment `190..199` | Do not promote or retry as a retained setup segment. The v415 focused probe was intentionally smaller than the closed `206..230`/`206..305` swings and did reduce CD pressure (`blocking_vb 58 -> 54`, reads/due `10/10 -> 9/9`), but strict timing regressed from scene `1401`, active loop/target `1072/1040`, overrun `32` to scene `1407`, active loop/target `1073/1040`, overrun `33`. This proves the setup-debt problem applies even to the isolated first late read; future VISITOR3 low work needs existing-resident-byte compression/aliasing or generated deadline ownership, not more setup buffers. |
| VISITOR3 low grouped append `206..230` / `218..230` | Do not promote or retry as a local read-group table. The v421-v423 sweep tried guarded and unguarded `206..230` with a 24-sector capacity plus a normal-capacity `218..230` append-edge variant; all completed variants stayed exact-flat at `1401`, `1072/1040`, overrun `32`, blocking `58`, hidden refill `0`, reads/due `10/10`, while growing/shifting foreground hot code. Future ownership for this cluster needs generated scheduler metadata beyond the existing grouped-append path or pack-side byte placement inside current resident coverage. |
| VISITOR3 motion-copy frames `119..123` plus high frame `115` | Done; keep as part of the current VISITOR3 baseline, now superseded by later sparse/re-anchor/setup/no-op/resident-slot/frame-delta/setup-segment passes. The v181 scene-specific compact motion marker moves already-composited background rows, restores exposed clean spans, and draws only residual pixels; v182 applies the same family of state-hull motion-copy payload to high-tide frame `115`. Later v188/v189/v193/v202/v204/v205/v206/v207/v213/v214/v216/v227/v234/v237/v238/v248/v249/v291/v292/v299/v302/v327/v338/v452/v460/v462/v464/v477/v501/v510/v629 passes move the accepted profile to high `1063/1040` and low `1062/1040` while preserving the `1555450` byte footprints, current LBAs `22611/23371`, and the `217088` byte PS-EXE bucket. Next retries should expand motion-copy through generated eligibility, not hand-tail repoints. |
| VISITOR3 frame `116` opcode-1 hull shapes | Do not promote or retry without a different format. The v195 target-hull shape saved `7800` active bytes but regressed high to `1109/1028` with hidden refill `2`; the v208 copy-only hull shape saved `9102` bytes and removed cleanup rows, but regressed high harder to `1114/1029`, overrun `85`, and blocking `114`. Frame `116` is a byte trap under the current motion opcode. |
| VISITOR3 v184 terminal zero/origin/read probes | Do not promote or retry as scalar pack surgery or hand tables. Terminal zero-run trimming saved `0` bytes; compact-origin rebasing saved only `12` high-tide bytes at frame `113`, saved `0` low-tide bytes, and no terminal bytes; low hull-mode precursor retries still regressed frame `117` to `1110/1028`; and terminal `16`-sector read groups cut some reads but regressed high `1104 -> 1108` and low `1108 -> 1112`. Next VISITOR3 work needs a custom data format, precomposed ownership, or generated deadline scheduling. |
| VISITOR3 v185 motion row-copy runtime path | Do not promote. The generic same-row pointer-copy variant crossed the PS-EXE bucket `215040 -> 217088` and shifted VISITOR3 pack LBAs by one sector. The narrowed left-tile-only variant stayed at `215040` with fixed LBAs, but was exact-flat on both tides: high stayed `1104/1030`, `blocking_vb=128`, `loop_reads=23`; low stayed `1108/1028`, `blocking_vb=143`, `loop_reads=27`. Motion-copy CPU is no longer the limiting VISITOR3 lever at this granularity. |
| VISITOR3 generic compact motion-marker dispatch | Do not promote or retry in the current code shape. The v376 dispatcher to the dormant motion-X decoder grew the PS1 executable to `214K`, disabled effective prefetch on VISITOR3 high (`policy=none`), and regressed to `1161/1031` with `blocking_vb=505`, `loop_reads=143`, and `due_misses=143`. Any future marker runtime needs to be code-size-neutral and memory-budgeted before emulator time. |
| Dormant compact motion-X decoder | Done as cleanup, not a speed lane. The v377 source cleanup removes the unused decoder and helpers after VISITOR3 low, WALKSTUF1 low/high, and BUILDING2 high stayed exact-flat with fixed pack LBAs and the same `217088` byte PS-EXE bucket. Do not reintroduce a generic marker dispatch unless the new path is code-size-neutral, prefetch-budgeted, and validated first against VISITOR3 plus WALKSTUF1 canaries. |
| VISITOR3 v186 compact motion-copy metadata | Do not promote. Accepted motion-copy rows are all contiguous one-span rows, so opcode v2 can remove per-row `relY/spanCount` metadata and save `3906` high bytes plus `3485` low bytes. The compact repack kept LBAs and PS-EXE bucket fixed but regressed high `1104 -> 1113` with hidden refill `0 -> 3` and low `1108 -> 1112`; sparse-in-place kept high flat but regressed low to `1119/1027`, blocking `143 -> 152`, hidden refill `0 -> 2`. The byte saving shifts cadence/accounting in the wrong place. |
| VISITOR3 v187 generic narrow dirty-row upload | Do not promote or retry as a hot runtime heuristic. The x-band byte model remains compelling (budgeted high `75` frames: `967720` narrow bytes vs `7575040` full-width bytes; low `76` frames: `922704` vs `7391360`), but the code-only scratch-copy implementation grew `grDrawBackground` by `816` bytes, crossed the PS-EXE bucket `215040 -> 217088`, shifted downstream hot symbols by `+816`, and failed before `JCPERF2` on both VISITOR3 tides. Retry only as a pack-authored precomposed/background-owned payload, code-size-neutral deferred upload opcode, or generated ownership metadata outside the full-width upload hot path. |
| VISITOR3 `20 KiB` retained window with `12` VBlank slack | Do not promote or retry as scalar window/slack tuning. It improved total scene duration by shortening setup/load shape, but active loop regressed on both tides: high `1118 -> 1131`, blocking `150 -> 210`, reads `27 -> 39`; low `1126 -> 1139`, blocking `170 -> 212`, reads `31 -> 41`. Hidden refill stayed `0` and layout stayed fixed, so the failure is scheduler/CD ownership, not binary layout. |
| VISITOR3 low `20 KiB` stream window after v216 setup-prime | Do not promote or retry as a scalar source tweak. The parameter-only `prefetch-window 20480` path disabled setup-prime and regressed low to `1105/1035` with hidden refill `3`; the source-level setup-prime-preserving v226 path improved low locally to `1093/1035`, but broad canaries failed because the extra foreground code shifted WALKSTUF1 high/low refill and overrun badly. Retry this only as layout-pinned/generated data ownership or a second retained-window design that does not grow `foregroundPilotPlay`. |
| VISITOR3 low setup-prime `200 KiB` / `216 KiB` | Do not promote or retry as scalar low-prime tuning. `216 KiB` regressed low `1126 -> 1127` and blocking `170 -> 173`; `200 KiB` regressed low to `1152/1024`, blocking `191`, and hidden refill `3`. Keep the accepted `208 KiB` low cap. |
| VISITOR3 high setup-prime `256 KiB` after stage guard | Do not retry as scalar high-prime tuning. With the v127 stage guard active, `256 KiB` reduced high loop reads by one but regressed high to `1131/1027`, overrun `104`, blocking `155`, and hidden refill `3`. Keep high at `232 KiB`; larger residency is phase-negative under the current scheduler. |
| VISITOR3 high clean-relief setup-prime restoration | Do not retry as scalar high-prime tuning under the allocator-era clean-relief path. Re-enabling high setup-prime before the `68 KiB` relief window exhausted CACHE at `320`, `304`, `240`, `208`, and `128 KiB`; the only measurable `96 KiB` version regressed high `1082/1042 -> 1091/1041`, overrun `40 -> 50`, blocking `50 -> 63`, and due `3 -> 4`. Keep high setup-prime suppressed while clean-relief is active. |
| VISITOR3 high clean-rect64 plus prime128 | Do not retry as a scalar striping-plus-prime combo. Splitting clean rectangles to `64 KiB` chunks let the probe pass the earlier large-chunk cliff, but it still exhausted CACHE before JCPERF2 after six clean chunks while a `128 KiB` high setup-prime window was resident. Revisit only after reducing clean-rect byte volume, moving clean/setup ownership out of CACHE, or adding generated scheduler/pack ownership. |
| VISITOR3 high split CACHE setup segment `103..127` | Do not retry as manual setup residency. The full extra `103..127` segment exhausted CACHE during clean-rect allocation, and the smaller affordable `113..127` tail regressed high from `1075/1044`, blocking `45`, to `1086/1043`, blocking `57`, despite improving hidden refill. This window needs clean/upload memory reduction, pack-side byte reduction, or generated deadline ownership rather than another retained setup slice. |
| VISITOR3 high terminal-to-active setup swap `103..127` | Do not retry as a same-buffer segment trade. Replacing terminal `277..293` with full active `103..127` exceeded TRANSIENT (`req=172032 have=168956`), while the largest fitting `103..125` slice regressed high from `1075/1044`, blocking `45`, to `1108/1041`, blocking `84`, reads `4 -> 6`, and due `3 -> 5`. The active window needs extra allocator-safe residency or generated deadline ownership; dropping terminal residency makes cadence worse. |
| VISITOR3 high early `34..37` direct/raw-gap cluster | Do not retry as scalar direct-stage suppression, 3-sector micro-windowing, raw retained-gap relocation for frames `56..58`, larger retained-gap relocation for frames `56..63`, or scalar slack/direct-stage followups. The best forms improved visible loop/overrun but all strict-safe candidates either regressed hidden refill (`5 -> 6` or `5 -> 16`) or moved the debt into visible blocking (`50 -> 56/57`). Reopen only with generated deadline ownership that schedules the cluster earlier than the unsafe slack slot, or with a data format that reduces work without shifting the downstream window boundary. |
| VISITOR3 high early `34..40` setup/read-group sweep | Do not retry as hand-authored setup residency or a scalar read group. The post-clean-relief read-plan correction exposed `34..40`, but the terminal-swap setup form regressed to `1116/1042`, the additive fourth setup segment regressed to `1092/1040`, and the live `{34,40}` group was exact-flat at `1082/1042`. The early cluster needs generated deadline ownership or pack/data-shape reduction, not another manual resident slice/table row. |
| VISITOR3 no-op FGP3 entry prune | Do not promote. Removing the visually no-op entries reduced VISITOR3 high `loop_vb 1139 -> 1115`, `blocking_vb 191 -> 123`, `loop_reads 33 -> 29`, and active payload `737600 -> 659318`, but the shortened cadence created hidden refill debt: high `prefetch_overrun_vb 0 -> 56`, low `0 -> 17`. Treat this as evidence that VISITOR3 needs scheduler-owned prefetch placement or budgeted upload-ready data, not isolated entry-count pruning. |
| VISITOR3 no-op empty-hold recast | Do not promote. The pack-side scanner found `0` current VISITOR3 high/low FGP3/v4 payload entries with both cleanup and draw pixel counts at zero, so active payload stays `737600 -> 737600` and no binary runtime probe is available. The old prune win removed entries that still carry real cleanup/draw work under the current data shape. |
| VISITOR3 entry-origin recentering | Do not promote. The latest v184 host-side size gate over the current motion baseline found only a `12`-byte high-tide saving at frame `113`, `0` low-tide saving, and no terminal-frame reduction. It is not enough to change CD duration or justify a runtime gate. |
| Dirty upload rect cap `8 -> 24` | Do not promote. VISITOR3 analyzer estimated only a tiny loop-level upload-byte reduction from eliminating the three cap-hit frames, and the runtime probe was exact-flat on VISITOR3 high/low (`1357/1023` and `1361/1023`) with no key metric improvement. Move upload work to generated selective upload-ready payloads or restore/compose coalescing rather than global rect-cap tweaks. |
| ACTIVITY9 high FGP3 read group `447..463` | Do not retry under the current data shape. It was tested with the low FGP3 group and stayed exact-flat on high tide, so only the low table was promoted. Revisit only after ACTIVITY9 high pack data, append-start ownership metadata, or scheduler timing changes. |
| ACTIVITY9 FGP3/v3 cleanup-metadata compaction | Do not promote as a paired high/low pack change under the current gate. It saved `257210` active payload bytes per tide and improved low `loop_vb 2098 -> 2087`, but high regressed `2094 -> 2099` with stable layout. Retry only with a high-tide window/cadence retune or explicit tide-specific promotion logic that keeps high flat. |
| BUILDING6 `48 KiB` window plus `15..39` group | Do not promote or retry as a scalar larger-window path. It saved reads but regressed high to `2568/2443` with hidden refill `117` and low to `2565/2445` with hidden refill `96`; require generated scheduler ownership or a shrinking/selective FGP2 encoder first. |
| BUILDING6 pal4 padded/direct FGP3 | Do not benchmark direct pal4 temporal-residual conversion under the current validated packs. The size gate expands `1444370 -> 1601445`, and accepting layout movement was measured in v154 and still failed: high `2520/2442 -> 2618/2418`, blocking `62 -> 283`; low `2515/2437 -> 2621/2419`, blocking `70 -> 292`. Retry only with a shrinking/selective/keyframed encoder, generated scheduler ownership, or a motion format with RAM-mirror/dirty-state proof. |
| BUILDING2 high duplicate-payload cache | Do not promote as a local runtime cache. The v515-v521 sweep proved the repeated-payload reads are real (`loop_reads 58 -> 56`), but fixed-bucket forms regressed to `1351/1308`, overrun `43`, blocking `58`, and refill `19`. The only loop win required oversized PS-EXE/CD phase (`217088 -> 219136`, `BUIL2.FG2` LBA `6181 -> 6182`) and still regressed blocking `54 -> 56`. Retry BUILDING2 high through generated deadline ownership or pack-side visual-work reduction, not another hot-loop duplicate cache. |
| BUILDING2 high preserve-offset trim-tail hot cluster | Do not retry entries `89`, `90`, or `91`, or the full current preserve-offset trim batch, as same-offset pack surgery. The full batch saved reads/refill (`45 -> 43`, `16 -> 15`) but regressed `1347/1313`, blocking `39`, to `1349/1313`, blocking `41`; each isolated hot entry reproduced the same negative profile. Future B2-high byte work needs generated deadline ownership, placement-aware repacking, or render/upload reduction rather than shrinking these payloads in place. |
| BUILDING6 v146 upload/motion refresh | Do not spend emulator time on raw BUILDING6 upload append. Same-footprint slack is still `1` byte and safe-pixel coverage is still `0`; the actionable path is generated zero-shift/motion residual format work or scheduler ownership, not another scalar source probe. |
| BUILDING4 high read group `537..561` | Do not promote or retry as a raw 24-sector hand-coded group. It saved two reads but regressed loop, blocking, and refill pressure; require generated scheduler/cost metadata before larger BUILDING4 high append groups. |
| BUILDING4 high offscreen draw-span clip | Do not promote as a direct mirror of the v652 low-tide win. The v715 fresh current-code gate was safe but only improved overrun `32 -> 31` by raising target `2815 -> 2816`; loop/blocking/refill stayed `2847/37/33`, and the result is still worse than the published `2844/2816` matrix row. Retry only after a row refresh or with scheduler/static-upload work that beats the documented baseline. |
| BUILDING4 low append group `178..202` and larger fresh-fill windows | Do not promote or retry as a local hand table or scalar window growth. The v387 append-group probe stayed exact-flat against the fresh current baseline because the cluster is fresh-fill/window-owned, not append-owned. The `40 KiB` and `48 KiB` window variants proved the transaction signal is real but phase-negative: reads fell to `23`/`18`, while loop and blocking regressed to `2881/68` and `2897/85`. Future BUILDING4 low work needs generated deadline ownership, pack-side byte/cleanup reduction, or selective preprocessing. |
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
| VISITOR3 prepared-present threshold `3` or `5` after v127 | Do not retry as a local slack-constant tweak. Both sides of the accepted `4` threshold stayed exact-flat on the pre-v181 VISITOR3 baseline (`1118/1028` high and `1126/1025` low), with blocking, reads, hidden refill, pack LBAs, and PS-EXE bucket unchanged. Continue only through generated scheduler ownership or safe pack/data-shape work. |
| VISITOR3 high `144..156` current-window group after cleanup compaction | Do not retry as a source-table append. The fresh post-BUILDING4 baseline probe stayed exact-flat on VISITOR3 high/low (`1406/1019` and `1405/1015`), reported `group_hits=0`, and only shifted hot symbols by `+4` bytes. Retry this sector cluster only if scheduler-owned refill metadata changes when the append is attempted. |
| VISITOR3 high early `40..56` direct grouped append | Do not retry as a hand-authored table. The current allocator-era focused gate with `{40,56,0}` was exact-flat at `1388/1071/1045`, overrun `26`, blocking/refill `35/1`, loop reads `7`, and due `2`, matching the earlier `{40..52}` no-op. The whole `40..64` family needs generated deadline/refill ownership or pack/render data-shape reduction, not another static source row. |
| VISITOR3 high early `40..52` fresh/direct group ownership | Do not retry as scalar runtime ownership. The fresh grouped-window probe saved one loop read (`7 -> 6`) but regressed high to `1398/1082/1043`, overrun `39`, blocking/refill `47/14`, and crossed both VISITOR3 LBA and PS-EXE bucket identity gates. Reopen only with generated deadline/refill-budget scheduling, code-size-neutral metadata, or pack/render data-shape work that keeps the saved read hidden. |
| VISITOR3 high early `40..46` direct grouped append | Do not retry separately. The current high-tide tight-window refill already extends past sector `56`, and the `{40..56}` superset was exact-flat; a `{40..46}` hand table cannot form a smaller measured boundary in this code path. |
| VISITOR3 resident-window staging reuse | Do not retry as a broad local staging eligibility change. It saved one VISITOR3 high duplicate read and reduced blocking, but turned hidden refill into `prefetch_overrun_vb 7 -> 40` and regressed low tide `loop_vb 1405 -> 1409`. Retry only with explicit generated refill budget/cadence ownership, not by treating every resident large entry as stageable. |
| WALKSTUF1 high entry55 no-op rewrite | Do not retry entry55/source frame65 as a standalone empty-entry or preserve-offset no-op trim. Canonical W1-high matrix boot proved both save about `4.7 KiB` and reduce reads `41 -> 39`, but regress cadence to `1475/1438`, blocking `63/64`, and refill `18`. Reopen only with generated per-frame deadline/refill ownership or a paired scheduler/data-shape change that proves no visible/refill regression. |
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
| Compact FGP3/v4 compositor scoped `O2` | Do not promote or retry as a standalone compiler toggle. The v409 probe removed `optimize("Os")` from `grCompositePacked4CompactTemporalResidualToBackground()` and kept WALKSTUF1 low's pack LBA plus the `217088` byte EXE bucket fixed, but regressed low from `1484/1431`, blocking `72`, refill `22`, reads/due `67/12` to `1486/1428`, blocking `81`, refill `27`, reads/due `69/13`. Keep the compositor scoped to `-Os`; revisit only with a split/layout-pinned rewrite or a generated data-shape change that reduces compositor work enough to dominate codegen phase drift. |
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
| JOHNNY1 current-fit read groups | Do not retry `131..147`, `145..161`, or `138..154` as local hand tables. The v177 probes stayed exact-flat on both tides with `group_hits=0`; retry only with generated append ownership, larger retained windows, or a different data shape. |
| JOHNNY1 combined late read groups | Do not retry the paired `131..147` + `145..161` table as a scene-specific hand-authored source branch. The v391 current-baseline retest kept both tides exact-flat at `2069`, `1973/1945`, overrun `28`, blocking/refill `25`, loop reads `7`, and due misses `0`, while growing/shifting foreground hot code. A useful JOHNNY1 scheduler win now needs generated deadline ownership or a materially different retained-window design. |
| JOHNNY1 zero-tail FGP3 compaction | Do not promote as part of the under-99 speed loop. The v392 pack-only compaction safely trims `129450` trailing zero bytes from each JOHNNY1 pack, but timing remains exact-flat and low-tide LBA shifts `14202 -> 14139`. Keep it as a possible future whole-disc footprint cleanup, not as a scene-speed baseline. |
| JOHNNY1 direct black framebuffer cleanup clear | Do not promote the v393 source-side helper. Direct black `TILE` clears improved high locally (`1973/1945 -> 1972/1946`, overrun `28 -> 26`, blocking/refill `25 -> 23`), but grew the PS-EXE bucket `217088 -> 219136`, shifted high LBA `13983 -> 13984`, and low tide regressed scene `2069 -> 2070` with no loop/blocking win plus LBA `14202 -> 14203`. Keep the black-cleanup/upload signal, but retry only as a code-size-neutral primitive path, pack-authored black cleanup elision, or generated upload/restore plan with fixed layout. |
| Upload coordinate static tables | Do not retry; static tables grew `grDrawBackground` and did not move timing. |
| Async CD | Async state ownership and polling metrics exist in a trace build. |
| `Setloc` skipping | Full frame hashes and work-identity gates prove every frame rendered. |
| Upload rect cap `6` | Cross-scene matrix proves `max_upload_rects <= 6`; current fishing1-only retest has no measured win. |
| Present-path polling cleanup | Replace only as part of a scheduler/input design; direct removal regressed pressure and weakens pause responsiveness. |
| 4-VBlank catch-up guards | Do not retry as another local threshold guard; the prepared-plus-window-resident form was exact no-op. Retry only as structural hold rebalance or a first-class prepared/dual-buffer scheduler. |
