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
At the time of writing it contains 600+ experiment rows going back to
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
120 timing-bearing battle-card rows, the average is **+17.4% over target /
87.1% target speed**.

## Scene Battle Card

As of 2026-04-29, all 126 scene/tide variants have current headless
perf measurements under `compact-fgp3-v2-fullmatrix`. 63 of 63 scenes have
at least one routed variant, and 63 scenes have both high- and low-tide
variants routed. 120 rows carry active-loop timing; `mary3`, `suzy1`, and
`suzy2` high/low complete as metadata-only routes and are excluded from
speed averages. The latest matrix run is `2026-04-29T18:13:13`; per-row freshness and stats version are shown on the
[scene ledger]({{ '/scenes/' | relative_url }}). The values below are `over
target / target speed (loop_vb/target_vb)`, with `blk` and `due` called out
when nonzero.

Every current row is stamped `compact-fgp3-v2-fullmatrix`, produced after
FGP3 padding removal with the parallel, case-local CD harness. Older
`padded-fgp3-v1` / `compact-fgp3-v1` rows are historical only.

| Scene | High tide | Low tide |
|---|---:|---:|
| `activity1` | +3.1% / 97.0% (4373/4243); blk 1 | +3.1% / 97.0% (4373/4243); blk 1 |
| `activity10` | +11.8% / 89.4% (1406/1257); due 1; blk 13 | +11.4% / 89.8% (1401/1258); due 2; blk 14 |
| `activity11` | +8.0% / 92.6% (1862/1724); due 1; blk 7 | +8.0% / 92.6% (1862/1724); due 1; blk 7 |
| `activity12` | +9.5% / 91.3% (1548/1414); blk 5 | +9.4% / 91.4% (1543/1411); due 1; blk 8 |
| `activity4` | +13.1% / 88.4% (1206/1066); blk 6 | +12.6% / 88.8% (1204/1069); blk 2 |
| `activity5` | +9.4% / 91.4% (1867/1707); due 4; blk 27 | +9.0% / 91.8% (1860/1707); due 3; blk 26 |
| `activity6` | +14.5% / 87.3% (1043/911) | +14.5% / 87.3% (1043/911) |
| `activity7` | +22.6% / 81.6% (750/612); blk 7 | +21.3% / 82.5% (747/616) |
| `activity8` | +17.3% / 85.3% (1058/902); due 5; blk 21 | +16.4% / 85.9% (1050/902); due 4; blk 12 |
| `activity9` | +11.0% / 90.1% (2267/2042); due 10; blk 96 | +11.2% / 90.0% (2272/2044); due 8; blk 94 |
| `building1` | +23.4% / 81.1% (951/771); due 8; blk 63 | +19.9% / 83.4% (935/780); due 4; blk 37 |
| `building2` | +20.2% / 83.2% (1559/1297); due 19; blk 150 | +20.1% / 83.3% (1556/1296); due 20; blk 150 |
| `building3` | +9.4% / 91.4% (1565/1430); blk 5 | +9.6% / 91.3% (1571/1434) |
| `building4` | +12.2% / 89.2% (3128/2789); due 49; blk 355 | +12.1% / 89.2% (3126/2788); due 49; blk 353 |
| `building5` | +5.0% / 95.2% (3504/3336); due 6; blk 52 | +4.5% / 95.7% (3498/3348); due 2; blk 16 |
| `building6` | +12.9% / 88.6% (2744/2431); due 49; blk 342 | +12.9% / 88.6% (2747/2433); due 49; blk 343 |
| `building7` | +4.8% / 95.4% (3843/3668); due 4; blk 43 | +4.2% / 96.0% (3830/3676); blk 12 |
| `fishing1` | +12.2% / 89.1% (1207/1076) | +12.2% / 89.1% (1207/1076) |
| `fishing2` | +7.6% / 92.9% (1899/1765); blk 3 | +7.4% / 93.1% (1898/1767) |
| `fishing3` | +7.4% / 93.1% (2095/1951); due 1; blk 21 | +6.6% / 93.8% (2090/1960); blk 3 |
| `fishing4` | +15.0% / 87.0% (968/842); blk 2 | +15.7% / 86.5% (975/843); blk 1 |
| `fishing5` | -9.3% / 110.3% (807/890) | -9.2% / 110.1% (808/890) |
| `fishing6` | +19.1% / 84.0% (893/750); blk 6 | +18.7% / 84.2% (894/753); blk 4 |
| `fishing7` | +19.2% / 83.9% (863/724); blk 6 | +19.4% / 83.7% (866/725); blk 2 |
| `fishing8` | +11.9% / 89.4% (1393/1245); blk 13 | +11.2% / 89.9% (1388/1248); blk 13 |
| `johnny1` | +9.4% / 91.4% (2125/1942); blk 31 | +9.6% / 91.2% (2129/1942); blk 33 |
| `johnny2` | +7.2% / 93.2% (1878/1751) | +7.3% / 93.2% (1878/1750); blk 1 |
| `johnny3` | +11.3% / 89.8% (1299/1167); due 1; blk 6 | +11.8% / 89.4% (1303/1165); blk 6 |
| `johnny4` | +11.1% / 90.0% (1349/1214) | +11.1% / 90.0% (1349/1214) |
| `johnny5` | +16.5% / 85.8% (954/819); blk 1 | +16.3% / 86.0% (954/820) |
| `johnny6` | +3.4% / 96.7% (2895/2800); blk 27 | +3.4% / 96.7% (2896/2800); blk 27 |
| `mary1` | +3.7% / 96.4% (5004/4826); due 2; blk 49 | +3.2% / 96.9% (4994/4839); due 1; blk 26 |
| `mary2` | +1.6% / 98.4% (2284/2247); blk 7 | +1.6% / 98.4% (2285/2249); blk 6 |
| `mary3` | metadata-only (0/0) | metadata-only (0/0) |
| `mary4` | -2.4% / 102.4% (1968/2016); due 3; blk 28 | -2.6% / 102.7% (1966/2019); due 3; blk 24 |
| `mary5` | +6.6% / 93.8% (1687/1583); blk 7 | +6.6% / 93.8% (1688/1583); blk 8 |
| `miscgag1` | +15.5% / 86.6% (1105/957); blk 13 | +14.8% / 87.1% (1101/959); blk 7 |
| `miscgag2` | +0.1% / 99.9% (1356/1355) | +0.1% / 99.9% (1356/1355) |
| `stand1` | +73.6% / 57.6% (349/201); blk 5 | +70.9% / 58.5% (347/203) |
| `stand10` | +25.5% / 79.7% (674/537); blk 2 | +26.5% / 79.1% (678/536); blk 1 |
| `stand11` | +25.7% / 79.6% (675/537); blk 3 | +27.4% / 78.5% (683/536); blk 7 |
| `stand12` | +10.1% / 90.9% (1599/1453); blk 12 | +11.9% / 89.4% (1622/1450); blk 35 |
| `stand15` | +14.3% / 87.5% (1127/986); blk 7 | +14.9% / 87.0% (1135/988); blk 11 |
| `stand16` | +11.7% / 89.5% (1327/1188); blk 8 | +12.1% / 89.2% (1333/1189); due 1; blk 13 |
| `stand2` | +30.5% / 76.6% (633/485); blk 10 | +30.3% / 76.7% (632/485); blk 8 |
| `stand3` | +24.4% / 80.4% (693/557) | +25.1% / 79.9% (697/557) |
| `stand4` | +11.8% / 89.4% (1361/1217); blk 7 | +12.3% / 89.0% (1366/1216); blk 9 |
| `stand5` | +9.2% / 91.5% (1595/1460); blk 2 | +10.8% / 90.3% (1606/1450); blk 20 |
| `stand6` | +10.2% / 90.7% (1501/1362); blk 4 | +10.7% / 90.3% (1505/1359); blk 8 |
| `stand7` | +25.6% / 79.6% (676/538); blk 3 | +26.9% / 78.8% (680/536); blk 5 |
| `stand8` | +27.0% / 78.7% (635/500) | +28.5% / 77.8% (640/498); blk 3 |
| `stand9` | +25.5% / 79.7% (674/537); blk 2 | +26.3% / 79.2% (678/537); blk 1 |
| `suzy1` | metadata-only (0/6) | metadata-only (0/6) |
| `suzy2` | metadata-only (0/6) | metadata-only (0/6) |
| `visitor1` | +21.0% / 82.6% (812/671); blk 13 | +20.4% / 83.1% (809/672); blk 14 |
| `visitor3` | +50.5% / 66.4% (1526/1014); due 23; blk 368 | +51.4% / 66.1% (1547/1022); due 22; blk 332 |
| `visitor4` | +39.8% / 71.5% (590/422); due 1; blk 26 | +37.7% / 72.6% (584/424); blk 11 |
| `visitor5` | +17.6% / 85.0% (1274/1083); due 9; blk 79 | +14.3% / 87.5% (1244/1088); due 6; blk 49 |
| `visitor6` | +7.5% / 93.0% (2195/2042); blk 13 | +8.3% / 92.3% (2209/2040); blk 22 |
| `visitor7` | +8.9% / 91.9% (1768/1624); blk 3 | +9.7% / 91.1% (1780/1622); blk 10 |
| `walkstuf1` | +144.8% / 40.8% (3320/1356); due 318; blk 1800 | +177.3% / 36.1% (3755/1354); due 328; blk 2195 |
| `walkstuf2` | +29.6% / 77.2% (596/460); blk 1 | +29.6% / 77.2% (596/460); blk 1 |
| `walkstuf3` | +8.1% / 92.5% (2460/2276); due 6; blk 79 | +7.9% / 92.7% (2466/2285); due 5; blk 66 |

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
CD-heavy scenes (`walkstuf1`, `visitor3`, `building4`, `building6`, `walkstuf3`). The
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
- **Timing compression before throughput work.** The timing-bearing matrix
  average is still +17.4% over target, with several much worse CD-bound
  outliers; compressing the timing files would expose the same throughput
  bottleneck without fixing it.
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
