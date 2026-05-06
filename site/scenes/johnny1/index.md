---
layout: scene
title: JOHNNY 1 — The End
ads: JOHNNY
tag: 1
slug: johnny1
status: validated
description: "JOHNNY.ADS scene 1: The End. Validated on PS1 with full-screen black-backdrop playback."
image: /assets/img/johnny1-ps1-the-end.png
image_alt: "JOHNNY 1 on PS1: the original Sierra 'The End' scroll graphic — Johnny waves from his island at sunset, palm tree silhouetted."
---

## Validated

Validated on PS1/DuckStation on 2026-05-02 after the scene moved to a
full-screen black-backdrop playback path. That matches the source scene
and removes the ocean/island clean-rect memory pressure that caused the
previous JOHNNY1 loop BSOD.

<figure>
  <img src="{{ '/assets/img/johnny1-ps1-the-end.png' | relative_url }}" width="1127" height="677" alt="JOHNNY 1 The End title card running on PS1." />
  <figcaption>JOHNNY 1 · The End, captured from DuckStation.</figcaption>
</figure>

<figure>
  <img src="{{ '/assets/img/johnny1-ps1-frog-clock.png' | relative_url }}" width="1127" height="677" alt="Frog clock transition frame running on PS1." />
  <figcaption>The frog clock transition frame from the same validation run.</figcaption>
</figure>

## Pack identifiers

- ADS dispatch: <code>JOHNNY.ADS scene 1</code>
- Slug: <code>johnny1</code>
- High-tide pack: <code>FG/JOHNNY1.FG2</code>
- Low-tide pack: <code>FG/JOHN1LOW.FG2</code>

## What this scene probably is

Day 11 final: frog clock, sunset silhouette, plane overhead, Johnny
parachutes down, then the `The End` title card.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **HIGH**.

## Validation Notes

This is not an island-relative scene. The source animation is drawn over
black, so the PS1 runtime now skips ocean/island setup for `johnny1` and
uses black cleanup for temporal residual spans instead of allocating a
large clean-rect snapshot. The same pass also made saved memcard mute
state load before `soundInit()`, so boot-time ambience does not start
before user settings are applied.
