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
bottom. The retrospective on how the matrix moved from the compact baseline
to the current battle card —
[*From 87 to 99.5: the post-validation performance loop*]({{ '/lab/from-87-to-99-5/' | relative_url }})
— is in the Lab. If you paid for this, you were cheated. Open source and free.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

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
and is rendered as the live, sortable, color-coded battle card at
[/perf/]({{ '/perf/' | relative_url }}). It is not the human
scene-promotion ledger at [/scenes/]({{ '/scenes/' | relative_url }});
the two ledgers stay separate on purpose — different bars,
different cadences, different failure modes.

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

The per-pack detail analyzer
[`scripts/analyze-fg2-preprocess-plans.py`]({{ site.github_url }}/blob/main/scripts/analyze-fg2-preprocess-plans.py)
now parses both FGP2 and FGP3 temporal-residual payloads. Its VISITOR3 output
splits cap-hit frames from saving-heavy frames, which keeps the next
upload-ready experiment selective instead of a whole-pack conversion. The
current VISITOR3 frame sheet is
[`docs/ps1/performance-preprocess-visitor3-hotspots.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-preprocess-visitor3-hotspots.csv).

The current post-`-O2` tooling pass also records compact baseline
fingerprints in every perf summary and classifies foreground read-plan
candidates by observed append-start ownership, current grouped-read capacity,
and visible-CD cost class. That makes stale-baseline comparisons, no-op read
groups, and tight visible-cluster candidates visible before a runtime source
edit.

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
fishing1 high-tide playback at `loop_vb=1069` against a target of
`target_vb=1072`. The original headless perf-loop baseline was
`loop_vb=1426`, so the FISHING 1 canary is down `357` VBlanks
(`25.04%` loop reduction).

## Where it sits at {{ site.release.tag }}

The current accepted fishing1 high-tide run, captured in the perf log:

```text
policy = stage1_window
buf    = 137048
hits   = 155
due_misses = 0
blocking_vb = 5
prefetch.overrun_vb = 6
loop_vb = 1069
overrun_vb = 0
target_vb = 1072
restore_bytes = 251,144
upload_bytes  = 10,648,960
dirty_rows    = 16,639
upload_rects  = 460
trip = 0   fallback = 0   frame_mismatch = 0
sound_late = 0   cd_fail = 0
```

That is **-0.3% over target**, or **100.3% of target speed**. Across the
120 timing-bearing battle-card rows, the average is **+0.8% over target /
99.5% target speed** (`0.8231%` exact over target / `99.4858%` exact target speed).

## Scene Battle Card

As of 2026-05-06, all 126 scene/tide variants have current headless
perf measurements. The latest updated rows are stamped
`building5-fgp3-padded-v080`,
`visitor3-low-group170-186-v080b`,
`walkstuf1-fgp2-setup-prime-v080`,
`visitor3-setup-prime-192k-v080`,
`visitor3-high-group170-186-v080-current`,
`activity9-lowgroup-v072c`,
`activity9-fgp3-v072c`,
`activity9-window-v072c`,
`activity4-fishing4-v072c-prefetch-relief`,
`building4-6-johnny6-v072c-prefetch-relief`,
`activity1-v072c-current-refresh`,
`activity11-12-v072c-prefetch-relief`,
`stale-next-v072c-current-refresh`,
`mary1-v072c-prefetch-relief`,
`stale-layout-v072c-current-refresh`,
`activity9-v072c-prefetch-relief`,
`stale-pressure2-v072c-current-refresh`,
`johnny1-v072c-prefetch-relief`,
`stale-pressure-v072c-current-refresh`,
`activity10-johnny3-v072-prefetch-relief`,
`stale-zero2-v072b-current-refresh`,
`stale-zero-v072b-current-refresh`,
`stale-top-v072b-current-refresh`,
`visitor5-v072-prefetch-relief`,
`mismatch-top-v072-current-refresh`,
`stand-family-v072-current-refresh`,
`visitor4-v072-current-refresh`,
`stand1-v072-current-refresh`,
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
run is `2026-05-06T17:38:07`; per-row freshness and stats version are shown on
the [battle card]({{ '/perf/' | relative_url }}). The values below are
`over target / target speed (loop_vb/target_vb)`, with `blk` and `due` called
out when nonzero.

The complete matrix pass is `compact-fgp3-v2-fullmatrix`; accepted follow-up
rows now use `building5-fgp3-padded-v080`,
`visitor3-low-group170-186-v080b`,
`walkstuf1-fgp2-setup-prime-v080`,
`visitor3-setup-prime-192k-v080`,
`visitor3-high-group170-186-v080-current`,
`activity9-lowgroup-v072c`,
`activity9-fgp3-v072c`,
`activity9-window-v072c`,
`building4-6-johnny6-v072c-prefetch-relief`,
`activity4-fishing4-v072c-prefetch-relief`,
`activity1-v072c-current-refresh`,
`activity11-12-v072c-prefetch-relief`,
`stale-next-v072c-current-refresh`,
`mary1-v072c-prefetch-relief`,
`stale-layout-v072c-current-refresh`,
`activity9-v072c-prefetch-relief`,
`stale-pressure2-v072c-current-refresh`,
`johnny1-v072c-prefetch-relief`,
`stale-pressure-v072c-current-refresh`,
`activity10-johnny3-v072-prefetch-relief`,
`stale-zero2-v072b-current-refresh`,
`stale-zero-v072b-current-refresh`,
`stale-top-v072b-current-refresh`,
`visitor5-v072-prefetch-relief`,
`mismatch-top-v072-current-refresh`,
`stand-family-v072-current-refresh`,
`visitor4-v072-current-refresh`,
`stand1-v072-current-refresh`,
`visitor3-v072-prefetch-relief`,
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

<table class="scene-table perf-summary-table">
  <thead>
    <tr>
      <th>Scene</th>
      <th>High tide</th>
      <th>Low tide</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><code>activity1</code></td>
      <td>-0.4% / 100.4% (2754/2764); blk 1</td>
      <td>-0.4% / 100.4% (2754/2765)</td>
    </tr>
    <tr>
      <td><code>activity4</code></td>
      <td>-0.1% / 100.1% (1065/1066); blk 4</td>
      <td>-0.4% / 100.4% (1064/1068); blk 1</td>
    </tr>
    <tr>
      <td><code>activity5</code></td>
      <td>-1.1% / 101.1% (1730/1749); blk 2</td>
      <td>-1.0% / 101.0% (1731/1749); blk 2</td>
    </tr>
    <tr>
      <td><code>activity6</code></td>
      <td>+0.1% / 99.9% (912/911)</td>
      <td>+0.1% / 99.9% (912/911)</td>
    </tr>
    <tr>
      <td><code>activity7</code></td>
      <td>-0.5% / 100.5% (593/596)</td>
      <td>-0.3% / 100.3% (594/596)</td>
    </tr>
    <tr>
      <td><code>activity8</code></td>
      <td>-0.7% / 100.7% (898/904); blk 1</td>
      <td>-0.6% / 100.6% (899/904); blk 2</td>
    </tr>
    <tr>
      <td><code>activity9</code></td>
      <td>+2.2% / 97.9% (2101/2056); due 2; blk 44</td>
      <td>+1.8% / 98.2% (2093/2056); due 5; blk 43</td>
    </tr>
    <tr>
      <td><code>activity10</code></td>
      <td>+0.0% / 100.0% (1259/1259); due 1; blk 7</td>
      <td>-0.1% / 100.1% (1255/1256); due 2; blk 17</td>
    </tr>
    <tr>
      <td><code>activity11</code></td>
      <td>+0.5% / 99.5% (1729/1720); due 1; blk 10</td>
      <td>+0.7% / 99.3% (1729/1717); due 1; blk 14</td>
    </tr>
    <tr>
      <td><code>activity12</code></td>
      <td>-0.1% / 100.1% (1411/1412); blk 7</td>
      <td>-0.1% / 100.1% (1409/1411); due 1; blk 10</td>
    </tr>
    <tr>
      <td><code>building1</code></td>
      <td>+2.1% / 98.0% (794/778); blk 21</td>
      <td>+1.9% / 98.1% (794/779); blk 21</td>
    </tr>
    <tr>
      <td><code>building2</code></td>
      <td>+14.9% / 87.1% (1476/1285); due 37; blk 286</td>
      <td>+14.6% / 87.2% (1465/1278); due 40; blk 279</td>
    </tr>
    <tr>
      <td><code>building3</code></td>
      <td>-0.1% / 100.1% (5460/5465)</td>
      <td>-0.1% / 100.1% (5460/5465)</td>
    </tr>
    <tr>
      <td><code>building4</code></td>
      <td>+7.6% / 92.9% (2985/2774); due 40; blk 285</td>
      <td>+7.1% / 93.4% (2981/2784); due 14; blk 199</td>
    </tr>
    <tr>
      <td><code>building5</code></td>
      <td>-0.1% / 100.1% (3343/3348); blk 5</td>
      <td>-0.1% / 100.1% (3345/3347); blk 8</td>
    </tr>
    <tr>
      <td><code>building6</code></td>
      <td>+3.2% / 96.9% (2520/2442); due 1; blk 62</td>
      <td>+3.2% / 96.9% (2515/2437); due 2; blk 70</td>
    </tr>
    <tr>
      <td><code>building7</code></td>
      <td>-0.0% / 100.0% (3132/3133); blk 9</td>
      <td>-0.1% / 100.1% (3130/3133); blk 7</td>
    </tr>
    <tr>
      <td><code>fishing1</code></td>
      <td>-0.6% / 100.6% (1068/1074); blk 2</td>
      <td>-0.7% / 100.7% (1067/1074); blk 1</td>
    </tr>
    <tr>
      <td><code>fishing2</code></td>
      <td>-0.1% / 100.1% (1761/1763); blk 6</td>
      <td>-0.3% / 100.3% (1759/1765); blk 3</td>
    </tr>
    <tr>
      <td><code>fishing3</code></td>
      <td>+0.4% / 99.6% (1960/1952); due 1; blk 18</td>
      <td>+0.1% / 99.9% (1956/1954); blk 6</td>
    </tr>
    <tr>
      <td><code>fishing4</code></td>
      <td>-0.8% / 100.8% (835/842); blk 2</td>
      <td>-1.1% / 101.1% (834/843)</td>
    </tr>
    <tr>
      <td><code>fishing5</code></td>
      <td>-0.6% / 100.6% (885/890)</td>
      <td>-0.6% / 100.6% (885/890)</td>
    </tr>
    <tr>
      <td><code>fishing6</code></td>
      <td>-1.2% / 101.2% (744/753)</td>
      <td>-1.2% / 101.2% (744/753)</td>
    </tr>
    <tr>
      <td><code>fishing7</code></td>
      <td>-1.4% / 101.4% (715/725)</td>
      <td>-1.4% / 101.4% (715/725)</td>
    </tr>
    <tr>
      <td><code>fishing8</code></td>
      <td>-0.8% / 100.8% (1243/1253)</td>
      <td>-0.8% / 100.8% (1243/1253)</td>
    </tr>
    <tr>
      <td><code>johnny1</code></td>
      <td>+1.5% / 98.5% (1974/1944); blk 27</td>
      <td>+1.5% / 98.5% (1974/1944); blk 27</td>
    </tr>
    <tr>
      <td><code>johnny2</code></td>
      <td>+0.6% / 99.4% (1761/1751); due 3; blk 16</td>
      <td>+0.5% / 99.5% (1758/1750); due 3; blk 16</td>
    </tr>
    <tr>
      <td><code>johnny3</code></td>
      <td>-0.3% / 100.3% (1158/1161); due 1; blk 10</td>
      <td>-0.8% / 100.8% (1157/1166)</td>
    </tr>
    <tr>
      <td><code>johnny4</code></td>
      <td>-0.8% / 100.8% (1204/1214)</td>
      <td>-0.8% / 100.8% (1204/1214)</td>
    </tr>
    <tr>
      <td><code>johnny5</code></td>
      <td>-1.1% / 101.1% (811/820)</td>
      <td>-1.2% / 101.2% (810/820)</td>
    </tr>
    <tr>
      <td><code>johnny6</code></td>
      <td>+1.1% / 98.9% (2832/2800); blk 28</td>
      <td>+1.1% / 98.9% (2832/2800); blk 28</td>
    </tr>
    <tr>
      <td><code>mary1</code></td>
      <td>+0.8% / 99.2% (4867/4830); due 2; blk 47</td>
      <td>+0.4% / 99.6% (4860/4840); due 1; blk 31</td>
    </tr>
    <tr>
      <td><code>mary2</code></td>
      <td>+0.2% / 99.8% (2250/2246); blk 4</td>
      <td>+0.3% / 99.7% (2253/2246); blk 7</td>
    </tr>
    <tr>
      <td><code>mary3</code></td>
      <td>validated; perf refresh pending</td>
      <td>validated; perf refresh pending</td>
    </tr>
    <tr>
      <td><code>mary4</code></td>
      <td>-2.4% / 102.4% (1968/2016); due 3; blk 28</td>
      <td>-2.6% / 102.7% (1966/2019); due 3; blk 24</td>
    </tr>
    <tr>
      <td><code>mary5</code></td>
      <td>+0.6% / 99.4% (1591/1582); blk 8</td>
      <td>+0.5% / 99.5% (1590/1582); blk 7</td>
    </tr>
    <tr>
      <td><code>miscgag1</code></td>
      <td>-0.8% / 100.8% (953/961)</td>
      <td>-0.8% / 100.8% (953/961)</td>
    </tr>
    <tr>
      <td><code>miscgag2</code></td>
      <td>-0.3% / 100.3% (1352/1356)</td>
      <td>-0.3% / 100.3% (1352/1356)</td>
    </tr>
    <tr>
      <td><code>stand1</code></td>
      <td>-4.0% / 104.1% (194/202)</td>
      <td>-4.0% / 104.1% (194/202)</td>
    </tr>
    <tr>
      <td><code>stand2</code></td>
      <td>-2.0% / 102.1% (480/490)</td>
      <td>-2.0% / 102.1% (480/490)</td>
    </tr>
    <tr>
      <td><code>stand3</code></td>
      <td>-1.8% / 101.8% (547/557)</td>
      <td>-1.8% / 101.8% (547/557)</td>
    </tr>
    <tr>
      <td><code>stand4</code></td>
      <td>-1.5% / 101.5% (1202/1220)</td>
      <td>-1.2% / 101.2% (1203/1218); blk 3</td>
    </tr>
    <tr>
      <td><code>stand5</code></td>
      <td>-1.2% / 101.2% (1442/1460)</td>
      <td>-1.2% / 101.2% (1442/1460)</td>
    </tr>
    <tr>
      <td><code>stand6</code></td>
      <td>-1.3% / 101.3% (1346/1364)</td>
      <td>-1.3% / 101.3% (1346/1364)</td>
    </tr>
    <tr>
      <td><code>stand7</code></td>
      <td>-3.3% / 103.5% (520/538)</td>
      <td>-3.3% / 103.5% (520/538)</td>
    </tr>
    <tr>
      <td><code>stand8</code></td>
      <td>-3.2% / 103.3% (483/499); blk 2</td>
      <td>-3.2% / 103.3% (483/499); blk 2</td>
    </tr>
    <tr>
      <td><code>stand9</code></td>
      <td>-3.3% / 103.5% (520/538)</td>
      <td>-3.0% / 103.1% (522/538)</td>
    </tr>
    <tr>
      <td><code>stand10</code></td>
      <td>-1.9% / 101.9% (528/538)</td>
      <td>-1.9% / 101.9% (528/538)</td>
    </tr>
    <tr>
      <td><code>stand11</code></td>
      <td>-1.9% / 101.9% (528/538)</td>
      <td>-1.9% / 101.9% (528/538)</td>
    </tr>
    <tr>
      <td><code>stand12</code></td>
      <td>-0.6% / 100.6% (1450/1459); blk 1</td>
      <td>-0.7% / 100.7% (1450/1460)</td>
    </tr>
    <tr>
      <td><code>stand15</code></td>
      <td>-1.8% / 101.8% (444/452)</td>
      <td>-1.8% / 101.8% (444/452)</td>
    </tr>
    <tr>
      <td><code>stand16</code></td>
      <td>+0.2% / 99.8% (473/472)</td>
      <td>+0.2% / 99.8% (473/472)</td>
    </tr>
    <tr>
      <td><code>suzy1</code></td>
      <td>metadata-only</td>
      <td>metadata-only</td>
    </tr>
    <tr>
      <td><code>suzy2</code></td>
      <td>metadata-only</td>
      <td>metadata-only</td>
    </tr>
    <tr>
      <td><code>visitor1</code></td>
      <td>-0.7% / 100.7% (672/677)</td>
      <td>-0.7% / 100.7% (672/677)</td>
    </tr>
    <tr>
      <td><code>visitor3</code></td>
      <td>+42.9% / 70.0% (1450/1015); due 31; blk 355</td>
      <td>+43.5% / 69.7% (1452/1012); due 32; blk 361</td>
    </tr>
    <tr>
      <td><code>visitor4</code></td>
      <td>-0.9% / 100.9% (424/428)</td>
      <td>-0.9% / 100.9% (424/428)</td>
    </tr>
    <tr>
      <td><code>visitor5</code></td>
      <td>+1.9% / 98.1% (1111/1090); blk 12</td>
      <td>+2.0% / 98.0% (1112/1090); blk 12</td>
    </tr>
    <tr>
      <td><code>visitor6</code></td>
      <td>-0.2% / 100.2% (2043/2047); blk 1</td>
      <td>-0.2% / 100.2% (2043/2047); blk 1</td>
    </tr>
    <tr>
      <td><code>visitor7</code></td>
      <td>-0.4% / 100.4% (1619/1625)</td>
      <td>-0.4% / 100.4% (1619/1625)</td>
    </tr>
    <tr>
      <td><code>walkstuf1</code></td>
      <td>+13.7% / 88.0% (1595/1403); due 57; blk 278</td>
      <td>+15.5% / 86.6% (1614/1397); due 49; blk 276</td>
    </tr>
    <tr>
      <td><code>walkstuf2</code></td>
      <td>-2.2% / 102.2% (451/461)</td>
      <td>-2.2% / 102.2% (451/461)</td>
    </tr>
    <tr>
      <td><code>walkstuf3</code></td>
      <td>+1.9% / 98.1% (2321/2278); due 6; blk 68</td>
      <td>+1.1% / 98.9% (2321/2295); due 5; blk 40</td>
    </tr>
  </tbody>
</table>
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

The FISHING1 canary remains under target, but the full battle card still has
CD-heavy scenes (`visitor3`, `walkstuf1`, `building2`, `activity9`,
`building4`, `building6`). The clean-pressure relief rows prove scene-local
CD policy can recover large due-miss collapses, while the refreshed stale rows
prove current-pack baselines must be cleared before ranking fixed overhead.

Next plausible wins, in priority order:

1. **Generated read grouping or setup/data-shape work.** VISITOR3 remains the
   largest gap at `+440/+435` VBlanks, and WALKSTUF1 still has
   `blocking_vb=278/276` after the PAL4 setup-prime win, so the next CD-shape
   pass needs generated cost metadata rather than hand-authored ranges.
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
  average is now +1.0% over target / 99.3% target speed, with several worse
  CD-bound outliers; compressing the timing files would expose the same
  throughput bottleneck without fixing it.
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

- [Performance battle card]({{ '/perf/' | relative_url }}) — the
  live timing matrix this reference manual describes the columns
  of. 126 scene/tide variants, sortable, color-coded.
- [From 87 to 99.5: the post-validation performance loop]({{ '/lab/from-87-to-99-5/' | relative_url }})
  — the retrospective on the optimization arc, including which
  experiments landed and which got rejected.
- [v0.8.1: what the soak found that the matrix didn't]({{ '/lab/v081-mary4-freeze/' | relative_url }})
  — the soak-loop war story; matrix and soak are not redundant.
- [Hardware]({{ '/docs/hardware/' | relative_url }}) — what the
  optimizations are running against.
- [Build & toolchain]({{ '/docs/build/' | relative_url }}) — how the
  PS1 binary is produced.
- [Build infrastructure]({{ '/docs/infrastructure/' | relative_url }}) —
  the wrapper around the perf iterate script.
- [Audio pipeline]({{ '/docs/audio/' | relative_url }}) — the SPU side,
  which has its own scheduling concerns.
- [Story-loop walks]({{ '/docs/walks/' | relative_url }}) — the
  walk subsystem's persistent clean buffer is part of the same
  pressure-accounting envelope the matrix above measures; the v0.8.0
  clean-rect retry path and v0.8.1 wave-band/split-rect pressure
  changes are documented there.
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
