---
layout: scene
title: FISHING 5 — Eaten by a shark, then spat back out
ads: FISHING
tag: 5
slug: fishing5
status: validated
description: "FISHING.ADS scene 5: Johnny goes fishing, gets eaten by a shark, then spat back out. Validated 2026-05-02."
image: /assets/img/fishing5-ps1-shark-bite.png
image_alt: "FISHING 5 on PS1 at night: a large grinning shark sits on the right side of the island mid-bite, with what is left of Johnny's gear in its jaws."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/fishing5-ps1-shark-bite.webp' | relative_url }}" />
    <img src="{{ '/assets/img/fishing5-ps1-shark-bite.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="FISHING 5 on PS1 at night: a large grinning shark sits on the right side of the island mid-bite, with what is left of Johnny's gear in its jaws." />
  </picture>
  <figcaption>
    FISHING 5 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The shark has eaten Johnny whole and is mid-pose on the right
    side of the island; the next beat spits him back out — the
    punchline the original audit's "eaten by a shark" line missed.
    The validated pack path replays the foreground ledger without
    the final-surface visibility mask, so the shark pixels survive
    instead of dropping to outline-only frames.
  </figcaption>
</figure>

Validated on 2026-05-02 after the shark interaction was rebuilt with a
full-frame keyed current-ledger overlay for both tide packs. The final
PS1/DuckStation signoff found no visible shark residue or missing shark
pixels, and SFX timing remained aligned.

## Pack identifiers

- ADS dispatch: <code>FISHING.ADS scene 5</code>
- Slug: <code>fishing5</code>

## What this scene is

Johnny casts a line, a shark eats him whole — and then promptly spits him back out. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; matches and sharpens the prior "eaten by a shark" caption-mapping guess (which missed the spit-out beat that gives the gag its punchline).

### Validation note

The defect was host-capture side. A full host capture contained stale
shark/Johnny overpaint, while a final-surface-masked foreground-only
capture could drop useful current shark pixels and leave outline-only
frames. The validated pack path replays the current foreground ledger
without the final-surface visibility mask and includes the current static
base BMP ledger draws for this scene's overlay capture.
