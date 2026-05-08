---
layout: scene
title: VISITOR 7 — Cracks a coconut on the tree, eats it (no-shake variant)
ads: VISITOR
tag: 7
slug: visitor7
status: validated
description: "VISITOR.ADS scene 7: Johnny cracks a coconut on the palm trunk and eats it — no-shake variant of VISITOR 6. Validated 2026-05-08."
---

Validated on 2026-05-04 under the current PS1 scene bar.

## Pack identifiers

- ADS dispatch: <code>VISITOR.ADS scene 7</code>
- Slug: <code>visitor7</code>

## What this scene is

Johnny already has a coconut. He cracks it open against the palm trunk and eats the coconut meat — the no-shake variant of the VISITOR 6 sequence. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; both scenes are coconut-eat beats but the shake-tree prelude is only on VISITOR 6.

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
