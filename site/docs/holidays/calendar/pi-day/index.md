---
layout: page
title: "Pi Day"
eyebrow: "Holiday 12"
subtitle: "March 14 — fixed date"
description: "Pi Day: March 14 — fixed date. Tiny chalkboard with pi and pie slice."
---

## When it falls

March 14 — fixed date.

Worked dates:

| Year | Date |
|------|------|
| 2026 | March 14 |
| 2030 | March 14 |
| 2050 | March 14 |

## What it shows on the island

From [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml):

> Tiny chalkboard with pi and pie slice.

## Sprite

Dimensions: **32 × 32** pixels. Anchored at island position **(404, 284)**.

Added by this port via the [holiday expansion]({{ '/docs/holidays/' | relative_url }}). `existing_sprite: null`. The visual is a 32x32 transparent emblem authored by an AI sub-agent in the shared 16-color CLUT defined in `scripts/holidays_art_lib.py`. It overlays the island corner during the holiday's window rather than reskinning the scene.

## Palette

<span style="display:inline-block;width:1em;height:1em;background:#F5E8C8;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#F5E8C8</code>, <span style="display:inline-block;width:1em;height:1em;background:#7A4A2A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#7A4A2A</code>, <span style="display:inline-block;width:1em;height:1em;background:#A9C9E0;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#A9C9E0</code>

Three-color hint from `holidays.yml`. This is a guide for the codegen step and the emblem authoring pass; the actual emblem samples from the shared 16-color CLUT, and the original Sierra sprites use the original game palette directly.

## Algorithm

Fixed date — no computation; the `HOLIDAY_KIND_FIXED` dispatch in `holidayForDate` compares `month` and `day` directly. See [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}).

## Emblem

This holiday has an emblem on the shared sheet at cell 7, pixel offset (224, 0). View the packed sheet on GitHub: [`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png). Full index: [Emblem gallery]({{ '/docs/holidays/emblems/' | relative_url }}).

## Source

- Row in [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) (search for `id: 12` or `short_name: "PI DAY"`).
- Generated row in [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c).
