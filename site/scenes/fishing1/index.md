---
layout: scene
title: FISHING 1 — Johnny casts a line
ads: FISHING
tag: 1
slug: fishing1
status: validated
description: "FISHING.ADS scene 1: Johnny casts a line. Validated under the FISHING 1 bar."
image: /assets/img/fishing1-ps1-cast.png
image_alt: "FISHING 1 on PS1: Johnny casts a fishing line off the island, sun overhead, palm tree in frame."
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/fishing1-ps1-cast.png' | relative_url }}" alt="FISHING 1 running on PS1 hardware: Johnny mid-cast." />
  <figcaption>FISHING 1 on PS1 hardware. The reference scene.</figcaption>
</figure>
## What happens

Johnny walks down to the water with a fishing rod, casts the line, and after a beat reels in a starfish. He looks at it, shrugs, and tosses it back. Then he packs up and walks back inland.

This is the reference scene for the whole pipeline. If FISHING 1 looks right and sounds right across all four variant flags, the host capture pipeline and the PS1 playback engine are working as designed. Every other scene gets validated against this bar.

## Validation

Validated as of `v0.3.6-ps1`. Variants exercised: night, low-tide, holiday, raft-stage.
This scene clears the [FISHING 1 bar]({{ '/about/method/' | relative_url }}) — pixel-perfect visuals plus synced SFX across every applicable variant.

## Pack identifiers

- ADS dispatch: <code>FISHING.ADS scene 1</code>
- Slug: <code>fishing1</code>
- High-tide pack: <code>FG/FISHING1.FG2</code>
- Low-tide pack: <code>FG/FISH1LOW.FG2</code>

## Variants

- **night** — Dusk/night palette swap (BOOTMODE `night 1`).
- **low-tide** — Tide state variant; different shoreline geometry (BOOTMODE `lowtide 1`).
- **holiday** — Holiday overlay variants — christmas, halloween, etc. (BOOTMODE `holiday N`).
- **raft-stage** — Cumulative raft-build state; raft sprite gains parts as the player progresses (BOOTMODE `raft-stage N`).

## Caption

This scene has on-screen caption text. Confidence: HIGH in the [caption audit]({{ '/docs/captions/' | relative_url }}).

<blockquote class="scene-caption">
Johnny goes fishing.<br />
He catches a starfish.<br />
He throws it back.
</blockquote>
