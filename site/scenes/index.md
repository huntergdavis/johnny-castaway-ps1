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

Current battle-card rollup as of 2026-04-29:

| Metric | Value |
|---|---:|
| Scenes visually validated | `2 / 63` |
| Validated scenes | `fishing1`, `fishing2` |
| Scene/tide variants timed | `75 / 126` |
| Scenes with at least one timed variant | `45 / 63` |
| Scenes with both high/low variants timed | `30 / 63` |
| Pending variants | `49 / 126` |
| Blocked variants | `2 / 126` |
| Measured average over target | `+12.6%` |
| Measured average target speed | `89.3%` |
| Latest perf matrix run | `2026-04-29T12:56:40` |
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

### 126-variant battle card

| Scene | Tide | Status | Latest Run | Over Target | Target Speed | VBlanks | Blocking | Prefetch | Due | Notes |
|---|---|---|---|---:|---:|---:|---:|---:|---:|---|
| `activity1` | high | measured | 2026-04-28T21:34:02 | +3.0% | 97.1% | 4373/4244 | 0 | 0 | 0 | padded FGP3 |
| `activity1` | low | measured | 2026-04-28T22:15:22 | +3.1% | 97.0% | 4373/4242 | 2 | 2 | 0 | padded FGP3 |
| `activity4` | high | measured | 2026-04-28T21:27:47 | +12.9% | 88.5% | 1205/1067 | 5 | 5 | 0 | padded FGP3 |
| `activity4` | low | measured | 2026-04-28T22:06:21 | +12.5% | 88.9% | 1203/1069 | 1 | 1 | 0 | padded FGP3 |
| `activity5` | high | measured | 2026-04-28T21:11:41 | +9.3% | 91.5% | 1866/1707 | 25 | 8 | 4 | padded FGP3 |
| `activity5` | low | measured | 2026-04-28T21:57:32 | +9.1% | 91.7% | 1861/1706 | 29 | 16 | 3 | padded FGP3 |
| `activity6` | high | measured | 2026-04-28T21:20:58 | +14.5% | 87.3% | 1043/911 | 0 | 0 | 0 | padded FGP3 |
| `activity6` | low | measured | 2026-04-28T23:00:11 | +14.5% | 87.3% | 1043/911 | 0 | 0 | 0 | padded FGP3; loop-validated because noloop shutdown hits post-scene artifact |
| `activity7` | high | measured | 2026-04-28T20:54:38 | +22.7% | 81.5% | 751/612 | 8 | 8 | 0 | padded FGP3 |
| `activity7` | low | measured | 2026-04-28T21:43:07 | +21.7% | 82.2% | 747/614 | 1 | 1 | 0 | padded FGP3 |
| `activity8` | high | pending | - | - | - | - | - | - | - |  |
| `activity8` | low | pending | - | - | - | - | - | - | - |  |
| `activity9` | high | measured | 2026-04-28T22:50:08 | +11.2% | 89.9% | 2268/2039 | 97 | 60 | 9 | padded FGP3; loop-validated because noloop shutdown hits post-scene artifact |
| `activity9` | low | measured | 2026-04-28T23:13:29 | +11.1% | 90.0% | 2271/2044 | 92 | 56 | 8 | padded FGP3; loop-validated because noloop shutdown hits post-scene artifact |
| `activity10` | high | measured | 2026-04-29T05:10:31 | +12.3% | 89.1% | 1408/1254 | 18 | 13 | 1 | padded FGP3 |
| `activity10` | low | measured | 2026-04-29T03:28:40 | +12.2% | 89.1% | 1408/1255 | 23 | 14 | 2 | padded FGP3 |
| `activity11` | high | measured | 2026-04-29T10:34:38 | +8.1% | 92.5% | 1864/1724 | 9 | 5 | 1 | padded FGP3 |
| `activity11` | low | measured | 2026-04-29T10:48:17 | +8.3% | 92.3% | 1866/1723 | 10 | 5 | 1 | padded FGP3 |
| `activity12` | high | measured | 2026-04-29T05:25:38 | +9.9% | 91.0% | 1551/1411 | 12 | 12 | 0 | padded FGP3 |
| `activity12` | low | measured | 2026-04-29T03:39:20 | +10.2% | 90.8% | 1551/1408 | 20 | 14 | 1 | padded FGP3 |
| `building1` | high | measured | 2026-04-29T07:52:19 | +25.6% | 79.6% | 966/769 | 76 | 52 | 6 | padded FGP3; prefetch_overrun tradeoff +2 accepted |
| `building1` | low | measured | 2026-04-29T11:16:06 | +24.2% | 80.5% | 953/767 | 62 | 46 | 4 | padded FGP3; loop-validated; due_misses tradeoff 2->4 accepted |
| `building2` | high | measured | 2026-04-29T00:51:42 | +21.8% | 82.1% | 1572/1291 | 173 | 66 | 20 | padded FGP3 |
| `building2` | low | measured | 2026-04-29T01:03:17 | +21.4% | 82.4% | 1570/1293 | 172 | 62 | 20 | padded FGP3 |
| `building3` | high | measured | 2026-04-29T06:29:00 | +9.4% | 91.4% | 1565/1431 | 4 | 4 | 0 | padded FGP3 |
| `building3` | low | pending | - | - | - | - | - | - | - |  |
| `building4` | high | measured | 2026-04-28T23:31:51 | +12.5% | 88.9% | 3141/2792 | 326 | 43 | 39 | padded FGP3 |
| `building4` | low | measured | 2026-04-28T23:43:30 | +12.4% | 89.0% | 3135/2790 | 335 | 46 | 40 | padded FGP3 |
| `building5` | high | measured | 2026-04-29T02:23:18 | +5.4% | 94.9% | 3514/3334 | 65 | 36 | 6 | padded FGP3 |
| `building5` | low | measured | 2026-04-29T02:32:45 | +4.8% | 95.4% | 3508/3346 | 33 | 23 | 2 | padded FGP3 |
| `building6` | high | measured | 2026-04-29T01:15:17 | +13.1% | 88.4% | 2754/2435 | 317 | 43 | 39 | padded FGP3 |
| `building6` | low | measured | 2026-04-29T01:29:52 | +12.9% | 88.6% | 2746/2433 | 312 | 55 | 38 | padded FGP3; loop-validated because noloop shutdown hits post-scene invalid-read spam |
| `building7` | high | measured | 2026-04-29T02:04:56 | +5.5% | 94.8% | 3861/3659 | 66 | 47 | 3 | padded FGP3 |
| `building7` | low | measured | 2026-04-29T02:13:58 | +4.5% | 95.7% | 3837/3672 | 23 | 23 | 0 | padded FGP3 |
| `fishing1` | high | measured | 2026-04-29T12:56:40 | +12.2% | 89.1% | 1207/1076 | 0 | 0 | 0 |  |
| `fishing1` | low | measured | 2026-04-27T17:40:04 | +12.2% | 89.1% | 1207/1076 | 0 | 0 | 0 |  |
| `fishing2` | high | measured | 2026-04-28T17:40:29 | +7.5% | 93.0% | 1898/1765 | 2 | 2 | 0 |  |
| `fishing2` | low | measured | 2026-04-27T18:50:02 | +7.4% | 93.1% | 1898/1767 | 0 | 0 | 0 |  |
| `fishing3` | high | measured | 2026-04-28T17:36:58 | +7.1% | 93.4% | 2093/1955 | 15 | 10 | 1 |  |
| `fishing3` | low | measured | 2026-04-28T16:50:36 | +6.6% | 93.8% | 2090/1960 | 3 | 3 | 0 |  |
| `fishing4` | high | measured | 2026-04-29T10:23:32 | +16.7% | 85.7% | 978/838 | 15 | 15 | 0 | padded FGP3 |
| `fishing4` | low | pending | - | - | - | - | - | - | - |  |
| `fishing5` | high | pending | - | - | - | - | - | - | - |  |
| `fishing5` | low | pending | - | - | - | - | - | - | - |  |
| `fishing6` | high | measured | 2026-04-29T11:07:36 | +19.2% | 83.9% | 893/749 | 7 | 7 | 0 | padded FGP3; loop-validated because noloop shutdown hits post-scene invalid-read spam |
| `fishing6` | low | measured | 2026-04-29T11:44:20 | +19.6% | 83.6% | 897/750 | 10 | 10 | 0 | padded FGP3 |
| `fishing7` | high | measured | 2026-04-29T10:04:49 | +19.4% | 83.8% | 863/723 | 7 | 7 | 0 | padded FGP3 |
| `fishing7` | low | pending | - | - | - | - | - | - | - |  |
| `fishing8` | high | measured | 2026-04-29T04:19:49 | +12.4% | 88.9% | 1400/1245 | 21 | 21 | 0 | padded FGP3 |
| `fishing8` | low | measured | 2026-04-29T12:00:44 | +11.1% | 90.1% | 1387/1249 | 11 | 11 | 0 | padded FGP3 |
| `johnny1` | high | measured | 2026-04-28T23:54:52 | +9.5% | 91.3% | 2128/1943 | 31 | 31 | 0 | padded FGP3; manual accept because strict gate only failed zero-baseline prefetch_overrun |
| `johnny1` | low | measured | 2026-04-29T00:05:46 | +9.8% | 91.1% | 2132/1942 | 37 | 37 | 0 | padded FGP3; manual accept because strict gate only failed zero-baseline prefetch_overrun |
| `johnny2` | high | measured | 2026-04-29T12:09:01 | +7.3% | 93.2% | 1878/1750 | 1 | 1 | 0 | padded FGP3 |
| `johnny2` | low | measured | 2026-04-29T12:17:15 | +7.3% | 93.2% | 1878/1750 | 1 | 1 | 0 | padded FGP3 |
| `johnny3` | high | measured | 2026-04-29T05:42:22 | +12.8% | 88.7% | 1308/1160 | 20 | 16 | 1 | padded FGP3 |
| `johnny3` | low | measured | 2026-04-29T08:55:48 | +12.2% | 89.1% | 1305/1163 | 10 | 10 | 0 | padded FGP3 |
| `johnny4` | high | pending | - | - | - | - | - | - | - |  |
| `johnny4` | low | pending | - | - | - | - | - | - | - |  |
| `johnny5` | high | pending | - | - | - | - | - | - | - |  |
| `johnny5` | low | pending | - | - | - | - | - | - | - |  |
| `johnny6` | high | measured | 2026-04-29T01:46:20 | +3.6% | 96.5% | 2901/2799 | 33 | 33 | 0 | padded FGP3; manually accepted because only prefetch_overrun_vb regressed while loop and blocking improved materially |
| `johnny6` | low | measured | 2026-04-29T01:55:45 | +3.8% | 96.3% | 2905/2798 | 37 | 37 | 0 | padded FGP3; manually accepted because only prefetch_overrun_vb regressed while loop and blocking improved materially |
| `mary1` | high | measured | 2026-04-29T00:18:38 | +4.5% | 95.7% | 5028/4813 | 87 | 75 | 2 | padded FGP3 |
| `mary1` | low | measured | 2026-04-29T00:30:11 | +3.8% | 96.4% | 5011/4830 | 50 | 45 | 1 | padded FGP3 |
| `mary2` | high | measured | 2026-04-29T04:40:25 | +1.7% | 98.3% | 2286/2248 | 8 | 8 | 0 | padded FGP3 |
| `mary2` | low | measured | 2026-04-29T04:55:27 | +1.7% | 98.3% | 2286/2248 | 8 | 8 | 0 | padded FGP3 |
| `mary3` | high | pending | - | - | - | - | - | - | - |  |
| `mary3` | low | pending | - | - | - | - | - | - | - |  |
| `mary4` | high | measured | 2026-04-29T12:37:06 | -2.5% | 102.5% | 1969/2019 | 27 | 10 | 4 | FGP2 baseline; plain padded FGP3 rejected due prefetch-overrun regression |
| `mary4` | low | pending | - | - | - | - | - | - | - |  |
| `mary5` | high | measured | 2026-04-29T03:50:10 | +7.4% | 93.1% | 1698/1581 | 20 | 20 | 0 | padded FGP3 |
| `mary5` | low | measured | 2026-04-29T04:00:29 | +7.0% | 93.4% | 1694/1583 | 14 | 14 | 0 | padded FGP3 |
| `miscgag1` | high | measured | 2026-04-29T11:24:47 | +16.9% | 85.6% | 1115/954 | 26 | 26 | 0 | padded FGP3 |
| `miscgag1` | low | measured | 2026-04-29T11:35:00 | +14.6% | 87.3% | 1099/959 | 5 | 5 | 0 | padded FGP3 |
| `miscgag2` | high | pending | - | - | - | - | - | - | - |  |
| `miscgag2` | low | pending | - | - | - | - | - | - | - |  |
| `stand1` | high | pending | - | - | - | - | - | - | - |  |
| `stand1` | low | pending | - | - | - | - | - | - | - |  |
| `stand2` | high | pending | - | - | - | - | - | - | - |  |
| `stand2` | low | pending | - | - | - | - | - | - | - |  |
| `stand3` | high | pending | - | - | - | - | - | - | - |  |
| `stand3` | low | pending | - | - | - | - | - | - | - |  |
| `stand4` | high | measured | 2026-04-29T07:31:30 | +12.2% | 89.1% | 1365/1216 | 12 | 12 | 0 | padded FGP3 |
| `stand4` | low | pending | - | - | - | - | - | - | - |  |
| `stand5` | high | measured | 2026-04-29T06:44:33 | +9.7% | 91.2% | 1600/1459 | 8 | 8 | 0 | padded FGP3 |
| `stand5` | low | pending | - | - | - | - | - | - | - |  |
| `stand6` | high | measured | 2026-04-29T07:00:48 | +10.5% | 90.5% | 1503/1360 | 8 | 8 | 0 | padded FGP3 |
| `stand6` | low | pending | - | - | - | - | - | - | - |  |
| `stand7` | high | measured | 2026-04-29T12:54:14 | +26.1% | 79.3% | 676/536 | 5 | 5 | 0 | padded FGP3 |
| `stand7` | low | pending | - | - | - | - | - | - | - |  |
| `stand8` | high | pending | - | - | - | - | - | - | - |  |
| `stand8` | low | pending | - | - | - | - | - | - | - |  |
| `stand9` | high | pending | - | - | - | - | - | - | - |  |
| `stand9` | low | pending | - | - | - | - | - | - | - |  |
| `stand10` | high | pending | - | - | - | - | - | - | - |  |
| `stand10` | low | pending | - | - | - | - | - | - | - |  |
| `stand11` | high | pending | - | - | - | - | - | - | - |  |
| `stand11` | low | pending | - | - | - | - | - | - | - |  |
| `stand12` | high | measured | 2026-04-29T06:13:45 | +9.7% | 91.2% | 1597/1456 | 8 | 8 | 0 | padded FGP3 |
| `stand12` | low | pending | - | - | - | - | - | - | - |  |
| `stand15` | high | measured | 2026-04-29T12:30:59 | +15.2% | 86.8% | 1135/985 | 17 | 17 | 0 | padded FGP3 |
| `stand15` | low | pending | - | - | - | - | - | - | - |  |
| `stand16` | high | measured | 2026-04-29T10:13:31 | +11.7% | 89.5% | 1328/1189 | 7 | 7 | 0 | padded FGP3 |
| `stand16` | low | pending | - | - | - | - | - | - | - |  |
| `suzy1` | high | blocked | 2026-04-28T23:20:55 | - | - | - | - | - | - | baseline coverage gap: pack loads but loop_vb=0/last_frame=0; do not convert until fgpilot SUZY playback is understood |
| `suzy1` | low | pending | - | - | - | - | - | - | - |  |
| `suzy2` | high | blocked | 2026-04-29T02:57:54 | - | - | - | - | - | - | fgpilot coverage gap; pack loads but foreground loop never starts |
| `suzy2` | low | pending | - | - | - | - | - | - | - |  |
| `visitor1` | high | measured | 2026-04-29T09:38:48 | +22.5% | 81.6% | 822/671 | 22 | 22 | 0 | padded FGP3 |
| `visitor1` | low | pending | - | - | - | - | - | - | - |  |
| `visitor3` | high | measured | 2026-04-29T02:42:08 | +56.1% | 64.1% | 1581/1013 | 424 | 145 | 24 | padded FGP3; still CD-bound |
| `visitor3` | low | measured | 2026-04-29T02:51:12 | +58.4% | 63.1% | 1611/1017 | 404 | 99 | 26 | padded FGP3; still CD-bound |
| `visitor4` | high | pending | - | - | - | - | - | - | - |  |
| `visitor4` | low | pending | - | - | - | - | - | - | - |  |
| `visitor5` | high | measured | 2026-04-29T03:18:33 | +20.0% | 83.3% | 1295/1079 | 107 | 52 | 9 | padded FGP3; manually accepted because only prefetch_overrun_vb regressed while loop and blocking improved materially |
| `visitor5` | low | measured | 2026-04-29T10:57:15 | +15.2% | 86.8% | 1250/1085 | 62 | 27 | 6 | padded FGP3 |
| `visitor6` | high | measured | 2026-04-29T05:58:39 | +7.5% | 93.0% | 2198/2044 | 14 | 14 | 0 | padded FGP3 |
| `visitor6` | low | pending | - | - | - | - | - | - | - |  |
| `visitor7` | high | measured | 2026-04-29T07:16:36 | +9.6% | 91.2% | 1777/1621 | 15 | 15 | 0 | padded FGP3 |
| `visitor7` | low | pending | - | - | - | - | - | - | - |  |
| `walkstuf1` | high | pending | - | - | - | - | - | - | - |  |
| `walkstuf1` | low | pending | - | - | - | - | - | - | - |  |
| `walkstuf2` | high | pending | - | - | - | - | - | - | - |  |
| `walkstuf2` | low | pending | - | - | - | - | - | - | - |  |
| `walkstuf3` | high | measured | 2026-04-29T03:08:07 | +10.4% | 90.6% | 2512/2275 | 130 | 90 | 6 | padded FGP3 |
| `walkstuf3` | low | pending | - | - | - | - | - | - | - |  |

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
