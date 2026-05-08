---
layout: scene
title: FISHING 2 — A life raft drifts past
ads: FISHING
tag: 2
slug: fishing2
status: validated
description: "FISHING.ADS scene 2: A life raft drifts past. Validated under the FISHING 1 bar."
---

## What happens

Johnny is fishing again. While he waits, a small life raft drifts past in the water behind him. He doesn't see it. The raft drifts on out of frame, and Johnny eventually reels in nothing and walks back.

Second scene to clear the FISHING 1 bar. Same fishing-pose loop as scene 1, but with a different background animation layered in. Confirmed pixel-stable and SFX-synced across night, low-tide, holiday, and raft-stage variants.

## Validation

Validated as of `2026-04-23`. Variants exercised: night, low-tide, holiday, raft-stage.
This scene clears the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}) — pixel-perfect visuals plus synced SFX across every applicable variant.

## Pack identifiers

- ADS dispatch: <code>FISHING.ADS scene 2</code>
- Slug: <code>fishing2</code>
- High-tide pack: <code>FG/FISHING2.FG2</code>
- Low-tide pack: <code>FG/FISH2LOW.FG2</code>

## Variants

- **night** — Dusk/night palette swap (BOOTMODE `night 1`).
- **low-tide** — Tide state variant; different shoreline geometry (BOOTMODE `lowtide 1`).
- **holiday** — Holiday overlay variants — christmas, halloween, etc. (BOOTMODE `holiday N`).
- **raft-stage** — Cumulative raft-build state; raft sprite gains parts as the player progresses (BOOTMODE `raft-stage N`).

## Caption

This scene has on-screen caption text. Confidence: HIGH in the [caption audit]({{ '/docs/captions/' | relative_url }}).

<blockquote class="scene-caption">
Johnny goes fishing.<br />
He catches a boot.<br />
He keeps the boot.
</blockquote>
