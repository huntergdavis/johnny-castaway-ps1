---
layout: scene
title: STAND 6 — Looks at the palm
ads: STAND
tag: 6
slug: stand6
status: validated
description: "STAND.ADS scene 6: Looks at the palm. Validated on PS1."
---

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 6</code>
- Slug: <code>stand6</code>

## What this scene is

Idle at SPOT_B SE: taps a foot and looks at the palm tree.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **MED**.

## Validation notes

Visual signoff passed after regenerating high and low tide packs through the
STAND no-stitch fast path with a full-frame single-position foreground-only
overlay (the same export pattern that fixed `STAND 5`).

Boot route:
`fgpilot stand6 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
