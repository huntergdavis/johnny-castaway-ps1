---
layout: scene
title: MARY 4 — Heartbroken at the raft
ads: MARY
tag: 4
slug: mary4
status: validated
description: "MARY.ADS scene 4: Heartbroken at the raft. Validated with generic multi-view scene-relative stitching."
---

Validated 2026-05-03 under the FISHING 1 bar: pixel-perfect human
visual signoff plus synced captured SFX.

## Pack identifiers

- ADS dispatch: <code>MARY.ADS scene 4</code>
- Slug: <code>mary4</code>
- High-tide pack: <code>MARY4.FG2</code>
- Low-tide pack: <code>MARY4LOW.FG2</code>
- Source-table note: generic multi-view scene-relative stitch; production island placement remains variable

## What this scene probably is

(Guess; day 7.) Johnny works on the raft; the mermaid is heartbroken nearby.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **MED**.

## Validation notes

The single-position host capture clipped different island-relative
pixels at different runtime placements. The validated pack uses the
generic multi-view stitch: normal, far-left, and far-right
foreground-only host views are merged into one scene-relative
foreground canvas, with a magenta synthetic base so first-frame
foreground pixels are retained.

Far-right PS1 stress playback at <code>island-pos 300 54</code> passed.
That is evidence for pack completeness, not a production pin; normal
story playback can keep randomized island placement.
