---
layout: scene
title: SUZY 2 — Johnny drifts in on his raft and meets Suzy
ads: SUZY
tag: 2
slug: suzy2
status: validated
description: "SUZY.ADS scene 2: Johnny drifts in on his raft and is reunited with Suzy. Validated 2026-05-08."
image: /assets/img/suzy2-ps1-rendezvous.png
image_alt: "SUZY 2 on PS1: Suzy and Johnny embrace on the beach back home, his raft beached on the sand beside them, the city skyline behind."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/suzy2-ps1-rendezvous.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="SUZY 2 on PS1: Suzy and Johnny embrace on the beach back home, his raft beached on the sand beside them, the city skyline behind." />
  <figcaption>
    SUZY 2 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The rendezvous payoff: Johnny has drifted in on his raft and
    is reunited with Suzy on the home beach
    (<code>SUZBEACH.SCR</code> backdrop again, raft beached
    beside them). This scene closes the long arc seeded by every
    <a href="{{ '/scenes/mary5/' | relative_url }}">MARY 5</a>
    raft-pack-and-sail-off + Johnny's SOS-bottle saga
    (<a href="{{ '/scenes/johnny2/' | relative_url }}">JOHNNY 2</a>,
    <a href="{{ '/scenes/johnny5/' | relative_url }}">JOHNNY 5</a>,
    <a href="{{ '/scenes/johnny4/' | relative_url }}">JOHNNY 4</a>
    bottle returns) +
    <a href="{{ '/scenes/suzy1/' | relative_url }}">SUZY 1</a>'s
    inbound letter-and-daydream beat. Engineering retro: the FG2
    pack uses a full-frame foreground-only overlay with static
    scene-local art included so <code>MRAFT.BMP</code> stays in the
    pack — without that, Johnny floated in without the raft body —
    and SFX playback leaves mixer headroom for overlapping samples
    so late audio beats don't clip.
  </figcaption>
</figure>

Validated on 2026-05-04 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

SUZY 2 is a scene-specific backdrop case: the runtime loads
`SUZBEACH.SCR` instead of painting the normal island/ocean background.
The foreground pack uses a full-frame foreground-only overlay with static
scene-local art included so `MRAFT.BMP` stays in the pack; without that,
Johnny floated in without the raft body. SFX playback also leaves mixer
headroom for overlapping samples so the scene does not clip during the
late audio beats.

## Pack identifiers

- ADS dispatch: <code>SUZY.ADS scene 2</code>
- Slug: <code>suzy2</code>

## What this scene is

Johnny drifts in on his raft and is reunited with Suzy at the shore — the rendezvous payoff for the raft-build and SOS-bottle arcs. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; sharpens the prior "raft drifts to her" caption-mapping with the meet-up made explicit.

### Validation Notes

High and low tide packs were regenerated together and signed off by
human visual + audible review. See [the method]({{ '/about/method/' | relative_url }})
for the longer version.

## Notable runtime history

`SUZY 2` high and low both appear on the
[performance battle card]({{ '/perf/' | relative_url }}) as measured rows.
The current high/low rows are close to target at `2655/2633` VBlanks with
`0` due misses. `SUZY 1` is also measured now, using the longer timing window
that scene requires.
