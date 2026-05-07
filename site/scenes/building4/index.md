---
layout: scene
title: BUILDING 4 — Sand castle vs. lilliputians
ads: BUILDING
tag: 4
slug: building4
status: validated
description: "BUILDING.ADS scene 4: Sand castle vs. lilliputians. Validated on PS1 after generic multi-view capture and terminal cleanup."
---

Validated on 2026-05-05 under the FISHING 1 bar. The high/low packs were
regenerated through the generic normal/far-left/far-right foreground-only
multi-view stitch, then converted to FGP3 with an explicit terminal cleanup
frame so the final Johnny/bird foreground row restores cleanly.

## Pack identifiers

- ADS dispatch: <code>BUILDING.ADS scene 4</code>
- Slug: <code>building4</code>

## What this scene probably is

(Guess.) Johnny builds a castle; tiny figures attack it. He retreats up the palm.

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

## Notable runtime history

`BUILDING 4` is the scene the post-v0.7.0 random-run soak named when
the
[clean-rect]({{ '/docs/glossary/#clean-rect' | relative_url }})
allocation pipeline first regressed under
[walk-clean]({{ '/docs/walks/' | relative_url }}) memory
pressure. The `v0.8.0-ps1` fix retries the large scene clean
snapshot after releasing the stale walk clean buffer and recaptures
the walk baseline so the scene loads cleanly even after a long
random-position approach. `BUILDING 4` also appears on the v0.8.0
[clean-memory-relief drop-prefetch]({{ '/lab/from-87-to-99-5/' | relative_url }})
exception list — the per-scene opt-in that drops the prefetch
window during large clean snapshots. The follow-on `v0.8.1-ps1`
[stability fix]({{ '/lab/v081-mary4-freeze/' | relative_url }})
generalized the same pressure-estimator path for every random-
position scene.
