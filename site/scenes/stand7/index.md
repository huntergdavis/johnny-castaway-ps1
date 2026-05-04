---
layout: scene
title: STAND 7 — Lifts hat, looks around
ads: STAND
tag: 7
slug: stand7
status: validated
description: "STAND.ADS scene 7: Lifts hat, looks around. Validated on PS1."
---

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 7</code>
- Slug: <code>stand7</code>

## What this scene is

Idle at SPOT_C NE: lifts hat, looks around.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **LOW**.

## Validation notes

Visual signoff passed after regenerating high and low tide packs through the
STAND no-stitch fast path with a full-frame single-position foreground-only
overlay (the same export pattern that fixed `STAND 5` and `STAND 6`).

Boot route:
`fgpilot stand7 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
