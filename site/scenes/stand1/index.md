---
layout: scene
title: STAND 1 — Standing at the edge of the island
ads: STAND
tag: 1
slug: stand1
status: validated
description: "STAND.ADS scene 1: Johnny stands at the edge of the island. Validated 2026-05-08."
image: /assets/img/stand1-ps1-edge-stand.png
image_alt: "STAND 1 on PS1 at night: Johnny stands at the leftmost edge of the island in an idle pose, a short subtle stand-loop scene."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/stand1-ps1-edge-stand.png' | relative_url }}"
       width="961" height="720"
       loading="lazy"
       decoding="async"
       alt="STAND 1 on PS1 at night: Johnny stands at the leftmost edge of the island in an idle pose, a short subtle stand-loop scene." />
  <figcaption>
    STAND 1 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The minimal beat: Johnny is at the leftmost edge of the
    island in an idle pose. The whole pack is 35 source frames
    and a 169-VBlank timeline (~2.8s at 60 Hz) with no captured
    SFX — the visible reset is easy to miss. STAND 1 is also
    one of the canonical
    <a href="{{ '/about/' | relative_url }}">"hard cluster"</a>
    scenes — foreground-only multi-view scenes (with
    <a href="{{ '/scenes/miscgag1/' | relative_url }}">MISCGAG 1</a>,
    <a href="{{ '/scenes/miscgag2/' | relative_url }}">MISCGAG 2</a>,
    and the wide LILLIPUTIAN arrival) that all needed the
    generic normal / far-left / far-right host stitch before
    their packs replayed cleanly.
  </figcaption>
</figure>

Validated 2026-05-03 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 1</code>
- Slug: <code>stand1</code>

## What this scene is

Johnny stands at the edge of the island, idle — one of the standing-pose loops in the STAND family. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; matches the prior "edge of the island" caption-mapping.

## Validation Notes

`STAND 1` is intentionally short and subtle: the regenerated pack has
35 source frames and a 169-vblank timeline, about 2.8 seconds at 60 Hz.
There are no captured SFX events. The visible behavior is a small idle
stance loop whose reset is easy to miss.

High/low FG2 packs were regenerated through the generic normal,
far-left, and far-right foreground-only multi-view stitch. Production
island placement remains variable-position safe.

See [the method]({{ '/about/method/' | relative_url }}) for the longer
version of the validation bar.
