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

Current battle-card rollup as of 2026-04-30:

| Metric | Value |
|---|---:|
| Scenes visually validated | `2 / 63` |
| Validated scenes | `fishing1`, `fishing2` |
| Scene/tide variants routed through headless perf | `126 / 126` |
| Timing-bearing variants | `120 / 126` |
| Scenes with at least one active-loop timed variant | `60 / 63` |
| Scenes with both high/low variants measured | `63 / 63` |
| Pending variants | `0 / 126` |
| Blocked variants | `0 / 126` |
| Timing-bearing average over target | `+14.8%` |
| Timing-bearing average target speed | `88.0%` |
| Latest perf matrix run | `2026-04-30T13:25:26` |
| Stats version | mixed: latest rows use `compact-fgp3-v47-fishing4low-auto-prime`; earlier follow-up rows use `compact-fgp3-v46-johnny4low-auto-prime` through `compact-fgp3-v3-stand12low`; full-matrix baseline rows remain `compact-fgp3-v2-fullmatrix` |
| FISHING 1 canary | `1207 / 1076 VBlanks`, `+12.2%`, `89.1% target speed`, `blocking_vb=0` |

The durable numeric source is
[`docs/ps1/performance-scene-matrix.csv`]({{ site.github_url }}/blob/main/docs/ps1/performance-scene-matrix.csv).
The experiment trail is
[`docs/ps1/performance-experiment-log.md`]({{ site.github_url }}/blob/main/docs/ps1/performance-experiment-log.md).
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
  rows use `compact-fgp3-v47-fishing4low-auto-prime`; earlier follow-up rows use
  `compact-fgp3-v46-johnny4low-auto-prime` through `compact-fgp3-v3-stand12low`; the
  complete full-matrix baseline remains `compact-fgp3-v2-fullmatrix` for
  rows not rerun since that pass.

### 126-variant battle card

