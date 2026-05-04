---
layout: scene
title: BUILDING 7 — Builds up the raft
ads: BUILDING
tag: 7
slug: building7
status: pending
description: "BUILDING.ADS scene 7: Builds up the raft. Not yet validated."
---

Not yet validated.

## Pack identifiers

- ADS dispatch: <code>BUILDING.ADS scene 7</code>
- Slug: <code>building7</code>

## What this scene probably is

(Guess.) Johnny gathers wood and adds it to the raft.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **MED**.

### How this scene gets validated

The same way every scene does: under the FISHING 1 bar.

A host-side Johnny Reborn capture/export pass produces a
base-diff `.FG2` foreground pack and a JSONL of sound events. The PS1
build replays that pack at native resolution through every variant the
original game randomized between (night, low-tide, holiday overlays,
raft-stage progress where applicable). The
[regtest harness]({{ '/docs/regtest/' | relative_url }}) checks that the
visuals come out frame-identical and the SFX cues land on the same
ticks. Once that holds across all applicable variants, the scene moves
to `validated` and a row turns green in the
[ledger]({{ '/scenes/' | relative_url }}).

Until then, this page exists so the catalogue is complete — not because
the scene is finished. See [the method]({{ '/about/method/' | relative_url }})
for the longer version.
