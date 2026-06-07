---
layout: page
title: "Christmas"
eyebrow: "Holiday 3"
subtitle: "December 25 — fixed date"
description: "Christmas: December 25 — fixed date. Palm decorated as a Christmas tree with shells and starfish, Santa hat on Johnny, stocking on the message-bottle."
---

## When it falls

December 25 — fixed date.

Worked dates:

| Year | Date |
|------|------|
| 2026 | December 25 |
| 2030 | December 25 |
| 2050 | December 25 |

## What it shows on the island

From [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml):

> Palm decorated as a Christmas tree with shells and starfish, Santa hat on Johnny, stocking on the message-bottle.

## Sprite

Dimensions: **56 × 65** pixels. Anchored at island position **(404, 267)**.

This is one of the four **original Sierra sprites** preserved from the 1992 game. `existing_sprite` index `2`. The asset is the original full-island scene — Johnny appears in costume, the palm or message bottle is reskinned. The four originals are New Year's Day, St. Patrick's Day, Halloween, and Christmas; together they pin holiday IDs 1–4. See [the holidays landing]({{ '/docs/holidays/' | relative_url }}) for the full 36-entry list including the 32 added by this port.

## Palette

<span style="display:inline-block;width:1em;height:1em;background:#C8102E;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#C8102E</code>, <span style="display:inline-block;width:1em;height:1em;background:#1F5A2A;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#1F5A2A</code>, <span style="display:inline-block;width:1em;height:1em;background:#D4AF37;border:1px solid #444;vertical-align:middle"></span>&nbsp;<code>#D4AF37</code>

Three-color hint from `holidays.yml`. This is a guide for the codegen step and the emblem authoring pass; the actual emblem samples from the shared 16-color CLUT, and the original Sierra sprites use the original game palette directly.

## Algorithm

Fixed date — no computation; the `HOLIDAY_KIND_FIXED` dispatch in `holidayForDate` compares `month` and `day` directly. See [Date-algorithm core]({{ '/docs/holidays/algorithm/' | relative_url }}).

## Emblem

No 32x32 emblem entry; the visual for this holiday is the original Sierra full-island sprite, not a sheet emblem.

## Source

- Row in [`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml) (search for `id: 3` or `short_name: "CHRISTMAS"`).
- Generated row in [`src/scene/holidays_table.c`]({{ site.github_url }}/blob/main/src/scene/holidays_table.c).
- Original Sierra sprite (1992 game). Sprite frame index `2`.
