# PS1 Port — Current Status

> 🌐 **Rendered version:** **[/about/status/](https://hunterdavis.com/johnny-castaway-ps1/about/status/)** — this doc rendered on the project website with cross-links and prose context. The GitHub copy here is the source.


**Last updated:** 2026-05-23 (`perf/allocator-era-under-green-20260517` after
the WALKSTUF1-high frame0 consume speed promotion, the WALKSTUF1-high `200..215` previous-visible cleanup headroom pass, the VISITOR3-low phase1 / segment4 `38..79` promotion, the WALKSTUF1-high prep2 frame-gate promotion, the WALKSTUF1-high `185..191` direct-stage rescue, the WALKSTUF1-high `383..399` transient setup-slice / clean-cap speed pass, the WALKSTUF1-high `183..199` late-layout / `372..384` owner refill-headroom pass, the WALKSTUF1-high `372..388` fresh-owner retarget speed pass, the WALKSTUF1-high frames `189..191` direct-stage headroom pass, the WALKSTUF1-high entry134 same-speed screen-clip headroom pass, the WALKSTUF1-high `{108..124}` same-speed CD-pressure pass, the WALKSTUF1-high `62..66` clip plus `{92..108}`/`{272..284}` read-group speed pass, the W1-high frame138, active-loop, and early offscreen clip headroom passes, the BUILDING2-high fixed-footprint physical compaction speed pass, the VISITOR3-low one-VBlank
phase retime, the VISITOR3-low slack-knee speed promotion, the W1-low compact
trim/retarget phase green promotion, the W1-high and BUILDING2-high one-VBlank phase promotions, the VISITOR3-high
segment3/exact-clean speed promotion, the D4/local-LZ decoder inline
code-headroom passes, and the `JCPERF2`-only perf-reporting code-headroom
pass; all 63 scenes remain validated, all 126 high/low rows are
timing-bearing, and the public headless battle card is `+0.1956%` over target
/ `99.8062%` target speed).

## Overall

The game boots on DuckStation, loads resources from CD, and runs scene
animations. `ACTIVITY 1`, `ACTIVITY 4`, `ACTIVITY 5`, `ACTIVITY 6`, `ACTIVITY 7`, `ACTIVITY 8`, `ACTIVITY 9`, `ACTIVITY 10`, `ACTIVITY 11`,
`ACTIVITY 12`, `FISHING 1`, `FISHING 2`, `FISHING 3`, `FISHING 4`,
`FISHING 5`, `FISHING 6`, `FISHING 7`, `FISHING 8`, `BUILDING 1`,
`BUILDING 2`, `BUILDING 3`, `BUILDING 4`, `BUILDING 5`, `BUILDING 6`, `BUILDING 7`, `JOHNNY 1`, `JOHNNY 2`,
`JOHNNY 3`, `JOHNNY 4`, `JOHNNY 5`, `JOHNNY 6`, `MARY 1`,
`MARY 2`, `MARY 3`, `MARY 4`, `MARY 5`, `MISCGAG 1`, `MISCGAG 2`,
`STAND 1`-`STAND 12`, `STAND 15`, `STAND 16`, `SUZY 1`, `SUZY 2`,
`VISITOR 1`, `VISITOR 3`, `VISITOR 4`, `VISITOR 5`, `VISITOR 6`,
`VISITOR 7`, `WALKSTUF 1`, `WALKSTUF 2`, and `WALKSTUF 3` have been validated
under the project's current acceptance bar: pixel-perfect visuals plus
synced SFX, across every applicable variant (night / low-tide / holiday
/ raft-stage), signed off by human visual + audible review. `ACTIVITY 9`
uses an Activity9-specific wide stitch plus source `BOAT.PSB` repair at the
legacy clip edges; the helper also carries boat draw metadata through held
frames so the late bow does not flicker while production island placement
stays variable.
`FISHING 7`
and `FISHING 8` were recaptured with a far-left host/test island position
(`x=-300,y=54`) so all scene-relative pixels are present, but production
runtime placement is now variable again. `JOHNNY 1` and `JOHNNY 6` are
full-screen black-backdrop scenes, so they bypass the ocean/island setup
path and use black cleanup for temporal residual spans.
`JOHNNY 2` is pinned to the host-captured island position
(`x=-64,y=54`) and uses a keyed lower-band overlay plus explicit
hold redistribution for the island/SOS thought bubbles.
`FISHING 5` uses a full-frame keyed current-ledger overlay for its shark
interaction so stale full-host overpaint is not replayed and useful
current shark pixels are not masked out.
`JOHNNY 3` remains valid at variable island positions; the right-shift
run was a diagnostic probe, not a durable placement requirement.
`JOHNNY 4` was captured and tested at `x=-64,y=54` to keep all bottle
message pixels in frame, but production placement remains variable; its
full-frame keyed foreground-only overlay avoids stale bottle overpaint
and the full-host SOS bubble blue-line artifact.
`JOHNNY 5` was captured and tested at `x=80,y=54` so the thrown-bottle
splash is present in the source pack; its full-frame keyed
foreground-only overlay removes stale lower-band overpaint, and hold
timing now pauses on the SOS note bubble instead of the blank
post-bubble rows. Production placement remains variable.
`JOHNNY 6` uses the same black-backdrop runtime classification as
`JOHNNY 1`; no ocean/island background is painted for that scene.
`MARY 1` was visually and audibly signed off on the historical MARY1
validation route (`x=-124,y=37`, raft-stage `5`) without changing the
pack or runtime.
`MARY 2` uses a wide scene-relative multi-view capture stitch so one
pack can carry more than one screen width of island-relative action:
line, mermaid/splash, boot/splash, lower-water cleanup, and the fish
thought-bubble shell are complete while production placement stays
variable-position safe.
`MARY 3` was rebuilt from a far-right full-frame keyed foreground-only
host capture (`x=80,y=54`) so the action left of the island is complete.
The host capture ledger now invalidates stale sprite-surface references
before BMP/layer frees, and the current PS1 perf path preserves stage/window
prefetch under clean pressure with an 8-VBlank MARY3 window guard.
`MARY 4` uses the generic multi-view scene-relative stitch: normal,
far-left, and far-right foreground-only host views are merged into one
pack so island-relative action is complete across runtime placements.
`MARY 5` uses the same generic multi-view stitch, but it is also a
story-flag policy case: `NORAFT` suppresses the generic raft because the
scene carries its own raft art, and `FIRST` skips the walk-in before the
frog/full-wipe transition takes over.
`MISCGAG 1` and `MISCGAG 2` were regenerated through the generic
normal/far-left/far-right foreground-only multi-view stitch and
visually/audibly signed off on the normal high-tide/night route.
`STAND 1` uses the same generic multi-view capture path; it has no
captured SFX events and its 35-frame / 169-vblank idle loop resets
quickly enough that the restart is hard to see.
`STAND 2`-`STAND 12`, `STAND 15`, and `STAND 16` are validated on the normal
high-tide/night route. `STAND 5`-`STAND 9`, `STAND 15`, and `STAND 16` use the
STAND no-stitch export fast path, but that
fast path still keeps a full-frame single-position foreground-only
overlay so static first-frame Johnny pixels are not suppressed as
background. The FG2 runtime now ticks ocean wave animation every frame
so no-stitch scenes get moving water from the engine instead of the
foreground-only pack.
`SUZY 1` is a scene-specific-backdrop case: the source scene uses
`SUZBEACH.SCR`, so the FG2 runtime skips island/ocean/wave/holiday setup
and uses that beach screen as the clean restore baseline.
`SUZY 2` uses the same beach backdrop, but also needs the scene-local
static `MRAFT.BMP` raft in the foreground overlay. Its high/low packs
were rebuilt with a full-frame foreground-only overlay that includes
static base draws, and SFX playback now leaves mixer headroom so its
overlapping raft samples do not clip.
`VISITOR 1` was regenerated through the generic normal/far-left/far-right
foreground-only multi-view stitch and validated on the low-tide/night
route. Its pack carries one captured SFX event and a wide scene-relative
foreground union for the Lilliputian arrival; production placement remains
variable-position safe.
`VISITOR 3` uses a scene-specific synthesis path on top of the standard
normal/far-left/far-right capture set. Foreground-only views keep the moving
sprite pixels clean, the helper accumulates the red ship hull from full-host
live crash frames only, FGP3 temporal cleanup clears the post-crash blank
rows, and hold timing keeps the real frame-158 splash visible without stale
right-side splash residue. Production placement remains variable-position
safe.
`VISITOR 4` uses the generic normal/far-left/far-right multi-view stitch
with production-variable island placement. Live validation confirmed this is
the coconut/plane gag in the current mapping, superseding the old red-boat
catalogue label.
`VISITOR 5` uses the generic normal/far-left/far-right multi-view stitch
with production-variable island placement. Scene-specific hold redistribution
moves time into the coconut impact and downed-plane action rows so the gag is
readable without changing the pack's total 915-vblank duration.
`VISITOR 6` uses the generic normal/far-left/far-right multi-view stitch
with production-variable island placement, plus a narrow full-host
impact-delta injection for source frames 120:141. Foreground-only capture
keeps Johnny and the coconut clean but omits the background-owned tree
shake/strike pixels, so only that proved live window is copied from the
full-host capture.
`VISITOR 7` uses the generic normal/far-left/far-right multi-view stitch
with production-variable island placement. The captured coconut/tree
impact pixels were present, but dedupe left the strike rows too short, so
source frames 32, 62, 71, and 80 now receive redistributed hold time while
the total 1355-vblank scene duration stays unchanged.
`WALKSTUF 3` validates the existing on-disc `WALK3.FG2` / `WALK3LOW.FG2`
packs on the normal high-tide/night route with visual + audible signoff;
no pack/runtime rework was needed.

| Component | Status |
|---|---|
| Build system (Docker + CMake + mkpsxiso) | Complete |
| CD-ROM I/O (`cdrom_ps1.c`) | Complete |
| Graphics layer (`graphics_ps1.c`) | Complete |
| Input layer (`events_ps1.c` + `spi.c`) | Complete — direct SPI driver replaces the broken BIOS pad path |
| Resource system (hashed + LRU) | Complete |
| Scene playback (fgpilot, `foreground_pilot.c`) | Primary render path; 63/63 scenes fully validated |
| Story-loop walks (`walk_pilot.c`, `walk_render.c`) | Working — Johnny walks between scene endpoints with wave motion, holiday re-stamping, palm-tree cover-up, and a persistent 340x224 erase buffer; the release candidate survived a ~10-minute DuckStation soak with no `JCBSOD` or `JCWALK` allocation failures |
| Freeplay/debug mode (`scene_freeplay.c`) | Working — menu-launched direct-control Johnny with D-pad/analog movement, L2/R2 speed modifiers, fishing, immediate R1+D-pad world toggles, gag/visitor catalogs, sound test, Select clear-screen rebuild, frog-clock loading transitions, and no steady-state frame allocations |
| Audio layer (`sound_ps1.c`) | Working — VAG preload at boot + round-robin SPU voices + captured SFX replay; mute via direct SPU register writes (`SpuSetCommonMasterVolume` is not honored by DuckStation HLE) |
| Telemetry / debug overlay | Complete |
| Perf instrumentation (`ps1_perf.c`) | Complete — level-gated `JCPERF2` TTY summary by default; legacy `JCPERF` can be re-enabled at compile time with `PS1_PERF_LEGACY_TRACE=1`; levels remain OFF/SUMMARY/DETAIL/DEBUG via `ps1PerfSetLevel` |
| Pause menu (`pause_menu.c`) | Complete — Start opens overlay; custom 8x8 font (FntFlush is empirically broken in scene-runtime context); POLY_F4 dim + panel quads |
| User settings persistence (`memcard.c`) | Working / expanding — v6 saves persist holiday mode separately from manual holiday id; broader menu-option persistence remains future work |
| TTY printf | Reliable on PSn00bSDK + DuckStation as of 2026-04-25 |

## Scenes: 63 / 63 fully validated

The per-scene ledger lives in [scene-status.md](scene-status.md). That
file is the source of truth for what is complete under the current bar;
this page gives the narrative around it.

Latest point release: `v0.8.16-ps1` is the memory-region allocator stability
release after the `v0.8.15-ps1` WALKSTUF1 high setup-segment performance
promotion. All 63 scenes remain validated, all 126 high/low variants are routed
and timing-bearing, and MARY1/2/3 plus SUZY1/2 are measured; `suzy3` is not a
standalone scene route. The allocator route now separates BOOT, CACHE, and
TRANSIENT lifetimes, wipes scene-boundary transient allocations, reuses CACHE
storage through free-list/LRU paths, and gates every `memAlloc` call site with a
nearby `MEM_REGION_RATIONALE`. The current allocator matrix battle card is
`+0.5699%` over target / `99.4843%` target speed, raw signed `-0.1470%` /
`100.2147%`, with `118` green, `4` yellow, `2` orange, and `2` red rows.
Earlier pack/data wins remain in force: the MARY3
guarded prefetch-preserve follow-up moves high/low from `2402/2295` and
`2402/2296` to `2296/2294` and `2297/2295`, collapsing blocking
`690/693 -> 53/51` while keeping hidden refill overrun at zero. The latest
post-release BUILDING1 pass converts both packs to compact FGP3/v4 while
skipping auto-resident setup prime for that scene: high moves `792/778 ->
784/782`, low moves `794/779 -> 787/782`, and both rows are now green. The
newer VISITOR5 high-only compact-FGP3/no-autoprime pass keeps low tide on the
prior pack while moving high `1111/1090 -> 1104/1092`, cutting overrun
`21 -> 12`, blocking `12 -> 11`, and loop-read time `123 -> 91`. The newer
BUILDING2 low restore-minus-current/slack-4 pass moves low `1383/1304 ->
1349/1316`, cutting blocking `118 -> 83` and hidden refill `5 -> 1`. The
newer WALKSTUF3 high compact-FGP3/v4 pass moves current-layout control
`2325/2282 -> 2310/2290`, cutting overrun `43 -> 20`, blocking `65 -> 47`,
hidden refill `32 -> 18`, and loop reads `74 -> 37`. The newer BUILDING6
compact-FGP3/v4 pass converts both packs inside the original `1444370` byte
footprint, moves high `2520/2442 -> 2482/2457` and low `2515/2437 ->
2485/2456`, cuts blocking `62/70 -> 25/28`, hidden refill `64/66 -> 27/29`,
loop reads `74/73 -> 42/42`, and keeps LBAs `10754/11460`. The latest
ACTIVITY9 high compact-FGP3/v4 pass keeps the original `1745484` byte
footprint and LBA `3108`, moves high `2094/2056 -> 2082/2062`, cuts blocking
`37 -> 24`, hidden refill `23 -> 17`, and loop reads `52 -> 25`. The later
ACTIVITY9 low compact-FGP3/v4 pass keeps the original `1745484` byte footprint
and LBA `3961`, moves low `2085/2058 -> 2075/2061`, cuts blocking `29 -> 17`,
loop reads `59 -> 47`, loop-read time `289 -> 232`, and due misses `3 -> 1`.
The latest WALKSTUF3 low compact-FGP3/v4 pass
keeps the original `994669` byte footprint and LBA `26906`, moves low
`2321/2293 -> 2310/2295`, cuts blocking
`41 -> 26`, hidden refill `21 -> 17`, loop reads `72 -> 29`, and moves low into
green. The latest JOHNNY1 compact-FGP3/v4 pass keeps both original `448370`
byte footprints, moves high/low `1977/1943 -> 1974/1945`, cuts blocking
`31 -> 26`, hidden refill `31 -> 26`, loop reads `16 -> 7`, loop-read time
`95 -> 56`, and keeps LBAs `13982/14201`. The VISITOR3 timing stack first moved
high frame `140` plus tail frames `142..144` into the already paid high setup
segment instead of adding another setup-prime cap. It preserves the `1555450`
byte pack footprint, kept the then-current LBAs fixed, and holds
the `217088` byte PS-EXE bucket. High improves `1075/1037 -> 1074/1038`,
overrun `38 -> 36`, blocking `61 -> 58`, loop reads `11 -> 10`, loop-read time
`61 -> 58`, and due misses `11 -> 10`; hidden refill remains `0`. The v299
high frame-131 residency pass aliases duplicate frames `121`/`123` to frame
`120`, compacts the resident setup-prime tail, copies frame `131` fully into
paid setup-prime coverage, and improves high again to `1070/1039`, blocking
`49`, loop reads `9`, loop-read time `49`, and due misses `9`. The v464
one-sector setup segment for the frame `132` D4 payload moves high to
`1067/1039`, blocking `45`, loop reads `8`, loop-read time `45`, and due
misses `8`. The follow-up
low no-op alias pass points frames `114..117` at frame `113`'s already-resident
two-byte payload and improves low `1079/1039 -> 1075/1039`, blocking
`70 -> 67`, loop reads `16 -> 12`, and loop-read time `84 -> 67`; due misses
stay `12` and hidden refill stays `0`. The v302 low resident-segment pass
aliases duplicate frame `123` to frame `121`, expands the second setup segment
from `24` to `27` sectors, copies frame `128` resident, and improves low again
to `1071/1039`, blocking `63`, loop reads `11`, loop-read time `63`, and due
misses `11`.
The v629 pack-only high follow-up keeps the sector-203 frame132/frame137 setup
baseline, then reuses the proven low-tail compact cleanup payloads and repacks
frames `141/140/142/143/144` plus sound events into the existing `277..293`
setup segment. High improves to `1063/1040`, overrun `23`, `blocking_vb=35`,
loop reads `6`, loop-read time `35`, due misses `6`, and hidden refill `0`.
The v327 low resident-slot swap moves frame `129` into the already paid frame
`128` segment2 slot while frame `128` points back to its original cold payload.
Low improves to `1072/1040`, blocking `64`, loop reads `11`, loop-read time
`64`, and due misses `11` without changing pack size or LBA.
The v338 low tail pack-only pass keeps the same public `1072/1040` timing but
compacts frame `143` cleanup and moves frame `144` terminal cleanup into the
already paid setup window. VISITOR3 low now reports `blocking_vb=58`,
`loop_reads=10`, loop-read time `58`, and due misses `10`; non-VISITOR3
controls stayed flat.
The later v452/v470 low D4 passes move frame `129` to a 609-byte previous-frame
delta and frame `132` to a 768-byte previous-frame delta; v477 relocates that
frame132 payload into an unused setup-prime gap. Low now reports `1065/1041`,
overrun `24`, `blocking_vb=45`, loop reads `8`, loop-read time `45`, due
misses `8`, hidden refill `0`, and fixed pack LBA/sectors.
The v340/v383/v384/v428/v474/v598 WALKSTUF1 read-group passes add `344..360`,
`422..434`, `427..443`, `443..455`, and `444..456` after the accepted
`201..213` and `213..229` groups, then extend the first low-tide post-prime
retained boundary to `78..91`. High now uses the setup-segment `242..388`
baseline at `1476/1441`, overrun `35`, blocking `49`, hidden refill `17`,
loop reads `37`, loop-read time `182`, and due misses `7`. Low now improves to `1477/1432`, overrun `45`, blocking `65`,
hidden refill `20`, loop reads `58`, loop-read time `259`, and due misses
`11` after the low-tide-only `394..410` retained-read group, the v859 frame87
fixed-sector payload shrink, and the same-speed v860/v861/v862/v863 frame89/frame98/frame27/frame101 payload shrinks.
The BUILDING2 high row keeps the accepted `60..72`, `206..230`, and `226..242`
retained groups, replaces the old tail row with the `83..95` scheduler row,
adds guarded `271..287` plus `315..327`, then adds `{185..197}` as same-speed
CD-pressure work,
and primes allocator-safe CACHE slices at relative sectors `3..35` and
`202..242` during setup. On current HEAD it measures
`1347/1313`, overrun `34`, blocking `39`, refill overrun `16`, loop reads `43`,
loop-read time `197`, and due misses `5`. It also carries the v664 late-only offscreen work-volume clip
for frames `168..177`, the v698 post-hot tail clip for frames `94..104`, the
v700 frame `92` clip, the v701 frame `91` clip, the v702 frame `90` clip, the
v703 frame `89` clip, and the v877/v879/v880 preserve-offset entry `172` /
source frame `231`, entry `171` / source frame `228`, entry `96` / source
frame `119`, and the current entries `92`/`94`/`95` payload trims. All keep
timing/CD exact-flat while dropping runtime frame rows/spans/pixels from
`18144/110717/468636 -> 18030/105645/446246` and active payload
`674798 -> 663590`.
BUILDING2 low now keeps the v626 slack-8 `218..229` row, v660 offscreen
draw-span clip, and v739 draw-tail trim ahead of the accepted low groups, then
primes relative sectors `112..128` and `226..262` during setup with a low-only
`80 KiB` clean-strip cap, slack-5 window guard, and `{141,153}`. It now measures
`1327/1318`, overrun `9`, blocking/refill `47/0`, reads `27`, and due `9`.
VISITOR3 high now adds the v629 tail-pack repack, keeping the v501 D4/setup
baseline while fitting frames `141/140/142/143/144` plus sound events into the
existing `277..293` setup segment. It improves to `1063/1040`, overrun `23`,
blocking/read time `35`, and reads/due `6/6`.
BUILDING4 low now clips offscreen PAL4 draw spans directly in the pack,
improving to `2853/2816`, overrun `37`, blocking/read time `40`/`215`, and
prefetch overrun `34` while preserving fixed pack footprints and the `217088`
byte PS-EXE bucket. The current dirty-upload band retune then widens the
clean-row merge gap to `8`, moving BUILDING4 low to `2849/2816`, overrun `33`,
blocking/refill `38/31`, read time `222`, and due `1`; the latest `24 KiB`
stream-window knee moves it into green at `2847/2820`, overrun `27`,
blocking/refill `32/27`, read time `252`, and due `1`. WALKSTUF1 high/low now also carry late-tail work-volume
clips, high v837 adds frame55/frame138/frame51/frame49/frame47/frame45/frame43/frame56/frame57/frame136/frame135/frame139 offscreen clips, and high v839/v841/v842/v843/v844/v882/v884 shrink frame51/frame49/frame47/frame45/frame43/frame138/frame135 in place with preserved offsets. High now primes relative sectors `242..388` during setup and measures
`1476/1441` with blocking/refill `49/17`, reads/due `37/7`, while dropping
runtime rows/spans/pixels to `16859/129919/731016` and active payload to
`860009`. Low v817 adds the
`394..410` retained read group after the v653/v705/v802/v846/v847/v849/v852/v853/v855/v859/v860/v861/v862/v863/v864/v865/v866/v867/v868/v869/v870/v871/v872/v873/v874/v875/v876 work-volume chain and
improves to `1477/1432` with blocking/refill `65/20`,
loop reads/read VBlanks `58/259`, and due `11`, while v846/v847/v849/v852/v853/v855/v859/v860/v861/v862/v863/v864/v865/v866/v867/v868/v869/v870/v871/v872/v873/v874/v875/v876 trim frames
`53`, `136`, `79`, `81`, `129`, `139`, `87`, `89`, `98`, `27`, `101`, `93`, `94`, `97`, `99`, `100`, `134`, `91`, `92`, `95`, `140`, `108`, `109`, and `107` in place and drop active payload to `790322` without moving offsets, still carrying the
late-tail draw reduction that dropped `39072` draw
pixels, `4263` spans, `313` draw rows, `79` dirty rows, and `50560` upload
bytes. The v665/v666/v668/v669/v672/v673/v674/v675/v678/v680/v684/v685/v686/v687/v688/v689/v690/v691/v692/v693/v694/v695/v696 follow-ups clip isolated low mid/left/pre-tail/mid-right/pre-left-edge/post-left/late-left2/frame65/post-left-singleton/mid-right-ad/ae/af/frame1/post-mid/frame3/frame140/frame61/frame60/frame62/frame59/frame58/frame63 offscreen clusters,
also exact-flat, dropping runtime frame rows/spans/pixels from
`17298/135025/785455 -> 16272/116912/650623`. The latest W1-high speed
follow-up clips entries `62..66` and adds `{92..108}` plus `{272..284}` read
groups after the no-`144` mid-cluster/frame138/active-loop/early offscreen
headroom passes. It improves high from `1469/1440` to `1469/1441`, overrun
`29 -> 28`, blocking/refill `42/12 -> 41/11`, reads/due `41/7 -> 37/6`, and
drops high runtime rows/spans/pixels `16547/121551/660244 ->
16547/120919/658340`. BUILDING4 low v827 now carries
the same-speed no-shift payload lane through frame `286`, keeping
`3128/2853/2816`, blocking/refill `40/34`, read VBlanks `215`, and due `1`
while dropping active payload `855284 -> 810226`; the current gap-8 dirty-upload
band merge retune moves the active row to `2849/2816` without changing that
pack layout, and the latest `24 KiB` B4-low window retune moves the row into
green at `2847/2820` without moving pack LBA or the PS-EXE bucket. The JOHNNY1 local-LZ payload
swing now compresses entries `1` and `50` in both high/low packs, cutting
active payload `316608 -> 112093` bytes and moving both tides from
`1973/1945` to `1948/1945`. WALKSTUF1 low now primes relative sectors
`238..344` during setup after low-only 48 KiB clean-rect chunking, then adds
the `{91,107}` first-boundary read group and a split TRANSIENT `344..350`
setup edge, improving the current allocator-era row `1479/1435 -> 1473/1447`,
overrun `44 -> 26`, blocking/refill `65/18 -> 41/7`,
reads/read time `50/230 -> 36/186`, and due `10 -> 5`;
then trims frame132's draw tail in the compacted low pack and adds `{378,390}`
to reach `1470/1446`, overrun `24`, blocking/refill `34/6`,
reads/read time `30/159`, and due `4`; the latest setup/owner pass shifts the
main setup residency to `244..350`, keeps a split `179..185` edge, and adds
`{113,129}` to hold `1470/1446` while improving scene `1812 -> 1809`,
blocking/refill `34/6 -> 33/5`, and reads/read time `30/159 -> 26/150`; the
same-speed `{355,371}` follow-up keeps timing flat and lowers reads/read time
again to `24/146`; the fresh-owner `160..176` pocket then keeps speed and read
time flat while lowering blocking/refill to `32/4`.
Setup stays inside the canary allowance. WALKSTUF1 high keeps the `198..244`
setup slice and retargets the second retained slice from `411..435` to
`286..344`, adds `{149,165}`, encodes frame `92` as D4, and now adds
`{423,439}`, `{404,416}`, `{395,411}`, and retargeted `{411,423}` as same-speed CD-work rows. The D4/read-owner track improves the
allocator-era row `1475/1433 -> 1471/1440`, overrun `42 -> 31`,
blocking/refill `76/15 -> 56/13`, loop reads/read time `55/229 -> 45/209`,
and due `15 -> 10`; `{423,439}` plus `{404,416}` lower loop reads/read time
again `42/205 -> 41/200`, and the newest prepare-first scheduler row moves the
current row to `1472/1441` while cutting blocking/due `56/10 -> 43/7` and
keeping overrun/refill flat at `31/13`; `{395,411}` lowers loop read work, and
the newest `{411,423}` replacement keeps the same timing exact-flat while
reducing loop reads/read time `42/201 -> 41/198`.
BUILDING4 low now carries the v971 local-LZ entry270
follow-up on top of entry30/entry33, cutting active payload
`807263 -> 799277` and improving to `2851/2815`, overrun `36`, and refill
`35`. The visible-speed guarded read-group pass improves BUILDING2 high active
loop `1351 -> 1347`, overrun `38 -> 34`, blocking `50 -> 41`, read time
`207 -> 203`, and due `7 -> 6`; the accepted hidden-refill tradeoff is
`14 -> 16`. The newest same-loop guarded row adds `315..327`, keeping
`1347/1313`, overrun `34`, and refill `16` flat while reducing blocking
`41 -> 39`, reads `47 -> 45`, read time `203 -> 199`, and due `6 -> 5`;
the follow-up `{185..197}` row keeps speed/blocking/refill flat while reducing
reads/read time `45/199 -> 43/197`, and the newest `{158..174}` row keeps the
same timing exact-flat while lowering CD work again to `40/189`.
After the allocator refresh, BUILDING2 high guarded read-group pressure promotions,
the BUILDING4 low gap-8 dirty-upload band retune, the W1-high `286..344`
pressure promotion, the VISITOR3 low frame138 raw-gap promotion, the
W1-low clean-rect/setup-edge promotion, the W1-low `{91,107}` first-boundary
read-group promotion, the W1-low split `344..350` setup edge, the VISITOR3 high frame56/57 raw-gap plus tight-refill
promotion, the BUILDING2 low `226..238` setup-segment promotion, the
VISITOR3 high `56 KiB` cap promotion, the BUILDING2 low `226..262` +
clean80 green promotion, the VISITOR3 high `64 KiB` clean-strip cap, the
BUILDING4 low `24 KiB` stream-window green promotion, the W1-low frame132
payload trim, the W1-low `{378..390}` read-group promotion, and the W1-low
same-speed `{355..371}` read-work row, the W1-high frame56/`{178..194}`
CD-pressure promotion plus `{423..439}`, `{404..416}`, `{395..411}`, and retargeted `{411..423}`, W1-high prepare-first
scheduler ownership, the VISITOR3-high 80 KiB clean-relief
window promotion, and the BUILDING2-high entries `92`/`94`/`95` trim plus
`{185..197}` and `{158..174}` same-speed CD work, W1-low fresh-owner
`160..176`, the hot foreground scene-ID cache source-headroom pass, the
W1-high and VISITOR3-low fixed-layout cleanup headroom passes, the W1-low
frame `87..99` cleanup-slack promotion, the VISITOR3-low frame `135`
gap-placed D4 speed promotion, and the VISITOR3-low `88..104`, `72..88`, plus `16..32` read-group
speed promotion, plus the VISITOR3-high entry `62` cleanup-only headroom pass,
the W1-low entry `90..99` fixed-layout canonicalization pass,
the public battle card is now
`+0.1956%` over target / `99.8062%` target speed; the raw signed optimization
rollup is `-0.5213%` / `100.5366%`.
Since the compact full-matrix baseline was about `17.4%` over target /
`87.1%` target speed, the headless methodology has removed about `17.20`
public over-target points and added about `12.70` public target-speed points.

Prior point release: `v0.8.2-ps1` is the VISITOR3 guarded-read performance
release. All 63 scenes remain validated, all 126 high/low variants remain
routed through headless perf, and the VISITOR3 restore-minus-current
pack baseline preserved pack LBAs and the `215040` byte PS-EXE bucket. The
subsequent BUILDING4 restore-minus-current pack pass, VISITOR3 low read-group
prune, WALKSTUF1 high setup-prime retune, VISITOR3 high/low offscreen draw
clips, BUILDING2 high restore-minus-current pass, BUILDING2 grouped-read
passes, VISITOR3 low code-shape pass, and VISITOR3 v4 draw-tail trim
stageguard pass are now superseded by the current public-capped `v0.8.7`
rollup above.
See
[release-notes-0.8.7.md](release-notes-0.8.7.md).

Current performance baseline: VISITOR3 uses cleanup-compact FGP3 data plus
FGP3/v4 compact PAL4 draw metadata, an inlined compact metadata decoder, and
pack-side cleanup spans with current-frame redraw coverage removed plus scoped
`-Os` background composite helpers, v4 draw-tail trimming, a VISITOR3 stage
guard, and the motion-copy FGP3 payload for yacht translation frames `119..123`
plus high-tide frame `115`, shared frames `118`/`124`, high-only frame `117`,
and high-only re-anchored frames `127`/`126`/`125`, plus high/low persistent setup segments
for sectors `277..293` and `281..305`, the v214 high setup-prime cap
expansion, the v216 guarded low segment `150..174`, the v227 low resident
frame-125/frame-126 re-anchor plus v234 frame-118 and v237 frame-127 resident
copies, the v238 high frame-127/frame-130 resident-copy compaction, the
v248 low frame-114/frame-117 no-op residual compaction, the v249 low
frame-113 no-op residual compaction, the v291 high frame-140/tail
setup-segment copy, the v292 low frame-114/frame-117 no-op payload alias, the
v299 high frame-121/frame-123 resident alias plus frame-131 setup-prime copy,
and the v302 low frame-123 resident alias plus frame-128 setup-segment copy,
plus the WALKSTUF1 high v288
gap1/window-prefetch guard and low v289 gap1 prefix pack, the VISITOR3 low
frame129/frame132 D4 deltas plus the frame132 setup-prime gap relocation, and
the VISITOR3 high frame132/frame137 D4 deltas with a high-tide slack4 window
guard plus a one-sector high setup segment for frames `132` and `137`, and the
v629 high tail-pack repack, plus the allocator-era VISITOR3 low third setup
segment for sectors `206..230`, the VISITOR3 high frame139 raw relocation into
the retained `203..229` gap, the VISITOR3 high third setup segment for
sectors `228..262`, the VISITOR3 low frame138 raw-gap relocation inside
the extended `206..232` third segment, and the VISITOR3 high frame56/frame57
raw relocation into the retained `228..262` gap with the `56 KiB` tight-refill
cap, followed by the high-only `64 KiB` clean-strip cap and the `80 KiB`
clean-relief stream-window retune.
VISITOR3 high is now
`1071/1045` with `blocking_vb=35`; low is
`1065/1039` with `blocking_vb=75`. BUILDING2 high/low are `1347/1313` and
`1327/1318`, ACTIVITY9 high/low are `2082/2062` and `2075/2061`, WALKSTUF1
high/low are now
`1472/1441` and `1470/1446`, WALKSTUF3 high/low are `2310/2290` and
`2310/2295`, JOHNNY1 high/low are both green at `1948/1945`, and the FISHING1 high control sits at the public cap
(`1068/1072`, raw signed under target). BUILDING4 now uses the same pack-side
restore-minus-current cleanup, with low-tide offscreen draw-span clipping on
top plus the gap-8 dirty-upload band merge retune: high is `2843/2816` with
`blocking_vb=34`, and low is now green at `2847/2820` with `blocking_vb=32`. The earlier WALKSTUF1 high
`144 KiB` setup-prime retune is superseded by the compact-pack baseline.
JOHNNY2 and related current-pack clean-pressure work are preserved in the
matrix; the next true outliers are residual WALKSTUF1 work, BUILDING2 high
residual work, VISITOR3 high/low, and selective
upload-ready bands.
VISITOR3 local C
read-table rows, threshold-only fallthrough probes, and terminal tail-atlas
repoints are now exhausted; the next VISITOR3 attempt should extend the
scene-owned motion/precomposed-data path, add a true residual dictionary, or add
generated scheduler ownership.

The preprocessing opportunity matrix now includes x-band rect totals, cap
hits, rects per frame, and exact-upload interval counts. VISITOR3 remains the
top graphics-preprocess target, but both tides show x-band cap pressure, so the
next upload-ready experiment should be selective/thresholded rather than a
blanket conversion. The per-frame preprocess analyzer now supports FGP3
temporal-residual packs directly: for VISITOR3 it separates the cap-hit crash
frames (`134..136`, where blanket x-band saves `0%`) from the profitable
non-cap frames (`121..133`, where align4 x-band saves roughly `65..75%`). The
first tracked threshold plan is
[performance-preprocess-visitor3-hotspots.csv](performance-preprocess-visitor3-hotspots.csv):
it selects `92 / 144` high-tide frames, excludes `6` cap-hit frames, and
estimates `5730024` upload bytes saved inside the selected subset. The same
current size gate rejects the default upload-ready append as layout-neutral:
the payload plus rect metadata would need `2111224` bytes against only
`970076` bytes of padded high-pack zero-tail slack. The analyzer now also emits
a same-footprint budgeted subset for that exact slack: `78 / 92`
default-selected frames fit in `968904` payload+rect bytes, leave `1172` bytes
of slack, and retain `4232112` modeled upload bytes saved. Raw foreground-only
payloads remain unsafe because the selected x-bands have no guaranteed
draw-covered pixel source, so the next VISITOR3 graphics probe needs a safe
background-owned/precomposed payload source, compression plus ownership, a
shrinking pack transform, or a deliberate layout-moving experiment with full
canaries.

Milestone releases:
- `v0.8.16-ps1` — memory-region allocator stability release. Promotes
  BOOT/CACHE/TRANSIENT allocation lifetimes, scene-boundary TRANSIENT wipes,
  CACHE free-list/LRU reuse, generated pack-header metrics, and allocator
  rationale gates. The R34 full matrix remains `126/126` PASS with 0 BSODs.
  Public rollup after the allocator refresh is `+0.5699%` over target /
  `99.4843%` target speed, raw signed `-0.1470%` / `100.2147%`, with `118`
  green, `4` yellow, `2` orange, and `2` red rows.
- `v0.8.15-ps1` — WALKSTUF1 high setup-resident CD promotion. High tide primes
  relative sectors `242..388` during setup, improving the active loop/target
  from `1481/1428` to `1476/1441`, overrun `53 -> 35`, blocking/refill
  `88/24 -> 49/17`, reads/read time `67/287 -> 37/182`, and due `15 -> 7`.
  Public rollup is `+0.2285%` over target / `99.7746%` target speed with `119`
  green and `7` yellow rows.
- `v0.8.14-ps1` — JOHNNY1 local-LZ green promotion. Both JOHNNY1 tides compress
  full-frame entries `1` and `50` inside the existing `448370` byte high/low
  packs, preserve pack LBA/sectors and the `217088` byte PS-EXE bucket, and move
  both rows to `1948/1945`, overrun `3`, blocking/refill `5`, and target speed
  `99.85%`. Public rollup is `+0.2492%` over target / `99.7548%` target speed
  with `119` green and `7` yellow rows.
- `v0.8.13-ps1` — under-99 payload-work checkpoint. WALKSTUF1 low advances to
  `walkstuf1-low-frame106-inplace-v910`, BUILDING2 high advances to
  `building2-high-frame173-inplace-v914`, BUILDING4 low advances to
  `building4-low-frame283-inplace-v913`, and recent JOHNNY1/B2-high/W1-low
  misses are logged and closed. Public rollup is `+0.2697%` over target /
  `99.7347%` target speed with `117` green and `9` yellow rows.
- Current `main` after `v0.8.8-ps1` — promotes the BUILDING2 high `206..230`
  and `226..242` read groups plus 24-sector grouped-read capacity, the
  BUILDING6 scene-local slack4 guard, WALKSTUF1 high
  `344..360`, `422..434`, and shared dual-tail `443..455` / `444..456`,
  WALKSTUF1 low staged-prepare scheduler fallback, VISITOR3 low tail
  pack-only compaction plus frame129/frame132 D4 and frame132 setup-prime gap
  relocation, and VISITOR3 high frame132/137 D4 plus one-sector frame132 setup
  residency plus frame137 co-residency, plus WALKSTUF1 low `78..91` retained
  post-prime boundary ownership and shared `427..443` CD-work reduction, and
  VISITOR5 high/low `30..46` retained-read ownership, and BUILDING2 low
  `218..229` slack-8 retained-read ownership plus v739 draw-tail trimming, the
  VISITOR3 high `277..293` tail-pack repack, BUILDING4 low offscreen draw-span clipping, BUILDING2 low
  offscreen draw-span work-volume clipping, BUILDING2 high late/post-hot offscreen
  work-volume clipping, and WALKSTUF1 high/low late-tail plus high frame55/frame138/frame51/frame49/frame47/frame45/frame43/frame56/frame57/frame136/frame135/frame139 offscreen, high frame51/frame49/frame47/frame45/frame43/frame138/frame135 in-place payload shrinking, and low mid/left/pre-tail/mid-right/pre-left-edge/post-left/late-left2/frame1/post-mid/frame3/frame140/frame61/frame60/frame62/frame59/frame58/frame63 offscreen
  work reductions plus the WALKSTUF1 low `394..410` retained-read promotion
  plus frame53/frame136/frame79/frame81/frame129/frame139/frame87/frame89/frame98/frame27/frame101/frame93/frame94/frame97/frame99/frame100/frame134/frame91/frame92/frame95/frame140/frame108/frame109/frame107 no-shift payload shrinking, BUILDING2 high frame172/frame171/frame96 no-shift payload shrinking, the JOHNNY1 local-LZ full-frame payload compression, and the WALKSTUF1 low `209..225` retained-read row.
  The current public battle card is `+0.1956%` / `99.8062%` with `124` green, `2` yellow, and `0` orange rows;
  BUILDING2 high is now green at `1330/1317` with blocking/read/due `32/33/4`, BUILDING2 low now measures
  `1327/1318` after trimming active payload `660236 -> 538534` and priming relative sectors `112..128` plus `226..262` with clean80/slack5 shaping, BUILDING2 high now banks previous-visible cleanup, screen-clip, trim-tail work, safe-tail trims, and fixed-footprint physical compaction to
  active payload `520974`, WALKSTUF1 high is
  `1462/1445` with blocking/refill `35`/`5`, loop reads/read time `36`/`182`, rows/spans/pixels
  `16540/120651/654904`, and dirty rows `28303` after the frame0 consume speed promotion, the `200..215` previous-visible cleanup headroom pass, the prep2 `128..191` frame-gate promotion, the `185..191` direct-stage rescue, the `383..399` transient setup-slice / clean-cap speed pass, the `183..199` late-layout / `372..384` owner refill-headroom pass, the `372..388` fresh-owner retarget speed pass, frames `189..191` direct-stage headroom pass, entry134 screen-clip headroom pass, `{108..124}` CD-pressure pass, entry `62..66` clip plus `{92..108}`/`{272..284}` read-group speed pass, entry `58..61` tail-trim/phase-3 speed pass, and no-`144` mid-cluster/frame138/active-loop/early offscreen clip headroom passes, WALKSTUF1 low is now green at `1461/1447`
  with blocking/refill `31`/`2`, loop reads/read time `22`/`117`, and active payload
  `708288` after the compact trim/retarget phase pass, VISITOR3 high is
  `1065/1046` with blocking/read count `34`/`8`,
  VISITOR3 low is `1062/1045` with blocking/read count `38`/`12`, JOHNNY1 high/low are
  green at `1948/1945`, BUILDING4 low is
  green at `2847/2820` with blocking/read time `32`/`252`, and VISITOR5 high/low are
  now green at `1101/1096` and `1102/1097`.
- `v0.8.7-ps1` — deterministic BOOTMODE scene selection and Scene Explorer
  preview stability. Adds auditable direct-scene boot logging, expected-scene
  gates for headless perf runs, Suzy backdrop cleanup hardening, and heapless
  Scene Explorer thumbnail streaming while preserving the full 126-row
  `+0.3156%` / `99.6902%` public battle card.
- `v0.8.6-ps1` — WALKSTUF1 / VISITOR3 performance follow-through. Promotes
  WALKSTUF1 low gap6-prefix + slack-guard, WALKSTUF1 high window-prefetch /
  slack4 guard, and VISITOR3 high/low setup-segment resident copies for frames
  `131` / `128`.
- `v0.8.5-ps1` — full 126-row headless performance matrix baseline. Keeps
  the `v0.8.4` thumbnail/content work, records 126/126 timing-bearing rows,
  and publishes `+0.3184%` over target / `99.6876%` target speed.
- `v0.8.4-ps1` — custom Scene Explorer thumbnails for all 63 scenes, plus
  scene titles and bodies reconciled against what the discs actually play.
- `v0.8.3-ps1` — WALKSTUF1 compact foreground performance. Converts both
  WALKSTUF1 tides to padded compact FGP3/v4 restore-minus-current packs,
  reducing high/low active loop time and visible CD blocking while preserving
  CD layout and the validated 63-scene bar.
- `v0.8.2-ps1` — VISITOR3 guarded-read performance and docs/site sync. Lowers
  VISITOR3 high visible CD pressure while preserving loop cadence and keeps the
  validated 63-scene bar intact.
- `v0.8.1-ps1` — clean-rect pressure stability. Large ocean clean snapshots
  now estimate the real pressure footprint before allocation, including
  wave-band expansion and upper/lower split rects; the fix covers the
  random-position scenes found by the scan and keeps the `v0.8.0` performance
  baseline compatible.
- `v0.8.0-ps1` — complete-scene performance baseline. Keeps the 63/63
  visual + audible scene ledger intact, promotes the current 126-variant
  headless battle card, and fixes a randomized BUILDING4 clean-rect pressure
  BSOD by releasing stale walk-clean buffers before retrying the large scene
  clean snapshot.
- `v0.7.2-ps1` — story-loop walking bugfix. Inter-scene walks now require the
  remembered backdrop key to match the next scene's tide, raft, night,
  holiday, and island X/Y state; otherwise the runtime skips the walk and lets
  the scene reload its own background.
- `v0.7.1-ps1` — holiday-mode point release. Adds memory-card schema v6 with
  `holidayMode`, exposes `AUTO DATE:ORIG4` / `AUTO DATE` / `NONE` /
  `ORIGINAL 4` / `EXPANDED` as distinct policies, migrates old auto saves to
  the original-four default, and fixes holiday overlay z-order during
  story-loop walks.
- `v0.7.0-ps1` — first complete-scene release. All 63 original Johnny
  Castaway scenes are validated under the current visual + audible signoff
  bar. `ACTIVITY 9` completed the sweep with an Activity9-specific wide stitch,
  source `BOAT.PSB` bow/stern repair at the legacy clip edges, a narrow overlap
  band to remove the boat seam, and metadata-held boat draw carry-forward so
  the late bow remains stable.
- `v0.6.13-ps1` — promotes `VISITOR 4`, `VISITOR 5`, `VISITOR 6`, and `VISITOR 7`.
  VISITOR4 is the coconut/plane gag in the current scene mapping, correcting
  the old red-boat catalogue label. VISITOR5 regenerates high/low packs through
  the generic multi-view stitch and redistributes hold time into the coconut
  impact / downed-plane rows. Total scene duration remains 915 vblanks.
  VISITOR6 regenerates high/low packs through the same multi-view path and
  injects only the full-host source frames 120:141 deltas needed for the
  background-owned tree shake/strike pixels. VISITOR7 regenerates high/low
  packs through the same multi-view path and redistributes hold time onto
  source frames 32, 62, 71, and 80 so the coconut/tree impact frames read
  without changing total scene duration.
- `v0.6.12-ps1` — promotes `STAND 2`-`STAND 12`,
  `STAND 15`, `STAND 16`, `SUZY 1`, `SUZY 2`, `VISITOR 1`, `VISITOR 3`,
  and `WALKSTUF 3` after visual signoff. The STAND
  scenes passed the normal high-tide/night route. `STAND 5`-`STAND 9`,
  `STAND 15`, and `STAND 16` ride the STAND no-stitch
  export fast path with a full-frame
  foreground-only overlay the pure base-diff shortcut needed after it
  faded Johnny's legs on `STAND 5`. `STAND 8` also lands a runtime
  fix: the FG2 frame loop now ticks ocean wave animation every frame
  so no-stitch scenes — which carry no captured water frames in their
  foreground-only pack — get moving water from the engine instead of
  the static `OCEAN00.SCR` bg. Scenes whose pack carries its own water
  frames are unaffected because the foreground compose still draws on
  top. The scene-loader direct launch path now skips the stale
  story walk before the frog/full-wipe transition, and high-pressure
  clean snapshots release optional caches before allocation. `SUZY 1`
  also proves the scene-specific-backdrop route: its source beach screen
  is loaded from `SUZBEACH.SCR`, not painted as the standard island/ocean
  scene. `SUZY 2` keeps the scene-local `MRAFT.BMP` raft in a
  static-base foreground overlay and uses SFX mixer headroom so its
  overlapping raft samples play without clipping. `VISITOR 1` uses the
  standard multi-view stitch and validates the Lilliputian arrival with
  one captured SFX event. `VISITOR 3` adds a scene-specific ship-crash
  synthesis helper so the red hull accumulates only during live crash
  frames and the final splash remains readable without replaying stale
  full-host residue. `WALKSTUF 3` validates the existing on-disc
  `WALK3.FG2` / `WALK3LOW.FG2` packs on the normal high-tide/night
  route without rework.
- `v0.6.11-ps1` — promotes `MISCGAG 1`, `MISCGAG 2`, and `STAND 1`
  after regenerating high/low packs through the generic multi-view
  stitch. `STAND 1` is a short 169-vblank idle loop with no captured SFX
  events; the subtle reset is expected.
- `v0.6.10-ps1` — scene-validation bugfix release; promotes `MARY 5`
  after generic multi-view stitching rebuilt high/low packs with the
  generic raft off. Direct scene playback now honors story flags:
  `NORAFT` clamps the external raft off even when a broad `raft-stage`
  test token is present, and `FIRST` skips the walk prelude before
  full-wipe scenes.
- `v0.6.10-ps1` also carries the `MARY 4` promotion after the
  generic multi-view stitch rebuilt high/low packs from normal,
  far-left, and far-right foreground-only captures, restoring both
  sides of the island-relative action and passing far-right visual
  stress playback without a production placement pin.
- `v0.6.9-ps1` — scene-validation bugfix release; promotes `MARY 3`
  after far-right full-frame keyed foreground-only recapture restored
  the left-of-island action, host capture stopped replaying stale
  sprite-surface pointers, low-memory clean-snapshot relief kept the
  large pack stable, and the late dinner/thought beat was retimed onto
  the readable frames.
- `v0.6.8-ps1` — scene-validation bugfix release; promotes `MARY 2`
  after the wide scene-relative multi-view stitch restored edge-clipped
  line, mermaid, boot/splash, and lower-water pixels, and after
  full-host bubble injection restored the fish thought-bubble shell.
- Current main after `v0.6.6-ps1` — promotes `MARY 1` after visual +
  audible signoff on the legacy validation route (`x=-124,y=37`,
  raft-stage `5`), with no pack or runtime changes required. It also
  promotes `JOHNNY 6` through the full-screen black-backdrop runtime
  path and `JOHNNY 5` after the splash-capture and SOS-note timing fixes.
- `v0.6.6-ps1` — scene-validation bugfix release; revalidates
  `FISHING 7` and `FISHING 8` after rebuilding high/low packs from a
  far-left full-frame foreground-only capture, proving pack completeness
  and removing the old runtime island-position pin.
- `v0.6.5-ps1` — scene-validation bugfix release; promotes `FISHING 5`
  after the shark capture was rebuilt with a full-frame keyed
  current-ledger overlay that removes stale host overpaint without
  dropping current shark pixels.
- `v0.6.4-ps1` — scene-validation bugfix release; promotes `JOHNNY 2`
  after the first-SOS-bottle capture was rebuilt with pinned island
  placement, keyed lower-band cleanup, and thought-bubble hold timing
  for the island/SOS frames.
- `v0.6.3-ps1` — scene-validation bugfix release; promotes `FISHING 7`,
  `FISHING 8`, and `JOHNNY 1`, adds the `JOHNNY 1` full-screen
  black-backdrop cleanup path, and applies saved mute settings before
  audio startup.
- `v0.6.2-ps1` — scene-validation bugfix release; promotes `FISHING 6`
  after terminal FGP3 cleanup repair removes the final splash and
  fishing-pole residue.
- `v0.6.1-ps1` — freeplay clean-rect bugfix release; clean rects now
  follow randomized island placement, fixing bottom-edge Johnny foot
  cleanup and freeplay return stability.
- `v0.6.0-ps1` — ocean ambience release; optional CC0 ocean loop on a
  dedicated SPU voice, pause-menu toggle, and memcard-persisted
  preference.
- `v0.5.0-ps1` — freeplay/debug release; promotes direct-control Johnny,
  pause-menu gag/visitor catalogs, sound test, immediate world toggles,
  frog-clock loading transitions, Select clear-screen rebuild, and
  freeplay memory rules into the main release build.
- `v0.4.20-ps1` — walking-loop release; promotes story-loop Johnny
  walking, palm-tree occlusion, holiday persistence during transitions,
  and deterministic walk erase memory into the main release build.
- `v0.3.9-ps1` (commit `111efa9f`) — fishing3 overnight loop-stability
  release; confirms the current runtime can run long sessions without
  the previous scene-to-scene leak.
- `v0.3.6-ps1` (commit `f2737253`) — fishing1 pixel-perfect with full SFX
  across all variants.
- Prior visual-only release `v0.3.5-ps1` (commit `9448d49f`) —
  superseded by the full-SFX release above.

Milestone release cadence from here shifts away from scene-count milestones.
All 63 scenes are validated; future releases should be cut around bugfixes,
speed/loading wins, memory-pressure reductions, settings persistence, and
feature polish.

## Primary render methodology: hybrid scene playback (fgpilot)

Desktop host = authoritative renderer and capture source. PS1 = hybrid
replay target. For a validated scene, the PS1 runtime does not
reconstruct the full TTM/ADS scene graph; it replays host-captured
foreground pixels plus captured SFX events and owns only the narrow
runtime surface (background, waves, holiday overlay, SPU playback,
input).

The internal code name for this path is `fgpilot`; public / operator
documentation is migrating to the name **PS1 scene playback**. See
[ps1-branch-cleanup-plan.yaml](ps1-branch-cleanup-plan.yaml) §
`fgpilot_naming_migration_plan`.

### Pipeline

```
desktop host ──► capture-host-scene.sh ──► high/low frames + frame-meta JSONs + sound-events.jsonl
                                  │
                                  ▼
            export-scene-foreground-pilot.sh
                                  │
                                  ▼
            build-scene-foreground-pack.py
                                  │
                                  ▼   (FG2: pal4/indexed8 spans + sound-event table)
               generated/ps1/foreground/*.FG2 ──► CD image ──► PS1
                                                                │
                                                                ▼
                                              foreground_pilot.c (replay)
                                                                │
                                                                ▼
                                             sound_ps1.c soundPlay() on cue
```

### Acceptance model

Primary gate is **human visual + audible signoff** on the scene-playback
path. Regtest, binary-library scans, and harness-based validation are
preserved as secondary / historical tooling — useful for targeted
questions, not for certifying a scene as done. See
[TESTING.md](TESTING.md).

FG1 packs, FOC draw packs, per-scene establishing RAWs, and ADS/TTM
console runtime routes are no longer part of the active methodology.
The PS1 executable now links only the scene-playback runtime plus the
minimal background/audio/input/CD layers it needs.

## Audio

SPU is initialised at boot; all 23 VAG sound effects are preloaded into
SPU RAM; `soundPlay(nb)` drives a round-robin over 8 voices. Captured
`0xC051 PLAY_SAMPLE` events from the host TTM interpreter ship in the
foreground pack and are fired from `foreground_pilot.c` during replay,
with a 3-frame delay constant to align key-on with the visible frame.

The VAG encoder (`scripts/wav2vag.py`) and the SPU upload/playback path
(`sound_ps1.c`) were extensively debugged during the `v0.3.6-ps1`
milestone; see commit `355227fa` for the bug list (shift-exponent
inversion, ADPCM nibble-pair order, SPU DMA 64-byte alignment,
ADSR1 attack-rate orientation, etc).

## Historical status numbers (not current)

Older validation models were reset by the plan documented in
`ps1-branch-cleanup-plan.yaml` §
`historical_status_surfaces_and_meanings`. They are preserved here for
searchability — **do not cite them as current progress**:

| Count | Date | Meaning at the time | Source |
|---|---|---|---|
| 25 / 63 | 2026-03-21 | Restore-rollout verified scenes | `docs/ps1/project-history.md`, older `current-status.md` |
| 27 / 63 | 2026-03-21 | Research-snapshot verified | `docs/ps1/research/CURRENT_STATUS_2026-03-21.md` |
| 63 / 63 | 2026-03-29 | Harness-level validation claim | `docs/ps1/research/VALIDATION_LOG_2026-03-29.md` — *retroactively demoted as a false summit* |
| 57 / 63 | 2026-04-04 | Scenes rendering with island content | `docs/ps1/TESTING.md` (older) |
| 60 / 63 | 2026-04-04..07 | Bringup in the headless regtest surface | `docs/ps1/TESTING.md` (older), `config/ps1/regtest-scenes.txt` |
| **1 / 63** | **2026-04-22** | **Human-signed reference scene under the full visual + SFX bar** | **this doc, `scene-status.md`** |
| **2 / 63** | **2026-04-24** | **Scene ledger after `FISHING 2` promotion; `FISHING 3` remained bring-up** | **this doc, `scene-status.md`** |
| **5 / 63** | **2026-05-01** | **Scene ledger after `FISHING 3`, `FISHING 4`, and `FISHING 6` promotion; `FISHING 5` remains blocked** | **this doc, `scene-status.md`** |
| **6 / 63** | **2026-05-01** | **Scene ledger after `FISHING 7` promotion; initially captured as a single-position replay, superseded on 2026-05-03 by far-left recapture plus variable-position runtime playback** | **this doc, `scene-status.md`** |
| **7 / 63** | **2026-05-01** | **Scene ledger after `FISHING 8` promotion; initially shared the `FISHING 7` captured-position rule, superseded on 2026-05-03 by the same far-left recapture pattern** | **this doc, `scene-status.md`** |
| **8 / 63** | **2026-05-02** | **Scene ledger after `JOHNNY 1` promotion; full-screen black-backdrop playback avoids ocean/island clean-rect memory pressure** | **this doc, `scene-status.md`** |
| **9 / 63** | **2026-05-02** | **Scene ledger after `JOHNNY 2` promotion; pinned capture plus keyed lower-band cleanup fixes bottle/feet residue and thought-bubble timing** | **this doc, `scene-status.md`** |
| **10 / 63** | **2026-05-02** | **Scene ledger after `FISHING 5` promotion; full-frame keyed current-ledger overlay fixes stale shark overpaint and outline-only shark frames** | **this doc, `scene-status.md`** |
| **11 / 63** | **2026-05-02** | **Scene ledger after `JOHNNY 3` promotion; right-shift diagnostic run proves complete source pixels without adding a runtime island-position pin** | **this doc, `scene-status.md`** |
| **12 / 63** | **2026-05-03** | **Scene ledger after `JOHNNY 4` promotion plus `FISHING 7`/`FISHING 8` revalidation; all three use capture/test positions only and production variable island placement** | **this doc, `scene-status.md`** |
| **13 / 63** | **2026-05-03** | **Scene ledger after `JOHNNY 5` promotion; host/test `x=80,y=54` capture restores the thrown-bottle splash, but production island placement remains variable** | **this doc, `scene-status.md`** |
| **14 / 63** | **2026-05-03** | **Scene ledger after `JOHNNY 6` promotion; black-backdrop routing avoids painting ocean/island behind an all-black source scene** | **this doc, `scene-status.md`** |
| **15 / 63** | **2026-05-03** | **Scene ledger after `MARY 1` promotion; legacy validation route `x=-124,y=37`, raft-stage `5`, needed no pack/runtime changes** | **this doc, `scene-status.md`** |
| **16 / 63** | **2026-05-03** | **Scene ledger after `MARY 2` promotion; wide scene-relative multi-view stitch plus full-host thought-bubble injection makes the scene random-position safe** | **this doc, `scene-status.md`** |
| **17 / 63** | **2026-05-03** | **Scene ledger after `MARY 3` promotion; far-right foreground-only recapture, stale host-surface invalidation, and low-memory clean-snapshot relief make the scene stable and readable** | **this doc, `scene-status.md`** |
| **22 / 63** | **2026-05-03** | **Current scene ledger after `STAND 1` promotion; current generic multi-view stitch regenerated high/low packs and the 35-frame / 169-vblank no-SFX idle loop passed human visual signoff** | **this doc, `scene-status.md`** |
| **23 / 63** | **2026-05-04** | **Current scene ledger after `STAND 2` promotion; normal high-tide/night playback passed human visual signoff and direct scene-loader launches now skip the stale story-walk prelude** | **this doc, `scene-status.md`** |
| **24 / 63** | **2026-05-04** | **Current scene ledger after `STAND 3` promotion; normal high-tide/night playback passed human visual signoff on the short hat-lift idle loop** | **this doc, `scene-status.md`** |
| **25 / 63** | **2026-05-04** | **Current scene ledger after `STAND 4` promotion; high/low packs were regenerated through the generic multi-view stitch and normal high-tide/night playback passed human visual signoff on the tapping-foot idle loop** | **this doc, `scene-status.md`** |
| **63 / 63** | **2026-05-05** | **Current scene ledger after `ACTIVITY 9` promotion; all scenes are validated under the current visual + audible signoff bar, and ACTIVITY9 high/low packs were rebuilt through a wide stitch plus source `BOAT.PSB` edge repair, clip-edge overlap, and held-frame boat draw carry-forward** | **this doc, `scene-status.md`, `release-notes-0.7.0.md`** |
| **62 / 63** | **2026-05-05** | **Scene ledger after `ACTIVITY 1` promotion; high/low packs were rebuilt from a capped two-beat story capture (`FG_EXPORT_ACTIVITY1_CAPTURE_FRAMES=400`), source frames `148` and `348` hold the animal scorecards, and `patch-activity1-tree-foreground.py` keys foreground-only tree-band contamination against the full-host composite so the pre-pop hat/white pixels and tree-occlusion ghosts are gone in both loops** | **this doc, `scene-status.md`** |
| **61 / 63** | **2026-05-05** | **Current scene ledger after `ACTIVITY 7` promotion; high/low packs were re-exported through the no-stitch fast path with frame-wide keyed overlay to clean ghosted Johnny pose residue on the right side of the island from base-diff against the static-Johnny base on the bathes-and-seagull-steals-clothes loop** | **this doc, `scene-status.md`** |
| **60 / 63** | **2026-05-05** | **Current scene ledger after `ACTIVITY 4` promotion; signoff on the existing on-disc ACTIVITY4.FG2 / ACTV4LOW.FG2 packs (no rework); high-tide nighttime route, reads-with-seagull-on-head** | **this doc, `scene-status.md`** |
| **59 / 63** | **2026-05-05** | **Current scene ledger after `ACTIVITY 5` promotion; high/low packs were re-exported single-position with the JOHNNY 2-style split keyed overlay (upper bubble lane on full base-diff so the storm-cloud thought bubble + connector dots survive, lower third on keyed overlay so the dive splash band cleans up without ghost trails) plus +30 vblanks of hold on source frame 46 so the gag is readable** | **this doc, `scene-status.md`** |
| **58 / 63** | **2026-05-05** | **Current scene ledger after `ACTIVITY 6` promotion; high/low packs were re-exported through the no-stitch fast path with frame-wide keyed overlay to clean ghosted Johnny pose residue from base-diff against the static-Johnny base on the reads/falls-asleep/coconut-bonk loop** | **this doc, `scene-status.md`** |
| **57 / 63** | **2026-05-05** | **Current scene ledger after `BUILDING 7` promotion plus upstream `ACTIVITY 8`, `ACTIVITY 10`, `ACTIVITY 11`, `ACTIVITY 12`, `WALKSTUF 1`, and `WALKSTUF 2`; BUILDING1-7 high/low packs use the generic multi-view stitch, BUILDING2 injects persistent full-host sandcastle pixels, BUILDING4 uses terminal FGP3 cleanup, and BUILDING7 patches the middle campfire from clean animated foreground rows before FGP3 cleanup** | **this doc, `scene-status.md`** |
| 44 / 63 | 2026-05-04 | `VISITOR 7` promotion; high/low packs use the generic multi-view stitch plus hold redistribution for source frames 32, 62, 71, and 80 so the coconut/tree impact reads clearly | this doc, `scene-status.md` |
| 43 / 63 | 2026-05-04 | `VISITOR 6` promotion; high/low packs use the generic multi-view stitch plus full-host impact-delta injection for source frames 120:141 to restore background-owned coconut/tree impact pixels | this doc, `scene-status.md` |
| 42 / 63 | 2026-05-04 | `VISITOR 4` promotion; live validation confirms VISITOR4 is the coconut/plane gag in the current scene mapping, and its high/low packs use the generic multi-view stitch | this doc, `scene-status.md` |
| 41 / 63 | 2026-05-04 | `VISITOR 5` promotion; high/low packs use the generic multi-view stitch plus scene-specific hold redistribution for the coconut impact and downed-plane motion | this doc, `scene-status.md` |
| 40 / 63 | 2026-05-04 | `VISITOR 3` and `WALKSTUF 3` promotion; VISITOR3 uses rebuilt high/low red-ship/splash synthesis packs, while WALKSTUF3 validates existing on-disc WALK3 packs without rework | this doc, `scene-status.md` |
| 39 / 63 | 2026-05-04 | `VISITOR 3` promotion; high/low packs were rebuilt with VISITOR3-specific red-ship/splash synthesis and visual + audible playback passed signoff | this doc, `scene-status.md` |
| 38 / 63 | 2026-05-04 | `VISITOR 1` promotion; high/low packs were regenerated through the generic normal/far-left/far-right foreground-only multi-view stitch and low-tide/night playback passed visual + audible signoff | this doc, `scene-status.md` |
| 37 / 63 | 2026-05-04 | `SUZY 2` promotion; high/low packs were regenerated with a static-base foreground overlay for `MRAFT.BMP`, and SFX playback now leaves mixer headroom for overlapping raft samples | this doc, `scene-status.md` |
| 36 / 63 | 2026-05-04 | `SUZY 1` promotion; high/low packs were regenerated and runtime playback now loads `SUZBEACH.SCR` as the scene-specific beach backdrop instead of painting the normal island/ocean background | this doc, `scene-status.md` |
| 35 / 63 | 2026-05-04 | Scene ledger after `STAND 16` promotion; high/low packs were regenerated through the STAND no-stitch fast-path export and normal high-tide/night center-spyglass playback passed human visual signoff | this doc, `scene-status.md` |
| 34 / 63 | 2026-05-04 | Scene ledger after `STAND 15` promotion; high/low packs were regenerated through the STAND no-stitch fast-path export and normal high-tide/night playback passed human visual signoff | this doc, `scene-status.md` |
| 33 / 63 | 2026-05-04 | Scene ledger after `STAND 12` promotion; same host export quirk as `STAND 10`/`STAND 11`, the previously-committed 276 KB FG2 pack was kept and signed off as-is on the normal high-tide/night route | this doc, `scene-status.md` |
| 32 / 63 | 2026-05-04 | Scene ledger after `STAND 11` promotion; same host export quirk as `STAND 10`, the previously-committed 95 KB FG2 pack was kept and signed off as-is on the normal high-tide/night route | this doc, `scene-status.md` |
| 31 / 63 | 2026-05-04 | Scene ledger after `STAND 10` promotion; the host engine exits `STAND.ADS:10` after only 2 frames so the no-stitch export collapses to an empty pack, and the previously-committed 96 KB FG2 pack was kept and signed off as-is on the normal high-tide/night route | this doc, `scene-status.md` |
| 30 / 63 | 2026-05-04 | Scene ledger after `STAND 9` promotion; normal high-tide/night playback passed human visual signoff through the same STAND no-stitch fast-path export and per-frame wave tick `STAND 8` introduced | this doc, `scene-status.md` |
| 29 / 63 | 2026-05-04 | Scene ledger after `STAND 8` promotion; the FG2 runtime now ticks ocean wave animation every frame so STAND no-stitch scenes get moving water from the engine instead of the static foreground-only pack, and normal high-tide/night playback passed human visual signoff | this doc, `scene-status.md` |
| 28 / 63 | 2026-05-04 | Scene ledger after `STAND 7` promotion; high/low packs were regenerated through the same STAND no-stitch fast-path export `STAND 5` and `STAND 6` use, and normal high-tide/night playback passed human visual signoff | this doc, `scene-status.md` |
| 27 / 63 | 2026-05-04 | Scene ledger after `STAND 6` promotion; high/low packs were regenerated through the same STAND no-stitch fast-path export `STAND 5` proved out, and normal high-tide/night playback passed human visual signoff | this doc, `scene-status.md` |
| 26 / 63 | 2026-05-04 | Scene ledger after `STAND 5` promotion; high/low packs were regenerated through the STAND no-stitch fast path with a full-frame foreground-only overlay, and normal high-tide/night playback passed human visual signoff | this doc, `scene-status.md` |
| **21 / 63** | **2026-05-03** | **Scene ledger after `MISCGAG 2` promotion; current generic multi-view stitch regenerated high/low packs and normal high-tide/night playback passed human visual + audible signoff** | **this doc, `scene-status.md`** |
| **20 / 63** | **2026-05-03** | **Scene ledger after `MISCGAG 1` promotion; current generic multi-view stitch regenerated high/low packs and normal high-tide/night playback passed human visual + audible signoff** | **this doc, `scene-status.md`** |
| **19 / 63** | **2026-05-03** | **Scene ledger after `MARY 5` promotion; story flags now clamp external raft state for `NORAFT` scenes and skip walk preludes for `FIRST` full-wipe scenes** | **this doc, `scene-status.md`** |
| **18 / 63** | **2026-05-03** | **Scene ledger after `MARY 4` promotion; generic normal/far-left/far-right foreground-only stitching restores both sides of island-relative action while production placement remains variable** | **this doc, `scene-status.md`** |

Each older count belongs to a different definition of "verified";
they are not comparable to each other or to today's number. The current
scene ledger starts clean from the human-signed scene-playback bar
because that is the only surface that matches the present proven
baseline.

## Build size

| | |
|---|---|
| PS-EXE (`jcreborn.exe`) | ~84 KB after removing legacy ADS/TTM/FG1 runtime links |
| CD image (`jcreborn.bin`) | 9.9 MB with routed fishing FG2 packs, `TITLE.RAW`, minimal SCR/PSB/SND assets, and `RESOURCE.MAP` / `RESOURCE.001` retained for active metadata + palette lookup |
| Generated FG2 corpus | 126 high/low packs for 63 scenes, ~343 MB |

## Known limitations

- PS1 `printf()` is now reliable through DuckStation TTY logging
  (verified 2026-04-25). It still must not be called from per-frame
  hot paths because text I/O alters timing; use the telemetry overlay,
  the `ps1_perf` module, or one-shot snapshots like `JCPAUSE`/`JCPERF`
  for runtime visibility.
- `FntFlush` (PSn00bSDK font path) is empirically broken in the scene
  runtime context — primitives accumulate but produce no visible
  pixels. The pause menu uses a custom embedded 8x8 ASCII font
  instead; do not regress to `FntFlush` for new on-screen text.
- BIOS pad system (`InitPAD` / `StartPAD`) is unusable in our
  PSn00bSDK 0.24 + DuckStation environment. All pad input goes through
  `src/spi.c` (timer-2 + SIO0 IRQ-driven, 250 Hz). Note the
  DuckStation-specific quirk: poll TX must be `tx_len=5`, not the
  spicyjpeg reference's `tx_len=4`, or button bytes are dropped.
- `SpuSetCommonMasterVolume` is not honored by DuckStation HLE; sound
  mute writes the SPU master-volume registers directly.
- FG1/FOC and per-scene RAW paths are retired; do not add new docs,
  routes, generated artifacts, or CD entries for them.
- Scene coverage beyond the 16 signed-off rows is pending scene-by-scene
  bring-up via the loop in [development-workflow.md](development-workflow.md).

## See also

- [milestones-2026-04-25.md](milestones-2026-04-25.md) — TTY,
  perf module, pause menu, holiday expansion, SPI driver, memcard
- [scene-status.md](scene-status.md) — per-scene ledger
- [pause-menu-design.md](pause-menu-design.md) — locked design
- [holidays-expansion-design.md](holidays-expansion-design.md) — 36-holiday plan
- [walk-implementation-plan.md](walk-implementation-plan.md) — story-loop walking notes
- [freeplay-mode-design.md](freeplay-mode-design.md) — freeplay/debug mode controls, menus, memory rules, and telemetry
- [release-notes-0.8.14.md](release-notes-0.8.14.md) — JOHNNY1 local-LZ green promotion release notes
- [release-notes-0.8.13.md](release-notes-0.8.13.md) — under-99 payload-work checkpoint release notes
- [release-notes-0.8.5.md](release-notes-0.8.5.md) — full 126-row headless performance matrix release notes
- [release-notes-0.8.4.md](release-notes-0.8.4.md) — custom Scene Explorer thumbnail release notes
- [release-notes-0.5.0.md](release-notes-0.5.0.md) — freeplay/debug release notes
- [release-notes-0.4.20.md](release-notes-0.4.20.md) — walking-loop release notes
- [performance-optimization-plan.md](performance-optimization-plan.md) — perf backlog
- [development-workflow.md](development-workflow.md) — bring-up loop
- [TESTING.md](TESTING.md) — validation strategy
- [hardware-specs.md](hardware-specs.md)
- [project-history.md](project-history.md) — development narrative
- [research/README.md](research/README.md) — design logs (historical)
- [ps1-branch-cleanup-plan.yaml](ps1-branch-cleanup-plan.yaml) — in-flight cleanup contract
