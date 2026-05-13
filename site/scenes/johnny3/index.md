---
layout: scene
title: JOHNNY 3 — Writes and sends a letter to Suzy
ads: JOHNNY
tag: 3
slug: johnny3
status: validated
description: "JOHNNY.ADS scene 3: Johnny writes and sends a letter to Suzy. Validated 2026-05-02."
image: /assets/img/johnny3-ps1-letter-suzy.png
image_alt: "JOHNNY 3 on PS1 at night: Johnny stands on the left side of the island writing a letter, a thought bubble above him showing him and Suzy embracing on a beach."
image_width: 640
image_height: 448
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/johnny3-ps1-letter-suzy.webp' | relative_url }}" />
    <img src="{{ '/assets/img/johnny3-ps1-letter-suzy.png' | relative_url }}"
       width="640" height="448"
       fetchpriority="high"
       decoding="async"
       alt="JOHNNY 3 on PS1 at night: Johnny stands on the left side of the island writing a letter, a thought bubble above him showing him and Suzy embracing on a beach." />
  </picture>
  <figcaption>
    JOHNNY 3 on PS1, captured headlessly via
    <a href="{{ '/docs/scripted-input/' | relative_url }}">scripts/run-regtest.sh</a>
    after the v0.8.4-ps1 chapter-select grind missed this scene
    on the on-PS1 capture pass — see the
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select-grind retrospective</a>
    for why the original sweep was 62 of 63. The romantic
    thought-bubble: Johnny is writing the letter on the island,
    picturing himself with Suzy back home on the beach. JOHNNY 3
    is the outbound counterpart to
    <a href="{{ '/scenes/suzy1/' | relative_url }}">SUZY 1</a>,
    where Suzy receives the letter and daydreams in return —
    closing that 2-scene cross-island story-arc on the site.
  </figcaption>
</figure>

Validated on 2026-05-02 after visual and audible signoff. A right-shifted
island-position probe confirmed the full source pixels are present in the
pack; that was a diagnostic check only, not a runtime placement
requirement.

## Pack identifiers

- ADS dispatch: <code>JOHNNY.ADS scene 3</code>
- Slug: <code>johnny3</code>

## What this scene is

Johnny sits down, writes a letter to Suzy, and sends it off the island. Confirmed by direct on-PS1 playback observation; the earlier "his own SOS returns" caption-mapping guess was wrong — this scene is the outbound Suzy-letter beat, not a returning SOS bottle.

### Validation note

Like the later `FISHING 7`/`FISHING 8` revalidation, this scene treats
explicit island positions as capture/test evidence only. The standard
variable island placement remains the durable runtime policy unless a
scene proves it was host-clipped or requires a specific original
position.
