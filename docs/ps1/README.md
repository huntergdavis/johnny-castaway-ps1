# Johnny Reborn — PlayStation 1 Port

> 🌐 **Rendered version:** **[/docs/](https://hunterdavis.com/johnny-castaway-ps1/docs/)** — this doc rendered on the project website with cross-links and prose context. The GitHub copy here is the source.


Quick-start entrypoint for the PS1 branch. The primary render path is
**hybrid scene playback** (internal code name `fgpilot`): host-captured
foreground pixels, captured SFX, and a narrow PS1 runtime that handles
background, waves, holiday overlay, and SFX playback.

## Current status

| | |
|---|---|
| Release | `v0.8.12-ps1` |
| Reference scene | `FISHING 1` — pixel-perfect visuals + synced SFX across night / low-tide / holiday / raft-stage |
| Scenes fully validated under the reference bar | **63 / 63** |
| Headless perf battle card | **126 / 126** variants routed; **126 / 126** timing-bearing; **+0.2697% public over target / 99.7347% public target speed** |
| Pack corpus | High/low packs generated and routed for all 63 scenes |
| Full ledger | [scene-status.md](scene-status.md) |

`v0.8.12-ps1` is the current performance point release, with later mainline
optimization wins tracked in the battle card and matrix.
It keeps all 63 scenes visually/audibly validated, preserves deterministic
BOOTMODE scene selection and heapless Scene Explorer preview loading, and the
latest mainline promotes the BUILDING4 low offscreen draw-span clip plus the
v913 frame283 fixed-sector payload shrink, the
BUILDING2 high/low offscreen draw-span work-volume clips, the BUILDING2 low
draw-tail trim, the WALKSTUF1 high/low late-tail work-volume clips plus the W1-high frame55/frame138/frame51/frame49/frame47/frame45/frame43/frame56/frame57/frame136/frame135/frame139 offscreen clips, the W1-high frame51/frame49/frame47/frame45/frame43/frame138/frame135 preserve-offset payload shrinks, the WALKSTUF1 low `394..410`
retained-read win, and the WALKSTUF1 high late-tail physical compaction after
the VISITOR3 high tail-pack repack and the VISITOR5 high/low and BUILDING2 low
retained-read wins; the current low fixed-sector payload lane is now
`walkstuf1-low-frame106-inplace-v910`. The
public battle card is now `+0.2697%` over target /
`99.7347%` target speed across all 126
timing-bearing rows after the MARY3, BUILDING1, VISITOR5 high, BUILDING2 low,
WALKSTUF3 high, BUILDING6 compact, ACTIVITY9 high compact, and WALKSTUF3 low
compact, JOHNNY1 compact, ACTIVITY9 low compact, and VISITOR3 motion-copy plus
low/high setup-segment, high frame-126/frame-125 re-anchor, and high
setup-prime plus guarded low second-segment and low frame-125/frame-126
resident re-anchor plus low frame-118/frame-127 resident-copy, high
frame-127/frame-130 resident-copy, low frame-114/frame-117 no-op residual,
low frame-113 no-op residual, the VISITOR3 high frame-140 setup-segment copy,
VISITOR3 low frame-114/frame-117 no-op payload aliasing, and the VISITOR3
high frame-121/frame-123 resident alias plus frame-131 setup-prime copy,
VISITOR3 low frame-123 resident alias plus frame-128 setup-segment copy,
and WALKSTUF1 high sector `201..213` read-group follow-ups plus the WALKSTUF1
high gap-compressed/window-prefetch guard and low gap6-prefix plus slack-guard
follow-up, the WALKSTUF1 high `213..229` read-group/slack4 promotion, the
VISITOR3 low frame128/frame129 resident-slot swap, the WALKSTUF1 low
staged-prepare-before-window scheduler fallback, the VISITOR3 low tail
pack-only compaction, the WALKSTUF1 high `344..360` read-group promotion,
the BUILDING6 scene-local slack4 guard, the BUILDING2 high `226..242`
and `206..230` retained-read groups plus 24-sector grouped-read capacity,
the WALKSTUF1 high `422..434` / `444..456`
CD-work reductions, and the shared WALKSTUF1 low/high `443..455` /
`444..456` dual-tail reduction, plus VISITOR3 low frame129/frame132 and high
frame132/frame137 D4 previous-frame deltas plus the VISITOR3 high one-sector
frame132/frame137 setup segment and VISITOR3 low frame132 setup-prime gap relocation,
the VISITOR5 high/low `30..46` retained-read groups, the BUILDING2 low
`218..229` slack8 row and v739 draw-tail trim, the VISITOR3 high `277..293` tail-pack repack, the
BUILDING4 low offscreen draw-span clip, the BUILDING2 high/low offscreen
draw-span work-volume clips, the W1-high frame55/frame138/frame51/frame49/frame47/frame45/frame43/frame56/frame57/frame136/frame135/frame139 offscreen clips plus the W1-high frame51/frame49/frame47/frame45/frame43/frame138/frame135 in-place payload shrinks, the W1-low frame79/frame81/frame129/frame139/frame87/frame89/frame98/frame27/frame101/frame93/frame94/frame97/frame99/frame100/frame134/frame91/frame92/frame95/frame140/frame108/frame109/frame107/frame110/frame111/frame106 in-place payload shrinks, plus the JOHNNY1 low refresh; the raw signed
optimization matrix is `-0.4975%` / `100.5171%`.
That is about `17.13` public over-target points removed and `12.63` public
target-speed points added since the compact full-matrix baseline. MARY1/2/3
and SUZY1/2 are measured and green; SUZY3 is not a standalone Johnny
Castaway scene route, only an asset/reference naming source.

The latest WALKSTUF1 high baseline extends the shared retained read groups to
`201..213`, `213..229`, `344..360`, `422..434`, `443..455`, and `444..456`,
then physically compacts the already-clipped late-tail payloads, clips the
offscreen frame `55`, frame `138`, frame `51`, frame `49`, frame `47`, frame `45`, frame `43`, frame `56`, frame `57`, frame `136`, frame `135`, and frame `139` draw streams, and shrinks frames `51`, `49`, `47`, `45`, `43`, `138`, and `135` in place with preserved offsets. It is still a same-speed row at `1764`,
active loop/target `1476/1434`, overrun `42`, and blocking/refill `81/23`, but
active payload drops `918345 -> 860009`, CD sectors `605 -> 586`, loop
reads/read time `65/282 -> 63/276`, and runtime rows/spans/pixels
`17296/134136/776856 -> 16859/129919/731016`; due misses stay `16`, pack
LBA/sectors stay `24883/750`, and the PS-EXE bucket stays `217088`. The tail
groups, physical compaction, frame55/frame138/frame51/frame49/frame47/frame45/frame43/frame56/frame57/frame136/frame135/frame139 clips, and frame51/frame49/frame47/frame45/frame43/frame138/frame135 in-place shrinks are same-speed work reductions,
so they do not count as VBlank speed wins.

The latest WALKSTUF1 low baseline is the v474 first post-prime boundary group
on top of the shared tail groups, v331 staged-prepare scheduler fallback,
v705 late-tail physical compaction, isolated offscreen work-volume clips
through v726, the v747/v749/v750/v751/v753/v755/v756/v757/v759/v762/v763/v766/v767/v769/v770/v771/v772/v773/v774/v775/v776/v777/v779/v780/v781/v782/v783/v784/v785/v786/v787/v788/v789/v790/v791/v794/v795/v797/v798/v800/v801/v802/v846/v847/v849/v852/v853/v855/v859/v860/v861/v862/v863/v864/v865/v866/v867/v868/v869/v870/v871/v872/v873/v874/v875/v876/v908/v909/v910 no-shift
in-place shrinks for frames `51`, `49`, `47`, `61`, `62`, `58`, `45`, `37`,
`35`, `43`, `41`, `57`, `33`, `67`, `68`, `69`, `32`, `133`, `5`, `141`,
`70`, `30`, `6`, `71`, `72`, `142`, `73`, `131`, `74`, `19`, `28`, `138`,
`145`, `75`, `76`, `77`, `130`, `135`, `1`, `88`, `90`, `3`, `53`, `136`, `79`, `81`, `129`, `139`, `87`, `89`, `98`, `27`, `101`, `93`, `94`, `97`, `99`, `100`, `134`, `91`, `92`, `95`, `140`, `108`, `109`, `107`, `110`, `111`, and `106`, the
v760 bounded CD fast-poll restoration, and the v817 low `394..410` retained
read group. Low is now `1477/1432` at `96.95%` target speed with overrun `45`,
blocking/refill `65/20`, loop reads/read time `58/259`, and due misses `11`,
while active payload drops `879801 -> 790208` without moving pack offsets,
LBA/sectors, or the PS-EXE bucket.

The latest BUILDING2 high baseline keeps retained groups `60..72`, `206..230`,
`226..242`, and `249..257` with the grouped-read window capacity raised to 24
sectors. The v441 pass improves the current high row from scene `1603` to
`1602`, active loop `1352/1311 -> 1351/1311`, overrun `41 -> 40`, blocking
`55 -> 54`, hidden refill `19 -> 18`, loop reads `61 -> 58`, and loop-read time
`262 -> 257`; due misses stay `7`, pack LBA stays fixed, and BUILDING2 low,
WALKSTUF1 high/low, and VISITOR3 low canaries stay flat. The v664 follow-up
clips late offscreen draw spans for frames `168..177`, v698 extends the safe
same-speed subset to frames `94..104`, v700/v701/v702/v703 add boundary frames
`92`, `91`, `90`, and `89`, and v877/v879/v880 trim entries `172` / source
frame `231`, `171` / source frame `228`, and `96` / source frame `119` in place
with preserved offsets. All stay exact-flat at `1351/1311`, overrun `40`,
blocking/refill `54/18`, reads/read time `58/257`, and due `7`, while dropping
runtime frame rows/spans/pixels from `18144/110717/468636 -> 18030/105645/446246`
and active payload `674798 -> 672026`.

The latest BUILDING2 low baseline keeps the v626 `218..229` slack8 retained
row and v660 offscreen low-tide draw-span clip, then trims dead draw-tail
payload inside the existing pack footprint. It improves to `1339/1317`,
overrun `22`, blocking/refill `53/0`, loop reads/read time `37/150`, and due
misses `12` while trimming active payload `660236 -> 538534` across `41`
entries. Pack size, LBA, sectors, and the `217088` byte PS-EXE bucket stay
fixed, so this now counts as a speed-bearing BUILDING2 low baseline.

The latest VISITOR3 high baseline keeps the resident-copy and D4 data-shape
work, preloads the 768-byte frame `132` and 503-byte frame `137` D4 payloads
through the same one-sector setup segment at sector `203`, then reuses the
proven low-tail compact cleanup payloads and repacks frames
`141/140/142/143/144` plus sound events inside the existing `277..293` setup
segment. It keeps the `1555450` byte pack footprint, `22611/760` LBA/sectors,
and `217088` byte PS-EXE bucket fixed while moving high `1065/1039 -> 1063/1040`:
overrun `26 -> 23`, blocking/read time `41 -> 35`, loop reads `7 -> 6`, and
due misses `7 -> 6`; hidden refill stays `0`. The latest low row keeps the v452 frame128/frame129 resident-slot
swap plus frame129 D4 delta and v470 frame132 D4 delta, then relocates frame
`137` into an unused setup-prime gap at sector `99`: `1062/1040`, overrun
`22`, blocking `42`, loop reads/due `7/7`, hidden refill `0`.
VISITOR3 remains a custom data-shape target, but local threshold/read-table/tail-atlas,
metadata-shrink, row-copy, and generic narrow-upload probes stay closed; future
work should build on scene-owned motion/precomposed data or generated scheduler
ownership.

`v0.8.2-ps1` is the prior performance point release after `v0.8.1-ps1`.
It keeps all 63 scenes visually/audibly validated, preserves the routed
126-variant headless matrix, and is now followed by the FGP3/v4 compact
metadata reader inline baseline, VISITOR3 pack-side restore-minus-current
cleanup, BUILDING4 pack-side restore-minus-current cleanup, VISITOR3
read-table headroom cleanup, WALKSTUF1 high setup-prime cap retune, VISITOR3
high/low offscreen draw clips, BUILDING2 high restore-minus-current cleanup,
the BUILDING2 high `60..72` grouped-read pass, and the BUILDING2 low
`365..381` grouped-read pass, the VISITOR3 low code-shape pass, and the
VISITOR3 v4 draw-tail trim plus stage guard. Those internal checkpoints are
superseded by the current public-capped `v0.8.7` rollup above.

`v0.8.1-ps1` is a clean-rect pressure stability point release. It fixes a
randomized long-run scene-load freeze by estimating the actual clean
background backup footprint before allocation, including ocean wave-band
expansion and upper/lower split rects. Focused `MARY 4` and representative
`FISHING 1` pressure routes complete with `scene-end` and `alloc_fail=0`.

`v0.8.0-ps1` is the complete-scene performance baseline. Every original
scene remains validated under the visual + audible signoff bar, every high
and low tide scene variant is routed through the headless matrix, and the
current timing-bearing rows now use the public-capped `v0.8.7` rollup above
after the post-release VISITOR3, BUILDING2, BUILDING4,
ACTIVITY9, JOHNNY2 clean-pressure, WALKSTUF1, selector-cleanup, FGP3/v4
compact draw metadata, compact decoder inline, and pack-side
restore-minus-current promotions plus the current grouped-read and VISITOR3
code-shape/data-shape promotions.
MARY2's padded FGP3 conversion exposed a clean-memory prefetch miss; the
MARY2-local relief restores `stage1_window`, moves high/low to `2241/2248`
and `2242/2250`, and collapses due misses from `233` to `0`.

`v0.7.2-ps1` fixes the story-loop walking regression where Johnny could walk
over water when the next scene used a different island backdrop key.

`v0.7.1-ps1` adds persisted holiday mode and makes `AUTO DATE:ORIG4` the
fresh/no-card default.

`v0.7.0-ps1` is the complete-scene validation milestone: all 63 original
scenes are validated under the current bar.

`v0.6.13-ps1` validates `VISITOR 4`, `VISITOR 5`, `VISITOR 6`, and
`VISITOR 7`. VISITOR4, VISITOR5, and VISITOR7 use the generic
normal/far-left/far-right multi-view stitch; VISITOR5 and VISITOR7 also
redistribute hold time into coconut impact/action rows. VISITOR6 adds a
narrow full-host impact-delta injection over source frames 120:141 because
the tree shake/strike pixels are background-owned in the host render.

`v0.6.12-ps1` validates `VISITOR 3` after VISITOR3-specific multi-view
synthesis rebuilt the high/low packs, and also validates `WALKSTUF 3`
against the existing on-disc `WALK3.FG2` / `WALK3LOW.FG2` packs without
rework. Clean foreground-only views provide the VISITOR3 moving sprites,
the helper accumulates the red ship hull only from live full-host crash
frames, FGP3 cleanup clears the post-crash rows, and hold timing keeps the
real splash frame visible without stale right-side residue.

`v0.6.10-ps1` validates `MARY 5` after generic multi-view
scene-relative stitching rebuilt the high/low packs with the generic raft
off. Direct scene playback now respects story flags too: `NORAFT` clamps
the external raft off, and `FIRST` skips the walk prelude before full-wipe
scenes.

The release also includes the post-`v0.6.11-ps1` validations for `STAND 2`-`STAND 12`,
`STAND 15`, `STAND 16`, `SUZY 1`, `SUZY 2`, and `VISITOR 1`. `STAND 5`-`STAND 9`,
`STAND 15`, and `STAND 16` use the STAND no-stitch
export fast path with a full-frame single-position foreground-only
overlay so static frame-0 Johnny pixels are not dropped. The FG2
runtime now also ticks ocean wave animation every frame so no-stitch
scenes get moving water from the engine instead of from the
foreground-only pack; scenes whose pack carries its own water frames
are unaffected because the foreground compose still draws on top.
`SUZY 1` and `SUZY 2` use the source `SUZBEACH.SCR` backdrop instead
of the island/ocean runtime path. `SUZY 2` also keeps `MRAFT.BMP` in
the foreground overlay so Johnny rides the raft, and the SFX mixer now
has headroom for overlapping raft samples. `VISITOR 1` validates through
the standard multi-view stitch with one captured SFX event. The scene-loader path now skips the stale
walk prelude before direct scene launches, and high-pressure clean
snapshots release optional walk/prefetch caches before allocating.

`v0.6.8-ps1` is a scene-validation bugfix release: `MARY 2` is now
validated after a wide scene-relative multi-view stitch restored
edge-clipped line, mermaid, boot/splash, and lower-water pixels, while
full-host bubble injection restored the fish thought-bubble shell.
Production island placement remains variable.

`v0.6.6-ps1` is a scene-validation bugfix release: `FISHING 7` and
`FISHING 8` were rebuilt from far-left full-frame foreground-only host
captures, then validated with far-left stress playback. The old runtime
island-position pins are gone; both scenes now follow normal random
island placement. It builds on `v0.6.5-ps1` (`FISHING 5` shark capture
cleanup and `JOHNNY 4` variable-position bottle-message validation),
`v0.6.4-ps1`
(`JOHNNY 2`), `v0.6.3-ps1` (`FISHING 7`, `FISHING 8`, `JOHNNY 1`),
`v0.6.2-ps1` (`FISHING 6`), `v0.6.1-ps1` (freeplay clean-rect placement fix), `v0.6.0-ps1`
(ocean ambience), and `v0.5.0-ps1`
(freeplay/debug mode). See [release-notes-0.5.0.md](release-notes-0.5.0.md)
and [freeplay-mode-design.md](freeplay-mode-design.md) for the freeplay
milestone.

The previous `v0.4.20-ps1` release promoted the story-loop walk connector:
Johnny walks from one scene endpoint to the next instead of teleporting
between FG2 packs. See [release-notes-0.4.20.md](release-notes-0.4.20.md).

"Fully validated" means human visual + audible signoff on the scene-playback
path. Older counts (`25/63`, `60/63`, etc.) belong to earlier validation
models and are preserved as history in `current-status.md`, not carried
forward as current progress.

## Quick start

### Prerequisites
- Docker (for the `jc-reborn-ps1-dev:amd64` build image + PSn00bSDK)
- DuckStation (Flatpak: `org.duckstation.DuckStation`)

### Build + run the reference scene
```bash
./scripts/rebuild-and-let-run.sh noclean
```
Builds the PS1 executable, generates `jcreborn.bin/.cue`, launches
DuckStation pointed at the cue. Boots into `FISHING 1` via `BOOTMODE.TXT`
(`fgpilot fishing1`).

### Validate a variant
```bash
# night / low-tide / raft-stage / holiday
./scripts/rebuild-and-let-run.sh noclean fgpilot fishing1 night 1
./scripts/rebuild-and-let-run.sh noclean fgpilot fishing1 lowtide 1
./scripts/rebuild-and-let-run.sh noclean fgpilot fishing1 raft-stage 5
./scripts/rebuild-and-let-run.sh noclean fgpilot fishing1 holiday 4
```

### Bring up a new scene
See [development-workflow.md](development-workflow.md) for the full
capture → pack → validate loop. High level:
1. `./scripts/export-scene-foreground-pilot.sh` — host high/low capture → base-diff FG2 packs + sound-event JSONL.
2. Add the FG2 pack entries to `config/ps1/cd_layout.xml`.
3. Add scene routing in `foreground_pilot.c`.
4. Rebuild ISO, launch via `rebuild-and-let-run.sh`, iterate to pixel-perfect.
5. Tick the row in `scene-status.md`.

## Why PS1?

Johnny Reborn's tight memory footprint and native 640×480 target fit PS1
closely:
- **Main RAM**: 2 MB
- **VRAM**: 1 MB
- **SPU RAM**: 512 KB (holds all 23 SFX VAGs preloaded at boot)
- **Native output**: 640×480 interlaced

The port is deliberately **hybrid**: the desktop host is the authoritative
renderer and capture source; PS1 replays captured foreground + SFX and
owns only the narrow runtime surface (background, wave animation,
holiday overlay, SPU playback, input).

### Architecture

**Desktop host capture still uses:**
- Core engine (`ttm.c`, `ads.c`, `story.c`)
- Game logic (`walk.c`, `calcpath.c`, `island.c`)
- Utilities (`utils.c`, `config.c`, `bench.c`)

**Linked into the PS1 executable:**
- Minimal boot + scene loop (`jc_reborn.c`)
- Resource metadata + LRU support (`resource.c`, `utils.c`, `uncompress.c`)
- Background/island helpers (`island.c`)
- Scene-playback runtime (`foreground_pilot.c`)

**PS1-specific:**
- `graphics_ps1.c` — PSn00bSDK GPU + software compositing
- `sound_ps1.c` — SPU playback (VAG preload at boot + round-robin voices)
- `events_ps1.c` — PSX controller input
- `cdrom_ps1.c` — CD-ROM file I/O
- `foreground_pilot.c` — FG2 pack loader, frame-advance, SFX event firing

**Offline pipeline (`scripts/`):**
- `capture-host-scene.sh` — desktop capture (frames + metadata + sound events)
- `export-scene-foreground-pilot.sh` — wraps capture + pack build for a scene
- `build-scene-foreground-pack.py` — FG2 span compiler (visuals + SFX)
- `wav2vag.py` — WAV → PS1 SPU ADPCM VAG encoder
- `make-cd-image.sh` / `build-ps1.sh` / `rebuild-and-let-run.sh` — build + launch

## Controller mapping

Normal screensaver mode needs no input. Press **Start** for the pause
menu.

| Control | Action |
|---|---|
| Start | Open pause menu / resume |
| D-pad / left analog | Move cursor or adjust values in menus |
| Cross | Select / apply |
| Circle | Back from any menu or submenu |

Freeplay mode is launched from the pause menu:

| Control | Action |
|---|---|
| D-pad / left analog | Walk Johnny; movement cancels the current action |
| L2 held | Slow walk |
| R2 held | Fast walk |
| Circle | Fish from the nearest side |
| Select | Clear screen and rebuild the island |
| R1 + Up | Toggle day/night |
| R1 + Down | Toggle high/low tide |
| R1 + Left | Cycle raft stage |
| R1 + Right | Cycle holiday overlay |
| Start | Open pause menu |

## Documentation

**Current truth**
- [scene-status.md](scene-status.md) — per-scene ledger under the reference bar
- [current-status.md](current-status.md) — detailed progress + history of earlier validation models
- [milestones-2026-04-25.md](milestones-2026-04-25.md) — TTY printf, perf module + Docker regtest, pause menu, holiday-expansion design, SPI driver, memcard
- [pause-menu-design.md](pause-menu-design.md) — locked pause-menu design
- [holidays-expansion-design.md](holidays-expansion-design.md) — 35-holiday plan
- [performance-optimization-plan.md](performance-optimization-plan.md) — perf backlog
- [performance-preprocess-opportunities.md](performance-preprocess-opportunities.md) + [performance-preprocess-opportunities.csv](performance-preprocess-opportunities.csv) — current FG2/FGP3 pack-time preprocessing target sheet
- [performance-o2-audit.md](performance-o2-audit.md) + [performance-o2-audit.csv](performance-o2-audit.csv) — current `-O2` / `-Os` sweep queue
- [development-workflow.md](development-workflow.md) — operator loop for bringing up a new scene
- [TESTING.md](TESTING.md) — validation strategy (primary = human signoff; regtest = legacy)
- [release-notes-0.8.9.md](release-notes-0.8.9.md) — WALKSTUF1 low in-place payload reduction release notes
- [release-notes-0.8.8.md](release-notes-0.8.8.md) — VISITOR5 high retained-read promotion release notes
- [release-notes-0.8.7.md](release-notes-0.8.7.md) — deterministic boot selection and Scene Explorer preview stability release notes
- [release-notes-0.8.6.md](release-notes-0.8.6.md) — WALKSTUF1 / VISITOR3 setup-segment compaction release notes
- [release-notes-0.8.5.md](release-notes-0.8.5.md) — full 126-row headless performance matrix release notes
- [release-notes-0.8.4.md](release-notes-0.8.4.md) — custom Scene Explorer thumbnail release notes
- [release-notes-0.8.3.md](release-notes-0.8.3.md) — WALKSTUF1 compact foreground release notes
- [release-notes-0.8.2.md](release-notes-0.8.2.md) — VISITOR3 guarded-read performance release notes
- [release-notes-0.8.1.md](release-notes-0.8.1.md) — clean-rect pressure stability release notes
- [release-notes-0.8.0.md](release-notes-0.8.0.md) — complete-scene performance baseline release notes
- [walk-implementation-plan.md](walk-implementation-plan.md) — story-loop walk connector, including `v0.4.20` implementation notes
- [freeplay-mode-design.md](freeplay-mode-design.md) — freeplay/debug mode controls, menus, memory rules, and telemetry
- [release-notes-0.5.0.md](release-notes-0.5.0.md) — release notes for the freeplay/debug milestone
- [release-notes-0.4.20.md](release-notes-0.4.20.md) — release notes and soak evidence for the walking-loop milestone

**Platform reference**
- [hardware-specs.md](hardware-specs.md) — PS1 hardware
- [api-mapping.md](api-mapping.md) — SDL2 → PSn00bSDK mapping
- [build-system.md](build-system.md) — CMake / Docker / CD generation
- [toolchain-setup.md](toolchain-setup.md) — dev environment

**History / archaeology** (kept searchable, not current truth)
- [project-history.md](project-history.md)
- [research/README.md](research/README.md) — design logs and prior status snapshots
- [audio-optimization-spec.md](audio-optimization-spec.md)
- [ps1-branch-cleanup-plan.yaml](ps1-branch-cleanup-plan.yaml) — in-flight cleanup contract

**Retired paths:** FG1/FOC packs, per-scene establishing RAWs, and
ADS/TTM console runtime routes are historical only. Do not add new active
generation, routing, CD entries, or docs for those paths.

## External references

- [PSn00bSDK](https://github.com/Lameguy64/PSn00bSDK)
- [mkpsxiso](https://github.com/Lameguy64/mkpsxiso)
- [DuckStation](https://github.com/stenzek/duckstation)
- [PS1 Dev Resources](https://psx.arthus.net/)

## License

GPL-3.0, same as main project.
