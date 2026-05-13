---
layout: scene
title: JOHNNY 4 — His own SOS bottle washes back
ads: JOHNNY
tag: 4
slug: johnny4
status: validated
description: "JOHNNY.ADS scene 4: Johnny's own SOS bottle washes back onto the shore. Validated 2026-05-03."
image: /assets/img/johnny4-ps1-bottle-returns.png
image_alt: "JOHNNY 4 on PS1 at night: Johnny stands on the left shoreline holding the bottle that just washed back, a red question-mark hovering above his head."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/johnny4-ps1-bottle-returns.webp' | relative_url }}" />
    <img src="{{ '/assets/img/johnny4-ps1-bottle-returns.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="JOHNNY 4 on PS1 at night: Johnny stands on the left shoreline holding the bottle that just washed back, a red question-mark hovering above his head." />
  </picture>
  <figcaption>
    JOHNNY 4 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The "wait, this is mine?" beat: Johnny is at the shoreline with
    the SOS bottle he tossed out earlier, now washed back onto the
    same beach, with a red question-mark hovering as the
    realization-cue. (The outbound SOS-note beat is
    <a href="{{ '/scenes/johnny2/' | relative_url }}">JOHNNY 2</a>;
    this scene is the punchline that closes that loop.) The
    validated pack uses a full-frame foreground-only keyed overlay
    so the bottle pixels and SOS thought-bubble survive without
    stale full-host overpaint or thin ocean-colored bubble streaks.
  </figcaption>
</figure>

Validated on 2026-05-03 after visual and audible signoff.

## Pack identifiers

- ADS dispatch: <code>JOHNNY.ADS scene 4</code>
- Slug: <code>johnny4</code>

## What this scene is

Johnny's SOS bottle — the one he tossed out earlier — washes back onto the shore. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "writes a fresh SOS" caption-mapping guess was wrong (that's the outbound beat; this scene is the gag where the bottle comes back to him).

### Validation note

This was another bottle-message capture bug. Host full-surface frames
could carry stale bottle overpaint and a thin ocean-colored line through
the SOS thought bubble. The clean foreground-only stream contained the
correct bottle and bubble pixels, so `JOHNNY 4` now uses a full-frame
keyed foreground-only overlay for both high and low tide packs.

The scene was captured and tested at `island-pos -64 54` so the bottle
message stayed fully in frame. That is not a production runtime pin:
normal story/freeplay placement remains variable unless a future scene
proves it genuinely requires a fixed island position.
