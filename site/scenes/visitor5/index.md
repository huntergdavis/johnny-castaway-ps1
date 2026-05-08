---
layout: scene
title: VISITOR 5 — Visitor takes Johnny
ads: VISITOR
tag: 5
slug: visitor5
status: validated
description: "VISITOR.ADS scene 5: Visitor takes Johnny. Validated on PS1/DuckStation with readable coconut impact timing."
---

Validated on 2026-05-04 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

## Pack identifiers

- ADS dispatch: <code>VISITOR.ADS scene 5</code>
- Slug: <code>visitor5</code>

## What this scene probably is

(Guess; weak caption fit.) Final left-island variant — a visitor takes Johnny off the island. Caption mapping is uncertain.

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

VISITOR 5 uses the generic normal/far-left/far-right foreground-only
multi-view stitch so island-relative action is complete across production
placements. Its high/low packs also use scene-specific hold redistribution
so the coconut impact and downed-plane motion stay readable without
changing total scene duration.

See [the method]({{ '/about/method/' | relative_url }}) for the longer version.
