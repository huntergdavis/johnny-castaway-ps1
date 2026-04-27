---
layout: scene
title: MARY 4 — Heartbroken at the raft
ads: MARY
tag: 4
slug: mary4
status: pending
description: "MARY.ADS scene 4: Heartbroken at the raft. Not yet validated."
---

Not yet validated.

## Pack identifiers

- ADS dispatch: <code>MARY.ADS scene 4</code>
- Slug: <code>mary4</code>
- Source-table note: untested in regtest

## What this scene probably is

(Guess; day 7.) Johnny works on the raft; the mermaid is heartbroken nearby.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **MED**.

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
