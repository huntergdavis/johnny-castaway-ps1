---
layout: scene
title: MISCGAG 2 — Goes to bathe; a shark scares him off
ads: MISCGAG
tag: 2
slug: miscgag2
status: validated
description: "MISCGAG.ADS scene 2: Johnny goes to take a bath in the surf and a shark turns up and scares him off. Validated 2026-05-08."
image: /assets/img/miscgag2-ps1-bath-towel.png
image_alt: "MISCGAG 2 on PS1 at night: Johnny stands under the palm tree with a small white towel spread on the sand next to him, set up for a bath that the shark will interrupt."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/miscgag2-ps1-bath-towel.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="MISCGAG 2 on PS1 at night: Johnny stands under the palm tree with a small white towel spread on the sand next to him, set up for a bath that the shark will interrupt." />
  <figcaption>
    MISCGAG 2 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The bath-prep setup beat: Johnny under the palm with the towel
    spread on the sand, the moment before the shark turns up in
    the surf and scares him off. (For Johnny eaten by a shark, see
    <a href="{{ '/scenes/fishing5/' | relative_url }}">FISHING 5</a>;
    for the shark dragging Johnny like a water-skier, see
    <a href="{{ '/scenes/fishing4/' | relative_url }}">FISHING 4</a>.)
    MISCGAG 2 is also part of the foreground-only multi-view
    hard-cluster (with
    <a href="{{ '/scenes/miscgag1/' | relative_url }}">MISCGAG 1</a>,
    <a href="{{ '/scenes/stand1/' | relative_url }}">STAND 1</a>,
    and the wide LILLIPUTIAN arrival) that all needed the generic
    normal / far-left / far-right host stitch before packs
    replayed cleanly.
  </figcaption>
</figure>

Validated on 2026-05-03 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

## Pack identifiers

- ADS dispatch: <code>MISCGAG.ADS scene 2</code>
- Slug: <code>miscgag2</code>
- High pack: <code>MISCGAG2.FG2</code>
- Low pack: <code>MISC2LOW.FG2</code> (regenerated for parity; the scene's story flags do not randomize low tide)

## What this scene is

Johnny heads down to the surf to take a bath. A shark turns up in the water and scares him off before he can settle in. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the prior "towel, toe, shark" caption-mapping had the towel and shark right but missed the bath-and-scare-off framing.

## Validation Notes

`MISCGAG 2` passed human visual + audible signoff after the current export
pipeline rebuilt both high and low packs with the generic normal/far-left/far-right
foreground-only multi-view stitch. The signed route was the normal
high-tide/night playback route.

Runtime island placement remains variable; the controlled host/test island
positions only prove the scene-relative pack carries all pixels.
