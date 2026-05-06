---
layout: scene
title: JOHNNY 6 — Office daydream
ads: JOHNNY
tag: 6
slug: johnny6
status: validated
description: "JOHNNY.ADS scene 6: Office daydream. Validated with black-backdrop playback."
image: /assets/img/johnny6-ps1-date-dream.png
image_alt: "JOHNNY 6 on PS1: Johnny dreams about his island date — pixel art rendering on PlayStation."
---

## Validated

Validated on PS1/DuckStation on 2026-05-03 after routing the scene
through the full-screen black-backdrop runtime path. The source scene is
drawn over black, so the PS1 path now avoids painting ocean/island
background behind it.

<figure>
  <img src="{{ '/assets/img/johnny6-ps1-office.png' | relative_url }}" width="1127" height="677" loading="lazy" decoding="async" alt="JOHNNY 6 office daydream frame running on PS1." />
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

## What this scene probably is

(Guess; day 10 final.) Johnny dreams of being back in an office: clock, typing, then dreams of the island and the mermaid; ends sad.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **HIGH**.

## Validation Notes

This is not an island-relative scene. It belongs to the same black
backdrop class as `johnny1`: the foreground pack is replayed over a
black base surface, and temporal residual cleanup restores to black
instead of a saved ocean/island clean rect.
