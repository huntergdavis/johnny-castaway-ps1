---
layout: page
title: "Mother's Day"
eyebrow: "Holiday 19"
subtitle: "2nd Sunday of May"
description: "Mother's Day: 2nd Sunday of May. Coconut vase bouquet."
---

## When it falls

2nd Sunday of May — computed via Nth-weekday-of-month.

Worked dates:

| Year | Date |
|------|------|
| 2026 | May 10 |
| 2030 | May 12 |
| 2050 | May 8 |

## What it shows on the island

From [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml):

> Coconut vase bouquet.

## Sprite

Dimensions: **32 × 32** pixels. Anchored at island position **(404, 284)**.

Added by this port via the [holiday expansion]({{ '/docs/holidays/' | relative_url }}). `existing_sprite: null`. The visual is a 32x32 transparent emblem authored by an AI sub-agent in the shared 16-color CLUT defined in `scripts/holidays_art_lib.py`. It overlays the island corner during the holiday's window rather than reskinning the scene.

## Palette

<span style="display:inline-block;width:1em;height:1em;background:#FFB6D9;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#FFB6D9</code>, <span style="display:inline-block;width:1em;height:1em;background:#C8B6FF;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#C8B6FF</code>, <span style="display:inline-block;width:1em;height:1em;background:#A8C8A8;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#A8C8A8</code>

Three-color hint from `holidays.yml`. This is a guide for the codegen step and the emblem authoring pass; the actual emblem samples from the shared 16-color CLUT, and the original Sierra sprites use the original game palette directly.

## Algorithm

Computed by `holidayNthWeekdayOfMonth(n, weekday, month, year)`, which calls `holidayDayOfWeek` (Tomohiko Sakamoto / Zeller). See [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}#nth-weekday-of-month).

## Emblem

This holiday has an emblem on the shared sheet at cell 15, pixel offset (224, 32). View the packed sheet on GitHub: [`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png). Full index: [Emblem gallery]({{ '/docs/holidays/emblems/' | relative_url }}).

## Source

- Row in [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) (search for `id: 19` or `short_name: "MOTHERS DAY"`).
- Generated row in [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c).
