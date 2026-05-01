# v0.5.0-ps1 Release Notes

**Date:** 2026-05-01  
**Tag:** `v0.5.0-ps1`  
**Theme:** Freeplay and debug mode

`v0.5.0-ps1` is the first release where the PS1 port is not only a
screensaver playback engine. Johnny can be driven directly around the
island with the controller, using the same background, wave, holiday,
caption, sound, and clean-rect systems that the signed-off story scenes use.

## Headline Feature

Freeplay mode is now menu-launched from normal screensaver playback:

1. Boot into the usual scene loop.
2. Press Start.
3. Select `Freeplay: OFF`.
4. The current scene tears down.
5. The original `MEANWHIL.BMP` frog clock appears.
6. Freeplay builds the live island and hands control to the player.

Inside freeplay, Start opens the same pause menu and `Freeplay: ON` exits
back to the normal screensaver loop. Entry, exit, clear-screen, and world
rebuild transitions all show the frog clock so expensive teardown/rebuild
work never looks like a frozen game.

## Freeplay Controls

| Control | Behavior |
|---|---|
| D-pad / left analog | Walk Johnny around the island. Movement cancels the current action immediately. |
| L2 held | Slow walk. |
| R2 held | Fast walk. |
| Circle | Fish from the nearest side of the island. Right-side fishing mirrors the left-facing frames. |
| Select | Clear the freeplay screen, cancel transient actions, rebuild the backdrop, and return Johnny to idle. |
| R1 + Up | Toggle day/night immediately. |
| R1 + Down | Toggle high/low tide immediately. |
| R1 + Left | Cycle raft stage 0..5 immediately. |
| R1 + Right | Cycle holiday overlay immediately. |
| Start | Open the pause menu. |

Menu navigation is standardized: Cross selects, Circle backs out everywhere,
Start resumes or backs out depending on depth.

## Debug Cockpit

The pause menu now carries the catalog features that were too confusing as
button chords:

- **Freeplay Options**: Gags, Visitors, Controls, Clear.
- **Freeplay Gags**: named Johnny actions with source BMP, frame count,
  description, and rough memory cost.
- **Freeplay Visitors**: named outside-world events with the same metadata.
- **World Options**: Day/Night, Tide, Raft, Holidays, Island Position.
  In freeplay, changes apply immediately after a frog-clock rebuild.
- **Sound Test**: selector-style SPU effect player.
- **Accessibility**: captions and sound-accessibility toggles.
- **Controls**: on-device reference for the final freeplay controls.

Gag and visitor loads are fail-soft. If an optional asset cannot load, the
mode reports the skip instead of crashing.

## Rendering And Memory

Freeplay is live C code, not a captured FG2 pack. That made the memory bar
stricter, not looser:

- no allocation in the steady-state movement/tick/draw path;
- optional BMP loads only at menu or action boundaries;
- every optional action slot is released before replacement or exit;
- the freeplay clean-rect snapshot is hard-freed on exit;
- caption/banner rows mark their dirty area before present;
- largest-heap probes are behind telemetry flags, not part of normal entry;
- the frog loading helper releases its temporary `MEANWHIL.BMP` slot
  unconditionally;
- waves advance on a throttled freeplay cadence so the surf matches the
  normal screensaver feel.

This release keeps the `v0.4.20-ps1` story-loop walking work intact:
normal scenes still connect through walking transitions, and freeplay reuses
the same Johnny movement foundation instead of carrying a second renderer.

## Verification

Release candidate checks:

- `./scripts/build-ps1.sh`
- `./scripts/make-cd-image.sh`
- website static build under `docs/`
- website red-team relative-link pass
- human visual signoff in DuckStation for freeplay entry, walking, fishing,
  pause-menu world changes, clear-screen, frog transitions, exit, and re-entry

The primary acceptance gate remains human visual and audible signoff. The
automation answers "does it build and route correctly"; the signed-off run
answers "does Johnny feel right on the island."
