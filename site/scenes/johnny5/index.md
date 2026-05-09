---
layout: scene
title: JOHNNY 5 — Sends an SOS bottle
ads: JOHNNY
tag: 5
slug: johnny5
status: validated
description: "JOHNNY.ADS scene 5: Johnny writes a fresh SOS and sends the bottle out. Validated 2026-05-08."
---

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
