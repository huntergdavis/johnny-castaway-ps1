---
layout: page
title: Status
eyebrow: Component-level state after v0.8.9-ps1
subtitle: What's working, what's broken, what's in motion -- one row per subsystem.
description: Component-level status of the Johnny Castaway PS1 port — renderer, audio, input, captions, holidays, pause menu, memcard, regtest, host capture, CD packaging.
---

## Headline

Current release is **`{{ site.release.tag }}`**. Scenes validated
under the project's acceptance bar (pixel-perfect visuals plus synced
SFX, signed off across every applicable variant -- night, low-tide,
holiday, raft-stage):
**{{ site.release.scenes_validated }} / {{ site.release.scenes_total }}**.
`v0.8.9-ps1` is the current release: every row in the live per-scene
ledger is signed off, all 126 high/low scene variants are routed and
timing-bearing, deterministic BOOTMODE scene selection is logged and gated,
VISITOR5 high/low now share green `30..46` retained-read baselines, and the
latest WALKSTUF1 low in-place payload reductions are promoted. The
chapter-select grid in the in-game
[Scene Explorer]({{ '/docs/glossary/#scene-explorer' | relative_url }})
keeps the custom on-PS1-captured thumbnails for every one of the 63 scenes
from `v0.8.4-ps1` while streaming previews without a large paused-menu heap
allocation. The public headless battle card is `+0.2708%` over
target / `99.7337%` target speed; the raw signed optimization matrix is
`-0.4963%` / `100.5160%`.
The newest accepted BUILDING2 low pack promotion trims dead draw-tail payload
without changing pack size/LBA or the PS-EXE bucket, improving low to
`1339/1317`, overrun `22`, blocking/read time `53`/`150`, reads `37`, and due
`12`. BUILDING4 low remains on the accepted offscreen PAL4 draw-span clip at
`2853/2816`, overrun `37`, blocking/read time `40`/`215`, and prefetch overrun
`34`; v746 now shrinks frame `291` in-place, preserving offsets while reducing
active payload `855284 -> 849109` with exact-flat timing. The newest WALKSTUF1 baselines are same-speed late-tail work-volume
clips: high v654 clips frames `194..210` and keeps `1476/1434`,
blocking/refill `81/23`, reads/due `65/16`; low v653 clips frames `202..210`
and keeps `1478/1431`, blocking/refill `64/20`, with v705 plus v760 bringing
    loop reads/read time to `60/272` and due misses to `11`, and v747/v749/v750/v751/v753/v755/v756/v757/v759/v762/v763/v766/v767/v769/v770/v771/v772/v773/v774/v775/v776/v777
    also shrinking low frames `51`/`49`/`47`/`61`/`62`/`58`/`45`/`37`/`35`/`43`/`41`/`57`/`33`/`67`/`68`/`69`/`32`/`133`/`5`/`141`/`70`/`30` in-place to `815013` active payload after the release merge. Both drop
