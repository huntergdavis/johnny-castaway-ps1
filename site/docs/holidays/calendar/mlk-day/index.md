---
layout: page
title: "MLK Jr. Day"
eyebrow: "Holiday 6"
subtitle: "3rd Monday of January"
description: "MLK Jr. Day: 3rd Monday of January. Tiny podium with speech paper and dove."
---

## When it falls

3rd Monday of January — computed via Nth-weekday-of-month.

Worked dates:

| Year | Date |
|------|------|
| 2026 | January 19 |
| 2030 | January 21 |
| 2050 | January 17 |

## What it shows on the island

From [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml):

> Tiny podium with speech paper and dove.

## Sprite

Dimensions: **32 × 32** pixels. Anchored at island position **(404, 284)**.

Added by this port via the [holiday expansion]({{ '/docs/holidays/' | relative_url }}). `existing_sprite: null`. The visual is a 32x32 transparent emblem authored by an AI sub-agent in the shared 16-color CLUT defined in `scripts/holidays_art_lib.py`. It overlays the island corner during the holiday's window rather than reskinning the scene.

## Palette

<span style="display:inline-block;width:1em;height:1em;background:#7B1F2A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#7B1F2A</code>, <span style="display:inline-block;width:1em;height:1em;background:#F5E8C8;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#F5E8C8</code>, <span style="display:inline-block;width:1em;height:1em;background:#C9A227;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#C9A227</code>

Three-color hint from `holidays.yml`. This is a guide for the codegen step and the emblem authoring pass; the actual emblem samples from the shared 16-color CLUT, and the original Sierra sprites use the original game palette directly.

## Algorithm

Computed by `holidayNthWeekdayOfMonth(n, weekday, month, year)`, which calls `holidayDayOfWeek` (Tomohiko Sakamoto / Zeller). See [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}#nth-weekday-of-month).

## Emblem

This holiday has an emblem on the shared sheet at cell 1, pixel offset (32, 0). View the packed sheet on GitHub: [`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png). Full index: [Emblem gallery]({{ '/docs/holidays/emblems/' | relative_url }}).

## Source

- Row in [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) (search for `id: 6` or `short_name: "MLK DAY"`).
- Generated row in [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c).
