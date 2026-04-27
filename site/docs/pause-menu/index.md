---
layout: page
title: Pause menu
eyebrow: Reference
subtitle: State machine, Options sub-screen, three editors, Credits screen, and the shared font atlas.
description: How the Johnny Castaway PS1 pause menu works — state machine, Options toggles, Set Time / Island Pos / Seed editors, the Credits screen, and font sharing with captions.
---

A labor of love by Hunter Davis. The pause menu is the PS1 build's only
in-game UI. It opens with **Start**, dims the framebuffer with a translucent
quad, draws a solid panel, and renders text using an embedded 8x8 ASCII
font that lives in VRAM. The same font atlas is shared with the
[closed-captions]({{ '/docs/captions/' | relative_url }}) module, which
is why the captions module calls `pauseMenuEnsureFontUploaded()` on first
use rather than waiting for the user to ever open the pause menu.

Earlier versions of `pause_menu.c` were 540 lines of fully-written,
link-broken, never-called code with three undefined externs and no entry
in the CMake source list. The current implementation is wired up properly,
exits the rect-mode `dimBackground` pixel-modify trap by drawing the dim
as a `POLY_F4` quad, and shares the OT carefully with `FntFlush` so font
rendering and the quads don't fight.

If you paid for this, you were cheated. Open source and free.

## Controller mapping

| Button       | Action |
|--------------|--------|
| **Start**    | Open the pause menu. While inside, Start backs out of any sub-screen and resumes play. |
| **D-pad ↑↓** | Move selection in the active list. |
| **D-pad ←→** | In editors, change the field under the cursor. In the Options screen, toggle the highlighted setting. |
| **Cross (✕)** | Confirm / enter the highlighted entry. |
| **Triangle (△)** | Back out one level. |
| **Select**   | Reserved (no current binding). |

Navigation debounces via the menu's own `prevButtons` cache so a long-press
doesn't autorepeat unintentionally.

## State machine

`enum PauseMenuState` lives in
[`src/pause_menu.h`]({{ site.github_url }}/blob/main/src/pause_menu.h):

```c
enum PauseMenuState {
    PAUSE_MENU_MAIN,
    PAUSE_MENU_OPTIONS,
    PAUSE_MENU_SCENE_INFO,
    PAUSE_MENU_CONTROLS,    /* legacy / no longer surfaced from main */
    PAUSE_MENU_SET_TIME,
    PAUSE_MENU_ISLAND_POS,
    PAUSE_MENU_SET_SEED,
    PAUSE_MENU_CREDITS,
};
```

- **`PAUSE_MENU_MAIN`** is the entry screen. It's the surface the player
  sees when Start is first pressed.
- **`PAUSE_MENU_OPTIONS`** is a flat toggle list; see below.
- **`PAUSE_MENU_SCENE_INFO`** is a single dense diagnostic page reading
  from `gPs1Perf` via the `ps1PerfGet*` accessors. Live: scene name, loop
  iteration, variant flags, pilot mode, pack name + frame index, free RAM,
  uptime, build date, and (when perf is enabled) the full counter block.
- **`PAUSE_MENU_SET_TIME`**, **`PAUSE_MENU_ISLAND_POS`**, and
  **`PAUSE_MENU_SET_SEED`** are editors — see *Editors* below.
- **`PAUSE_MENU_CREDITS`** is the four-line drawCredits screen. Voice
  anchor for the entire site.
- **`PAUSE_MENU_CONTROLS`** is a legacy reference card kept around but
  not surfaced from the main menu in current builds.

`pauseMenuGetState()` / `pauseMenuSetState()` expose the current state to
external callers; `pauseMenuShow()` / `pauseMenuHide()` are the open /
close hooks; `pauseMenuUpdate()` runs one frame and returns 1 while the
menu should stay open or 0 when the user resumes.

## Options sub-screen

Flat list of toggles. Each flips with D-pad ←/→ or Cross.

