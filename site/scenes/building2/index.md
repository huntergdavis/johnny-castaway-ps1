---
layout: scene
title: BUILDING 2 — Roasts a boot
ads: BUILDING
tag: 2
slug: building2
status: validated
description: "BUILDING.ADS scene 2: Lilliputian sandcastle scene. Validated on PS1 after full-host sandcastle injection and cleanup."
---

Validated on 2026-05-05 under the FISHING 1 bar. The high/low packs were
regenerated through the generic normal/far-left/far-right foreground-only
multi-view stitch, with the persistent full-host sandcastle injected so the
red flag and planes originate from the correct base. The pack stays in FGP3
residual form so disappeared Lilliputian, plane, sand, and splash pixels clean
up explicitly.

## Pack identifiers

- ADS dispatch: <code>BUILDING.ADS scene 2</code>
- Slug: <code>building2</code>

## What this scene probably is

(Guess.) Johnny roasts and eats an old boot over a campfire.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **HIGH**.

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
