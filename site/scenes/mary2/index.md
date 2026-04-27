---
layout: scene
title: MARY 2 — First encounter
ads: MARY
tag: 2
slug: mary2
status: pending
description: "MARY.ADS scene 2: First encounter. Not yet validated."
---

Not yet validated.

## Pack identifiers

- ADS dispatch: <code>MARY.ADS scene 2</code>
- Slug: <code>mary2</code>
- Source-table note: untested in regtest

## What this scene probably is

(Guess; day 1.) Mermaid surfaces with a necklace, gives Johnny a life preserver, proposes a date.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **HIGH**.

### How this scene gets validated

The same way every scene does: under the FISHING 1 bar.

A pixel-perfect host capture (ScummVM via the export script) produces a
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
