---
layout: scene
title: VISITOR 1 — Misses a speedboat passing by
ads: VISITOR
tag: 1
slug: visitor1
status: validated
description: "VISITOR.ADS scene 1: Johnny looks around the island and completely misses a speedboat driving by behind him. Validated 2026-05-08."
image: /assets/img/visitor1-ps1-misses-speedboat.png
image_alt: "VISITOR 1 on PS1 at night: a red and white speedboat zips past on the upper-left of the frame while Johnny stands at the front of the island looking the other way."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/visitor1-ps1-misses-speedboat.png' | relative_url }}"
       width="961" height="720"
       loading="lazy"
       decoding="async"
       alt="VISITOR 1 on PS1 at night: a red and white speedboat zips past on the upper-left of the frame while Johnny stands at the front of the island looking the other way." />
  <figcaption>
    VISITOR 1 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The miss-the-boat beat in one frame: the speedboat is mid-pass
    on the upper left, Johnny is at the front of the island looking
    the wrong way and never turns. Engineering note — VISITOR 1
    was rebuilt through the generic normal / far-left / far-right
    foreground-only multi-view stitch so the wide scene-relative
    union (Lilliputian arrival on this same ADS family) survives,
    plus one captured SFX event for the speedboat.
  </figcaption>
</figure>

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>VISITOR.ADS scene 1</code>
- Slug: <code>visitor1</code>

## What this scene is

A speedboat zips past the island; Johnny is looking around in the wrong direction and never sees it. The boat continues on without stopping. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "lilliputians arrive" caption-mapping guess was wrong (the lilliputian gags are in the BUILDING family).

## Validation Notes

VISITOR 1 was regenerated through the standard normal/far-left/far-right
foreground-only multi-view stitch. The resulting high and low tide FG2 packs
carry a wide scene-relative foreground union for the Lilliputian arrival and
one captured SFX event.

The validation route used low tide, night, no holiday overlay, raft-stage 4,
and `island-pos -154 54` for review. Production playback remains variable
island-position safe; the controlled host/test positions were capture
coverage probes, not runtime pins.
