# Pause Toggles and Captions Handoff

This file is for the next agent picking up after the holiday release.

## Current State

- Repo: `/home/hunter/workspace/jc_ps1_sandbox`
- Released main commit: `686b20dd release: v0.3.10-ps1 - Holiday emblems and pause menu updates`
- Release tag: `v0.3.10-ps1`
- GitHub release: `https://github.com/huntergdavis/Johnny-Castaway-PS1/releases/tag/v0.3.10-ps1`
- Current feature branch created from released `main`: `feature/pause-toggles-captions`
- No implementation edits were made on this branch before this handoff file.

The user explicitly wants all future code work done on a feature branch and merged to `main` only after they approve.

## Goal

Implement:

- A cleaner pause-menu sub-screen for environment/options toggles.
- Selectable day/night state.
- Selectable holiday state.
- Selectable raft state.
- Selectable high-tide/low-tide state.
- Closed caption toggle.
- Closed caption display during scenes, using the existing scene-to-caption data.

After implementation, build and run for review. Do not merge to `main` until the user gives explicit go-ahead.

## Files Already Inspected

- `src/pause_menu.c`
- `src/pause_menu.h`
- `src/jc_reborn.c`
- `src/foreground_pilot.c`
- `src/foreground_pilot.h`
- `src/ps1_captions.c`
- `src/ps1_captions.h`
- `src/ps1_accessibility.c`
- `src/ps1_accessibility.h`
- `src/memcard.c`
- `src/memcard.h`
- `CMakeLists.txt`

## Important Findings

`src/pause_menu.c` currently has the main menu items:

- Resume
- Sound
- Day/Night
- Holiday
- Save Settings to Memcard
- Reset Screensaver Loop
- Next Scene
- TTY Perf Log
- Debug Info
- Set Time/Date

It already has a pause sub-screen enum shape via `enum PauseMenuState`, including `PAUSE_MENU_MAIN`, `PAUSE_MENU_SET_TIME`, `PAUSE_MENU_SCENE_INFO`, and `PAUSE_MENU_CONTROLS`.

`src/jc_reborn.c` currently exposes:

```c
int hostForcedNight = -1;
int hostForcedHoliday = -1;
```

But these are still static and need to become externally controllable if the pause menu is going to change them:

```c
static int hostForcedLowTide = -1;
static int hostForcedRaftStage = -1;
```

`fgLoopApplyVariant()` in `src/jc_reborn.c` already honors those variables:

```c
islandState.lowTide = (hostForcedLowTide >= 0) ? hostForcedLowTide : (rand() & 1);
islandState.raft = (hostForcedRaftStage >= 0) ? hostForcedRaftStage : (rand() % 6);
```

So the pause menu mainly needs extern access and cycling UI.

`src/ps1_captions.c` and `src/ps1_captions.h` already exist. They provide:

```c
void captionsSetEnabled(int enabled);
int captionsGetEnabled(void);
void captionsOnSceneStart(const char *sceneId);
void captionsOnAdsStart(const char *adsName, uint16 adsTag);
const char *captionsGetCurrent(void);
```

But `CMakeLists.txt` does not include:

```text
src/ps1_captions.c
src/ps1_accessibility.c
```

At minimum, add `src/ps1_captions.c` once captions are wired.

The caption data currently lives as `static const` arrays inside `src/ps1_captions.h`. That is not ideal because every translation unit including the header gets a private copy. If only `ps1_captions.c` includes it, it is tolerable. Better cleanup: move the `captions[]` and `captionSceneMap[]` arrays into `ps1_captions.c` and leave only structs plus function declarations in the header.

## Suggested Pause Menu Shape

Keep the main menu short:

- Resume
- Options
- Save Settings to Memcard
- Reset Screensaver Loop
- Next Scene
- Debug Info
- Set Time/Date

Create a new options/environment sub-screen for:

- Sound: ON/MUTED
- Day/Night: AUTO/DAY/NIGHT
- Tide: AUTO/HIGH/LOW
- Raft: AUTO/NONE/STAGE 1/STAGE 2/STAGE 3/STAGE 4/STAGE 5
- Holiday: AUTO/NONE/holiday list
- Captions: OFF/ON
- Perf Log: OFF/SUMMARY/DETAIL/DEBUG

Reusing `PAUSE_MENU_CONTROLS` as an "Options" screen is possible, but cleaner is adding a new enum value such as `PAUSE_MENU_OPTIONS`. The old Controls screen can be removed or left reachable from Debug Info if desired.

## Toggle Semantics

Use this convention:

- `-1` means AUTO/random/date-driven.
- `0` means explicit off/none/high tide/day depending on field.
- Positive values mean explicit enabled/specific variant.

