---
layout: scene
title: JOHNNY 4 — His own SOS bottle washes back
ads: JOHNNY
tag: 4
slug: johnny4
status: validated
description: "JOHNNY.ADS scene 4: Johnny's own SOS bottle washes back onto the shore. Validated 2026-05-08."
---

Validated on 2026-05-03 after visual and audible signoff.

## Pack identifiers

- ADS dispatch: <code>JOHNNY.ADS scene 4</code>
- Slug: <code>johnny4</code>

## What this scene is

Johnny's SOS bottle — the one he tossed out earlier — washes back onto the shore. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "writes a fresh SOS" caption-mapping guess was wrong (that's the outbound beat; this scene is the gag where the bottle comes back to him).

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
