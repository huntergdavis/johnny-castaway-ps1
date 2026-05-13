# Johnny Castaway — PlayStation 1

A PS1 port of Sierra's classic *Johnny Castaway* screen saver. All 63
scenes are validated under the project's pixel-perfect visual + synced-SFX
bar. The runtime is a hybrid pipeline — the desktop host captures every
scene from the real Sierra engine into authored foreground packs; the PS1
replays those packs and owns only the narrow surface it must (background,
wave animation, holiday overlays, controller input, SPU audio).

> **Website ▸ [hunterdavis.com/johnny-castaway-ps1](https://hunterdavis.com/johnny-castaway-ps1/)** — full project site with the live scene ledger, performance battle card, deep-dive essays, devlog, and history. *This README is the short version.*

**Quick links:** [Download](#download-and-play) · [Status](#status) · [Quick start](#quick-start) · [Method](#method) · [Pause menu](#pause-menu) · [Captions](#closed-captions) · [Holidays](#holidays) · [Hardware](#hardware-target) · [Controls](#controls) · [Documentation](#documentation) · [Acknowledgements](#acknowledgements) · [License](#license)

<p align="center">
  <img src="docs/readme/johnny6-ps1-office.png" width="47%" alt="JOHNNY 6 on PS1: Johnny working in an office">
  <img src="docs/readme/johnny6-ps1-date-dream.png" width="47%" alt="JOHNNY 6 on PS1: Johnny dreaming about his island date">
</p>

<p align="center"><code>JOHNNY 6</code> on PS1 — office daydream · island date dream.</p>

<p align="center">
  <img src="docs/readme/fishing1-ps1-cast.png" width="31%" alt="FISHING 1 on PS1: daytime cast">
  <img src="docs/readme/fishing1-ps1-raft.png" width="31%" alt="FISHING 1 on PS1: raft-stage variant">
  <img src="docs/readme/fishing1-ps1-night.png" width="31%" alt="FISHING 1 on PS1: night variant">
</p>

<p align="center"><code>FISHING 1</code> — the project's reference scene: daytime cast · raft variant · night variant.</p>

## Download and play

Latest release → **[Releases page](https://github.com/huntergdavis/johnny-castaway-ps1/releases/latest)** · or read the **[Play page](https://hunterdavis.com/johnny-castaway-ps1/play/)** for the full quickstart with controller map.

Direct download (auto-tracks the latest tag):

- **[jcreborn.bin](https://github.com/huntergdavis/johnny-castaway-ps1/releases/latest/download/jcreborn.bin)** — PS1 CD image
- **[jcreborn.cue](https://github.com/huntergdavis/johnny-castaway-ps1/releases/latest/download/jcreborn.cue)** — cuesheet

Load `jcreborn.cue` in [DuckStation](https://www.duckstation.org/) (or any PS1 emulator). Boots straight into the screensaver loop; press **Start** for the pause menu.

## Status

| | |
|---|---|
| Current release | **`v0.8.7-ps1`** — deterministic BOOTMODE scene selection + Scene Explorer preview stability |
| Reference bar | **`FISHING 1`** — pixel-perfect visuals + synced SFX across every applicable variant (night / low-tide / holiday / raft-stage) |
| Scenes validated | **63 / 63** — see the live [scene ledger](https://hunterdavis.com/johnny-castaway-ps1/scenes/) or [`docs/ps1/scene-status.md`](docs/ps1/scene-status.md) |
| Headless perf | **126 / 126** scene/tide rows are routed and timing-bearing; public-capped average is **+0.3033% over target / 99.7023% target speed**. Live battle card at [/perf/](https://hunterdavis.com/johnny-castaway-ps1/perf/) · CSV at [`performance-scene-matrix.csv`](docs/ps1/performance-scene-matrix.csv) |
| Perf harness | `--require-improvement` gates now fail if the supplied baseline summary has no matching case label, preventing false-pass optimization promotions. |
| Acceptance gate | human visual + audible signoff |

The mainline shifted from "prove every scene" to **performance polish, stability, and content** at `v0.7.0-ps1`. Recent releases:

- `main` after `v0.8.7-ps1` — BUILDING2 low stores frames `71` and `77` as previous-frame D4 deltas, improving to `1349/1320`, overrun `29`, blocking `70`, prefetch overrun `0`, reads/due `52/15`; VISITOR3 high now stores frames `132` and `137` as D4 deltas and uses a high-tide slack4 guard, cutting blocking `52 -> 50` with loop/target flat at `1071/1040`; the latest WALKSTUF1 high current-control refresh records `1476/1434`, overrun `42`, blocking `81`, refill `23`. Public rollup remains `+0.3033%` over target / `99.7023%` target speed; WALKSTUF1 low, BUILDING2 high, VISITOR3 high, WALKSTUF1 high, and VISITOR3 low remain the top under-99 rows.
- `v0.8.7-ps1` — deterministic BOOTMODE scene selection, expected-scene gates in the headless perf harness, Suzy backdrop cleanup hardening, and heapless Scene Explorer thumbnail streaming. Public rollup remains `+0.3156%` over target / `99.6902%` target speed.
- `v0.8.6-ps1` — WALKSTUF1 low gap6-prefix + slack-guard promotion, WALKSTUF1 high window-prefetch / slack4 guard, and VISITOR3 high/low setup-segment resident copies for frames `131` / `128`. Public rollup `+0.3157%` over target / `99.6902%` target speed.
- `v0.8.5-ps1` — full 126-row headless performance matrix baseline.
- `v0.8.4-ps1` — on-PS1 captured thumbnails for every Scene Explorer slot; per-scene metadata reconciled against what the discs play. [Retrospective.](https://hunterdavis.com/johnny-castaway-ps1/lab/chapter-select-grind/)
- `v0.8.3-ps1` — WALKSTUF1 compact FGP3/v4 restore-minus-current packs.
- `v0.8.2-ps1` — VISITOR3 guarded-read group performance promotion.
- `v0.8.1-ps1` — clean-rect pressure-estimator stability fix surfaced by long randomized soak. [Retrospective.](https://hunterdavis.com/johnny-castaway-ps1/lab/v081-mary4-freeze/)
- `v0.8.0-ps1` — complete-scene performance baseline. [Retrospective.](https://hunterdavis.com/johnny-castaway-ps1/lab/from-87-to-99-5/)

Full release history at **[/releases/](https://hunterdavis.com/johnny-castaway-ps1/releases/)**. Per-release release notes live in [`docs/ps1/release-notes-*.md`](docs/ps1/).

## Quick start

**Prerequisites:** Docker (the build runs in `jc-reborn-ps1-dev:amd64` with PSn00bSDK 0.24 baked in), DuckStation, and the original Sierra data files (see [Original data files](#original-data-files) below).

```bash
./scripts/rebuild-and-let-run.sh noclean
```

Builds the PS1 EXE, packs `jcreborn.bin` / `jcreborn.cue` via `mkpsxiso`, launches DuckStation, and boots into `FISHING 1` via `BOOTMODE.TXT`. Default mode is the screensaver loop (each replay randomizes night / low-tide / raft / holiday). Add `noloop` to the boot string for a single-shot play.

A watchdog (`RUN_TIMEOUT_SECONDS`, default 300s) kills the emulator if it's left running; override with `RUN_TIMEOUT_SECONDS=<n>` or `0` to disable.

To bring up a new scene, see **[/docs/dev-workflow/](https://hunterdavis.com/johnny-castaway-ps1/docs/dev-workflow/)** ([raw](docs/ps1/development-workflow.md)).

## Original data files

The CD image build needs Sierra's original `Johnny Castaway` data files locally:

| File | Bytes | md5 |
|---|---:|---|
| `RESOURCE.MAP` | 1,461 | `8bb6c99e9129806b5089a39d24228a36` |
| `RESOURCE.001` | 1,175,645 | `374e6d05c5e0acd88fb5af748948c899` |

Drop them under `jc_resources/`. The repo tracks extracted VAGs and other derived artifacts; the master files are gitignored and must be present locally.

Optional — sound effects from [JCOS resources](https://github.com/nivs1978/Johnny-Castaway-Open-Source/tree/master/JCOS/Resources):

<details>
<summary><code>sound0..sound24.wav</code> expected hashes</summary>

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

## Method

The PS1 build is deliberately hybrid, not a from-scratch engine rewrite:

1. **Desktop host** runs Sierra's TTM/ADS interpreter and captures every visible foreground draw plus every `PLAY_SAMPLE` opcode into a per-frame JSON bundle.
2. A **pack compiler** turns that capture into PS1-native FG2 / FGP3 packs — high-tide and low-tide base-diff spans plus a per-frame sound-event table.
3. On **PS1**, [`foreground_pilot.c`](src/foreground_pilot.c) loads the pack and stamps captured frames in step with a narrow runtime that handles background, wave animation, holiday overlay, and SPU playback. SFX fire on cue with a 3-frame delay so sample key-on matches the visible trigger.

Full pipeline — pack format byte layout, hardware constraints hit on the way, the SPI pad-poll fix, dirty-rect bookkeeping — at **[/about/method/](https://hunterdavis.com/johnny-castaway-ps1/about/method/)**.

## Pause menu

<p align="center">
  <img src="docs/readme/scene-explorer.png" width="72%" alt="Scene Explorer running on PS1: top band reads SCENE EXPLORER, 5/63 * validated, FISHING 5 Eaten by a shark, Family Fishing, Frames 69; the captured-on-PS1 thumbnail of FISHING 5 (shark on the right side of the island chewing Johnny) sits in the middle; bottom band reads Pack FG/FISHING5.FG2, navigation hints LEFT/RIGHT scene, L1/R1 family, X play, Triangle loop, O back">
</p>

<p align="center">Pause → <strong>Scene Explorer</strong>: jump straight to any of the 63 scenes. Each entry shows the captured-on-PS1 thumbnail, family, frame count, and pack name. New in <code>v0.8.4-ps1</code>; full reference at <a href="https://hunterdavis.com/johnny-castaway-ps1/docs/pause-menu/#scene-explorer">/docs/pause-menu/#scene-explorer</a>.</p>

Press **Start** mid-scene. Twelve sub-screens reachable from the main pause overlay:

- **Scene Set** · **Scene Explorer** — pool selector across seven categories, plus the chapter-select grid above.
- **Freeplay: ON / OFF** · **Freeplay Options** — direct-control Johnny mode plus its gag, visitor, and controls catalogs.
- **World Options** — day/night, tide, raft stage, holidays, island position.
- **Accessibility** — captions, sound, ocean ambience, Sound Test.
- **System** — save to memcard, set time/date, set RNG seed, perf log, reset scene, next scene.

Auto-generated walkthrough with screenshots of every menu screen at **[/help/menu/](https://hunterdavis.com/johnny-castaway-ps1/help/menu/)**.

## Holidays

<p align="center">
  <img src="docs/ps1/holidays-emblems/holiday-emblems-preview.png" width="72%" alt="Added holiday emblem sprite sheet">
</p>

The original Sierra game shipped four baked-in holiday decorations (Christmas, New Year, Halloween, St. Patrick's). This port extends that to **36 US holidays** via a code-generated table and a pure-algorithm date core (Meeus for Easter, Nth-weekday math for the others) — no external date library, no expiring tables, works for 100+ years. Reference at **[/docs/holidays/](https://hunterdavis.com/johnny-castaway-ps1/docs/holidays/)**.

## Closed captions

<p align="center">
  <img src="docs/readme/fishing1-captions.png" width="62%" alt="FISHING 1 with closed captions enabled, showing a dark band at the bottom of the frame with subtitle text">
</p>

Pause → **Accessibility** → **Captions: ON**. A dark band appears at the bottom of the frame for ~5 seconds at scene start with a short descriptive subtitle. Off by default; the toggle persists to memcard. Glyph atlas is shared with the pause menu (one font upload, no duplication).

The corpus was authored fresh from scene content — see [`docs/ps1/caption-audit-2026-04-26.yaml`](docs/ps1/caption-audit-2026-04-26.yaml) for confidence ratings, and **[/docs/captions/#post-validation-runtime-corrections-v084-ps1](https://hunterdavis.com/johnny-castaway-ps1/docs/captions/#post-validation-runtime-corrections-v084-ps1)** for the v0.8.4 chapter-select-grind reconciliation (several scene-to-caption mappings drifted from the on-PS1 gags and have been corrected on the website's per-scene pages — the runtime `captionSceneMap[]` is open work).

Implementation: [`src/ps1_captions.c`](src/ps1_captions.c) / [`.h`](src/ps1_captions.h) (corpus + renderer), [`src/foreground_pilot.c`](src/foreground_pilot.c) (per-scene fire), [`src/graphics_ps1.c`](src/graphics_ps1.c) (`captionsRender()` inside `grUpdateDisplay`).

## Hardware target

| | |
|---|---|
| Main RAM | 2 MB |
| VRAM | 1 MB |
| SPU RAM | 512 KB (all 23 SFX VAGs preloaded at boot) |
| Output | 640 × 480 interlaced, NTSC |

Every rendering decision is forced by this budget. A full-frame video approach was ruled out early (614 KB per 640×480 16-bit frame × 63 scenes ≈ gigabytes); the hybrid-playback model sidesteps that by keeping foreground content in authored packs and a narrow runtime for background / waves / overlays.

Reference + the gotchas hit in practice at **[/docs/hardware/](https://hunterdavis.com/johnny-castaway-ps1/docs/hardware/)**.

## Controls

Normal screensaver mode needs no input. Press **Start** to open the pause menu.

| Control | Action |
|---|---|
| Start | Open pause menu / resume |
| D-pad / left analog | Move cursor or adjust values |
| Cross | Select / apply |
| Circle | Back from any menu or submenu |

**Freeplay** (launched from the pause menu) gives direct control of Johnny:

| Control | Action |
|---|---|
| D-pad / left analog | Walk Johnny; movement cancels the current action |
| L2 / R2 held | Slow / fast walk |
| Circle | Fish from the nearest side of the island |
| Select | Clear screen, cancel transient actions, rebuild island |
| R1 + Up / Down / Left / Right | Day/night · tide · raft stage · holiday |
| Start | Open pause menu |

## Documentation

The website is the rendered, cross-linked, prose-context view; the GitHub paths under `docs/ps1/` are the raw source. Pick whichever you prefer.

**Top-level surfaces**

- **[/play/](https://hunterdavis.com/johnny-castaway-ps1/play/)** — download + DuckStation quickstart + controls.
- **[/help/](https://hunterdavis.com/johnny-castaway-ps1/help/)** — auto-generated player help with menu screenshots.
- **[/scenes/](https://hunterdavis.com/johnny-castaway-ps1/scenes/)** — live ledger of all 63 scenes + per-scene case studies.
- **[/perf/](https://hunterdavis.com/johnny-castaway-ps1/perf/)** — 126-variant headless perf battle card.
- **[/about/](https://hunterdavis.com/johnny-castaway-ps1/about/)** — project overview ([method](https://hunterdavis.com/johnny-castaway-ps1/about/method/), [status](https://hunterdavis.com/johnny-castaway-ps1/about/status/), [history](https://hunterdavis.com/johnny-castaway-ps1/about/history/), [voice guide](https://hunterdavis.com/johnny-castaway-ps1/about/voice/), [dev environment](https://hunterdavis.com/johnny-castaway-ps1/about/dev-environment/)).
- **[/docs/](https://hunterdavis.com/johnny-castaway-ps1/docs/)** — eighteen reference manuals: build, captions, holidays, pause menu, freeplay, story-loop walks, regtest, scripted input, performance, hardware, audio, infrastructure, file formats, AI sub-agents, vision-classifier, the SDL2 → PSn00bSDK API mapping, dev workflow, glossary.
- **[/lab/](https://hunterdavis.com/johnny-castaway-ps1/lab/)** — sixteen feature-length retrospectives (LLM-assisted dev, hallucination control, build farm, regression practice, the soak-loop freeze, the chapter-select grind, the post-validation perf arc).
- **[/hack/](https://hunterdavis.com/johnny-castaway-ps1/hack/)** — for curious hackers: learning C from this codebase, porting Johnny to a new platform, the printf-driven perf loop, the visual-debug catalog.
- **[/archaeology/](https://hunterdavis.com/johnny-castaway-ps1/archaeology/)** — the full 5-chapter project story.
- **[/devlog/](https://hunterdavis.com/johnny-castaway-ps1/devlog/)** — dated worklogs preserved verbatim.
- **[/credits/](https://hunterdavis.com/johnny-castaway-ps1/credits/)** · **[/legal/](https://hunterdavis.com/johnny-castaway-ps1/legal/)** · **[/faq/](https://hunterdavis.com/johnny-castaway-ps1/faq/)**.

**Raw doc source** — [`docs/ps1/`](docs/ps1/) on GitHub. Notable entry points: [`scene-status.md`](docs/ps1/scene-status.md) (per-scene ledger), [`current-status.md`](docs/ps1/current-status.md) (narrative), [`development-workflow.md`](docs/ps1/development-workflow.md) (per-scene loop), [`performance-scene-matrix.csv`](docs/ps1/performance-scene-matrix.csv) + [`performance-experiment-log.md`](docs/ps1/performance-experiment-log.md) (perf source of truth), [`release-notes-*.md`](docs/ps1/) (per-release notes), [`research/`](docs/ps1/research/) (dated design logs ↔ `/devlog/`), [`archaeology/`](docs/ps1/archaeology/) (timeline, retired tools, vision-artifacts).

## Repo lineage

This project began as a branch of [jno6809/jc_reborn](https://github.com/jno6809/jc_reborn) focused on a PlayStation 1 port. It has diverged far enough — hybrid scene-playback pipeline, per-scene captures, FG2 / FGP3 pack format, PS1 SPU playback path, scene-by-scene validation ledger — that it now lives in its own repository. Without that engine decode this port wouldn't exist.

## Acknowledgements

Short list; the full version with context per name is at **[/credits/](https://hunterdavis.com/johnny-castaway-ps1/credits/)**.

- [jno6809/jc_reborn](https://github.com/jno6809/jc_reborn) — engine decode + the original Johnny Reborn project
- Hans Milling (`nivs1978`), [JCOS](https://github.com/nivs1978/Johnny-Castaway-Open-Source)
- Alexandre Fontoura (`xesf`), [Castaway](https://github.com/xesf/castaway)
- [Sierra Chest's Johnny Castaway archive](http://sierrachest.com/index.php?a=games&id=255&title=johnny-castaway)
- Jeff Tunnell · Kevin and Liam Ryan · Jaap · Gregori · Guido
- [PSn00bSDK](https://github.com/Lameguy64/PSn00bSDK) · [mkpsxiso](https://github.com/Lameguy64/mkpsxiso) · [DuckStation](https://github.com/stenzek/duckstation)
- BigSoundBank ("Sea: Waves" CC0) — ocean ambience source

## Transparency

Claude, Gemini, and OpenAI Codex were all used extensively across this project — for programming, debugging support, and generating prose for the website. Decisions and the merge bar are Hunter's; first drafts often were not. Full disclosure at **[/docs/agents/](https://hunterdavis.com/johnny-castaway-ps1/docs/agents/)** and on the footer of every page of the site.

## License

GPL-3.0, inherited from upstream `jc_reborn`.
