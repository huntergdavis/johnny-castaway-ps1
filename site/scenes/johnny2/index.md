---
layout: scene
title: JOHNNY 2 — First SOS bottle
ads: JOHNNY
tag: 2
slug: johnny2
status: validated
description: "JOHNNY.ADS scene 2: First SOS bottle. Validated on PS1 scene playback."
---

Validated on 2026-05-02 under the FISHING 1 bar.

This scene is pinned to the host-captured island position
`x=-64,y=54`. The high/low FG2 packs use lower-band keyed overlay
cleanup for the moving bottle/feet region and explicit hold timing for
the island/SOS thought bubbles.

## Pack identifiers

- ADS dispatch: <code>JOHNNY.ADS scene 2</code>
- Slug: <code>johnny2</code>

## What this scene probably is

(Guess; day 2.) Johnny finds a bottle, writes an SOS, corks it, throws it out to sea.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **HIGH**.

### How this scene was validated

The same way every scene does: under the FISHING 1 bar.

A host-side Johnny Reborn capture/export pass produces a
base-diff `.FG2` foreground pack and a JSONL of sound events. The PS1
build replays that pack at native resolution through every variant the
original game randomized between (night, low-tide, holiday overlays,
raft-stage progress where applicable). The
[regtest harness]({{ '/docs/regtest/' | relative_url }}) checks that the
visuals come out frame-identical and the SFX cues land on the same
ticks.

This one passed visual + audible review after the capture was rebuilt
at the pinned island position and the bubble hold timing was tuned so
the island/SOS frames hold instead of the trailing empty bubble chain.
See [the method]({{ '/about/method/' | relative_url }}) for the longer
version.
