---
layout: page
title: "Election Day"
eyebrow: "Holiday 33"
subtitle: "1st Tuesday after the 1st Monday of November"
description: "Election Day: 1st Tuesday after the 1st Monday of November. Ballot box with checked ballot."
---

## When it falls

1st Tuesday after the 1st Monday of November — dispatched as `HOLIDAY_KIND_ELECTION_DAY`.

Worked dates:

| Year | Date |
|------|------|
| 2026 | November 3 |
| 2030 | November 5 |
| 2050 | November 8 |

## What it shows on the island

From [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml):

> Ballot box with checked ballot.

## Sprite

Dimensions: **32 × 32** pixels. Anchored at island position **(404, 284)**.

Added by this port via the [holiday expansion]({{ '/docs/holidays/' | relative_url }}). `existing_sprite: null`. The visual is a 32x32 transparent emblem authored by an AI sub-agent in the shared 16-color CLUT defined in `scripts/holidays_art_lib.py`. It overlays the island corner during the holiday's window rather than reskinning the scene.

## Palette

<span style="display:inline-block;width:1em;height:1em;background:#A82A3A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#A82A3A</code>, <span style="display:inline-block;width:1em;height:1em;background:#E0E0E0;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#E0E0E0</code>, <span style="display:inline-block;width:1em;height:1em;background:#2A3A6A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#2A3A6A</code>

Three-color hint from `holidays.yml`. This is a guide for the codegen step and the emblem authoring pass; the actual emblem samples from the shared 16-color CLUT, and the original Sierra sprites use the original game palette directly.

## Algorithm

Dispatched as `HOLIDAY_KIND_ELECTION_DAY`: 1st Monday of November via `holidayNthWeekdayOfMonth`, then add 1. See [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}#election-day).

## Emblem

This holiday has an emblem on the shared sheet at cell 29, pixel offset (160, 96). View the packed sheet on GitHub: [`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png). Full index: [Emblem gallery]({{ '/docs/holidays/emblems/' | relative_url }}).

## Source

- Row in [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) (search for `id: 33` or `short_name: "ELECTION"`).
- Generated row in [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c).
