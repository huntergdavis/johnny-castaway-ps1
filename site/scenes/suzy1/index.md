---
layout: scene
title: SUZY 1 — Suzy finds a letter from Johnny
ads: SUZY
tag: 1
slug: suzy1
status: validated
description: "SUZY.ADS scene 1: Suzy back home finds a letter that Johnny sent her from the island. Validated 2026-05-08."
image: /assets/img/suzy1-ps1-letter-daydream.png
image_alt: "SUZY 1 on PS1: Suzy stands on a beach in front of a city skyline holding a letter from Johnny, a thought bubble above her head showing the island Johnny is castaway on."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/suzy1-ps1-letter-daydream.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="SUZY 1 on PS1: Suzy stands on a beach in front of a city skyline holding a letter from Johnny, a thought bubble above her head showing the island Johnny is castaway on." />
  <figcaption>
    SUZY 1 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The letter-and-daydream beat: Suzy is on the beach back home —
    <code>SUZBEACH.SCR</code> backdrop, city skyline, palm-island
    cutaway in the corner — with a thought bubble of Johnny's
    island as she reads his letter. SUZY 1 is the inbound
    counterpart to
    <a href="{{ '/scenes/johnny3/' | relative_url }}">JOHNNY 3</a>'s
    outbound "writes a letter to Suzy" beat. Engineering note: not
    an island/ocean scene — the FG2 runtime now loads
    <code>SUZBEACH.SCR</code> as the clean backdrop for SUZY scenes
    instead of painting the standard island and water behind them.
    Both SUZY rows are metadata-only on
    <a href="{{ '/perf/' | relative_url }}">/perf/</a> on purpose
    (no deterministic scene-end so they're excluded from
    target-speed averages — visual signoff still holds).
  </figcaption>
</figure>

Validated on 2026-05-04 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

SUZY 1 is not an island/ocean scene. The PS1 runtime now loads the
source `SUZBEACH.SCR` screen as the clean backdrop for SUZY scenes,
then replays the foreground pack over that beach background.

## Pack identifiers

- ADS dispatch: <code>SUZY.ADS scene 1</code>
- Slug: <code>suzy1</code>

## What this scene is

Suzy, back home, finds a letter that Johnny sent her from the island — the inbound side of the JOHNNY 3 letter-to-Suzy gag. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "sunbathing daydream" caption-mapping guess was wrong.

### Validation notes

High and low packs were regenerated with the current foreground exporter.
The blocking defect was backdrop classification: the scene was correctly
captured, but runtime playback still painted the normal island and water
behind it. `SUZBEACH.SCR` is now included on disc and selected by the
FG2 runtime for SUZY scenes.

## Notable runtime history

`SUZY 1` high and low both appear on the
[performance battle card]({{ '/perf/' | relative_url }}) as measured rows.
They need a longer `12000`-frame matrix budget because the valid scene-end
lands after the default `7200`-frame timing window. The current rows are close
to target at `5763/5738` VBlanks for both tides.
