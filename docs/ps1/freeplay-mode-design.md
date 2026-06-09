# Castaway Freeplay And Debug Mode

> Rendered version: `/docs/freeplay/` on the project website.

**Status:** signed off for `v0.5.0-ps1`

**Release:** `v0.5.0-ps1`

**Runtime slug:** `freeplay`
**Primary entry:** normal screensaver → Start menu → Freeplay

## Purpose

Freeplay is the direct-control Johnny mode. It is also the debugging
cockpit for interactive asset tests that do not fit into the passive
screensaver loop.

Every ordinary PS1 scene is an `.FG2` foreground pack captured by the host
tooling and replayed on the console. Freeplay cannot be a captured pack:
controller input branches every frame. It is a live PS1 scene in
`src/scene_freeplay/scene_freeplay.c`, built on the same background, clean-rect, wave,
holiday, caption, sound, and pause-menu infrastructure as the rest of the
port.

## Entry And Exit

Normal release flow:

1. Boot into ordinary screensaver mode.
2. Press Start.
3. Select `Freeplay: OFF`.
4. The current scene tears down.
5. The `MEANWHIL.BMP` frog clock appears as the loading indicator.
6. Freeplay builds its island backdrop and starts.

Exit flow:

1. Press Start inside freeplay.
2. Select `Freeplay: ON`.
3. Freeplay releases action slots, clean rects, captions, and backdrop
   state.
4. The frog clock appears again while the normal loop takes back over.
5. The next screensaver scene starts from a clean story-loop state.

Boot-token test entry still exists:

```text
fgpilot freeplay
```

The release path should be tested through the menu, because the menu path
exercises teardown, loading feedback, and return-to-loop behavior.

## Final Controller Contract

Live freeplay controls are intentionally small:

| Control | Behavior |
|---|---|
| D-pad / left analog | Walk Johnny 4-way. Movement cancels any active gag/action immediately. |
| L2 held | Slow walk. |
| R2 held | Fast walk. |
| Circle | Fishing. If Johnny is on the right side, draw the same fishing frames flipped horizontally. |
| Select | Clear screen: cancel transient actions, rebuild the clean backdrop, and return to idle. |
| R1 + Up | Toggle day/night immediately. |
| R1 + Down | Toggle high/low tide immediately. |
| R1 + Left | Cycle raft stage immediately. |
| R1 + Right | Cycle holiday overlay immediately. |
| Start | Open pause menu. |

Menu controls:

| Control | Behavior |
|---|---|
| Cross | Select / apply. |
| Circle | Back everywhere. |
| Start | Back/resume, depending on current menu level. |
| D-pad / analog | Move cursor or adjust values. |

Removed from the live controller layer:

- contextual Cross actions;
- L1 direct-action shortcuts;
- Triangle/Square gag cycling;
- hidden multi-shoulder carnival combo;
- R1 + face-button environment shortcuts;
- drunk walk.

Those ideas made the mode harder to understand. Catalog-like behavior now
belongs in menus where it can be named and described.

## Pause Menu Structure

No submenu should exceed the visible PS1-safe menu height. If a feature
needs more than five or six rows, it gets a selector/catalog screen.

Current freeplay-relevant menus:

| Menu | Rows / behavior |
|---|---|
| Main | Resume, Freeplay ON/OFF, Freeplay Options, World Options, Accessibility, System. |
| Freeplay Options | Gags, Visitors, Controls, Clear, Back. |
| Freeplay Gags | Selector page with title, source BMP, frame count, rough memory cost, description, and `X spawn now`. |
| Freeplay Visitors | Same selector pattern for outside-world events. |
| World Options | Day/Night, Tide, Raft, Holidays, Island Position, Back. |
| Holidays | Set mode: Auto Date / None / Original 4 / Expanded; selected holiday within the active set. |
| Sound Test | Selector page for individual SPU effects. |
| Controls | On-device reference for the final freeplay controls. |

When a world option changes while freeplay is active, the menu closes, the
frog clock appears, freeplay applies the same state mutation as the live
`R1 + D-pad` controls, and the backdrop is rebuilt immediately.

## Runtime Architecture

`freeplayRun()` owns a tight loop:

```c
while (!gFreeplayExitRequested) {
    cur = fpReadButtons();
    pressed = cur & ~gFp.prevButtons;
    gFp.prevButtons = cur;

    fpApplyInput(cur, pressed);
    fpTick();
    fpFrame();
    gFp.frame++;
}
```

