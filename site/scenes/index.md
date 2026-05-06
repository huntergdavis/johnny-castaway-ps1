---
title: Scenes
eyebrow: Live ledger
subtitle: Every scene the game has, every one's status, no fudge.
redirect_from:
  - /docs/testing-status/
description: The Johnny Castaway scene ledger — 63 scenes, current validation state, last-verified release tag, per-scene case studies.
---

{% assign all_scenes      = site.data.scenes %}
{% assign validated_count = all_scenes | where: "status", "validated"  | size %}
{% assign bringup_count   = all_scenes | where: "status", "in-bringup" | size %}
{% assign pending_count   = all_scenes | where: "status", "pending"    | size %}
{% assign blocked_count   = all_scenes | where: "status", "blocked"    | size %}
{% assign total_count     = all_scenes | size %}

The original Johnny Castaway shipped 63 scenes — short looped vignettes
the screensaver picked from at random. This port aims to reproduce all
63 in original order on the PlayStation 1, with no scenes added or
dropped. The captions and the holiday overlay set are the only things
expanded past the source material.

A scene counts as **validated** when it clears what the project calls
the **FISHING 1 bar**: pixel-perfect visuals against the host-captured
reference, plus SFX cues that land on the same engine ticks, across
every variant flag that applies to the scene (night palette, low-tide
shoreline, holiday overlay, raft-stage progression). That's a higher
bar than "it ran once and didn't crash," which is why most rows below
are still `pending`.

The source of truth for this ledger is
[`docs/ps1/scene-status.md`]({{ site.github_url }}/blob/main/docs/ps1/scene-status.md)
in the project repo. The table on this page is generated from
[`site/_data/scenes.yml`]({{ site.github_url }}/blob/main/site/_data/scenes.yml),
which is parsed from that file. Current tally: **{{ validated_count }} of
{{ total_count }} scenes** validated. `{{ site.release.tag }}` is the
release line the validated rows were last verified on. For the longer
explanation of how a scene moves from `pending` to `validated`, see
[the method]({{ '/about/method/' | relative_url }}) and
[the regtest harness]({{ '/docs/regtest/' | relative_url }}). The older
host-capture baselines have their own
[regtest case shelf]({{ '/archaeology/regtest-references/cases/' | relative_url }}),
which is useful when a scene needs to be compared against the frozen desktop
reference instead of the PS1 replay path.

## At a glance

<ul class="scene-counts">
  <li><span class="scene-status ok">validated</span> &nbsp; {{ validated_count }} / {{ total_count }}</li>
  <li><span class="scene-status wip">in-bring-up</span> &nbsp; {{ bringup_count }}</li>
  <li><span class="scene-status pending">pending</span> &nbsp; {{ pending_count }}</li>
  <li><span class="scene-status blocked">blocked</span> &nbsp; {{ blocked_count }}</li>
</ul>

## The ledger

Sorted by ADS file then by tag. Scene names link to the per-scene case
study; every scene has one, including the pending ones (which mostly
just say "not yet validated" and label what the scene probably is).

{% assign sorted_scenes = all_scenes | sort: "tag" | sort: "ads" %}

<table class="scene-table">
  <thead>
    <tr>
      <th class="scene-tag">ADS · tag</th>
      <th class="scene-name">Scene</th>
      <th class="scene-status">Status</th>
      <th>Last verified</th>
      <th>Notes</th>
    </tr>
  </thead>
  <tbody>
    {% for s in sorted_scenes %}
      {% if s.status == "validated"  %}{% assign cls = "ok"      %}{% endif %}
      {% if s.status == "in-bringup" %}{% assign cls = "wip"     %}{% endif %}
      {% if s.status == "pending"    %}{% assign cls = "pending" %}{% endif %}
      {% if s.status == "blocked"    %}{% assign cls = "blocked" %}{% endif %}
      <tr>
        <td class="scene-tag">{{ s.ads }} {{ s.tag }}</td>
        <td class="scene-name"><a href="{{ '/scenes/' | append: s.slug | append: '/' | relative_url }}">{{ s.slug }}</a></td>
        <td class="scene-status {{ cls }}">{{ s.status }}</td>
        <td>{% if s.last_verified != "" %}<code>{{ s.last_verified }}</code>{% else %}—{% endif %}</td>
        <td>{{ s.notes }}</td>
      </tr>
    {% endfor %}
  </tbody>
</table>

## Performance battle card

