---
layout: page
title: Current Testing Status
eyebrow: Live progress
subtitle: The current scene-validation and headless performance battle card for the PS1 port.
description: Current Johnny Castaway PS1 testing status: scene validation, headless performance progress, and the full 126-variant timing battle card.
---

This is the working status page. It tracks two different bars:

- **Scene validation** is human visual + audible signoff. That is the release
  bar for "done."
- **Headless performance** is automated DuckStation timing. That is the
  optimization battle card for "how fast does this scene/tide variant run."

Do not mix them. A scene can be timed here without being visually certified.

## Rollup

Current battle-card rollup as of 2026-04-29:

| Metric | Value |
|---|---:|
| Scenes visually validated | `2 / 63` |
| Validated scenes | `fishing1`, `fishing2` |
| Scene/tide variants timed | `57 / 126` |
| Scenes with at least one timed variant | `34 / 63` |
| Scenes with both high/low variants timed | `23 / 63` |
| Pending variants | `67 / 126` |
| Blocked variants | `2 / 126` |
| Measured average over target | `+12.1%` |
| Measured average target speed | `89.8%` |
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

## Reading The Table

- **Over Target**: how far `loop_vb` is above the captured target timing.
  Lower is better.
- **Target Speed**: `target_vb / loop_vb`. `100%` means exact target cadence.
- **VBlanks**: `loop_vb/target_vb`.
- **Blocking**: visible CD/blocking VBlanks.
- **Prefetch**: prefetch overrun VBlanks.
- **Due**: due-frame misses.

## 126-Variant Battle Card

