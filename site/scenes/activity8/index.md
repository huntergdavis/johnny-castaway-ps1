---
layout: scene
title: ACTIVITY 8 — Reads book upside down
ads: ACTIVITY
tag: 8
slug: activity8
status: validated
last_verified: "2026-05-05"
description: "ACTIVITY.ADS scene 8: Reads book upside down. Validated 2026-05-05."
---

Validated 2026-05-05. Visual + audible signoff after re-exporting
high/low packs through the no-stitch fast path with frame-wide keyed
overlay — fixes ghosted Johnny pose residue from base-diff against the
static-Johnny base.

## Pack identifiers

- ADS dispatch: <code>ACTIVITY.ADS scene 8</code>
- Slug: <code>activity8</code>

## What this scene probably is

(Guess.) Reading-under-tree variant where Johnny is holding the book upside down.

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