The ledger above tracks the **release bar** — pixel-perfect human signoff. This section tracks the **performance bar** — automated DuckStation timing. They are different bars and should not be mixed:

- **Scene validation** is human visual + audible signoff. That is the release
  bar for "done."
- **Headless performance** is automated DuckStation timing. That is the
  optimization battle card for "how fast does this scene/tide variant run."

A scene can be timed here without being visually certified.

### Rollup

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
| Timing-bearing average over target | `+1.0%` (`0.9628%` exact) |
| Timing-bearing average target speed | `99.4%` (`99.3677%` exact) |
| Latest perf matrix run | `2026-05-06T04:41:05` |
| Stats version | mixed: latest refreshed rows use `activity4-fishing4-v072c-prefetch-relief`, `building4-6-johnny6-v072c-prefetch-relief`, `activity1-v072c-current-refresh`, `activity11-12-v072c-prefetch-relief`, `stale-next-v072c-current-refresh`, `mary1-v072c-prefetch-relief`, `stale-layout-v072c-current-refresh`, `activity9-v072c-prefetch-relief`, `stale-pressure2-v072c-current-refresh`, `johnny1-v072c-prefetch-relief`, `stale-pressure-v072c-current-refresh`, `activity10-johnny3-v072-prefetch-relief`, `stale-zero2-v072b-current-refresh`, `stale-zero-v072b-current-refresh`, `stale-top-v072b-current-refresh`, `visitor5-v072-prefetch-relief`, `mismatch-top-v072-current-refresh`, `stand-family-v072-current-refresh`, `visitor4-v072-current-refresh`, `stand1-v072-current-refresh`, `visitor3-v072-prefetch-relief`, `walkstuf1-v072-prefetch-relief`, `mary2-v068-wide-stitch`, `fishing5-v065-current-ledger-overlay`, `johnny2-v064-validation-refresh`, `compact-fgp3-v66-final-frame-hold`, `compact-fgp3-v64-building2-group318-330`, `compact-fgp3-v63-building2low-prime`, and `indexed8-row-local-dirty-v1`; other refreshed rows include `compact-fgp3-v62-fishing3low-group253-265`, `compact-fgp3-v61-fishing3low-group163-175`, `compact-fgp3-v60-visitor3high-group230-242`, `compact-fgp3-v59-visitor3high-group72-84`, `indexed8-tile-local-compose-v1`, `compact-fgp3-v58-activity9high-window20-table`, `compact-fgp3-v57-policy-table-refactor`, and `compact-fgp3-v49-walkstuf2-auto-prime` through `compact-fgp3-v29-smallprime`; full-matrix baseline rows remain `compact-fgp3-v2-fullmatrix` |
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

### Reading the table

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
  refreshed rows use `activity4-fishing4-v072c-prefetch-relief`,
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

### 126-variant battle card

