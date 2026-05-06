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

The current compiler-flag sweep is tracked in
[`docs/ps1/performance-o2-audit.md`]({{ site.github_url }}/blob/main/docs/ps1/performance-o2-audit.md)
and its machine-readable
[`performance-o2-audit.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-o2-audit.csv).
That report is regenerated from `build-ps1/compile_commands.json` and
`build-ps1/jcreborn.map` before each `-O2` probe.

The current pack-time graphics preprocessing target sheet is
[`docs/ps1/performance-preprocess-opportunities.md`]({{ site.github_url }}/blob/main/docs/ps1/performance-preprocess-opportunities.md)
and its machine-readable
[`performance-preprocess-opportunities.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-preprocess-opportunities.csv).
It ranks today’s FG2/FGP3 packs for selective upload-ready or cleanup metadata
work without changing the runtime baseline.

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
fishing1 high-tide playback at `loop_vb=1067` against a target of
`target_vb=1074`. The original headless perf-loop baseline was
`loop_vb=1426`, so the FISHING 1 canary is down `359` VBlanks
(`25.18%` loop reduction).

## Where it sits at {{ site.release.tag }}

The current accepted fishing1 high-tide run, captured in the perf log:

```text
policy = stage1_window
buf    = 137048
hits   = 155
due_misses = 0
blocking_vb = 2
prefetch.overrun_vb = 2
loop_vb = 1067
overrun_vb = 0
target_vb = 1074
restore_bytes = 251,144
upload_bytes  = 8,643,840
dirty_rows    = 13,506
upload_rects  = 439
trip = 0   fallback = 0   frame_mismatch = 0
sound_late = 0   cd_fail = 0
```

That is **-0.7% over target**, or **100.7% of target speed**. Across the
120 timing-bearing battle-card rows, the average is **+7.6% over target /
93.5% target speed** (`7.5934%` exact over target / `93.5083%` exact target speed).

## Scene Battle Card

As of 2026-05-06, all 126 scene/tide variants have current headless
perf measurements. The latest updated rows are stamped
`visitor5-v072-prefetch-relief`,
`mismatch-top-v072-current-refresh`,
`stand-family-v072-current-refresh`,
`visitor4-v072-current-refresh`,
`stand1-v072-current-refresh`,
`walkstuf1-v072-prefetch-relief`,
`visitor3-v072-prefetch-relief`,
`mary2-v068-wide-stitch`,
`fishing5-v065-current-ledger-overlay`,
`johnny2-v064-validation-refresh`,
`compact-fgp3-v66-final-frame-hold`,
`compact-fgp3-v64-building2-group318-330`,
`compact-fgp3-v63-building2low-prime`, and
`indexed8-row-local-dirty-v1`; other refreshed rows include
`compact-fgp3-v62-fishing3low-group253-265`,
`compact-fgp3-v61-fishing3low-group163-175`,
`compact-fgp3-v60-visitor3high-group230-242`,
`compact-fgp3-v59-visitor3high-group72-84`, `indexed8-tile-local-compose-v1`,
`compact-fgp3-v58-activity9high-window20-table`, `compact-fgp3-v57-policy-table-refactor`, and `compact-fgp3-v49-walkstuf2-auto-prime` through `compact-fgp3-v29-smallprime`, and the full-matrix baseline rows are stamped
`compact-fgp3-v2-fullmatrix`. 63 of 63 scenes have at least one routed
variant, and 63 scenes have both high- and low-tide variants routed. 120 rows
carry active-loop timing; `suzy1` and `suzy2` high/low complete as
metadata-only routes and are excluded from speed averages. `mary3` is visually
validated but still needs a perf-matrix refresh. The latest matrix
run is `2026-05-06T00:23:01`; per-row freshness and stats version are shown on
the [scene ledger]({{ '/scenes/' | relative_url }}). The values below are
`over target / target speed (loop_vb/target_vb)`, with `blk` and `due` called
out when nonzero.

