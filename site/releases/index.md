---
title: Releases
eyebrow: Tagged versions · what shipped, when
subtitle: A short index of every milestone and stability release. Themes, headlines, links to the full notes and the disc image.
description: Every tagged release of the Johnny Castaway PS1 fan port — themes, dated headlines, and links to the full notes and downloadable disc image for each version.
redirect_from:
  - /changelog/
---

A labor of love by Hunter Davis. The project tags milestones every ~10 newly validated scenes and cuts smaller stability releases between them when the runtime needs a fix that doesn't change the validated set. Below is the short version of what each tag carried — the full notes live alongside the source for anyone who wants the root-cause-and-mitigation depth.

The current release line is **`{{ site.release.tag }}`** with
**{{ site.release.scenes_validated }} / {{ site.release.scenes_total }}** scenes signed off under the
[FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## Latest

### `v0.8.3-ps1` — WALKSTUF1 compact foreground performance plus VISITOR3 motion payloads
<time datetime="2026-05-08"><em>2026-05-08</em></time>

This point release promotes the WALKSTUF1 compact foreground pack pass and
records the later VISITOR3 motion-copy payload follow-up as the current
headless performance baseline.

- **WALKSTUF1 gets compact FGP3/v4 packs.** Both tides move from PAL4/FGP2 to
  padded compact FGP3/v4 restore-minus-current packs while preserving the
  `1535263` byte pack footprints, original LBAs, and the `215040` byte PS-EXE
  bucket.
- **The outlier gap drops sharply.** WALKSTUF1 high improves `1592/1406 ->
  1491/1426`; low improves `1604/1407 -> 1489/1427`.
- **Visible CD pressure falls.** High blocking drops `275 -> 85`; low blocking
  drops `270 -> 86`; loop reads drop from `134/132` to `69/69`.
- **Battle card is now public-capped at native speed.** After the follow-up
  MARY3, BUILDING1, VISITOR5 high, BUILDING2 low, WALKSTUF3 high, BUILDING6,
  ACTIVITY9 high, WALKSTUF3 low, JOHNNY1 compact, ACTIVITY9 low compact, and
  VISITOR3 motion-copy/code-headroom/CD-pressure/setup-prime/resident-pack/no-op residual
  passes through v248, all 126 timing-bearing rows average `+0.3558%` public
  over target / `99.6529%` public target speed; the raw signed CSV is
  `-0.4126%` / `100.4367%` for
  optimization work.
- **VISITOR3 motion-copy payloads are promoted.** Frames `119..123` in both
  VISITOR3 tides, high-tide frame `115`, shared frames `118`/`124`, and
  high-only frame `117`, high-only re-anchored frames `127`/`126`/`125`, and
  the high-only `320 KiB` setup-prime cap, the guarded low `150..174`
  second setup segment, the low frame-125/frame-126 resident re-anchor, and
  the low frame-118/frame-127 resident copies, the high frame-127/frame-130
  resident-copy compaction, plus the low frame-114/frame-117 no-op residual
  compaction now
  move already-composited
  background rows or CD residency out of the active loop;
  the v204/v205 setup segments keep low sectors `281..305` and high sectors
  `277..293` resident before the active loop. High improves `1118/1028 ->
  1075/1037`, low improves `1126/1025 -> 1086/1035`, blocking drops
  `150/170 -> 59/93`, and the packs
  keep fixed LBAs and the `215040` byte PS-EXE bucket.

The post-validation perf retrospective at
[/lab/from-87-to-99-5/]({{ '/lab/from-87-to-99-5/' | relative_url }})
covers v0.8.0 → v0.8.3 as one continuous arc.

[Full notes]({{ '/source/docs/ps1/release-notes-0.8.3/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.8.3-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

## Earlier milestones

### `v0.8.2-ps1` — VISITOR3 guarded-read performance
<time datetime="2026-05-07"><em>2026-05-07</em></time>

This point release promotes the next VISITOR3 high-tide guarded generated-window
read group and ships the current upstream website/docs polish with the latest
performance battle-card numbers.

- **VISITOR3 high visible CD pressure improves.** The guarded `138..162` read
  window lowers `blocking_vb 294 -> 293`, `loop_reads 40 -> 39`, and
  `loop_read_vb 335 -> 332`.
- **Loop cadence stays fixed.** VISITOR3 high remains `1406/1019` VBlanks with
  `overrun_vb=387` and `prefetch_overrun_vb=7`.
- **Battle card remains near target.** The 120 timing-bearing rows average
  `+0.5706%` over target / `99.6769%` target speed.
- **Site/docs are current with `main`.** The release includes the latest site
  navigation, glossary, lab/feed, structured-data, and page-TOC polish.

The post-validation perf retrospective at
[/lab/from-87-to-99-5/]({{ '/lab/from-87-to-99-5/' | relative_url }})
covers this and the v0.8.3 follow-on as continuations of the
v0.8.0 baseline arc.

[Full notes]({{ '/source/docs/ps1/release-notes-0.8.2/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.8.2-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.8.1-ps1` — clean-rect pressure stability
<time datetime="2026-05-06"><em>2026-05-06</em></time>

A randomized long-run soak exposed a scene-load freeze after a large split
clean-rect save. The pressure estimator was counting only foreground pack
bounds, not the ocean wave band or the upper/lower split actually saved for
restore. `v0.8.1` fixes that accounting and keeps the complete-scene
performance baseline intact. The retrospective on what the soak found that
the matrix didn't is at
[/lab/v081-mary4-freeze/]({{ '/lab/v081-mary4-freeze/' | relative_url }}).

- **Scene-load freeze fixed.** Large clean snapshots now account for wave-band
  expansion and split rects before allocation.
- **Pressure relief is generalized.** The fix covers every random-position
  scene with the same large-clean risk, not just the failing MARY4 route.
- **Focused soak routes pass.** MARY4 and FISHING1 pressure routes complete
  with `scene-end` and `alloc_fail=0`.

[Full notes]({{ '/source/docs/ps1/release-notes-0.8.1/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.8.1-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.8.0-ps1` — complete-scene performance baseline
<time datetime="2026-05-06"><em>2026-05-06</em></time>

The first release after the post-validation polish phase to promote the headless optimization methodology as a release baseline. All 63 scenes stay green under the visual + audible bar, and all 126 high/low variants are now routed through the perf matrix. Timing-bearing rows now average **+0.9% over target / [99.5% target speed]({{ '/docs/glossary/#target-speed' | relative_url }})** — about 16.5 percentage points of over-target gap closed since the compact full-matrix baseline. The retrospective on which experiments moved that line is at [/lab/from-87-to-99-5/]({{ '/lab/from-87-to-99-5/' | relative_url }}).

- **63 / 63 scenes still validated** after the post-validation bugfix pass.
- **126 / 126 variants routed**, 120 carrying active-loop timing.
- **Performance is near target** — `+0.9%` average over target, `99.5%` target speed.
- **ACTIVITY 9 is now an optimized validated outlier** — wide-boat stitch + padded FGP3 residual packs + scoped low-tide read group.
- **Random-run clean-rect pressure fixed** — the BUILDING4 soak regression that exposed walk-clean memory pressure now releases the stale walk clean buffer, retries the large scene clean snapshot, and recaptures the walk baseline cleanly.

[Full notes]({{ '/source/docs/ps1/release-notes-0.8.0/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.8.0-ps1)
&nbsp;·&nbsp;
[Download .bin / .cue]({{ '/play/' | relative_url }})

### `v0.7.2-ps1` — story-loop walk backdrop guard
<time datetime="2026-05-05"><em>2026-05-05</em></time>

A randomized [story-loop walking]({{ '/docs/walks/' | relative_url }}) regression let Johnny walk over water and leave repeated walking poses when the next scene's island backdrop state differed from the framebuffer left by the previous scene. The fix was a backdrop key — the runtime remembers the tide / raft / night / holiday / island position that produced the previous frame and refuses to start a walk unless the next scene matches.

- **Walks now require a matching backdrop key.** Inter-scene walks only run when tide, raft, night, holiday, and island X/Y all match the previous rendered scene.
- **Scene-policy changes force a clean scene load.** Moving from a variable-position scene to a fixed/left-island/no-raft/tide/holiday variant inside the same sequence no longer draws Johnny over stale water.
- **Menu and freeplay resets clear the walk context.** Scene Set changes, Scene Explorer launches, and Freeplay exits all invalidate the remembered backdrop.

[Full notes]({{ '/source/docs/ps1/release-notes-0.7.2/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.7.2-ps1)

### `v0.7.1-ps1` — persisted holiday mode and first-run defaults
<time datetime="2026-05-05"><em>2026-05-05</em></time>

Memory-card schema bumped to v6 to separate the holiday *policy* from the manual override, so the pause menu can offer five distinct holiday modes. New boots default to `AUTO DATE:ORIG4` — Sierra's original four overlays, automatic by date.

- **`AUTO DATE:ORIG4` is now the fresh/no-card default.** Reflects the source material's holiday set.
- **Holiday mode is persisted separately from manual selection.** Memory card schema v6.
- **Walking holiday z-order corrected.** Story-loop walks now stamp holiday overlays before Johnny so default decorations don't paint over the walking sprite.

[Full notes]({{ '/source/docs/ps1/release-notes-0.7.1/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.7.1-ps1)

### `v0.7.0-ps1` — complete scene validation
<time datetime="2026-05-05"><em>2026-05-05</em></time>

The 63-scene grind ended here. Every routed scene the original Sierra game had now plays pixel-perfect on the PS1 with synced SFX across every applicable variant. The retrospective on what the daily loop actually looked like is at [/lab/the-63-scene-grind/]({{ '/lab/the-63-scene-grind/' | relative_url }}).

- **63 / 63 scenes validated.** The live ledger is fully green under the FISHING 1 reference bar.
- **ACTIVITY 9 completed the sweep.** The final scene needed a wide-boat repair path because `BOAT.PSB` can extend past the legacy 640px scene clip.
- **Landing page now uses the ACTIVITY 9 boat screenshot.** The hero image shows the final validated scene running in the PS1 build.

[Full notes]({{ '/source/docs/ps1/release-notes-0.7.0/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.7.0-ps1)

### `v0.6.0-ps1` — ocean ambience
<time datetime="2026-05-01"><em>2026-05-01</em></time>

A 20-second CC0 ocean-loop sample plays on a dedicated SPU voice
underneath every scene. Toggleable from the pause menu; choice
persists to the memory card. Zero per-frame CPU cost — the SPU
loops the sample in hardware while the main CPU keeps doing
playback work.

- **Looping ocean ambience** on SPU voice slot reserved at boot.
- **Pause → Accessibility → Ocean: ON / OFF**, persisted to memcard.
- **Bootmode token** for capture/test discs that need a known-state run.

[GitHub release]({{ site.github_url }}/releases/tag/v0.6.0-ps1)

### `v0.6.1-ps1` through `v0.6.13-ps1` — the validation-cluster patches
<em><time datetime="2026-05-01">2026-05-01</time> → <time datetime="2026-05-04">2026-05-04</time></em>

The stability releases between `v0.6.0-ps1` (ocean ambience) and
`v0.7.0-ps1` (complete scene validation) are scene-by-scene
bring-up tags rather than feature milestones. Each one carries a
specific scene clearing the FISHING 1 bar (FISHING 5, MARY 5,
VISITOR 4–7, and others) plus the small bugfix needed to get the
scene through. They aren't enumerated here because the per-scene
narrative belongs in the [history page]({{ '/about/history/' | relative_url }})
and the [scene ledger]({{ '/scenes/' | relative_url }}) tracks the
final state. The full list of GitHub-tagged builds in this cluster
is at [{{ site.repo }}/releases]({{ site.github_url }}/releases).

### `v0.5.0-ps1` — Freeplay and debug mode
<time datetime="2026-05-01"><em>2026-05-01</em></time>

The release that turned the project from a passive screensaver port into something a player could touch. Direct-control Johnny via D-pad / analog, gag and visitor debug catalogs in the pause menu, controllable world state, and a no-allocation steady-state freeplay loop.

- **Freeplay mode** with D-pad / analog walking, fishing, and L2/R2 speed modifiers.
- **Pause-menu debug catalogs** for gags, visitors, sound effects, controls, world options, accessibility, and system pages.
- **Frog-clock loading transitions** keep the screen coherent between scene swaps.
- **Steady-state freeplay loop does not allocate.** Important because freeplay can run indefinitely.

[Full notes]({{ '/source/docs/ps1/release-notes-0.5.0/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.5.0-ps1)

### `v0.4.20-ps1` — story-loop walking
<time datetime="2026-04-30"><em>2026-04-30</em></time>

The first build where Johnny no longer teleports between finished scenes. The PS1 screensaver loop now carries his spot and heading forward, runs Sierra's original `walk_data.h` route table, and visibly walks him across the island before the next FG2 scene begins.

- **Story-loop walking** wired into the PS1 screensaver loop using the original route table and `JOHNWALK` sprite bank.
- **The ocean keeps animating during walks**, and active holiday overlays persist across scene → walk → scene transitions.
- **Palm-tree occlusion works.** The trunk and leaves are re-stamped over Johnny when the route passes behind the tree.
- **Tight, persistent walk erase buffer** (340×224, ≈149 KB) replaces the early build's per-walk free/malloc churn that fragmented the heap.

[Full notes]({{ '/source/docs/ps1/release-notes-0.4.20/' | relative_url }})
&nbsp;·&nbsp;
[GitHub release]({{ site.github_url }}/releases/tag/v0.4.20-ps1)

## Where to go from here

- [Play]({{ '/play/' | relative_url }}) is the download page — latest `.bin` / `.cue`, DuckStation quickstart, controller map.
- The [FAQ]({{ '/faq/' | relative_url }}) covers the recurring questions: what this is, why PS1, is this legal, do I need Sierra files, real-hardware support.
- The [scene ledger]({{ '/scenes/' | relative_url }}) is the live status of every scene at the current release.
- The [history page]({{ '/about/history/' | relative_url }}) is the longer narrative — pre-port era, first PS1 attempts, the hybrid pivot, the 63-scene grind.
- The [devlog]({{ '/devlog/' | relative_url }}) is the daily worklog. Releases are the milestones; the devlog is the run-up.
- Older releases (`v0.3.x` and earlier) aren't included above because they predate the per-version release-notes habit. Their commit messages and the [history page]({{ '/about/history/' | relative_url }}) cover the same ground.

The full list of all tagged releases on GitHub is at
[{{ site.repo }}/releases]({{ site.github_url }}/releases).
