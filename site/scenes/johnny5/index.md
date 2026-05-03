---
layout: scene
title: JOHNNY 5 — Imagines a 3pm date
ads: JOHNNY
tag: 5
slug: johnny5
status: validated
description: "JOHNNY.ADS scene 5: Imagines a 3pm date. Validated after splash capture and SOS-note timing repair."
---

Validated on 2026-05-03 after visual and audible signoff.

## Pack identifiers

- ADS dispatch: <code>JOHNNY.ADS scene 5</code>
- Slug: <code>johnny5</code>

## What this scene probably is

(Guess.) Johnny writes a message and imagines a clock reading 3pm — preparing for the date.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **HIGH**.

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
