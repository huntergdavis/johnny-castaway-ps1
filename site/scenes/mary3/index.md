---
layout: scene
title: MARY 3 — Mermaid sneak-up
ads: MARY
tag: 3
slug: mary3
status: validated
description: "MARY.ADS scene 3: Mermaid sneak-up. Validated on PS1 after far-right full-frame foreground recapture and low-memory clean-snapshot relief."
---

Validated on 2026-05-03.

## Pack identifiers

- ADS dispatch: <code>MARY.ADS scene 3</code>
- Slug: <code>mary3</code>

## What this scene is

Johnny is fishing; the mermaid swims up behind him. He thinks it's a fish.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **HIGH**.

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
[performance battle card]({{ '/perf/' | relative_url }}) as measured rows.
The current refresh confirms the scene is not a playback bug, but it is a
real perf outlier: high is `2402/2295` VBlanks and low is `2402/2296`, with
heavy visible CD blocking from the clean-pressure path. Visual signoff (the
FISHING 1 bar) still holds.
