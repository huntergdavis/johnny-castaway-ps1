---
layout: scene
title: MARY 1 — Date with Mary the mermaid
ads: MARY
tag: 1
slug: mary1
status: validated
description: "MARY.ADS scene 1: Johnny goes on a date with Mary the mermaid. Validated 2026-05-03."
image: /assets/img/mary1-ps1-dinner-date.png
image_alt: "MARY 1 on PS1 at night: Johnny and Mary the mermaid sit together at a small table under the palm tree on the island, mid-dinner-date, the moon overhead and the raft beside them."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/mary1-ps1-dinner-date.webp' | relative_url }}" />
    <img src="{{ '/assets/img/mary1-ps1-dinner-date.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="MARY 1 on PS1 at night: Johnny and Mary the mermaid sit together at a small table under the palm tree on the island, mid-dinner-date, the moon overhead and the raft beside them." />
  </picture>
  <figcaption>
    MARY 1 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The canonical Mary scene that the entire MARY.ADS file is
    named for: Johnny and Mary the mermaid sit together at a small
    table on the island, mid-date. (For the rest of the Mary saga,
    see <a href="{{ '/scenes/mary2/' | relative_url }}">MARY 2</a>
    boot-instead-of-fish,
    <a href="{{ '/scenes/mary3/' | relative_url }}">MARY 3</a>,
    <a href="{{ '/scenes/mary4/' | relative_url }}">MARY 4</a>
    raft-heartbreak, and
    <a href="{{ '/scenes/mary5/' | relative_url }}">MARY 5</a>.)
  </figcaption>
</figure>

## Validated

Validated on PS1/DuckStation on 2026-05-03 after visual and audible
signoff on the historical MARY1 validation route:

`fgpilot mary1 lowtide 0 night 1 holiday 0 raft-stage 5 island-pos -124 37 seed 1`

No pack or runtime change was required.

## Pack identifiers

- ADS dispatch: <code>MARY.ADS scene 1</code>
- Slug: <code>mary1</code>
- High-tide pack: <code>FG/MARY1.FG2</code>
- Low-tide pack: <code>FG/MARY1LOW.FG2</code>

## What this scene is

Johnny meets Mary the mermaid at the shoreline and the two go on a date — the canonical Mary scene that the ADS file is named for. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the prior "fancy dinner" caption-mapping had the date right but missed the mermaid character.

## Validation Notes

The validated route uses the historical scene placement
`x=-124,y=37` and raft-stage `5`. That value is recorded as validation
evidence; this promotion does not add a runtime placement pin or alter
the foreground pack.
