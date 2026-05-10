---
layout: scene
title: JOHNNY 5 — Sends an SOS bottle
ads: JOHNNY
tag: 5
slug: johnny5
status: validated
description: "JOHNNY.ADS scene 5: Johnny writes a fresh SOS and sends the bottle out. Validated 2026-05-08."
image: /assets/img/johnny5-ps1-sos-send.png
image_alt: "JOHNNY 5 on PS1 at night: Johnny stands on the left shoreline holding the bottle he is about to throw out, the SOS message already corked inside."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/johnny5-ps1-sos-send.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="JOHNNY 5 on PS1 at night: Johnny stands on the left shoreline holding the bottle he is about to throw out, the SOS message already corked inside." />
  <figcaption>
    JOHNNY 5 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The fresh-SOS-send beat: Johnny is at the shoreline with a
    new bottle, message already corked inside, about to throw it
    out. JOHNNY 5 is the third scene in Johnny's three-scene SOS
    saga —
    <a href="{{ '/scenes/johnny2/' | relative_url }}">JOHNNY 2</a>
    (recycles a found bottle),
    <a href="{{ '/scenes/johnny5/' | relative_url }}">JOHNNY 5</a>
    (sends a fresh one),
    <a href="{{ '/scenes/johnny4/' | relative_url }}">JOHNNY 4</a>
    (one of his bottles eventually washes back). The validated
    pack uses a full-frame foreground-only keyed overlay so the
    bottle pixels survive without stale full-host overpaint, and
    holds the note-bubble frame instead of landing on the blank
    row after it.
  </figcaption>
</figure>

Validated on 2026-05-03 after visual and audible signoff.

## Pack identifiers

- ADS dispatch: <code>JOHNNY.ADS scene 5</code>
- Slug: <code>johnny5</code>

## What this scene is

Johnny writes a fresh SOS message, corks it into a bottle, and sends it out from the shore. Distinct from JOHNNY 2 (which is the gag where a bottle washes up first and he puts a note inside it). Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "imagines a 3pm date" caption-mapping guess was wrong.

### Validation note

This was another bottle-message capture bug, but the failing pixel was
the splash back in the water. Capturing the host frames at `x=-64`
kept the note visible, but clipped the thrown-bottle splash. The high
and low packs now capture/test at `x=80,y=54`, where the splash is in
frame, then replay scene-relative pixels through normal PS1 placement.

The lower band also needed the same stale-overpaint treatment as the
other bottle scenes: a full-frame keyed foreground-only overlay prevents
old feet/bottle pixels from accumulating. The SOS note was already in
the capture, but the hold timing was landing on the blank row after it;
the pack now shifts that hold time onto the note bubble itself.

The `x=80,y=54` value is a host/test capture position, not a production
runtime pin. Story and freeplay placement remain variable.