The complete matrix pass is `compact-fgp3-v2-fullmatrix`; accepted follow-up
rows now use `visitor5-v072-prefetch-relief`,
`mismatch-top-v072-current-refresh`,
`stand-family-v072-current-refresh`,
`visitor4-v072-current-refresh`,
`stand1-v072-current-refresh`,
`visitor3-v072-prefetch-relief`,
`walkstuf1-v072-prefetch-relief`,
`compact-fgp3-v66-final-frame-hold`,
`mary2-v068-wide-stitch`,
`fishing5-v065-current-ledger-overlay`,
`johnny2-v064-validation-refresh`,
`compact-fgp3-v64-building2-group318-330`,
`compact-fgp3-v63-building2low-prime`, and
`indexed8-row-local-dirty-v1`; other refreshed rows include
`compact-fgp3-v62-fishing3low-group253-265`,
`compact-fgp3-v61-fishing3low-group163-175`,
`compact-fgp3-v60-visitor3high-group230-242`,
`compact-fgp3-v59-visitor3high-group72-84`, `indexed8-tile-local-compose-v1`,
`compact-fgp3-v58-activity9high-window20-table`, `compact-fgp3-v57-policy-table-refactor`, and `compact-fgp3-v49-walkstuf2-auto-prime` through `compact-fgp3-v29-smallprime`. Older `padded-fgp3-v1` / `compact-fgp3-v1`
rows are historical only.

