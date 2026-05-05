---
layout: scene
title: VISITOR 7 — Coconut tree impact
ads: VISITOR
tag: 7
slug: visitor7
status: validated
description: "VISITOR.ADS scene 7: Coconut tree impact. Validated on PS1 with regenerated high/low FG2 packs."
---

VISITOR 7 is validated under the current PS1 scene bar.

## Pack identifiers

- ADS dispatch: <code>VISITOR.ADS scene 7</code>
- Slug: <code>visitor7</code>

## Validation Notes

The current mapping is the coconut/tree impact gag. High and low tide packs
were regenerated through the generic normal/far-left/far-right foreground-only
multi-view stitch, so runtime island placement stays variable-position safe.

The first validation pass had the correct captured pixels, but the coconut
impact frames read as missing because dedupe left the strike rows too short.
The final pack redistributes hold time onto source frames `32`, `62`, `71`,
and `80`; total scene duration remains unchanged.

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

See [the method]({{ '/about/method/' | relative_url }}) for the longer version.
