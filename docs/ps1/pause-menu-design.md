# Pause Menu — Design

**Status**: design locked, implementation not started
**Branch**: `pause-menu-wire` (to be created at start of work)
**Audience**: PS1 hardware target (DuckStation for testing, real hardware compatibility is a hard requirement). Host build out of scope.

---

## 1. Why the current pause never worked

Two compounding findings from the audit:

1. **`pause_menu.c` is 540 lines of fully-written, link-broken, never-called code.**
   - Not in `CMakeLists.txt` — has been an orphan source file for at least the recent cleanup
   - Even if added to the build, three undefined externs would block linking (`ps1SoftHour`, `ps1SoftMonth`, `ps1SoftDay`)
   - And `pad_buff` is `static` in `events_ps1.c`, breaking the existing `extern uint8 pad_buff[2][34];` reference
   - Zero call sites for `pauseMenuInit` / `pauseMenuShow` / `pauseMenuUpdate` anywhere in the tree

2. **What pause does today**: `events_ps1.c::eventsWaitTick` toggles a primitive `pause = !pause; while (paused) VSync(0);` busy-loop. Screen freezes on the last frame, no UI, no menu, no escape until you press Start again. That's why pressing Start "feels broken."

The plan: wire it up properly, fix the latent link bugs, and make the existing screens great while we're in there.

---

## 2. Decisions locked

| # | Item | Locked answer |
|:-:|---|---|
| 1 | Visual style | Solid translucent panel + dimmed background (via POLY_F4 quads — see §6) |
| 2 | Reset behavior | Fresh random pick from `kProvenScenes` + re-randomize variants |
| 3 | Sound persistence | Session-only (no memory card) |
| 4 | Quit option | Removed |
| 5 | Build-info | Debug-info screen only |
| 6 | Host parity | Out of scope (PS1 only) |
| 7 | Debug refresh rate | Per-frame (60 Hz; pause is paused, plenty of CPU) |
| 8 | Reset auto-resume | Yes — close menu immediately on Reset |
| 9 | TTY `JCPAUSE` snapshot on Show | Yes — one-shot printf, never per-frame |
| 10 | Debug Info layout | Single dense page |
| 11 | Toggle Perf Counters | Separate menu item (calls `ps1PerfSetEnabled` at runtime) |

---

## 3. Final menu structure

| Order | Item | Notes |
|:-:|---|---|
| 1 | **Resume** | Closes menu |
| 2 | **Sound: ON / OFF** | Toggles `soundDisabled` |
| 3 | **Debug Info ▶** | Live page reading `gPs1Perf` via new accessors |
| 4 | **Toggle Perf Counters: ON / OFF** | Calls `ps1PerfSetEnabled(0/1)` |
| 5 | **Set Time / Date ▶** | Existing 5-field editor (after preflight Blocker 2 fix) |
| 6 | **Controls ▶** | Reference card |
| 7 | **Reset Screensaver Loop** | Auto-resume; fresh random pick |
| 8 | (grayed) Captions | Placeholder |
| 9 | (grayed) Scene Order | Placeholder |
| 10 | (grayed) Freeplay Mode | Wires up in freeplay-mode Phase 6 |

Navigation: D-pad up/down, Cross to select, Start to resume immediately from any sub-screen.

---

## 4. Debug Info sub-screen — single dense page

```
═══════════════ DEBUG INFO ═══════════════

  Scene:        fishing3   (loop iter 4)
  Variant:      night · low-tide · raft 3 · holiday off
  Pilot mode:   FG_RUNTIME_SCENE_PACK

  Pack:         FISHING3.FG2   frame 166/174
  Free RAM:     847 KB         Uptime: 00:14:32
  Build:        Apr 25 2026

  ──── Perf counters (this scene) ────

  Loops:        rendered 847   held 3208
  Advances:     174   late 0   max-elapsed 1 vb
  Entries:      max payload 9216 B   max hold 11 vb

  CD reads:     174 (0 fail)   total 176 KB
                max CD vb: 2   total CD vb: 312

  GFX restore:  847 calls   4.3 MB
  GFX compose:  847 calls   174 rows · 593 spans
  GFX upload:   847 calls   8.1 MB   max 1 vb

  [perf disabled — boot with `perf` to enable]

═══ START to go back ═══
```