| Scene | Tide | Status | Over Target | Target Speed | VBlanks | Blocking | Prefetch | Due | Notes |
|---|---|---|---:|---:|---:|---:|---:|---:|---|
| `activity1` | high | measured | +3.0% | 97.1% | 4373/4244 | 0 | 0 | 0 | padded FGP3 |
| `activity1` | low | measured | +3.1% | 97.0% | 4373/4242 | 2 | 2 | 0 | padded FGP3 |
| `activity4` | high | measured | +12.9% | 88.5% | 1205/1067 | 5 | 5 | 0 | padded FGP3 |
| `activity4` | low | measured | +12.5% | 88.9% | 1203/1069 | 1 | 1 | 0 | padded FGP3 |
| `activity5` | high | measured | +9.3% | 91.5% | 1866/1707 | 25 | 8 | 4 | padded FGP3 |
| `activity5` | low | measured | +9.1% | 91.7% | 1861/1706 | 29 | 16 | 3 | padded FGP3 |
| `activity6` | high | measured | +14.5% | 87.3% | 1043/911 | 0 | 0 | 0 | padded FGP3 |
| `activity6` | low | measured | +14.5% | 87.3% | 1043/911 | 0 | 0 | 0 | padded FGP3; loop-validated because noloop shutdown hits post-scene artifact |
| `activity7` | high | measured | +22.7% | 81.5% | 751/612 | 8 | 8 | 0 | padded FGP3 |
| `activity7` | low | measured | +21.7% | 82.2% | 747/614 | 1 | 1 | 0 | padded FGP3 |
| `activity8` | high | pending | - | - | - | - | - | - |  |
| `activity8` | low | pending | - | - | - | - | - | - |  |
| `activity9` | high | measured | +11.2% | 89.9% | 2268/2039 | 97 | 60 | 9 | padded FGP3; loop-validated because noloop shutdown hits post-scene artifact |
| `activity9` | low | measured | +11.1% | 90.0% | 2271/2044 | 92 | 56 | 8 | padded FGP3; loop-validated because noloop shutdown hits post-scene artifact |
| `activity10` | high | measured | +12.3% | 89.1% | 1408/1254 | 18 | 13 | 1 | padded FGP3 |
| `activity10` | low | measured | +12.2% | 89.1% | 1408/1255 | 23 | 14 | 2 | padded FGP3 |
| `activity11` | high | pending | - | - | - | - | - | - |  |
| `activity11` | low | pending | - | - | - | - | - | - |  |
| `activity12` | high | measured | +9.9% | 91.0% | 1551/1411 | 12 | 12 | 0 | padded FGP3 |
| `activity12` | low | measured | +10.2% | 90.8% | 1551/1408 | 20 | 14 | 1 | padded FGP3 |
| `building1` | high | measured | +25.6% | 79.6% | 966/769 | 76 | 52 | 6 | padded FGP3; prefetch_overrun tradeoff +2 accepted |
| `building1` | low | pending | - | - | - | - | - | - |  |
| `building2` | high | measured | +21.8% | 82.1% | 1572/1291 | 173 | 66 | 20 | padded FGP3 |
| `building2` | low | measured | +21.4% | 82.4% | 1570/1293 | 172 | 62 | 20 | padded FGP3 |
| `building3` | high | measured | +9.4% | 91.4% | 1565/1431 | 4 | 4 | 0 | padded FGP3 |
| `building3` | low | pending | - | - | - | - | - | - |  |
| `building4` | high | measured | +12.5% | 88.9% | 3141/2792 | 326 | 43 | 39 | padded FGP3 |
| `building4` | low | measured | +12.4% | 89.0% | 3135/2790 | 335 | 46 | 40 | padded FGP3 |
| `building5` | high | measured | +5.4% | 94.9% | 3514/3334 | 65 | 36 | 6 | padded FGP3 |
| `building5` | low | measured | +4.8% | 95.4% | 3508/3346 | 33 | 23 | 2 | padded FGP3 |
| `building6` | high | measured | +13.1% | 88.4% | 2754/2435 | 317 | 43 | 39 | padded FGP3 |
| `building6` | low | measured | +12.9% | 88.6% | 2746/2433 | 312 | 55 | 38 | padded FGP3; loop-validated because noloop shutdown hits post-scene invalid-read spam |
| `building7` | high | measured | +5.5% | 94.8% | 3861/3659 | 66 | 47 | 3 | padded FGP3 |
| `building7` | low | measured | +4.5% | 95.7% | 3837/3672 | 23 | 23 | 0 | padded FGP3 |
| `fishing1` | high | measured | +12.2% | 89.1% | 1207/1076 | 0 | 0 | 0 |  |
| `fishing1` | low | measured | +12.2% | 89.1% | 1207/1076 | 0 | 0 | 0 |  |
| `fishing2` | high | measured | +7.5% | 93.0% | 1898/1765 | 2 | 2 | 0 |  |
| `fishing2` | low | measured | +7.4% | 93.1% | 1898/1767 | 0 | 0 | 0 |  |
| `fishing3` | high | measured | +7.1% | 93.4% | 2093/1955 | 15 | 10 | 1 |  |
| `fishing3` | low | measured | +6.6% | 93.8% | 2090/1960 | 3 | 3 | 0 |  |
| `fishing4` | high | pending | - | - | - | - | - | - |  |
| `fishing4` | low | pending | - | - | - | - | - | - |  |
| `fishing5` | high | pending | - | - | - | - | - | - |  |
| `fishing5` | low | pending | - | - | - | - | - | - |  |
| `fishing6` | high | pending | - | - | - | - | - | - |  |
| `fishing6` | low | pending | - | - | - | - | - | - |  |
| `fishing7` | high | pending | - | - | - | - | - | - |  |
| `fishing7` | low | pending | - | - | - | - | - | - |  |
| `fishing8` | high | measured | +12.4% | 88.9% | 1400/1245 | 21 | 21 | 0 | padded FGP3 |
| `fishing8` | low | pending | - | - | - | - | - | - |  |
| `johnny1` | high | measured | +9.5% | 91.3% | 2128/1943 | 31 | 31 | 0 | padded FGP3; manual accept because strict gate only failed zero-baseline prefetch_overrun |
| `johnny1` | low | measured | +9.8% | 91.1% | 2132/1942 | 37 | 37 | 0 | padded FGP3; manual accept because strict gate only failed zero-baseline prefetch_overrun |
| `johnny2` | high | pending | - | - | - | - | - | - |  |
| `johnny2` | low | pending | - | - | - | - | - | - |  |
| `johnny3` | high | measured | +12.8% | 88.7% | 1308/1160 | 20 | 16 | 1 | padded FGP3 |
| `johnny3` | low | measured | +12.2% | 89.1% | 1305/1163 | 10 | 10 | 0 | padded FGP3 |
| `johnny4` | high | pending | - | - | - | - | - | - |  |
| `johnny4` | low | pending | - | - | - | - | - | - |  |
| `johnny5` | high | pending | - | - | - | - | - | - |  |
| `johnny5` | low | pending | - | - | - | - | - | - |  |
| `johnny6` | high | measured | +3.6% | 96.5% | 2901/2799 | 33 | 33 | 0 | padded FGP3; manually accepted because only prefetch_overrun_vb regressed while loop and blocking improved materially |
| `johnny6` | low | measured | +3.8% | 96.3% | 2905/2798 | 37 | 37 | 0 | padded FGP3; manually accepted because only prefetch_overrun_vb regressed while loop and blocking improved materially |
| `mary1` | high | measured | +4.5% | 95.7% | 5028/4813 | 87 | 75 | 2 | padded FGP3 |
| `mary1` | low | measured | +3.7% | 96.4% | 5011/4830 | 50 | 45 | 1 | padded FGP3 |
| `mary2` | high | measured | +1.7% | 98.3% | 2286/2248 | 8 | 8 | 0 | padded FGP3 |
| `mary2` | low | measured | +1.7% | 98.3% | 2286/2248 | 8 | 8 | 0 | padded FGP3 |
| `mary3` | high | pending | - | - | - | - | - | - |  |
| `mary3` | low | pending | - | - | - | - | - | - |  |
| `mary4` | high | pending | - | - | - | - | - | - |  |
| `mary4` | low | pending | - | - | - | - | - | - |  |
| `mary5` | high | measured | +7.4% | 93.1% | 1698/1581 | 20 | 20 | 0 | padded FGP3 |
| `mary5` | low | measured | +7.0% | 93.4% | 1694/1583 | 14 | 14 | 0 | padded FGP3 |
| `miscgag1` | high | pending | - | - | - | - | - | - |  |
| `miscgag1` | low | pending | - | - | - | - | - | - |  |
| `miscgag2` | high | pending | - | - | - | - | - | - |  |
| `miscgag2` | low | pending | - | - | - | - | - | - |  |
| `stand1` | high | pending | - | - | - | - | - | - |  |
| `stand1` | low | pending | - | - | - | - | - | - |  |
| `stand2` | high | pending | - | - | - | - | - | - |  |
| `stand2` | low | pending | - | - | - | - | - | - |  |
| `stand3` | high | pending | - | - | - | - | - | - |  |
| `stand3` | low | pending | - | - | - | - | - | - |  |
| `stand4` | high | measured | +12.3% | 89.1% | 1365/1216 | 12 | 12 | 0 | padded FGP3 |
| `stand4` | low | pending | - | - | - | - | - | - |  |
| `stand5` | high | measured | +9.7% | 91.2% | 1600/1459 | 8 | 8 | 0 | padded FGP3 |
| `stand5` | low | pending | - | - | - | - | - | - |  |
| `stand6` | high | measured | +10.5% | 90.5% | 1503/1360 | 8 | 8 | 0 | padded FGP3 |
| `stand6` | low | pending | - | - | - | - | - | - |  |
| `stand7` | high | pending | - | - | - | - | - | - |  |
| `stand7` | low | pending | - | - | - | - | - | - |  |
| `stand8` | high | pending | - | - | - | - | - | - |  |
| `stand8` | low | pending | - | - | - | - | - | - |  |
| `stand9` | high | pending | - | - | - | - | - | - |  |
| `stand9` | low | pending | - | - | - | - | - | - |  |
| `stand10` | high | pending | - | - | - | - | - | - |  |
| `stand10` | low | pending | - | - | - | - | - | - |  |
| `stand11` | high | pending | - | - | - | - | - | - |  |
| `stand11` | low | pending | - | - | - | - | - | - |  |
| `stand12` | high | measured | +9.7% | 91.2% | 1597/1456 | 8 | 8 | 0 | padded FGP3 |
| `stand12` | low | pending | - | - | - | - | - | - |  |
| `stand15` | high | pending | - | - | - | - | - | - |  |
| `stand15` | low | pending | - | - | - | - | - | - |  |
| `stand16` | high | pending | - | - | - | - | - | - |  |
| `stand16` | low | pending | - | - | - | - | - | - |  |
| `suzy1` | high | blocked | - | - | - | - | - | - | baseline coverage gap: pack loads but loop_vb=0/last_frame=0; do not convert until fgpilot SUZY playback is understood |
| `suzy1` | low | pending | - | - | - | - | - | - |  |
| `suzy2` | high | blocked | - | - | - | - | - | - | fgpilot coverage gap; pack loads but foreground loop never starts |
| `suzy2` | low | pending | - | - | - | - | - | - |  |
| `visitor1` | high | pending | - | - | - | - | - | - |  |
| `visitor1` | low | pending | - | - | - | - | - | - |  |
| `visitor3` | high | measured | +56.1% | 64.1% | 1581/1013 | 424 | 145 | 24 | padded FGP3; still CD-bound |
| `visitor3` | low | measured | +58.4% | 63.1% | 1611/1017 | 404 | 99 | 26 | padded FGP3; still CD-bound |
| `visitor4` | high | pending | - | - | - | - | - | - |  |
| `visitor4` | low | pending | - | - | - | - | - | - |  |
| `visitor5` | high | measured | +20.0% | 83.3% | 1295/1079 | 107 | 52 | 9 | padded FGP3; manually accepted because only prefetch_overrun_vb regressed while loop and blocking improved materially |
| `visitor5` | low | pending | - | - | - | - | - | - |  |
| `visitor6` | high | measured | +7.5% | 93.0% | 2198/2044 | 14 | 14 | 0 | padded FGP3 |
| `visitor6` | low | pending | - | - | - | - | - | - |  |
| `visitor7` | high | measured | +9.6% | 91.2% | 1777/1621 | 15 | 15 | 0 | padded FGP3 |
| `visitor7` | low | pending | - | - | - | - | - | - |  |
| `walkstuf1` | high | pending | - | - | - | - | - | - |  |
| `walkstuf1` | low | pending | - | - | - | - | - | - |  |
| `walkstuf2` | high | pending | - | - | - | - | - | - |  |
| `walkstuf2` | low | pending | - | - | - | - | - | - |  |
| `walkstuf3` | high | measured | +10.4% | 90.6% | 2512/2275 | 130 | 90 | 6 | padded FGP3 |
| `walkstuf3` | low | pending | - | - | - | - | - | - |  |