| Scene | Tide | Status | Latest Run | Stats Version | Over Target | Target Speed | VBlanks | Blocking | Prefetch | Due | Notes |
|---|---|---|---|---|---:|---:|---:|---:|---:|---:|---|
| `activity1` | high | measured | 2026-04-29T16:47:26 | compact-fgp3-v2-fullmatrix | +3.1% | 97.0% | 4373/4243 | 1 | 1 | 0 |  |
| `activity1` | low | measured | 2026-04-29T16:49:09 | compact-fgp3-v2-fullmatrix | +3.1% | 97.0% | 4373/4243 | 1 | 1 | 0 |  |
| `activity4` | high | measured | 2026-04-30T08:08:06 | compact-fgp3-v34-visitor1high-prime | +12.4% | 88.9% | 1202/1069 | 0 | 0 | 0 |  |
| `activity4` | low | measured | 2026-04-30T06:58:15 | compact-fgp3-v31-auto224 | +12.4% | 88.9% | 1202/1069 | 0 | 0 | 0 |  |
| `activity5` | high | measured | 2026-04-29T16:52:48 | compact-fgp3-v2-fullmatrix | +9.4% | 91.4% | 1867/1707 | 27 | 10 | 4 |  |
| `activity5` | low | measured | 2026-04-29T16:53:53 | compact-fgp3-v2-fullmatrix | +9.0% | 91.8% | 1860/1707 | 26 | 12 | 3 |  |
| `activity6` | high | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +14.5% | 87.3% | 1043/911 | 0 | 0 | 0 |  |
| `activity6` | low | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +14.5% | 87.3% | 1043/911 | 0 | 0 | 0 |  |
| `activity7` | high | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +21.3% | 82.4% | 746/615 | 0 | 0 | 0 |  |
| `activity7` | low | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +21.3% | 82.4% | 746/615 | 0 | 0 | 0 |  |
| `activity8` | high | measured | 2026-04-30T13:07:10 | compact-fgp3-v45-activity8-auto-prime | +15.2% | 86.8% | 1043/905 | 0 | 0 | 0 |  |
| `activity8` | low | measured | 2026-04-30T13:07:10 | compact-fgp3-v45-activity8-auto-prime | +15.2% | 86.8% | 1043/905 | 0 | 0 | 0 |  |
| `activity9` | high | measured | 2026-04-29T17:00:26 | compact-fgp3-v2-fullmatrix | +11.0% | 90.1% | 2267/2042 | 96 | 59 | 10 |  |
| `activity9` | low | measured | 2026-04-29T17:01:37 | compact-fgp3-v2-fullmatrix | +11.2% | 90.0% | 2272/2044 | 94 | 58 | 8 |  |
| `activity10` | high | measured | 2026-04-30T07:23:55 | compact-fgp3-v33-auto288 | +10.9% | 90.2% | 1399/1262 | 0 | 0 | 0 |  |
| `activity10` | low | measured | 2026-04-29T17:03:48 | compact-fgp3-v2-fullmatrix | +11.4% | 89.8% | 1401/1258 | 14 | 4 | 2 |  |
| `activity11` | high | measured | 2026-04-30T06:58:15 | compact-fgp3-v31-auto224 | +7.8% | 92.8% | 1859/1725 | 0 | 0 | 0 |  |
| `activity11` | low | measured | 2026-04-30T06:58:15 | compact-fgp3-v31-auto224 | +7.8% | 92.8% | 1859/1725 | 0 | 0 | 0 |  |
| `activity12` | high | measured | 2026-04-30T08:59:42 | compact-fgp3-v38-activity12high-prime | +9.0% | 91.7% | 1543/1415 | 0 | 0 | 0 |  |
| `activity12` | low | measured | 2026-04-29T17:08:49 | compact-fgp3-v2-fullmatrix | +9.4% | 91.4% | 1543/1411 | 8 | 4 | 1 |  |
| `building1` | high | measured | 2026-04-29T17:09:55 | compact-fgp3-v2-fullmatrix | +23.3% | 81.1% | 951/771 | 63 | 25 | 8 |  |
| `building1` | low | measured | 2026-04-29T17:10:45 | compact-fgp3-v2-fullmatrix | +19.9% | 83.4% | 935/780 | 37 | 18 | 4 |  |
| `building2` | high | measured | 2026-04-29T17:11:35 | compact-fgp3-v2-fullmatrix | +20.2% | 83.2% | 1559/1297 | 150 | 45 | 19 |  |
| `building2` | low | measured | 2026-04-29T17:12:34 | compact-fgp3-v2-fullmatrix | +20.1% | 83.3% | 1556/1296 | 150 | 42 | 20 |  |
| `building3` | high | measured | 2026-04-29T17:13:36 | compact-fgp3-v2-fullmatrix | +9.4% | 91.4% | 1565/1430 | 5 | 5 | 0 |  |
| `building3` | low | measured | 2026-04-29T17:14:35 | compact-fgp3-v2-fullmatrix | +9.6% | 91.3% | 1571/1434 | 0 | 0 | 0 |  |
| `building4` | high | measured | 2026-04-29T17:15:34 | compact-fgp3-v2-fullmatrix | +12.2% | 89.2% | 3128/2789 | 355 | 21 | 49 |  |
| `building4` | low | measured | 2026-04-29T17:20:47 | compact-fgp3-v2-fullmatrix | +12.1% | 89.2% | 3126/2788 | 353 | 21 | 49 |  |
| `building5` | high | measured | 2026-04-29T17:20:47 | compact-fgp3-v2-fullmatrix | +5.0% | 95.2% | 3504/3336 | 52 | 25 | 6 |  |
| `building5` | low | measured | 2026-04-29T17:24:22 | compact-fgp3-v2-fullmatrix | +4.5% | 95.7% | 3498/3348 | 16 | 11 | 2 |  |
| `building6` | high | measured | 2026-04-29T17:24:22 | compact-fgp3-v2-fullmatrix | +12.9% | 88.6% | 2744/2431 | 342 | 9 | 49 |  |
| `building6` | low | measured | 2026-04-29T17:25:44 | compact-fgp3-v2-fullmatrix | +12.9% | 88.6% | 2747/2433 | 343 | 11 | 49 |  |
| `building7` | high | measured | 2026-04-29T17:25:59 | compact-fgp3-v2-fullmatrix | +4.8% | 95.4% | 3843/3668 | 43 | 24 | 4 |  |
| `building7` | low | measured | 2026-04-29T17:27:03 | compact-fgp3-v2-fullmatrix | +4.2% | 96.0% | 3830/3676 | 12 | 12 | 0 |  |
| `fishing1` | high | measured | 2026-04-30T13:25:26 | compact-fgp3-v47-fishing4low-auto-prime | +12.2% | 89.1% | 1207/1076 | 0 | 0 | 0 |  |
| `fishing1` | low | measured | 2026-04-29T17:28:34 | compact-fgp3-v2-fullmatrix | +12.2% | 89.1% | 1207/1076 | 0 | 0 | 0 |  |
| `fishing2` | high | measured | 2026-04-29T17:28:52 | compact-fgp3-v2-fullmatrix | +7.6% | 92.9% | 1899/1765 | 3 | 3 | 0 |  |
| `fishing2` | low | measured | 2026-04-29T17:29:29 | compact-fgp3-v2-fullmatrix | +7.4% | 93.1% | 1898/1767 | 0 | 0 | 0 |  |
| `fishing3` | high | measured | 2026-04-30T09:33:43 | compact-fgp3-v39-visitor7high-prime | +7.3% | 93.2% | 2095/1952 | 20 | 15 | 1 |  |
| `fishing3` | low | measured | 2026-04-29T17:30:34 | compact-fgp3-v2-fullmatrix | +6.6% | 93.8% | 2090/1960 | 3 | 3 | 0 |  |
| `fishing4` | high | measured | 2026-04-30T13:25:26 | compact-fgp3-v47-fishing4low-auto-prime | +14.7% | 87.2% | 967/843 | 0 | 0 | 0 |  |
| `fishing4` | low | measured | 2026-04-30T13:24:28 | compact-fgp3-v47-fishing4low-auto-prime | +14.7% | 87.2% | 967/843 | 0 | 0 | 0 |  |
| `fishing5` | high | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | -9.4% | 110.4% | 807/891 | 0 | 0 | 0 |  |
| `fishing5` | low | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | -9.5% | 110.5% | 806/891 | 0 | 0 | 0 |  |
| `fishing6` | high | measured | 2026-04-30T09:00:55 | compact-fgp3-v38-activity12high-prime | +18.2% | 84.6% | 890/753 | 0 | 0 | 0 |  |
| `fishing6` | low | measured | 2026-04-30T07:13:22 | compact-fgp3-v32-auto256 | +18.2% | 84.6% | 890/753 | 0 | 0 | 0 |  |
| `fishing7` | high | measured | 2026-04-30T09:00:55 | compact-fgp3-v38-activity12high-prime | +18.3% | 84.5% | 858/725 | 0 | 0 | 0 |  |
| `fishing7` | low | measured | 2026-04-30T06:58:15 | compact-fgp3-v31-auto224 | +18.3% | 84.5% | 858/725 | 0 | 0 | 0 |  |
| `fishing8` | high | measured | 2026-04-29T17:34:39 | compact-fgp3-v2-fullmatrix | +11.9% | 89.4% | 1393/1245 | 13 | 14 | 0 |  |
| `fishing8` | low | measured | 2026-04-30T07:23:55 | compact-fgp3-v33-auto288 | +10.1% | 90.8% | 1380/1253 | 0 | 0 | 0 |  |
| `johnny1` | high | measured | 2026-04-29T17:35:40 | compact-fgp3-v2-fullmatrix | +9.4% | 91.4% | 2125/1942 | 31 | 31 | 0 |  |
| `johnny1` | low | measured | 2026-04-29T17:36:10 | compact-fgp3-v2-fullmatrix | +9.6% | 91.2% | 2129/1942 | 33 | 33 | 0 |  |
| `johnny2` | high | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +7.3% | 93.2% | 1878/1751 | 0 | 0 | 0 |  |
| `johnny2` | low | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +7.3% | 93.2% | 1878/1751 | 0 | 0 | 0 |  |
| `johnny3` | high | measured | 2026-04-30T09:00:55 | compact-fgp3-v38-activity12high-prime | +11.3% | 89.8% | 1298/1166 | 0 | 0 | 0 |  |
| `johnny3` | low | measured | 2026-04-30T06:58:15 | compact-fgp3-v31-auto224 | +11.3% | 89.8% | 1298/1166 | 0 | 0 | 0 |  |
| `johnny4` | high | measured | 2026-04-30T13:18:41 | compact-fgp3-v46-johnny4low-auto-prime | +10.5% | 90.5% | 1341/1214 | 0 | 0 | 0 |  |
| `johnny4` | low | measured | 2026-04-30T13:17:30 | compact-fgp3-v46-johnny4low-auto-prime | +10.5% | 90.5% | 1341/1214 | 0 | 0 | 0 |  |
| `johnny5` | high | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +15.5% | 86.6% | 947/820 | 0 | 0 | 0 |  |
| `johnny5` | low | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +15.5% | 86.6% | 947/820 | 0 | 0 | 0 |  |
| `johnny6` | high | measured | 2026-04-29T17:40:45 | compact-fgp3-v2-fullmatrix | +3.4% | 96.7% | 2895/2800 | 27 | 27 | 0 |  |
| `johnny6` | low | measured | 2026-04-29T17:41:03 | compact-fgp3-v2-fullmatrix | +3.4% | 96.7% | 2896/2800 | 27 | 27 | 0 |  |
| `mary1` | high | measured | 2026-04-29T17:42:09 | compact-fgp3-v2-fullmatrix | +3.7% | 96.4% | 5004/4826 | 49 | 40 | 2 |  |
| `mary1` | low | measured | 2026-04-29T17:42:28 | compact-fgp3-v2-fullmatrix | +3.2% | 96.9% | 4994/4839 | 26 | 20 | 1 |  |
| `mary2` | high | measured | 2026-04-29T17:44:11 | compact-fgp3-v2-fullmatrix | +1.6% | 98.4% | 2284/2247 | 7 | 7 | 0 |  |
| `mary2` | low | measured | 2026-04-29T17:44:28 | compact-fgp3-v2-fullmatrix | +1.6% | 98.4% | 2285/2249 | 6 | 6 | 0 |  |
| `mary3` | high | measured | 2026-04-29T17:45:25 | compact-fgp3-v2-fullmatrix | - | - | 0/0 | 0 | 0 | 0 | metadata-only; no active-loop timing; excluded from speed averages |
| `mary3` | low | measured | 2026-04-29T17:45:37 | compact-fgp3-v2-fullmatrix | - | - | 0/0 | 0 | 0 | 0 | metadata-only; no active-loop timing; excluded from speed averages |
| `mary4` | high | measured | 2026-04-29T17:46:07 | compact-fgp3-v2-fullmatrix | -2.4% | 102.4% | 1968/2016 | 28 | 12 | 3 |  |
| `mary4` | low | measured | 2026-04-29T17:46:13 | compact-fgp3-v2-fullmatrix | -2.6% | 102.7% | 1966/2019 | 24 | 10 | 3 |  |
| `mary5` | high | measured | 2026-04-29T17:47:11 | compact-fgp3-v2-fullmatrix | +6.6% | 93.8% | 1687/1583 | 7 | 7 | 0 |  |
| `mary5` | low | measured | 2026-04-29T17:47:23 | compact-fgp3-v2-fullmatrix | +6.6% | 93.8% | 1688/1583 | 8 | 8 | 0 |  |
| `miscgag1` | high | measured | 2026-04-30T07:13:22 | compact-fgp3-v32-auto256 | +14.2% | 87.6% | 1097/961 | 0 | 0 | 0 |  |
| `miscgag1` | low | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +14.0% | 87.7% | 1096/961 | 0 | 0 | 0 |  |
| `miscgag2` | high | measured | 2026-04-30T06:58:15 | compact-fgp3-v31-auto224 | -0.3% | 100.3% | 1352/1356 | 0 | 0 | 0 |  |
| `miscgag2` | low | measured | 2026-04-30T06:58:15 | compact-fgp3-v31-auto224 | -0.3% | 100.3% | 1352/1356 | 0 | 0 | 0 |  |
| `stand1` | high | measured | 2026-04-30T05:35:49 | compact-fgp3-v29-smallprime | +69.3% | 59.1% | 342/202 | 0 | 0 | 0 |  |
| `stand1` | low | measured | 2026-04-30T05:35:49 | compact-fgp3-v29-smallprime | +69.3% | 59.1% | 342/202 | 0 | 0 | 0 |  |
| `stand2` | high | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +27.5% | 78.4% | 626/491 | 0 | 0 | 0 |  |
| `stand2` | low | measured | 2026-04-30T05:35:49 | compact-fgp3-v29-smallprime | +27.5% | 78.4% | 626/491 | 0 | 0 | 0 |  |
| `stand3` | high | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +24.4% | 80.4% | 694/558 | 0 | 0 | 0 |  |
| `stand3` | low | measured | 2026-04-30T05:33:36 | compact-fgp3-v29-smallprime | +24.4% | 80.4% | 694/558 | 0 | 0 | 0 |  |
| `stand4` | high | measured | 2026-04-30T07:13:22 | compact-fgp3-v32-auto256 | +11.3% | 89.8% | 1359/1221 | 0 | 0 | 0 |  |
| `stand4` | low | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +11.2% | 89.9% | 1358/1221 | 0 | 0 | 0 |  |
| `stand5` | high | measured | 2026-04-30T07:23:55 | compact-fgp3-v33-auto288 | +9.2% | 91.6% | 1595/1461 | 0 | 0 | 0 |  |
| `stand5` | low | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +9.1% | 91.7% | 1594/1461 | 0 | 0 | 0 |  |
| `stand6` | high | measured | 2026-04-30T07:23:55 | compact-fgp3-v33-auto288 | +10.0% | 90.9% | 1501/1365 | 0 | 0 | 0 |  |
| `stand6` | low | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +9.9% | 91.0% | 1500/1365 | 0 | 0 | 0 |  |
| `stand7` | high | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +25.0% | 80.0% | 674/539 | 0 | 0 | 0 |  |
| `stand7` | low | measured | 2026-04-30T05:35:49 | compact-fgp3-v29-smallprime | +25.1% | 79.9% | 673/538 | 0 | 0 | 0 |  |
| `stand8` | high | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +26.9% | 78.8% | 636/501 | 0 | 0 | 0 |  |
| `stand8` | low | measured | 2026-04-30T05:35:49 | compact-fgp3-v29-smallprime | +27.0% | 78.7% | 635/500 | 0 | 0 | 0 |  |
| `stand9` | high | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +25.0% | 80.0% | 674/539 | 0 | 0 | 0 |  |
| `stand9` | low | measured | 2026-04-30T05:35:49 | compact-fgp3-v29-smallprime | +25.1% | 79.9% | 673/538 | 0 | 0 | 0 |  |
| `stand10` | high | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +25.0% | 80.0% | 674/539 | 0 | 0 | 0 |  |
| `stand10` | low | measured | 2026-04-30T05:35:49 | compact-fgp3-v29-smallprime | +25.1% | 79.9% | 673/538 | 0 | 0 | 0 |  |
| `stand11` | high | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +25.1% | 79.9% | 673/538 | 0 | 0 | 0 |  |
| `stand11` | low | measured | 2026-04-30T05:35:49 | compact-fgp3-v29-smallprime | +25.1% | 79.9% | 673/538 | 0 | 0 | 0 |  |
| `stand12` | high | measured | 2026-04-30T07:23:55 | compact-fgp3-v33-auto288 | +9.1% | 91.7% | 1594/1461 | 0 | 0 | 0 |  |
| `stand12` | low | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +9.1% | 91.7% | 1593/1460 | 0 | 0 | 0 |  |
| `stand15` | high | measured | 2026-04-30T06:58:15 | compact-fgp3-v31-auto224 | +13.4% | 88.2% | 1123/990 | 0 | 0 | 0 |  |
| `stand15` | low | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +13.4% | 88.1% | 1122/989 | 0 | 0 | 0 |  |
| `stand16` | high | measured | 2026-04-30T07:23:55 | compact-fgp3-v33-auto288 | +11.0% | 90.1% | 1322/1191 | 0 | 0 | 0 |  |
| `stand16` | low | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +11.1% | 90.0% | 1323/1191 | 0 | 0 | 0 |  |
| `suzy1` | high | measured | 2026-04-29T18:01:51 | compact-fgp3-v2-fullmatrix | - | - | 0/6 | 0 | 0 | 0 | metadata-only; no active-loop timing; excluded from speed averages |
| `suzy1` | low | measured | 2026-04-29T18:01:58 | compact-fgp3-v2-fullmatrix | - | - | 0/6 | 0 | 0 | 0 | metadata-only; no active-loop timing; excluded from speed averages |
| `suzy2` | high | measured | 2026-04-29T18:02:29 | compact-fgp3-v2-fullmatrix | - | - | 0/6 | 0 | 0 | 0 | metadata-only; no active-loop timing; excluded from speed averages |
| `suzy2` | low | measured | 2026-04-29T18:02:35 | compact-fgp3-v2-fullmatrix | - | - | 0/6 | 0 | 0 | 0 | metadata-only; no active-loop timing; excluded from speed averages |
| `visitor1` | high | measured | 2026-04-30T08:11:17 | compact-fgp3-v34-visitor1high-prime | +18.8% | 84.2% | 804/677 | 0 | 0 | 0 |  |
| `visitor1` | low | measured | 2026-04-30T07:13:22 | compact-fgp3-v32-auto256 | +17.3% | 85.3% | 794/677 | 0 | 0 | 0 |  |
| `visitor3` | high | measured | 2026-04-29T18:04:12 | compact-fgp3-v2-fullmatrix | +50.5% | 66.4% | 1526/1014 | 368 | 133 | 23 |  |
| `visitor3` | low | measured | 2026-04-29T18:04:14 | compact-fgp3-v2-fullmatrix | +51.4% | 66.1% | 1547/1022 | 332 | 111 | 22 |  |
| `visitor4` | high | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +32.7% | 75.4% | 568/428 | 0 | 0 | 0 |  |
| `visitor4` | low | measured | 2026-04-30T06:13:51 | compact-fgp3-v30-mediumprime | +32.7% | 75.4% | 568/428 | 0 | 0 | 0 |  |
| `visitor5` | high | measured | 2026-04-29T18:06:00 | compact-fgp3-v2-fullmatrix | +17.6% | 85.0% | 1274/1083 | 79 | 34 | 9 |  |
| `visitor5` | low | measured | 2026-04-29T18:06:05 | compact-fgp3-v2-fullmatrix | +14.3% | 87.5% | 1244/1088 | 49 | 14 | 6 |  |
| `visitor6` | high | measured | 2026-04-29T18:08:18 | compact-fgp3-v2-fullmatrix | +7.5% | 93.0% | 2195/2042 | 13 | 13 | 0 |  |
| `visitor6` | low | measured | 2026-04-30T07:23:55 | compact-fgp3-v33-auto288 | +6.8% | 93.6% | 2188/2048 | 0 | 0 | 0 |  |
| `visitor7` | high | measured | 2026-04-30T09:32:23 | compact-fgp3-v39-visitor7high-prime | +8.7% | 92.0% | 1766/1625 | 0 | 0 | 0 |  |
| `visitor7` | low | measured | 2026-04-30T09:33:43 | compact-fgp3-v39-visitor7high-prime | +8.7% | 92.0% | 1766/1625 | 0 | 0 | 0 |  |
| `walkstuf1` | high | measured | 2026-04-30T12:25:17 | compact-fgp3-v44-walkstuf1-high-window54 | +42.2% | 70.3% | 2024/1423 | 449 | 133 | 25 |  |
| `walkstuf1` | low | measured | 2026-04-30T12:26:37 | compact-fgp3-v44-walkstuf1-high-window54 | +44.8% | 69.0% | 2048/1414 | 460 | 139 | 34 |  |
| `walkstuf2` | high | measured | 2026-04-29T18:12:10 | compact-fgp3-v2-fullmatrix | +29.6% | 77.2% | 596/460 | 1 | 1 | 0 |  |
| `walkstuf2` | low | measured | 2026-04-29T18:12:28 | compact-fgp3-v2-fullmatrix | +29.6% | 77.2% | 596/460 | 1 | 1 | 0 |  |
| `walkstuf3` | high | measured | 2026-04-29T18:12:55 | compact-fgp3-v2-fullmatrix | +8.1% | 92.5% | 2460/2276 | 79 | 45 | 6 |  |
| `walkstuf3` | low | measured | 2026-04-29T18:13:13 | compact-fgp3-v2-fullmatrix | +7.9% | 92.7% | 2466/2285 | 66 | 38 | 5 |  |

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
