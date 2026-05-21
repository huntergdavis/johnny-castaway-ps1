---
layout: page
title: Status
eyebrow: Component-level state after v0.8.16-ps1
subtitle: What's working, what's broken, what's in motion -- one row per subsystem.
description: Component-level status of the Johnny Castaway PS1 port — renderer, audio, input, captions, holidays, pause menu, memcard, regtest, host capture, CD packaging.
---

## Headline

Current release is **`{{ site.release.tag }}`**. Scenes validated
under the project's acceptance bar (pixel-perfect visuals plus synced
SFX, signed off across every applicable variant -- night, low-tide,
holiday, raft-stage):
**{{ site.release.scenes_validated }} / {{ site.release.scenes_total }}**.
The current release line keeps every row in the live per-scene ledger signed
off, all 126 high/low scene variants routed and timing-bearing, deterministic
BOOTMODE scene selection logged and gated, and the new memory-region allocator
promoted into mainline. BOOT allocations now seal after startup, CACHE
allocations reuse long-lived resource storage through free-list/LRU paths, and
TRANSIENT scene allocations can be wiped between major scene loads. The
chapter-select grid in the in-game
[Scene Explorer]({{ '/docs/glossary/#scene-explorer' | relative_url }})
keeps the custom on-PS1-captured thumbnails for every one of the 63 scenes
from `v0.8.4-ps1` while streaming previews without a large paused-menu heap
allocation. The public headless battle card is `+0.2480%` over
target / `99.7557%` target speed; the raw signed optimization matrix is
about `-0.4689%` / `100.4860%`. The allocator validation branch records a R34
full matrix of `126/126` PASS with 0 BSODs, and the latest targeted W1/B2 plus
VISITOR3 clean-relief/setup-edge checkpoints keep the top allocator-era rows
measured inside the allocator budget. The latest source-headroom pass caches
the active foreground scene ID in hot scheduler paths, keeps the five-yellow
canary exact-flat, and shrinks tracked foreground scheduler symbols while the
PS-EXE bucket stays fixed. The latest BUILDING2-high no-decode trim-draw-tail
promotion improves B2-high to `1341/1313` while preserving file size and pack
LBA. The prior VISITOR3-low preserve-entry-size screen-clip promotion keeps
VISITOR3-low at `1071/1039` while preserving entry sizes and pack LBA. The
latest W1-low preserve-entry-size screen-clip
promotion keeps W1-low at `1470/1446` while improving hidden refill `4 -> 3`
and preserving entry sizes and pack LBA. The latest BUILDING2 high
preserve-entry-size screen-clip promotion improves B2-high to `1343/1312`
while preserving entry sizes and pack LBA. The latest VISITOR3 high
screen-clip headroom pass
clips offscreen cleanup spans in entries `101` and `116`, dropping active
payload `437785 -> 436469` with exact-flat five-yellow timing. Release gates
now require a nearby `MEM_REGION_RATIONALE` for every
`memAlloc` call site.
The prior accepted JOHNNY1 pack promotion compresses full-frame entries `1`
and `50` behind a scene-local local-LZ sentinel, preserving pack footprint and
the `217088` byte PS-EXE bucket while moving both tides to `1948/1945`,
overrun `3`, blocking/refill `5`, and target speed `99.85%`.
The accepted BUILDING2 low setup-segment promotions prime relative sectors
`112..128` and `226..262` during setup without changing pack size/LBA, use a
low-only `80 KiB` clean-strip cap, raise the low window slack to `5`, and add
`{141,153}`, improving low to `1327/1318`, overrun `9`, blocking `47`,
reads `27`, refill `0`, and due `9`. BUILDING2 high now layers guarded `271..287` plus `315..327` rows on top
of the allocator-safe setup slices, then the previous-visible cleanup promotion
moves it to `1343/1312`, overrun `31`, blocking/refill `50/17`, and due `7`;
the entries `92`/`94`/`95` payload trim plus cleanup pass cut B2-high active
payload `669408 -> 574094`, and `{185..197}` plus `{158..174}` bank same-speed
CD-pressure work under the new baseline. BUILDING4 low now carries the v971 local-LZ entry270 follow-up, the gap-8 dirty-upload band merge retune, and the `24 KiB` stream-window promotion at
`2847/2820`, overrun `27`, blocking/read time `32`/`252`, and prefetch overrun
`27`, cutting active payload `807263 -> 799277` while preserving pack layout.
The newest WALKSTUF1
baselines use allocator-safe targeted setup slices instead of full setup
buffers: high now measures `1472/1441`, blocking/refill/due `43/13/7`, with
loop reads/read time at `41/198` after keeping relative sectors
`198..244`, extending the second slice to `286..344`, adding `{149,165}`,
encoding frame `92` as previous-frame D4, adding `{423,439}`, `{404,416}`,
`{395,411}`, and retargeted `{411,423}`,
and letting high tide use prepare-before-window scheduler ownership; low replaces the old `197..243` plus
`410..434` split with a single `238..344` retained CACHE setup segment after
low-only 48 KiB clean-rect chunking, then adds the `{91,107}` first-boundary
read group and a split TRANSIENT `344..350` setup edge, retargets setup to
`244..350` plus split `179..185`, adds `{113,129}`, and then adds
`{355,371}` as same-speed read-work, then applies the preserve-entry-size
screen-clip pass for hidden-refill headroom. It now measures `1470/1446`,
blocking/refill/due `32/3/4` after the fresh-owner `160..176` pocket, with
loop reads/read time at `24/146`.
Both paths keep pack LBA/sectors and the PS-EXE bucket fixed while
reducing CD pressure. VISITOR3 now keeps only its tiny
stage1 prefetch frame buffer plus bounded clean-relief stream windows: high
uses an `80 KiB` window with terminal overread trimming, merged setup segment
`203..262`, frame `139` raw-gap relocation, frames `56` and `57` raw in that
paid gap with a `56 KiB` tight-refill cap, high clean strips capped at
`64 KiB`, and the early retained setup edge `40..47`.
High measures `1070/1046`, overrun `24`, blocking
`35`, reads/due `5/2`; low uses a
`16 KiB` slack-5 window plus a third retained setup segment extended to
`206..232`, with frame `138` raw relocated into that paid gap, and measures
`1065/1039`, overrun `26`, blocking `75`, reads/due `18/14`. Both rows moved out of red and the orange band is now empty
without reintroducing the clean-rect allocation
failure. BUILDING4 high now primes relative sectors `264..288` during setup,
improving `2847/2816 -> 2843/2816`, overrun `31 -> 27`, blocking/refill
`36/32 -> 34/30`, and moving that row into green. VISITOR5 high/low both remain green on the matching `30..46`
retained-read shape. WALKSTUF1 high/low now measure `1472/1441`
and `1470/1446`; the latest high frame92 D4 stack cuts high overrun
`34 -> 31` without moving layout, and the latest high scheduler row cuts
blocking/due `56/10 -> 43/7`, while low keeps the `238..344` retained
setup segment, the split `344..350` setup edge, the no-shift payload lane, and
the frame132 payload trim plus `{378..390}` read group. BUILDING2 low now keeps the accepted
`218..229` slack-8 retained-read row plus v739 draw-tail trimming, then primes
relative sectors `112..128` and `226..262` during setup, with the allocator-era
matrix at `1327/1318`, overrun `9`, blocking/refill `47/0`,
reads `27`, and due `9`. BUILDING2 high currently measures
`1343/1312`, overrun `31`, blocking `50`, refill overrun `17`, and due misses
`7`; the allocator-safe setup slices plus the `83..95`, `{158..174}`, guarded
`271..287`, `315..327`, `{185..197}`, and previous-visible cleanup promotion leave reads/read time at
`44/196`, and same-speed offscreen clips now reduce its runtime rows/spans/pixels to
`18030/105645/446246`, and the v877/v879/v880 preserve-offset frame172/frame171/frame96
trims plus the entries `92`/`94`/`95` follow-up reduce active payload
`674798 -> 663590`; the new cleanup pass drops it further to `574094` without moving pack layout.
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
- **Milestone release cadence.** `v0.8.16-ps1` promotes the memory-region
  allocator: BOOT/CACHE/TRANSIENT lifetimes, scene-boundary transient wipes,
  CACHE free-list/LRU reuse, generated pack-header metrics, and allocator
  rationale gates. `v0.8.15-ps1` promotes the WALKSTUF1 high setup-resident CD
  row. `v0.8.14-ps1` promotes JOHNNY1 high/low local-LZ full-frame compression.
  `v0.8.9-ps1` promotes the current mainline
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
