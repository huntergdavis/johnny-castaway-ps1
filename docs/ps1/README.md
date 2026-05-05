# Johnny Reborn — PlayStation 1 Port

> 🌐 **Rendered version:** **[/docs/](https://hunterdavis.com/johnny-castaway-ps1/docs/)** — this doc rendered on the project website with cross-links and prose context. The GitHub copy here is the source.


Quick-start entrypoint for the PS1 branch. The primary render path is
**hybrid scene playback** (internal code name `fgpilot`): host-captured
foreground pixels, captured SFX, and a narrow PS1 runtime that handles
background, waves, holiday overlay, and SFX playback.

## Current status

| | |
|---|---|
| Release | `v0.6.13-ps1` |
| Reference scene | `FISHING 1` — pixel-perfect visuals + synced SFX across night / low-tide / holiday / raft-stage |
| Scenes fully validated under the reference bar | **50 / 63** (`ACTIVITY 8`, `ACTIVITY 10`, `ACTIVITY 11`, `ACTIVITY 12`, `FISHING 1`, `FISHING 2`, `FISHING 3`, `FISHING 4`, `FISHING 5`, `FISHING 6`, `FISHING 7`, `FISHING 8`, `JOHNNY 1`, `JOHNNY 2`, `JOHNNY 3`, `JOHNNY 4`, `JOHNNY 5`, `JOHNNY 6`, `MARY 1`, `MARY 2`, `MARY 3`, `MARY 4`, `MARY 5`, `MISCGAG 1`, `MISCGAG 2`, `STAND 1`, `STAND 2`, `STAND 3`, `STAND 4`, `STAND 5`, `STAND 6`, `STAND 7`, `STAND 8`, `STAND 9`, `STAND 10`, `STAND 11`, `STAND 12`, `STAND 15`, `STAND 16`, `SUZY 1`, `SUZY 2`, `VISITOR 1`, `VISITOR 3`, `VISITOR 4`, `VISITOR 5`, `VISITOR 6`, `VISITOR 7`, `WALKSTUF 1`, `WALKSTUF 2`, `WALKSTUF 3`) |
| Pack corpus | FG2 high/low packs generated for all 63 scenes; CD/runtime routing remains scene-by-scene |
| Full ledger | [scene-status.md](scene-status.md) |

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