draw pixels/spans/rows without changing timing/CD counters. VISITOR3 high keeps the compact tail-pack promotion at `1063/1040`, and
VISITOR5 high/low both remain green on the matching `30..46` retained-read
shape. The VISITOR3 low frame132/frame137 setup-prime relocation keeps the
fixed `1555450` byte VIST3LOW footprint, LBA/sectors `23371/760`, and
`217088` byte PS-EXE bucket while improving low to `1062/1040`, overrun `22`,
blocking `42`, reads/due `7/7`. WALKSTUF1 high/low now measure `1476/1434`
and `1478/1431`; the latest WALKSTUF1 scalar and post-prepare scheduler
closures kept that timing unchanged and moved the next attempt toward
generated ownership or pack-side work. BUILDING2 low now adds the accepted
`218..229` slack-8 retained-read row plus v739 draw-tail trimming, improving to
`1339/1317`, overrun `22`, blocking/refill `53/0`, reads/read time `37/150`,
and due `12`. BUILDING2 high currently measures
`1351/1311`, overrun `40`, blocking `54`, hidden refill `18`, and due misses
`7`; same-speed offscreen clips now reduce its runtime rows/spans/pixels to
`18030/105645/446246`.
The live ledger is at
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
| Build system (Docker + CMake + mkpsxiso) | Complete | `config/ps1/Dockerfile.ps1` on `linux/amd64`; PSn00bSDK 0.24; `scripts/rebuild-and-let-run.sh` is the one-command entry. |
| CD-ROM I/O (`cdrom_ps1.c`, ~2,570 lines) | Complete | Reimplements `fopen`/`fread` against CD sectors. `CdSearchFile` / `CdRead` / `CdSync` integration. The rest of `resource.c` is unchanged from upstream. |
| Renderer (`graphics_ps1.c`, ~4,900+ lines) | Complete | 4-bit indexed sprite format (`indexedPixels`), palette LUT compositing, 4-pixel unrolled inner loop, opaque sprite fast-path, dirty-rect background restore (~80-95% reduction in per-frame data movement), black-backdrop temporal cleanup for full-screen scenes like `JOHNNY 1`, and `grForceFullRedrawNextFrame` for pause-menu resume. `FntFlush` is empirically broken in the scene-runtime context -- do not regress on-screen text to it. |
| Audio (`sound_ps1.c`, ~445 lines) + SPU | Working | All 23 VAG SFX preloaded into SPU RAM at boot. Round-robin over 8 voices. Captured `0xC051 PLAY_SAMPLE` events ship in the FG2 pack and fire from `foreground_pilot.c` with a 3-frame key-on delay. Mute writes the SPU master-volume registers directly because `SpuSetCommonMasterVolume` is not honored by DuckStation HLE. The VAG encoder (`scripts/wav2vag.py`) was extensively debugged during the `v0.3.6-ps1` milestone (commit `355227fa`); see that commit for the full bug list. |
| Input (`events_ps1.c` + `src/spi.c`) | Complete | Direct SPI driver, timer-2 + SIO0 IRQ at 250 Hz, lifted from spicyjpeg's `pads` example. The BIOS pad path (`InitPAD`/`StartPAD`) is unusable in PSn00bSDK 0.24 + DuckStation. Poll TX is `tx_len=5`, not 4 -- DuckStation only delivers button bytes when the full 5-byte sequence comes from the TX buffer. |
| Closed captions (`src/ps1_captions.{c,h}`) | Working | On/off via Pause -> Accessibility -> Captions. Dark band at the bottom of the frame for ~5 seconds at scene start with descriptive subtitle text. Glyph atlas shared with the pause menu. Caption corpus from the upstream `closed_captions` branch of `jc_reborn`; the original sequential ADS-tag map had ~20 mismatches and was re-audited (`docs/ps1/caption-audit-2026-04-26.yaml`). HIGH-confidence matches dominate; LOW-confidence slots remain on STAND idles and a few VISITOR / WALKSTUF edges. |
| Holidays (36 of them, code-generated) | Working | Holiday emblem sprite sheet packed into the PS1 holiday overlay. Selectable via Pause -> World Options -> Holidays and `BOOTMODE.TXT`. Generation is offline; design notes in `docs/ps1/holidays-expansion-design.md`. |
| [Story-loop walks]({{ '/docs/walks/' | relative_url }}) (`walk_pilot.c`, `walk_render.c`) | Working | Johnny walks between scene endpoints using Sierra's original `walk_data.h` routes instead of teleporting. The runtime restores from a persistent tight clean buffer, keeps waves moving, re-stamps holiday overlays behind Johnny during walk frames, and covers Johnny behind the palm trunk/leaves. `v0.7.2` adds a backdrop-key guard so walks only run when the next scene matches the previous rendered tide, raft, night, holiday, and island X/Y state; `v0.8.0` adds a clean-rect retry path; `v0.8.1` counts wave-band/split-rect clean pressure before allocation. |
| [Freeplay/debug mode]({{ '/docs/glossary/#freeplay' | relative_url }}) (`scene_freeplay.c`) | Working | `v0.5.0-ps1` promotes direct-control Johnny: D-pad/analog walking, L2/R2 speed modifiers, fishing, immediate R1+D-pad world toggles, gag/visitor debug catalogs, sound test, Select clear-screen rebuild, [frog-clock]({{ '/docs/glossary/#frog-clock' | relative_url }}) loading transitions, and a no-allocation steady-state frame loop. |
| Pause menu (`pause_menu.c`) | Complete | Start opens the overlay mid-scene; custom embedded 8x8 ASCII font (because `FntFlush` is broken in scene context); `POLY_F4` dim quad + panel quads. Twelve sub-screens: Scene Set, Scene Explorer, Freeplay Options, Controls, World Options, Holidays, Set Island Position, Accessibility, Sound Test, System, Set Time/Date, Set RNG Seed. |
| [Memcard]({{ '/docs/glossary/#memcard' | relative_url }}) persistence (`memcard.c`) | Working / expanding | Pause-menu choices persist to `bu00:` block 0. Save/load wired; restore-on-boot wired. v6 saves persist holiday mode separately from manual holiday id. Broader menu-option persistence remains future work. |
| [Telemetry / debug overlay]({{ '/docs/glossary/#telemetry-overlay' | relative_url }}) | Complete | Five-panel left-edge overlay; gated by `grPs1TelemetryEnabled` in `graphics_ps1.c` with row writes scattered through `src/ads.c` and helpers like `grPs1SetLastBmpTelemetry`. Off by default in release builds. Decoded out of captured frames by `scripts/decode-ps1-bars.py` into `telemetry.json` during [regtest]({{ '/docs/regtest/' | relative_url }}). The historical visual-debug substrate when TTY printf was unsafe; still the right tool for per-frame state since text I/O can perturb timing. |
| Perf instrumentation (`ps1_perf.c`) | Complete | Level-gated `JCPERF` / `JCPERF2` TTY lines. Levels: `OFF`, `SUMMARY`, `DETAIL`, `DEBUG`. Set via `ps1PerfSetLevel`. Off in normal operation; the user feeds `JCPERF` output to a perf-debug agent when chasing regressions. |
| TTY printf | Reliable | Restored 2026-04-25 on PSn00bSDK 0.24 + DuckStation through bounded `vprintf` plus DuckStation TTY/file logging. Gated BOOTMODE probes (`printf-test`, `logtest`). Must not be called per-frame -- text I/O alters timing. |
| Regtest harness | Working | `config/ps1/regtest-scenes.txt` + `scripts/run-regtest.sh` drive a headless DuckStation pass. Source of truth for "boots and renders something." Not the source of truth for "looks right" -- that's still human signoff. |
| Host capture pipeline | Working | `scripts/capture-host-scene.sh` runs the desktop build under controlled boot state; emits high/low PNG frames, `frame-meta.json`, `sound-events.jsonl`. `scripts/export-scene-foreground-pilot.sh` now defaults new scene bring-up to normal/far-left/far-right foreground-only multi-view stitching before compiling FG2. |
| CD packaging (mkpsxiso) | Complete | `config/ps1/cd_layout.xml`. Routed scenes contribute high-tide + low-tide pack entries; the full FG2 corpus is ~343&nbsp;MB, of which a selected subset rides on the disc. Build outputs `jcreborn.bin` + `jcreborn.cue`. CD image at `{{ site.release.tag }}` is ~79&nbsp;MB; PS-EXE is ~208&nbsp;KiB after legacy ADS/TTM/FG1 paths were stripped from the linker pass. |

