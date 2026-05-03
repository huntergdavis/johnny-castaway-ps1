---
layout: post
title: "Scene Set lineup expands to seven categories — May 3, 2026"
date: 2026-05-03 18:00:00 -0700
editor_note: "Scene Set ships with the full categorical lineup: Johnny Stories, Mary Visits, Visitors, Activities, and Misc & Suzy on top of the existing All Scenes and Fishing Only. Pools point at real scenes whose FG2 packs are already on disc, so they play immediately and look better in place as visual signoff lands."
source_path: docs/ps1/pause-menu-design.md
description: "Pause menu Scene Set selector grows from two pools to seven; not-yet-validated scenes play today and self-upgrade as scenes get visually signed off."
---

The Scene Set menu item shipped two days ago with two pools: *All Scenes*
(the catch-all proven rotation) and *Fishing Only*. The plan was always
that more sets would follow as scene families validated. After looking
at the actual catalog state, the right move turned out to be the
opposite — ship the whole lineup right now, and let the pools improve
in place as scenes get visually signed off.

## What's in the lineup

Seven sets, all live as of v0.6.9-ps1:

| Set | Pool | Validation |
|---|---|---|
| All Scenes | catch-all (proven rotation) | n/a |
| Fishing Only | `fishing1`..`fishing8` | all ✅ |
| Johnny Stories | `johnny1`..`johnny6` | `1`..`4` ✅, `5`/`6` playing pre-validation |
| Mary Visits | `mary1`..`mary5` | `mary2` ✅ as of v0.6.8, others playing pre-validation |
| Visitors | `visitor1`, `visitor3`..`visitor7` | playing pre-validation |
| Activities | `activity1`, `activity4`..`activity12` | playing pre-validation |
| Misc & Suzy | `suzy1`, `suzy2`, `miscgag1`, `miscgag2` | playing pre-validation |

Two scene names were left out on purpose. `visitor2`, `activity2`, and
`activity3` have no FG2 packs on disc — including them in a pool would
risk a pack-start failure on a random pick — so the pools follow the
shape of the actual asset table. Suzy and Miscgag were combined
because each family alone is only two scenes; combined they're a
believable rotation.

## Why ship pre-validation

Scene Set is a random-rotation pool selector, not a scene viewer. The
runtime cost of a not-yet-validated scene is the same as a validated
one: load the FG2 pack, play it, tick the calendar. The visuals may
not be pixel-perfect, the SFX may need rework, but the scene plays. As
soon as a scene moves from ⏳ to ✅ in
[scene-status](/johnny-castaway-ps1/docs/scene-status/), it looks
right in whichever pool already contains it.

The alternative was empty pools that fall back to *All Scenes* until
their family validates. The selector framework supports this — the
fallback already exists at `jc_reborn.c:281` — but the menu would have
five rows that all played the same content, which is exactly the
"why are we showing this option" UX problem we built the pool labels
to avoid. Real scenes from day one is the better trade.

## How the framework holds up

The original Scene Set design doc said adding a set should be a
one-line entry in `gSceneSetPools` and a one-line entry in
`kSceneSetNames`. That held. The expansion patch is 60 lines of
diff: five `kSet<Family>Scenes` arrays, five new pool entries, five
new label strings, and an updated table in the pause-menu reference.
No engine changes, no new pipeline, no walk-or-frog-clock surgery
needed.

The on-disc FG2 catalog also held. Every scene named in any pool
already had its pack shipped — there was nothing to bake, no asset
re-work, no `cd_layout.xml` edit. That's the dividend of the FG2 +
pack-start design choice, and it pays off here: the *playable*
catalog has been ahead of the *validated* catalog for a while, and
the Scene Set framework is just the first feature that makes that
gap user-visible in a useful way.
