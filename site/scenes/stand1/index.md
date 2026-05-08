---
layout: scene
title: STAND 1 — Standing at the edge of the island
ads: STAND
tag: 1
slug: stand1
status: validated
description: "STAND.ADS scene 1: Johnny stands at the edge of the island. Validated 2026-05-08."
---

Validated 2026-05-03 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 1</code>
- Slug: <code>stand1</code>

## What this scene is

Johnny stands at the edge of the island, idle — one of the standing-pose loops in the STAND family. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; matches the prior "edge of the island" caption-mapping.

## Validation Notes

`STAND 1` is intentionally short and subtle: the regenerated pack has
35 source frames and a 169-vblank timeline, about 2.8 seconds at 60 Hz.
There are no captured SFX events. The visible behavior is a small idle
stance loop whose reset is easy to miss.

High/low FG2 packs were regenerated through the generic normal,
far-left, and far-right foreground-only multi-view stitch. Production
island placement remains variable-position safe.

See [the method]({{ '/about/method/' | relative_url }}) for the longer
version of the validation bar.
