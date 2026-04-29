---
layout: page
title: Performance work
eyebrow: Reference + log
subtitle: What "performance" means on a 33 MHz machine, what got measured, what got tried, and what stuck.
description: Performance work on the Johnny Castaway PS1 fan port — frame budget, perf instrumentation, the experiment ledger of what was tried and what landed in the runtime.
---

A labor of love by Hunter Davis. This page is the running summary of perf
work on the PS1 port at {{ site.release.tag }}: where the bottleneck is, what
got measured, which experiments stayed in the build, and which got reverted.
The full per-experiment ledger lives in the source tree; the link is at the
bottom. If you paid for this, you were cheated. Open source and free.

## The constraint

"Performance" on a PS1 means a different shape of problem than performance
on anything modern.

The MIPS R3000A core runs at 33.8688 MHz with no FPU. The GPU is
fixed-function — sprites, primitives, an ordering table, no shaders. Audio
is a separate processor with its own RAM. The CD is a 2x drive: 300 KB/s
sustained, 150 ms cold seek. There is no memory bandwidth budget worth
talking about for a 16-color screensaver port; the bandwidth budget is the
CD's, and it gets spent in seek latency, not transfer time.

The frame budget at 60 Hz is 16.6 ms. *Johnny Castaway* is a 1992 VGA
screensaver — at the source level, foreground content changes roughly four
times per second. The PS1 still has to draw a frame at 60 Hz, but it can
hold the same content frame after frame for many VBlanks at a stretch. The
VBlank cadence is the rendering loop's heartbeat; the *interesting* timing
is which VBlanks have actual work in them and which are held idle.

That asymmetry is what shapes the runtime. A held VBlank is free CPU and
free CD bus. The whole optimization story is about scheduling work — CD
reads, RAM tile composition, dirty-row uploads — into held VBlanks before
the next "real" frame arrives. When that scheduling fails, the active
frame's VBlank gets stretched and `loop_vb` goes up.

The frame budget for a screensaver is more forgiving than a game. Nothing
the user does requires sub-frame latency. But the project's acceptance bar
is pixel-perfect playback against host-captured reference frames, which
means the runtime cannot drop frames or compress timing files to "feel
faster" — it has to render every captured entry on the captured beat.
Slack exists in the held intervals; it does not exist in the entries.

## What was measured

The perf instrumentation lives in
[`src/ps1_perf.c`]({{ site.github_url }}/blob/main/src/ps1_perf.c). It is
gated so it adds zero cost when off.

Three signal sources:

- **TTY printf** at scene-start and scene-end with structured `JCPERF` /
  `JCPERF2` records. Levels: `OFF`, `SUMMARY`, `DETAIL`, `DEBUG`. Only the
  on-demand records cross the TTY surface; per-frame text is forbidden in
  hot paths because it perturbs timing.
- **`ps1_perf` module counters** for VBlank-level metrics: `loop_vb`,
  `target_vb`, `overrun_vb`, `blocking_vb`, `prefetch_overrun_vb`,
  `due_misses`, `restore_bytes`, `upload_bytes`, `dirty_rows`,
  `upload_rects`, `loop_reads`. Each scene-end record dumps the
  steady-state values for that run.
- **Regtest harness frame timing.** The headless DuckStation in
  [`scripts/run-regtest.sh`]({{ site.github_url }}/blob/main/scripts/run-regtest.sh)
  boots the disc image, captures PNGs, and ingests the TTY records into
  per-run summary JSON files under `scratch/ps1-perf-iterate/<runId>/`.

Every experiment goes through the same gate:
[`scripts/ps1-perf-iterate.sh`]({{ site.github_url }}/blob/main/scripts/ps1-perf-iterate.sh)
runs the case, compares it to a baseline `summary.json`, and either
promotes (if a key metric improved without a material regression in
`loop_vb` / `blocking_vb` / `prefetch_overrun_vb` / scene identity) or
rejects with a recorded failure reason.

The full experiment log is at
[`docs/ps1/performance-experiment-log.md`]({{ site.github_url }}/blob/main/docs/ps1/performance-experiment-log.md).
At the time of writing it contains roughly 130 experiments going back to
2026-04-25. Most of them failed.

