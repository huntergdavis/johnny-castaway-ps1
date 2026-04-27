# PS1 Port — Current Status

> 🌐 **Rendered version:** **[/about/status/](https://hunterdavis.com/johnny-castaway-ps1/about/status/)** — this doc rendered on the project website with cross-links and prose context. The GitHub copy here is the source.


**Last updated:** 2026-04-25 (release `v0.3.9-ps1`, commit `111efa9f`;
plus the 2026-04-25 batch — see [milestones-2026-04-25.md](milestones-2026-04-25.md)).

## Overall

The game boots on DuckStation, loads resources from CD, and runs scene
animations. `FISHING 1` and `FISHING 2` have been validated under the
project's current acceptance bar: pixel-perfect visuals plus synced SFX,
across every applicable variant (night / low-tide / holiday /
raft-stage), signed off by human visual + audible review. `FISHING 3`
is in bring-up: loop-stable and tide-correct on FG2, but not yet
promoted to the validated ledger.

| Component | Status |
|---|---|
| Build system (Docker + CMake + mkpsxiso) | Complete |
| CD-ROM I/O (`cdrom_ps1.c`) | Complete |
| Graphics layer (`graphics_ps1.c`) | Complete |
| Input layer (`events_ps1.c` + `spi.c`) | Complete — direct SPI driver replaces the broken BIOS pad path |
| Resource system (hashed + LRU) | Complete |
| Scene playback (fgpilot, `foreground_pilot.c`) | Primary render path; 2/63 scenes fully validated |
| Audio layer (`sound_ps1.c`) | Working — VAG preload at boot + round-robin SPU voices + captured SFX replay; mute via direct SPU register writes (`SpuSetCommonMasterVolume` is not honored by DuckStation HLE) |
| Telemetry / debug overlay | Complete |
| Perf instrumentation (`ps1_perf.c`) | Complete — level-gated `JCPERF`/`JCPERF2` TTY lines (OFF/SUMMARY/DETAIL/DEBUG via `ps1PerfSetLevel`) |
| Pause menu (`pause_menu.c`) | Complete — Start opens overlay; custom 8x8 font (FntFlush is empirically broken in scene-runtime context); POLY_F4 dim + panel quads |
| User settings persistence (`memcard.c`) | In progress — pause-menu choices save to `bu00:` |
| TTY printf | Reliable on PSn00bSDK + DuckStation as of 2026-04-25 |

## Scenes: 2 / 63 fully validated

The per-scene ledger lives in [scene-status.md](scene-status.md). That
file is the source of truth for what is complete under the current bar;
this page gives the narrative around it.

Milestone releases:
- `v0.3.9-ps1` (commit `111efa9f`) — fishing3 overnight loop-stability
  release; confirms the current runtime can run long sessions without
  the previous scene-to-scene leak.
- `v0.3.6-ps1` (commit `f2737253`) — fishing1 pixel-perfect with full SFX
  across all variants.
- Prior visual-only release `v0.3.5-ps1` (commit `9448d49f`) —
  superseded by the full-SFX release above.

Milestone release cadence from here: cut every 10 scenes that reach the
same validated bar. Smaller stability releases may happen between scene
milestones.

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
| **2 / 63** | **2026-04-24** | **Current scene ledger after `FISHING 2` promotion; `FISHING 3` remains bring-up** | **this doc, `scene-status.md`** |

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
- Scene coverage beyond FISHING 1 and FISHING 2 is pending scene-by-scene bring-up
  via the loop in [development-workflow.md](development-workflow.md).

## See also

- [milestones-2026-04-25.md](milestones-2026-04-25.md) — TTY,
  perf module, pause menu, holiday expansion, SPI driver, memcard
- [scene-status.md](scene-status.md) — per-scene ledger
- [pause-menu-design.md](pause-menu-design.md) — locked design
- [holidays-expansion-design.md](holidays-expansion-design.md) — 35-holiday plan
- [performance-optimization-plan.md](performance-optimization-plan.md) — perf backlog
- [development-workflow.md](development-workflow.md) — bring-up loop
- [TESTING.md](TESTING.md) — validation strategy
- [hardware-specs.md](hardware-specs.md)
- [project-history.md](project-history.md) — development narrative
- [research/README.md](research/README.md) — design logs (historical)
- [ps1-branch-cleanup-plan.yaml](ps1-branch-cleanup-plan.yaml) — in-flight cleanup contract
