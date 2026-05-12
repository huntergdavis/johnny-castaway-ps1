# Johnny Reborn — PlayStation 1 Port

> 🌐 **Rendered version:** **[/docs/](https://hunterdavis.com/johnny-castaway-ps1/docs/)** — this doc rendered on the project website with cross-links and prose context. The GitHub copy here is the source.


Quick-start entrypoint for the PS1 branch. The primary render path is
**hybrid scene playback** (internal code name `fgpilot`): host-captured
foreground pixels, captured SFX, and a narrow PS1 runtime that handles
background, waves, holiday overlay, and SFX playback.

## Current status

| | |
|---|---|
| Release | `v0.8.7-ps1` |
| Reference scene | `FISHING 1` — pixel-perfect visuals + synced SFX across night / low-tide / holiday / raft-stage |
| Scenes fully validated under the reference bar | **63 / 63** |
| Headless perf battle card | **126 / 126** variants routed; **126 / 126** timing-bearing; **+0.3141% public over target / 99.6920% public target speed** |
| Pack corpus | High/low packs generated and routed for all 63 scenes |
| Full ledger | [scene-status.md](scene-status.md) |

`v0.8.7-ps1` is the current stability point release after the `v0.8.6-ps1`
performance follow-through. It keeps all 63 scenes visually/audibly validated,
preserves the custom Scene Explorer thumbnails and reconciled scene metadata
from `v0.8.4-ps1`, keeps the full 126-row headless matrix as the public
performance baseline, and hardens deterministic BOOTMODE scene selection,
Suzy backdrop cleanup, and heapless Scene Explorer preview loading. The public
battle card is now
`+0.3141%` over target / `99.6920%` target speed across all 126
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
the BUILDING6 scene-local slack4 guard, and the BUILDING2 high `226..242`
retained-read group; the raw signed optimization matrix is `-0.4529%`.
That is about `17.08` public over-target points removed and `12.59` public
target-speed points added since the compact full-matrix baseline. MARY1/2/3
and SUZY1/2 are measured and green; SUZY3 is not a standalone Johnny
Castaway scene route, only an asset/reference naming source.

The latest WALKSTUF1 high baseline extends the high-tide retained read groups
to `201..213`, `213..229`, and `344..360`. Against the v316 high baseline it
keeps scene and loop flat at `1768` / `1480`, moves target `1429 -> 1432`,
overrun `51 -> 48`, blocking `85 -> 83`, loop reads `69 -> 67`, and
loop-read time `301 -> 292`; due misses stay `16`, hidden refill stays `26`,
and low plus selected controls stay flat.

The latest WALKSTUF1 low baseline is the v331 staged-prepare scheduler
fallback. Low is now `1484/1431` at `96.43%` target speed; it improved
overrun `54 -> 53`, blocking `74 -> 72`, hidden refill `24 -> 22`, and
loop-read time `290 -> 288` as a bounded public-pressure tradeoff.

The latest VISITOR3 baseline is `visitor3-high-f131-resident-alias121123-v299`
plus `visitor3-low-tail-pack-only-v338`. The high pass aliases duplicate
frames `121` and `123` to frame `120`, compacts the resident setup-prime tail,
and copies frame `131` fully inside the already paid setup-prime coverage. It
keeps the `1555450` byte pack footprint and `217088` byte PS-EXE bucket fixed,
and moves high `1074/1038 -> 1070/1039`: overrun `36 -> 31`, blocking
`58 -> 49`, loop reads `10 -> 9`, loop-read time `58 -> 49`, and due misses
`10 -> 9`. The low v302 pass aliases duplicate frame `123` to frame `121`,
compacts frames `118..128` into the second setup segment, grows that segment
from `24` to `27` sectors, and copies frame `128` resident while keeping the
`1555450` byte pack footprint fixed. Low moves `1075/1039 -> 1071/1039`:
overrun `36 -> 32`, blocking `67 -> 63`, loop reads `12 -> 11`, loop-read time
`67 -> 63`, and due misses `12 -> 11`; hidden refill stays `0`.
The v338 low tail pack-only pass supersedes the v327 row by bounding frame
`143` cleanup and moving frame `144`'s terminal cleanup fully inside the
existing setup window. It keeps public timing at `1072/1040` but lowers
blocking `64 -> 58`, loop reads `11 -> 10`, loop-read time `64 -> 58`, and due
misses `11 -> 10`.
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
