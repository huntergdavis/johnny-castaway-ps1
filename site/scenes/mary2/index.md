---
layout: scene
title: MARY 2 — Mary visits while Johnny fishes; he mistakes her for a fish, catches a boot
ads: MARY
tag: 2
slug: mary2
status: validated
description: "MARY.ADS scene 2: Mary the mermaid swims up while Johnny is fishing; he mistakes her for a fish on the line, then reels in a boot. Validated 2026-05-03."
image: /assets/img/mary2-ps1-mermaid-fish.png
image_alt: "MARY 2 on PS1 at night: Johnny stands on his raft fishing with the rod arched out over the water while Mary the mermaid surfaces near the shoreline."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/mary2-ps1-mermaid-fish.webp' | relative_url }}" />
    <img src="{{ '/assets/img/mary2-ps1-mermaid-fish.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="MARY 2 on PS1 at night: Johnny stands on his raft fishing with the rod arched out over the water while Mary the mermaid surfaces near the shoreline." />
  </picture>
  <figcaption>
    MARY 2 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    Johnny is on the raft mid-cast; Mary the mermaid has surfaced
    at the shoreline. The "He catches a boot" line in the
    <a href="{{ '/docs/captions/' | relative_url }}">caption audit</a>
    actually fits this scene — see the
    <a href="{{ '/faq/' | relative_url }}">FAQ Q on caption-vs-scene-title divergence</a>
    for why FISHING 2 carries a caption that belongs to MARY 2.
  </figcaption>
</figure>

Validated on 2026-05-03 after visual and audible signoff, including
far-right and true far-left runtime stress playback.

## Pack identifiers

- ADS dispatch: <code>MARY.ADS scene 2</code>
- Slug: <code>mary2</code>

## What this scene is

Johnny is fishing when Mary the mermaid swims up. He mistakes her for a fish on the line — and after the confusion clears, he ends up reeling in a boot instead. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "first encounter" caption-mapping had the meet-cute right but missed the boot beat. (This is also where the "He catches a boot" line in the FISHING.ADS caption audit actually belongs — explaining the mismatch flagged on FISHING 2.)

### Validation note

This scene proved that one island-relative vignette can expose more than
one screen width of source pixels. A single host position clipped useful
action depending on where the island sat: the opening fishing line, Mary
and her splash, the boot throw, and the lower-water cleanup all needed
different sightlines.

The validated high and low packs are built from a wide scene-relative
multi-view stitch. Foreground-only captures at controlled host/test
positions restore the edge-clipped action pixels and remove stale
lower-band overpaint. The fish thought-bubble interval needed a separate
full-host bubble injection because foreground-only capture kept the fish
but dropped the white bubble shell.

Those host/test positions are capture evidence, not runtime pins.
Production playback remains variable-position safe; visual stress runs
passed at far-right and true far-left placement.