When `ps1PerfEnabled == 0`, the perf block shows zeros + the disabled hint. When ON, real numbers. The non-perf block (Pack / Free RAM / Uptime / Build / Scene / Variant) populates regardless of perf state — it comes from `gFgRuntime`, `getTotalMemoryUsed`, frame counter, `__DATE__`, `gPs1Perf.sceneName` (which is set even when perf is OFF — `ps1PerfBeginScene` always copies the name).

### Data sources

| Field | Source | New code? |
|---|---|---|
| Scene name | `ps1PerfGetSceneName()` (NEW accessor) | + accessor |
| Loop iteration | `gScreensaverIterCount` (NEW global in jc_reborn.c) | ~3 lines |
| Variant flags | `islandState.{night,lowTide,raft,holiday}` | + extern decl |
| Pilot mode | `foregroundPilotRuntimeMode()` (existing) | none |
| Pack name + frame index | `gFgRuntime.sceneName` + `frameIndex` / `header.frameCount` | + accessors |
| Free RAM | `getTotalMemoryUsed()` / `getMemoryBudget()` | none |
| Uptime | `gFrameCount / 60` (NEW global incremented in `grUpdateDisplay`) | ~3 lines |
| Build date | `__DATE__` | none |
| All perf counters | `ps1PerfGet*` accessors (NEW) reading `gPs1Perf` | ~16 inline accessors |

