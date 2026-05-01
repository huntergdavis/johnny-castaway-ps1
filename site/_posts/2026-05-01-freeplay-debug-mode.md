---
layout: post
title: "Freeplay debug mode — May 1, 2026"
date: 2026-05-01
editor_note: "Direct-control Johnny moved from design sketch to a playtestable PS1 mode: analog walking, fishing, menu-driven gag/visitor catalogs, immediate world toggles, frog loading transitions, and a stricter no-per-frame-allocation rule."
source_path: docs/ps1/freeplay-mode-design.md
description: "Implementation notes for freeplay/debug mode in the Johnny Castaway PS1 port."
---

Freeplay changed the project from a passive screensaver port into something
you can touch.

That sounds small until you look at what the PS1 runtime had become by this
point. The ordinary scenes are carefully captured foreground streams. They
are deterministic, measurable, and repeatable. Freeplay is none of those
things. It reads the controller every frame, moves Johnny around the island,
lets the player fish, opens debug catalogs, changes the tide and holidays,
and then has to cleanly hand the whole machine back to the normal random
screensaver loop.

The breakthrough was not making the controller move a sprite. The
breakthrough was refusing to make a second engine. Freeplay reuses the same
pieces that made walking safe in `v0.4.20`: a known island backdrop, sparse
clean rects, the shared walk renderer, holiday stamping, captions, SPU sound,
and the pause menu. It is live C code, but it stands on the replay runtime
instead of fighting it.

## The control scheme got smaller

The early builds tried to use the whole controller. Gags on face buttons.
Summons on face buttons. L1 shortcuts. R1 shortcuts. Context-sensitive Cross.
Combos. It was technically cute and practically awful.

The current version is deliberately simpler:

- D-pad or analog walks Johnny.
- L2 slows him down.
- R2 speeds him up.
- Circle fishes.
- Select clears and rebuilds the freeplay screen.
- R1 plus D-pad changes the four world states: day/night, tide, raft, and
  holiday.
- Start opens the pause menu.

Everything else moved into menu catalogs. That was the right split. The
controller is for the things a player does while looking at Johnny. Catalogs
are for actions that need names, descriptions, source asset filenames, frame
counts, and memory notes.

## The pause menu became a debug cockpit

Freeplay Options now owns Gags, Visitors, Controls, and Clear. Gags and
Visitors are selector pages: title, BMP, frame count, rough memory cost,
description, and "X spawn now."

That sounds like UI polish, but it is also engineering leverage. A failed
visitor load can skip cleanly instead of crashing. A bad frame range can be
identified by name instead of by button folklore. A sound can be tested from
the sound-test menu without needing to stumble into the one original scene
that plays it.

Circle is Back everywhere. That rule matters more than it sounds. A debug
mode that needs a handwritten controller legend every time you open it is
not finished.

## The frog clock became the transition contract

Freeplay has several expensive boundaries:

- enter from an arbitrary screensaver scene;
- exit back to the normal loop;
- clear the freeplay screen;
- apply world options from the pause menu.

Those boundaries now show the original meanwhile frog clock. It is the
right visual language for Johnny, and it also tells the tester that the
runtime is doing real teardown/rebuild work rather than freezing.

The memory rule is strict: the frog frame uses a temporary `TTtmSlot`, draws
one frame, and releases `MEANWHIL.BMP` unconditionally. It is a loading
frame, not a permanent resident.

## Memory was the real feature

The freeplay bug reports were exactly the kind of bug a forever-running PS1
program deserves: black pixels left by an overlay, a delayed second entry
that looked like a freeze, and a crash after leaving freeplay once.

The fixes tightened the shape of the mode:

- overlays mark their screen-space rows dirty before present, so captions
  and banners do not leave stale pixels behind;
- freeplay hard-frees its clean-rect snapshot on exit instead of leaving a
  dormant 300+ KB allocation parked in the heap;
- default entry/exit no longer runs largest-heap probes, because probes are
  malloc/free cycles and therefore part of the thing being tested;
- heap probes remain behind `freeplay-log`, `freeplay-detail`, and
  `freeplay-debug`;
- movement, ticking, and drawing do no allocation in the steady-state frame
  loop.

That is the bar for this project now. If a feature only works for five
minutes, it does not work.

## What got signed off

The last visual pass was the important one because the remaining bugs were
all "forever program" bugs, not feature-checkbox bugs:

- entry from the normal pause menu shows the frog immediately;
- exit from freeplay shows the frog before the screensaver loop resumes;
- second entry no longer has a dead pause that reads as a freeze;
- world options apply immediately in freeplay;
- R1 plus D-pad changes day/night, tide, raft, and holidays immediately;
- Select clears the screen and rebuilds the island;
- waves run at the same visual cadence as the screensaver path;
- the frame loop stays allocation-free unless telemetry is explicitly enabled.

That is the `v0.5.0-ps1` milestone: the first version where Johnny
Castaway on PS1 is not just watching the island, but walking around on it.
