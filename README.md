# Johnny Castaway — PlayStation 1

**Website: [hunterdavis.com/johnny-castaway-ps1](https://hunterdavis.com/johnny-castaway-ps1/)** — the full project site, with screenshots, scene ledger, deep dives, devlog, and history. *This README is the short version.*

A PS1 port of Sierra's classic *Johnny Castaway* screen saver,
using a hybrid scene-playback pipeline: desktop host is the authoritative
renderer and capture source; the PS1 runtime replays authored foreground
packs + captured SFX and owns only the narrow surface it must (background,
wave animation, holiday overlay, input, SPU).

<p align="center">
  <img src="docs/readme/johnny6-ps1-office.png" width="47%" alt="JOHNNY 6 on PS1: Johnny working in an office">
  <img src="docs/readme/johnny6-ps1-date-dream.png" width="47%" alt="JOHNNY 6 on PS1: Johnny dreaming about his island date">
</p>

<p align="center">
  <code>JOHNNY 6</code> on PS1, captured from DuckStation: office daydream · island date dream.
</p>

<p align="center">
  <img src="docs/readme/fishing1-ps1-cast.png" width="31%" alt="FISHING 1 on PS1: daytime cast">
  <img src="docs/readme/fishing1-ps1-raft.png" width="31%" alt="FISHING 1 on PS1: raft-stage variant">
  <img src="docs/readme/fishing1-ps1-night.png" width="31%" alt="FISHING 1 on PS1: night variant">
</p>

<p align="center">
  <code>FISHING 1</code> reference scene: daytime cast · raft variant · night variant.
</p>

<p align="center">
  <img src="docs/readme/pause-menu.png" width="62%" alt="Pause menu overlay running on PS1">
</p>

<p align="center">
  Press <strong>START</strong> mid-scene for the pause menu — Resume, Scene Set, Freeplay ON/OFF, Freeplay Options, World Options, Accessibility, and System. Scene Set scrolls the screensaver pool with Left/Right across seven categories (All Scenes, Fishing Only, Johnny Stories, Mary Visits, Visitors, Activities, Misc &amp; Suzy) and commits with Cross or Start; Freeplay Options carries gags, visitors, controls, and clear-screen; World Options carries day/night, tide, raft, holidays, and island position; Accessibility carries captions, sound, ocean ambience, and Sound Test; System carries save, time/date, RNG seed, perf log, reset scene, and next scene.
</p>

<p align="center">
  <img src="docs/ps1/holidays-emblems/holiday-emblems-preview.png" width="72%" alt="Added holiday emblem sprite sheet">
</p>

<p align="center">
  Thirty-two added holiday emblems packed into the PS1 holiday sprite sheet.
</p>

<p align="center">
  <img src="docs/readme/fishing1-captions.png" width="62%" alt="FISHING 1 with closed captions enabled">
</p>

<p align="center">
  Closed captions toggle from <strong>Pause → Accessibility → Captions: ON</strong>. A dark band appears at the bottom of the frame for ~5 seconds at scene start with descriptive subtitle text — accessibility-first and tied to the original Sierra scene-by-scene caption corpus.
</p>

## Where to read more

The website is the canonical surface — most of what's below is mirrored
there with proper cross-linking, prose context, and per-section depth:

- **[/play/](https://hunterdavis.com/johnny-castaway-ps1/play/)** — download + DuckStation quickstart + controls.
- **[/help/](https://hunterdavis.com/johnny-castaway-ps1/help/)** — player help, menu screenshots, and the generated menu guide.
- **[/about/method/](https://hunterdavis.com/johnny-castaway-ps1/about/method/)** — hybrid pipeline deep-dive.
- **[/scenes/](https://hunterdavis.com/johnny-castaway-ps1/scenes/)** — live ledger of all 63 scenes + per-scene case studies.
- **[/archaeology/](https://hunterdavis.com/johnny-castaway-ps1/archaeology/)** — the full 5-chapter project story.
- **[/devlog/](https://hunterdavis.com/johnny-castaway-ps1/devlog/)** — dated worklogs preserved verbatim.
- **[/lab/](https://hunterdavis.com/johnny-castaway-ps1/lab/)** — magazine-length essays on methodology (LLM-assisted dev, hallucination control, build farm, regression practice).
- **[/hack/](https://hunterdavis.com/johnny-castaway-ps1/hack/)** — for curious hackers: learning C from this codebase, porting Johnny to a new platform, the printf-driven perf loop, the visual-debug script catalog.
- **[/docs/](https://hunterdavis.com/johnny-castaway-ps1/docs/)** — every reference manual: build, captions, holidays, pause menu, freeplay, scripted input, regtest, API mapping, the SDL2-compat shim, FG2 pack format, dirty-region template.
- **[/credits/](https://hunterdavis.com/johnny-castaway-ps1/credits/)** — the labor-of-love list.

## Download and play

Latest release → [**Releases page**](https://github.com/huntergdavis/johnny-castaway-ps1/releases/latest) — or read the [Play page on the site](https://hunterdavis.com/johnny-castaway-ps1/play/) for the full quickstart with controls.

Or grab the files directly (auto-updates to the latest release):

- [**jcreborn.bin**](https://github.com/huntergdavis/johnny-castaway-ps1/releases/latest/download/jcreborn.bin) — PS1 CD image
- [**jcreborn.cue**](https://github.com/huntergdavis/johnny-castaway-ps1/releases/latest/download/jcreborn.cue) — cuesheet

Load `jcreborn.cue` in [DuckStation](https://www.duckstation.org/) (or any PS1 emulator).

## Status

| | |
|---|---|
| Current release | **`v0.8.3-ps1`** |
| Reference scene | **`FISHING 1`** — pixel-perfect visuals + synced SFX across every applicable variant (night / low-tide / holiday / raft-stage) |
| Scenes fully validated under the reference bar | **63 / 63** (`ACTIVITY 1`, `ACTIVITY 4`, `ACTIVITY 5`, `ACTIVITY 6`, `ACTIVITY 7`, `ACTIVITY 8`, `ACTIVITY 9`, `ACTIVITY 10`, `ACTIVITY 11`, `ACTIVITY 12`, `BUILDING 1`, `BUILDING 2`, `BUILDING 3`, `BUILDING 4`, `BUILDING 5`, `BUILDING 6`, `BUILDING 7`, `FISHING 1`, `FISHING 2`, `FISHING 3`, `FISHING 4`, `FISHING 5`, `FISHING 6`, `FISHING 7`, `FISHING 8`, `JOHNNY 1`, `JOHNNY 2`, `JOHNNY 3`, `JOHNNY 4`, `JOHNNY 5`, `JOHNNY 6`, `MARY 1`, `MARY 2`, `MARY 3`, `MARY 4`, `MARY 5`, `MISCGAG 1`, `MISCGAG 2`, `STAND 1`, `STAND 2`, `STAND 3`, `STAND 4`, `STAND 5`, `STAND 6`, `STAND 7`, `STAND 8`, `STAND 9`, `STAND 10`, `STAND 11`, `STAND 12`, `STAND 15`, `STAND 16`, `SUZY 1`, `SUZY 2`, `VISITOR 1`, `VISITOR 3`, `VISITOR 4`, `VISITOR 5`, `VISITOR 6`, `VISITOR 7`, `WALKSTUF 1`, `WALKSTUF 2`, `WALKSTUF 3`) |
| Per-scene ledger | [scene-status.md](docs/ps1/scene-status.md) · [/scenes/](https://hunterdavis.com/johnny-castaway-ps1/scenes/) (rendered) |
| Narrative status | [current-status.md](docs/ps1/current-status.md) · [/about/status/](https://hunterdavis.com/johnny-castaway-ps1/about/status/) (rendered) |
| Headless perf battle card | **126 / 126** scene/tide variants routed; **126 / 126** have active-loop timing; **63 / 63** scenes have both tide variants measured; public-capped timing-bearing average is **+0.4% over target / 99.6% target speed** |
| Latest perf matrix run | **`2026-05-08T17:54:07`** (`last_run_at` in the CSV) |
| Perf stats version | Newest optimized/code-headroom rows use `johnny1-compact-fgp3-v173`, `walkstuf3-low-compact-fgp3-v171`, `activity9-high-compact-fgp3-v167`, `building6-compact-fgp3-v165`, `walkstuf3-high-compact-fgp3-v163`, `building2-low-restore-window-slack4-v160`, `visitor5-high-compact-fgp3-noautoprime-v158`, `building1-compact-fgp3-noautoprime-v157`, `mary3-preserve-window-slack8-v149`, `missing-scenes-current-v001`, `visitor3-tail-trim-stageguard-v127`, `graphics-composite-os-v111`, `building2-low-group365-381-v110`, `building2-high-group60-72-v109`, `building2-high-restore-minus-current-v108`, `visitor3-low-offscreen-exitright-v106`, `visitor3-high-offscreen-drawclip-v105`, `walkstuf1-compact-fgp3-v141`, `visitor3-low-readgroup-prune-v088`, `building4-restore-minus-current-v087`, `visitor3-restore-minus-current-v086`, `visitor3-high-readgroup-prune-v084`, `compact-u16-inline-v083`, `fgp3v4-drawcompact-all-v082`, `activity9-dead-readgroup-prune-v082`, `read-group-selector-single-assign-v082`, `visitor3-high-remove-72-84-v082`, `visitor3-high-remove-144-160-v082`, `johnny2-prefetch-relief-v081`, `activity9-low-fgp3-cleanup-compact-v081`, `building4-fgp3-cleanup-compact-window-v081`, `building2-fgp3-cleanup-compact-v081`, `visitor3-fgp3-cleanup-compact-v081`, `mary2-prefetch-relief-v081`, `mary2-fgp3-padded-v081`, `johnny2-fgp3-padded-v081`, `mary5-fgp3-padded-v081`, `activity11-fgp3-padded-v081`, `building5-fgp3-padded-v080`, and `walkstuf1-fgp2-setup-prime-v080`; the full row-level version history is in `performance-scene-matrix.csv` |
| Perf source of truth | [performance-scene-matrix.csv](docs/ps1/performance-scene-matrix.csv) · [performance-experiment-log.md](docs/ps1/performance-experiment-log.md) · [performance-read-candidate-matrix.md](docs/ps1/performance-read-candidate-matrix.md) · [performance-preprocess-opportunities.md](docs/ps1/performance-preprocess-opportunities.md) · [performance-o2-audit.md](docs/ps1/performance-o2-audit.md) · [/perf/](https://hunterdavis.com/johnny-castaway-ps1/perf/) (rendered battle card) |
| Primary acceptance gate | human visual + audible signoff |

`v0.8.3-ps1` is a performance point release after `v0.8.2-ps1`. It promotes
`walkstuf1-compact-fgp3-v141`, converting both WALKSTUF1 PAL4/FGP2 packs into
padded compact FGP3/v4 restore-minus-current packs while preserving the
`1535263` byte pack footprints, pack LBAs, and the `215040` byte PS-EXE
bucket. After the MARY3 guarded prefetch-preserve promotion, the BUILDING1
compact-FGP3/no-autoprime follow-up, the VISITOR5 high-only compact
FGP3/no-autoprime follow-up, the BUILDING2 low restore-minus-current slack-4
follow-up, the WALKSTUF3 high compact-FGP3 follow-up, the BUILDING6
compact-FGP3/v4 follow-up, the ACTIVITY9 high compact-FGP3/v4 follow-up, the
WALKSTUF3 low compact-FGP3/v4 follow-up, and the JOHNNY1 compact-FGP3/v4
follow-up, the current public-capped rollup is `+0.4404%` over target /
`99.5782%` target speed across all 126 timing-bearing rows; the raw signed CSV records
`-0.3281%` over target / `100.3620%` target speed for
internal optimization analysis.

The follow-up VISITOR3 fallthrough-threshold probe did not promote:
`visitor3-fallthrough5-v142` stayed exact-flat on both tides, so the runtime
keeps the accepted `6` VBlank fallthrough guard and the WALKSTUF1 compact pass
remains the release baseline.

`v0.8.2-ps1` is a performance point release after `v0.8.1-ps1`. It promotes
the VISITOR3 high-tide guarded generated-window read group `138..162`, lowering
visible blocking `294 -> 293`, loop reads `40 -> 39`, and loop-read VBlanks
`335 -> 332` while keeping loop timing, pack LBAs, and the PS-EXE sector bucket
stable. It also merges the current upstream website/docs polish into the
performance branch and regenerates the public site with the latest battle-card
numbers.

`v0.8.1-ps1` is a stability point release on top of the complete-scene
performance baseline. It fixes a long-run scene-load freeze by estimating the
actual clean-rect backup footprint before allocation, including ocean wave-band
expansion and upper/lower split rects. The focused MARY4 and FISHING1 pressure
routes complete with `scene-end` and `alloc_fail=0`, and the same-commit
VISITOR3 high/low refresh stayed at the current matrix baseline.

`v0.8.0-ps1` is the complete-scene performance baseline. All 63 scenes remain
signed off, all 126 high/low scene variants are routed, and the current 126
timing-bearing rows average **+0.4% over target / 99.6% target speed** under
the public cap that prevents faster-than-target rows from reporting above
100%. Since the compact full-matrix baseline was about **+17.4% over target /
87.1% target speed**, the public-capped view has removed about **16.96
percentage points** of over-target gap and added about **12.48 target-speed
points**.

Current performance work is focused on rows below 99% first, then the remaining
top outliers. The current promoted source/data pass is
`johnny1-compact-fgp3-v173`: it converts both JOHNNY1 black-backdrop packs to
padded compact FGP3/v4 restore-minus-current data inside the original `448370`
byte footprints. High and low both improve `1977/1943 -> 1974/1945`, cut
overrun `34 -> 29`, blocking `31 -> 26`, hidden refill `31 -> 26`, loop reads
`16 -> 7`, loop-read time `95 -> 56`, and restore bytes `591816 -> 408872`;
pack LBAs stay fixed at `13982/14201`, sectors stay `219`, and the `215040`
byte PS-EXE bucket is unchanged. The previous WALKSTUF3 low compact-FGP3/v4
pass converts low to padded compact restore-minus-current data inside the
original `994669` byte footprint, moves `2321/2293 -> 2310/2295`, cuts
blocking `41 -> 26`, hidden refill `21 -> 17`, loop reads `72 -> 29`, and due
misses `5 -> 2`. The previous ACTIVITY9 high compact-FGP3/v4 pass converts high to padded compact
data inside the original `1745484` byte footprint, moves `2094/2056 ->
2082/2062`, cuts blocking `37 -> 24`, hidden refill `23 -> 17`, and loop reads
`52 -> 25` while keeping low tide exact-flat as a canary. The previous BUILDING6
compact pass converts both BUILDING6 packs to padded compact FGP3/v4
restore-minus-current data inside the original `1444370` byte footprint. High
improves `2520/2442 -> 2482/2457`, cuts blocking `62 -> 25`, hidden refill
`64 -> 27`, loop reads `74 -> 42`, and due misses `1 -> 0`; low improves
`2515/2437 -> 2485/2456`, cuts blocking `70 -> 28`, hidden refill `66 -> 29`,
loop reads `73 -> 42`, and due misses `2 -> 0`. Pack LBAs stay fixed at
`10754/11460`, sectors stay `706`, and the `215040` byte PS-EXE bucket is
unchanged. The earlier WALKSTUF3 high compact pass converts the
high-tide pack to padded compact FGP3/v4 restore-minus-current data while
preserving the `1026922` byte footprint and current pack LBA. High improves
current-layout control `2325/2282 -> 2310/2290`, cuts overrun `43 -> 20`,
blocking `65 -> 47`, hidden refill `32 -> 18`, loop reads `74 -> 37`, and
loop-read time `378 -> 183`. The earlier WALKSTUF1 compact pass converts both
WALKSTUF1 PAL4/FGP2
packs into padded compact FGP3/v4 restore-minus-current packs while preserving
the `1535263` byte footprints, pack LBAs, and the `215040` byte PS-EXE bucket.
High improves `1592/1406 -> 1491/1426`, overrun `186 -> 65`, blocking
`275 -> 85`, loop reads `134 -> 69`, loop read time `586 -> 300`, and due
misses `55 -> 13`; low improves `1604/1407 -> 1489/1427`, overrun `197 ->
62`, blocking `270 -> 86`, prefetch overrun `55 -> 27`, loop reads `132 ->
69`, loop read time `604 -> 305`, and due misses `50 -> 12`. The newer MARY3 guarded
prefetch-preserve promotion keeps `prefetch_overrun_vb=0` while moving high/low
from `2402/2295` and `2402/2296` to `2296/2294` and `2297/2295`; blocking
collapses `690/693 -> 53/51`, loop reads drop `255 -> 44`, and due misses
drop `255 -> 13`. The newer BUILDING1 compact-FGP3/no-autoprime pass moves
high from `792/778` to `784/782` and low from `794/779` to `787/782`,
cutting loop reads `23 -> 17` on both tides while keeping full scene time
lower and moving both rows into the green band. The newer VISITOR5 high-only
compact-FGP3/no-autoprime pass keeps low tide on the prior pack and moves high
from `1111/1090` to `1104/1092`, cutting overrun `21 -> 12`, blocking
`12 -> 11`, prefetch overrun `12 -> 11`, and loop-read time `123 -> 91`. The
newer BUILDING2 low restore-minus-current/slack-4 pass moves low from
`1383/1304` to `1349/1316`, cutting overrun `79 -> 33`, blocking `118 -> 83`,
hidden refill `5 -> 1`, loop reads `55 -> 54`, and loop-read time `251 -> 227`.
The current public-capped rollup is `+0.4404%` over target / `99.5782%` target
speed; the raw signed optimization matrix is `-0.3281%` / `100.3620%`. The next
true rows are VISITOR3 low/high, WALKSTUF1 high/low, BUILDING2 high/low,
VISITOR5 low, JOHNNY1 high/low residual, BUILDING4 low, BUILDING6 high/low, ACTIVITY9
low, and remaining generated scheduler/data-shape work.
The current VISITOR3 default selective upload-ready plan is closed as a
same-footprint append: it saves a modeled `6114568` upload bytes, but its
`2462072` bytes of payload plus rect metadata exceed the current `814847` bytes
of padded pack slack per tide. The follow-up budgeted analyzer target fits that
same slack by selecting `74 / 96` default-selected frames, using `814184`
payload+rect bytes, leaving `663` bytes of slack, and retaining `3858104`
modeled upload bytes saved. The direct raw pack-emitted implementation lane is
now blocked by the analyzer: `0` selected x-band bytes are fully covered by
current opaque draw spans, so the modeled VISITOR3 win would require restored
background pixels that are dynamic at runtime or the already-rejected runtime
scratch-packing path.
The VISITOR3 no-op entry-prune probe is also closed: it reduced active payload
and visible high-tide blocking, but failed the gate by moving the cost into
hidden prefetch overrun (`0 -> 56` high, `0 -> 17` low). The safer empty-hold
recast is closed too: the current VISITOR3 high/low FGP3/v4 payloads have `0`
entries with both cleanup and draw pixel counts at zero, so there is no
cadence-preserving no-op payload to erase under the current assets.
The VISITOR3 `97..109` setup-owned segment retry is closed as well: a dedicated
`24 KiB` persistent setup cache kept high/low layout fixed but measured exact
flat against that baseline (`1139/1024` and `1140/1024`), so that
read-plan candidate now requires generated scheduler ownership or a payload
shape change rather than another local setup/grouped-read variant.
A data-only sector-alignment probe is also closed: spending `38957` bytes of
pack slack to align the large late payloads cut modeled uncovered sectors
`306 -> 294`, but shifted the measured CD phase and regressed VISITOR3 high/low
to `1143/1024` and `1151/1024`.
The duplicate-payload table-reuse lane is closed too: phase-preserving dedupe
removed `5006` active bytes and one loop read but regressed high `1139 -> 1140`
and low `1140 -> 1163`, while full dedupe removed `144068` active bytes yet
regressed high/low to `1158/1024` and `1165/1024`. VISITOR3 payload reuse now
needs a scheduler-costed pack planner rather than another layout-neutral offset
rewrite.
The broad offscreen-clip lane is closed, but tide-specific safe subsets are
promoted. Clipping VISITOR3 compact residuals to the visible screen can reduce
logical payload `737600 -> 655911` per tide, but the size-shrinking version
converts visible wins into hidden prefetch overrun (`0 -> 72/77`), and the low
`ship-left` subset deterministically regresses low tide to `1151/1024`.
Keeping the entry-size-preserving draw clip in high tide improves high to
`1137/1024`; applying only low-tide exit-right entries `139..143` improves low
to `1138/1024` while high and broad controls remain flat.
The later scoped composite-helper `-Os` pass keeps those pack-side wins and
moves VISITOR3 low again to `1135/1024` with `blocking_vb=184`.

`v0.7.2-ps1` is a story-loop walking bugfix release. It prevents Johnny from
walking across stale island backdrops by comparing the full backdrop key
(tide, raft, night, holiday, island X/Y) before allowing an inter-scene walk.
If the next scene would use a different island state, the runtime now skips
the walk and lets the scene reload its own background, avoiding water-walk
trails during randomized transitions.

`v0.7.1-ps1` is a settings/holiday polish release. It adds a persisted
holiday mode field, separates automatic holiday policy from manual holiday
selection, and makes **AUTO DATE:ORIG4** the fresh/no-card default so the
screensaver starts with Sierra's original four-holiday behavior unless the
user opts into the expanded generated calendar. It also fixes menu labeling
for holiday `NONE` and draws holiday overlays behind Johnny during story-loop
walk frames so holiday decorations do not cover the walking sprite.

`v0.7.0-ps1` is the first complete-scene release: all 63 original Johnny
Castaway scenes are validated under the project's current pixel-perfect visual
+ synced-SFX bar. From here, the mainline shifts to bugfixes, speed work, and
new features rather than proving basic scene coverage.

The final promotion was `ACTIVITY 9`, which was rebuilt with
an Activity9-specific wide stitch plus `patch-activity9-boat-foreground.py`,
which fills clipped `BOAT.PSB` bow/stern pixels from source, overlaps the
legacy clip edge to remove the stitch seam, and carries boat draw positions
across metadata-held frames so the late bow no longer flickers.

`v0.6.13-ps1` is a scene-validation bugfix release: `VISITOR 4`,
`VISITOR 5`, `VISITOR 6`, and `VISITOR 7` are validated. VISITOR4 is the
coconut/plane gag in the current scene mapping, correcting an old red-boat
catalogue label. VISITOR4, VISITOR5, and VISITOR7 use regenerated high/low
packs from the generic normal/far-left/far-right multi-view stitch; VISITOR5
and VISITOR7 also redistribute hold time into coconut impact/action rows so
the gags read without changing total scene duration. VISITOR6 adds a narrow
full-host impact-delta injection for source frames 120:141 because
foreground-only capture keeps Johnny/coconut clean but omits background-owned
tree shake/strike pixels.

`v0.6.12-ps1` is a scene-validation bugfix release: `VISITOR 3` and
`WALKSTUF 3` are now validated. VISITOR3 high/low packs use scene-specific
multi-view synthesis: clean foreground-only views provide the moving
sprites, the helper accumulates the red ship hull only from live full-host
crash frames, FGP3 cleanup clears the post-crash rows, and hold timing
keeps the real splash frame visible without stale right-side residue.
WALKSTUF3 passed visual + audible signoff against the existing on-disc
`WALK3.FG2` / `WALK3LOW.FG2` packs without rework.

`v0.6.10-ps1` is a scene-validation bugfix release: `MARY 5` is now
validated. The high/low packs were rebuilt through the generic multi-view
scene-relative stitch with the generic raft off, and the runtime now applies
story flags to direct scene playback: `NORAFT` suppresses the external raft
and `FIRST` skips the walk prelude before full-wipe scenes.

The release also includes the post-`v0.6.11-ps1` validations for `STAND 2`-`STAND 12`,
`STAND 15`, `STAND 16`, `SUZY 1`, `SUZY 2`, and `VISITOR 1`. `STAND 5`-`STAND 9`,
`STAND 15`, and `STAND 16` use the STAND no-stitch export fast path, and the FG2 runtime now ticks ocean wave
animation every frame so no-stitch scenes get moving water from the
engine instead of from the foreground-only pack. The fast path keeps a
full-frame single-position foreground-only overlay so static frame-0 Johnny pixels
are not dropped. `SUZY 1` and `SUZY 2` use the source `SUZBEACH.SCR`
backdrop instead of the island/ocean runtime path; `SUZY 2` also keeps
the scene-local `MRAFT.BMP` raft in the foreground overlay and uses SFX
mixer headroom so overlapping raft samples do not clip. The scene-loader
path now skips the stale walk prelude before direct scene launches, and
high-pressure clean snapshots release optional walk/prefetch caches before
allocating. `VISITOR 1` validates through the standard multi-view stitch
with one captured SFX event and variable production island placement.

`v0.6.8-ps1` is a scene-validation bugfix release: `MARY 2` is now
validated after rebuilding high/low packs from a wide scene-relative
multi-view stitch. Foreground-only captures restore the fishing line,
Mary splash, boot/splash, and lower-water cleanup across island
placements; full-host bubble injection restores the fish thought-bubble
shell that foreground-only capture omitted. Far-right and true far-left
stress playback passed, and production island placement remains variable.

`v0.6.6-ps1` is a scene-validation bugfix release: `FISHING 7` and
`FISHING 8` were rebuilt from far-left full-frame foreground-only host
captures, then validated with far-left stress playback. The old runtime
island-position pins are gone; both scenes now follow normal random
island placement. It builds on `v0.6.5-ps1` (`FISHING 5` shark capture
cleanup and `JOHNNY 4` variable-position bottle-message validation),
`v0.6.4-ps1`
(`JOHNNY 2` pinned lower-band cleanup and thought-bubble timing),
`v0.6.3-ps1` (`FISHING 7`, `FISHING 8`, and `JOHNNY 1`), `v0.6.2-ps1`
(`FISHING 6` terminal FGP3 cleanup), `v0.6.1-ps1` (freeplay clean-rects
follow randomized island placement), and `v0.6.0-ps1`
(ocean ambience: a CC0-sourced 20-second ocean loop on a dedicated SPU
voice, pause-menu toggle, and memcard-persisted preference).

The ocean ambience audio runs at zero CPU cost: the SPU plays the ADPCM
loop in hardware via sample-defined loop flags, so JCPERF2 timing is
unchanged. ~123 KB of SPU RAM used, ~257 KB still free for future
ambience or music slots. The release line also builds on `v0.5.0-ps1`
(freeplay/debug mode — direct-control Johnny, sound test, scene
catalogs) and `v0.4.20-ps1` (walking-loop — Johnny walks between story
scene endpoints instead of teleporting).

One scene at a time is promoted to the "fully validated" bar. Older
count-based validation models (`25/63`, `60/63`, `63/63` etc.) from the
harness-and-restore-pilot eras are preserved as history in
`current-status.md`; none carry forward as current progress.

Headless perf timing is a separate battle card, not the scene-promotion
bar. The current FISHING 1 canary is `loop_vb=1068` against
`target_vb=1074` (**0.0% public over target / 100.0% public target speed**;
raw signed row is `-0.6%` / `100.6%`) with
`blocking_vb=2`, `prefetch_overrun_vb=2`, and `due_misses=0`. All 126 routed
rows now carry active-loop timing after the MARY3 and SUZY1/2 refresh; SUZY1
requires a longer `12000`-frame matrix budget because its valid scene-end
lands after the default `7200`-frame window.

The current headless performance baseline includes the accepted `activity9`
low-tide FGP3 grouped append, the VISITOR3-only `192 KiB` setup-prime resident
cap, the latest WALKSTUF1 PAL4 setup-prime policy, and the BUILDING5 padded
FGP3 conversion. VISITOR3 high now trims
blocking `361 -> 355`, prefetch overrun `21 -> 14`, and loop reads `52 -> 45`;
WALKSTUF1 high trims `loop_vb 1640 -> 1595` and blocking `318 -> 278`, while
WALKSTUF1 low trims `loop_vb 1631 -> 1614` and blocking `296 -> 276`. VISITOR3
low now adds the accepted `170..186` grouped append and moves `loop_vb
1453 -> 1452`, overrun `441 -> 440`, and blocking `362 -> 361`. JOHNNY2 now
preserves stage/window prefetch under large clean-rect pressure, moving high
and low from `1801/1751` and `1800/1751` to `1741/1751`; blocking collapses
`369/377 -> 0`, due misses go `144 -> 0`, and loop reads drop `144 -> 8`.
BUILDING5's
padded FGP3 conversion moves high tide `3359/3346 -> 3343/3348` and low tide
`3357/3347 -> 3345/3347`, cutting visible blocking to `5/8` VBlanks while
keeping the original `818670`-byte CD footprint. ACTIVITY11's padded FGP3
conversion moves high tide `1729/1720 -> 1715/1722` and low tide
`1729/1717 -> 1717/1722`, eliminating active-loop overrun in both rows while
keeping the original `433970`-byte CD footprint. MARY5 then moves high tide
`1591 -> 1581` and low tide `1592 -> 1581`, eliminating both active-loop
overruns while preserving the `646602`-byte CD footprint. The earlier JOHNNY2
padded FGP3 step improved the same-commit baseline from `1833 -> 1801/1800`,
then the clean-pressure relief above completed the row. MARY2's padded FGP3 refresh exposed
a clean-memory relief miss, so the MARY2-local exception restores
`stage1_window` prefetch and moves high/low to `2241/2248` and `2242/2250`;
blocking collapses from `668/662` VBlanks to `2/2`, and due misses go
`233 -> 0`. BUILDING2 then promotes the cleanup-compact FGP3/v3 metadata
shape for both validated packs: high tide moves `1468/1285 -> 1430/1289`,
blocking `301 -> 212`, and loop reads `96 -> 82`; low tide moves
`1465/1276 -> 1429/1286`, blocking `334 -> 193`, and loop reads `87 -> 68`.
BUILDING4 now promotes the same cleanup-compact FGP3/v3 metadata shape plus
scene-local stream-window retuning: high tide moves `2985/2774 -> 2939/2786`,
blocking `285 -> 240`, and hidden refill overrun `51 -> 27`; low tide moves
`2981/2784 -> 2945/2798`, blocking `199 -> 117`, and hidden refill overrun
`119 -> 114`. The later BUILDING4 pack-side restore-minus-current pass keeps
the padded `1714154`-byte pack footprint and fixed LBAs while dropping active
payload `1032442 -> 855284`: high tide moves `2939/2786 -> 2844/2816`,
blocking `240 -> 37`, and hidden refill overrun `27 -> 30`; low tide moves
`2945/2798 -> 2855/2815`, blocking `117 -> 46`, and hidden refill overrun
`114 -> 38`.
ACTIVITY9 low now promotes the same cleanup-compact FGP3/v3 metadata shape:
low tide moves `2098/2056 -> 2087/2056`, blocking `47 -> 42`, and hidden
refill overrun `19 -> 12`; the later ACTIVITY9 high compact-FGP3/v4 pass moves
high tide to `2082/2062` while preserving the original padded pack footprint,
and the latest WALKSTUF3 low compact-FGP3/v4 pass moves low tide to
`2310/2295`.
The public-capped timing-bearing average is now `+0.4404%` over target /
`99.5782%` target speed after the JOHNNY1 compact-FGP3/v4 promotion.
The raw signed optimization matrix is `-0.3281%` / `100.3620%`. The latest
JOHNNY1 pass moves both tides `1977/1943 -> 1974/1945`, cuts overrun `34 -> 29`,
blocking `31 -> 26`, hidden refill `31 -> 26`, loop reads `16 -> 7`, and
loop-read time `95 -> 56` while preserving both `448370` byte pack footprints,
fixed LBAs `13982/14201`, and the `215040` byte PS-EXE bucket. The prior
WALKSTUF3 low pass moves `2321/2293 -> 2310/2295`, cuts overrun `28 -> 15`,
blocking `41 -> 26`, hidden refill `21 -> 17`, loop reads `72 -> 29`, and due
misses `5 -> 2` while preserving the `994669` byte pack footprint and fixed LBA
`26906`. The prior ACTIVITY9 high pass
moves `2094/2056 -> 2082/2062`, cuts blocking `37 -> 24`, hidden refill
`23 -> 17`, and loop reads `52 -> 25`. The prior BUILDING6 pass moves
high `2520/2442 -> 2482/2457` and low `2515/2437 -> 2485/2456`, cuts blocking
`62/70 -> 25/28`, hidden refill `64/66 -> 27/29`, loop reads `74/73 -> 42/42`,
and due misses `1/2 -> 0/0` while preserving the `1444370` byte pack
footprints, fixed LBAs, and the `215040` byte PS-EXE bucket. The prior
WALKSTUF3 high pass moves current-layout control
`2325/2282 -> 2310/2290`, cuts overrun `43 -> 20`, blocking `65 -> 47`, hidden
refill `32 -> 18`, loop reads `74 -> 37`, and loop-read time `378 -> 183`
while preserving the `1026922` byte footprint and current pack LBA. The MARY3 pass
moves high from `2402/2295` to `2296/2294` and low from `2402/2296` to
`2297/2295`, cutting blocking `690/693 -> 53/51` while keeping hidden refill
overrun at zero. The prior WALKSTUF1 source/data pass moves WALKSTUF1 high from
`1592/1406` to `1491/1426`, cuts
overrun `186 -> 65`, lowers blocking `275 -> 85`, lowers hidden refill
`51 -> 32`, and lowers loop reads `134 -> 69`; it also moves WALKSTUF1 low
from `1604/1407` to `1489/1427`, cuts overrun `197 -> 62`, lowers blocking
`270 -> 86`, lowers hidden refill `55 -> 27`, and lowers loop reads
`132 -> 69`. The earlier VISITOR3 tail-trim stageguard pass moved VISITOR3 high
from `1137/1024` to `1118/1028`, cut overrun `113 -> 90`, lowered blocking
`190 -> 150`, lowered loop reads `33 -> 27`, and kept hidden prefetch debt at
`0`; it also moved VISITOR3 low from `1135/1024` to `1126/1025`, cut overrun
`111 -> 101`, lowered blocking `184 -> 170`, lowered loop reads `33 -> 31`,
and kept `prefetch_overrun_vb=0`. The latest VISITOR3 high canary drift was
reproduced with original WALKSTUF1 FGP2 packs restored, so it is not attributed
to the WALKSTUF1 compact pack candidate. The earlier VISITOR3 low code-shape pass moved
VISITOR3 low from `1138/1024` to `1135/1024`, cut overrun `114 -> 111`,
lowered blocking `191 -> 184`, lowered loop read time `200 -> 194`, and shrank
the ELF by `8848` bytes. The earlier BUILDING2 low `365..381` grouped-read pass moved
BUILDING2 low from `1385/1303` to `1383/1304`, cut overrun `82 -> 79`,
lowered blocking `121 -> 118`, lowered hidden prefetch overrun `8 -> 5`, and
lowered loop reads `57 -> 55`. The earlier BUILDING2 high `60..72` grouped-read pass moved high from
`1353/1311` to `1349/1316`, cut overrun `42 -> 33`, lowered blocking
`56 -> 48`, and lowered hidden prefetch overrun `20 -> 12`. The earlier
BUILDING2 restore-minus-current pack pass moved high from `1394/1301` to
`1353/1311`. The earlier VISITOR3 low exit-right offscreen
draw clip moves VISITOR3 low tide from `1140/1024` to `1138/1024`, cuts
overrun `116 -> 114`, and lowers blocking `194 -> 191`. The earlier
high-only offscreen draw clip moves VISITOR3 high from `1139/1024` to
`1137/1024`, cuts overrun `115 -> 113`, and lowers blocking `191 -> 190`.
The earlier WALKSTUF1 high setup-prime cap retune moves high tide from `1595/1402`
to `1592/1406`, cuts overrun
`193 -> 186`, lowers loop reads `136 -> 134`, and keeps low tide exact-flat at
`1604/1407`. The current VISITOR3 low read-group prune keeps the prior
BUILDING4 rollup exact-flat while recovering `36` bytes from
`foregroundPilotPlay`. The earlier
VISITOR3 high guarded generated-window append `138..162` trims
blocking `294 -> 293`, loop reads `40 -> 39`, and loop read VBlanks
`335 -> 332` while leaving the loop-time average unchanged. The follow-up
`visitor3-high-remove-144-160-v082` cleanup drops the now-redundant guarded
`144..160` row after exact-flat broad canaries, preserving that baseline while
recovering table headroom. The later `visitor3-high-remove-72-84-v082`
cleanup drops a second now-dead high-tide row after the read plan proved setup
prime coverage already owns sectors `1..97`; broad canaries remain exact-flat
and the later `read-group-selector-single-assign-v082` cleanup keeps the
13-case canary set exact-flat while shrinking `foregroundPilotPlay` by
`36` bytes. The later `activity9-dead-readgroup-prune-v082` cleanup removes
the now-dead ACTIVITY9 low FGP3/v1 read-group selector, keeps the same
13-case canary timing/LBAs, and recovers another `16` bytes from
`foregroundPilotPlay`. The FGP3/v4 draw-metadata pass then reduces active PAL4
metadata in all current compact residual packs while keeping the same padded
on-disc size: VISITOR3 blocking drops `293/301 -> 244/253`, BUILDING2 blocking
drops `212/193 -> 176/144`, ACTIVITY9 low blocking drops `42 -> 28`, and the
FISHING1 control remains under target. The compact reader inline follow-up
then moves VISITOR3 high/low `1369/1023 -> 1357/1023` and
`1376/1023 -> 1361/1023`, BUILDING2 high/low `1405/1298 -> 1394/1301` and
`1395/1294 -> 1385/1303`, and keeps ACTIVITY9 low plus FISHING1 high within
the broad stability gate. The current VISITOR3 high read-group prune removes
the exhausted high-tide retained-read table (`138..162`, `170..186`,
`230..242`) after focused and broad exact-flat canaries, keeping the same
rollup while shrinking `foregroundPilotPlay` by `48` bytes for the next
generated scheduler/data-shape pass. The pack-side restore-minus-current pass
then removes cleanup spans hidden under the same frame's PAL4 draw spans,
dropping VISITOR3 high/low from `1357/1023` and `1361/1023` to `1139/1024`
and `1140/1024` with fixed padded pack sizes, fixed LBAs, and no PS-EXE change.
The BUILDING4 restore-minus-current follow-up removes the same redundant
current-frame cleanup from BUILDING4 and moves the matrix slightly under
target; the stale VISITOR3 low read-group prune closes the last local VISITOR3
table row without changing timing; the WALKSTUF1 high setup-prime cap is
superseded by the compact FGP3/v4 restore-minus-current pack baseline; the
VISITOR3 default selective upload-ready footprint
gate rejects the naive same-footprint append; and the budgeted VISITOR3
analyzer target now proves a smaller `74`-frame subset can fit inside the
current pack slack before runtime-format work begins. The raw foreground-only
upload payload route is now rejected because none of those selected x-bands are
fully draw-covered; they depend on restored background pixels. A follow-up
no-op entry-prune probe is rejected because it shortens VISITOR3 cadence
without owning the hidden refill schedule. The pack-side empty-hold recast also
finds `0` eligible zero-visual-work entries in the current VISITOR3 high/low
payloads, closing that safer no-op variant. The setup-owned `97..109` segment
retry is exact-flat too, closing the remaining local version of that read-plan
candidate. The large-payload sector-alignment probe then proves that reducing
modeled sectors by padding is not sufficient: it regresses VISITOR3 high/low
cadence despite fixed file size and LBAs. The following packed-draw metadata
probe finally finds a real VISITOR3 byte-reduction signal (`737600 -> 659455`
payload bytes per tide and `1139 -> 1127` / `1140 -> 1124` loop VBlanks), but
its new runtime decoder crosses the PS-EXE sector bucket and shifts foreground
LBAs, regressing BUILDING2 and BUILDING4 canaries, so it is logged but not
promoted. A layout-neutral retry keeps pack LBAs and the `215040` byte PS-EXE
bucket fixed with a smaller payload marker and function-scoped PAL4 span `-Os`,
but it regresses VISITOR3 high (`1139 -> 1148`) while improving low tide
(`1140 -> 1135`), so that C-side packed-draw route is also rejected. A
zero-runtime-code entry-origin shift gate saves `0` payload bytes on the current
VISITOR3 high/low FGP3/v4 packs, so that coordinate recentering lane is closed
before emulator time. The follow-up duplicate-payload table-reuse probe is also
rejected: exact duplicate bodies exist, but reusing them shifts enough payload
phase to regress VISITOR3 even with fixed pack size, fixed LBAs, fixed PS-EXE
bucket, and one fewer loop read.
The high-only offscreen draw-clip follow-up is promoted: it preserves the
`1555450` byte high-tide pack footprint, all entry sizes and offsets, pack
LBA `22472`, and the `215040` byte PS-EXE bucket while trimming offscreen
draw work in `17` entries. It improves VISITOR3 high `1139/1024 -> 1137/1024`
and `blocking_vb 191 -> 190`; low tide and the broad controls stay flat.
The low exit-right follow-up is also promoted: it preserves the `1555450` byte
low-tide pack footprint, all entry sizes and offsets, pack LBA `23232`, and
the `215040` byte PS-EXE bucket while trimming entries `139..143`. It improves
VISITOR3 low `1140/1024 -> 1138/1024` and `blocking_vb 194 -> 191`; high tide
and the broad controls stay flat. The low `ship-left` and `ship-and-exit`
subsets are rejected because they reproduce the bad `1151/1024` cadence.
Since the compact full-matrix baseline was about `+17.4%` over target /
`87.1%` target speed, the public-capped view has removed about `16.96`
percentage points of over-target gap and added about `12.48` points of target
speed.

The current planning pass also fingerprints perf baselines before comparison
and scores foreground read groups by observed append-start fireability, runtime
group-capacity fit, and visible-CD cost class before source edits. These
host-side filters prevent more no-op probes where a range covers useful reads
but cannot fire in the runtime append path, and they separate long-gap
candidates from tight clusters that historically regressed visible blocking;
they do not change PS1 timing or the battle-card totals.

The headless harness now has opt-in scripted controller input. Run
`./scripts/ps1-menu-input-harness.sh` to build a temporary pad-script disc,
press through the pause menu under DuckStation regtest, capture every major
menu screen, and regenerate the website's
[`/help/menu/`](https://hunterdavis.com/johnny-castaway-ps1/help/menu/)
guide from real PS1 screenshots. The detailed runbook lives in
[`docs/ps1/scripted-input-harness.md`](docs/ps1/scripted-input-harness.md).

## Method

The PS1 build is deliberately hybrid, not a from-scratch engine rewrite:

- **Desktop host** runs the real game logic (TTM/ADS interpreter) and
  captures every visible foreground draw plus every `PLAY_SAMPLE` opcode
  to a per-frame JSON bundle.
- A **pack compiler** turns that capture into PS1-native FG2 packs:
  high-tide and low-tide full-render base-diff spans plus a per-frame
  sound-event table.
- On **PS1**, the runtime (`foreground_pilot.c`) loads the pack, stamps
  captured frames in step with a narrow runtime that handles background,
  wave animation, holiday overlay, and SPU playback. SFX fire on cue via
  a per-pack event cursor with a 3-frame delay so sample key-on matches
  the visible trigger.

All 63 original scenes are validated end-to-end under the current visual +
audible bar. The scene-by-scene loop remains the regression bar for future
bugfixes and performance changes.

The full pipeline — pack format byte layout, hardware constraints hit
on the way, the SPI pad-poll fix, dirty-rect bookkeeping — is detailed
at [/about/method/](https://hunterdavis.com/johnny-castaway-ps1/about/method/).

## Quick start

### Prerequisites
- Docker (for the `jc-reborn-ps1-dev:amd64` build image + PSn00bSDK)
- DuckStation (Flatpak: `org.duckstation.DuckStation`)
- Original Sierra *Johnny Castaway* data files (see below)

### Build + run the reference scene

```bash
./scripts/rebuild-and-let-run.sh noclean
```

Builds the PS1 executable, generates `jcreborn.bin/.cue`, launches
DuckStation with the cue, and boots into `FISHING 1` via `BOOTMODE.TXT`.
Default mode is a **screensaver loop**: on each replay the runtime
randomizes night/low-tide/raft/holiday unless you forced those tokens in
`BOOTMODE.TXT`. Add `noloop` to the boot string for a single-shot play.

An emergency watchdog (`RUN_TIMEOUT_SECONDS`, default 300s) will kill the
emulator if it's left running — keeps overnight sessions from filling
the disk. Override with `RUN_TIMEOUT_SECONDS=<n>` or `0` to disable.

### Bring up a new scene
The full capture → pack → wire → validate loop is in
[docs/ps1/development-workflow.md](docs/ps1/development-workflow.md) — also rendered at
[/docs/dev-workflow/](https://hunterdavis.com/johnny-castaway-ps1/docs/dev-workflow/) with cross-links to the regtest harness and per-scene ledger.

## Original data files

The game still needs the original Sierra data files alongside the repo
tree before a CD image can be built:

| File | Bytes | md5 |
|---|---:|---|
| `RESOURCE.MAP` | 1,461 | `8bb6c99e9129806b5089a39d24228a36` |
| `RESOURCE.001` | 1,175,645 | `374e6d05c5e0acd88fb5af748948c899` |

Optional sound effects can be taken from [JCOS resources](https://github.com/nivs1978/Johnny-Castaway-Open-Source/tree/master/JCOS/Resources):

<details>
<summary>sound0..sound24.wav expected hashes</summary>

| File | Bytes | md5 |
|---|---:|---|
| `sound0.wav` | 10,768 | `53695b0df262c2a8772f69b95fd89463` |
| `sound1.wav` | 11,264 | `35d08fdf2b29fc784cbec78b1fe9a7f2` |
| `sound2.wav` | 1,536 | `f93710cc6f70633393423a8a152a2c85` |
| `sound3.wav` | 7,680 | `05a08cd60579e3ebcf26d650a185df25` |
| `sound4.wav` | 5,120 | `be4dff1a2a8e0fc612993280df721e0d` |
| `sound5.wav` | 3,072 | `24deaef44c8b5bb84678978564818103` |
| `sound6.wav` | 15,872 | `eb1055b6cf3d6d7361e9a00e8b088036` |
| `sound7.wav` | 15,360 | `cab94bace3ef401238daded2e2acec34` |
| `sound8.wav` | 2,560 | `39515446ceb703084d446bd3c64bfbb0` |
| `sound9.wav` | 3,584 | `f86d5ce3a43cbe56a8af996427d5c173` |
| `sound10.wav` | 20,480 | `5b8535f625094aa491bf8e6246342c77` |
| `sound12.wav` | 5,632 | `8c173a95da644082e573a0a67ee6d6a3` |
| `sound14.wav` | 11,776 | `e064634cfb9125889ce06314ca01a1ea` |
| `sound15.wav` | 3,072 | `b3db873332dda51e925533c009352c90` |
| `sound16.wav` | 7,680 | `2eabfe83958db0cad77a3a9492d65fe7` |
| `sound17.wav` | 4,608 | `2497d51f0e1da6b000dae82090531008` |
| `sound18.wav` | 14,336 | `994a5d06f9ff416215f1874bc330e769` |
| `sound19.wav` | 3,584 | `5e9cb5a08f39cf555c9662d921a0fed7` |
| `sound20.wav` | 7,680 | `80e7eb0e0c384a51e642e982446fcf1d` |
| `sound21.wav` | 5,120 | `1a3ab0c7cec89d7d1cd620abdd161d91` |
| `sound22.wav` | 1,536 | `a0f4179f4877cf49122cd87ac7908a1e` |
| `sound23.wav` | 2,048 | `52fc04e523af3b28c4c6758cdbcafb84` |
| `sound24.wav` | 9,728 | `5a6696cda2a07969522ac62db3e66757` |

</details>

These live under `jc_resources/`. The repo tracks extracted VAGs and
other derived artifacts; the master `RESOURCE.MAP`/`RESOURCE.001` are
gitignored and must be present locally to build a CD image.

## Hardware target

| | |
|---|---|
| Main RAM | 2 MB |
| VRAM | 1 MB |
| SPU RAM | 512 KB (all 23 SFX VAGs preloaded at boot) |
| Output | 640 × 480 interlaced, NTSC |

Every rendering decision is forced by this budget. A full-frame video
approach was ruled out early (614 KB per 640×480 16-bit frame × 63
scenes ≈ gigabytes). The hybrid-playback model sidesteps that by keeping
foreground content in authored packs and a narrow runtime for
background / waves / overlays.

Full hardware reference + the gotchas hit in practice: [/docs/hardware/](https://hunterdavis.com/johnny-castaway-ps1/docs/hardware/).

## Controller mapping

Normal screensaver mode needs no input. Press **Start** to open the pause
menu.

| Control | Action |
|---|---|
| Start | Open pause menu / resume |
| D-pad / left analog | Move cursor or adjust values in menus |
| Cross | Select / apply |
| Circle | Back from any menu or submenu |

Freeplay mode is launched from the pause menu. While freeplay is active:

| Control | Action |
|---|---|
| D-pad / left analog | Walk Johnny; movement cancels the current action |
| L2 held | Slow walk |
| R2 held | Fast walk |
| Circle | Fish from the nearest side of the island |
| Select | Clear screen, cancel transient actions, and rebuild the island |
| R1 + Up | Toggle day/night |
| R1 + Down | Toggle high/low tide |
| R1 + Left | Cycle raft stage |
| R1 + Right | Cycle holiday overlay |
| Start | Open pause menu |

## Closed captions

Pause → **Accessibility** → **Captions: ON** turns on closed captions. While
playing, a dark semi-transparent band appears at the bottom of the
frame for ~5 seconds at each scene start with descriptive subtitle
text. Off by default; toggle is per-session unless saved to memcard.

The text corpus comes from the [`closed_captions`](https://github.com/huntergdavis/jc_reborn/tree/closed_captions)
branch of the upstream `jc_reborn` engine — three short lines per
scene at ~30–35 chars/line, condensed for the PS1's display
constraints. The PS1 port adds a runtime hook
(`captionsOnAdsStart`) that fires when the foreground pilot starts
each scene, plus a content-driven re-audit of the ADS-tag → caption
map (the original sequential map had ~20 mismatches; see
[`docs/ps1/caption-audit-2026-04-26.yaml`](docs/ps1/caption-audit-2026-04-26.yaml)).

Implementation:

- `src/ps1_captions.{c,h}` — caption text, scene-to-ADS map, runtime state, on-screen renderer.
- `src/foreground_pilot.c` — fires `captionsOnAdsStart("FISHING", N)` (etc.) at scene start; bails when captions are disabled, so it's free when off.
- `src/graphics_ps1.c` — `captionsRender()` draws the band + text inside `grUpdateDisplay` after the scene compose, before VSync.
- Glyph atlas is shared with the pause menu (one font upload, no duplication).

Confirmed-correct ADS+tag mappings (per visual sign-off):

| ADS+tag | scene plays | caption |
|---|---|---|
| FISHING 1 | starfish | "Johnny goes fishing. He catches a starfish. He throws it back." |
| FISHING 2 | life raft | "Johnny goes fishing. He catches a life raft. He drags it ashore." |
| FISHING 3 | octopus  | "Johnny goes fishing. He catches five green fish. Then an angry octopus. The octopus chokes him." |

Other 60 ADS+tag mappings are content-driven best-fit assignments
from the audit; HIGH-confidence matches dominate (FISHING 1–3 above,
all 6 JOHNNY, all 5 MARY, both MISCGAG, both SUZY, BUILDING 1–5).
LOW-confidence slots — STAND idle variants and a few VISITOR / WALKSTUF
edges — will be refined as more scenes are validated under the
fishing1 bar.

Full caption corpus, audit confidence breakdown by ADS file, and the
rendering pipeline: [/docs/captions/](https://hunterdavis.com/johnny-castaway-ps1/docs/captions/).

## Documentation

The website is the rendered, cross-linked, prose-context view; the
GitHub paths are the raw source. Both are kept; pick whichever you
prefer.

**Current truth** — [website](https://hunterdavis.com/johnny-castaway-ps1/about/status/)
- [scene-status.md](docs/ps1/scene-status.md) ↔ [/scenes/](https://hunterdavis.com/johnny-castaway-ps1/scenes/)
- [current-status.md](docs/ps1/current-status.md) ↔ [/about/status/](https://hunterdavis.com/johnny-castaway-ps1/about/status/)
- [development-workflow.md](docs/ps1/development-workflow.md) ↔ [/docs/dev-workflow/](https://hunterdavis.com/johnny-castaway-ps1/docs/dev-workflow/)
- [TESTING.md](docs/ps1/TESTING.md) — validation strategy
- [performance-scene-matrix.csv](docs/ps1/performance-scene-matrix.csv) ↔ [/scenes/](https://hunterdavis.com/johnny-castaway-ps1/scenes/) — 63-scene / 126-variant perf battle card
- [performance-experiment-log.md](docs/ps1/performance-experiment-log.md) — accepted and rejected optimization history
- [performance-read-candidate-matrix.md](docs/ps1/performance-read-candidate-matrix.md) + [performance-read-candidate-matrix.csv](docs/ps1/performance-read-candidate-matrix.csv) — ranked foreground read-plan candidate queue
- [performance-preprocess-opportunities.md](docs/ps1/performance-preprocess-opportunities.md) + [performance-preprocess-opportunities.csv](docs/ps1/performance-preprocess-opportunities.csv) — current FG2/FGP3 pack-time preprocessing target sheet
- [performance-o2-audit.md](docs/ps1/performance-o2-audit.md) + [performance-o2-audit.csv](docs/ps1/performance-o2-audit.csv) — current `-O2` / `-Os` sweep queue
- [docs/ps1/README.md](docs/ps1/README.md) — branch entrypoint
- [release-notes-0.8.3.md](docs/ps1/release-notes-0.8.3.md) — WALKSTUF1 compact foreground release notes
- [release-notes-0.8.2.md](docs/ps1/release-notes-0.8.2.md) — VISITOR3 guarded-read performance release notes
- [release-notes-0.8.1.md](docs/ps1/release-notes-0.8.1.md) — clean-rect pressure stability release notes
- [release-notes-0.8.0.md](docs/ps1/release-notes-0.8.0.md) — complete-scene performance baseline release notes

**Platform reference** — [website /docs/](https://hunterdavis.com/johnny-castaway-ps1/docs/)
- [hardware-specs.md](docs/ps1/hardware-specs.md) ↔ [/docs/hardware/](https://hunterdavis.com/johnny-castaway-ps1/docs/hardware/)
- [api-mapping.md](docs/ps1/api-mapping.md) ↔ [/docs/api/](https://hunterdavis.com/johnny-castaway-ps1/docs/api/)
- [build-system.md](docs/ps1/build-system.md) + [toolchain-setup.md](docs/ps1/toolchain-setup.md) ↔ [/docs/build/](https://hunterdavis.com/johnny-castaway-ps1/docs/build/)
- [pause-menu-design.md](docs/ps1/pause-menu-design.md) ↔ [/docs/pause-menu/](https://hunterdavis.com/johnny-castaway-ps1/docs/pause-menu/)
- [freeplay-mode-design.md](docs/ps1/freeplay-mode-design.md) ↔ [/docs/freeplay/](https://hunterdavis.com/johnny-castaway-ps1/docs/freeplay/)
- [regtest-harness.md](docs/ps1/regtest-harness.md) + [regtest-quickstart.md](docs/ps1/regtest-quickstart.md) ↔ [/docs/regtest/](https://hunterdavis.com/johnny-castaway-ps1/docs/regtest/)
- [scripted-input-harness.md](docs/ps1/scripted-input-harness.md) ↔ [/docs/scripted-input/](https://hunterdavis.com/johnny-castaway-ps1/docs/scripted-input/)
- [holidays-*.md](docs/ps1/) (4 files) ↔ [/docs/holidays/](https://hunterdavis.com/johnny-castaway-ps1/docs/holidays/) (with [algorithm](https://hunterdavis.com/johnny-castaway-ps1/docs/holidays/algorithm/) + [emblem gallery](https://hunterdavis.com/johnny-castaway-ps1/docs/holidays/emblems/) + 36 per-holiday pages)
- [walk-implementation-plan.md](docs/ps1/walk-implementation-plan.md) — implemented story-loop walking design and memory-stability notes for `v0.4.20-ps1`
- [release-notes-0.5.0.md](docs/ps1/release-notes-0.5.0.md) — release notes for freeplay/debug mode
- [release-notes-0.4.20.md](docs/ps1/release-notes-0.4.20.md) — release notes and soak evidence for the walking-loop milestone

**History + archaeology** — [website /archaeology/](https://hunterdavis.com/johnny-castaway-ps1/archaeology/)
- [project-history.md](docs/ps1/project-history.md) ↔ [/about/history/](https://hunterdavis.com/johnny-castaway-ps1/about/history/) and the 5-chapter [/archaeology/](https://hunterdavis.com/johnny-castaway-ps1/archaeology/) narrative
- [docs/ps1/archaeology/](docs/ps1/archaeology/) — timeline, tools, status surfaces, team perspective, assumptions, memory constraints, blog source map ↔ rendered drill-downs at [/archaeology/data/](https://hunterdavis.com/johnny-castaway-ps1/archaeology/data/), [/archaeology/timeline/](https://hunterdavis.com/johnny-castaway-ps1/archaeology/timeline/), [/archaeology/team/](https://hunterdavis.com/johnny-castaway-ps1/archaeology/team/), and per-subdirectory pages (binary-library, regtest-references, host-script-review, retired-scripts, retired-src, vision-artifacts)
- [docs/ps1/research/](docs/ps1/research/) — dated design logs ↔ [/devlog/](https://hunterdavis.com/johnny-castaway-ps1/devlog/) (rendered with editor's notes)
- [ps1-branch-cleanup-plan.yaml](docs/ps1/ps1-branch-cleanup-plan.yaml) — cleanup contract

## Repo lineage

This project began as a branch of [jno6809/jc_reborn](https://github.com/jno6809/jc_reborn)
focused on a PlayStation 1 port. It has diverged far enough (hybrid
scene-playback pipeline, per-scene captures, FG2 pack format, PS1
SPU playback path, scene-by-scene validation ledger) that it now lives
in its own repository. The original `jc_reborn` decoded the Johnny
Castaway engine — without that foundation this port wouldn't exist.

## Acknowledgements

Short list below; the full version with context per name lives at
[/credits/](https://hunterdavis.com/johnny-castaway-ps1/credits/).

- [jno6809/jc_reborn](https://github.com/jno6809/jc_reborn) — engine
  decode + the original Johnny Reborn project
- Hans Milling (`nivs1978`), [JCOS](https://github.com/nivs1978/Johnny-Castaway-Open-Source)
- Alexandre Fontoura (`xesf`), [Castaway](https://github.com/xesf/castaway)
- [Sierra Chest's Johnny Castaway archive](http://sierrachest.com/index.php?a=games&id=255&title=johnny-castaway)
- Jeff Tunnel · Kevin and Liam Ryan · Jaap · Gregori · Guido
- [PSn00bSDK](https://github.com/Lameguy64/PSn00bSDK) · [mkpsxiso](https://github.com/Lameguy64/mkpsxiso) · [DuckStation](https://github.com/stenzek/duckstation)

## Transparency

Claude, Gemini, and OpenAI Codex were all used extensively across this
project — for programming, debugging support, and generating the prose
on the website. Decisions and the merge bar are Hunter's; first drafts
often were not. The full disclosure footer carries on every page of
[the site](https://hunterdavis.com/johnny-castaway-ps1/).

## License

GPL-3.0, inherited from upstream `jc_reborn`.
