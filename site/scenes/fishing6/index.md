---
layout: scene
title: FISHING 6 — Big green fish
ads: FISHING
tag: 6
slug: fishing6
status: validated
description: "FISHING.ADS scene 6: Big green fish. Validated under the FISHING 1 bar."
---

## Validated

Validated on PS1/DuckStation on 2026-05-01 after a terminal FGP3 cleanup fix removed the last splash and fishing-pole residue from the final frame.

This scene clears the [FISHING 1 bar]({{ '/about/method/' | relative_url }}) — pixel-perfect visuals plus synced SFX across every applicable variant.

## Pack identifiers

- ADS dispatch: <code>FISHING.ADS scene 6</code>
- Slug: <code>fishing6</code>
- High-tide pack: <code>FG/FISHING6.FG2</code>
- Low-tide pack: <code>FG/FISH6LOW.FG2</code>

## What this scene probably is

(Guess.) Johnny catches a big green fish that spits water in his face. He throws it back.

## Validation note

The blocker was not placement or tide selection. The high/low FGP3 packs
kept a tiny final splash and the top pixels of the fishing pole alive in
the residual state. The fix narrows the final cleanup entry for both
packs so those pixels restore to the clean background before scene end.
