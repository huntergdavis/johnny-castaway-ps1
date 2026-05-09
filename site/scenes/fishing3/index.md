---
layout: scene
title: FISHING 3 — Octopus steals the fish and walks off
ads: FISHING
tag: 3
slug: fishing3
status: validated
description: "FISHING.ADS scene 3: Johnny pulls up an octopus, which steals his fish and walks off. Validated under the FISHING 1 bar."
---

## What happens

Johnny fishes, and an octopus comes up on the line — but instead of being the catch, the octopus snatches the fish that was on the hook and walks off with it. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail.

(The on-screen caption block below — preserved from the Sierra original — says "He catches a crab. It snaps his nose," which is a different fishing gag in the game. The caption-to-scene mapping in the original audit appears to be approximate; the on-PS1 pack for FISHING.ADS scene 3 plays the octopus-steals-fish beat.)

Third scene to clear the FISHING 1 bar. The loop holds together, the tide-state variant draws the right water line, and the octopus sequence was visually and audibly signed off on PS1/DuckStation on 2026-05-01.

## Validation

This scene clears the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}) — pixel-perfect visuals plus synced SFX across every applicable variant.

## Pack identifiers

- ADS dispatch: <code>FISHING.ADS scene 3</code>
- Slug: <code>fishing3</code>
- High-tide pack: <code>FG/FISHING3.FG2</code>
- Low-tide pack: <code>FG/FISH3LOW.FG2</code>

## Variants applicable

- **night** — Dusk/night palette swap (BOOTMODE `night 1`).
- **low-tide** — Tide state variant; different shoreline geometry (BOOTMODE `lowtide 1`).
- **holiday** — Holiday overlay variants — christmas, halloween, etc. (BOOTMODE `holiday N`).
- **raft-stage** — Cumulative raft-build state; raft sprite gains parts as the player progresses (BOOTMODE `raft-stage N`).

## Caption

On-screen caption text. Confidence: HIGH in the [caption audit]({{ '/docs/captions/' | relative_url }}).

<blockquote class="scene-caption">
Johnny goes fishing.<br />
He catches a crab.<br />
It snaps his nose.
</blockquote>