| Scene | High tide | Low tide |
|---|---:|---:|
| `activity1` | +3.1% / 97.0% (4373/4243); blk 1 | +3.1% / 97.0% (4373/4243); blk 1 |
| `activity4` | +12.4% / 88.9% (1202/1069) | +12.4% / 88.9% (1202/1069) |
| `activity5` | +9.4% / 91.4% (1867/1707); due 4; blk 27 | +9.0% / 91.8% (1860/1707); due 3; blk 26 |
| `activity6` | +14.5% / 87.3% (1043/911) | +14.5% / 87.3% (1043/911) |
| `activity7` | -0.5% / 100.5% (593/596) | -0.3% / 100.3% (594/596) |
| `activity8` | -0.7% / 100.7% (898/904); blk 1 | -0.6% / 100.6% (899/904); blk 2 |
| `activity9` | +10.4% / 90.6% (2259/2047); due 6; blk 84 | +11.2% / 90.0% (2272/2044); due 8; blk 94 |
| `activity10` | +10.9% / 90.2% (1399/1262) | +11.4% / 89.8% (1401/1258); due 2; blk 14 |
| `activity11` | +7.8% / 92.8% (1859/1725) | +7.8% / 92.8% (1859/1725) |
| `activity12` | +9.0% / 91.7% (1543/1415) | +9.4% / 91.4% (1543/1411); due 1; blk 8 |
| `building1` | +23.3% / 81.1% (951/771); due 8; blk 63 | +19.9% / 83.4% (935/780); due 4; blk 37 |
| `building2` | +19.8% / 83.5% (1552/1296); due 19; blk 144 | +18.9% / 84.1% (1542/1297); due 20; blk 138 |
| `building3` | +9.4% / 91.4% (1565/1430); blk 5 | +9.1% / 91.7% (1564/1434) |
| `building4` | +4.9% / 95.4% (2928/2792); due 34; blk 234 | +4.6% / 95.6% (2925/2797); due 2; blk 96 |
| `building5` | +5.0% / 95.2% (3504/3336); due 6; blk 52 | +4.5% / 95.7% (3498/3348); due 2; blk 16 |
| `building6` | +5.3% / 95.0% (2561/2433); due 33; blk 223 | +5.3% / 95.0% (2564/2436); due 33; blk 217 |
| `building7` | +4.8% / 95.4% (3843/3668); due 4; blk 43 | +4.2% / 96.0% (3830/3676); blk 12 |
| `fishing1` | -0.7% / 100.7% (1067/1074); blk 2 | +12.2% / 89.1% (1207/1076) |
| `fishing2` | +7.6% / 92.9% (1899/1765); blk 3 | +7.4% / 93.1% (1898/1767) |
| `fishing3` | +0.4% / 99.6% (1960/1952); due 1; blk 18 | +0.1% / 99.9% (1956/1954); blk 6 |
| `fishing4` | +14.7% / 87.2% (967/843) | +14.7% / 87.2% (967/843) |
| `fishing5` | -9.4% / 110.4% (807/891) | -9.5% / 110.5% (806/891) |
| `fishing6` | +18.2% / 84.6% (890/753) | +18.2% / 84.6% (890/753) |
| `fishing7` | -1.4% / 101.4% (715/725) | -1.4% / 101.4% (715/725) |
| `fishing8` | +11.9% / 89.4% (1393/1245); blk 13 | +10.1% / 90.8% (1380/1253) |
| `johnny1` | +9.4% / 91.4% (2125/1942); blk 31 | +9.6% / 91.2% (2129/1942); blk 33 |
| `johnny2` | +0.6% / 99.4% (1761/1751); due 3; blk 16 | +0.5% / 99.5% (1758/1750); due 3; blk 16 |
| `johnny3` | +11.3% / 89.8% (1298/1166) | +11.3% / 89.8% (1298/1166) |
| `johnny4` | +10.5% / 90.5% (1341/1214) | +10.5% / 90.5% (1341/1214) |
| `johnny5` | +15.5% / 86.6% (947/820) | +15.5% / 86.6% (947/820) |
| `johnny6` | +3.4% / 96.7% (2895/2800); blk 27 | +3.4% / 96.7% (2896/2800); blk 27 |
| `mary1` | +3.7% / 96.4% (5004/4826); due 2; blk 49 | +3.2% / 96.9% (4994/4839); due 1; blk 26 |
| `mary2` | +0.2% / 99.8% (2250/2246); blk 4 | +0.3% / 99.7% (2253/2246); blk 7 |
| `mary3` | validated; perf refresh pending | validated; perf refresh pending |
| `mary4` | -2.4% / 102.4% (1968/2016); due 3; blk 28 | -2.6% / 102.7% (1966/2019); due 3; blk 24 |
| `mary5` | +6.6% / 93.8% (1687/1583); blk 7 | +6.6% / 93.8% (1688/1583); blk 8 |
| `miscgag1` | +14.2% / 87.6% (1097/961) | +14.0% / 87.7% (1096/961) |
| `miscgag2` | -0.3% / 100.3% (1352/1356) | -0.3% / 100.3% (1352/1356) |
| `stand1` | -4.0% / 104.1% (194/202) | -4.0% / 104.1% (194/202) |
| `stand2` | -2.0% / 102.1% (480/490) | -2.0% / 102.1% (480/490) |
| `stand3` | +24.4% / 80.4% (694/558) | +24.4% / 80.4% (694/558) |
| `stand4` | +11.3% / 89.8% (1359/1221) | +11.2% / 89.9% (1358/1221) |
| `stand5` | +9.2% / 91.6% (1595/1461) | +9.1% / 91.7% (1594/1461) |
| `stand6` | +10.0% / 90.9% (1501/1365) | +9.9% / 91.0% (1500/1365) |
| `stand7` | -3.3% / 103.5% (520/538) | -3.3% / 103.5% (520/538) |
| `stand8` | -3.2% / 103.3% (483/499); blk 2 | -3.2% / 103.3% (483/499); blk 2 |
| `stand9` | -3.3% / 103.5% (520/538) | -3.0% / 103.1% (522/538) |
| `stand10` | -1.9% / 101.9% (528/538) | -1.9% / 101.9% (528/538) |
| `stand11` | -1.9% / 101.9% (528/538) | -1.9% / 101.9% (528/538) |
| `stand12` | +9.1% / 91.7% (1594/1461) | +9.1% / 91.7% (1593/1460) |
| `stand15` | +13.4% / 88.2% (1123/990) | +13.4% / 88.1% (1122/989) |
| `stand16` | +11.0% / 90.1% (1322/1191) | +11.1% / 90.0% (1323/1191) |
| `suzy1` | metadata-only | metadata-only |
| `suzy2` | metadata-only | metadata-only |
| `visitor1` | -0.7% / 100.7% (672/677) | -0.7% / 100.7% (672/677) |
| `visitor3` | +44.1% / 69.4% (1455/1010); due 31; blk 363 | +44.0% / 69.4% (1453/1009); due 32; blk 365 |
| `visitor4` | -0.9% / 100.9% (424/428) | -0.9% / 100.9% (424/428) |
| `visitor5` | +1.9% / 98.1% (1111/1090); blk 12 | +2.0% / 98.0% (1112/1090); blk 12 |
| `visitor6` | +7.5% / 93.0% (2195/2042); blk 13 | +6.8% / 93.6% (2188/2048) |
| `visitor7` | +8.7% / 92.0% (1766/1625) | +8.7% / 92.0% (1766/1625) |
| `walkstuf1` | +16.1% / 86.1% (1637/1410); due 54; blk 297 | +16.0% / 86.2% (1634/1409); due 55; blk 304 |
| `walkstuf2` | +28.4% / 77.9% (593/462) | +28.4% / 77.9% (593/462) |
| `walkstuf3` | +8.1% / 92.5% (2460/2276); due 6; blk 79 | +7.9% / 92.7% (2466/2285); due 5; blk 66 |

