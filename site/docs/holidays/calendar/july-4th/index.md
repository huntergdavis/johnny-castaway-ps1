---
layout: page
title: "Independence Day"
eyebrow: "Holiday 24"
subtitle: "July 4 — fixed date"
description: "Independence Day: July 4 — fixed date. Firework burst and small flag."
---

## When it falls

July 4 — fixed date.

Worked dates:

| Year | Date |
|------|------|
| 2026 | July 4 |
| 2030 | July 4 |
| 2050 | July 4 |

## What it shows on the island

From [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml):

> Firework burst and small flag.

## Sprite

Dimensions: **32 × 32** pixels. Anchored at island position **(404, 284)**.

Added by this port via the [holiday expansion]({{ '/docs/holidays/' | relative_url }}). `existing_sprite: null`. The visual is a 32x32 transparent emblem authored by an AI sub-agent in the shared 16-color CLUT defined in `scripts/holidays_art_lib.py`. It overlays the island corner during the holiday's window rather than reskinning the scene.

## Palette

<span style="display:inline-block;width:1em;height:1em;background:#C8102E;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#C8102E</code>, <span style="display:inline-block;width:1em;height:1em;background:#FFFFFF;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#FFFFFF</code>, <span style="display:inline-block;width:1em;height:1em;background:#1A2A5E;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#1A2A5E</code>

Three-color hint from `holidays.yml`. This is a guide for the codegen step and the emblem authoring pass; the actual emblem samples from the shared 16-color CLUT, and the original Sierra sprites use the original game palette directly.

## Algorithm

Fixed date — no computation; the `HOLIDAY_KIND_FIXED` dispatch in `holidayForDate` compares `month` and `day` directly. See [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}).

## Emblem

This holiday has an emblem on the shared sheet at cell 20, pixel offset (128, 64). View the packed sheet on GitHub: [`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png). Full index: [Emblem gallery]({{ '/docs/holidays/emblems/' | relative_url }}).

## Source

- Row in [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) (search for `id: 24` or `short_name: "JULY 4TH"`).
- Generated row in [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c).