| Scene | Tide | Status | Latest Run | Stats Version | Over Target | Target Speed | VBlanks | Blocking | Prefetch | Due | Notes |
|---|---|---|---|---|---:|---:|---:|---:|---:|---:|---|
| `activity1` | high | measured | 2026-05-06T04:01:15 | activity1-v072c-current-refresh | -0.4% | 100.4% | 2754/2764 | 1 | 1 | 0 | current validated pack refresh; baseline correction |
| `activity1` | low | measured | 2026-05-06T04:01:15 | activity1-v072c-current-refresh | -0.4% | 100.4% | 2754/2765 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `activity4` | high | measured | 2026-05-06T04:41:05 | activity4-fishing4-v072c-prefetch-relief | -0.1% | 100.1% | 1065/1066 | 4 | 4 | 0 | clean-snapshot relief restores stage1_window prefetch versus fresh ACTIVITY4 current row; accepted hidden-refill tradeoff remains |
| `activity4` | low | measured | 2026-05-06T04:41:05 | activity4-fishing4-v072c-prefetch-relief | -0.4% | 100.4% | 1064/1068 | 1 | 1 | 0 | clean-snapshot relief collapses ACTIVITY4 low due misses and restores stage1_window prefetch |
| `activity5` | high | measured | 2026-05-06T02:48:00 | stale-layout-v072c-current-refresh | -1.1% | 101.1% | 1730/1749 | 2 | 2 | 0 | current validated pack refresh; baseline correction |
| `activity5` | low | measured | 2026-05-06T02:48:00 | stale-layout-v072c-current-refresh | -1.0% | 101.0% | 1731/1749 | 2 | 2 | 0 | current validated pack refresh; baseline correction |
| `activity6` | high | measured | 2026-05-06T00:55:09 | stale-zero-v072b-current-refresh | +0.1% | 99.9% | 912/911 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `activity6` | low | measured | 2026-05-06T00:55:09 | stale-zero-v072b-current-refresh | +0.1% | 99.9% | 912/911 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `activity7` | high | measured | 2026-05-05T23:47:29 | mismatch-top-v072-current-refresh | -0.5% | 100.5% | 593/596 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `activity7` | low | measured | 2026-05-05T23:47:29 | mismatch-top-v072-current-refresh | -0.3% | 100.3% | 594/596 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `activity8` | high | measured | 2026-05-05T23:47:29 | mismatch-top-v072-current-refresh | -0.7% | 100.7% | 898/904 | 1 | 1 | 0 | current validated pack refresh; baseline correction |
| `activity8` | low | measured | 2026-05-05T23:47:29 | mismatch-top-v072-current-refresh | -0.6% | 100.6% | 899/904 | 2 | 2 | 0 | current validated pack refresh; baseline correction |
| `activity9` | high | measured | 2026-05-06T02:29:16 | activity9-v072c-prefetch-relief | +7.0% | 93.4% | 2194/2050 | 139 | 12 | 25 | activity9 clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff |
| `activity9` | low | measured | 2026-05-06T02:29:16 | activity9-v072c-prefetch-relief | +8.2% | 92.4% | 2218/2050 | 175 | 17 | 48 | activity9 clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff |
| `activity10` | high | measured | 2026-05-06T01:47:28 | activity10-johnny3-v072-prefetch-relief | +0.0% | 100.0% | 1259/1259 | 7 | 4 | 1 |  |
| `activity10` | low | measured | 2026-05-06T01:47:28 | activity10-johnny3-v072-prefetch-relief | -0.1% | 100.1% | 1255/1256 | 17 | 4 | 2 |  |
| `activity11` | high | measured | 2026-05-06T03:40:30 | activity11-12-v072c-prefetch-relief | +0.5% | 99.5% | 1729/1720 | 10 | 4 | 1 | validated pack clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff |
| `activity11` | low | measured | 2026-05-06T03:40:30 | activity11-12-v072c-prefetch-relief | +0.7% | 99.3% | 1729/1717 | 14 | 9 | 1 | validated pack clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff |
| `activity12` | high | measured | 2026-05-06T03:40:30 | activity11-12-v072c-prefetch-relief | -0.1% | 100.1% | 1411/1412 | 7 | 7 | 0 | validated pack clean-snapshot relief exception keeps stage1_window prefetch; current high tide is baseline clean |
| `activity12` | low | measured | 2026-05-06T03:40:30 | activity11-12-v072c-prefetch-relief | -0.1% | 100.1% | 1409/1411 | 10 | 6 | 1 | validated pack clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff |
| `building1` | high | measured | 2026-05-06T00:38:12 | stale-top-v072b-current-refresh | +2.1% | 98.0% | 794/778 | 21 | 21 | 0 | current validated pack refresh; baseline correction |
| `building1` | low | measured | 2026-05-06T00:38:12 | stale-top-v072b-current-refresh | +1.9% | 98.1% | 794/779 | 21 | 21 | 0 | current validated pack refresh; baseline correction |
| `building2` | high | measured | 2026-05-06T01:59:35 | stale-pressure-v072c-current-refresh | +14.9% | 87.1% | 1476/1285 | 286 | 66 | 37 | current validated pack refresh; baseline correction |
| `building2` | low | measured | 2026-05-06T01:59:35 | stale-pressure-v072c-current-refresh | +14.6% | 87.2% | 1465/1278 | 279 | 48 | 40 | current validated pack refresh; baseline correction |
| `building3` | high | measured | 2026-05-06T03:26:10 | stale-next-v072c-current-refresh | -0.1% | 100.1% | 5460/5465 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `building3` | low | measured | 2026-05-06T03:26:10 | stale-next-v072c-current-refresh | -0.1% | 100.1% | 5460/5465 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `building4` | high | measured | 2026-05-06T04:15:06 | building4-6-johnny6-v072c-prefetch-relief | +7.6% | 93.0% | 2985/2774 | 285 | 51 | 40 | clean-snapshot relief restores stage1_window prefetch versus fresh failure baseline; accepted hidden-refill tradeoff remains |
| `building4` | low | measured | 2026-05-06T04:15:06 | building4-6-johnny6-v072c-prefetch-relief | +7.1% | 93.4% | 2981/2784 | 199 | 119 | 14 | clean-snapshot relief restores stage1_window prefetch versus fresh failure baseline; accepted hidden-refill tradeoff remains |
| `building5` | high | measured | 2026-05-06T02:48:00 | stale-layout-v072c-current-refresh | +0.4% | 99.6% | 3359/3346 | 20 | 20 | 0 | current validated pack refresh; baseline correction |
| `building5` | low | measured | 2026-05-06T02:48:00 | stale-layout-v072c-current-refresh | +0.4% | 99.6% | 3359/3347 | 19 | 19 | 0 | current validated pack refresh; baseline correction |
| `building6` | high | measured | 2026-05-06T04:15:06 | building4-6-johnny6-v072c-prefetch-relief | +3.2% | 96.9% | 2520/2442 | 62 | 64 | 1 | clean-snapshot relief restores stage1_window prefetch versus fresh failure baseline; accepted hidden-refill tradeoff remains |
| `building6` | low | measured | 2026-05-06T04:15:06 | building4-6-johnny6-v072c-prefetch-relief | +3.2% | 96.9% | 2515/2437 | 70 | 66 | 2 | clean-snapshot relief restores stage1_window prefetch versus fresh failure baseline; accepted hidden-refill tradeoff remains |
| `building7` | high | measured | 2026-05-06T02:48:00 | stale-layout-v072c-current-refresh | -0.0% | 100.0% | 3132/3133 | 9 | 9 | 0 | current validated pack refresh; baseline correction |
| `building7` | low | measured | 2026-05-06T02:48:00 | stale-layout-v072c-current-refresh | -0.1% | 100.1% | 3130/3133 | 7 | 7 | 0 | current validated pack refresh; baseline correction |
| `fishing1` | high | measured | 2026-05-06T01:14:22 | stale-zero2-v072b-current-refresh | -0.6% | 100.6% | 1068/1074 | 2 | 2 | 0 | current validated pack refresh; baseline correction |
| `fishing1` | low | measured | 2026-05-06T01:14:22 | stale-zero2-v072b-current-refresh | -0.7% | 100.7% | 1067/1074 | 1 | 1 | 0 | current validated pack refresh; baseline correction |
| `fishing2` | high | measured | 2026-05-06T03:26:10 | stale-next-v072c-current-refresh | -0.1% | 100.1% | 1761/1763 | 6 | 6 | 0 | current validated pack refresh; baseline correction |
| `fishing2` | low | measured | 2026-05-06T03:26:10 | stale-next-v072c-current-refresh | -0.3% | 100.3% | 1759/1765 | 3 | 3 | 0 | current validated pack refresh; baseline correction |
| `fishing3` | high | measured | 2026-05-01T07:40:38 | compact-fgp3-v66-final-frame-hold | +0.4% | 99.6% | 1960/1952 | 18 | 13 | 1 |  |
| `fishing3` | low | measured | 2026-05-01T07:40:38 | compact-fgp3-v66-final-frame-hold | +0.1% | 99.9% | 1956/1954 | 6 | 6 | 0 |  |
| `fishing4` | high | measured | 2026-05-06T04:41:05 | activity4-fishing4-v072c-prefetch-relief | -0.8% | 100.8% | 835/842 | 2 | 2 | 0 | clean-snapshot relief keeps FISHING4 high under target with stage1_window prefetch restored |
| `fishing4` | low | measured | 2026-05-06T04:41:05 | activity4-fishing4-v072c-prefetch-relief | -1.1% | 101.1% | 834/843 | 0 | 0 | 0 | clean-snapshot relief collapses FISHING4 low due misses and makes the row CD-clean |
| `fishing5` | high | measured | 2026-05-02T22:39:34 | fishing5-v065-current-ledger-overlay | -0.6% | 100.6% | 885/890 | 0 | 0 | 0 |  |
| `fishing5` | low | measured | 2026-05-02T22:39:34 | fishing5-v065-current-ledger-overlay | -0.6% | 100.6% | 885/890 | 0 | 0 | 0 |  |
| `fishing6` | high | measured | 2026-05-06T00:38:12 | stale-top-v072b-current-refresh | -1.2% | 101.2% | 744/753 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `fishing6` | low | measured | 2026-05-06T00:38:12 | stale-top-v072b-current-refresh | -1.2% | 101.2% | 744/753 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `fishing7` | high | measured | 2026-05-05T23:47:29 | mismatch-top-v072-current-refresh | -1.4% | 101.4% | 715/725 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `fishing7` | low | measured | 2026-05-05T23:47:29 | mismatch-top-v072-current-refresh | -1.4% | 101.4% | 715/725 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `fishing8` | high | measured | 2026-05-06T01:14:22 | stale-zero2-v072b-current-refresh | -0.8% | 100.8% | 1243/1253 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `fishing8` | low | measured | 2026-05-06T01:14:22 | stale-zero2-v072b-current-refresh | -0.8% | 100.8% | 1243/1253 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `johnny1` | high | measured | 2026-05-06T02:08:50 | johnny1-v072c-prefetch-relief | +1.5% | 98.5% | 1974/1944 | 27 | 27 | 0 | johnny1 clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff |
| `johnny1` | low | measured | 2026-05-06T02:08:50 | johnny1-v072c-prefetch-relief | +1.5% | 98.5% | 1974/1944 | 27 | 27 | 0 | johnny1 clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff |
| `johnny2` | high | measured | 2026-05-02T19:06:01 | johnny2-v064-validation-refresh | +0.6% | 99.4% | 1761/1751 | 16 | 0 | 3 | validated v0.6.4 refresh; island-pos -64 54; correctness clean |
| `johnny2` | low | measured | 2026-05-02T19:06:01 | johnny2-v064-validation-refresh | +0.5% | 99.5% | 1758/1750 | 16 | 1 | 3 | validated v0.6.4 refresh; island-pos -64 54; correctness clean |
| `johnny3` | high | measured | 2026-05-06T01:47:28 | activity10-johnny3-v072-prefetch-relief | -0.3% | 100.3% | 1158/1161 | 10 | 6 | 1 |  |
| `johnny3` | low | measured | 2026-05-06T01:47:28 | activity10-johnny3-v072-prefetch-relief | -0.8% | 100.8% | 1157/1166 | 0 | 0 | 0 |  |
| `johnny4` | high | measured | 2026-05-06T01:14:22 | stale-zero2-v072b-current-refresh | -0.8% | 100.8% | 1204/1214 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `johnny4` | low | measured | 2026-05-06T01:14:22 | stale-zero2-v072b-current-refresh | -0.8% | 100.8% | 1204/1214 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `johnny5` | high | measured | 2026-05-06T00:55:09 | stale-zero-v072b-current-refresh | -1.1% | 101.1% | 811/820 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `johnny5` | low | measured | 2026-05-06T00:55:09 | stale-zero-v072b-current-refresh | -1.2% | 101.2% | 810/820 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `johnny6` | high | measured | 2026-05-06T04:15:06 | building4-6-johnny6-v072c-prefetch-relief | +1.1% | 98.9% | 2832/2800 | 28 | 28 | 0 | clean-snapshot relief restores stage1_window prefetch versus fresh current row |
| `johnny6` | low | measured | 2026-05-06T04:15:06 | building4-6-johnny6-v072c-prefetch-relief | +1.1% | 98.9% | 2832/2800 | 28 | 28 | 0 | clean-snapshot relief restores stage1_window prefetch versus fresh current row |
| `mary1` | high | measured | 2026-05-06T03:05:20 | mary1-v072c-prefetch-relief | +0.8% | 99.2% | 4867/4830 | 47 | 37 | 2 | mary1 clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff |
| `mary1` | low | measured | 2026-05-06T03:05:20 | mary1-v072c-prefetch-relief | +0.4% | 99.6% | 4860/4840 | 31 | 24 | 1 | mary1 clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff |
| `mary2` | high | measured | 2026-05-03T14:22:57 | mary2-v068-wide-stitch | +0.2% | 99.8% | 2250/2246 | 4 | 4 | 0 | validated v0.6.8 wide multi-view stitch; perf route uses island-pos -154 54; far-right and true far-left visual stress passed |
| `mary2` | low | measured | 2026-05-03T14:22:57 | mary2-v068-wide-stitch | +0.3% | 99.7% | 2253/2246 | 7 | 7 | 0 | validated v0.6.8 wide multi-view stitch; perf route uses island-pos -154 54; far-right and true far-left visual stress passed |
| `mary3` | high | measured | 2026-04-29T17:45:25 | compact-fgp3-v2-fullmatrix | - | - | 0/0 | 0 | 0 | 0 | validated 2026-05-03 after x=80 full-frame foreground-only recapture and low-memory clean-snapshot relief; active-loop timing still needs refresh |
| `mary3` | low | measured | 2026-04-29T17:45:37 | compact-fgp3-v2-fullmatrix | - | - | 0/0 | 0 | 0 | 0 | validated 2026-05-03 after x=80 full-frame foreground-only recapture and low-memory clean-snapshot relief; active-loop timing still needs refresh |
| `mary4` | high | measured | 2026-04-29T17:46:07 | compact-fgp3-v2-fullmatrix | -2.4% | 102.4% | 1968/2016 | 28 | 12 | 3 | validated 2026-05-03 after generic multi-view stitch; active timing predates refreshed pack; far-right x=300 visual stress passed |
| `mary4` | low | measured | 2026-04-29T17:46:13 | compact-fgp3-v2-fullmatrix | -2.6% | 102.7% | 1966/2019 | 24 | 10 | 3 | validated 2026-05-03 after generic multi-view stitch; active timing predates refreshed pack; far-right x=300 visual stress passed |
| `mary5` | high | measured | 2026-05-06T03:26:10 | stale-next-v072c-current-refresh | +0.6% | 99.4% | 1591/1582 | 8 | 8 | 0 | current validated pack refresh; baseline correction |
| `mary5` | low | measured | 2026-05-06T03:26:10 | stale-next-v072c-current-refresh | +0.5% | 99.5% | 1590/1582 | 7 | 7 | 0 | current validated pack refresh; baseline correction |
| `miscgag1` | high | measured | 2026-05-06T00:55:09 | stale-zero-v072b-current-refresh | -0.8% | 100.8% | 953/961 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `miscgag1` | low | measured | 2026-05-06T00:55:09 | stale-zero-v072b-current-refresh | -0.8% | 100.8% | 953/961 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `miscgag2` | high | measured | 2026-04-30T06:58:15 | compact-fgp3-v31-auto224 | -0.3% | 100.3% | 1352/1356 | 0 | 0 | 0 |  |
| `miscgag2` | low | measured | 2026-04-30T06:58:15 | compact-fgp3-v31-auto224 | -0.3% | 100.3% | 1352/1356 | 0 | 0 | 0 |  |
| `stand1` | high | measured | 2026-05-05T23:17:25 | stand1-v072-current-refresh | -4.0% | 104.1% | 194/202 | 0 | 0 | 0 | current validated 18-entry host-deadline pack refresh; baseline correction |
| `stand1` | low | measured | 2026-05-05T23:17:25 | stand1-v072-current-refresh | -4.0% | 104.1% | 194/202 | 0 | 0 | 0 | current validated 18-entry host-deadline pack refresh; baseline correction |
| `stand2` | high | measured | 2026-05-05T23:34:06 | stand-family-v072-current-refresh | -2.0% | 102.1% | 480/490 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand2` | low | measured | 2026-05-05T23:34:06 | stand-family-v072-current-refresh | -2.0% | 102.1% | 480/490 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand3` | high | measured | 2026-05-06T00:38:12 | stale-top-v072b-current-refresh | -1.8% | 101.8% | 547/557 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand3` | low | measured | 2026-05-06T00:38:12 | stale-top-v072b-current-refresh | -1.8% | 101.8% | 547/557 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand4` | high | measured | 2026-05-06T01:14:22 | stale-zero2-v072b-current-refresh | -1.5% | 101.5% | 1202/1220 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand4` | low | measured | 2026-05-06T01:14:22 | stale-zero2-v072b-current-refresh | -1.2% | 101.2% | 1203/1218 | 3 | 3 | 0 | current validated pack refresh; baseline correction |
| `stand5` | high | measured | 2026-05-06T01:14:22 | stale-zero2-v072b-current-refresh | -1.2% | 101.2% | 1442/1460 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand5` | low | measured | 2026-05-06T01:14:22 | stale-zero2-v072b-current-refresh | -1.2% | 101.2% | 1442/1460 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand6` | high | measured | 2026-05-06T01:14:22 | stale-zero2-v072b-current-refresh | -1.3% | 101.3% | 1346/1364 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand6` | low | measured | 2026-05-06T01:14:22 | stale-zero2-v072b-current-refresh | -1.3% | 101.3% | 1346/1364 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand7` | high | measured | 2026-05-05T23:34:06 | stand-family-v072-current-refresh | -3.3% | 103.5% | 520/538 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand7` | low | measured | 2026-05-05T23:34:06 | stand-family-v072-current-refresh | -3.3% | 103.5% | 520/538 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand8` | high | measured | 2026-05-05T23:34:06 | stand-family-v072-current-refresh | -3.2% | 103.3% | 483/499 | 2 | 2 | 0 | current validated pack refresh; baseline correction |
| `stand8` | low | measured | 2026-05-05T23:34:06 | stand-family-v072-current-refresh | -3.2% | 103.3% | 483/499 | 2 | 2 | 0 | current validated pack refresh; baseline correction |
| `stand9` | high | measured | 2026-05-05T23:34:06 | stand-family-v072-current-refresh | -3.3% | 103.5% | 520/538 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand9` | low | measured | 2026-05-05T23:34:06 | stand-family-v072-current-refresh | -3.0% | 103.1% | 522/538 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand10` | high | measured | 2026-05-05T23:34:06 | stand-family-v072-current-refresh | -1.9% | 101.9% | 528/538 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand10` | low | measured | 2026-05-05T23:34:06 | stand-family-v072-current-refresh | -1.9% | 101.9% | 528/538 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand11` | high | measured | 2026-05-05T23:34:06 | stand-family-v072-current-refresh | -1.9% | 101.9% | 528/538 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand11` | low | measured | 2026-05-05T23:34:06 | stand-family-v072-current-refresh | -1.9% | 101.9% | 528/538 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand12` | high | measured | 2026-05-06T01:14:22 | stale-zero2-v072b-current-refresh | -0.6% | 100.6% | 1450/1459 | 1 | 1 | 0 | current validated pack refresh; baseline correction |
| `stand12` | low | measured | 2026-05-06T01:14:22 | stale-zero2-v072b-current-refresh | -0.7% | 100.7% | 1450/1460 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand15` | high | measured | 2026-05-06T00:55:09 | stale-zero-v072b-current-refresh | -1.8% | 101.8% | 444/452 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand15` | low | measured | 2026-05-06T00:55:09 | stale-zero-v072b-current-refresh | -1.8% | 101.8% | 444/452 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand16` | high | measured | 2026-05-06T01:14:22 | stale-zero2-v072b-current-refresh | +0.2% | 99.8% | 473/472 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `stand16` | low | measured | 2026-05-06T01:14:22 | stale-zero2-v072b-current-refresh | +0.2% | 99.8% | 473/472 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `suzy1` | high | measured | 2026-04-29T18:01:51 | compact-fgp3-v2-fullmatrix | - | - | 0/6 | 0 | 0 | 0 | metadata-only; no active-loop timing; excluded from speed averages |
| `suzy1` | low | measured | 2026-04-29T18:01:58 | compact-fgp3-v2-fullmatrix | - | - | 0/6 | 0 | 0 | 0 | metadata-only; no active-loop timing; excluded from speed averages |
| `suzy2` | high | measured | 2026-04-29T18:02:29 | compact-fgp3-v2-fullmatrix | - | - | 0/6 | 0 | 0 | 0 | metadata-only; no active-loop timing; excluded from speed averages |
| `suzy2` | low | measured | 2026-04-29T18:02:35 | compact-fgp3-v2-fullmatrix | - | - | 0/6 | 0 | 0 | 0 | metadata-only; no active-loop timing; excluded from speed averages |
| `visitor1` | high | measured | 2026-05-05T23:47:29 | mismatch-top-v072-current-refresh | -0.7% | 100.7% | 672/677 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `visitor1` | low | measured | 2026-05-05T23:47:29 | mismatch-top-v072-current-refresh | -0.7% | 100.7% | 672/677 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `visitor3` | high | measured | 2026-05-05T22:51:29 | visitor3-v072-prefetch-relief | +44.1% | 69.4% | 1455/1010 | 363 | 21 | 31 | validated v0.7 pack; visitor3 clean-snapshot relief exception restores stage1_window prefetch against fresh current baseline with accepted hidden-refill tradeoff |
| `visitor3` | low | measured | 2026-05-05T22:51:29 | visitor3-v072-prefetch-relief | +44.0% | 69.4% | 1453/1009 | 365 | 23 | 32 | validated v0.7 pack; visitor3 clean-snapshot relief exception restores stage1_window prefetch against fresh current baseline with accepted hidden-refill tradeoff |
| `visitor4` | high | measured | 2026-05-05T23:27:49 | visitor4-v072-current-refresh | -0.9% | 100.9% | 424/428 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `visitor4` | low | measured | 2026-05-05T23:27:49 | visitor4-v072-current-refresh | -0.9% | 100.9% | 424/428 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `visitor5` | high | measured | 2026-05-06T00:23:01 | visitor5-v072-prefetch-relief | +1.9% | 98.1% | 1111/1090 | 12 | 12 | 0 | validated pack; visitor5 clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff |
| `visitor5` | low | measured | 2026-05-06T00:23:01 | visitor5-v072-prefetch-relief | +2.0% | 98.0% | 1112/1090 | 12 | 12 | 0 | validated pack; visitor5 clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff |
| `visitor6` | high | measured | 2026-05-06T02:48:00 | stale-layout-v072c-current-refresh | -0.2% | 100.2% | 2043/2047 | 1 | 1 | 0 | current validated pack refresh; baseline correction |
| `visitor6` | low | measured | 2026-05-06T02:48:00 | stale-layout-v072c-current-refresh | -0.2% | 100.2% | 2043/2047 | 1 | 1 | 0 | current validated pack refresh; baseline correction |
| `visitor7` | high | measured | 2026-05-06T02:48:00 | stale-layout-v072c-current-refresh | -0.4% | 100.4% | 1619/1625 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `visitor7` | low | measured | 2026-05-06T02:48:00 | stale-layout-v072c-current-refresh | -0.4% | 100.4% | 1619/1625 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `walkstuf1` | high | measured | 2026-05-05T22:25:30 | walkstuf1-v072-prefetch-relief | +16.1% | 86.1% | 1637/1410 | 297 | 67 | 54 | validated FGP2/pal4 pack; walkstuf1 clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff |
| `walkstuf1` | low | measured | 2026-05-05T22:25:30 | walkstuf1-v072-prefetch-relief | +16.0% | 86.2% | 1634/1409 | 304 | 73 | 55 | validated FGP2/pal4 pack; walkstuf1 clean-snapshot relief exception restores stage1_window prefetch with accepted hidden-refill tradeoff |
| `walkstuf2` | high | measured | 2026-05-06T00:38:12 | stale-top-v072b-current-refresh | -2.2% | 102.2% | 451/461 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `walkstuf2` | low | measured | 2026-05-06T00:38:12 | stale-top-v072b-current-refresh | -2.2% | 102.2% | 451/461 | 0 | 0 | 0 | current validated pack refresh; baseline correction |
| `walkstuf3` | high | measured | 2026-05-06T02:23:47 | stale-pressure2-v072c-current-refresh | +1.9% | 98.1% | 2321/2278 | 68 | 36 | 6 | current validated pack refresh after longer noloop window; baseline correction |
| `walkstuf3` | low | measured | 2026-05-06T02:23:47 | stale-pressure2-v072c-current-refresh | +1.1% | 98.9% | 2321/2295 | 40 | 20 | 5 | current validated pack refresh after longer noloop window; baseline correction |

