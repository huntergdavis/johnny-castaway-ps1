---
layout: page
title: "Moon Landing Day"
eyebrow: "Holiday 25"
subtitle: "July 20 — fixed date"
description: "Moon Landing Day: July 20 — fixed date. Toy rocket and moon."
---

## When it falls

July 20 — fixed date.

Worked dates:

| Year | Date |
|------|------|
| 2026 | July 20 |
| 2030 | July 20 |
| 2050 | July 20 |

## What it shows on the island

From [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml):

> Toy rocket and moon.

## Sprite

Dimensions: **32 × 32** pixels. Anchored at island position **(404, 284)**.

Added by this port via the [holiday expansion]({{ '/docs/holidays/' | relative_url }}). `existing_sprite: null`. The visual is a 32x32 transparent emblem authored by an AI sub-agent in the shared 16-color CLUT defined in `scripts/holidays_art_lib.py`. It overlays the island corner during the holiday's window rather than reskinning the scene.

## Palette

<span style="display:inline-block;width:1em;height:1em;background:#7A8A9A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#7A8A9A</code>, <span style="display:inline-block;width:1em;height:1em;background:#1A1A4A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#1A1A4A</code>, <span style="display:inline-block;width:1em;height:1em;background:#F0EAD6;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#F0EAD6</code>

Three-color hint from `holidays.yml`. This is a guide for the codegen step and the emblem authoring pass; the actual emblem samples from the shared 16-color CLUT, and the original Sierra sprites use the original game palette directly.

## Algorithm

Fixed date — no computation; the `HOLIDAY_KIND_FIXED` dispatch in `holidayForDate` compares `month` and `day` directly. See [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}).

## Emblem

This holiday has an emblem on the shared sheet at cell 21, pixel offset (160, 64). View the packed sheet on GitHub: [`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png). Full index: [Emblem gallery]({{ '/docs/holidays/emblems/' | relative_url }}).

## Source

- Row in [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) (search for `id: 25` or `short_name: "MOON LAND"`).
- Generated row in [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c).
