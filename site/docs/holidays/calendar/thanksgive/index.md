---
layout: page
title: "Thanksgiving"
eyebrow: "Holiday 35"
subtitle: "4th Thursday of November"
description: "Thanksgiving: 4th Thursday of November. Cornucopia with fruit."
---

## When it falls

4th Thursday of November — computed via Nth-weekday-of-month.

Worked dates:

| Year | Date |
|------|------|
| 2026 | November 26 |
| 2030 | November 28 |
| 2050 | November 24 |

## What it shows on the island

From [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml):

> Cornucopia with fruit.

## Sprite

Dimensions: **32 × 32** pixels. Anchored at island position **(404, 284)**.

Added by this port via the [holiday expansion]({{ '/docs/holidays/' | relative_url }}). `existing_sprite: null`. The visual is a 32x32 transparent emblem authored by an AI sub-agent in the shared 16-color CLUT defined in `scripts/holidays_art_lib.py`. It overlays the island corner during the holiday's window rather than reskinning the scene.

## Palette

<span style="display:inline-block;width:1em;height:1em;background:#7A3A1A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#7A3A1A</code>, <span style="display:inline-block;width:1em;height:1em;background:#C99A2A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#C99A2A</code>, <span style="display:inline-block;width:1em;height:1em;background:#8A1A1A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#8A1A1A</code>

Three-color hint from `holidays.yml`. This is a guide for the codegen step and the emblem authoring pass; the actual emblem samples from the shared 16-color CLUT, and the original Sierra sprites use the original game palette directly.

## Algorithm

Computed by `holidayNthWeekdayOfMonth(n, weekday, month, year)`, which calls `holidayDayOfWeek` (Tomohiko Sakamoto / Zeller). See [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}#nth-weekday-of-month).

## Emblem

This holiday has an emblem on the shared sheet at cell 31, pixel offset (224, 96). View the packed sheet on GitHub: [`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png). Full index: [Emblem gallery]({{ '/docs/holidays/emblems/' | relative_url }}).

## Source

- Row in [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) (search for `id: 35` or `short_name: "THANKSGIVE"`).
- Generated row in [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c).
