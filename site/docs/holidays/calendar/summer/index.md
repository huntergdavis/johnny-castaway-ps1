---
layout: page
title: "First Day of Summer"
eyebrow: "Holiday 22"
subtitle: "Summer solstice"
description: "First Day of Summer: Summer solstice. Smiling summer sun with shades."
---

## When it falls

Summer solstice (around June 21) — fixed canonical date, ±1 day tolerance.

Worked dates:

| Year | Date |
|------|------|
| 2026 | June 21 |
| 2030 | June 21 |
| 2050 | June 21 |

## What it shows on the island

From [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml):

> Smiling summer sun with shades.

## Sprite

Dimensions: **32 × 32** pixels. Anchored at island position **(404, 284)**.

Added by this port via the [holiday expansion]({{ '/docs/holidays/' | relative_url }}). `existing_sprite: null`. The visual is a 32x32 transparent emblem authored by an AI sub-agent in the shared 16-color CLUT defined in `scripts/holidays_art_lib.py`. It overlays the island corner during the holiday's window rather than reskinning the scene.

## Palette

<span style="display:inline-block;width:1em;height:1em;background:#5AD0E8;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#5AD0E8</code>, <span style="display:inline-block;width:1em;height:1em;background:#FFD93D;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#FFD93D</code>, <span style="display:inline-block;width:1em;height:1em;background:#FFFFFF;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#FFFFFF</code>

Three-color hint from `holidays.yml`. This is a guide for the codegen step and the emblem authoring pass; the actual emblem samples from the shared 16-color CLUT, and the original Sierra sprites use the original game palette directly.

## Algorithm

Returned by `holidaySummerSolstice` / `holidayWinterSolstice` as a fixed canonical date. See [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}#solstice-and-equinox).

## Emblem

This holiday has an emblem on the shared sheet at cell 18, pixel offset (64, 64). View the packed sheet on GitHub: [`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png). Full index: [Emblem gallery]({{ '/docs/holidays/emblems/' | relative_url }}).

## Source

- Row in [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) (search for `id: 22` or `short_name: "SUMMER"`).
- Generated row in [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c).
