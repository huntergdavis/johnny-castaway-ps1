---
layout: scene
title: VISITOR 1 — Lilliputians arrive
ads: VISITOR
tag: 1
slug: visitor1
status: validated
description: "VISITOR.ADS scene 1: Lilliputians arrive. Validated on PS1 after generic multi-view capture stitching."
---

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>VISITOR.ADS scene 1</code>
- Slug: <code>visitor1</code>

## What This Scene Is

A three-mast ship arrives and the Lilliputians tie Johnny down.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **MED**.

## Validation Notes

VISITOR 1 was regenerated through the standard normal/far-left/far-right
foreground-only multi-view stitch. The resulting high and low tide FG2 packs
carry a wide scene-relative foreground union for the Lilliputian arrival and
one captured SFX event.

The validation route used low tide, night, no holiday overlay, raft-stage 4,
and `island-pos -154 54` for review. Production playback remains variable
island-position safe; the controlled host/test positions were capture
coverage probes, not runtime pins.