Total new code for telemetry: ~25 lines (down from v3's 70 lines, thanks to perf module already collecting most data).

---

## 5. Render pipeline (locked)

Each pause frame, in this exact order:

1. `ClearOTagR(ot[0], OT_LENGTH)` — claim the runtime's OT
2. Build dim quad at OT priority N (back) — POLY_F4, semi-trans 50%, RGB(0,0,0), full screen
3. Build panel quad at OT priority N-1 (front of dim, behind text) — POLY_F4, no semi-trans, RGB(0x10, 0x18, 0x40), centered ~520×320
4. `DrawOTag(&ot[0][OT_LENGTH-1])` — render quads
5. `DrawSync(0)` — wait
6. `FntPrint` text content (FntFlush has its own primitive path, separate from `ot[0]`)
7. `FntFlush(fontID)` — composites text on top
8. `VSync(0)` — pace at 60 Hz

**Critical: bgTile pixels are never modified.** On resume, just stop drawing the quads — the next runtime frame's `grDrawBackground` re-uploads the bg (which was already correct in tile RAM the whole time).

---

## 6. Visual approach — why POLY_F4 not pixel-modify

The existing `dimBackground()` in `pause_menu.c` (lines 182–201) modifies `bgTile{0,1,3,4}->pixels` in place. **Two compounding bugs**:

### Bug A: doesn't mark dirty
After modifying pixels it does NOT call `grMarkAllTilesDirty()`. So `grDrawBackground()` (uploads only dirty rows) sees clean state and uploads nothing → no visible dimming at all.

### Bug B: no clean copy in rect-mode
Even if we fix (A), fishing scenes use **rect-mode** clean backup. `bgTile{0,1,3,4}Clean` arrays are `NULL` during rect-mode play. On resume, `grRestoreBgFromRects` only restores the rects (foreground bbox + waves) — the rest of the screen stays dimmed forever.

### Why POLY_F4 wins
Three options were considered:

| Option | Approach | Cost | Notes |
|---|---|---|---|
| A — No dim | Solid panel only | 0 lines | Cleanest, but loses the "fade" feel |
| B — Snapshot + restore | malloc 614 KB on open, restore on close | ~40 lines | 614 KB transient alloc on fragmented heap = risk |
| **C — Translucent quad** | POLY_F4 over framebuffer each pause frame | ~20 lines | Picked. bgTile state untouched. |

The codebase has POLY_FT4 (textured) infra for sprite rendering but no POLY_F4 (flat). Adding POLY_F4 + `setSemiTrans` is ~20 lines of standard PSn00bSDK pattern. The same primitive type also does the solid panel (different color, different OT priority, no semi-trans).

---

## 7. Source-level red-team — preflight blockers

Must be addressed in **Phase 0 (preflight)** before P1 even compiles.

### Blocker 1: `pause_menu.c` not in `CMakeLists.txt`
`grep "pause_menu" CMakeLists.txt` returns nothing. Add it to the source list. (`ps1_perf.c` precedent shows the path.)

### Blocker 2: three undefined externs
`pause_menu.c` declares (lines 36, 457–459, 494):
```c
extern int    ps1SoftHour;     // ❌ NO DEFINITION ANYWHERE
extern int    ps1SoftMonth;    // ❌ NO DEFINITION ANYWHERE
extern int    ps1SoftDay;      // ❌ NO DEFINITION ANYWHERE
extern uint8  pad_buff[2][34]; // ❌ static in events_ps1.c
```

Three concrete fixes:
- Define `ps1SoftHour/Month/Day` in `utils.c` (PS1 section), three `int` globals with sane defaults — ~5 lines
- Drop `static` from `pad_buff` declaration in `events_ps1.c:33`

### Blocker 3 (resolved by perf module): scene name population
v3 had `ps1AdsCurrentName` not populated for fgpilot scenes. **Resolved**: read `ps1PerfGetSceneName()` (new accessor over `gPs1Perf.sceneName`), which IS populated by `ps1PerfBeginScene(loopScene)` — called from `jc_reborn.c::main` at the top of every screensaver iteration regardless of perf-enable state.

### Blocker 4: `dimBackground` broken in rect-mode (see §6)
P2 task — replace pixel-modify with POLY_F4 quad approach.

### Blocker 5: OT/primitive ownership during pause
`grUpdateDisplay` builds the OT each frame. When paused, runtime is halted — nothing builds the OT. To render our F4 quads we must:
- `ClearOTagR(ot[0], OT_LENGTH)` ourselves at the top of `pauseMenuUpdate`
- Build quads + `DrawOTag(&ot[0][OT_LENGTH-1])` ourselves
- `FntFlush` already has its own primitive path independent of `ot[0]`, so font + our F4 don't conflict

### Blocker 6: `ps1StoryDbg{Phase,Seq}` may not exist post-cleanup
`pause_menu.c` references them at line 41–42. Verify in P0; if removed in cleanup, drop the corresponding fields from the Scene Info screen.

### Blocker 7: in-flight SFX on pause
SPU voices already key-on'd continue playing during pause. Mute on pause-show via new `soundMuteAll()` calling `SpuSetKey(0, 0xFFFFFF)`. ~3 lines in `sound_ps1.c`.

---

## 8. Wiring — file-by-file

### `events_ps1.c`
- Drop `static` from `pad_buff` (line 33)
- Replace busy-loop pause path (lines 92–97 + 148–160) with:
  ```c
  if (buttons & PAD_START) {
      pauseMenuShow();
      while (pauseMenuUpdate()) {
          VSync(0);
      }
      while ((~((PADTYPE*)pad_buff[0])->btn) & PAD_START) VSync(0);
  }
  ```
- Delete obsolete `quit`, `pause`, `frameAdvance`, `maxSpeed` globals + SELECT/CIRCLE/TRIANGLE handlers (PS1-only, host parity not needed)

### `utils.c` (PS1 section)
- Add `int ps1SoftHour = 12; int ps1SoftMonth = 6; int ps1SoftDay = 30;`

### `ps1_perf.h`
- Add ~16 read-only accessors (`ps1PerfGetSceneName`, `ps1PerfGetRenderedLoops`, `ps1PerfGetHeldLoops`, `ps1PerfGetAdvances`, `ps1PerfGetLateAdvances`, `ps1PerfGetMaxElapsed`, `ps1PerfGetEntries`, `ps1PerfGetMaxPayload`, `ps1PerfGetMaxHold`, `ps1PerfGetCdReads`, `ps1PerfGetCdFailures`, `ps1PerfGetCdBytes`, `ps1PerfGetMaxCdElapsed`, `ps1PerfGetRestoreBytes`, `ps1PerfGetUploadBytes`, `ps1PerfGetMaxUploadElapsed`)
- Inline in header where simple

### `pause_menu.c`
- Replace `dimBackground` pixel-modify with POLY_F4 dim + panel quads + own OT (Blocker 4)
- Add Debug Info sub-screen reading `ps1PerfGet*` accessors
- Add Toggle Perf Counters menu item (calls `ps1PerfSetEnabled`)
- Repurpose `MENU_NEXT_SCENE` → `MENU_RESET_LOOP`, `MENU_DIRECT_CONTROL` → `MENU_FREEPLAY` (grayed)
- Rename `pauseMenuRequestNextScene` → `pauseMenuRequestResetLoop`
- Add `gPauseMenuResetRequested` global
- On Show: `soundMuteAll()` + one-shot `printf("JCPAUSE …")` TTY snapshot
- Read scene name via `ps1PerfGetSceneName()` (not `ps1AdsCurrentName`)

### `foreground_pilot.c`
- Add `&& !gPauseMenuResetRequested` to ocean-runtime while-loop (one condition tweak)
- Optional: add `fgRuntimeFrameIndex/Total` accessors if not already in `gPs1Perf`

### `graphics_ps1.c`
- No new code needed — perf module already collects what we need
- Add `gFrameCount` global + per-frame increment for uptime (~3 lines)

### `sound_ps1.c`
- Add `void soundMuteAll(void)` — `SpuSetKey(0, 0xFFFFFF)`

### `jc_reborn.c`
- After `graphicsInit()`: `pauseMenuInit()`
- In screensaver `do` loop: read `pauseMenuRequestResetLoop`, clear `explicitScene`, increment `gScreensaverIterCount`

### `CMakeLists.txt`
- Add `src/pause_menu.c` to source list

---

## 9. Phase plan — commit per phase

Branch: **`pause-menu-wire`** off current `main`. Merge to `main` only after sign-off.

| Phase | Scope | Effort | Commit msg |
|:-:|---|---|---|
| **P0** | Preflight: drop `static` from `pad_buff`, define `ps1Soft*`, add `pause_menu.c` to CMakeLists, verify `ps1StoryDbg*` | 1 hr | `pause: P0 — preflight (link blockers + missing externs)` |
| **P1** | Wire `pauseMenuInit` at boot, replace pause busy-loop in `eventsWaitTick`. Verify Resume + Sound work | 1 hr | `pause: P1 — wire menu into Start input` |
| **P2** | Replace `dimBackground` pixel-modify with POLY_F4 dim + panel quads | 2 hrs | `pause: P2 — translucent dim + solid panel via POLY_F4` |
| **P3** | Add `ps1PerfGet*` accessors. Add Debug Info sub-screen consuming them. Read `ps1PerfGetSceneName` for the Scene field. | 1.5 hrs | `pause: P3 — debug info sub-screen + perf accessors` |
| **P4** | Add `Toggle Perf Counters` menu item | ½ hr | `pause: P4 — perf toggle entry` |
| **P5** | Reset Screensaver Loop entry + plumb `gPauseMenuResetRequested` | 1 hr | `pause: P5 — reset-loop entry + auto-resume` |
| **P6** | Mute SPU on pause-show, one-shot `printf("JCPAUSE …")` snapshot, polish typography | 1 hr | `pause: P6 — mute-on-pause + JCPAUSE snapshot` |
| **P7** | Cross-scene smoke test (fishing1/2/3, screensaver loop, title), regression scan, perf-on/off both cycles | ½ hr | `pause: P7 — smoke verified` |

**Total: ~8 hours**.

### Per-phase validation
- After every phase: re-run fishing1/2/3 in screensaver, confirm no regression, pause + resume each
- After P3: pause and verify all debug fields populated with real numbers (not 0/empty) when perf is enabled
- After P5: pause → Reset Loop → confirm fresh scene with re-randomized variants
- After P7: 5-min idle screensaver loop, pause every ~1 min, resume, check for memory drift

---

## 10. Risk register

| # | Risk | Severity | Mitigation |
|:-:|---|:-:|---|
| 1 | `pause_menu.c` orphan in CMake | Crit | P0 fixes |
| 2 | `ps1Soft*` undefined symbols | Crit | P0 fixes |
| 3 | `pad_buff` static, link error | Crit | P0 fixes |
| 4 | `dimBackground` broken in rect-mode | High | P2 — POLY_F4 quads |
| 5 | OT/primitive ownership during pause | Medium | P2 builds + draws own OT |
| 6 | POLY_F4 + semi-trans is new pattern | Medium | Standard PSn00bSDK API |
| 7 | `ps1StoryDbg*` may have been removed | Medium | P0 verifies; if gone, drop fields |
| 8 | Mid-scene reset latency (waits for current frame) | Low | Acceptable — 1 frame max |
| 9 | In-flight SPU SFX on pause | Low | P6: `soundMuteAll` |
| 10 | Pad debounce double-handling | Low | pause_menu owns own debounce via `prevButtons` |
| 11 | FntFlush conflicting with our OT | Low | FntFlush has own primitive path, verified independent |
| 12 | perf-plan Phase 1 may break pause if held-frame wait path doesn't poll input | Medium | Perf-plan's `P1-03` explicitly handles this; document the coupling (§12) |
| 13 | `ps1PerfSetEnabled(0)` zeros counters mid-scene | Low | Documented behavior; user-initiated toggle is fine |
| 14 | perf scene boundaries don't fire for non-screensaver paths | Low | Pause menu shows hint when scene-name is empty |

---

## 11. Real-hardware compatibility constraints

Treated as a hard requirement. Implementation rules:

1. **No DuckStation-specific shortcuts.** Only PSn00bSDK APIs.
2. **No printf in per-frame paths.** Per the perf doc: text I/O alters timing. One-shot printf on pause-Show is fine.
3. **No new VRAM math.** Existing infra handled; pause introduces no new VRAM addressing.
4. **POLY_F4 + semi-trans is a standard PSn00bSDK primitive.** Works identically on real hardware and DuckStation.
5. **Memory ceiling 2 MB strict.** Pause adds no significant allocation; bg state untouched.
6. **Controller protocol.** PSn00bSDK pad polling works identically on real hardware.
7. **SPU mute-all** uses standard `SpuSetKey(0, 0xFFFFFF)` API.

---

## 12. Forward compatibility — perf-plan Phase 1

The performance plan's Phase 1 (`P1-01..06` in `docs/ps1/performance-optimization-plan.md`) introduces a "display-only wait path" for held VBlanks that skips restore/compose/upload entirely.

**Critical detail in `P1-03`**:
> Add a display-only wait path that handles `VSync` and controller polling without `LoadImage`.

When perf-plan Phase 1 lands, the scene's main loop will route held VBlanks through this new wait path instead of `grUpdateDisplay`. **The pause-trigger plumbing must continue to fire on every VBlank, not just rendered ones.**

**Coupling**: pause-menu plan ships first; perf-plan Phase 1 must respect "input poll every VBlank" or pause stops working during held frames. Perf-plan's `P1-03` already explicitly mentions controller polling — this is covered as long as the implementer of perf-Phase-1 reads this doc.

If perf-plan Phase 1 ships first instead, no risk — its wait path is added before pause needs it.

---

## 13. Open implementation notes

### `gPs1Perf` access pattern
- `gPs1Perf` is `static` in `ps1_perf.c`. Read access from `pause_menu.c` is via the new `ps1PerfGet*` accessors — never direct.
- Toggling perf OFF zeros `gPs1Perf`. Pause menu always reads-with-zero-tolerance; never assumes prior values.

### Frame counter semantics
- `gFrameCount` increments in `grUpdateDisplay`. Only fires on rendered frames (not held ones).
- After perf-Phase-1, held VBlanks won't go through `grUpdateDisplay`. `gFrameCount` will then under-count. Mitigation: when that lands, move the increment to a deeper VSync layer or add a separate held-frame counter.

### Scene-name persistence across screensaver iterations
- `ps1PerfBeginScene` copies into `gPs1Perf.sceneName` AT iteration start.
- `ps1PerfEndScene` does NOT clear the name. So between iterations (between EndScene and next BeginScene) the name remains valid.
- Pause menu can read `ps1PerfGetSceneName()` at any time and get the most-recent scene name, even if the scene "ended" milliseconds ago.

### Reset semantics
- `gPauseMenuResetRequested` is a one-shot flag. Set by menu, consumed by jc_reborn loop, cleared after consumption.
- Mid-scene: pause-menu sets flag, returns 0 from `pauseMenuUpdate`, eventsWaitTick exits, scene's while-loop sees flag, exits cleanly. fgBackdropRelease(1) cleans up. Main loop's next iteration consumes the flag and re-randomizes.

### Memory pressure during pause
- POLY_F4 quads: ~32 bytes each in primitive buffer. Negligible.
- `FntFlush`: uses its own buffer.
- No allocations during pause.
- No memory pressure expected.

---

## 14. Final greenlight checklist

**Confirmed by user (2026-04-25)**:

- [x] Solid panel + dimmed background (via POLY_F4 quads)
- [x] Reset = fresh random pick from `kProvenScenes`
- [x] Sound persistence: session-only
- [x] Quit option: removed
- [x] Build-info on Debug screen only
- [x] Host parity: out of scope
- [x] Debug refresh: per-frame
- [x] Reset auto-resumes
- [x] TTY `JCPAUSE` snapshot on pause-Show: yes
- [x] Debug Info: single dense page
- [x] Toggle Perf Counters: separate menu item
- [x] Branch: `pause-menu-wire`, commit per phase

Implementation ready to start when greenlit.

---

## Changelog

- **2026-04-25**: Initial design locked at v4 — preflight phase for link blockers, POLY_F4 visual approach replacing the broken pixel-modify dim, Debug Info screen wired to `gPs1Perf` via new accessors (free thanks to existing perf instrumentation), forward-compat note for perf-plan Phase 1's held-frame wait path.
