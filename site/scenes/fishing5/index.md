---
layout: scene
title: FISHING 5 — Eaten by a shark, then spat back out
ads: FISHING
tag: 5
slug: fishing5
status: validated
description: "FISHING.ADS scene 5: Johnny goes fishing, gets eaten by a shark, then spat back out. Validated 2026-05-08."
---

Validated on 2026-05-02 after the shark interaction was rebuilt with a
full-frame keyed current-ledger overlay for both tide packs. The final
PS1/DuckStation signoff found no visible shark residue or missing shark
pixels, and SFX timing remained aligned.

## Pack identifiers

- ADS dispatch: <code>FISHING.ADS scene 5</code>
- Slug: <code>fishing5</code>

## What this scene is

Johnny casts a line, a shark eats him whole — and then promptly spits him back out. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; matches and sharpens the prior "eaten by a shark" caption-mapping guess (which missed the spit-out beat that gives the gag its punchline).

### Validation note

The defect was host-capture side. A full host capture contained stale
shark/Johnny overpaint, while a final-surface-masked foreground-only
capture could drop useful current shark pixels and leave outline-only
frames. The validated pack path replays the current foreground ledger
without the final-surface visibility mask and includes the current static
base BMP ledger draws for this scene's overlay capture.
