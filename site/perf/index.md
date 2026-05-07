---
title: Performance battle card
eyebrow: Headless · 126-variant matrix
subtitle: Click a column header to sort. Target Speed cells are color-coded — ≥99% green, ≥80% yellow, <80% red.
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
from the compact baseline to its current `{{ site.release.perf_target_speed_pct }}%`
target-speed average is at
[/lab/from-87-to-99-5/]({{ '/lab/from-87-to-99-5/' | relative_url }}).
A scene can be timed here without being visually certified.

## At a glance

<p class="scene-perf-legend" aria-label="Target speed distribution as of {{ site.release.tag }}">
  Target Speed distribution at <code>{{ site.release.tag }}</code>:
  <span class="spd-key spd-green">98 ≥ 99%</span>
  <span class="spd-key spd-yellow">20 ≥ 80%</span>
  <span class="spd-key spd-red">2 &lt; 80%</span>
  out of 120 timing-bearing rows. 6 rows have no timing data yet.
</p>

The two red rows are [`visitor3`]({{ '/scenes/visitor3/' | relative_url }}) high (`70.0%`) and [`visitor3`]({{ '/scenes/visitor3/' | relative_url }})
low (`69.7%`) after the `v0.8.0` setup-prime + group 170..186
work; the largest single optimization target left on the matrix.
The yellow cluster includes the wide-action and [`BUILDING2`]({{ '/scenes/building2/' | relative_url }}) rows
still finishing their prefetch-relief and stream-window work.

The 6 untimed rows are [`MARY 3`]({{ '/scenes/mary3/' | relative_url }}) high/low (active-loop timing not
yet refreshed against the current pack), and [`SUZY 1`]({{ '/scenes/suzy1/' | relative_url }}) + [`SUZY 2`]({{ '/scenes/suzy2/' | relative_url }})
high/low (metadata-only; the SUZY mermaid scenes don't reach a
deterministic scene-end so they're excluded from speed averages
on purpose).

## Rollup

Current battle-card rollup as of 2026-05-06:

| Metric | Value |
|---|---:|
| Scenes visually validated | `{{ validated_count }} / {{ total_count }}` |
| Validated scenes | all 63 original scenes; see the live ledger above for the source rows |
| Scene/tide variants routed through headless perf | `126 / 126` |
| Timing-bearing variants | `120 / 126` |
| Scenes with at least one active-loop timed variant | `60 / 63` |
| Scenes with both high/low variants measured | `63 / 63` |
| Pending variants | `0 / 126` |
| Blocked variants | `0 / 126` |
| Timing-bearing average over target | `+0.8%` (`0.8229%` exact) |
| Timing-bearing average target speed | `99.5%` (`99.4860%` exact) |
| Latest perf matrix run | `2026-05-06T17:38:07` |
| Stats version | mixed: latest refreshed rows use `activity9-lowgroup-v072c`, `activity9-fgp3-v072c`, `activity4-fishing4-v072c-prefetch-relief`, `building4-6-johnny6-v072c-prefetch-relief`, `activity1-v072c-current-refresh`, `activity11-12-v072c-prefetch-relief`, `stale-next-v072c-current-refresh`, `mary1-v072c-prefetch-relief`, `stale-layout-v072c-current-refresh`, `activity9-v072c-prefetch-relief`, `stale-pressure2-v072c-current-refresh`, `johnny1-v072c-prefetch-relief`, `stale-pressure-v072c-current-refresh`, `activity10-johnny3-v072-prefetch-relief`, `stale-zero2-v072b-current-refresh`, `stale-zero-v072b-current-refresh`, `stale-top-v072b-current-refresh`, `visitor5-v072-prefetch-relief`, `mismatch-top-v072-current-refresh`, `stand-family-v072-current-refresh`, `visitor4-v072-current-refresh`, `stand1-v072-current-refresh`, `visitor3-v072-prefetch-relief`, `walkstuf1-v072-prefetch-relief`, `mary2-v068-wide-stitch`, `fishing5-v065-current-ledger-overlay`, `johnny2-v064-validation-refresh`, `compact-fgp3-v66-final-frame-hold`, `compact-fgp3-v64-building2-group318-330`, `compact-fgp3-v63-building2low-prime`, and `indexed8-row-local-dirty-v1`; other refreshed rows include `compact-fgp3-v62-fishing3low-group253-265`, `compact-fgp3-v61-fishing3low-group163-175`, `compact-fgp3-v60-visitor3high-group230-242`, `compact-fgp3-v59-visitor3high-group72-84`, `indexed8-tile-local-compose-v1`, `compact-fgp3-v58-activity9high-window20-table`, `compact-fgp3-v57-policy-table-refactor`, and `compact-fgp3-v49-walkstuf2-auto-prime` through `compact-fgp3-v29-smallprime`; full-matrix baseline rows remain `compact-fgp3-v2-fullmatrix` |
| FISHING 1 canary | `1068 / 1074 VBlanks`, `-0.6%`, `100.6% target speed`, `blocking_vb=2` |

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

