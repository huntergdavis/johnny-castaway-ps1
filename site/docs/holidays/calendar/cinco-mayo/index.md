---
layout: page
title: "Cinco de Mayo"
eyebrow: "Holiday 18"
subtitle: "May 5 — fixed date"
description: "Cinco de Mayo: May 5 — fixed date. Sombrero and maraca."
---

## When it falls

May 5 — fixed date.

Worked dates:

| Year | Date |
|------|------|
| 2026 | May 5 |
| 2030 | May 5 |
| 2050 | May 5 |

## What it shows on the island

From [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml):

> Sombrero and maraca.

## Sprite

Dimensions: **32 × 32** pixels. Anchored at island position **(404, 284)**.

Added by this port via the [holiday expansion]({{ '/docs/holidays/' | relative_url }}). `existing_sprite: null`. The visual is a 32x32 transparent emblem authored by an AI sub-agent in the shared 16-color CLUT defined in `scripts/holidays_art_lib.py`. It overlays the island corner during the holiday's window rather than reskinning the scene.

## Palette

<span style="display:inline-block;width:1em;height:1em;background:#E63946;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#E63946</code>, <span style="display:inline-block;width:1em;height:1em;background:#1F7A3A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#1F7A3A</code>, <span style="display:inline-block;width:1em;height:1em;background:#FFD93D;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#FFD93D</code>

Three-color hint from `holidays.yml`. This is a guide for the codegen step and the emblem authoring pass; the actual emblem samples from the shared 16-color CLUT, and the original Sierra sprites use the original game palette directly.

## Algorithm

Fixed date — no computation; the `HOLIDAY_KIND_FIXED` dispatch in `holidayForDate` compares `month` and `day` directly. See [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}).

## Emblem

This holiday has an emblem on the shared sheet at cell 14, pixel offset (192, 32). View the packed sheet on GitHub: [`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png). Full index: [Emblem gallery]({{ '/docs/holidays/emblems/' | relative_url }}).

## Source

- Row in [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) (search for `id: 18` or `short_name: "CINCO MAYO"`).
- Generated row in [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c).
