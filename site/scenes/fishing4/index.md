---
layout: scene
title: FISHING 4 — Hooks a shark, gets pulled around like a speedboat
ads: FISHING
tag: 4
slug: fishing4
status: validated
description: "FISHING.ADS scene 4: Johnny hooks a shark and gets pulled around the ocean like a water-skier behind a speedboat. Validated 2026-05-08."
image: /assets/img/fishing4-ps1-shark.png
image_alt: "FISHING 4 on PS1 at night: a shark fin cuts the water on the right edge of the frame, a fishing line connecting it to Johnny standing under the palm tree on the island."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/fishing4-ps1-shark.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="FISHING 4 on PS1 at night: a shark fin cuts the water on the right edge of the frame, a fishing line connecting it to Johnny standing under the palm tree on the island." />
  <figcaption>
    FISHING 4 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The hooked-shark beat: the fin is on the right edge of the
    frame, the fishing line stretches across the open water back
    to Johnny under the palm — the next beat drags him out like a
    water-skier. FISHING 4 is also a <code>LEFT_ISLAND</code>
    scene; the fgpilot path now derives that draw-offset from
    <code>story_data.h</code> so the island baseline holds the
    original ADS thread compensation.
  </figcaption>
</figure>

Validated on PS1/DuckStation on 2026-05-01 after the fgpilot path was corrected to apply the original `LEFT_ISLAND` scene draw offset.

## Pack identifiers

- ADS dispatch: <code>FISHING.ADS scene 4</code>
- Slug: <code>fishing4</code>

## What this scene is

Johnny casts a line, hooks a shark, and the shark takes off — dragging Johnny around the ocean like a water-skier being towed by a speedboat. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; matches and sharpens the prior caption-mapping guess (the audit had HIGH confidence on the shark).

### Validation

This scene clears the [FISHING 1 bar]({{ '/about/method/' | relative_url }}) — pixel-perfect visuals plus synced SFX across every applicable variant.

The important wrinkle was placement: `FISHING 4` is a `LEFT_ISLAND`
scene, so the island baseline lives at the far-left fixed position while
the scene sprites use the original ADS thread compensation. The PS1
fgpilot path now derives that offset from `story_data.h`.
