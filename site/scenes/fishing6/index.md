---
layout: scene
title: FISHING 6 — Fish spits water in his face, thrown back
ads: FISHING
tag: 6
slug: fishing6
status: validated
description: "FISHING.ADS scene 6: Johnny catches a fish; it spits water in his face and he throws it back. Validated 2026-05-08."
image: /assets/img/fishing6-ps1-spits-water.png
image_alt: "FISHING 6 on PS1 at night: Johnny stands at the left shoreline holding a green fish that is squirting water directly into his face."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/fishing6-ps1-spits-water.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="FISHING 6 on PS1 at night: Johnny stands at the left shoreline holding a green fish that is squirting water directly into his face." />
  <figcaption>
    FISHING 6 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The squirt beat: Johnny has reeled in a big green fish and it
    is spitting water at his face; the next beat tosses it back
    into the ocean. The validated FGP3 packs narrow the terminal
    cleanup entry on both tides so the final splash and the top
    pixels of the fishing pole restore to the clean background
    before scene end — earlier residue is what blocked validation,
    not placement or tide selection.
  </figcaption>
</figure>

## Validated

Validated on PS1/DuckStation on 2026-05-01 after a terminal FGP3 cleanup fix removed the last splash and fishing-pole residue from the final frame.

This scene clears the [FISHING 1 bar]({{ '/about/method/' | relative_url }}) — pixel-perfect visuals plus synced SFX across every applicable variant.

## Pack identifiers

- ADS dispatch: <code>FISHING.ADS scene 6</code>
- Slug: <code>fishing6</code>
- High-tide pack: <code>FG/FISHING6.FG2</code>
- Low-tide pack: <code>FG/FISH6LOW.FG2</code>

## What this scene is

Johnny casts a line and reels in a fish. The fish squirts water in his face, and he tosses it back into the ocean. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the prior "big green fish" caption-mapping had the fish right but missed the squirt-and-toss beats that make the gag.

## Caption

On-screen caption text. Confidence: HIGH in the
[caption audit]({{ '/docs/captions/' | relative_url }}).

<blockquote class="scene-caption">
Johnny goes fishing.<br />
He catches a big green fish.<br />
It spits water in his face.
</blockquote>

## Validation note

The blocker was not placement or tide selection. The high/low FGP3 packs
kept a tiny final splash and the top pixels of the fishing pole alive in
the residual state. The fix narrows the final cleanup entry for both
packs so those pixels restore to the clean background before scene end.
