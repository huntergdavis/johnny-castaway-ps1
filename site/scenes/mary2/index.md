---
layout: scene
title: MARY 2 — First encounter
ads: MARY
tag: 2
slug: mary2
status: validated
description: "MARY.ADS scene 2: First encounter. Validated after wide multi-view stitch and fish thought-bubble repair."
---

Validated on 2026-05-03 after visual and audible signoff, including
far-right and true far-left runtime stress playback.

## Pack identifiers

- ADS dispatch: <code>MARY.ADS scene 2</code>
- Slug: <code>mary2</code>

## What this scene probably is

(Guess; day 1.) Mermaid surfaces with a necklace, gives Johnny a life preserver, proposes a date.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **HIGH**.

### Validation note

This scene proved that one island-relative vignette can expose more than
one screen width of source pixels. A single host position clipped useful
action depending on where the island sat: the opening fishing line, Mary
and her splash, the boot throw, and the lower-water cleanup all needed
different sightlines.

The validated high and low packs are built from a wide scene-relative
multi-view stitch. Foreground-only captures at controlled host/test
positions restore the edge-clipped action pixels and remove stale
lower-band overpaint. The fish thought-bubble interval needed a separate
full-host bubble injection because foreground-only capture kept the fish
but dropped the white bubble shell.

Those host/test positions are capture evidence, not runtime pins.
Production playback remains variable-position safe; visual stress runs
passed at far-right and true far-left placement.
