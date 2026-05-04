---
layout: scene
title: STAND 8 — Tap, lift, look
ads: STAND
tag: 8
slug: stand8
status: validated
description: "STAND.ADS scene 8: Tap, lift, look. Validated on PS1."
---

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 8</code>
- Slug: <code>stand8</code>

## What this scene is

Idle at SPOT_C E: composite of two short stances.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **LOW**.

## Validation notes

Visual signoff passed after regenerating high and low tide packs through the
STAND no-stitch fast path AND adding a per-frame wave tick to the FG2
runtime. The earlier STAND validations (5/6/7) had silently lost ocean
animation: the no-stitch foreground-only pack carries Johnny pixels but no
captured water frames, so the static `OCEAN00.SCR` bg never advanced. The
new tick mirrors `adsPilotTickBackgroundWaves`' timer pattern, advancing
one wave frame every `gFgBackdropThread.delay` vblanks and redrawing the
last wave on intermediate frames so `grRestoreBgFromRects` doesn't leave a
gap. Scenes whose pack carries its own water frames are unaffected — the
foreground compose still draws on top.

Boot route:
`fgpilot stand8 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
