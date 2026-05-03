# Pause Menu Design

> Rendered version: `/docs/pause-menu/` on the project website.

**Status:** current for `v0.5.0-ps1`

**Runtime files:** `src/pause_menu.c`, `src/pause_menu.h`
**Related:** [freeplay-mode-design.md](freeplay-mode-design.md)

The pause menu is the PS1 port's in-game control room. It opens with
Start, draws a translucent dim quad and compact panel over the current
framebuffer, and renders text with the embedded 8x8 ASCII font atlas shared
with closed captions.

## Controller Contract

| Control | Behavior |
|---|---|
| Start | Open pause menu. Inside the menu, resume or back out depending on depth. |
| D-pad / left analog | Move cursor, change selector values, or edit numeric fields. |
| Cross | Select / apply. |
| Circle | Back everywhere. |

Circle is the global back button. No submenu gets to invent a second meaning
for it.

## Menu Structure

The `v0.5.0` menu is split into compact sub-screens so no page grows past the
visible PS1-safe panel.

| Main entry | Purpose |
|---|---|
| Resume | Close the menu and continue. |
| Freeplay ON/OFF | Enter freeplay from normal mode, or exit freeplay back to the screensaver loop. |
| Freeplay Options | Gags, visitors, controls, and clear-screen tools. |
| World Options | Day/night, tide, raft, holiday, and island position. |
| Accessibility | Captions, sound mute, ocean ambience, and sound test. |
| System | Save settings, set time/date, set RNG seed, perf log, reset scene, next scene. |

## State Enum

`src/pause_menu.h` exposes:

```c
enum PauseMenuState {
    PAUSE_MENU_MAIN,
    PAUSE_MENU_FREEPLAY_OPTIONS,
    PAUSE_MENU_FREEPLAY_GAGS,
    PAUSE_MENU_FREEPLAY_VISITORS,
    PAUSE_MENU_OPTIONS,
    PAUSE_MENU_HOLIDAYS,
    PAUSE_MENU_ACCESSIBILITY,
    PAUSE_MENU_SOUND_TEST,
    PAUSE_MENU_SYSTEM,
    PAUSE_MENU_SCENE_INFO,
    PAUSE_MENU_CONTROLS,
    PAUSE_MENU_SET_TIME,
    PAUSE_MENU_ISLAND_POS,
    PAUSE_MENU_SET_SEED,
    PAUSE_MENU_CREDITS,
};
```

`PAUSE_MENU_SCENE_INFO` and `PAUSE_MENU_CREDITS` remain compiled states from
the earlier menu generation, but they are not surfaced from the compact
`v0.5.0` System page. The active public diagnostics path is telemetry, and
the public credit/legal text lives on the website.

## Freeplay Integration

The pause menu drives freeplay through one-shot request flags:

- `pauseMenuRequestFreeplay`
- `pauseMenuRequestExitFreeplay`
- `pauseMenuRequestFreeplayGag`
- `pauseMenuRequestFreeplayVisitor`
- `pauseMenuRequestFreeplayClear`
- `pauseMenuRequestFreeplayWorldRefresh`

`scene_freeplay.c` consumes the requests, clears them, and performs the
heavy work outside the pause-menu draw path. Enter/exit/clear/world-rebuild
paths show the meanwhile frog loading frame before rebuilding the live island.

## World Options

World Options owns:

- Day/Night;
- Tide;
- Raft stage;
- Holiday mode and selection;
- Island position.

When freeplay is active, changing these values closes the menu, sets
`pauseMenuRequestFreeplayWorldRefresh`, shows the frog loading frame, cancels
transient actions, and rebuilds the backdrop immediately. In normal
screensaver mode, the settings affect the next applicable scene/variant path.

## Accessibility And Sound Test

Accessibility owns captions, sound mute, ocean ambience, and Sound Test. Sound
Test is a selector over the SPU effects. It calls the same `soundPlay()` path
used by scene playback and freeplay actions, which makes individual samples
testable without waiting for a scene to hit the correct frame.

## Render Pipeline

The menu does not modify background tile pixels. Each pause frame:

1. waits for VBlank;
2. restores the raw scene frame once when opening;
3. draws a semi-transparent `POLY_F4` dim quad;
4. draws the solid panel;
5. draws text sprites from the embedded font atlas;
6. presents.

On close, `grForceFullRedrawNextFrame()` forces the scene renderer to repaint
over the menu. This avoids the old bug class where a pixel-modified dim pass
left stale screen rows behind after resume.

## Memory Rules

- No heap allocation in the per-frame menu update.
- No background pixel mutation.
- Font upload is idempotent through `pauseMenuEnsureFontUploaded()`.
- Menu actions that require heavy work use request flags and let the scene
  loop handle teardown/rebuild after the menu closes.
- Per-frame `printf` stays off. The menu may emit one-shot diagnostics only
  when explicitly compiled for them.
