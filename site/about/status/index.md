---
layout: page
title: Status
eyebrow: Component-level state at v0.6.6-ps1
subtitle: What's working, what's broken, what's in motion -- one row per subsystem.
description: Component-level status of the Johnny Castaway PS1 port — renderer, audio, input, captions, holidays, pause menu, memcard, regtest, host capture, CD packaging.
---

## Headline

Current release is **`{{ site.release.tag }}`**. Scenes validated
under the project's acceptance bar (pixel-perfect visuals plus synced
SFX, signed off across every applicable variant -- night, low-tide,
holiday, raft-stage):
**{{ site.release.scenes_validated }} / {{ site.release.scenes_total }}**.
The signed-off scenes are `FISHING 1`, `FISHING 2`, `FISHING 3`,
`FISHING 4`, `FISHING 5`, `FISHING 6`, `FISHING 7`, `FISHING 8`,
`JOHNNY 1`, `JOHNNY 2`, `JOHNNY 3`, and `JOHNNY 4`. The live per-scene ledger is at
[/scenes/]({{ '/scenes/' | relative_url }}); the per-scene workflow
that drives the bar is in
[`docs/ps1/scene-status.md`](https://github.com/{{ site.repo }}/blob/main/docs/ps1/scene-status.md).

The narrative below is per-component, not per-scene. The regtest
harness (`config/ps1/regtest-scenes.txt` + `scripts/run-regtest.sh`)
is the source of truth for "does this still build and boot." Human
visual + audible signoff is the source of truth for "is this scene
done."

## Per-component status

| Component | Status | Notes |
|---|---|---|
| Build system (Docker + CMake + mkpsxiso) | Complete | `Dockerfile.ps1` on `linux/amd64`; PSn00bSDK 0.24; `scripts/rebuild-and-let-run.sh` is the one-command entry. |
| CD-ROM I/O (`cdrom_ps1.c`, ~2,280 lines) | Complete | Reimplements `fopen`/`fread` against CD sectors. `CdSearchFile` / `CdRead` / `CdSync` integration. The rest of `resource.c` is unchanged from upstream. |
| Renderer (`graphics_ps1.c`, ~3,300+ lines) | Complete | 4-bit indexed sprite format (`indexedPixels`), palette LUT compositing, 4-pixel unrolled inner loop, opaque sprite fast-path, dirty-rect background restore (~80-95% reduction in per-frame data movement), black-backdrop temporal cleanup for full-screen scenes like `JOHNNY 1`, and `grForceFullRedrawNextFrame` for pause-menu resume. `FntFlush` is empirically broken in the scene-runtime context -- do not regress on-screen text to it. |
| Audio (`sound_ps1.c`, ~184 lines) + SPU | Working | All 23 VAG SFX preloaded into SPU RAM at boot. Round-robin over 8 voices. Captured `0xC051 PLAY_SAMPLE` events ship in the FG2 pack and fire from `foreground_pilot.c` with a 3-frame key-on delay. Mute writes the SPU master-volume registers directly because `SpuSetCommonMasterVolume` is not honored by DuckStation HLE. The VAG encoder (`scripts/wav2vag.py`) was extensively debugged during the `v0.3.6-ps1` milestone (commit `355227fa`); see that commit for the full bug list. |
| Input (`events_ps1.c` + `src/spi.c`) | Complete | Direct SPI driver, timer-2 + SIO0 IRQ at 250 Hz, lifted from spicyjpeg's `pads` example. The BIOS pad path (`InitPAD`/`StartPAD`) is unusable in PSn00bSDK 0.24 + DuckStation. Poll TX is `tx_len=5`, not 4 -- DuckStation only delivers button bytes when the full 5-byte sequence comes from the TX buffer. |
| Closed captions (`src/ps1_captions.{c,h}`) | Working | On/off via Pause -> Accessibility -> Captions. Dark band at the bottom of the frame for ~5 seconds at scene start with descriptive subtitle text. Glyph atlas shared with the pause menu. Caption corpus from the upstream `closed_captions` branch of `jc_reborn`; the original sequential ADS-tag map had ~20 mismatches and was re-audited (`docs/ps1/caption-audit-2026-04-26.yaml`). HIGH-confidence matches dominate; LOW-confidence slots remain on STAND idles and a few VISITOR / WALKSTUF edges. |
| Holidays (36 of them, code-generated) | Working | Holiday emblem sprite sheet packed into the PS1 holiday overlay. Selectable via Pause -> World Options -> Holidays and `BOOTMODE.TXT`. Generation is offline; design notes in `docs/ps1/holidays-expansion-design.md`. |
| Story-loop walks (`walk_pilot.c`, `walk_render.c`) | Working | Johnny walks between scene endpoints using Sierra's original `walk_data.h` routes instead of teleporting. The runtime restores from a persistent tight clean buffer, keeps waves moving, re-stamps holiday overlays, and covers Johnny behind the palm trunk/leaves. `v0.4.20-ps1` soak evidence: about 599 seconds in DuckStation, no `JCBSOD`, no `JCWALK` allocation failures. |
| Freeplay/debug mode (`scene_freeplay.c`) | Working | `v0.5.0-ps1` promotes direct-control Johnny: D-pad/analog walking, L2/R2 speed modifiers, fishing, immediate R1+D-pad world toggles, gag/visitor debug catalogs, sound test, Select clear-screen rebuild, frog-clock loading transitions, and a no-allocation steady-state frame loop. |
| Pause menu (`pause_menu.c`) | Complete | Start opens the overlay mid-scene; custom embedded 8x8 ASCII font (because `FntFlush` is broken in scene context); `POLY_F4` dim quad + panel quads. Compact sub-screens: Freeplay Options, World Options, Accessibility, Sound Test, System, and the date/island/seed editors. |
| Memcard persistence (`memcard.c`) | In progress | Pause-menu choices persist to `bu00:` block 0. Save/load wired; restore-on-boot wired. Edge cases on a fresh / formatted memcard are still under iteration. |
| Telemetry / debug overlay | Complete | Five-panel overlay; gated by `debugMode`. Toggle with Select. The historical visual-debug substrate when TTY printf was unsafe; still the right tool for per-frame state. |
| Perf instrumentation (`ps1_perf.c`) | Complete | Level-gated `JCPERF` / `JCPERF2` TTY lines. Levels: `OFF`, `SUMMARY`, `DETAIL`, `DEBUG`. Set via `ps1PerfSetLevel`. Off in normal operation; the user feeds `JCPERF` output to a perf-debug agent when chasing regressions. |
| TTY printf | Reliable | Restored 2026-04-25 on PSn00bSDK 0.24 + DuckStation through bounded `vprintf` plus DuckStation TTY/file logging. Gated BOOTMODE probes (`printf-test`, `logtest`). Must not be called per-frame -- text I/O alters timing. |
| Regtest harness | Working | `config/ps1/regtest-scenes.txt` + `scripts/run-regtest.sh` drive a headless DuckStation pass. Source of truth for "boots and renders something." Not the source of truth for "looks right" -- that's still human signoff. |
| Host capture pipeline | Working | `scripts/capture-host-scene.sh` runs the desktop build under controlled boot state; emits high/low PNG frames, `frame-meta.json`, `sound-events.jsonl`. `scripts/export-scene-foreground-pilot.sh` then drives `scripts/build-scene-foreground-pack.py` to compile the FG2. |
| CD packaging (mkpsxiso) | Complete | `config/ps1/cd_layout.xml`. Routed scenes contribute high-tide + low-tide pack entries; routing is selective during bring-up because the full FG2 corpus is ~343&nbsp;MB. Build outputs `jcreborn.bin` + `jcreborn.cue`. CD image at the current routed set is ~9.9&nbsp;MB; PS-EXE is ~84&nbsp;KB after legacy ADS/TTM/FG1 paths were stripped. |

## What's currently broken

Honest list, narrowed to specifics:

- **Scene coverage past the signed-off set.** The other 51 scenes in
  [`scene-status.md`](https://github.com/{{ site.repo }}/blob/main/docs/ps1/scene-status.md)
  are unverified under the current bar; some have older bring-up
  notes from the harness era that no longer count as current
  evidence.
- **`MARY 1`, `MARY 2`, `MARY 4`** are untested in regtest. Those rows in
  `scene-status.md` are the SoT.
- **Memcard fresh-card edge cases.** Save / load works on a card
  that already holds the project's block. Behavior on a freshly
  formatted card or a card with a corrupted block is still being
  shaken out.
- **`SpuSetCommonMasterVolume` HLE divergence.** Worked around with
  direct register writes; the divergence itself is a DuckStation
  HLE quirk, not a project bug, but it does mean audio-on-real-
  hardware needs verification before any release calls itself
  hardware-confirmed.
- **Real hardware testing.** All current validation is on
  DuckStation. A real-PSone run is on the long-term list. Until
  then, treat hardware claims with the appropriate skepticism.
- **Older count-based status models** (`25/63`, `27/63`, `60/63`,
  `63/63`) from prior validation eras are *retired*, not current.
  They are preserved in
  [`current-status.md`](https://github.com/{{ site.repo }}/blob/main/docs/ps1/current-status.md)
  for searchability and demoted explicitly. Do not cite them as
  current progress.

If a specific bug isn't called out here, it lives either in the
GitHub issue tracker on
[{{ site.repo }}]({{ site.github_url }}) or in the dated worklogs
under [/devlog/]({{ '/devlog/' | relative_url }}). The regtest
harness output is the SoT for build-and-boot regressions.

## What's currently in motion

Pulled from the live narrative in
[`docs/ps1/current-status.md`](https://github.com/{{ site.repo }}/blob/main/docs/ps1/current-status.md):

- **`JOHNNY 5` bring-up.** Next focus is continuing the signed
  scene-by-scene ledger after the FISHING7/FISHING8 recapture
  revalidation. The bring-up loop is in
  [`docs/ps1/development-workflow.md`](https://github.com/{{ site.repo }}/blob/main/docs/ps1/development-workflow.md).
- **Scene-by-scene FG2 routing.** All 63 scenes have generated
  high-tide and low-tide FG2 packs sitting in the corpus; routing
  them onto the CD image is selective during bring-up so the
  image stays manageable.
- **Memcard hardening.** Closing the fresh-card and corrupted-
  block edge cases.
- **Caption audit follow-through.** HIGH-confidence ADS-tag
  mappings are in; LOW-confidence STAND/VISITOR/WALKSTUF slots
  will be refined as those scenes pass the bar.
- **`fgpilot` -> "PS1 scene playback" naming migration.** The
  internal code-name in `foreground_pilot.c` and friends remains
  `fgpilot`; the public-facing name is moving to "PS1 scene
  playback." The migration plan lives in
  `docs/ps1/ps1-branch-cleanup-plan.yaml` under
  `fgpilot_naming_migration_plan`.
- **Milestone release cadence.** `v0.6.6-ps1` removes the old
  FISHING7/FISHING8 runtime pins after far-left recapture proved both
  packs are random-position safe. `v0.5.0-ps1` is the freeplay/debug
  release. `v0.4.20-ps1` was the walking-loop stability release.
  `v0.3.9-ps1` was the fishing3-loop-stability release; `v0.3.6-ps1`
  was fishing1 with full SFX across all variants.

For dated context on any of the above, see
[/devlog/]({{ '/devlog/' | relative_url }}).
