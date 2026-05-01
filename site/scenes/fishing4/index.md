---
layout: scene
title: FISHING 4 — Caught by a shark
ads: FISHING
tag: 4
slug: fishing4
status: validated
description: "FISHING.ADS scene 4: Caught by a shark. Validated under the FISHING 1 bar."
---

Validated on PS1/DuckStation on 2026-05-01 after the fgpilot path was corrected to apply the original `LEFT_ISLAND` scene draw offset.

## Pack identifiers

- ADS dispatch: <code>FISHING.ADS scene 4</code>
- Slug: <code>fishing4</code>

## What this scene probably is

(Guess from caption audit.) Johnny hooks a shark; the shark drags him out of frame jet-ski style.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **HIGH**.

### Validation

This scene clears the [FISHING 1 bar]({{ '/about/method/' | relative_url }}) — pixel-perfect visuals plus synced SFX across every applicable variant.

The important wrinkle was placement: `FISHING 4` is a `LEFT_ISLAND`
scene, so the island baseline lives at the far-left fixed position while
the scene sprites use the original ADS thread compensation. The PS1
fgpilot path now derives that offset from `story_data.h`.