Recommended labels:

- Day/Night: `AUTO`, `DAY`, `NIGHT`
- Tide: `AUTO`, `HIGH`, `LOW`
- Raft: `AUTO`, `NONE`, `1`, `2`, `3`, `4`, `5`
- Holiday: existing `holidayShortName()` flow
- Captions: `OFF`, `ON`

For raft, current runtime uses `rand() % 6`, so valid forced stages are probably `0..5`. Confirm visually whether `0` means no raft. If so, label `0` as `NONE`.

## Captions Wiring

Likely places:

- Include `ps1_captions.h` from `foreground_pilot.c` or `jc_reborn.c`.
- Call `captionsOnSceneStart()` when a foreground pilot scene starts.
- Display `captionsGetCurrent()` every frame after scene composition and before display flip.

For the current validated fgpilot scene names (`fishing1`, `fishing2`, possibly `fishing3`), direct scene IDs do not match `sceneNN`. There are two options:

1. Add a small mapping from fgpilot names to caption IDs.
2. Use ADS/tag mapping if the runtime knows the ADS file and tag.

Existing caption map says:

```c
{"scene17", "FISHING", 1},
{"scene18", "FISHING", 2},
{"scene19", "FISHING", 3},
{"scene20", "FISHING", 4},
{"scene21", "FISHING", 5},
{"scene22", "FISHING", 6},
{"scene23", "FISHING", 7},
{"scene24", "FISHING", 8},
```

If `fishing1` corresponds to `FISHING` tag 1, then call:

```c
captionsOnAdsStart("FISHING", 1);
```

Confirm exact mapping against generated foreground packs or existing scene manifests before relying on it.

## Caption Rendering

`pause_menu.c` has a working custom 8x8 ASCII font uploader and SPRT text renderer. Do not copy the whole pause menu into captions unless necessary.

Practical implementation choices:

1. Extract/reuse a minimal text renderer shared by pause menu and captions.
2. Simpler first pass: add a small caption renderer to `pause_menu.c` or a new `ps1_caption_overlay.c` using the same font texture constants and glyph data.
3. If reusing pause font, ensure it is uploaded before captions render. The font texture occupies:

```c
#define PM_FONT_VRAM_X 640
#define PM_FONT_VRAM_Y 256
#define PM_CLUT_VRAM_X 640
#define PM_CLUT_VRAM_Y 360
```

For captions, draw near the bottom of the 640x480 frame, centered or left aligned inside a dark semi-transparent band. Keep text width conservative, around 35 characters per line as the data suggests.

Avoid drawing captions while the pause menu is visible, or let the pause menu cover them.

## Memcard and Boot Override Notes

The user previously noticed saved game settings can override boot flag settings. There was discussion about either:

- A boot flag deciding whether memcard settings are honored.
- Or boot parameters always overriding saved game storage.

Prefer the second behavior for explicit boot flags: boot parameters should win over loaded memcard values. The code already has:

```c
static int hostBootForcedNightValid = 0;
static int hostBootForcedHolidayValid = 0;
```

Extend this pattern if adding boot overrides for tide/raft/captions, but keep the first implementation focused on pause-menu selection unless needed.

`src/memcard.c` currently persists sound, day/night override, holiday override, and soft time/date. If persisting raft/tide/captions:

- Preserve backward compatibility.
- Bump or extend the save layout carefully.
- Keep old saves readable.

If time is tight, implement runtime toggles first, then decide with the user whether raft/tide/captions should persist.

## Build and Run Commands

Quick checks:

```bash
python3 scripts/holidays-test.py
./scripts/build-host.sh
./scripts/build-ps1.sh
./scripts/make-cd-image.sh
```

Run for review:

```bash
RUN_TIMEOUT_SECONDS=600 ./scripts/rebuild-and-let-run.sh
```

If only doing incremental PS1 rebuild:

```bash
RUN_TIMEOUT_SECONDS=600 ./scripts/rebuild-and-let-run.sh noclean
```

Known warning noise from current main includes PSn00bSDK pedantic enum/anonymous union warnings, `memcard.c` implicit `VSync`, and existing pause/island warnings. Do not confuse those with new failures.

## Workflow Constraint

Do not merge to `main` without explicit user approval.

Work on:

```bash
feature/pause-toggles-captions
```

When ready for review:

```bash
git status --short --branch
./scripts/build-ps1.sh
./scripts/make-cd-image.sh
RUN_TIMEOUT_SECONDS=600 ./scripts/rebuild-and-let-run.sh noclean
```

Then ask the user to review in DuckStation.