| Setting    | States | Persists | Notes |
|------------|--------|----------|-------|
| Sound      | ON / OFF | session | Toggles `soundDisabled`. On switching to OFF, freeplay-style `soundMuteAll()` calls `SpuSetKey(0, 0xFFFFFF)` so any in-flight SFX stop. |
| DayNight   | DAY / NIGHT | session | Forces the palette state used by background rendering. |
| Tide       | HIGH / LOW | session | Mirrors `islandState.lowTide`. Affects pack selection in `fgCompactOverlayPackPathForScene`. |
| Raft       | 0..5 | session | Cumulative raft-build stage. |
| Holiday    | none / 1..36 | session | Cycles through `gHolidays[]` via `holidayNextId` / `holidayPrevId`. See [Holidays]({{ '/docs/holidays/' | relative_url }}). |
| Captions   | ON / OFF | session | Toggles `captionsSetEnabled`. See [Closed captions]({{ '/docs/captions/' | relative_url }}). |
| Perf       | ON / OFF | session | Calls `ps1PerfSetEnabled(0/1)`. When OFF, the Scene Info page's perf block shows zeros + a `[perf disabled — boot with 'perf' to enable]` hint. |

Nothing persists across power cycles — there's no memory-card surface for
settings. Boot tokens (e.g. `perf`, `night 1`, `lowtide 1`, `holiday N`,
`raft-stage N`) are how persistent variants are pinned across runs.

## Editors

Three editors share a five-field cursor pattern. D-pad ←/→ moves the cursor
between fields, ↑/↓ increments / decrements, Cross commits, Triangle
backs out.

### Set Time / Date

Edits `ps1SoftHour`, `ps1SoftMonth`, `ps1SoftDay` (defined in `utils.c`,
default 12 / 6 / 30). The PS1 has no real-time clock — these are the
software-side date inputs that decide which holiday (if any) is in
effect, whether the night palette applies, etc. Editing them lets the
author force any date for visual testing without rebuilding the disc.

### Set Island Pos

Edits the island position offset (the same surface that responds to
`island-pos <x> <y>` boot tokens — see [Development workflow]({{ '/docs/dev-workflow/' | relative_url }})).
Useful when validating that scenes still composite correctly at non-default
island offsets.

### Set RNG Seed

Edits the seed used to drive screensaver scene rotation and per-scene
variant choices. Pinning a seed is how a specific scene ordering gets
reproduced run-to-run for screenshot comparison.

## Credits

The credits screen is the voice anchor for the project. Four lines:

> A labor of love by Hunter Davis.
>
> Hunter does not own or have a license to the Johnny Castaway character. The original creator generously allows fan ports.
>
> If you paid for this, you were cheated.
>
> Open source and free.

It is the same text that appears on
[the home page]({{ '/' | relative_url }}),
[the about page]({{ '/about/' | relative_url }}), and at the bottom of
the [credits page]({{ '/credits/' | relative_url }}). When the in-game
text and the website text drift, the website is wrong.

## Render pipeline (per pause frame)

The pause menu renders into the runtime's existing OT (`ot[0]`), then
calls its own `DrawSync` and `VSync` because the runtime's main loop is
suspended while the menu is up. Order:

1. `ClearOTagR(ot[0], OT_LENGTH)` — claim the runtime's OT.
2. Build the dim quad at OT priority N (back) — `POLY_F4`, semi-trans 50%,
   RGB(0,0,0), full screen.
3. Build the panel quad at priority N-1 (in front of dim, behind text) —
   `POLY_F4`, no semi-trans, RGB(0x10, 0x18, 0x40), centered ~520x320.
4. `DrawOTag(&ot[0][OT_LENGTH-1])` — render the quads.
5. `DrawSync(0)` — wait for GPU.
6. `FntPrint` text content (PSn00bSDK BIOS font path uses its own
   primitive list, separate from `ot[0]`).
