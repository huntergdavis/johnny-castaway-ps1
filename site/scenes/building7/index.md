---
layout: scene
title: BUILDING 7 — Builds up the raft
ads: BUILDING
tag: 7
slug: building7
status: validated
description: "BUILDING.ADS scene 7: Builds up the raft. Validated on PS1."
---

Validated on 2026-05-05 after regenerating high/low packs through the generic
normal/far-left/far-right foreground-only multi-view stitch. The middle
campfire interval is reconstructed from clean animated foreground rows instead
of stale full-host pixels, then the packs are converted to FGP3 with an
explicit cleanup frame.

User visual signoff passed on the normal high-tide/night validation route. A
last-100-frame full-host review did not show a fish-skeleton draw in this
scene.

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

This scene is now in the validated set under the current FISHING 1 bar. See
[the method]({{ '/about/method/' | relative_url }}) for the longer version.
