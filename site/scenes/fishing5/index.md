---
layout: scene
title: FISHING 5 — Eaten by a shark
ads: FISHING
tag: 5
slug: fishing5
status: blocked
description: "FISHING.ADS scene 5: Eaten by a shark. Blocked by visible shark cleanup residue."
---

Blocked on 2026-05-01 verification: the shark leaves visible leftover
sprites/frames instead of fully wiping between animation frames.

## Pack identifiers

- ADS dispatch: <code>FISHING.ADS scene 5</code>
- Slug: <code>fishing5</code>

## What this scene probably is

(Guess.) Johnny hooks a shark; the shark eats him, then spits him back onto the beach.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **HIGH**.

### Current blocker

This is not an old FG2-pack issue: the current high/low packs are already
FGP3 temporal-residual packs and contain cleanup spans. The remaining
failure is likely in the residual cleanup/runtime contract or in the
generated cleanup coverage for this scene.

Do not sign this scene off until the shark residue is gone under the
same FISHING 1 bar used by the validated scenes.