- **Over Target**: how far `loop_vb` is above the captured target timing.
  Lower is better.
- **Target Speed**: `target_vb / loop_vb`. `100%` means exact target cadence.
- **VBlanks**: `loop_vb/target_vb`.
- **Blocking**: visible CD/blocking VBlanks.
- **Prefetch**: prefetch overrun VBlanks.
- **Due**: due-frame misses.
- **Latest Run**: ISO timestamp derived from the headless summary path
  (`scratch/ps1-perf-iterate/YYYYMMDD-HHMMSS`); `-` means no current
  matrix run has been recorded for that variant.
- **Stats Version**: performance/layout version for that row. The latest
  refreshed rows use `activity9-lowgroup-v072c`,
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
  `walkstuf1-v072-prefetch-relief`,
  `mary2-v068-wide-stitch`,
  `fishing5-v065-current-ledger-overlay`,
  `johnny2-v064-validation-refresh`,
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
  <span class="spd-key spd-yellow">≥ 80% (close)</span>
  <span class="spd-key spd-red">&lt; 80% (well below)</span>
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
    ≥80% yellow, &lt;80% red.
  </caption>
  <thead>
    <tr>
      <th>Scene</th>
      <th>Tide</th>
      <th>Status</th>
      <th>Latest Run</th>
      <th>Stats Version</th>
      <th>Over Target</th>
      <th>Target Speed</th>
      <th>VBlanks</th>
      <th>Blocking</th>
      <th>Prefetch</th>
      <th>Due</th>
      <th>Notes</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><code>activity1</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T04:01:15</td>
      <td>activity1-v072c-current-refresh</td>
      <td>-0.4%</td>
      <td class="spd-green">100.4%</td>
      <td>2754/2764</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>activity1</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T04:01:15</td>
      <td>activity1-v072c-current-refresh</td>
      <td>-0.4%</td>
      <td class="spd-green">100.4%</td>
      <td>2754/2765</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>activity4</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T04:41:05</td>
      <td>activity4-fishing4-v072c-prefetch-relief</td>
      <td>-0.1%</td>
      <td class="spd-green">100.1%</td>
      <td>1065/1066</td>
      <td>4</td>
      <td>4</td>
      <td>0</td>
      <td>clean-snapshot relief restores stage1_window prefetch versus fresh ACTIVITY4 current row; accepted hidden-refill tradeoff remains</td>
    </tr>
    <tr>
      <td><code>activity4</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T04:41:05</td>
      <td>activity4-fishing4-v072c-prefetch-relief</td>
      <td>-0.4%</td>
      <td class="spd-green">100.4%</td>
      <td>1064/1068</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td>clean-snapshot relief collapses ACTIVITY4 low due misses and restores stage1_window prefetch</td>
    </tr>
    <tr>
      <td><code>activity5</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T02:48:00</td>
      <td>stale-layout-v072c-current-refresh</td>
      <td>-1.1%</td>
      <td class="spd-green">101.1%</td>
      <td>1730/1749</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>activity5</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T02:48:00</td>
      <td>stale-layout-v072c-current-refresh</td>
      <td>-1.0%</td>
      <td class="spd-green">101.0%</td>
      <td>1731/1749</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>activity6</code></td>
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
    <tr>
      <td><code>activity6</code></td>
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
    <tr>
      <td><code>activity7</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:47:29</td>
      <td>mismatch-top-v072-current-refresh</td>
      <td>-0.5%</td>
      <td class="spd-green">100.5%</td>
      <td>593/596</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>activity7</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:47:29</td>
      <td>mismatch-top-v072-current-refresh</td>
      <td>-0.3%</td>
      <td class="spd-green">100.3%</td>
      <td>594/596</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>activity8</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:47:29</td>
      <td>mismatch-top-v072-current-refresh</td>
      <td>-0.7%</td>
      <td class="spd-green">100.7%</td>
      <td>898/904</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>activity8</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:47:29</td>
      <td>mismatch-top-v072-current-refresh</td>
      <td>-0.6%</td>
      <td class="spd-green">100.6%</td>
      <td>899/904</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>activity9</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T07:14:27</td>
      <td>activity9-fgp3-v072c</td>
      <td>+2.2%</td>
      <td class="spd-yellow">97.9%</td>
      <td>2101/2056</td>
      <td>44</td>
      <td>28</td>
      <td>2</td>
      <td>padded pal4 FGP3 temporal-residual conversion keeps the original 1745484-byte CD footprint while shrinking runtime payload to 1453793 bytes</td>
    </tr>
    <tr>
      <td><code>activity9</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T07:45:20</td>
      <td>activity9-lowgroup-v072c</td>
      <td>+1.8%</td>
      <td class="spd-yellow">98.2%</td>
      <td>2093/2056</td>
      <td>43</td>
      <td>14</td>
      <td>5</td>
      <td>low-tide pal4 FGP3 grouped append for sectors 624..636 reduces visible CD pressure after the padded FGP3 conversion</td>
    </tr>
    <tr>
      <td><code>activity10</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T01:47:28</td>
      <td>activity10-johnny3-v072-prefetch-relief</td>
      <td>+0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>1259/1259</td>
      <td>7</td>
      <td>4</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr>
      <td><code>activity10</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:47:28</td>
      <td>activity10-johnny3-v072-prefetch-relief</td>
      <td>-0.1%</td>
      <td class="spd-green">100.1%</td>
      <td>1255/1256</td>
      <td>17</td>
      <td>4</td>
      <td>2</td>
      <td></td>
    </tr>
    <tr>
      <td><code>activity11</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T03:40:30</td>
      <td>activity11-12-v072c-prefetch-relief</td>
      <td>+0.5%</td>
      <td class="spd-green">99.5%</td>
      <td>1729/1720</td>
      <td>10</td>
      <td>4</td>
      <td>1</td>
      <td>validated pack clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff</td>
    </tr>
    <tr>
      <td><code>activity11</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T03:40:30</td>
      <td>activity11-12-v072c-prefetch-relief</td>
      <td>+0.7%</td>
      <td class="spd-green">99.3%</td>
      <td>1729/1717</td>
      <td>14</td>
      <td>9</td>
      <td>1</td>
      <td>validated pack clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff</td>
    </tr>
    <tr>
      <td><code>activity12</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T03:40:30</td>
      <td>activity11-12-v072c-prefetch-relief</td>
      <td>-0.1%</td>
      <td class="spd-green">100.1%</td>
      <td>1411/1412</td>
      <td>7</td>
      <td>7</td>
      <td>0</td>
      <td>validated pack clean-snapshot relief exception keeps stage1_window prefetch; current high tide is baseline clean</td>
    </tr>
    <tr>
      <td><code>activity12</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T03:40:30</td>
      <td>activity11-12-v072c-prefetch-relief</td>
      <td>-0.1%</td>
      <td class="spd-green">100.1%</td>
      <td>1409/1411</td>
      <td>10</td>
      <td>6</td>
      <td>1</td>
      <td>validated pack clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff</td>
    </tr>
    <tr>
      <td><code>building1</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T00:38:12</td>
      <td>stale-top-v072b-current-refresh</td>
      <td>+2.1%</td>
      <td class="spd-yellow">98.0%</td>
      <td>794/778</td>
      <td>21</td>
      <td>21</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>building1</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T00:38:12</td>
      <td>stale-top-v072b-current-refresh</td>
      <td>+1.9%</td>
      <td class="spd-yellow">98.1%</td>
      <td>794/779</td>
      <td>21</td>
      <td>21</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>building2</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T01:59:35</td>
      <td>stale-pressure-v072c-current-refresh</td>
      <td>+14.9%</td>
      <td class="spd-yellow">87.1%</td>
      <td>1476/1285</td>
      <td>286</td>
      <td>66</td>
      <td>37</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>building2</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:59:35</td>
      <td>stale-pressure-v072c-current-refresh</td>
      <td>+14.6%</td>
      <td class="spd-yellow">87.2%</td>
      <td>1465/1278</td>
      <td>279</td>
      <td>48</td>
      <td>40</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>building3</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T03:26:10</td>
      <td>stale-next-v072c-current-refresh</td>
      <td>-0.1%</td>
      <td class="spd-green">100.1%</td>
      <td>5460/5465</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>building3</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T03:26:10</td>
      <td>stale-next-v072c-current-refresh</td>
      <td>-0.1%</td>
      <td class="spd-green">100.1%</td>
      <td>5460/5465</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>building4</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T04:15:06</td>
      <td>building4-6-johnny6-v072c-prefetch-relief</td>
      <td>+7.6%</td>
      <td class="spd-yellow">93.0%</td>
      <td>2985/2774</td>
      <td>285</td>
      <td>51</td>
      <td>40</td>
      <td>clean-snapshot relief restores stage1_window prefetch versus fresh failure baseline; accepted hidden-refill tradeoff remains</td>
    </tr>
    <tr>
      <td><code>building4</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T04:15:06</td>
      <td>building4-6-johnny6-v072c-prefetch-relief</td>
      <td>+7.1%</td>
      <td class="spd-yellow">93.4%</td>
      <td>2981/2784</td>
      <td>199</td>
      <td>119</td>
      <td>14</td>
      <td>clean-snapshot relief restores stage1_window prefetch versus fresh failure baseline; accepted hidden-refill tradeoff remains</td>
    </tr>
    <tr>
      <td><code>building5</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T17:38:07</td>
      <td>building5-fgp3-padded-v080</td>
      <td>-0.1%</td>
      <td class="spd-green">100.1%</td>
      <td>3343/3348</td>
      <td>5</td>
      <td>5</td>
      <td>0</td>
      <td>padded pal4 FGP3 temporal-residual conversion keeps the original 818670-byte CD footprint while shrinking runtime payload to 592755 bytes</td>
    </tr>
    <tr>
      <td><code>building5</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T17:38:07</td>
      <td>building5-fgp3-padded-v080</td>
      <td>-0.1%</td>
      <td class="spd-green">100.1%</td>
      <td>3345/3347</td>
      <td>8</td>
      <td>8</td>
      <td>0</td>
      <td>padded pal4 FGP3 temporal-residual conversion keeps the original 818670-byte CD footprint while shrinking runtime payload to 592755 bytes</td>
    </tr>
    <tr>
      <td><code>building6</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T04:15:06</td>
      <td>building4-6-johnny6-v072c-prefetch-relief</td>
      <td>+3.2%</td>
      <td class="spd-yellow">96.9%</td>
      <td>2520/2442</td>
      <td>62</td>
      <td>64</td>
      <td>1</td>
      <td>clean-snapshot relief restores stage1_window prefetch versus fresh failure baseline; accepted hidden-refill tradeoff remains</td>
    </tr>
    <tr>
      <td><code>building6</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T04:15:06</td>
      <td>building4-6-johnny6-v072c-prefetch-relief</td>
      <td>+3.2%</td>
      <td class="spd-yellow">96.9%</td>
      <td>2515/2437</td>
      <td>70</td>
      <td>66</td>
      <td>2</td>
      <td>clean-snapshot relief restores stage1_window prefetch versus fresh failure baseline; accepted hidden-refill tradeoff remains</td>
    </tr>
    <tr>
      <td><code>building7</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T02:48:00</td>
      <td>stale-layout-v072c-current-refresh</td>
      <td>-0.0%</td>
      <td class="spd-green">100.0%</td>
      <td>3132/3133</td>
      <td>9</td>
      <td>9</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>building7</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T02:48:00</td>
      <td>stale-layout-v072c-current-refresh</td>
      <td>-0.1%</td>
      <td class="spd-green">100.1%</td>
      <td>3130/3133</td>
      <td>7</td>
      <td>7</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>fishing1</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>-0.6%</td>
      <td class="spd-green">100.6%</td>
      <td>1068/1074</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>fishing1</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>-0.7%</td>
      <td class="spd-green">100.7%</td>
      <td>1067/1074</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>fishing2</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T03:26:10</td>
      <td>stale-next-v072c-current-refresh</td>
      <td>-0.1%</td>
      <td class="spd-green">100.1%</td>
      <td>1761/1763</td>
      <td>6</td>
      <td>6</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>fishing2</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T03:26:10</td>
      <td>stale-next-v072c-current-refresh</td>
      <td>-0.3%</td>
      <td class="spd-green">100.3%</td>
      <td>1759/1765</td>
      <td>3</td>
      <td>3</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>fishing3</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-01T07:40:38</td>
      <td>compact-fgp3-v66-final-frame-hold</td>
      <td>+0.4%</td>
      <td class="spd-green">99.6%</td>
      <td>1960/1952</td>
      <td>18</td>
      <td>13</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr>
      <td><code>fishing3</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-01T07:40:38</td>
      <td>compact-fgp3-v66-final-frame-hold</td>
      <td>+0.1%</td>
      <td class="spd-green">99.9%</td>
      <td>1956/1954</td>
      <td>6</td>
      <td>6</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr>
      <td><code>fishing4</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T04:41:05</td>
      <td>activity4-fishing4-v072c-prefetch-relief</td>
      <td>-0.8%</td>
      <td class="spd-green">100.8%</td>
      <td>835/842</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td>clean-snapshot relief keeps FISHING4 high under target with stage1_window prefetch restored</td>
    </tr>
    <tr>
      <td><code>fishing4</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T04:41:05</td>
      <td>activity4-fishing4-v072c-prefetch-relief</td>
      <td>-1.1%</td>
      <td class="spd-green">101.1%</td>
      <td>834/843</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>clean-snapshot relief collapses FISHING4 low due misses and makes the row CD-clean</td>
    </tr>
    <tr>
      <td><code>fishing5</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-02T22:39:34</td>
      <td>fishing5-v065-current-ledger-overlay</td>
      <td>-0.6%</td>
      <td class="spd-green">100.6%</td>
      <td>885/890</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr>
      <td><code>fishing5</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-02T22:39:34</td>
      <td>fishing5-v065-current-ledger-overlay</td>
      <td>-0.6%</td>
      <td class="spd-green">100.6%</td>
      <td>885/890</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr>
      <td><code>fishing6</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T00:38:12</td>
      <td>stale-top-v072b-current-refresh</td>
      <td>-1.2%</td>
      <td class="spd-green">101.2%</td>
      <td>744/753</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>fishing6</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T00:38:12</td>
      <td>stale-top-v072b-current-refresh</td>
      <td>-1.2%</td>
      <td class="spd-green">101.2%</td>
      <td>744/753</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>fishing7</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:47:29</td>
      <td>mismatch-top-v072-current-refresh</td>
      <td>-1.4%</td>
      <td class="spd-green">101.4%</td>
      <td>715/725</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>fishing7</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:47:29</td>
      <td>mismatch-top-v072-current-refresh</td>
      <td>-1.4%</td>
      <td class="spd-green">101.4%</td>
      <td>715/725</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>fishing8</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>-0.8%</td>
      <td class="spd-green">100.8%</td>
      <td>1243/1253</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>fishing8</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>-0.8%</td>
      <td class="spd-green">100.8%</td>
      <td>1243/1253</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>johnny1</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T02:08:50</td>
      <td>johnny1-v072c-prefetch-relief</td>
      <td>+1.5%</td>
      <td class="spd-yellow">98.5%</td>
      <td>1974/1944</td>
      <td>27</td>
      <td>27</td>
      <td>0</td>
      <td>johnny1 clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff</td>
    </tr>
    <tr>
      <td><code>johnny1</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T02:08:50</td>
      <td>johnny1-v072c-prefetch-relief</td>
      <td>+1.5%</td>
      <td class="spd-yellow">98.5%</td>
      <td>1974/1944</td>
      <td>27</td>
      <td>27</td>
      <td>0</td>
      <td>johnny1 clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff</td>
    </tr>
    <tr>
      <td><code>johnny2</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-02T19:06:01</td>
      <td>johnny2-v064-validation-refresh</td>
      <td>+0.6%</td>
      <td class="spd-green">99.4%</td>
      <td>1761/1751</td>
      <td>16</td>
      <td>0</td>
      <td>3</td>
      <td>validated v0.6.4 refresh; island-pos -64 54; correctness clean</td>
    </tr>
    <tr>
      <td><code>johnny2</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-02T19:06:01</td>
      <td>johnny2-v064-validation-refresh</td>
      <td>+0.5%</td>
      <td class="spd-green">99.5%</td>
      <td>1758/1750</td>
      <td>16</td>
      <td>1</td>
      <td>3</td>
      <td>validated v0.6.4 refresh; island-pos -64 54; correctness clean</td>
    </tr>
    <tr>
      <td><code>johnny3</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T01:47:28</td>
      <td>activity10-johnny3-v072-prefetch-relief</td>
      <td>-0.3%</td>
      <td class="spd-green">100.3%</td>
      <td>1158/1161</td>
      <td>10</td>
      <td>6</td>
      <td>1</td>
      <td></td>
    </tr>
    <tr>
      <td><code>johnny3</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:47:28</td>
      <td>activity10-johnny3-v072-prefetch-relief</td>
      <td>-0.8%</td>
      <td class="spd-green">100.8%</td>
      <td>1157/1166</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr>
      <td><code>johnny4</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>-0.8%</td>
      <td class="spd-green">100.8%</td>
      <td>1204/1214</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>johnny4</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>-0.8%</td>
      <td class="spd-green">100.8%</td>
      <td>1204/1214</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>johnny5</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T00:55:09</td>
      <td>stale-zero-v072b-current-refresh</td>
      <td>-1.1%</td>
      <td class="spd-green">101.1%</td>
      <td>811/820</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>johnny5</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T00:55:09</td>
      <td>stale-zero-v072b-current-refresh</td>
      <td>-1.2%</td>
      <td class="spd-green">101.2%</td>
      <td>810/820</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>johnny6</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T04:15:06</td>
      <td>building4-6-johnny6-v072c-prefetch-relief</td>
      <td>+1.1%</td>
      <td class="spd-yellow">98.9%</td>
      <td>2832/2800</td>
      <td>28</td>
      <td>28</td>
      <td>0</td>
      <td>clean-snapshot relief restores stage1_window prefetch versus fresh current row</td>
    </tr>
    <tr>
      <td><code>johnny6</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T04:15:06</td>
      <td>building4-6-johnny6-v072c-prefetch-relief</td>
      <td>+1.1%</td>
      <td class="spd-yellow">98.9%</td>
      <td>2832/2800</td>
      <td>28</td>
      <td>28</td>
      <td>0</td>
      <td>clean-snapshot relief restores stage1_window prefetch versus fresh current row</td>
    </tr>
    <tr>
      <td><code>mary1</code></td>
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
    <tr>
      <td><code>mary1</code></td>
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
    <tr>
      <td><code>mary2</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-03T14:22:57</td>
      <td>mary2-v068-wide-stitch</td>
      <td>+0.2%</td>
      <td class="spd-green">99.8%</td>
      <td>2250/2246</td>
      <td>4</td>
      <td>4</td>
      <td>0</td>
      <td>validated v0.6.8 wide multi-view stitch; perf route uses island-pos -154 54; far-right and true far-left visual stress passed</td>
    </tr>
    <tr>
      <td><code>mary2</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-03T14:22:57</td>
      <td>mary2-v068-wide-stitch</td>
      <td>+0.3%</td>
      <td class="spd-green">99.7%</td>
      <td>2253/2246</td>
      <td>7</td>
      <td>7</td>
      <td>0</td>
      <td>validated v0.6.8 wide multi-view stitch; perf route uses island-pos -154 54; far-right and true far-left visual stress passed</td>
    </tr>
    <tr>
      <td><code>mary3</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-04-29T17:45:25</td>
      <td>compact-fgp3-v2-fullmatrix</td>
      <td>-</td>
      <td>-</td>
      <td>0/0</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>validated 2026-05-03 after x=80 full-frame foreground-only recapture and low-memory clean-snapshot relief; active-loop timing still needs refresh</td>
    </tr>
    <tr>
      <td><code>mary3</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-04-29T17:45:37</td>
      <td>compact-fgp3-v2-fullmatrix</td>
      <td>-</td>
      <td>-</td>
      <td>0/0</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>validated 2026-05-03 after x=80 full-frame foreground-only recapture and low-memory clean-snapshot relief; active-loop timing still needs refresh</td>
    </tr>
    <tr>
      <td><code>mary4</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-04-29T17:46:07</td>
      <td>compact-fgp3-v2-fullmatrix</td>
      <td>-2.4%</td>
      <td class="spd-green">102.4%</td>
      <td>1968/2016</td>
      <td>28</td>
      <td>12</td>
      <td>3</td>
      <td>validated 2026-05-03 after generic multi-view stitch; active timing predates refreshed pack; far-right x=300 visual stress passed</td>
    </tr>
    <tr>
      <td><code>mary4</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-04-29T17:46:13</td>
      <td>compact-fgp3-v2-fullmatrix</td>
      <td>-2.6%</td>
      <td class="spd-green">102.7%</td>
      <td>1966/2019</td>
      <td>24</td>
      <td>10</td>
      <td>3</td>
      <td>validated 2026-05-03 after generic multi-view stitch; active timing predates refreshed pack; far-right x=300 visual stress passed</td>
    </tr>
    <tr>
      <td><code>mary5</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T03:26:10</td>
      <td>stale-next-v072c-current-refresh</td>
      <td>+0.6%</td>
      <td class="spd-green">99.4%</td>
      <td>1591/1582</td>
      <td>8</td>
      <td>8</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>mary5</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T03:26:10</td>
      <td>stale-next-v072c-current-refresh</td>
      <td>+0.5%</td>
      <td class="spd-green">99.5%</td>
      <td>1590/1582</td>
      <td>7</td>
      <td>7</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>miscgag1</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T00:55:09</td>
      <td>stale-zero-v072b-current-refresh</td>
      <td>-0.8%</td>
      <td class="spd-green">100.8%</td>
      <td>953/961</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>miscgag1</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T00:55:09</td>
      <td>stale-zero-v072b-current-refresh</td>
      <td>-0.8%</td>
      <td class="spd-green">100.8%</td>
      <td>953/961</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>miscgag2</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-04-30T06:58:15</td>
      <td>compact-fgp3-v31-auto224</td>
      <td>-0.3%</td>
      <td class="spd-green">100.3%</td>
      <td>1352/1356</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr>
      <td><code>miscgag2</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-04-30T06:58:15</td>
      <td>compact-fgp3-v31-auto224</td>
      <td>-0.3%</td>
      <td class="spd-green">100.3%</td>
      <td>1352/1356</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td></td>
    </tr>
    <tr>
      <td><code>stand1</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:17:25</td>
      <td>stand1-v072-current-refresh</td>
      <td>-4.0%</td>
      <td class="spd-green">104.1%</td>
      <td>194/202</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated 18-entry host-deadline pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand1</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:17:25</td>
      <td>stand1-v072-current-refresh</td>
      <td>-4.0%</td>
      <td class="spd-green">104.1%</td>
      <td>194/202</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated 18-entry host-deadline pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand2</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>-2.0%</td>
      <td class="spd-green">102.1%</td>
      <td>480/490</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand2</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>-2.0%</td>
      <td class="spd-green">102.1%</td>
      <td>480/490</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand3</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T00:38:12</td>
      <td>stale-top-v072b-current-refresh</td>
      <td>-1.8%</td>
      <td class="spd-green">101.8%</td>
      <td>547/557</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand3</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T00:38:12</td>
      <td>stale-top-v072b-current-refresh</td>
      <td>-1.8%</td>
      <td class="spd-green">101.8%</td>
      <td>547/557</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand4</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>-1.5%</td>
      <td class="spd-green">101.5%</td>
      <td>1202/1220</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand4</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>-1.2%</td>
      <td class="spd-green">101.2%</td>
      <td>1203/1218</td>
      <td>3</td>
      <td>3</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand5</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>-1.2%</td>
      <td class="spd-green">101.2%</td>
      <td>1442/1460</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand5</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>-1.2%</td>
      <td class="spd-green">101.2%</td>
      <td>1442/1460</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand6</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>-1.3%</td>
      <td class="spd-green">101.3%</td>
      <td>1346/1364</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand6</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>-1.3%</td>
      <td class="spd-green">101.3%</td>
      <td>1346/1364</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand7</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>-3.3%</td>
      <td class="spd-green">103.5%</td>
      <td>520/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand7</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>-3.3%</td>
      <td class="spd-green">103.5%</td>
      <td>520/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand8</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>-3.2%</td>
      <td class="spd-green">103.3%</td>
      <td>483/499</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand8</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>-3.2%</td>
      <td class="spd-green">103.3%</td>
      <td>483/499</td>
      <td>2</td>
      <td>2</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand9</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>-3.3%</td>
      <td class="spd-green">103.5%</td>
      <td>520/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand9</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>-3.0%</td>
      <td class="spd-green">103.1%</td>
      <td>522/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand10</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>-1.9%</td>
      <td class="spd-green">101.9%</td>
      <td>528/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand10</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>-1.9%</td>
      <td class="spd-green">101.9%</td>
      <td>528/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand11</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>-1.9%</td>
      <td class="spd-green">101.9%</td>
      <td>528/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand11</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:34:06</td>
      <td>stand-family-v072-current-refresh</td>
      <td>-1.9%</td>
      <td class="spd-green">101.9%</td>
      <td>528/538</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand12</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>-0.6%</td>
      <td class="spd-green">100.6%</td>
      <td>1450/1459</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand12</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T01:14:22</td>
      <td>stale-zero2-v072b-current-refresh</td>
      <td>-0.7%</td>
      <td class="spd-green">100.7%</td>
      <td>1450/1460</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand15</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T00:55:09</td>
      <td>stale-zero-v072b-current-refresh</td>
      <td>-1.8%</td>
      <td class="spd-green">101.8%</td>
      <td>444/452</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand15</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T00:55:09</td>
      <td>stale-zero-v072b-current-refresh</td>
      <td>-1.8%</td>
      <td class="spd-green">101.8%</td>
      <td>444/452</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>stand16</code></td>
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
    <tr>
      <td><code>stand16</code></td>
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
    <tr>
      <td><code>suzy1</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-04-29T18:01:51</td>
      <td>compact-fgp3-v2-fullmatrix</td>
      <td>-</td>
      <td>-</td>
      <td>0/6</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>metadata-only; no active-loop timing; excluded from speed averages</td>
    </tr>
    <tr>
      <td><code>suzy1</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-04-29T18:01:58</td>
      <td>compact-fgp3-v2-fullmatrix</td>
      <td>-</td>
      <td>-</td>
      <td>0/6</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>metadata-only; no active-loop timing; excluded from speed averages</td>
    </tr>
    <tr>
      <td><code>suzy2</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-04-29T18:02:29</td>
      <td>compact-fgp3-v2-fullmatrix</td>
      <td>-</td>
      <td>-</td>
      <td>0/6</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>metadata-only; no active-loop timing; excluded from speed averages</td>
    </tr>
    <tr>
      <td><code>suzy2</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-04-29T18:02:35</td>
      <td>compact-fgp3-v2-fullmatrix</td>
      <td>-</td>
      <td>-</td>
      <td>0/6</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>metadata-only; no active-loop timing; excluded from speed averages</td>
    </tr>
    <tr>
      <td><code>visitor1</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:47:29</td>
      <td>mismatch-top-v072-current-refresh</td>
      <td>-0.7%</td>
      <td class="spd-green">100.7%</td>
      <td>672/677</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>visitor1</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:47:29</td>
      <td>mismatch-top-v072-current-refresh</td>
      <td>-0.7%</td>
      <td class="spd-green">100.7%</td>
      <td>672/677</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>visitor3</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T15:10:17</td>
      <td>visitor3-setup-prime-192k-v080</td>
      <td>+42.9%</td>
      <td class="spd-red">70.0%</td>
      <td>1450/1015</td>
      <td>355</td>
      <td>14</td>
      <td>31</td>
      <td>VISITOR3-only 192 KiB setup-prime resident cap reduces active-loop CD reads/blocking after accepted high-tide group 170..186; global cap remains 128 KiB</td>
    </tr>
    <tr>
      <td><code>visitor3</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T16:20:55</td>
      <td>visitor3-low-group170-186-v080b</td>
      <td>+43.5%</td>
      <td class="spd-red">69.7%</td>
      <td>1452/1012</td>
      <td>361</td>
      <td>19</td>
      <td>32</td>
      <td>VISITOR3 low grouped append 170..186 saves one visible read after the 192 KiB setup-prime baseline; high/FISHING1/BUILDING2 canaries stay flat</td>
    </tr>
    <tr>
      <td><code>visitor4</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-05T23:27:49</td>
      <td>visitor4-v072-current-refresh</td>
      <td>-0.9%</td>
      <td class="spd-green">100.9%</td>
      <td>424/428</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>visitor4</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-05T23:27:49</td>
      <td>visitor4-v072-current-refresh</td>
      <td>-0.9%</td>
      <td class="spd-green">100.9%</td>
      <td>424/428</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>visitor5</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T00:23:01</td>
      <td>visitor5-v072-prefetch-relief</td>
      <td>+1.9%</td>
      <td class="spd-yellow">98.1%</td>
      <td>1111/1090</td>
      <td>12</td>
      <td>12</td>
      <td>0</td>
      <td>validated pack; visitor5 clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff</td>
    </tr>
    <tr>
      <td><code>visitor5</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T00:23:01</td>
      <td>visitor5-v072-prefetch-relief</td>
      <td>+2.0%</td>
      <td class="spd-yellow">98.0%</td>
      <td>1112/1090</td>
      <td>12</td>
      <td>12</td>
      <td>0</td>
      <td>validated pack; visitor5 clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff</td>
    </tr>
    <tr>
      <td><code>visitor6</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T02:48:00</td>
      <td>stale-layout-v072c-current-refresh</td>
      <td>-0.2%</td>
      <td class="spd-green">100.2%</td>
      <td>2043/2047</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>visitor6</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T02:48:00</td>
      <td>stale-layout-v072c-current-refresh</td>
      <td>-0.2%</td>
      <td class="spd-green">100.2%</td>
      <td>2043/2047</td>
      <td>1</td>
      <td>1</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>visitor7</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T02:48:00</td>
      <td>stale-layout-v072c-current-refresh</td>
      <td>-0.4%</td>
      <td class="spd-green">100.4%</td>
      <td>1619/1625</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>visitor7</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T02:48:00</td>
      <td>stale-layout-v072c-current-refresh</td>
      <td>-0.4%</td>
      <td class="spd-green">100.4%</td>
      <td>1619/1625</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>walkstuf1</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T15:55:23</td>
      <td>walkstuf1-fgp2-setup-prime-v080</td>
      <td>+13.7%</td>
      <td class="spd-yellow">88.0%</td>
      <td>1595/1403</td>
      <td>278</td>
      <td>50</td>
      <td>57</td>
      <td>validated FGP2/pal4 pack; PAL4 setup-prime policy now primes the initial window while preserving stage1_window prefetch; accepted canaries exact-flat</td>
    </tr>
    <tr>
      <td><code>walkstuf1</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T15:56:38</td>
      <td>walkstuf1-fgp2-setup-prime-v080</td>
      <td>+15.5%</td>
      <td class="spd-yellow">86.6%</td>
      <td>1614/1397</td>
      <td>276</td>
      <td>59</td>
      <td>49</td>
      <td>validated FGP2/pal4 pack; PAL4 setup-prime policy now primes the initial window while preserving stage1_window prefetch; accepted canaries exact-flat</td>
    </tr>
    <tr>
      <td><code>walkstuf2</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T00:38:12</td>
      <td>stale-top-v072b-current-refresh</td>
      <td>-2.2%</td>
      <td class="spd-green">102.2%</td>
      <td>451/461</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>walkstuf2</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T00:38:12</td>
      <td>stale-top-v072b-current-refresh</td>
      <td>-2.2%</td>
      <td class="spd-green">102.2%</td>
      <td>451/461</td>
      <td>0</td>
      <td>0</td>
      <td>0</td>
      <td>current validated pack refresh; baseline correction</td>
    </tr>
    <tr>
      <td><code>walkstuf3</code></td>
      <td>high</td>
      <td>measured</td>
      <td>2026-05-06T02:23:47</td>
      <td>stale-pressure2-v072c-current-refresh</td>
      <td>+1.9%</td>
      <td class="spd-yellow">98.1%</td>
      <td>2321/2278</td>
      <td>68</td>
      <td>36</td>
      <td>6</td>
      <td>current validated pack refresh after longer noloop window; baseline correction</td>
    </tr>
    <tr>
      <td><code>walkstuf3</code></td>
      <td>low</td>
      <td>measured</td>
      <td>2026-05-06T02:23:47</td>
      <td>stale-pressure2-v072c-current-refresh</td>
      <td>+1.1%</td>
      <td class="spd-yellow">98.9%</td>
      <td>2321/2295</td>
      <td>40</td>
      <td>20</td>
      <td>5</td>
      <td>current validated pack refresh after longer noloop window; baseline correction</td>
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
  compact baseline (`+17.4%` over target) to its current
  `{{ site.release.perf_target_speed_pct }}%` target speed.
- [`docs/ps1/performance-scene-matrix.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-scene-matrix.csv)
  — the durable numeric source the table on this page is rendered
  from.