## What's currently broken

Honest list, narrowed to specifics:

- **Post-validation regressions.** Scene coverage is complete at 63/63, but
  future bugfixes can still be needed when broader play, new features, or
  performance changes disturb a signed-off path. `v0.8.1` fixed the latest
  random-run clean-rect pressure freeze; the scene ledger remains the
  acceptance source after each fix.
- **Memcard fresh-card edge cases.** Save / load works on a card
  that already holds the project's block. Behavior on a freshly
  formatted card or a card with a corrupted block is still being
  shaken out.
- **`SpuSetCommonMasterVolume` HLE divergence.** Worked around with
  direct register writes; the divergence itself is a DuckStation
  HLE quirk, not a project bug, but it does mean audio-on-real-
  hardware needs verification before any release calls itself
  hardware-confirmed.
- **Real hardware soak.** Smoke-tested on an SCPH-7501 via the
  [TonyHax](https://github.com/socram8888/tonyhax) softmod path —
  the disc boots and runs. Long-term observations on real
  hardware (extended idle, real SPU register behavior, real CD
  seek timing under random-position scene picks) are still on
  the wishlist. The `SpuSetCommonMasterVolume` HLE divergence
  above is a concrete reason to keep treating hardware audio as
  unverified until the soak loop covers it.
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

- **Optimization after full validation.** With all 63 scenes signed off, the
  next focus is preserving pixel-perfect playback while improving speed,
  loading, memory pressure, and release polish. `{{ site.release.tag }}`
  has the public-capped headless baseline at `{{ site.release.perf_target_speed_pct }}%`
  target speed across all 126 timing-bearing rows after the VISITOR3,
  WALKSTUF1, MARY3, BUILDING, ACTIVITY9, JOHNNY1, and WALKSTUF3
  promotion arc. The bring-up loop remains in
  [`docs/ps1/development-workflow.md`](https://github.com/{{ site.repo }}/blob/main/docs/ps1/development-workflow.md).
- **Scene-by-scene FG2 routing.** All 63 scenes have generated
  high-tide and low-tide FG2 packs sitting in the corpus; routing
  them onto the CD image is selective during bring-up so the
  image stays manageable.
- **Memcard hardening.** Closing the fresh-card and corrupted-
  block edge cases.
- **Persisted-options continuation.** `v0.7.1` shipped the holiday-mode slice:
  `AUTO DATE:ORIG4` is now the fresh/no-card default and memory-card schema v6
  stores automatic policy separately from manual holiday id. Captions, Scene
  Set, tide, raft, island-position, freeplay/perf preferences, and menu cursor
  defaults remain future work.
- **Caption audit follow-through.** Two layers, with one done in
  `v0.8.4-ps1` and one still open. *Done:* the website's *description*
  of which caption belongs to which scene was reconciled against the
  on-PS1 packs during the
  [chapter-select grind]({{ '/lab/chapter-select-grind/' | relative_url }})
  — per-scene `index.md` titles + bodies, `scenes.yml` notes,
  `scene-status.md` Notes column, and the in-game
  [Scene Explorer]({{ '/docs/glossary/#scene-explorer' | relative_url }})
  display strings. The runtime evidence overturned a substantial
  fraction of the audit's confidence ratings, including two HIGH-rated
  FISHING entries (`FISHING 2` boot, `FISHING 3` crab) that were exact
  text-pack mismatches and the `VISITOR 5` `NO_MATCH` candidate that
  turned out to be a clear unambiguous gag. *Open:* the runtime
  `captionSceneMap[]` array in
  [`src/ps1_captions.c`]({{ site.github_url }}/blob/main/src/ps1_captions.c)
  was not changed in `v0.8.4-ps1`; the on-screen caption band
  continues to play whatever caption the original audit picked, even
  on rows where the website now disagrees with that pick. Repointing
  the runtime mapping (or blanking the rows where text and pack can't
  be reconciled) is a future-release item. The
  [post-validation runtime corrections section]({{ '/docs/captions/#post-validation-runtime-corrections-v084-ps1' | relative_url }})
  lists the named mismaps.
- **`fgpilot` -> "PS1 scene playback" naming migration.** The
  internal code-name in `foreground_pilot.c` and friends remains
  `fgpilot`; the public-facing name is moving to "PS1 scene
  playback." The migration plan lives in
  `docs/ps1/ps1-branch-cleanup-plan.yaml` under
  `fgpilot_naming_migration_plan`.
- **Milestone release cadence.** `v0.8.9-ps1` promotes the current mainline
  optimization set: VISITOR5 low reuses the green `30..46` retained-read shape,
  BUILDING2 low carries the `218..229` slack8 row plus v739 draw-tail trim,
  and WALKSTUF1 low carries no-shift frame `51`/`49`/`47`/`61`/`62`/`58`/`45`/`37`/`35`
  in-place payload reductions. `v0.8.8-ps1` promotes the VISITOR5 high
  `30..46` retained-read group on top of the v0.8.7 stability hardening.
  `v0.8.7-ps1` is the deterministic BOOTMODE
  scene selection and Scene Explorer preview stability point release on top of
  the v0.8.6 performance follow-through: direct-scene boots are logged and
  gated, Suzy backdrop cleanup restores scene state, and Scene Explorer
  thumbnails stream without a large heap allocation. `v0.8.6-ps1` is the
  WALKSTUF1 / VISITOR3 setup-segment compaction follow-through point release
  on top of v0.8.5's full 126-row baseline (WALKSTUF1 low gap6-prefix + slack-guard,
  WALKSTUF1 high window-prefetch / slack4 guard, VISITOR3 high/low
  setup-segment resident copies for frames `131` / `128`). `v0.8.5-ps1`
  is the full 126-row headless performance matrix release.
  `v0.8.4-ps1` is the chapter-select
  thumbnails content release — 63 custom on-PS1-captured grid slots and
  a scene-page reconciliation pass against the actual on-PS1 packs.
  `v0.8.3-ps1` is the WALKSTUF1 compact foreground performance pass.
  `v0.8.2-ps1` is the VISITOR3 guarded-read performance pass.
  `v0.8.1-ps1` is the clean-rect pressure stability point release. `v0.8.0-ps1` is the complete-scene performance
  baseline. `v0.7.2-ps1`
  fixes stale-backdrop story-loop walks. `v0.7.1-ps1` adds the original-four
  auto holiday default and persisted holiday mode. `v0.7.0-ps1` validates all
  63 scenes and
  moves the project into bugfix/optimization mode. `v0.6.10-ps1` validates `MARY 5`
  after the `NORAFT` raft clamp and `FIRST` full-wipe transition fix.
  `v0.6.8-ps1` validates `MARY 2` after the wide multi-view stitch and fish thought-bubble repair.
  `v0.6.6-ps1` removes the old
  FISHING7/FISHING8 runtime pins after far-left recapture proved both
  packs are random-position safe. `v0.5.0-ps1` is the freeplay/debug
  release. `v0.4.20-ps1` was the walking-loop stability release.
  `v0.3.9-ps1` was the fishing3-loop-stability release; `v0.3.6-ps1`
  was fishing1 with full SFX across all variants.

For dated context on any of the above, see
[/devlog/]({{ '/devlog/' | relative_url }}).

## Related pages

- [About: Method]({{ '/about/method/' | relative_url }}) —
  the canonical methodology essay this status page tracks
  the implementation of.
- [About: History]({{ '/about/history/' | relative_url }}) —
  how the components above got to "complete" or "in motion."
- [About: dev environment]({{ '/about/dev-environment/' | relative_url }})
  — the workflow photograph behind every row in the
  component-level table.
- [Scene ledger]({{ '/scenes/' | relative_url }}) — the visual
  signoff bar; the per-scene status the table summarizes.
- [Performance battle card]({{ '/perf/' | relative_url }}) —
  the second ledger; per-scene/tide timing for every
  component listed under "performance" above.
- [Releases]({{ '/releases/' | relative_url }}) — short
  notes on every tagged version named in the milestone-cadence
  list above.
- [Glossary]({{ '/docs/glossary/' | relative_url }}) —
  vocabulary anchor for `FntFlush`, `clean-rect`,
  `fgpilot`, `BOOTMODE.TXT`, and the rest of the terms
  this page uses without scaffolding.
