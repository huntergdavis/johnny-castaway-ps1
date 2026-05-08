---
layout: scene
title: JOHNNY 2 — Bottle washes up; Johnny puts an SOS note inside
ads: JOHNNY
tag: 2
slug: johnny2
status: validated
description: "JOHNNY.ADS scene 2: a bottle washes up on the shore; Johnny puts an SOS note inside and tosses it back into the ocean. Validated 2026-05-08."
---

Validated on 2026-05-02 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

This scene is pinned to the host-captured island position
`x=-64,y=54`. The high/low FG2 packs use lower-band keyed overlay
cleanup for the moving bottle/feet region and explicit hold timing for
the island/SOS thought bubbles.

## Pack identifiers

- ADS dispatch: <code>JOHNNY.ADS scene 2</code>
- Slug: <code>johnny2</code>

## What this scene is

A bottle washes up on the shore. Johnny picks it up, puts an SOS note inside, and tosses it back into the ocean. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; matches and sharpens the prior "first SOS bottle" caption-mapping guess.

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
