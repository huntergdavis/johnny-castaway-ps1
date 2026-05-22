# Johnny Reborn — PlayStation 1 Port

> 🌐 **Rendered version:** **[/docs/](https://hunterdavis.com/johnny-castaway-ps1/docs/)** — this doc rendered on the project website with cross-links and prose context. The GitHub copy here is the source.


Quick-start entrypoint for the PS1 branch. The primary render path is
**hybrid scene playback** (internal code name `fgpilot`): host-captured
foreground pixels, captured SFX, and a narrow PS1 runtime that handles
background, waves, holiday overlay, and SFX playback.

## Current status

| | |
|---|---|
| Release | `v0.8.16-ps1` |
| Reference scene | `FISHING 1` — pixel-perfect visuals + synced SFX across night / low-tide / holiday / raft-stage |
| Scenes fully validated under the reference bar | **63 / 63** |
| Headless perf battle card | **126 / 126** variants routed; **126 / 126** timing-bearing; **+0.2340% public over target / 99.7691% public target speed** |
| Pack corpus | High/low packs generated and routed for all 63 scenes |
| Full ledger | [scene-status.md](scene-status.md) |

`v0.8.16-ps1` is the current release, with the memory-region allocator
promoted and the post-release optimization branch now focused on the final
under-99 rows. It keeps all 63 scenes visually/audibly validated, preserves
deterministic BOOTMODE scene selection and heapless Scene Explorer preview
loading, and the current battle card is `+0.2340%` over target /
`99.7691%` target speed across all 126 timing-bearing rows. The raw signed
optimization matrix is about `-0.4829%` / `100.4994%`; bands are `121`
green, `5` yellow, `0` orange, and `0` red. The latest allocator-era wins
include VISITOR3 high/low retained setup/data-shape work, the VISITOR3-high
setup-edge `40..47` retention, BUILDING4 high
setup residency, BUILDING2 high guarded read rows, BUILDING2 low `112..128`
setup residency, BUILDING4 low gap-8 dirty-upload merge plus the `24 KiB`
stream-window green promotion, WALKSTUF1 high `286..344` setup residency plus
same-speed `{395..411}` and retargeted `{411..423}` CD work, and WALKSTUF1 low `238..344` plus split
`344..350` setup residency
enabled by low-only 48 KiB clean-rect chunking, then `244..350` plus
`179..185` setup retargeting with `{113..129}` and same-speed `{355..371}`
read-work, plus the BUILDING2-high one-VBlank phase retime that moves that row
to `1340/1314`, overrun `26`, blocking/refill `45/12`, and target speed
`98.060%`. The current VISITOR3-low read-group stack adds `88..104` on top
of `16..32` and `72..88`, moving low to `1069/1039`, overrun `30`, blocking
`68`, loop reads/read time `14/91`, and due `11` while the other four yellow
rows stay exact-flat. The recent VISITOR3-high entry `62` cleanup-only pass
keeps file size, offsets, entry table sizes, LBA, and the PS-EXE bucket fixed
while reducing entry restore bytes `2724 -> 596`, runtime restore bytes
`56312 -> 54184`, and upload bytes `18038400 -> 18012160`; all five yellow
rows stay exact-flat. The recent W1-low entry `90..99` fixed-layout
canonicalization pass keeps file size, offsets, LBA, and the PS-EXE bucket
fixed while shrinking the active cluster `47579 -> 44511` bytes and total
low-pack active payload `755808 -> 752740`; all five yellow rows stay
exact-flat. The recent code-headroom pass caches the active
foreground scene ID once per scene start, keeping the five yellow rows
exact-flat while shrinking the tracked hot foreground scheduler symbols by
`172` bytes in the same `233472` byte PS-EXE bucket. MARY1/2/3 and SUZY1/2 are
measured and green; SUZY3 is not a standalone Johnny Castaway scene route,
only an asset/reference naming source.

The latest JOHNNY1 baseline, `johnny1-local-lz-v932`, stores a scene-local
copy/literal stream behind a sentinel inside the existing FGP3/v4 payloads.
Both high and low compress entries `1` and `50`, cutting active payload
`316608 -> 112093` bytes and moving both rows from `1973/1945` to `1948/1945`
with overrun `28 -> 3`, blocking/refill `25 -> 5`, loop read time `58 -> 37`,
and target speed `98.56% -> 99.85%`.

The latest WALKSTUF1 high allocator-era baseline keeps `198..244` and
`286..344` resident, carries the `{149,165}` read group, and encodes frame
`92` as previous-frame D4, then layers the frame56/source67 trim, `{178,194}`,
`{423,439}`, `{404,416}`, `{395,411}`, retargeted `{411,423}`, and high-tide prepare-before-window scheduler
ownership. It measures `1472/1441` at `97.894%` target speed, overrun `31`,
blocking/refill `43/13`, loop reads/read time `41/198`, and due `7`,
with pack LBA/sectors and the PS-EXE bucket fixed.

The latest WALKSTUF1 low allocator-era baseline keeps the no-shift payload
shrinks, replaces the old split tail residency with a CACHE `238..344` setup
segment, adds the `{91,107}` first-boundary read group, pays the small
`344..350` edge from TRANSIENT, trims frame132's draw tail in the compacted low
pack, adds `{378..390}`, then retargets setup to `244..350` plus split
`179..185` with `{113..129}` and adds `{355..371}` as same-speed read-work.
Low now measures `1470/1446` at `98.367%` target speed with overrun `24`,
blocking/refill `32/3`, loop reads/read time `24/146`, and due `4`, while
active payload is `764658` without changing pack size or sectors.

The latest BUILDING2 high allocator-era baseline keeps retained groups
`60..72`, `206..230`, `226..242`, `83..95`, `{158..174}`, guarded `271..287`,
`315..327`, and `{185..197}`, layers the previous-visible cleanup,
screen-clip, trim-tail, duplicate-alias, and safe draw-tail headroom passes,
then adds a one-VBlank high-tide phase retime. It measures `1340/1314` at
`98.060%` target speed, overrun `26`, blocking/refill `45/12`, loop reads/read
time `44/192`, and due `7`, with pack LBA/sectors and the `233472` byte PS-EXE
bucket fixed.

The latest BUILDING2 low baseline keeps the v626 `218..229` slack8 retained
row, v660 offscreen low-tide draw-span clip, and dead draw-tail payload trim,
then primes relative sectors `112..128` during setup. It keeps loop flat at
`1339`, improves target `1315 -> 1316`, overrun `24 -> 23`, blocking
`54 -> 53`, loop reads/read time `37/152 -> 34/141`, and due `12 -> 11`
while preserving pack size, LBA/sectors, and the `233472` byte PS-EXE bucket.
The row now measures `98.282%` target speed.

The current VISITOR3 high baseline keeps the resident-copy and D4 data-shape
work, merges the overlapping terminal setup coverage into retained relative
sectors `203..262`, keeps the `277..293` setup segment and early `40..47`
setup edge, then layers the `48..55` clean-relief segment retune, six exact
cleanup rects, and three-VBlank loop phase ballast. It keeps the `1555450` byte
pack footprint, `22619/760` LBA/sectors, and `233472` byte PS-EXE bucket fixed
while measuring `1067/1045` at `97.938%` target speed with overrun `22`,
blocking/refill `32/0`, loop reads/read time `12/85`, and due `2`.
The current low row keeps the resident-slot/D4 data-shape work, extends its
retained setup segment through `206..232`, relocates frame `138` raw into that
paid gap, and adds the `16..32`, `72..88`, and `88..104` read-group stack:
`1069/1039`, overrun `30`, blocking/refill `68/0`, loop reads/read time
`14/91`, and due `11`.
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
- [release-notes-0.8.14.md](release-notes-0.8.14.md) — JOHNNY1 local-LZ green promotion release notes
- [release-notes-0.8.13.md](release-notes-0.8.13.md) — under-99 payload-work checkpoint release notes
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
