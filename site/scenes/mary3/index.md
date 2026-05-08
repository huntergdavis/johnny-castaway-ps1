---
layout: scene
title: MARY 3 — Mary and Johnny exchange gifts and plan a date
ads: MARY
tag: 3
slug: mary3
status: validated
description: "MARY.ADS scene 3: Mary and Johnny exchange gifts at the shoreline and make plans to go on a date. Validated 2026-05-08."
---

Validated on 2026-05-03.

## Pack identifiers

- ADS dispatch: <code>MARY.ADS scene 3</code>
- Slug: <code>mary3</code>

## What this scene is

Mary the mermaid surfaces at the shoreline; she and Johnny exchange small gifts and make plans to go on a date together. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "mermaid sneak-up" caption-mapping guess was wrong.

## Validation notes

MARY 3 needed the island shifted right during host capture and visual
stress testing (`x=80,y=54`) so the action living left of the island was
fully visible to the pack compiler. Production playback is still
scene-relative; the host capture position is not a runtime pin.

The rebuilt high/low packs use a full-frame keyed foreground-only
overlay. That avoids stale full-host overpaint while preserving the
foreground gag. The late dinner/thought beat also has explicit
hold redistribution so the readable frames remain on screen long enough
without changing the total scene duration.

This scene also forced two infrastructure fixes: the host capture ledger
now clears stale sprite-surface references before BMP/layer surfaces are
freed, and the PS1 runtime drops optional prefetch buffers plus uses a
small streaming scratch path when a large clean snapshot would otherwise
fragment memory.

## Notable runtime history

`MARY 3` high and low both appear on the
[performance battle card]({{ '/perf/' | relative_url }}) without
[`target_speed`]({{ '/docs/glossary/#target-vb' | relative_url }})
values — they're two of the 6 untimed rows. The reason is
mechanical, not structural: the active-loop timing pre-dates the
current pack and hasn't been re-measured against the v0.7.2+
prefetch-relief baseline yet. Visual signoff (the FISHING 1 bar)
holds; the row will land in the green band on the next perf-matrix
refresh that touches MARY 3 specifically.
