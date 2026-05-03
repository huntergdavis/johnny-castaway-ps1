---
layout: scene
title: JOHNNY 4 — Writes a fresh SOS
ads: JOHNNY
tag: 4
slug: johnny4
status: validated
description: "JOHNNY.ADS scene 4: Writes a fresh SOS. Validated after full-frame foreground-only capture cleanup."
---

Validated on 2026-05-03 after visual and audible signoff.

## Pack identifiers

- ADS dispatch: <code>JOHNNY.ADS scene 4</code>
- Slug: <code>johnny4</code>

## What this scene probably is

(Guess.) Johnny writes a new message in a bottle and throws it back out.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **HIGH**.

### Validation note

This was another bottle-message capture bug. Host full-surface frames
could carry stale bottle overpaint and a thin ocean-colored line through
the SOS thought bubble. The clean foreground-only stream contained the
correct bottle and bubble pixels, so `JOHNNY 4` now uses a full-frame
keyed foreground-only overlay for both high and low tide packs.

The scene was captured and tested at `island-pos -64 54` so the bottle
message stayed fully in frame. That is not a production runtime pin:
normal story/freeplay placement remains variable unless a future scene
proves it genuinely requires a fixed island position.
