---
layout: scene
title: STAND 16 — Spyglass, center
ads: STAND
tag: 16
slug: stand16
status: validated
description: "STAND.ADS scene 16: Spyglass, center. Validated on PS1."
---

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 16</code>
- Slug: <code>stand16</code>

## What this scene is

Idle at SPOT_C S: spyglass + look-around. Near-duplicate of stand15.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **LOW**.

## Validation notes

Visual signoff passed on the normal high-tide/night route after
regenerating high and low tide packs through the STAND no-stitch fast
path. The pack uses the full-frame single-position foreground-only
overlay, and wave animation comes from the runtime FG2 wave tick.

Boot route:
`fgpilot stand16 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
