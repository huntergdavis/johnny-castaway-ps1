---
layout: scene
title: WALKSTUF 3 — Walk transition
ads: WALKSTUF
tag: 3
slug: walkstuf3
status: pending
description: "WALKSTUF.ADS scene 3: Walk transition. Not yet validated."
---

Not yet validated.

## Pack identifiers

- ADS dispatch: <code>WALKSTUF.ADS scene 3</code>
- Slug: <code>walkstuf3</code>

## What this scene probably is

(Guess; weak caption fit.) Pure walk-around transition; the audit notes no fitting caption.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **LOW**.

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