The frame path is:

1. `grBeginFrame()`
2. restore freeplay clean rects
3. tick waves on the throttled cadence
4. draw active fire/summon/action/Johnny
5. re-stamp active holiday overlay
6. mark banner/caption overlay rows dirty
7. present

The frame path must not allocate memory.

## Fishing

Circle is the dedicated fishing button. The freeplay fishing animation is
the **boring idle** — Johnny stands with the rod horizontal forward,
holding the line and waiting. The cast windup and the
get-pulled-by-fish frames are intentionally skipped: users want the
patient idle, not the frenetic catch.

Implementation: `MJFISH1.BMP` frames 9-13 (five idle poses) looped for
360 ticks (~6 s). Frames 0-8 are the cast windup; 14-19 are getting
pulled. Neither plays in freeplay. `MJFISH2.BMP` (the post-catch /
throw-fish-back reactions) is also unused.

There is no separate left-facing fishing asset. The native sprite has
Johnny on the left of the frame with the rod extending right (facing
right). The mirror threshold is the island midpoint
(`FP_FISH_CENTER_X = 390`, halfway between the 245 and 535 clamp
bounds). When Johnny is on the **left** half (`x < 390`), the sprite is
mirrored so he faces left into the left-hand water. On the right half
the native sprite is drawn unmodified.

## World State

Freeplay owns the visible `islandState` while active.

Immediate live toggles:

- day/night;
- high/low tide;
- raft stage 0..5;
- holiday overlay.

Pause-menu world options write through the same state:

- forced day/night, or Auto from soft time;
- forced tide, or current auto state;
- forced raft stage, or current auto state;
- forced holiday, None, Original 4, Expanded set, or Auto Date;
- forced island position.

When the visible state changes, freeplay cancels transient actions and
rebuilds the static backdrop before resuming input.

## Loading Indicator

All long freeplay transitions use the same frog clock:

- entering freeplay from the pause menu;
- exiting freeplay back to the screensaver loop;
- clearing the freeplay screen;
- applying world-option changes from the pause menu.

The helper draws `MEANWHIL.BMP` into a temporary empty background, presents
one frame, and releases the temporary `TTtmSlot` unconditionally. It is not
kept resident.

## Memory And Long-Run Rules

This mode is designed to run forever. Red-team every freeplay change
against these rules:

- no heap allocation in `fpFrame()`, `fpTick()`, or normal movement;
- no BMP load in the per-frame path;
- all optional action assets are fail-soft;
- every slot load has a matching release before replacement or exit;
- `JOHNWALK.BMP` is mandatory for entry; optional gags/visitors may skip;
- freeplay clean rects are hard-freed on exit, not merely deactivated;
- overlay banners/captions mark their screen rows dirty before present;
- heap-largest probes are telemetry-gated and absent from the default path;
- the frog loading helper releases `MEANWHIL.BMP` even after partial load
  failure;
- world rebuild and clear-screen paths must cancel transient action slots
  before rebuilding the backdrop.

The default freeplay path should not change heap shape just by being
entered, exited, and re-entered repeatedly.

## Telemetry

Boot tokens:

| Token | Level |
|---|---|
| `freeplay-log` | summary lines every 300 frames |
| `freeplay-detail` | asset and clean-rect detail |
| `freeplay-debug` | on-screen debug line |

Summary fields include frame count, mode, position, walk/action/summon
counts, clean-rect rebuilds/failures, asset failures, clean-rect bytes, and
largest heap probe. The probe is intentionally absent unless telemetry is
enabled.

## Signoff Checklist

- Enter freeplay from a normal scene and see frog loading immediately.
- Exit freeplay and see frog loading before the next story scene.
- Enter freeplay a second time and see frog loading without a confusing
  dead pause.
- Walk with D-pad and analog.
- L2 slows, R2 speeds up.
- Circle fishes on left and right; right side is mirrored.
- Start opens pause; Circle backs out of every submenu.
- R1 + D-pad world toggles apply immediately.
- World Options menu changes apply immediately when freeplay is active.
- Select clear-screen cancels leftovers and rebuilds the island.
- Freeplay can be entered/exited repeatedly without BSOD or heap drift.

All items above were visually signed off before the `v0.5.0-ps1` release.
