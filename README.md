# Johnny Castaway — PlayStation 1

A ground-up PS1 port of Sierra's classic *Johnny Castaway* screen saver,
using a hybrid scene-playback pipeline: desktop host is the authoritative
renderer and capture source; the PS1 runtime replays authored foreground
packs + captured SFX and owns only the narrow surface it must (background,
wave animation, holiday overlay, input, SPU).

<p align="center">
  <img src="docs/readme/fishing1-ps1-cast.png" width="31%" alt="FISHING 1 on PS1: daytime cast">
  <img src="docs/readme/fishing1-ps1-raft.png" width="31%" alt="FISHING 1 on PS1: raft-stage variant">
  <img src="docs/readme/fishing1-ps1-night.png" width="31%" alt="FISHING 1 on PS1: night variant">
</p>

<p align="center">
  <code>FISHING 1</code> on PS1, captured from DuckStation: daytime cast · raft variant · night variant.
</p>

## Status

| | |
|---|---|
| Current release | **`v0.3.6-ps1`** |
| Reference scene | **`FISHING 1`** — pixel-perfect visuals + synced SFX across every applicable variant (night / low-tide / holiday / raft-stage) |
| Scenes fully validated under the reference bar | **1 / 63** |
| Per-scene ledger | [docs/ps1/scene-status.md](docs/ps1/scene-status.md) |
| Narrative status | [docs/ps1/current-status.md](docs/ps1/current-status.md) |
| Primary acceptance gate | human visual + audible signoff |

One scene at a time is promoted to the "fully validated" bar. Older
count-based validation models (`25/63`, `60/63`, `63/63` etc.) from the
harness-and-restore-pilot eras are preserved as history in
`current-status.md`; none carry forward as current progress.

## Method

The PS1 build is deliberately hybrid, not a from-scratch engine rewrite:

- **Desktop host** runs the real game logic (TTM/ADS interpreter) and
  captures every visible foreground draw plus every `PLAY_SAMPLE` opcode
  to a per-frame JSON bundle.
- A **pack compiler** turns that capture into a PS1-native v2 foreground
  pack (`.FG1`): packed visuals + per-frame sound-event table.
- On **PS1**, the runtime (`foreground_pilot.c`) loads the pack, stamps
  captured frames in step with a narrow runtime that handles background,
  wave animation, holiday overlay, and SPU playback. SFX fire on cue via
  a per-pack event cursor with a 3-frame delay so sample key-on matches
  the visible trigger.

Single scene validated end-to-end (fishing1) is the anchor for the
scene-by-scene bring-up loop.

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
[docs/ps1/development-workflow.md](docs/ps1/development-workflow.md).

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

## Controller mapping

| PSX Button | Action |
|---|---|
| Start | Pause / Unpause |
| Select | Toggle debug |
| Triangle | Advance frame (paused) |
| Circle | Toggle max speed |
| X / L1 / L2 / R1 / R2 | Reserved |

## Documentation

**Current truth**
- [docs/ps1/README.md](docs/ps1/README.md) — branch entrypoint
- [docs/ps1/scene-status.md](docs/ps1/scene-status.md) — per-scene ledger
- [docs/ps1/current-status.md](docs/ps1/current-status.md) — project narrative + history
- [docs/ps1/development-workflow.md](docs/ps1/development-workflow.md) — scene bring-up loop
- [docs/ps1/TESTING.md](docs/ps1/TESTING.md) — validation strategy

**Platform reference**
- [docs/ps1/hardware-specs.md](docs/ps1/hardware-specs.md)
- [docs/ps1/api-mapping.md](docs/ps1/api-mapping.md) — SDL2 → PSn00bSDK
- [docs/ps1/build-system.md](docs/ps1/build-system.md)
- [docs/ps1/toolchain-setup.md](docs/ps1/toolchain-setup.md)

**History + archaeology**
- [docs/ps1/project-history.md](docs/ps1/project-history.md)
- [docs/ps1/archaeology/](docs/ps1/archaeology/) — timeline, tools, status surfaces, team perspective, assumptions, memory constraints, blog source map
- [docs/ps1/research/](docs/ps1/research/) — dated design logs
- [docs/ps1/ps1-branch-cleanup-plan.yaml](docs/ps1/ps1-branch-cleanup-plan.yaml) — cleanup contract

## Repo lineage

This project began as a branch of [jno6809/jc_reborn](https://github.com/jno6809/jc_reborn)
focused on a PlayStation 1 port. It has diverged far enough (hybrid
scene-playback pipeline, per-scene captures, FGP v2 pack format, PS1
SPU playback path, scene-by-scene validation ledger) that it now lives
in its own repository. The original `jc_reborn` decoded the Johnny
Castaway engine — without that foundation this port wouldn't exist.

## Acknowledgements

- [jno6809/jc_reborn](https://github.com/jno6809/jc_reborn) — engine
  decode + the original Johnny Reborn project
- Hans Milling (`nivs1978`), [JCOS](https://github.com/nivs1978/Johnny-Castaway-Open-Source)
- Alexandre Fontoura (`xesf`), [Castaway](https://github.com/xesf/castaway)
- [Sierra Chest's Johnny Castaway archive](http://sierrachest.com/index.php?a=games&id=255&title=johnny-castaway)
- Jeff Tunnel · Kevin and Liam Ryan · Jaap · Gregori · Guido
- [PSn00bSDK](https://github.com/Lameguy64/PSn00bSDK) · [mkpsxiso](https://github.com/Lameguy64/mkpsxiso) · [DuckStation](https://github.com/stenzek/duckstation)

## License

GPL-3.0, inherited from upstream `jc_reborn`.
