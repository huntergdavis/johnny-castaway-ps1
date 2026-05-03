---
layout: post
title: "Scene Set framework and an animated frog clock — May 3, 2026"
date: 2026-05-03
editor_note: "Pause menu gains a Scene Set row that constrains the screensaver pool by category (All Scenes / Fishing Only / …); Left/Right preview, Cross or Start commits. The frog-clock loading frame goes from a static image to a 36-vblank animation with hour and minute hands placed from the original MEANWHIL.TTM script."
source_path: docs/ps1/pause-menu-design.md
description: "PS1 port pause menu adds a Scene Set selector and an animated frog-clock loading transition; sequence-reset on cycle keeps walks intact."
---

This post is about two small features that ended up tangled together
because the PS1's heap rules don't let you treat them as separate
problems.

## Scene Set: pool-by-category

The screensaver loop has always picked from a pool called
`kProvenScenes`. As more scenes get validated the pool grows, but the
shape stays the same: one big bag, the loop draws a random scene out
of it, the FG2 pack plays, the loop ticks the calendar, and around it
goes. There was no way to say "I'd like the next half hour to be
fishing scenes only" without forcing a pinned scene from the dev
console — which then plays the same scene every iteration.

Scene Set lifts that into a first-class menu item. The new item sits
right under Resume, defaults to **All Scenes**, and the only other
ship-quality alternative right now is **Fishing Only** — the eight
validated `fishing1..fishing8` packs. As more scene families lock in,
they get a one-line entry in `gSceneSetPools` and a label in
`kSceneSetNames`, no other code changes needed. The picker reads
`pauseMenuSceneSet` each iteration and indexes into the pool array.

The interaction model is deliberately different from the rest of the
main menu. Left and Right scroll a *pending* preview, which the row
shows as `<Fishing Only>` with an asterisk if the preview hasn't been
committed yet. Cross or Start commits. Up or Down (which moves the
cursor off the row) discards the preview. That separation matters
because the alternative — "scrolling lands you in a new pool
immediately" — felt like an accidental click hazard the first time
the row got tested. The brackets are the visual cue that this row
takes horizontal input; everything else still uses Cross-to-select.

## Frog clock as a real animation

The PS1 port has had a "meanwhile" frog-clock loading frame since the
freeplay branch landed. Up to today it was a single static image: load
`MEANWHIL.BMP`, draw the card sprite at `(254, 100)`, draw one fixed
hand frame at `(287, 141)`, and call it done. It looked like a paused
clock, which is exactly the wrong feel for a loading transition.

Replacing the static draw with a hand-cycle was a one-line change in
isolation. Replacing it with a hand-cycle that *looked right* needed
the original script. `MEANWHIL.TTM` carries 16 hand sprites split
between an hour-hand set (sprites 5–16) and a minute-hand set
(sprites 1–4), and the script draws each frame at a sprite-specific
`(x, y)` because the bounding box of a rotated hand changes per
angle. Drawing every hand frame at the same top-left position made
the hand visibly drift around the clock face on each frame.

A short Python decoder over the TTM bytecode pulled out the per-sprite
draw positions; they're now a static `kMeanwhilePos[17][2]` table
inside `grShowMeanwhileLoadingFrame`. The animation cycles the hour
hand every four frames and the minute hand every frame, for a loop of
36 vblanks (~0.6 s at 60 Hz). On each frame: stamp the card, then the
current hour hand at its baked anchor, then the current minute hand
at its baked anchor.

## Walk vs frog clock: why the cycle does a sequence reset

The honest part of this post is what happened when those two features
met for the first time.

Cycling Scene Set fires the frog clock as its loading transition. The
frog clock calls `grInitEmptyBackground` to give the card a clean
black backdrop — that zeros the four background tiles in RAM. That
was always the case; the change is that the *next* thing now is a
walk transition (the screensaver loop tries to walk Johnny from his
last spot to the new scene's start). And the walk subsystem composes
Johnny over those background tiles. Black tiles → black borders
around Johnny.

The first attempt was to snapshot the tiles before the wipe and
restore them after. On desktop that's a nothing change. On PS1 it's
600 KB of memcpy across four `bgTile*` buffers, and the heap is
already tight enough that the snapshot allocation introduced a
noticeable hitch on the first cycle of the run. Restoring also didn't
fully solve the problem because the rect-mode clean buffers walk
actually reads from are not the same set as the full-tile clean
buffers `grSaveCleanBgTiles` populates.

The fix that actually worked is structural. Cycling Scene Set is a
sequence reset by definition: the user just told the loop to throw
out its remembered context. So
`pauseMenuRequestSceneSetCycle` consumers now also clear
`storyCurrentSpot` and `storyCurrentHdg` and set
`fgLoopSequenceJustReset = 1`. With the reset flag set, the existing
guard at the top of the loop skips the walk for that iteration. The
next scene's `foregroundPilotPlay` does its own `grLoadScreen` and
pulls the bg back to a known good state before any compositing
happens. No 600 KB snapshot, no walk corruption, no extra moving
pieces.

Walks resume on the very next iteration because the next scene
updates `storyCurrentSpot/Hdg` on completion, so the user-visible
behavior is just: change the pool, see the frog clock, see "Scene
Set: Fishing Only" caption, see a clean cut to the next fishing
scene.

## What this is not

This release does not add new scene sets beyond Fishing Only.
Johnny-only, Mary-only, and Visitors-only sets are obvious next
steps, but they need their own scenes validated under the Scene Set
flow before they ship. The `gSceneSetPools` table is structured so
that adding them is a label and an array literal, not a rewrite.
This release also does not add a separate art asset for the
animation; the frog clock uses Sierra's existing 17-frame
`MEANWHIL.BMP` exactly as shipped.

The PS1 port keeps trying to make every player-visible feature land
on top of the same handful of trusted runtime pieces. A new menu
item and a smarter loading frame are exactly the kind of additions
that should not need new pipelines, and after today they don't.
