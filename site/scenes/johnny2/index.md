---
layout: scene
title: JOHNNY 2 — Bottle washes up; Johnny puts an SOS note inside
ads: JOHNNY
tag: 2
slug: johnny2
status: validated
description: "JOHNNY.ADS scene 2: a bottle washes up on the shore; Johnny puts an SOS note inside and tosses it back into the ocean. Validated 2026-05-08."
image: /assets/img/johnny2-ps1-sos-note.png
image_alt: "JOHNNY 2 on PS1 at night: Johnny stands on the left shoreline holding a bottle, a thought bubble above him showing a sheet of paper labelled SOS."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/johnny2-ps1-sos-note.webp' | relative_url }}" />
    <img src="{{ '/assets/img/johnny2-ps1-sos-note.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="JOHNNY 2 on PS1 at night: Johnny stands on the left shoreline holding a bottle, a thought bubble above him showing a sheet of paper labelled SOS." />
  </picture>
  <figcaption>
    JOHNNY 2 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The SOS-note thought-bubble frame: Johnny is at the left
    shoreline holding the bottle that washed up, picturing the
    note he'll roll inside before tossing it back into the
    ocean. The bubble hold timing was specifically tuned so the
    island and SOS thought frames stay readable instead of
    rolling past as a trailing empty-bubble chain — the per-page
    perf retrospective at
    <a href="{{ '/lab/from-87-to-99-5/' | relative_url }}">/lab/from-87-to-99-5/</a>
    walks through the JOHNNY 2 padded-FGP3 + clean-pressure-relief
    follow-up that completed this row.
  </figcaption>
</figure>

Validated on 2026-05-02 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

This scene is pinned to the host-captured island position
`x=-64,y=54`. The high/low FG2 packs use lower-band keyed overlay
cleanup for the moving bottle/feet region and explicit hold timing for
the island/SOS thought bubbles.

## Pack identifiers

- ADS dispatch: <code>JOHNNY.ADS scene 2</code>
- Slug: <code>johnny2</code>

## What this scene is

A bottle washes up on the shore. Johnny picks it up, puts an SOS note inside, and tosses it back into the ocean. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; matches and sharpens the prior "first SOS bottle" caption-mapping guess.

### How this scene was validated

The same way every scene does: under the FISHING 1 bar.

A host-side Johnny Reborn capture/export pass produces a
base-diff `.FG2` foreground pack and a JSONL of sound events. The PS1
build replays that pack at native resolution through every variant the
original game randomized between (night, low-tide, holiday overlays,
raft-stage progress where applicable). The
[regtest harness]({{ '/docs/regtest/' | relative_url }}) checks that the
visuals come out frame-identical and the SFX cues land on the same
ticks.

This one passed visual + audible review after the capture was rebuilt
at the pinned island position and the bubble hold timing was tuned so
the island/SOS frames hold instead of the trailing empty bubble chain.
See [the method]({{ '/about/method/' | relative_url }}) for the longer
version.