Detail-tier attribution for the canary currently points at render and
restore pressure rather than CD stalls:

```text
sched.wait       = 722
sched.present    = 99
sched.cd_stage   = 137
sched.cd_window  = 19
gfx.restore_bytes = 251,144
gfx.upload_bytes  = 8,643,840
```

The canary now has only two visible CD/refill VBlanks, but the full battle card still has
CD-heavy scenes (`walkstuf1`, `walkstuf3`, `visitor3`, `building4`,
`building6`). The `FGP3/v2` indexed8 results, WALKSTUF1 split-window policy,
and the derived high/low setup-prime budgets prove host-side pack preprocessing plus
scene-local CD policy can move major outliers, but they also leave enough
residual pressure to keep the next experiments matrix-aware.

Next plausible wins, in priority order:

1. **Generated read grouping or setup segmentation for residual indexed8
   packs.** WALKSTUF1 high/low still have `blocking_vb=423/452`, so the
   format/window/setup-prime/compositor wins need a second CD-shape pass.
2. **FG2-specific present pipeline with explicit slack budgeting.** Earlier
   present-prep experiments regressed because they stole CD prefetch slack;
   the next scheduler needs separate render-prep and CD-prefetch budgets.
3. **X-aware dirty upload and rect-pressure control.** The FISHING 1 canary
   still restores 251 KB and uploads 8.5 MB; larger scenes carry more upload
   pressure.
4. **Specialized indexed8 and PAL4 compositors.** The pack-format wins reduce
   bytes, but dense scenes still pay per-span/per-pixel runtime costs.
5. **Remaining metadata-only scene diagnosis.** `suzy1` and `suzy2`
   still complete without active-loop timing, so their packs are not yet part
   of the speed average. `mary3` moved out of that class visually and needs a
   fresh matrix row.

The author considers the current build comfortable for the validated scenes,
not yet headroom-clean. The canary bottleneck is no longer raw CD stall; the
matrix bottleneck is uneven per-scene payload/read shape plus render/restore
pressure.

## Non-goals

A few things the perf work explicitly does not chase, with reasons:

- **Frame dropping.** Violates pixel-perfect playback. The acceptance
  bar requires every captured entry to render on its captured beat.
- **Timing compression before throughput work.** The timing-bearing matrix
  average is still +14.7% over target, with several much worse CD-bound
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
