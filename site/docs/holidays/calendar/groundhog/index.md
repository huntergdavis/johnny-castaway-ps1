---
layout: page
title: "Groundhog Day"
eyebrow: "Holiday 7"
subtitle: "February 2 — fixed date"
description: "Groundhog Day: February 2 — fixed date. Groundhog head popping from a burrow."
---

## When it falls

February 2 — fixed date.

Worked dates:

| Year | Date |
|------|------|
| 2026 | February 2 |
| 2030 | February 2 |
| 2050 | February 2 |

## What it shows on the island

From [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml):

> Groundhog head popping from a burrow.

## Sprite

Dimensions: **32 × 32** pixels. Anchored at island position **(404, 284)**.

Added by this port via the [holiday expansion]({{ '/docs/holidays/' | relative_url }}). `existing_sprite: null`. The visual is a 32x32 transparent emblem authored by an AI sub-agent in the shared 16-color CLUT defined in `scripts/holidays_art_lib.py`. It overlays the island corner during the holiday's window rather than reskinning the scene.

## Palette

<span style="display:inline-block;width:1em;height:1em;background:#7A4A2A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#7A4A2A</code>, <span style="display:inline-block;width:1em;height:1em;background:#E08A3C;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#E08A3C</code>, <span style="display:inline-block;width:1em;height:1em;background:#8A8A8A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#8A8A8A</code>

Three-color hint from `holidays.yml`. This is a guide for the codegen step and the emblem authoring pass; the actual emblem samples from the shared 16-color CLUT, and the original Sierra sprites use the original game palette directly.

## Algorithm

Fixed date — no computation; the `HOLIDAY_KIND_FIXED` dispatch in `holidayForDate` compares `month` and `day` directly. See [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}).

## Emblem

This holiday has an emblem on the shared sheet at cell 2, pixel offset (64, 0). View the packed sheet on GitHub: [`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png). Full index: [Emblem gallery]({{ '/docs/holidays/emblems/' | relative_url }}).

## Source

- Row in [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) (search for `id: 7` or `short_name: "GROUNDHOG"`).
- Generated row in [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c).
