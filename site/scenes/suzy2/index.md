---
layout: scene
title: SUZY 2 — Raft drifts to her
ads: SUZY
tag: 2
slug: suzy2
status: validated
description: "SUZY.ADS scene 2: Raft drifts to her. Validated on PS1 with the SUZBEACH.SCR backdrop, static-base raft foreground capture, and synced SFX."
---

Validated on 2026-05-04 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

SUZY 2 is a scene-specific backdrop case: the runtime loads
`SUZBEACH.SCR` instead of painting the normal island/ocean background.
The foreground pack uses a full-frame foreground-only overlay with static
scene-local art included so `MRAFT.BMP` stays in the pack; without that,
Johnny floated in without the raft body. SFX playback also leaves mixer
headroom for overlapping samples so the scene does not clip during the
late audio beats.

## Pack identifiers

- ADS dispatch: <code>SUZY.ADS scene 2</code>
- Slug: <code>suzy2</code>

## What This Scene Is

Johnny's raft drifts up to Suzy; she kisses him, then scolds him.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **HIGH**.

### Validation Notes

High and low tide packs were regenerated together and signed off by
human visual + audible review. See [the method]({{ '/about/method/' | relative_url }})
for the longer version.

## Notable runtime history

`SUZY 2` high and low both appear on the
[performance battle card]({{ '/perf/' | relative_url }}) as measured rows.
The current high/low rows are close to target at `2655/2633` VBlanks with
`0` due misses. `SUZY 1` is also measured now, using the longer timing window
that scene requires.