7. `FntFlush(fontID)` — composites text on top.
8. `VSync(0)` — pace at 60 Hz.

The framebuffer-priming detail: bgTile pixels are *never* modified by the
pause menu. On resume, the runtime's next `grDrawBackground` re-uploads
the bg from the dirty-row record — the bg state has been correct in tile
RAM the whole time. The earlier pixel-modify approach was broken because
(a) it never marked tiles dirty, so nothing actually re-uploaded, and
(b) rect-mode scenes have `bgTileNClean` arrays set to NULL, so even with
the dirty mark there was no clean copy to restore from.

## Font sharing with captions

```c
#define PAUSE_FONT_VRAM_X  640
#define PAUSE_FONT_VRAM_Y  256
#define PAUSE_CLUT_VRAM_X  640
#define PAUSE_CLUT_VRAM_Y  360
#define PAUSE_GLYPH_FIRST  0x20
#define PAUSE_GLYPH_COUNT  96
#define PAUSE_GLYPH_DRAW_W 16
#define PAUSE_GLYPH_DRAW_H 16

void pauseMenuEnsureFontUploaded(void);
```

Both the captions module and any other text overlay want the same 8x8
ASCII font. `pauseMenuEnsureFontUploaded()` is idempotent: first call
uploads the atlas + CLUT to VRAM at the constants above, every subsequent
call is a no-op. The captions module (`captionsRender`) calls it before
building its `SPRT` primitives so subtitle rendering works even if the
player has never opened the pause menu in the current session.

## Mute on pause-show

When `pauseMenuShow()` is called, the menu calls `soundMuteAll()` —
`SpuSetKey(0, 0xFFFFFF)` — to silence any in-flight SPU voices. Otherwise
SFX that were already key-on'd at the moment of pause keep playing through
the menu. This is one-shot; resuming does not re-key any voices.

The menu also emits a one-shot `JCPAUSE` TTY snapshot on show — never
per-frame, never on resume. Per-frame `printf` is forbidden in this
codebase because text I/O alters timing and fills the DuckStation log
file.

## Scene-switch flags

```c
extern int pauseMenuRequestNextScene;
extern int pauseMenuRequestResetLoop;
```

Both flags are one-shot. Set by the menu, consumed by the foreground
pilot's runtime loop, cleared after consumption.
`pauseMenuRequestResetLoop` triggers a fresh random pick from
`kProvenScenes` plus re-randomized variants — the "give me a different
scene" button. The foreground pilot's ocean-runtime while-loop checks the
flag every frame and exits early so the outer screensaver loop can pick
again.

## Real-hardware notes

- No DuckStation-specific shortcuts. Only PSn00bSDK APIs.
- `POLY_F4` + `setSemiTrans(p, 1)` is a standard PSn00bSDK pattern that
  works identically on hardware and emulator.
- No new VRAM math; the pause menu reuses the existing pause-font region.
- No allocations during pause; quad primitives are <32 bytes each in the
  primitive buffer, font rendering uses its own buffer.

## Related pages

- [Closed captions]({{ '/docs/captions/' | relative_url }}) — shares the
  font atlas; toggled from the Options sub-screen.
- [Holidays]({{ '/docs/holidays/' | relative_url }}) — the Holiday
  cycler walks `gHolidays[]`.
- [Freeplay mode]({{ '/docs/freeplay/' | relative_url }}) — the Freeplay
  Mode entry on the main menu launches the runtime-driven scene.

## View source on GitHub

- [`docs/ps1/pause-menu-design.md`]({{ site.github_url }}/blob/main/docs/ps1/pause-menu-design.md) — locked design and risk register.
- [`src/pause_menu.h`]({{ site.github_url }}/blob/main/src/pause_menu.h) — public API.
- [`src/pause_menu.c`]({{ site.github_url }}/blob/main/src/pause_menu.c) — implementation.
