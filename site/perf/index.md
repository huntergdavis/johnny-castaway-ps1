---
title: Performance battle card
eyebrow: Headless · 126-variant matrix
subtitle: Click a column header to sort. Target Speed cells are color-coded — ≥99% green, ≥95% orange, ≥90% yellow, <90% red.
description: The Johnny Castaway PS1 fan port headless-perf battle card — 126 scene/tide variants timed against target frame budget, color-coded, with sortable column headers.
---

{% assign all_scenes      = site.data.scenes %}
{% assign validated_count = all_scenes | where: "status", "validated"  | size %}
{% assign total_count     = all_scenes | size %}

A labor of love by Hunter Davis. This page is the other ledger
that lives next to the [scene ledger]({{ '/scenes/' | relative_url }}).
The scene ledger tracks visual signoff (pixel-perfect against host
capture, signed off by human review). This page tracks
**headless-perf timing** — automated DuckStation runs that measure
`loop_vb` against `target_vb` for every scene/tide variant. They
are different bars and should not be mixed; their failure modes
are uncorrelated.

If you paid for this, you were cheated. Open source and free.

The reference manual for the perf work is at
[/docs/performance/]({{ '/docs/performance/' | relative_url }});
the retrospective on the optimization loop that moved this matrix
from the compact baseline to its current public-capped `{{ site.release.perf_target_speed_pct }}%`
target-speed average is at
[/lab/from-87-to-99-5/]({{ '/lab/from-87-to-99-5/' | relative_url }}).
A scene can be timed here without being visually certified.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

<noscript>
<p><em>Note:</em> the column-header click-to-sort affordance the
subtitle mentions requires JavaScript. Without it, the matrix
below is fully readable but cells stay in source order. The
durable numeric source is the
<a href="https://github.com/{{ site.repo }}/blob/main/docs/ps1/performance-scene-matrix.csv">CSV</a>
linked in the Rollup section.</p>
</noscript>

## At a glance

<p class="scene-perf-legend" aria-label="Target speed distribution as of {{ site.release.tag }}">
  Target Speed distribution at <code>{{ site.release.tag }}</code>:
  <span class="spd-key spd-green">111 (88.1%) ≥ 99%</span>
  <span class="spd-key spd-orange">15 (11.9%) ≥ 95%</span>
  <span class="spd-key spd-yellow">0 (0.0%) ≥ 90%</span>
  <span class="spd-key spd-red">0 (0.0%) &lt; 90%</span>
  out of 126 timing-bearing rows. Every row now contributes to speed averages.
</p>

No timing-bearing row is in the red band after the VISITOR3, BUILDING4, and
BUILDING2 restore-minus-current pack passes, the BUILDING2 high `60..72`
and low `365..381` grouped-read passes, the VISITOR3 low scoped
composite-helper pass, the WALKSTUF1 compact FGP3/v4 restore-minus-current
pass, the BUILDING1/VISITOR5 compact-FGP3 no-autoprime follow-ups, the
BUILDING2 low restore-minus-current/slack-4 pass, the WALKSTUF3 high
compact-FGP3/v4 pass, the BUILDING6 compact-FGP3/v4 pass, the ACTIVITY9 high
compact-FGP3/v4 pass, the WALKSTUF3 low compact-FGP3/v4 pass, the JOHNNY1
compact-FGP3/v4 pass, the ACTIVITY9 low compact-FGP3/v4 pass, and the
VISITOR3 motion-copy/code-headroom/CD-pressure passes plus the low/high
persistent setup-segment, high frame-126/frame-125 re-anchor passes, the
high setup-prime cap expansion, the guarded low second setup segment, and the
low frame-125/frame-126 resident re-anchor plus frame-118 and frame-127
resident copies, high frame-127/frame-130 resident-copy compaction, and low
frame-114/frame-117 plus frame-113 no-op residual compaction. The
current VISITOR3 baseline is high `1075/1037` after v238
and low `1075/1039` after v249; high cuts overrun `54 -> 38`, blocking
`83 -> 59`, loop reads `15 -> 11`, loop-read time `83 -> 59`, and due misses
`15 -> 11`, while the v249 low follow-up cuts overrun `51 -> 36`, blocking
`93 -> 69`, loop reads `17 -> 16`, loop-read time `100 -> 83`, and due misses
`16 -> 12` with hidden refill still `0`.
Both paths keep fixed pack layout with deliberate setup tradeoffs.
The yellow band is now empty; the orange band (95–99%) holds WALKSTUF1
high/low (`95.5%` / `95.8%`), VISITOR3 high/low
(`96.5%` / `96.7%`), BUILDING2 high/low, VISITOR5 high/low, JOHNNY1 high/low,
BUILDING4 low, BUILDING6 high/low, JOHNNY6 high/low, and the
remaining wide-action
rows still finishing scheduler-owned read timing and selective-preprocessing
work. MARY3 high/low moved into green after the guarded prefetch-preserve pass,
BUILDING1 high/low moved into green after the compact-FGP3/no-autoprime pass,
WALKSTUF3 high moved into green after the compact-FGP3/v4 pass, and ACTIVITY9
high, WALKSTUF3 low, and ACTIVITY9 low moved into green after their
compact-FGP3/v4 restore-minus-current passes. The latest rejected VISITOR3
v183-v212 probes close low precursor motion-copy frames `114..118`, the C-side
fastspan path, terminal zero/origin trimming, low hull-motion retries, terminal
hand-authored read groups, simple motion row-copy runtime paths, compact
motion-copy metadata, frame `117` sparse hull in both/low-only forms, generic
narrow dirty-row upload, naive compact-v4 motion-marker dispatch, frame `125`
re-anchor, low frame `127` re-anchor, frame `116`/`114` copy-only hull shapes,
and low setup-prime caps above `208 KiB`; the remaining
VISITOR3 work is now custom data-shape, dictionary, pack-authored
precomposed-strip, or generated scheduler work.

All 126 rows now carry active-loop timing. [`SUZY 1`]({{ '/scenes/suzy1/' | relative_url }})
needs a longer `12000`-frame matrix budget because its valid
[scene-end]({{ '/docs/glossary/#scene-end' | relative_url }}) lands after the
default `7200`-frame timing window.

## Rollup

Current battle-card rollup as of <time datetime="2026-05-09">2026-05-09</time>:

| Metric | Value |
|---|---:|
| Scenes visually validated | `{{ validated_count }} / {{ total_count }}` (`100%`) |
| Validated scenes | all 63 original scenes; see the [live ledger]({{ '/scenes/' | relative_url }}) for the source rows |
| Scene/tide variants routed through headless perf | `126 / 126` (`100%`) |
| Timing-bearing variants | `126 / 126` (`100%`) |
| Scenes with at least one active-loop timed variant | `63 / 63` (`100%`) |
| Scenes with both high/low variants measured | `63 / 63` (`100%`) |
| Pending variants | `0 / 126` (`0%`) |
| Blocked variants | `0 / 126` (`0%`) |
| Timing-bearing average over target | `+0.4%` (`0.3459%` exact, public-capped) |
| Timing-bearing average target speed | `99.7%` (`99.6621%` exact, public-capped) |
| Latest perf matrix run | `2026-05-09T18:08:05` |
| Stats version | mixed across rows; newest optimized/code-headroom rows use `visitor3-low-noop113-v249`, `visitor3-low-noop114117-v248`, `visitor3-high-f127-f130-resident-copy-v238`, `activity9-low-compact-fgp3-v174`, `johnny1-compact-fgp3-v173`, `walkstuf3-low-compact-fgp3-v171`, `activity9-high-compact-fgp3-v167`, `building6-compact-fgp3-v165`, `walkstuf3-high-compact-fgp3-v163`, `building2-low-restore-window-slack4-v160`, `visitor5-high-compact-fgp3-noautoprime-v158`, `building1-compact-fgp3-noautoprime-v157`, `mary3-preserve-window-slack8-v149`, `missing-scenes-current-v001`, `visitor3-tail-trim-stageguard-v127`, `graphics-composite-os-v111`, `building2-low-group365-381-v110`, `building2-high-group60-72-v109`, `building2-high-restore-minus-current-v108`, `visitor3-low-offscreen-exitright-v106`, `visitor3-high-offscreen-drawclip-v105`, `walkstuf1-compact-fgp3-v141`, `visitor3-low-readgroup-prune-v088`, `building4-restore-minus-current-v087`, `visitor3-restore-minus-current-v086`, `visitor3-high-readgroup-prune-v084`, `compact-u16-inline-v083`, `fgp3v4-drawcompact-all-v082`, `activity9-dead-readgroup-prune-v082`, `read-group-selector-single-assign-v082`, `johnny2-prefetch-relief-v081`, `mary2-prefetch-relief-v081`, `mary5-fgp3-padded-v081`, `activity11-fgp3-padded-v081`, `building5-fgp3-padded-v080`, `walkstuf1-fgp2-setup-prime-v080`, `activity4-fishing4-v072c-prefetch-relief`, `building4-6-johnny6-v072c-prefetch-relief`, `activity1-v072c-current-refresh`, `activity11-12-v072c-prefetch-relief`, `stale-next-v072c-current-refresh`, `mary1-v072c-prefetch-relief`, `stale-layout-v072c-current-refresh`, `stale-pressure2-v072c-current-refresh`, and earlier matrix refresh versions. Per-row version is in the [`Stats Version` column below](#reading-the-table) and the [enumeration](#reading-the-table) is in the table-key section. |
| FISHING 1 canary | `1068 / 1074 VBlanks`, `0.0%` public over target, `100.0%` public target speed, `blocking_vb=2` |

The durable numeric source is
[`docs/ps1/performance-scene-matrix.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-scene-matrix.csv).
The experiment trail is
[`docs/ps1/performance-experiment-log.md`]({{ site.github_url }}/blob/main/docs/ps1/performance-experiment-log.md).
The pack-time preprocessing target sheet is
[`docs/ps1/performance-preprocess-opportunities.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-preprocess-opportunities.csv).
The human signoff ledger is
[`docs/ps1/scene-status.md`]({{ site.github_url }}/blob/main/docs/ps1/scene-status.md).

Maintenance rule: every accepted perf optimization, rejected experiment worth
preserving, or new scene/tide timing run must update the CSV, the experiment
log, [README.md]({{ site.github_url }}/blob/main/README.md),
[`docs/ps1/TESTING.md`]({{ site.github_url }}/blob/main/docs/ps1/TESTING.md),
and this page.

## Reading the table

- **Scene**: clicking the scene slug in any row updates the URL with
  `#perf-{slug}-{tide}` so you can copy a stable link to that exact
  row — useful when filing a follow-up against one variant. Same
  anchors are linked from each scene page's "Performance battle card"
  line.
- **[Over Target]({{ '/docs/glossary/#over-target' | relative_url }})**: how far `loop_vb` is above the captured target timing. Public site values are capped at `0.0%` for faster-than-target rows so fast scenes do not create negative debt. Lower is better.
- **[Target Speed]({{ '/docs/glossary/#target-speed' | relative_url }})**: public display of `target_vb / loop_vb`, capped at `100%` so no row reports faster than intended playback. The raw signed ratio remains in the CSV for optimization analysis.
- **[VBlanks]({{ '/docs/glossary/#vblank' | relative_url }})**: `loop_vb/target_vb` — see the
  [target_vb / loop_vb glossary entry]({{ '/docs/glossary/#target-vb' | relative_url }}).
- **[Blocking]({{ '/docs/glossary/#blocking-vb' | relative_url }})**: visible CD/blocking VBlanks.
- **[Prefetch]({{ '/docs/glossary/#prefetch-hits' | relative_url }})**: prefetch overrun VBlanks.
- **Due**: due-frame misses.
- **Latest Run**: ISO timestamp derived from the headless summary path
  (`scratch/ps1-perf-iterate/YYYYMMDD-HHMMSS`); `-` means no current
  matrix run has been recorded for that variant.
- **Stats Version**: performance/layout version for that row. The latest
  refreshed rows use `visitor3-low-noop113-v249`,
  `visitor3-low-noop114117-v248`,
  `visitor3-high-f127-f130-resident-copy-v238`,
  `johnny1-compact-fgp3-v173`,
  `walkstuf3-low-compact-fgp3-v171`,
  `activity9-high-compact-fgp3-v167`,
  `building6-compact-fgp3-v165`,
  `walkstuf3-high-compact-fgp3-v163`,
  `building2-low-restore-window-slack4-v160`,
  `visitor5-high-compact-fgp3-noautoprime-v158`,
  `building1-compact-fgp3-noautoprime-v157`,
  `mary3-preserve-window-slack8-v149`,
  `missing-scenes-current-v001`,
  `visitor3-tail-trim-stageguard-v127`,
  `graphics-composite-os-v111`,
  `building2-low-group365-381-v110`,
  `building2-high-group60-72-v109`,
  `building2-high-restore-minus-current-v108`,
  `visitor3-low-offscreen-exitright-v106`,
  `visitor3-high-offscreen-drawclip-v105`,
  `walkstuf1-compact-fgp3-v141`,
  `visitor3-low-readgroup-prune-v088`,
  `building4-restore-minus-current-v087`,
  `visitor3-restore-minus-current-v086`,
  `visitor3-high-readgroup-prune-v084`,
  `compact-u16-inline-v083`,
  `fgp3v4-drawcompact-all-v082`,
  `activity9-dead-readgroup-prune-v082`,
  `read-group-selector-single-assign-v082`,
  `visitor3-high-remove-144-160-v082`,
  `johnny2-prefetch-relief-v081`,
  `activity9-low-fgp3-cleanup-compact-v081`,
  `activity9-current-v081-refresh`,
  `building4-fgp3-cleanup-compact-window-v081`,
  `building2-fgp3-cleanup-compact-v081`,
  `visitor3-fgp3-cleanup-compact-v081`,
  `activity9-lowgroup-v072c`,
  `activity9-fgp3-v072c`,
  `activity9-window-v072c`,
  `activity4-fishing4-v072c-prefetch-relief`,
  `building4-6-johnny6-v072c-prefetch-relief`,
  `activity1-v072c-current-refresh`,
  `mary2-prefetch-relief-v081`,
  `mary2-fgp3-padded-v081`,
  `johnny2-fgp3-padded-v081`,
  `mary5-fgp3-padded-v081`,
  `activity11-fgp3-padded-v081`,
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
  `walkstuf1-v072-prefetch-relief`,
  `fishing5-v065-current-ledger-overlay`,
  `compact-fgp3-v66-final-frame-hold`,
  `compact-fgp3-v64-building2-group318-330`,
  `compact-fgp3-v63-building2low-prime`, and
  `indexed8-row-local-dirty-v1`; other refreshed rows
  include `compact-fgp3-v62-fishing3low-group253-265`,
  `compact-fgp3-v61-fishing3low-group163-175`,
  `compact-fgp3-v60-visitor3high-group230-242`,
  `compact-fgp3-v59-visitor3high-group72-84`,
  `indexed8-tile-local-compose-v1`,
  `compact-fgp3-v58-activity9high-window20-table`, `compact-fgp3-v57-policy-table-refactor`, and `compact-fgp3-v49-walkstuf2-auto-prime` through
  `compact-fgp3-v29-smallprime`; the complete full-matrix baseline remains
  `compact-fgp3-v2-fullmatrix` for rows not rerun since that pass.

## 126-variant battle card

<p class="scene-perf-legend" aria-label="Target speed legend">
  Target Speed colour key:
  <span class="spd-key spd-green">≥ 99% (at target)</span>
  <span class="spd-key spd-orange">≥ 95% (close)</span>
  <span class="spd-key spd-yellow">≥ 90% (slipping)</span>
  <span class="spd-key spd-red">&lt; 90% (well below)</span>
  Rows without timing data show "—" and stay uncolored.
</p>

<table class="scene-perf-table">
  <caption class="visually-hidden">
    Headless performance matrix at {{ site.release.tag }}:
    one row per scene/tide variant, with last run, stats version,
    over target percentage, target speed percentage, VBlanks taken,
    blocking VBlanks, prefetch hits, due-misses, and notes.
    Click any column header to sort ascending; click again for
    descending. Target Speed cells are color-coded: ≥99% green,
    ≥95% orange, ≥90% yellow, &lt;90% red.
  </caption>
  <thead>
    <tr>
      <th scope="col">Scene</th>
      <th scope="col">Tide</th>
      <th scope="col">Status</th>
      <th scope="col">Latest Run</th>
      <th scope="col">Stats Version</th>
      <th scope="col">Over Target</th>
      <th scope="col">Target Speed</th>
      <th scope="col">VBlanks</th>
      <th scope="col">Blocking</th>
      <th scope="col">Prefetch</th>
      <th scope="col">Due</th>
      <th scope="col">Notes</th>
    </tr>
  </thead>
  <tbody>
    <tr id="perf-activity1-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity1-high"><code>activity1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T04:01:15</td>
      <td>activity1-v072c-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>2754/2764</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-activity1-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity1-low"><code>activity1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T04:01:15</td>
      <td>activity1-v072c-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>2754/2765</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-activity4-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity4-high"><code>activity4</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T04:41:05</td>
      <td>activity4-fishing4-v072c-prefetch-relief</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1065/1066</td>
      <td>4</td>
      <td>4</td>
      <td>0</td>
      <td>clean-snapshot relief restores stage1_window prefetch versus fresh ACTIVITY4 current row; accepted hidden-refill tradeoff remains</td>
    </tr>
    <tr id="perf-activity4-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity4-low"><code>activity4</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T04:41:05</td>
      <td>activity4-fishing4-v072c-prefetch-relief</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1064/1068</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td>clean-snapshot relief collapses ACTIVITY4 low due misses and restores stage1_window prefetch</td>
    </tr>
    <tr id="perf-activity5-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity5-high"><code>activity5</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T02:48:00</td>
      <td>stale-layout-v072c-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1730/1749</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-activity5-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity5-low"><code>activity5</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T02:48:00</td>
      <td>stale-layout-v072c-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1731/1749</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-activity6-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity6-high"><code>activity6</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T00:55:09</td>
      <td>stale-zero-v072b-current-refresh</td>
      <td>+0.1%</td>
      <td class="spd-green">99.9%</td>
      <td>912/911</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-activity6-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity6-low"><code>activity6</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T00:55:09</td>
      <td>stale-zero-v072b-current-refresh</td>
      <td>+0.1%</td>
      <td class="spd-green">99.9%</td>
      <td>912/911</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-activity7-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity7-high"><code>activity7</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:47:29</td>
      <td>mismatch-top-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>593/596</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-activity7-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity7-low"><code>activity7</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:47:29</td>
      <td>mismatch-top-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>594/596</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-activity8-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity8-high"><code>activity8</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:47:29</td>
      <td>mismatch-top-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>898/904</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-activity8-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity8-low"><code>activity8</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:47:29</td>
      <td>mismatch-top-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>899/904</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-activity9-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity9-high"><code>activity9</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-08T18:43:51</td>
      <td>activity9-low-compact-fgp3-v174</td>
      <td>+1.0%</td>
      <td class="spd-green">99.0%</td>
      <td>2082/2062</td>
      <td>24</td>
      <td>17</td>
      <td>1</td>
      <td>Compact FGP3/v4 restore-minus-current high-tide pack; preserves footprint, LBA, and PS-EXE bucket while low tide now has its own compact pass</td>
    </tr>
    <tr id="perf-activity9-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity9-low"><code>activity9</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-09T18:08:05</td>
      <td>visitor3-low-noop113-v249</td>
      <td>+0.7%</td>
      <td class="spd-green">99.3%</td>
      <td>2075/2061</td>
      <td>17</td>
      <td>12</td>
      <td>1</td>
      <td>Compact FGP3/v4 restore-minus-current low-tide pack; revalidated flat under the v249 VISITOR3 broad no-regression gate</td>
    </tr>
    <tr id="perf-activity10-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity10-high"><code>activity10</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T01:47:28</td>
      <td>activity10-johnny3-v072-prefetch-relief</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1259/1259</td>
      <td>7</td>
      <td>4</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-activity10-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity10-low"><code>activity10</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:47:28</td>
      <td>activity10-johnny3-v072-prefetch-relief</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1255/1256</td>
      <td>17</td>
      <td>4</td>
      <td>2</td>
      <td></td>
    </tr>
    <tr id="perf-activity11-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity11-high"><code>activity11</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T20:36:09</td>
      <td>activity11-fgp3-padded-v081</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1715/1722</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td>padded FGP3 active-loop win; overrun eliminated with accepted setup-prime cost</td>
    </tr>
    <tr id="perf-activity11-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity11-low"><code>activity11</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T20:36:09</td>
      <td>activity11-fgp3-padded-v081</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1717/1722</td>
      <td>4</td>
      <td>4</td>
      <td>0</td>
      <td>padded FGP3 active-loop win; overrun eliminated with accepted setup-prime cost</td>
    </tr>
    <tr id="perf-activity12-high">
      <td><a class="scene-perf-rowlink" href="#perf-activity12-high"><code>activity12</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T03:40:30</td>
      <td>activity11-12-v072c-prefetch-relief</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1411/1412</td>
      <td>7</td>
      <td>7</td>
      <td>0</td>
      <td>validated pack clean-snapshot relief exception keeps stage1_window prefetch; current high tide is baseline clean</td>
    </tr>
    <tr id="perf-activity12-low">
      <td><a class="scene-perf-rowlink" href="#perf-activity12-low"><code>activity12</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T03:40:30</td>
      <td>activity11-12-v072c-prefetch-relief</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1409/1411</td>
      <td>10</td>
      <td>6</td>
      <td>1</td>
      <td>validated pack clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff</td>
    </tr>
    <tr id="perf-building1-high">
      <td><a class="scene-perf-rowlink" href="#perf-building1-high"><code>building1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-08T12:22:52</td>
      <td>building1-compact-fgp3-noautoprime-v157</td>
      <td>+0.3%</td>
      <td class="spd-green">99.7%</td>
      <td>784/782</td>
      <td>15</td>
      <td>9</td>
      <td>1</td>
      <td>compact FGP3/v4 pack with BUILDING1 auto-prime disabled</td>
    </tr>
    <tr id="perf-building1-low">
      <td><a class="scene-perf-rowlink" href="#perf-building1-low"><code>building1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-08T12:22:52</td>
      <td>building1-compact-fgp3-noautoprime-v157</td>
      <td>+0.6%</td>
      <td class="spd-green">99.4%</td>
      <td>787/782</td>
      <td>16</td>
      <td>14</td>
      <td>1</td>
      <td>compact FGP3/v4 pack with BUILDING1 auto-prime disabled</td>
    </tr>
    <tr id="perf-building2-high">
      <td><a class="scene-perf-rowlink" href="#perf-building2-high"><code>building2</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-09T18:08:05</td>
      <td>visitor3-low-noop113-v249</td>
      <td>+2.5%</td>
      <td class="spd-orange">97.6%</td>
      <td>1349/1316</td>
      <td>48</td>
      <td>12</td>
      <td>7</td>
      <td>revalidated flat under the v249 VISITOR3 broad no-regression gate</td>
    </tr>
    <tr id="perf-building2-low">
      <td><a class="scene-perf-rowlink" href="#perf-building2-low"><code>building2</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-09T18:08:05</td>
      <td>visitor3-low-noop113-v249</td>
      <td>+2.5%</td>
      <td class="spd-orange">97.6%</td>
      <td>1349/1316</td>
      <td>83</td>
      <td>1</td>
      <td>19</td>
      <td>restore-minus-current low pack plus 4-VBlank slack guard; revalidated flat under the v249 VISITOR3 broad no-regression gate</td>
    </tr>
    <tr id="perf-building3-high">
      <td><a class="scene-perf-rowlink" href="#perf-building3-high"><code>building3</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T03:26:10</td>
      <td>stale-next-v072c-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>5460/5465</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-building3-low">
      <td><a class="scene-perf-rowlink" href="#perf-building3-low"><code>building3</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T03:26:10</td>
      <td>stale-next-v072c-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>5460/5465</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-building4-high">
      <td><a class="scene-perf-rowlink" href="#perf-building4-high"><code>building4</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-08T18:43:51</td>
      <td>activity9-low-compact-fgp3-v174</td>
      <td>+1.0%</td>
      <td class="spd-orange">99.0%</td>
      <td>2844/2816</td>
      <td>37</td>
      <td>30</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-building4-low">
      <td><a class="scene-perf-rowlink" href="#perf-building4-low"><code>building4</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-08T18:43:51</td>
      <td>activity9-low-compact-fgp3-v174</td>
      <td>+1.4%</td>
      <td class="spd-orange">98.6%</td>
      <td>2855/2815</td>
      <td>46</td>
      <td>38</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-building5-high">
      <td><a class="scene-perf-rowlink" href="#perf-building5-high"><code>building5</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T17:38:07</td>
      <td>building5-fgp3-padded-v080</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>3343/3348</td>
      <td>5</td>
      <td>5</td>
      <td>0</td>
      <td>padded pal4 FGP3 temporal-residual conversion keeps the original 818670-byte CD footprint while shrinking runtime payload to 592755 bytes</td>
    </tr>
    <tr id="perf-building5-low">
      <td><a class="scene-perf-rowlink" href="#perf-building5-low"><code>building5</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T17:38:07</td>
      <td>building5-fgp3-padded-v080</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>3345/3347</td>
      <td>8</td>
      <td>8</td>
      <td>0</td>
      <td>padded pal4 FGP3 temporal-residual conversion keeps the original 818670-byte CD footprint while shrinking runtime payload to 592755 bytes</td>
    </tr>
    <tr id="perf-building6-high">
      <td><a class="scene-perf-rowlink" href="#perf-building6-high"><code>building6</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-08T15:26:52</td>
      <td>building6-compact-fgp3-v165</td>
      <td>+1.0%</td>
      <td class="spd-green">99.0%</td>
      <td>2482/2457</td>
      <td>25</td>
      <td>27</td>
      <td>0</td>
      <td>compact FGP3/v4 restore-minus-current pack; fixed footprint and LBA under current layout</td>
    </tr>
    <tr id="perf-building6-low">
      <td><a class="scene-perf-rowlink" href="#perf-building6-low"><code>building6</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-08T15:26:52</td>
      <td>building6-compact-fgp3-v165</td>
      <td>+1.2%</td>
      <td class="spd-orange">98.8%</td>
      <td>2485/2456</td>
      <td>28</td>
      <td>29</td>
      <td>0</td>
      <td>compact FGP3/v4 restore-minus-current pack; fixed footprint and LBA under current layout</td>
    </tr>
    <tr id="perf-building7-high">
      <td><a class="scene-perf-rowlink" href="#perf-building7-high"><code>building7</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T02:48:00</td>
      <td>stale-layout-v072c-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>3132/3133</td>
      <td>9</td>
      <td>9</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-building7-low">
      <td><a class="scene-perf-rowlink" href="#perf-building7-low"><code>building7</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T02:48:00</td>
      <td>stale-layout-v072c-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>3130/3133</td>
      <td>7</td>
      <td>7</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-fishing1-high">
      <td><a class="scene-perf-rowlink" href="#perf-fishing1-high"><code>fishing1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-09T18:08:05</td>
      <td>visitor3-low-noop113-v249</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1068/1074</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td>canary revalidated flat under the v249 VISITOR3 broad no-regression gate</td>
    </tr>
    <tr id="perf-fishing1-low">
      <td><a class="scene-perf-rowlink" href="#perf-fishing1-low"><code>fishing1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1067/1074</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-fishing2-high">
      <td><a class="scene-perf-rowlink" href="#perf-fishing2-high"><code>fishing2</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T03:26:10</td>
      <td>stale-next-v072c-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1761/1763</td>
      <td>6</td>
      <td>6</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-fishing2-low">
      <td><a class="scene-perf-rowlink" href="#perf-fishing2-low"><code>fishing2</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T03:26:10</td>
      <td>stale-next-v072c-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1759/1765</td>
      <td>3</td>
      <td>3</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-fishing3-high">
      <td><a class="scene-perf-rowlink" href="#perf-fishing3-high"><code>fishing3</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-07T13:24:41</td>
      <td>activity9-dead-readgroup-prune-v082</td>
      <td>+0.6%</td>
      <td class="spd-green">99.4%</td>
      <td>1962/1950</td>
      <td>17</td>
      <td>14</td>
      <td>1</td>
      <td>Exact-flat after pruning dead ACTIVITY9 low FGP3/v1 read-group selector; preserves accepted timing/LBAs and shrinks foregroundPilotPlay by 16 bytes</td>
    </tr>
    <tr id="perf-fishing3-low">
      <td><a class="scene-perf-rowlink" href="#perf-fishing3-low"><code>fishing3</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-07T13:24:41</td>
      <td>activity9-dead-readgroup-prune-v082</td>
      <td>+0.1%</td>
      <td class="spd-green">99.9%</td>
      <td>1957/1955</td>
      <td>9</td>
      <td>9</td>
      <td>0</td>
      <td>Exact-flat after pruning dead ACTIVITY9 low FGP3/v1 read-group selector; preserves accepted timing/LBAs and shrinks foregroundPilotPlay by 16 bytes</td>
    </tr>
    <tr id="perf-fishing4-high">
      <td><a class="scene-perf-rowlink" href="#perf-fishing4-high"><code>fishing4</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T04:41:05</td>
      <td>activity4-fishing4-v072c-prefetch-relief</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>835/842</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td>clean-snapshot relief keeps FISHING4 high under target with stage1_window prefetch restored</td>
    </tr>
    <tr id="perf-fishing4-low">
      <td><a class="scene-perf-rowlink" href="#perf-fishing4-low"><code>fishing4</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T04:41:05</td>
      <td>activity4-fishing4-v072c-prefetch-relief</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>834/843</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>clean-snapshot relief collapses FISHING4 low due misses and makes the row CD-clean</td>
    </tr>
    <tr id="perf-fishing5-high">
      <td><a class="scene-perf-rowlink" href="#perf-fishing5-high"><code>fishing5</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-02T22:39:34</td>
      <td>fishing5-v065-current-ledger-overlay</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>885/890</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-fishing5-low">
      <td><a class="scene-perf-rowlink" href="#perf-fishing5-low"><code>fishing5</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-02T22:39:34</td>
      <td>fishing5-v065-current-ledger-overlay</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>885/890</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-fishing6-high">
      <td><a class="scene-perf-rowlink" href="#perf-fishing6-high"><code>fishing6</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T00:38:12</td>
      <td>stale-top-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>744/753</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-fishing6-low">
      <td><a class="scene-perf-rowlink" href="#perf-fishing6-low"><code>fishing6</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T00:38:12</td>
      <td>stale-top-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>744/753</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-fishing7-high">
      <td><a class="scene-perf-rowlink" href="#perf-fishing7-high"><code>fishing7</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:47:29</td>
      <td>mismatch-top-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>715/725</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-fishing7-low">
      <td><a class="scene-perf-rowlink" href="#perf-fishing7-low"><code>fishing7</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:47:29</td>
      <td>mismatch-top-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>715/725</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-fishing8-high">
      <td><a class="scene-perf-rowlink" href="#perf-fishing8-high"><code>fishing8</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1243/1253</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-fishing8-low">
      <td><a class="scene-perf-rowlink" href="#perf-fishing8-low"><code>fishing8</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1243/1253</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-johnny1-high">
      <td><a class="scene-perf-rowlink" href="#perf-johnny1-high"><code>johnny1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-09T18:08:05</td>
      <td>visitor3-low-noop113-v249</td>
      <td>+1.5%</td>
      <td class="spd-orange">98.5%</td>
      <td>1974/1945</td>
      <td>26</td>
      <td>26</td>
      <td>0</td>
      <td>compact-FGP3/v4 profile revalidated flat under the v249 VISITOR3 broad no-regression gate</td>
    </tr>
    <tr id="perf-johnny1-low">
      <td><a class="scene-perf-rowlink" href="#perf-johnny1-low"><code>johnny1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-08T17:54:07</td>
      <td>johnny1-compact-fgp3-v173</td>
      <td>+1.5%</td>
      <td class="spd-orange">98.5%</td>
      <td>1974/1945</td>
      <td>26</td>
      <td>26</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-johnny2-high">
      <td><a class="scene-perf-rowlink" href="#perf-johnny2-high"><code>johnny2</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-07T04:18:16</td>
      <td>johnny2-prefetch-relief-v081</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1741/1751</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>clean-pressure prefetch relief preserves stage1_window; blocking 369-&gt;0 due misses 144-&gt;0 loop reads 144-&gt;8</td>
    </tr>
    <tr id="perf-johnny2-low">
      <td><a class="scene-perf-rowlink" href="#perf-johnny2-low"><code>johnny2</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-07T04:18:16</td>
      <td>johnny2-prefetch-relief-v081</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1741/1751</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>clean-pressure prefetch relief preserves stage1_window; blocking 377-&gt;0 due misses 144-&gt;0 loop reads 144-&gt;8</td>
    </tr>
    <tr id="perf-johnny3-high">
      <td><a class="scene-perf-rowlink" href="#perf-johnny3-high"><code>johnny3</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T01:47:28</td>
      <td>activity10-johnny3-v072-prefetch-relief</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1158/1161</td>
      <td>10</td>
      <td>6</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr id="perf-johnny3-low">
      <td><a class="scene-perf-rowlink" href="#perf-johnny3-low"><code>johnny3</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:47:28</td>
      <td>activity10-johnny3-v072-prefetch-relief</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1157/1166</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-johnny4-high">
      <td><a class="scene-perf-rowlink" href="#perf-johnny4-high"><code>johnny4</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1204/1214</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-johnny4-low">
      <td><a class="scene-perf-rowlink" href="#perf-johnny4-low"><code>johnny4</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1204/1214</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-johnny5-high">
      <td><a class="scene-perf-rowlink" href="#perf-johnny5-high"><code>johnny5</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T00:55:09</td>
      <td>stale-zero-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>811/820</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-johnny5-low">
      <td><a class="scene-perf-rowlink" href="#perf-johnny5-low"><code>johnny5</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T00:55:09</td>
      <td>stale-zero-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>810/820</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-johnny6-high">
      <td><a class="scene-perf-rowlink" href="#perf-johnny6-high"><code>johnny6</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T04:15:06</td>
      <td>building4-6-johnny6-v072c-prefetch-relief</td>
      <td>+1.1%</td>
      <td class="spd-orange">98.9%</td>
      <td>2832/2800</td>
      <td>28</td>
      <td>28</td>
      <td>0</td>
      <td>clean-snapshot relief restores stage1_window prefetch versus fresh current row</td>
    </tr>
    <tr id="perf-johnny6-low">
      <td><a class="scene-perf-rowlink" href="#perf-johnny6-low"><code>johnny6</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T04:15:06</td>
      <td>building4-6-johnny6-v072c-prefetch-relief</td>
      <td>+1.1%</td>
      <td class="spd-orange">98.9%</td>
      <td>2832/2800</td>
      <td>28</td>
      <td>28</td>
      <td>0</td>
      <td>clean-snapshot relief restores stage1_window prefetch versus fresh current row</td>
    </tr>
    <tr id="perf-mary1-high">
      <td><a class="scene-perf-rowlink" href="#perf-mary1-high"><code>mary1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T03:05:20</td>
      <td>mary1-v072c-prefetch-relief</td>
      <td>+0.8%</td>
      <td class="spd-green">99.2%</td>
      <td>4867/4830</td>
      <td>47</td>
      <td>37</td>
      <td>2</td>
      <td>mary1 clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff</td>
    </tr>
    <tr id="perf-mary1-low">
      <td><a class="scene-perf-rowlink" href="#perf-mary1-low"><code>mary1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T03:05:20</td>
      <td>mary1-v072c-prefetch-relief</td>
      <td>+0.4%</td>
      <td class="spd-green">99.6%</td>
      <td>4860/4840</td>
      <td>31</td>
      <td>24</td>
      <td>1</td>
      <td>mary1 clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff</td>
    </tr>
    <tr id="perf-mary2-high">
      <td><a class="scene-perf-rowlink" href="#perf-mary2-high"><code>mary2</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T22:29:02</td>
      <td>mary2-prefetch-relief-v081</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>2241/2248</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td>mary2 clean-memory relief restores stage1_window after padded FGP3; blocking 668-&gt;2 and due 233-&gt;0</td>
    </tr>
    <tr id="perf-mary2-low">
      <td><a class="scene-perf-rowlink" href="#perf-mary2-low"><code>mary2</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T22:29:02</td>
      <td>mary2-prefetch-relief-v081</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>2242/2250</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td>mary2 clean-memory relief restores stage1_window after padded FGP3; blocking 662-&gt;2 and due 233-&gt;0</td>
    </tr>
    <tr id="perf-mary3-high">
      <td><a class="scene-perf-rowlink" href="#perf-mary3-high"><code>mary3</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-08T11:12:57</td>
      <td>mary3-preserve-window-slack8-v149</td>
      <td>+0.1%</td>
      <td class="spd-green">99.9%</td>
      <td>2296/2294</td>
      <td>53</td>
      <td>0</td>
      <td>13</td>
      <td>MARY3 prefetch preserved with 8-VBlank window guard</td>
    </tr>
    <tr id="perf-mary3-low">
      <td><a class="scene-perf-rowlink" href="#perf-mary3-low"><code>mary3</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-08T11:12:57</td>
      <td>mary3-preserve-window-slack8-v149</td>
      <td>+0.1%</td>
      <td class="spd-green">99.9%</td>
      <td>2297/2295</td>
      <td>51</td>
      <td>0</td>
      <td>13</td>
      <td>MARY3 prefetch preserved with 8-VBlank window guard</td>
    </tr>
    <tr id="perf-mary4-high">
      <td><a class="scene-perf-rowlink" href="#perf-mary4-high"><code>mary4</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-04-29T17:46:07</td>
      <td>compact-fgp3-v2-fullmatrix</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1968/2016</td>
      <td>28</td>
      <td>12</td>
      <td>3</td>
      <td>validated 2026-05-03 after generic multi-view stitch; active timing predates refreshed pack; far-right x=300 visual stress passed</td>
    </tr>
    <tr id="perf-mary4-low">
      <td><a class="scene-perf-rowlink" href="#perf-mary4-low"><code>mary4</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-04-29T17:46:13</td>
      <td>compact-fgp3-v2-fullmatrix</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1966/2019</td>
      <td>24</td>
      <td>10</td>
      <td>3</td>
      <td>validated 2026-05-03 after generic multi-view stitch; active timing predates refreshed pack; far-right x=300 visual stress passed</td>
    </tr>
    <tr id="perf-mary5-high">
      <td><a class="scene-perf-rowlink" href="#perf-mary5-high"><code>mary5</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T21:07:37</td>
      <td>mary5-fgp3-padded-v081</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1581/1586</td>
      <td>5</td>
      <td>0</td>
      <td>1</td>
      <td>padded FGP3 active-loop win; overrun eliminated while preserving the 646602-byte CD footprint</td>
    </tr>
    <tr id="perf-mary5-low">
      <td><a class="scene-perf-rowlink" href="#perf-mary5-low"><code>mary5</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T21:07:37</td>
      <td>mary5-fgp3-padded-v081</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1581/1584</td>
      <td>6</td>
      <td>2</td>
      <td>1</td>
      <td>padded FGP3 active-loop win; overrun eliminated while preserving the 646602-byte CD footprint</td>
    </tr>
    <tr id="perf-miscgag1-high">
      <td><a class="scene-perf-rowlink" href="#perf-miscgag1-high"><code>miscgag1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T00:55:09</td>
      <td>stale-zero-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>953/961</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-miscgag1-low">
      <td><a class="scene-perf-rowlink" href="#perf-miscgag1-low"><code>miscgag1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T00:55:09</td>
      <td>stale-zero-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>953/961</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-miscgag2-high">
      <td><a class="scene-perf-rowlink" href="#perf-miscgag2-high"><code>miscgag2</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-04-30T06:58:15</td>
      <td>compact-fgp3-v31-auto224</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1352/1356</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>validated 2026-05-03 after generic multi-view stitch regenerated high/low packs; active timing predates refreshed pack</td>
    </tr>
    <tr id="perf-miscgag2-low">
      <td><a class="scene-perf-rowlink" href="#perf-miscgag2-low"><code>miscgag2</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-04-30T06:58:15</td>
      <td>compact-fgp3-v31-auto224</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1352/1356</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>validated 2026-05-03 after generic multi-view stitch regenerated high/low packs; active timing predates refreshed pack</td>
    </tr>
    <tr id="perf-stand1-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand1-high"><code>stand1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:17:25</td>
      <td>stand1-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>194/202</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand1-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand1-low"><code>stand1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:17:25</td>
      <td>stand1-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>194/202</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand2-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand2-high"><code>stand2</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>480/490</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand2-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand2-low"><code>stand2</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>480/490</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand3-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand3-high"><code>stand3</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T00:38:12</td>
      <td>stale-top-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>547/557</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-stand3-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand3-low"><code>stand3</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T00:38:12</td>
      <td>stale-top-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>547/557</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-stand4-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand4-high"><code>stand4</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1202/1220</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-stand4-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand4-low"><code>stand4</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1203/1218</td>
      <td>3</td>
      <td>3</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-stand5-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand5-high"><code>stand5</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1442/1460</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-stand5-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand5-low"><code>stand5</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1442/1460</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-stand6-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand6-high"><code>stand6</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1346/1364</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-stand6-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand6-low"><code>stand6</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1346/1364</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-stand7-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand7-high"><code>stand7</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>520/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand7-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand7-low"><code>stand7</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>520/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand8-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand8-high"><code>stand8</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>483/499</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand8-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand8-low"><code>stand8</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>483/499</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand9-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand9-high"><code>stand9</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>520/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand9-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand9-low"><code>stand9</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>522/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand10-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand10-high"><code>stand10</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>528/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand10-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand10-low"><code>stand10</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>528/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand11-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand11-high"><code>stand11</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>528/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand11-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand11-low"><code>stand11</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>528/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-stand12-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand12-high"><code>stand12</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1450/1459</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-stand12-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand12-low"><code>stand12</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1450/1460</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-stand15-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand15-high"><code>stand15</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T00:55:09</td>
      <td>stale-zero-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>444/452</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-stand15-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand15-low"><code>stand15</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T00:55:09</td>
      <td>stale-zero-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>444/452</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-stand16-high">
      <td><a class="scene-perf-rowlink" href="#perf-stand16-high"><code>stand16</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>+0.2%</td>
      <td class="spd-green">99.8%</td>
      <td>473/472</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-stand16-low">
      <td><a class="scene-perf-rowlink" href="#perf-stand16-low"><code>stand16</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>+0.2%</td>
      <td class="spd-green">99.8%</td>
      <td>473/472</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-suzy1-high">
      <td><a class="scene-perf-rowlink" href="#perf-suzy1-high"><code>suzy1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-08T09:40:12</td>
      <td>missing-scenes-current-v001</td>
      <td>0.4%</td>
      <td class="spd-green">99.6%</td>
      <td>5763/5738</td>
      <td>21</td>
      <td>21</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-suzy1-low">
      <td><a class="scene-perf-rowlink" href="#perf-suzy1-low"><code>suzy1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-08T09:40:12</td>
      <td>missing-scenes-current-v001</td>
      <td>0.4%</td>
      <td class="spd-green">99.6%</td>
      <td>5763/5738</td>
      <td>21</td>
      <td>21</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-suzy2-high">
      <td><a class="scene-perf-rowlink" href="#perf-suzy2-high"><code>suzy2</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-08T09:32:10</td>
      <td>missing-scenes-current-v001</td>
      <td>0.8%</td>
      <td class="spd-green">99.2%</td>
      <td>2655/2633</td>
      <td>19</td>
      <td>19</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-suzy2-low">
      <td><a class="scene-perf-rowlink" href="#perf-suzy2-low"><code>suzy2</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-08T09:32:10</td>
      <td>missing-scenes-current-v001</td>
      <td>0.8%</td>
      <td class="spd-green">99.2%</td>
      <td>2655/2633</td>
      <td>19</td>
      <td>19</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-visitor1-high">
      <td><a class="scene-perf-rowlink" href="#perf-visitor1-high"><code>visitor1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:47:29</td>
      <td>mismatch-top-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>672/677</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-visitor1-low">
      <td><a class="scene-perf-rowlink" href="#perf-visitor1-low"><code>visitor1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:47:29</td>
      <td>mismatch-top-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>672/677</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-visitor3-high">
      <td><a class="scene-perf-rowlink" href="#perf-visitor3-high"><code>visitor3</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-09T18:08:05</td>
      <td>visitor3-low-noop113-v249</td>
      <td>+3.7%</td>
      <td class="spd-orange">96.5%</td>
      <td>1075/1037</td>
      <td>59</td>
      <td>0</td>
      <td>11</td>
      <td>v238 high resident-copy profile revalidated flat under the v249 VISITOR3 broad no-regression gate</td>
    </tr>
    <tr id="perf-visitor3-low">
      <td><a class="scene-perf-rowlink" href="#perf-visitor3-low"><code>visitor3</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-09T18:08:05</td>
      <td>visitor3-low-noop113-v249</td>
      <td>+3.5%</td>
      <td class="spd-orange">96.7%</td>
      <td>1075/1039</td>
      <td>69</td>
      <td>0</td>
      <td>12</td>
      <td>v249 rewrites low frame 113 in place as a two-byte compact no-op residual after the v248 frame 114..117 compaction; active loop improves 1086/1035 to 1075/1039, overrun 51 to 36, blocking 93 to 69, loop reads 17 to 16, loop-read time 100 to 83, due misses 16 to 12, and hidden refill stays 0 with fixed pack LBA, sound offset, and PS-EXE bucket</td>
    </tr>
    <tr id="perf-visitor4-high">
      <td><a class="scene-perf-rowlink" href="#perf-visitor4-high"><code>visitor4</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:27:49</td>
      <td>visitor4-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>424/428</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-visitor4-low">
      <td><a class="scene-perf-rowlink" href="#perf-visitor4-low"><code>visitor4</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:27:49</td>
      <td>visitor4-v072-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>424/428</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr id="perf-visitor5-high">
      <td><a class="scene-perf-rowlink" href="#perf-visitor5-high"><code>visitor5</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-08T13:01:12</td>
      <td>visitor5-high-compact-fgp3-noautoprime-v158</td>
      <td>+1.1%</td>
      <td class="spd-orange">98.9%</td>
      <td>1104/1092</td>
      <td>11</td>
      <td>11</td>
      <td>0</td>
      <td>padded compact FGP3/v4 high pack with VISITOR5 auto-prime disabled; paired low compact rejected</td>
    </tr>
    <tr id="perf-visitor5-low">
      <td><a class="scene-perf-rowlink" href="#perf-visitor5-low"><code>visitor5</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T00:23:01</td>
      <td>visitor5-v072-prefetch-relief</td>
      <td>+2.0%</td>
      <td class="spd-orange">98.0%</td>
      <td>1112/1090</td>
      <td>12</td>
      <td>12</td>
      <td>0</td>
      <td>validated pack; visitor5 clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff</td>
    </tr>
    <tr id="perf-visitor6-high">
      <td><a class="scene-perf-rowlink" href="#perf-visitor6-high"><code>visitor6</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T02:48:00</td>
      <td>stale-layout-v072c-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>2043/2047</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-visitor6-low">
      <td><a class="scene-perf-rowlink" href="#perf-visitor6-low"><code>visitor6</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T02:48:00</td>
      <td>stale-layout-v072c-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>2043/2047</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-visitor7-high">
      <td><a class="scene-perf-rowlink" href="#perf-visitor7-high"><code>visitor7</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T02:48:00</td>
      <td>stale-layout-v072c-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1619/1625</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-visitor7-low">
      <td><a class="scene-perf-rowlink" href="#perf-visitor7-low"><code>visitor7</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T02:48:00</td>
      <td>stale-layout-v072c-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1619/1625</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-walkstuf1-high">
      <td><a class="scene-perf-rowlink" href="#perf-walkstuf1-high"><code>walkstuf1</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-09T18:08:05</td>
      <td>visitor3-low-noop113-v249</td>
      <td>+4.8%</td>
      <td class="spd-orange">95.5%</td>
      <td>1495/1427</td>
      <td>95</td>
      <td>26</td>
      <td>14</td>
      <td>padded compact FGP3/v4 restore-minus-current pack; current top under-99 row after v249 broad no-regression gate</td>
    </tr>
    <tr id="perf-walkstuf1-low">
      <td><a class="scene-perf-rowlink" href="#perf-walkstuf1-low"><code>walkstuf1</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-09T18:08:05</td>
      <td>visitor3-low-noop113-v249</td>
      <td>+4.3%</td>
      <td class="spd-orange">95.8%</td>
      <td>1489/1427</td>
      <td>86</td>
      <td>27</td>
      <td>12</td>
      <td>padded compact FGP3/v4 restore-minus-current pack; revalidated flat under the v249 VISITOR3 broad no-regression gate</td>
    </tr>
    <tr id="perf-walkstuf2-high">
      <td><a class="scene-perf-rowlink" href="#perf-walkstuf2-high"><code>walkstuf2</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T00:38:12</td>
      <td>stale-top-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>451/461</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-walkstuf2-low">
      <td><a class="scene-perf-rowlink" href="#perf-walkstuf2-low"><code>walkstuf2</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T00:38:12</td>
      <td>stale-top-v072b-current-refresh</td>
      <td>0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>451/461</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr id="perf-walkstuf3-high">
      <td><a class="scene-perf-rowlink" href="#perf-walkstuf3-high"><code>walkstuf3</code></a></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-08T14:44:48</td>
      <td>walkstuf3-high-compact-fgp3-v163</td>
      <td>+0.9%</td>
      <td class="spd-green">99.1%</td>
      <td>2310/2290</td>
      <td>47</td>
      <td>18</td>
      <td>6</td>
      <td>high-tide compact FGP3/v4 restore-minus-current pack; fixed footprint and LBA under current layout</td>
    </tr>
    <tr id="perf-walkstuf3-low">
      <td><a class="scene-perf-rowlink" href="#perf-walkstuf3-low"><code>walkstuf3</code></a></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-08T16:59:36</td>
      <td>walkstuf3-low-compact-fgp3-v171</td>
      <td>+0.7%</td>
      <td class="spd-green">99.4%</td>
      <td>2310/2295</td>
      <td>26</td>
      <td>17</td>
      <td>2</td>
      <td>low-tide compact FGP3/v4 restore-minus-current pack; broad controls and WALKSTUF3 high exact-flat</td>
    </tr>
  </tbody>
</table>

## See also

- [/scenes/]({{ '/scenes/' | relative_url }}) — the visual-signoff
  ledger this card lives next to. Different bar, different cadence,
  different failure modes.
- [/docs/performance/]({{ '/docs/performance/' | relative_url }}) —
  reference manual: what each column means, how `loop_vb` and
  `target_vb` are measured, the column-by-column glossary.
- [/lab/from-87-to-99-5/]({{ '/lab/from-87-to-99-5/' | relative_url }})
  — retrospective on which experiments moved the matrix from the
  compact baseline (`+17.4%` over target) to its current public-capped
  `{{ site.release.perf_target_speed_pct }}%` target speed.
- [/lab/v081-mary4-freeze/]({{ '/lab/v081-mary4-freeze/' | relative_url }})
  — the v0.8.1 stability follow-on that kept this matrix's mean
  untouched while fixing a clean-rect pressure freeze the per-commit
  matrix never reached. Soak loop story.
- [/docs/walks/]({{ '/docs/walks/' | relative_url }}) —
  reference manual for the walk subsystem; "Evolution by release"
  consolidates the v0.8.0 clean-rect retry path and v0.8.1
  wave-band/split-rect pressure accounting that the v0.8.1
  story above generalized to fourteen random-position scenes.
- [Glossary: experiment log]({{ '/docs/glossary/#experiment-log' | relative_url }})
  — the long-form decision record at `docs/ps1/performance-experiment-log.md`
  where every accepted and rejected probe got written down. Read
  before re-trying anything that looks promising.
- [`docs/ps1/performance-scene-matrix.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-scene-matrix.csv)
  — the durable numeric source the table on this page is rendered
  from.
