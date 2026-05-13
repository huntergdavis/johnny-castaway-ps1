---
layout: scene
title: JOHNNY 6 — At his office desk, daydreaming of the island
ads: JOHNNY
tag: 6
slug: johnny6
status: validated
description: "JOHNNY.ADS scene 6: Johnny is back at his office desk, daydreaming about his time on the island. Validated 2026-05-03."
image: /assets/img/johnny6-ps1-date-dream.png
image_alt: "JOHNNY 6 on PS1: Johnny dreams about his island date — pixel art rendering on PlayStation."
image_width: 1127
image_height: 677
---

## Validated

Validated on PS1/DuckStation on 2026-05-03 after routing the scene
through the full-screen black-backdrop runtime path. The source scene is
drawn over black, so the PS1 path now avoids painting ocean/island
background behind it.

<figure>
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/johnny6-ps1-office.webp' | relative_url }}" />
    <img src="{{ '/assets/img/johnny6-ps1-office.png' | relative_url }}" width="1127" height="677" fetchpriority="high" decoding="async" alt="JOHNNY 6 office daydream frame running on PS1." />
  </picture>
  <figcaption>JOHNNY 6 · Johnny working in the office, captured from DuckStation.</figcaption>
</figure>

<figure>
  <img src="{{ '/assets/img/johnny6-ps1-date-dream.png' | relative_url }}" width="1127" height="677" loading="lazy" decoding="async" alt="JOHNNY 6 island date dream frame running on PS1." />
  <figcaption>JOHNNY 6 · Johnny dreaming about his island date, captured from DuckStation.</figcaption>
</figure>

## Pack identifiers

- ADS dispatch: <code>JOHNNY.ADS scene 6</code>
- Slug: <code>johnny6</code>
- High-tide pack: <code>FG/JOHNNY6.FG2</code>
- Low-tide pack: <code>FG/JOHN6LOW.FG2</code>

## What this scene is

Johnny is back at his office desk, daydreaming about his time on the island — the inverse-castaway gag. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; matches the prior "office daydream" caption-mapping with the direction made explicit (he's at the office longing for the island, not on the island longing for the office).

## Validation Notes

This is not an island-relative scene. It belongs to the same black
backdrop class as `johnny1`: the foreground pack is replayed over a
black base surface, and temporal residual cleanup restores to black
instead of a saved ocean/island clean rect.
