---
layout: scene
title: FISHING 3 — An octopus interlude
ads: FISHING
tag: 3
slug: fishing3
status: in-bringup
description: "FISHING.ADS scene 3: An octopus interlude. In bring-up — loop-stable, not yet signed off."
---

## What happens

Johnny fishes; an octopus comes up out of the water and wraps a tentacle around him. There's a brief struggle, and then the octopus disappears back under and Johnny goes back to fishing as if nothing happened.

Currently in bring-up. The loop holds together — frames don't drop, the tide-state variant draws the right water line — but it has not yet been signed off as fishing-1-bar-equivalent. Likely the next scene to graduate.

## In bring-up

What's working: the FG2 pack loops cleanly and the tide-state variant draws the right shoreline. What's not: this scene has not yet been signed off as fishing-1-bar-equivalent. Specifically, the SFX-cue diff and the night/holiday/raft-stage cross-product are still pending review.

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