## How to read this

**Status column.**

- `validated` — clears the FISHING 1 bar across every applicable
  variant. Frame-diff and SFX-cue diff both clean.
- `in-bring-up` — the scene's FG2 pack loops without dropping frames
  and the tide variant looks correct, but it has not yet been signed
  off as fishing-1-bar-equivalent. The next likely promotion to
  `validated`.
- `pending` — has not been put through the regtest harness yet, or
  has been but isn't passing. The default state for any scene that
  hasn't had attention paid to it.
- `blocked` — currently can't even be brought up for evaluation. Some
  upstream issue (asset routing, dispatch table, or harness bug) is
  preventing the scene from running at all. Until that's lifted, the
  scene can't be validated.

**Variant flags** (used on the per-scene pages, derived from the source
table):

- `night` — dusk/night palette swap (BOOTMODE `night 1`).
- `low-tide` — tide-state variant; the shoreline geometry shifts
  (BOOTMODE `lowtide 1`).
- `holiday` — holiday overlay variants (christmas, halloween, etc.;
  BOOTMODE `holiday N`).
- `raft-stage` — cumulative raft-build state; the raft sprite gains
  parts as the player progresses (BOOTMODE `raft-stage N`).

For the caption text, mapping confidences, and which captions are
"least-bad guesses" rather than confirmed matches, see the
[caption audit]({{ '/docs/captions/' | relative_url }}). For the running
log of which scenes graduated and when, see
[the devlog]({{ '/devlog/' | relative_url }}). For source-level artifacts,
start at the [source library]({{ '/source/' | relative_url }}) and
[resource catalog]({{ '/resources/' | relative_url }}).
