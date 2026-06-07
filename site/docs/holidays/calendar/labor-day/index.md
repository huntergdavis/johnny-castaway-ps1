---
layout: page
title: "Labor Day"
eyebrow: "Holiday 29"
subtitle: "1st Monday of September"
description: "Labor Day: 1st Monday of September. Hard hat and tool."
---

## When it falls

1st Monday of September — computed via Nth-weekday-of-month.

Worked dates:

| Year | Date |
|------|------|
| 2026 | September 7 |
| 2030 | September 2 |
| 2050 | September 5 |

## What it shows on the island

From [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml):

> Hard hat and tool.

## Sprite

Dimensions: **32 × 32** pixels. Anchored at island position **(404, 284)**.

Added by this port via the [holiday expansion]({{ '/docs/holidays/' | relative_url }}). `existing_sprite: null`. The visual is a 32x32 transparent emblem authored by an AI sub-agent in the shared 16-color CLUT defined in `scripts/holidays_art_lib.py`. It overlays the island corner during the holiday's window rather than reskinning the scene.

## Palette

<span style="display:inline-block;width:1em;height:1em;background:#F5C518;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#F5C518</code>, <span style="display:inline-block;width:1em;height:1em;background:#3A5A8A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#3A5A8A</code>, <span style="display:inline-block;width:1em;height:1em;background:#5A3A1A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#5A3A1A</code>

Three-color hint from `holidays.yml`. This is a guide for the codegen step and the emblem authoring pass; the actual emblem samples from the shared 16-color CLUT, and the original Sierra sprites use the original game palette directly.

## Algorithm

Computed by `holidayNthWeekdayOfMonth(n, weekday, month, year)`, which calls `holidayDayOfWeek` (Tomohiko Sakamoto / Zeller). See [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}#nth-weekday-of-month).

## Emblem

This holiday has an emblem on the shared sheet at cell 25, pixel offset (32, 96). View the packed sheet on GitHub: [`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png). Full index: [Emblem gallery]({{ '/docs/holidays/emblems/' | relative_url }}).

## Source

- Row in [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) (search for `id: 29` or `short_name: "LABOR DAY"`).
- Generated row in [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c).