The full scene/tide battle card is
[`docs/ps1/performance-scene-matrix.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-scene-matrix.csv)
and is rendered as the perf battle card section of the [scene ledger]({{ '/scenes/' | relative_url }}).
It is not the human scene-promotion ledger; it is the timing sheet for
headless performance work.

## Experiments that didn't work

A representative slice of rejected experiments and why each one didn't
stick. The pattern is more useful than any individual line — almost every
"obvious" idea gets discarded because the PS1 runtime has counter-intuitive
cost structure.

- **Larger stream windows.** `40 KB`, `56 KB`, `64 KB`. Larger windows
  reduce CD transaction count but overrun held slack more often. The
  current default is `20 KB` after a long sweep; everything bigger lost.
- **Smaller stream windows.** `12 KB`, `14 KB`, `16 KB`. Smaller windows
  reduce per-refill overrun but starve due frames — `due_misses` rises
  and `blocking_vb` follows. The knee is sharp; one sector size in
  either direction matters.
- **Disabling stage1 isolation.** Booting with `no-stage1` to test
  whether stage-copy overhead was a real cost. The headless harness
  exited 137 before `JCPERF2` could record anything; the test was
  structurally inconclusive. Kept staging on.
- **Partial tail reads when a staged frame straddles the window end.**
  Sounded right on paper. In practice, smaller tail reads multiply CD
  transaction count and `due_misses` rises faster than the byte
  savings help. Rejected.
- **Compose-before-VSync sequencing.** Move the FG2 RAM composition
  before the VBlank wait so CPU work overlaps with previous-frame
  scanout, then upload after VBlank. Reduced `prefetch_overrun_vb` but
  stole held-prefetch time elsewhere; total `loop_vb` regressed by 12.
- **Held-loop no-slack wait skip.** Looked like a clean one-VBlank
  overshoot fix. Regressed loop, blocking, and refill metrics
  simultaneously; the skipped wait was load-bearing.
- **Async stream-window refill.** Naive async polling regressed
  `blocking_vb` badly. The CD subsystem has implicit ownership rules
  the synchronous path was respecting; the async path violated them.
  Rejected without a first-class CD-state ownership model.
- **`-O3` on hot translation units.** Less prepared RAM work in some
  scenes, but worse loop/blocking/refill timing overall. The
  optimization changed code shape enough that CD scheduling phase
  shifted unfavorably. Kept `-O2`.
- **Holiday overlap restamping.** Seed holiday decoration into the
  clean backdrop and only restamp it when the current FG2 frame
  overlaps. Logically sound, but the active fishing1 frames overlap
  the Christmas decoration enough that this didn't reduce dirty work.
  Pure no-op, rejected.
- **`vprintf` inline diagnostics.** Adding a CD-read histogram inline
  with `JCPERF` regressed timing even with detail-gating. The act of
  having the code present changed binary shape enough to move
  scheduling phase. Reverted; histograms now live in post-processing.
- **FG2 sound-event table in the metadata prefix.** Setup reads
  improved, but moving the table ahead of the payload shifted every
  payload by 36 bytes and badly worsened active CD phase. The pack
  layout is more sensitive to byte offsets than is comfortable.

The recurring lesson: changes that look like clean wins on paper often
shift CD scheduling phase in ways that are not visible until the full
scene runs. The headless gate is what catches this; experiments that
regress `loop_vb` or `blocking_vb` against a baseline get rejected
even when they "obviously" should have helped.

## Experiments that did

A condensed list of changes that survived and are in the runtime today.
They cluster into a few themes.

**Foreground prefetch and stream window:**

- Stage1 staging buffer for the next FG2 entry, prefetched during
  held VBlanks.
- Stream window default of `20 KB`, reduced from earlier `32 KB` after
  the post-pause-merge sweep showed it as the local minimum.
- 3 VBlank refill guard, raised from earlier 2/1 thresholds after
  smaller guards caused due-frame starvation.
- Forward-extend stream window when a straddling entry is detected:
  preserve the resident suffix and append-read only the missing
  aligned tail. Replaces overlapping full-window refills.
- Stage-copy fallthrough at 5 VBlanks: after a zero-VBlank stage copy
  from the resident window, immediately prefetch the following
  window if at least 5 held VBlanks remain. Converts idle held time
  into hidden CD work.
- Tight-slack direct staging up to `8 KB` for immediate payloads when
  the window refill would otherwise be skipped.

**Compositor:**

- PAL4 opaque-span compositor — FG2 PAL4 spans contain only visible
  pixels, so the per-pixel transparent-index branch was removable.
- Tile-local PAL4 fast path — split each span by destination tile
  once instead of per-pixel.
- Per-tile PAL4 row dirty marking — track which rows of which tiles
  changed, not just which tiles.
- Base-diff FG2 pack format — the active path requires base-diff
  packs, which makes RAM tile compositing the only render path and
  lets `grBeginFrame()` / `ClearOTagR()` skip when nothing's queued.

**Dirty-rect bookkeeping:**

- X-aware clean-rect restore — track dirty X extents per tile so
  RAM clean-background restore only touches the changed region.
- Vertical dirty-row upload bands with an 11-row gap merge —
  collapses adjacent uploads into wider rectangles.
- Long-hold host-deadline catch-up — a small render bookkeeping
  adjustment that traded seven extra speculative restore/compose
  calls for five fewer loop VBlanks.

**Code shape and link:**

- `-ffunction-sections -fdata-sections` plus `--gc-sections` for the
  PS1 link. The legacy ADS / TTM / FG1 / FOC runtime paths are still
  in the source tree but get stripped at link time.
- Removal of the foreground visual telemetry hot-path body, the
  legacy foreground diagnostic gate, the unused foreground "ever"
  diagnostics, the unused ADS foreground start hook, the obsolete
  FGPILOT ADS dispatch, the unused foreground status accessors, the
  dead foreground requested-mode state.

**Diagnostic gating:**

- Pad / SPI diagnostics gated default-off. The pause-menu work
  introduced always-on `JCPAD` / `JCSPI` sampling; a strict-gate
  red-team pass showed the diagnostics were costing 52 VBlanks of
  loop time. Default-off recovered that; `pad-diag` / `pad-debug`
  boot tokens still enable them on demand.

The cumulative effect is visible in the current accepted baseline:
fishing1 high-tide playback at `loop_vb=1207` against a target of
`target_vb=1076`. The original headless perf-loop baseline was
`loop_vb=1426`, so the FISHING 1 canary is down `219` VBlanks
(`15.36%` loop reduction).

## Where it sits at {{ site.release.tag }}

The current accepted fishing1 high-tide run, captured in the perf log:

```text
policy = stage1_window
buf    = 333656
hits   = 155
due_misses = 0
blocking_vb = 0
prefetch.overrun_vb = 0
loop_vb = 1207
overrun_vb = 131
target_vb = 1076
restore_bytes = 251,144
upload_bytes  = 8,533,120
dirty_rows    = 13,333
upload_rects  = 436
trip = 0   fallback = 0   frame_mismatch = 0
sound_late = 0   cd_fail = 0
```

That is **+12.2% over target**, or **89.1% of target speed**. Across the
measured battle-card rows, the average is **+12.3% over target / 89.6%
target speed**.

## Scene Battle Card

As of 2026-04-29, 64 of 126 scene/tide variants have current headless
perf measurements. 39 of 63 scenes have at least one timed variant; 25
scenes have both high- and low-tide variants timed. The values below are
`over target / target speed (loop_vb/target_vb)`, with `blk` and `due`
called out when nonzero.

| Scene | High tide | Low tide |
|---|---:|---:|
| `activity1` | +3.0% / 97.1% (4373/4244) | +3.1% / 97.0% (4373/4242); blk 2 |
| `activity4` | +12.9% / 88.5% (1205/1067); blk 5 | +12.5% / 88.9% (1203/1069); blk 1 |
| `activity5` | +9.3% / 91.5% (1866/1707); due 4, blk 25 | +9.1% / 91.7% (1861/1706); due 3, blk 29 |
| `activity6` | +14.5% / 87.3% (1043/911) | +14.5% / 87.3% (1043/911) |
| `activity7` | +22.7% / 81.5% (751/612); blk 8 | +21.7% / 82.2% (747/614); blk 1 |
| `activity8` | pending | pending |
| `activity9` | +11.2% / 89.9% (2268/2039); due 9, blk 97 | +11.1% / 90.0% (2271/2044); due 8, blk 92 |
| `activity10` | +12.3% / 89.1% (1408/1254); due 1, blk 18 | +12.2% / 89.1% (1408/1255); due 2, blk 23 |
| `activity11` | +8.1% / 92.5% (1864/1724); due 1, blk 9 | +8.3% / 92.3% (1866/1723); due 1, blk 10 |
| `activity12` | +9.9% / 91.0% (1551/1411); blk 12 | +10.2% / 90.8% (1551/1408); due 1, blk 20 |
| `building1` | +25.6% / 79.6% (966/769); due 6, blk 76 | pending |
| `building2` | +21.8% / 82.1% (1572/1291); due 20, blk 173 | +21.4% / 82.4% (1570/1293); due 20, blk 172 |
| `building3` | +9.4% / 91.4% (1565/1431); blk 4 | pending |
| `building4` | +12.5% / 88.9% (3141/2792); due 39, blk 326 | +12.4% / 89.0% (3135/2790); due 40, blk 335 |
| `building5` | +5.4% / 94.9% (3514/3334); due 6, blk 65 | +4.8% / 95.4% (3508/3346); due 2, blk 33 |
| `building6` | +13.1% / 88.4% (2754/2435); due 39, blk 317 | +12.9% / 88.6% (2746/2433); due 38, blk 312 |
| `building7` | +5.5% / 94.8% (3861/3659); due 3, blk 66 | +4.5% / 95.7% (3837/3672); blk 23 |
| `fishing1` | +12.2% / 89.1% (1207/1076) | +12.2% / 89.1% (1207/1076) |
| `fishing2` | +7.5% / 93.0% (1898/1765); blk 2 | +7.4% / 93.1% (1898/1767) |
| `fishing3` | +7.1% / 93.4% (2093/1955); due 1, blk 15 | +6.6% / 93.8% (2090/1960); blk 3 |
| `fishing4` | +16.7% / 85.7% (978/838); blk 15 | pending |
| `fishing5` | pending | pending |
| `fishing6` | pending | pending |
| `fishing7` | +19.4% / 83.8% (863/723); blk 7 | pending |
| `fishing8` | +12.4% / 88.9% (1400/1245); blk 21 | pending |
| `johnny1` | +9.5% / 91.3% (2128/1943); blk 31 | +9.8% / 91.1% (2132/1942); blk 37 |
| `johnny2` | pending | pending |
| `johnny3` | +12.8% / 88.7% (1308/1160); due 1, blk 20 | +12.2% / 89.1% (1305/1163); blk 10 |
| `johnny4` | pending | pending |
| `johnny5` | pending | pending |
| `johnny6` | +3.6% / 96.5% (2901/2799); blk 33 | +3.8% / 96.3% (2905/2798); blk 37 |
| `mary1` | +4.5% / 95.7% (5028/4813); due 2, blk 87 | +3.7% / 96.4% (5011/4830); due 1, blk 50 |
| `mary2` | +1.7% / 98.3% (2286/2248); blk 8 | +1.7% / 98.3% (2286/2248); blk 8 |
| `mary3` | pending | pending |
| `mary4` | pending | pending |
| `mary5` | +7.4% / 93.1% (1698/1581); blk 20 | +7.0% / 93.4% (1694/1583); blk 14 |
| `miscgag1` | pending | pending |
| `miscgag2` | pending | pending |
| `stand1` | pending | pending |
| `stand2` | pending | pending |
| `stand3` | pending | pending |
| `stand4` | +12.3% / 89.1% (1365/1216); blk 12 | pending |
| `stand5` | +9.7% / 91.2% (1600/1459); blk 8 | pending |
| `stand6` | +10.5% / 90.5% (1503/1360); blk 8 | pending |
| `stand7` | pending | pending |
| `stand8` | pending | pending |
| `stand9` | pending | pending |
| `stand10` | pending | pending |
| `stand11` | pending | pending |
| `stand12` | +9.7% / 91.2% (1597/1456); blk 8 | pending |
| `stand15` | pending | pending |
| `stand16` | +11.7% / 89.5% (1328/1189); blk 7 | pending |
| `suzy1` | BLOCKED | pending |
| `suzy2` | BLOCKED | pending |
| `visitor1` | +22.5% / 81.6% (822/671); blk 22 | pending |
| `visitor3` | +56.1% / 64.1% (1581/1013); due 24, blk 424 | +58.4% / 63.1% (1611/1017); due 26, blk 404 |
| `visitor4` | pending | pending |
| `visitor5` | +20.0% / 83.3% (1295/1079); due 9, blk 107 | +15.2% / 86.8% (1250/1085); due 6, blk 62 |
| `visitor6` | +7.5% / 93.0% (2198/2044); blk 14 | pending |
| `visitor7` | +9.6% / 91.2% (1777/1621); blk 15 | pending |
| `walkstuf1` | pending | pending |
| `walkstuf2` | pending | pending |
| `walkstuf3` | +10.4% / 90.6% (2512/2275); due 6, blk 130 | pending |

Detail-tier attribution for the canary currently points at render and
restore pressure rather than CD stalls:

```text
sched.wait       = 769
sched.present    = 105
sched.cd_stage   = 146
sched.cd_window  =   6
gfx.restore_bytes = 251,144
gfx.upload_bytes  = 8,533,120
```

The canary now has no visible CD stall, but the full battle card still has
CD-heavy scenes (`visitor3`, `building4`, `building6`, `walkstuf3`). The
next plausible wins, in priority order:

1. **FG2-specific present pipeline with explicit slack budgeting.**
   Earlier detail counters showed present/wait ownership as a real
   scheduling surface, but the first attempt at a staged-present
   scheduler regressed loop time by disrupting CD prefetch. The next
   design needs separate render-prep and CD-prefetch slack budgets —
   stealing held-frame prefetch cadence is what the first attempt got
   wrong.
2. **CD stall hiding beyond the current direct-stage / window path.**
   Fishing1 is already at `blocking_vb=0`, but the broader scene matrix
   still has large blocking counts. Every saved read still compounds.
3. **X-aware dirty upload and rect-pressure control.** Upload still
   restores 251 KB and uploads 8.5 MB in the FISHING 1 canary; larger
   scenes have much more pressure. Getting that down without changing
   pixels is still a clean win.
4. **Pack-emitted read groups and sector layout.** Current raw-window
   reads still leave large blocking counts in non-canary scenes; grouped
   metadata is the likely next CD breakthrough.
5. **Specialized PAL4 FG2 compositor.** Fishing frames are modest;
   larger scenes will make span/tile split and PAL4 conversion overhead
   more important.

The author considers the current build comfortable for the validated
scenes, not yet headroom-clean. The canary bottleneck is no longer raw CD
stall; the matrix bottleneck is uneven per-scene payload/read shape plus
render/restore pressure. Those are different bugs from the ones the perf
work was chasing early in the loop, and each new experiment needs to stay
matrix-aware instead of only optimizing FISHING 1.

## Non-goals

A few things the perf work explicitly does not chase, with reasons:

- **Frame dropping.** Violates pixel-perfect playback. The acceptance
  bar requires every captured entry to render on its captured beat.
- **Timing compression before throughput work.** Measured playback is
  1.55x over target; compressing the timing files would expose the
  same throughput bottleneck without fixing it.
- **Reintroducing FG1 / ADS / TTM runtime paths.** Those are retired
  from the active public path. The PS1 executable links only the
  scene-playback runtime plus the minimal background / audio / input
  / CD layers it needs.
- **Fixed island assumptions.** The runtime must randomly place the
  island per scene, so all optimizations must preserve scene-relative
  FG2 placement.
- **Direct framebuffer or progressive-mode experiments as first
  moves.** Prior history says these were unstable. Exhaust stable
  scene playback first.

## Related pages

- [Hardware]({{ '/docs/hardware/' | relative_url }}) — what the
  optimizations are running against.
- [Build & toolchain]({{ '/docs/build/' | relative_url }}) — how the
  PS1 binary is produced.
- [Build infrastructure]({{ '/docs/infrastructure/' | relative_url }}) —
  the wrapper around the perf iterate script.
- [Audio pipeline]({{ '/docs/audio/' | relative_url }}) — the SPU side,
  which has its own scheduling concerns.
- [Vision-classifier work]({{ '/docs/vision/' | relative_url }}) — the
  validation layer that runs against perf-experiment outputs.
- [Devlog]({{ '/devlog/' | relative_url }}) — perf work shows up
  day-by-day there.

## View source on GitHub

- [`docs/ps1/performance-optimization-plan.md`]({{ site.github_url }}/blob/main/docs/ps1/performance-optimization-plan.md)
- [`docs/ps1/performance-experiment-log.md`]({{ site.github_url }}/blob/main/docs/ps1/performance-experiment-log.md)
- [`src/ps1_perf.c`]({{ site.github_url }}/blob/main/src/ps1_perf.c)
- [`src/foreground_pilot.c`]({{ site.github_url }}/blob/main/src/foreground_pilot.c)
- [`scripts/ps1-perf-iterate.sh`]({{ site.github_url }}/blob/main/scripts/ps1-perf-iterate.sh)
