# PS1 Port — Current Status

> 🌐 **Rendered version:** **[/about/status/](https://hunterdavis.com/johnny-castaway-ps1/about/status/)** — this doc rendered on the project website with cross-links and prose context. The GitHub copy here is the source.


**Last updated:** 2026-05-09 (`v0.8.3-ps1` performance point release plus the
post-release BUILDING1, VISITOR5, BUILDING2 low, WALKSTUF3 high, BUILDING6,
ACTIVITY9 high, WALKSTUF3 low, JOHNNY1, ACTIVITY9 low, and VISITOR3
motion-copy/code-headroom/CD-pressure/setup-prime perf follow-ups through v213;
all 63 scenes remain validated, and the public headless battle card is
`+0.3930%` over target / `99.6194%` target speed).

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
| Perf instrumentation (`ps1_perf.c`) | Complete — level-gated `JCPERF`/`JCPERF2` TTY lines (OFF/SUMMARY/DETAIL/DEBUG via `ps1PerfSetLevel`) |
| Pause menu (`pause_menu.c`) | Complete — Start opens overlay; custom 8x8 font (FntFlush is empirically broken in scene-runtime context); POLY_F4 dim + panel quads |
| User settings persistence (`memcard.c`) | Working / expanding — v6 saves persist holiday mode separately from manual holiday id; broader menu-option persistence remains future work |
| TTY printf | Reliable on PSn00bSDK + DuckStation as of 2026-04-25 |

## Scenes: 63 / 63 fully validated

The per-scene ledger lives in [scene-status.md](scene-status.md). That
file is the source of truth for what is complete under the current bar;
this page gives the narrative around it.

Latest point release: `v0.8.3-ps1` is the WALKSTUF1 compact foreground
performance release. All 63 scenes remain validated, all 126 high/low variants
remain routed through headless perf. The promoted
`walkstuf1-compact-fgp3-v141` pass remains the release pack/data win, and the
MARY3 guarded prefetch-preserve follow-up moves high/low from `2402/2295` and
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
`95 -> 56`, and keeps LBAs `13982/14201`. The latest VISITOR3 timing pass adds
a high-only sparse-in-place frame `117` target-hull motion payload on top of
the v181 yacht translation payloads, high frame `115`, shared frame `124`, and
shared frame `118`, then adds high-only re-anchored frames `127`, `126`, and
`125`.
It preserves both `1555450` byte pack footprints and LBAs `22472/23232`,
improves high to `1099/1032`, moves low to `1102/1032`, and keeps high/low
persistent setup segments for sectors `277..293` and `281..305` plus the v213
high `288 KiB` setup-prime cap. Blocking is now `96/124`, loop-read time is
`96/131`, and due misses are `17/22`; the accepted setup tradeoffs raise high
total `scene_vb 1405 -> 1423` and low total `1408 -> 1415` while reducing
active-loop debt. The current runtime baseline keeps all broad canaries flat
while creating code-layout headroom from the earlier v197 dispatch removal.
The public battle card is now `+0.3930%` over target / `99.6194%` target speed
while preserving fixed pack LBAs and the
`215040` byte PS-EXE bucket.
Since the compact full-matrix baseline was about `17.4%` over target /
`87.1%` target speed, the headless methodology has removed about `17.01`
public over-target points and added about `12.52` public target-speed points.

Prior point release: `v0.8.2-ps1` is the VISITOR3 guarded-read performance
release. All 63 scenes remain validated, all 126 high/low variants remain
routed through headless perf, and the VISITOR3 restore-minus-current
pack baseline preserved pack LBAs and the `215040` byte PS-EXE bucket. The
subsequent BUILDING4 restore-minus-current pack pass, VISITOR3 low read-group
prune, WALKSTUF1 high setup-prime retune, VISITOR3 high/low offscreen draw
clips, BUILDING2 high restore-minus-current pass, BUILDING2 grouped-read
passes, VISITOR3 low code-shape pass, and VISITOR3 v4 draw-tail trim
stageguard pass are now superseded by the current public-capped `v0.8.3`
rollup above.
See
[release-notes-0.8.3.md](release-notes-0.8.3.md).

Current performance baseline: VISITOR3 uses cleanup-compact FGP3 data plus
FGP3/v4 compact PAL4 draw metadata, an inlined compact metadata decoder, and
pack-side cleanup spans with current-frame redraw coverage removed plus scoped
`-Os` background composite helpers, v4 draw-tail trimming, a VISITOR3 stage
guard, and the motion-copy FGP3 payload for yacht translation frames `119..123`
plus high-tide frame `115`, shared frames `118`/`124`, high-only frame `117`,
and high-only re-anchored frames `127`/`126`/`125`, plus high/low persistent setup segments
for sectors `277..293` and `281..305`, plus the v213 high setup-prime cap
expansion. VISITOR3 high is now `1099/1032` with
`blocking_vb=96`; low is
`1102/1032` with `blocking_vb=124`. BUILDING2 high/low are `1349/1316` and
`1349/1316`, ACTIVITY9 high/low are `2082/2062` and `2075/2061`, WALKSTUF1
high/low are now
`1491/1426` and `1489/1427`, WALKSTUF3 high/low are `2310/2290` and
`2310/2295`, JOHNNY1 high/low are both `1974/1945`, and the FISHING1 high control sits at the public cap
(`1068/1074`, raw signed under target). BUILDING4 now uses the same pack-side
restore-minus-current cleanup: high is `2844/2816` with `blocking_vb=37`, and
low is `2855/2815` with `blocking_vb=46`. The earlier WALKSTUF1 high
`144 KiB` setup-prime retune is superseded by the compact-pack baseline.
JOHNNY2 and related current-pack clean-pressure work are preserved in the
matrix; the next true outliers are VISITOR3, residual WALKSTUF1 work,
BUILDING2 residual work, VISITOR5 low, JOHNNY1 residual read/data-shape work, BUILDING4 low,
BUILDING6 residual work, ACTIVITY9 low, and selective upload-ready bands.
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
- [release-notes-0.5.0.md](release-notes-0.5.0.md) — freeplay/debug release notes
- [release-notes-0.4.20.md](release-notes-0.4.20.md) — walking-loop release notes
- [performance-optimization-plan.md](performance-optimization-plan.md) — perf backlog
- [development-workflow.md](development-workflow.md) — bring-up loop
- [TESTING.md](TESTING.md) — validation strategy
- [hardware-specs.md](hardware-specs.md)
- [project-history.md](project-history.md) — development narrative
- [research/README.md](research/README.md) — design logs (historical)
- [ps1-branch-cleanup-plan.yaml](ps1-branch-cleanup-plan.yaml) — in-flight cleanup contract
