---
layout: scene
title: STAND 5 — Looks out over the ocean
ads: STAND
tag: 5
slug: stand5
status: validated
description: "STAND.ADS scene 5: Looks out over the ocean. Validated on PS1."
---

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 5</code>
- Slug: <code>stand5</code>

## What this scene probably is

(Guess.) Idle at SPOT_B S: looks out to sea, adjusts hat and pants.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **MED**.

## Validation notes

Visual signoff passed after regenerating high and low tide packs through the
STAND no-stitch fast path.

Boot route:
`fgpilot stand5 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.

The first no-stitch attempt faded Johnny's legs because pure base-diff treated
frame-0 static pixels as background. The exporter fast path now keeps a
single-position foreground-only overlay while still skipping far-left/far-right
stitch captures for simple STAND scenes.
