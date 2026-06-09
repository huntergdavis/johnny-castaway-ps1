---
layout: page
title: "Talk Like a Pirate Day"
eyebrow: "Holiday 30"
subtitle: "September 19 — fixed date"
description: "Talk Like a Pirate Day: September 19 — fixed date. Pirate tricorn and skull face."
---

## When it falls

September 19 — fixed date.

Worked dates:

| Year | Date |
|------|------|
| 2026 | September 19 |
| 2030 | September 19 |
| 2050 | September 19 |

## What it shows on the island

From [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml):

> Pirate tricorn and skull face.

## Sprite

Dimensions: **32 × 32** pixels. Anchored at island position **(404, 284)**.

Added by this port via the [holiday expansion]({{ '/docs/holidays/' | relative_url }}). `existing_sprite: null`. The visual is a 32x32 transparent emblem authored by an AI sub-agent in the shared 16-color CLUT defined in `scripts/holidays_art_lib.py`. It overlays the island corner during the holiday's window rather than reskinning the scene.

## Palette

<span style="display:inline-block;width:1em;height:1em;background:#1A1A1A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#1A1A1A</code>, <span style="display:inline-block;width:1em;height:1em;background:#8A1A1A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#8A1A1A</code>, <span style="display:inline-block;width:1em;height:1em;background:#B89A4A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#B89A4A</code>

Three-color hint from `holidays.yml`. This is a guide for the codegen step and the emblem authoring pass; the actual emblem samples from the shared 16-color CLUT, and the original Sierra sprites use the original game palette directly.

## Algorithm

Fixed date — no computation; the `HOLIDAY_KIND_FIXED` dispatch in `holidayForDate` compares `month` and `day` directly. See [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}).

## Emblem

This holiday has an emblem on the shared sheet at cell 26, pixel offset (64, 96). View the packed sheet on GitHub: [`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png). Full index: [Emblem gallery]({{ '/docs/holidays/emblems/' | relative_url }}).

## Source

- Row in [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) (search for `id: 30` or `short_name: "PIRATE DAY"`).
- Generated row in [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c).
