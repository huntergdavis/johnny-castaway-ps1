# Castaway Freeplay — Direct-Control Johnny Mode

**Status**: design locked, implementation ready to begin
**Slug**: `freeplay` (boot via `fgpilot freeplay`; later via pause menu)
**Audience**: PS1 hardware target (DuckStation for primary testing, real PS1 hardware compatibility is a hard requirement)
**Implementation branch**: `freeplay-mode` (to be created at start of work)

---

## 1. Vision

A perpetual Saturday-morning Johnny Castaway: the player drives Johnny around the island and the world responds. Walking, fishing, building, summoning visitors, surviving cosmic moments. Every press reveals something — sometimes a gag you've seen, sometimes one you haven't. The sandcastle you built two minutes ago is still there. The fire you lit is still flickering. Friday remembers your previous visits. King Kong might appear on the horizon if you ask politely.

Goal: someone watching a friend play it should say "wait, I want to try" within thirty seconds.

---

## 2. Scope & non-goals

### In scope
- 4-way directional walking (D-pad / analog stick)
- Single-button context verb (Cross) that does the right thing per location
- 8 self-expression gags (Square cycle)
- 10 summonable visitors / events (Triangle cycle)
- Persistent island state during a session: sandcastles, fire, raft progress, coconut pile, holiday overlay, day/night, Friday-friendship counter
- Ambient autonomous life: cloud drift, occasional gulls, random coconut fall, auto-thirst, auto-fish-bite, auto-sleep
- Konami code + several other hidden combos
- Brief help overlay (Select)
- Family-friendly polish throughout
- Real PS1 hardware compatibility

### Non-goals
- Capture, export, or recording — freeplay is play-only
- Diagonal movement (4-way is forever)
- Save / persistence across sessions (state resets when scene exits)
- NPC dialog or quest system
- Score / progression
- NSFW romance — Mary romance is hearts-above-heads only

---

## 3. Architectural decision

Every other fgpilot scene (`fishing1`, `fishing2`, `fishing3`, etc.) plays back a captured `.FG2` pack of pre-rendered foreground frames. **Freeplay cannot work that way** — there's no canonical frame stream because gameplay branches with input.

So freeplay is a **runtime-driven scene**: it lives entirely in C code on the PS1, with the same surface as the ocean runtime (background SCR + wave tick + clean-rect restore + present), but the per-frame foreground sprite is computed live from a state machine, not streamed.

Key implications:
- No `.FG2` pack for freeplay
- No host-side capture step
- No CD asset additions (every BMP is already on disc)
- Reuses the post-cleanup PS1 backdrop API now living in `foreground_pilot.c`:
  - `fgBackdropPreloadBackgrndBmp()`
  - `fgBackdropEnableWaveBackdrop()`
  - `fgBackdropSaveCleanBgRectsForPack()`
  - `fgBackdropTickBackgroundWaves()`
  - `fgBackdropStampHoliday()`
  - `fgBackdropRelease(int keepBackgrnd)`
  - `grSaveCleanBgRects()`, `grRestoreBgFromRects()`

---

## 4. Control surface

PS1 has D-pad + 4 face buttons + L1 / L2 / R1 / R2 + Select + Start. Both pads are polled; either can drive freeplay.

### Tier A — always active

| Control | Action |
|---|---|
| D-pad / left analog | Walk 4-way. Analog has ~32-unit dead-zone. |
| Cross (✕) | **Context verb** — does the right thing for Johnny's current zone |
| Start (tap) | Exit freeplay → return to screensaver random rotation |
| Select | Toggle help overlay (3-sec auto-fade or until pressed again) |

### Tier B — self-expression (no modifier)

| Control | Action |
|---|---|
| Square (□) | Cycle through 8 self-gags (see §5) |
| Circle (○) | Re-fire the gag Square just did |

### Tier C — outside world (no modifier)

| Control | Action |
|---|---|
| Triangle (△) | Cycle through 10 summons (see §6) |

### Tier D — modifier combos

| Control | Action |
|---|---|
| L1 + Square | Direct: bonk-head |
| L1 + Triangle | Direct: summon Mary |
| L1 + Circle | Direct: summon King Kong |
| L1 + Cross | Direct: knock coconut on Johnny's head |
| R1 + ↑ | Toggle day / night |
| R1 + ↓ | Toggle low-tide / high-tide |
| R1 + → | Cycle holiday overlay (none → Halloween → St. Patrick → Christmas → New Year) |
| R1 + ← | Cycle raft-build progress (0 → 1 → 2 → 3 → 4 → 5 → 0) |
| R1 + Cross | Light / extinguish fire at Johnny's position |
| R1 + Square | Drop coconut |
| R1 + Triangle | Spawn seagull flock |
| R1 + Circle | Spawn cloud drift |
| L1 + R1 | Combo prefix for easter eggs (see §10) |

### Tier E — held / continuous

| Control | Action |
|---|---|
| L2 (held) | Sprint: walk speed 2 px / frame |
| R2 (held) | Tiptoe: walk speed 0.5 px / frame |

### Tier F — hidden (Konami code & friends)

See §10.

---

## 5. Self-gag catalog (Square cycle)

Order is deliberately mundane → escalating, so first impressions feel chill and discovery rewards repetition. Each gag is one-shot ~60–90 vblanks unless noted.

| Order | Gag | Sprite source | Vibe | Duration |
|:-:|---|---|---|---|
| 1 | **Eat** | `GJFFFOOD.BMP` | domestic | 90 vb |
| 2 | **Wipe brow / hot** | `GJHOT.BMP` | domestic | 90 vb |
| 3 | **Idea (lightbulb)** | `LITEBULB.BMP` overlaid on idle Johnny | first "aha!" | 120 vb |
| 4 | **Angry** | `GJANGRY.BMP` | first emote | 90 vb |
| 5 | **Bonk head** | `JOHNWALK.BMP` (specific frame) or dedicated | slapstick | 75 vb |
| 6 | **Drunk toggle** | `DRUNKJON.BMP` | silly state | sticky toggle |
| 7 | **Snazzy strut** | `MEXCWALK.BMP` (sprite filename retained from original Sierra assets; gag name decoupled from any nationality) | charm | 5 sec auto-revert |
| 8 | **Run away** | `GJRUNAWA.BMP` | action peak | 3-sec dash |

Cycle wraps. Re-pressing during lockout queues the next gag. Drunk is a flag, not a mode — it modifies all subsequent walking.

**Circle = re-fire whatever Square just did.**

---

## 6. Context-verb (Cross) zones

Cross is the discovery loop. The handler reads Johnny's current position and picks the appropriate behavior:

| Zone | x range | y range | Behavior |
|---|---|---|---|
| Left shore (fishing) | 240–290 | 280–305 | Toggle fishing pose (cast → hold) |
| Right shore (fishing) | 510–540 | 280–305 | Toggle fishing pose |
| Shallow water | 240–540 | 305–330 | Dive (one-shot ~120 vb, brief LILFISH school appears) |
| Beach center (build) | 310–470 | 240–290 | Build sandcastle stage; multi-press cycles `GJCASTLE.BMP` frames; on 5th press a permanent castle stamps and stage resets |
| Under palm tree | 430–480 | 220–250 | Knock coconut down — coconut falls, lands on Johnny's head (`COCOHEAD.BMP` lockout ~90 vb), coconut sprite drops to ground |
| Near lit fire | within 60 px of fire | any | Toss food in fire (food sprite appears in fire briefly) |
| Default (anywhere else) | — | — | Look around / scout horizon (hand to forehead, ~90 vb) |

---

## 7. Summon catalog (Triangle cycle)

10 summons. Each occupies a single summon slot at a time. Pressing Triangle during an active summon either queues or interrupts (tuneable).

| # | Summon | Sprites | Behavior | Duration |
|:-:|---|---|---|---|
| 1 | Seagull (single) | `GJGULL1.BMP`, `GJGULL1A.BMP` | Figure-8 across upper sky. SFX: distant cry. | 6 sec |
| 2 | Liliput parade | `LILIPUTS.BMP` | 5 tiny natives cross beach in single file. SFX: pitter-patter. | 8 sec |
| 3 | Biplane fly-by | `GJBIPLAN.BMP` | Plane banks across sky right→left, banner reads "JOHNNY". SFX: prop drone. | 6 sec |
| 4 | Native canoe (Friday) | `GJNAT1.BMP`, `GJNAT1LI.BMP` | Canoe paddles past. After friendship counter ≥ 1 he lands. After ≥ 3, full greeting between him and Johnny. | 10 sec |
| 5 | Visitor boat | `BOAT.BMP`, `GJVIS3/5/52.BMP` | Boat lands at right shore, visitor disembarks, gestures, leaves. SFX: boat whistle. | 12 sec |
| 6 | King Kong | `GJKINGKO.BMP` | Massive sprite on top-left horizon. Beats chest, roars. Auto-triggers Johnny's `GJRUNAWA` reaction. SFX: roar. | 8 sec |
| 7 | Mary the mermaid | `MJ_AMB.BMP`, `MJBATH.BMP` | Surfaces at left shore, peeks out, beckons. If Johnny walks within 50 px during her time, hearts appear above both heads (~2 sec). SFX: arpeggio. | 10 sec |
| 8 | Pirate cameo | `FISHMAN.BMP` (palette swap) or visitor stand-in | Walks across upper beach, plants flag, leaves. SFX: yarrr. | 8 sec |
| 9 | Seagull flock | `GJGULL1/2/3.BMP` + alts | 3 birds on independent figure-8s, swooping. SFX: gull chorus. | 10 sec |
| 10 | "Meanwhile…" panel | `MEANWHIL.BMP` | Full-screen comic panel flashes (~600 ms), then dismisses. Pure comedy beat. | 1.5 sec |

L1 + button shortcuts skip directly to Mary / King Kong / coconut / etc. without disturbing the cycle pointer.

---

## 8. Persistent island state

The "live life" layer. Once placed, things stay until the player exits freeplay. State resets on each fresh entry into freeplay.

| Object | Trigger | Limit | Restore handling |
|---|---|---|---|
| Sandcastle | Cross on beach center, 5th press finalizes | Up to 3 simultaneous | Stamped into clean baseline at place time. Sub-rect re-snapshot. |
| Lit fire | R1 + Cross | One at a time (re-press relocates) | Animated `FIRE1-5.BMP` cycle, redrawn per frame, covered by clean rect (NOT stamped permanently) |
| Raft stage | R1 + ← cycles 0–5 | Always at canonical raft position | Stamped at canonical position when stage changes. Re-snapshot on change. |
| Coconut on ground | knocked from tree | Up to 5 piled | Stamped at impact position. Pile of 5 → 6th triggers explosion easter egg. |
| Day / night | R1 + ↑ | Either | Affects palette of all rendering. Re-snapshot full clean baseline on toggle. |
| Holiday overlay | R1 + → | One of 5 (none + 4) | Loads `HOLIDAY.BMP` on first toggle to "on"; stamped per-frame via `fgBackdropStampHoliday` |
| Friday-friendship counter | Auto-incremented on each Native Canoe summon | 0–3 sticky during session | Influences Native Canoe summon behavior |

### Re-snapshot pattern (key implementation detail)

The freeplay scene owns its own array of clean rects (`gFreeplayOwnedRects[]`, up to 8 entries). When persistent state changes, freeplay rebuilds the list and calls `grSaveCleanBgRects(...)` with the full set. The bg tiles must already have the new state painted before the snapshot. This is the same pattern used by ocean-runtime for the wave + Johnny rect.

### Day / night re-stamping

When R1+↑ toggles day/night, the bg palette flips but stamped persistents (sandcastles, raft, coconuts) were captured at the old palette. Solution: on toggle, re-stamp ALL persistents into the new clean baseline, then re-snapshot.

---

## 9. Ambient life (no-input events)

The island has a heartbeat. These tick continuously regardless of input:

| Event | Cadence | What |
|---|---|---|
| Cloud drift | every 25–45 sec (random) | A `CLOUDS.BMP` cloud crosses sky |
| Solo seagull | every 30–90 sec | One bird crosses (silent, low-key) |
| Random coconut fall | every 60–180 sec, low chance | Coconut drops from tree even with no input. Lands on ground or on Johnny if he's under it. |
| Auto-thirst | after 90 sec of accumulated walking | Johnny auto-fires the "hot" gag once. Resets timer. |
| Auto-fish-bite | after 30 sec of fishing | Auto-triggers GJCATCH1 → 2 → 3 sequence with kingfish appearance. Big payoff! |
| Auto-sleep | 60 sec idle | Johnny yawns, sits down, "Z" sprite appears above. Wakes on any input. |

**Rule**: auto-events only fire while Johnny is in `IDLE` mode (or `WALK`, for thirst). They never interrupt cinematics, cycle gags, or summons.

---

## 10. Konami code & secrets

### The Konami sequence

**↑ ↑ ↓ ↓ ← → ← → Square Cross** within 4 seconds.

**Reward: "Castaway Cove Carnival"**
- All summonable NPCs spawn simultaneously: Mary at left shore, Friday's canoe arrives, Liliputs parade, biplane flies overhead with banner reading "PARTY!", seagull flock circles, King Kong roars from horizon
- Brief "MEANWHILE…" panel flash, then on-screen banner reads "CASTAWAY COVE CARNIVAL"
- Lasts ~12 seconds, **input locked** (cinematic)
- Layered SFX: boat whistle + roar + gulls + plane drone (cap simultaneous voices at 8)
- After: everyone exits in their normal patterns. Johnny does one celebratory snazzy-strut for 3 sec
- Achievement-style flash: "★ SECRET FOUND ★"

### Other secrets

| Secret | Trigger | Reward | Cinematic? |
|---|---|---|---|
| The Big One | Cross 7 times within 2 sec at fishing shore | Auto-cinematic GJCATCH1→2→3 with kingfish, fanfare SFX | Yes (~5 sec) |
| Sleep walker | Any button press during yawn animation frame 0 | Johnny sleepwalks figure-8 for 8 sec, palette dimmer | Yes (~8 sec) |
| Coconut stack-pocalypse | Knock 7th coconut after 6 piled | Tree shakes (1-px global jitter 2 frames), coconuts roll into a tower (cosmetic stamp), avalanche SFX. Tower stays for session. | No |
| Friday's friend | Native Canoe summoned 3 times in a session | On 3rd, Friday lands and walks alongside Johnny mirroring his actions for 30 sec | No |
| The Tornado | Hold L1+R1+L2+R2 for 2 sec | All summons fire in rapid succession, 1 sec each | Yes (~10 sec) |
| Holiday speedrun | Cycle through all holiday overlays in <2 sec via R1+-> spam | Brief montage flash with the overlays superimposed | No |
| Strut forever | Press Square exactly when Snazzy strut's timer expires (frame-perfect) | Strut extends 5 more sec, repeatable | No |
| Drunken master | Drunk-toggled ON, then bonk head 5 times | Johnny falls over backward, lays on beach with X-eyes for 5 sec, recovery animation | Yes (~6 sec) |

---

## 11. Help overlay (Select)

Brief, translucent panel, top-right corner, ~300×120 px. Auto-fades after 3 sec or until Select pressed again.

```
┌───────────────────────────────┐
│  D-pad   walk                 │
│  ✕       do thing here        │
│  □       gag      ○ replay    │
│  △       summon               │
│  L1+/R1+ secrets…             │
│  Start   exit                 │
└───────────────────────────────┘
```

**Implementation**: PSn00bSDK has `FntPrint`/`FntFlush` for BIOS font. Reuse `pause_menu.c`'s font infrastructure if compatible. Panel background is a semi-transparent black quad rendered as a `POLY_FT4` primitive.

---

## 12. Sound effects mapping

Tentative mapping (refined after Phase 0.2 SFX inventory):

| Action | Sample ID | Notes |
|---|---|---|
| Splash (cast / dive / coconut into water) | 4 | known from fishing |
| Reel-in / catch | 16 | known from fishing |
| Bonk | 5 | known from fishing |
| Chomp / eat | 8 | best guess |
| Idea ding | tbd | system beep if no dedicated sample |
| Drunk hiccup | tbd | |
| Footstep | none (silent) | original was silent |
| Coconut thunk | tbd | reuse bonk if no dedicated |
| Fire whoosh | tbd | |
| Castle build poof | tbd | |
| Mary arpeggio | tbd | skip if missing |
| King Kong roar | tbd | stretched bass sample if available |
| Biplane drone | tbd | |
| Seagull cry | tbd | |
| Trumpet fanfare (Big One) | tbd | |

If samples are unavailable, those actions stay silent — nothing breaks.

**SPU constraint**: cap simultaneous voices at 8 (PS1 SPU has 24 total but reserve headroom). Konami Carnival's layered SFX queues if it would overflow.

---

## 13. State machine — Johnny modes

Modes are mutually exclusive. One at a time.

```
IDLE  → WALK (input dx/dy nonzero)
IDLE/WALK → FISH      (Cross at shore zone)
IDLE/WALK → DIVE      (Cross in shallow water)
IDLE/WALK → BUILD     (Cross on beach center, multi-press)
IDLE/WALK → BONK | EAT | ANGRY | HOT | IDEA | STRUT | RUNAWAY | SCOUT
                       (Square cycle / Cross default zone)
FISH  → IDLE/WALK    (Cross again, or after auto-bite ends)
DIVE  → IDLE          (after duration)
BUILD → IDLE          (after stage finalize or movement)
*ONESHOT → IDLE       (after lockout timer expires)
SCOUT → IDLE          (after sweep done)
SLEEP → IDLE          (any input)
```

**Special states**:
- **DRUNK** is a flag, not a mode — modifies WALK rendering using `DRUNKJON.BMP`
- **CINEMATIC LOCK** — set by `cinematicLockUntilFrame`. While set, input layer reads pad as usual but `freeplayApplyInput` returns early; D-pad and all buttons including Start are ignored. Auto-clears when `gFrameCount >= cinematicLockUntilFrame`. Used by Big One, Konami Carnival, Sleep-walker, Drunken master, Tornado.

Distinction: **summons do NOT lock input** — Johnny remains controllable while seagulls / Mary / KingKong play out. **Cinematics DO lock input** — the player is meant to watch.

---

## 14. Walkable + interactive geometry

```
y=0  ┌────────────────────────────────────────────┐
     │                       SKY                  │
     │   biplane corridor: y=60–80                │
     │   cloud corridor:   y=80–140               │
     │   gull corridor:    y=100–180              │
     │   King Kong appears: x=80–180, y=80–200    │
y=200├─────────────  HORIZON  ───────────────────┤
     │  (visitor boat lands x=560, y=270)         │
     │      WALKABLE  (240..540, 220..305)        │
     │     ┌─tree z-region (430..480, 220..250)─┐ │
     │     │  (knock coconut here)              │ │
     │     │  CASTLE buildable  (310..470,      │ │
     │     │                     240..290)      │ │
     │     │  fire pit anywhere walkable        │ │
     │     └────────────────────────────────────┘ │
y=305├──── WATERLINE ─────────────────────────────┤
     │  L-shore fish (240..290, 280..305)         │
     │  R-shore fish (510..540, 280..305)         │
     │  shallow      (240..540, 305..330)         │
     │      LILIPUT parade y=295                  │
     │      Mary spawn  (260, 320)                │
     │      Friday canoe corridor y=325           │
y=480└────────────────────────────────────────────┘
```

These are estimates — Phase 1 implementation refines empirically.

---

## 15. Render pipeline order (locked)

Each frame in this order. Do not reorder without good reason.

1. `grBeginFrame()`
2. `grRestoreBgFromRects()` — restores baseline (already includes painted persistent objects)
3. `fgBackdropTickBackgroundWaves()` — wave overlay
4. **Animated persistent objects** (fire flicker, in-progress castle build) — these are NOT in baseline because they animate
5. **Summons** in spawn-time order (back-to-front roughly)
6. **Johnny** — last among foreground, so he overlaps summons that are spatially behind him; in tree z-region (Phase 5) render Johnny then re-stamp trunk
7. **Holiday overlay** if active (`fgBackdropStampHoliday`)
8. **Help overlay** if Select active (top-most)
9. **Achievement / SECRET FOUND text** if active (top-most, alpha)
10. `grUpdateDisplay(NULL, NULL, NULL)`

---

## 16. Memory budget

| Item | Size | Notes |
|---|---|---|
| `BACKGRND.BMP` slot 0 | ~93 KB | Sticky across screensaver loops |
| `HOLIDAY.BMP` variant slot | ~30 KB | Lazy-load on first holiday-toggle |
| `JOHNWALK.BMP` in freeplay slot 1 | ~100 KB | Always loaded |
| Currently-active gag BMP, freeplay slot 2 | ≤ 50 KB | LRU; one at a time |
| Currently-active summon BMP, freeplay slot 3 | ≤ 80 KB | LRU; one at a time. KingKong is largest. |
| Persistent-state BMPs (FIRE, COCONUTS, MRAFT, GJCASTLE) freeplay slot 4 | ≤ 60 KB | Concat where possible, swap as needed |
| `LITEBULB` stamped into Johnny render path | ≤ 5 KB | tiny |
| bg tiles (4) | 614 KB | |
| Clean rects | ~280 KB | Slightly larger than fishing3 due to summon corridors |
| Freeplay state struct + arrays | < 4 KB | |
| **Total** | **~1.3 MB** | comfortably under 2 MB ceiling |

If memory pressure surfaces: drop a couple of summons from the catalog, or aggressively LRU more slots.

---

## 17. File-level changes

### New files

**`src/scene_freeplay.h`** (~80 lines)
```c
#ifndef SCENE_FREEPLAY_H
#define SCENE_FREEPLAY_H
void freeplayRun(void);
int  freeplayExitRequested(void);
void freeplayClearExitRequest(void);
#endif
```

**`src/scene_freeplay.c`** (~1000–1200 lines, structured)
- §0 — Header / license
- §1 — Includes, constants, sprite-index references (from `freeplay_sprite_indices.h`)
- §2 — Type definitions:
  - `enum TFreeplayJohnnyMode` (IDLE, WALK, FISH, DIVE, BUILD, BONK, EAT, ANGRY, HOT, IDEA, STRUT, RUNAWAY, SCOUT, SLEEP)
  - `enum TFreeplaySummonKind` (SEAGULL, LILIPUTS, BIPLANE, CANOE, BOAT, KINGKONG, MARY, PIRATE, FLOCK, MEANWHILE)
  - `struct TFreeplayJohnny` { x, y, facing, mode, modeTimer, frameTimer, frameIdx, drunkFlag, walkStepCount, idleSec }
  - `struct TFreeplaySummon` { kind, x, y, frameIdx, frameTimer, lifetimeFramesRemaining, customData }
  - `struct TFreeplayPersistentObject` { kind, x, y, frameIdx, stage }
  - `struct TFreeplayState` { johnny, summons[8], persistents[16], dayNight, lowTide, holiday, raftStage, fridayCount, secretsFound bitset, comboBuffer, helpOverlayUntilFrame, ambientNextEventFrame, cinematicLockUntilFrame, gFreeplayExitRequested }
  - `struct TFreeplayInput` { dx, dy, pressedThisFrame bitmask, heldBitmask, heldFrames per button }
- §3 — Sprite registry (table + lazy-load helpers)
- §4 — Input layer (`freeplayReadInput`, `freeplayDetectKonami`, combo detection)
- §5 — Action handlers (one function per action, ~30 of them)
- §6 — Mode tick (frame stepping per mode)
- §7 — Summon tick + persistent-object tick
- §8 — Ambient life ticker
- §9 — Render layer (Johnny, summons, persistents, overlays)
- §10 — Help overlay (uses pause_menu fnt utilities if compatible, else minimal `FntPrint`)
- §11 — Top-level `freeplayRun()` (mirrors `fgPlayOceanRuntimeScene` setup, runs main loop, tears down)
- §12 — Exported API

**`src/freeplay_sprite_indices.h`** (~50 lines, generated from Phase 0.1 audit)
Constants: `FREEPLAY_JOHNWALK_IDLE=8`, `FREEPLAY_JOHNWALK_WALK_RIGHT_BASE=0`, etc.

### Edits

**`src/foreground_pilot.c`** — add freeplay dispatch (~5 lines)
Top of `foregroundPilotPlay()`, alongside the existing dispatch chain (`testcard`, `fgCompactOverlayPackPathForScene`, `titlecopy`, `isletest`, `oceantest`, `solidred`):
```c
if (fgSceneEquals(gForegroundPilotScene, "freeplay")) {
    freeplayRun();
    return;
}
```
Plus `#include "scene_freeplay.h"` near other includes.

**`src/jc_reborn.c`** — exit fall-back (~5 lines, both PS1 and host branches)
Inside the screensaver `do { foregroundPilotPlay(); … }`:
```c
if (freeplayExitRequested()) {
    freeplayClearExitRequest();
    explicitScene = NULL;  /* fall back to random kProvenScenes rotation */
}
```
**Do NOT** add `"freeplay"` to `kProvenScenes`.

**`CMakeLists.txt`** — add `src/scene_freeplay.c` to source list.

**`scripts/build-host.sh`** — add `src/scene_freeplay.c` to `SOURCES=()`.

### No changes
- `config/ps1/cd_layout.xml` — every BMP we need is already on disc (verified after the recent CD-layout cleanup commit `7d5221e3`)
- `config/ps1/regtest-scenes.txt` — freeplay isn't regtest-able
- `scripts/*.py` — no capture pipeline involvement

### Optional
- `docs/ps1/scene-status.md` — add a "Modes" section listing freeplay separately from numbered scenes

---

## 18. Phase plan

```
Phase 0   1 day    Sprite + SFX audit, pad-init verify
Phase 1a  ½ day    Skeleton: dispatch, walk, exit
Phase 1b  ½ day    First verb + first gag + help overlay
Phase 2   1.5 days Full self-gag cycle + 3 simple summons
Phase 3   2 days   Full summon catalog + env toggles + ambient life
Phase 4   2 days   Persistent state + Friday + auto-reactions
Phase 5   2 days   Konami + easter eggs + SFX + tree z-order + polish
Phase 6   ½ day    Pause-menu integration (ship last)
─────────────────
Total    ~10 days
```

### Branch and commit strategy

- All work on a new branch **`freeplay-mode`**, branched from current `main`
- One commit per phase (and additional sub-milestone commits within longer phases when there's a coherent atomic unit) so any phase can be rolled back independently while later phases carry their own fixes forward
- Branch is merged to `main` only after all phases are validated and the user signs off
- Commits target meaningful checkpoints, not arbitrary save points — each commit must compile + boot cleanly to a `freeplay`-bootable state (or, for Phase 0, a research-only doc commit)
- The agent runs all phases overnight if greenlit; commits are pushed to the branch as each phase completes

### Phase 0 — Prerequisites (no code, single commit)

**0.1 Sprite-index audit**
Dump every relevant BMP as PNG strips. Output: `docs/ps1/freeplay/sprite-indices.md` with index → pose mapping per BMP. Becomes source-of-truth for `src/freeplay_sprite_indices.h`.

**0.2 SFX inventory**
Audit sample IDs in `soundPlay()`. Throwaway test scene plays each ID with on-screen label. Output: `docs/ps1/freeplay/sfx-inventory.md` mapping ID → human description.

**0.3 Pad-init verification**
Grep for `InitPAD` / `StartPAD` / `PadStart`. If absent, freeplay bootstraps it. If present, freeplay just polls. Decision documented at the top of `scene_freeplay.c`.

Commit: `freeplay: phase 0 — sprite/SFX/pad-init audit`

### Phase 1a — Skeleton (½ day, single commit)

- New files `scene_freeplay.c/h` with stubs
- `foreground_pilot.c` dispatch wired
- `jc_reborn.c` exit-fall-back wired (predicate returns false in MVP)
- Walking with idle pose, walkable-rect clamping
- Start exits cleanly back to screensaver
- CMake + build-host.sh updated

**Acceptance**: `fgpilot freeplay` boots, Johnny appears at center-island, D-pad walks him, walk animation cycles, Start returns to fishing scenes cleanly.

Commit: `freeplay: phase 1a — skeleton (walk + exit)`

### Phase 1b — First verb + first gag + help (½ day, single commit)

- Cross at shore zones toggles fishing pose
- Square fires bonk-head one-shot with lockout
- Select toggles a single-page help overlay

**Acceptance**: walking + fish-at-shore + bonk + help overlay. Re-entering freeplay via boot resets state cleanly.

Commit: `freeplay: phase 1b — first verb (fish) + first gag (bonk) + help`

### Phase 2 — Gag rotation + simple summons (1.5 days, 1 commit)

- All 8 self-gags fire on Square-cycle in mundane→escalate order
- Circle re-fires last gag
- 3 simple summons: seagull (single), Liliput parade, biplane fly-by

**Acceptance**: each gag visually correct, summons spawn and complete cleanly without affecting Johnny.

Commit: `freeplay: phase 2 — gag rotation + simple summons`

### Phase 3 — Full summons + env toggles + ambient (2 days, 1–2 commits)

- All 10 summons via Triangle cycle
- L1+button shortcuts to specific summons
- R1+D-pad environment toggles (day/night, tide, holiday, raft-stage)
- Ambient: cloud drift, occasional gulls, auto-thirst
- Help overlay full version

**Acceptance**: full repertoire usable, day/night re-snapshots cleanly without artifacts.

Commit: `freeplay: phase 3 — full summons + env toggles + ambient`

### Phase 4 — Persistent state + Friday + auto-reactions (2 days, 1–2 commits)

- Sandcastle placement on beach center, multi-castle support, sub-rect re-snapshot
- Lit fire (R1+Cross), animated FIRE1-5
- Raft progress sticky
- Coconut pile state, occasional auto-fall easter egg
- Friday-friendship counter triggers escalating native interactions
- Auto-fish-bite, auto-sleep auto-events

**Acceptance**: build 3 castles, walk away, walk back — they're still there. Light fire, etc. Friday escalates correctly across 3 canoe summons.

Commit: `freeplay: phase 4 — persistent state + auto-events + Friday`

### Phase 5 — Konami + easter eggs + SFX + tree z-order + polish (2 days, 1–2 commits)

- Konami code → Carnival cinematic
- "The Big One" combo
- Other secrets (sleep walker, coconut stack-pocalypse, Tornado, etc.)
- Tree z-order: Johnny renders behind palm in tree zone
- All planned SFX wired
- Optional: subtle ambient (occasional cloud, distant seagull)

**Acceptance**: Konami triggers Carnival, "Big One" triggers cinematic, tree z-order works, SFX play on cue. Loop forever (10+ min) without leak / crash / freeze.

Commit: `freeplay: phase 5 — secrets + SFX + polish`

### Phase 6 — Pause-menu integration (½ day, ship last, 1 commit)

- Adds "Freeplay Mode" entry to `pause_menu.c`
- Selecting it sets `ps1BootForegroundOverlayScene = "freeplay"` (or equivalent), unpauses
- Current scene's main loop checks `sceneSwitchRequested()` predicate (already plumbed in 1a, returns true now)

**Acceptance**: in fishing1 scene, pause → "Freeplay Mode" → enters freeplay cleanly. Exit freeplay returns to screensaver.

Commit: `freeplay: phase 6 — pause-menu integration`

---

## 19. Validation per phase

See acceptance criteria attached to each phase in §18. Cross-phase smoke tests:

- After every phase: re-run fishing1/2/3 scenes via screensaver to confirm no regression
- After every phase: 5-minute idle screensaver loop — sanity check for memory drift
- After Phase 5: 30-minute freeplay session pressing every button — fuzzy soak test

---

## 20. Risk register

| # | Risk | Severity | Mitigation |
|---|---|---|---|
| 1 | JOHNWALK frame mapping wrong | Medium | Phase 0.1 audit |
| 2 | Pad init not done elsewhere | Medium | Phase 0.3 verify |
| 3 | SFX samples don't match what we need | Low | Phase 0.2 inventory; silent fallback |
| 4 | Persistent objects + clean rect snapshot ordering | Medium | Freeplay-owned rect list, careful re-snapshot pattern |
| 5 | Day/night toggle + persistent objects | Medium | Re-stamp ALL persistents on toggle, then re-snapshot |
| 6 | Multi-summon overlap z-order | Low | Spawn-time ordering; document |
| 7 | Tree z-order requires re-stamp | Low | Phase 5 only; defer |
| 8 | Konami detection edge cases | Low | Standard ring buffer + timeout |
| 9 | Help overlay rendering needs pause_menu infra | Low | Reuse pause_menu fnt utilities |
| 10 | Pause menu / Start collision | Low | Verify; switch to Hold-Start if needed |
| 11 | Auto-events fire while user is doing something | Low | Auto-events only fire from IDLE |
| 12 | Combo input timing windows feel wrong | Low | Tunable constants, iterate |
| 13 | KingKong sprite size memory | Low | Lazy-load only when summoned |
| 14 | Mary romance sprite framing | Low | Use specific known-good index |
| 15 | "Big One" cinematic blocking input | Low | Yes intentionally — it's a cinematic |
| 16 | Coconut explosion easter egg complexity | Low | Defer to Phase 5 polish |
| 17 | Real-hardware SPU voice limit | Low | Cap simultaneous voices at 8 |
| 18 | Pause-menu Start collision (Phase 6) | Low | Resolve with Phase 6 design pass |

---

## 21. Real-hardware compatibility constraints

Treated as a hard requirement. Implementation rules:

1. **No DuckStation-specific shortcuts.** Only PSn00bSDK APIs. No emulator-only behavior.
2. **VRAM addressing strict.** Existing infra handled; freeplay introduces no new VRAM math.
3. **CD timing tolerance.** Freeplay doesn't stream from CD per-frame (no .FG2), so it's safer than fishing scenes here. BMPs load once at scene start.
4. **No interrupt blocking.** Long animations advance frame-by-frame, never block main loop.
5. **Memory ceiling 2 MB strict.** Budget is 1.3 MB; comfortable.
6. **Controller protocol.** PSn00bSDK pad polling works identically on real hardware. Use only standard digital + analog reads.
7. **SPU voice limit (24).** Cap simultaneous SFX at 8; queue overflow.

---

## 22. Pause-menu integration (Phase 6)

Freeplay is intended to be summonable from the pause menu in normal screensaver scenes.

**Flow** (during normal scenes like fishing1):
1. User presses pause button (likely Start — verify in Phase 0.3)
2. Pause menu shows; user selects "Freeplay Mode"
3. Pause menu sets a scene-switch flag (e.g., `gPauseRequestedSceneSwitch = "freeplay"`), unpauses
4. Current scene's main loop checks `sceneSwitchRequested()` — exits cleanly
5. Screensaver outer loop calls `foregroundPilotPlay()` again, dispatches to freeplay

**Freeplay-internal exit**:
1. Inside freeplay, Start (tap) exits to screensaver random rotation
2. Long-press Start (>1 sec) ALSO opens pause menu inside freeplay (Phase 6 stretch)

**Conflict resolution**:
- Normal scenes: Start = pause menu (already wired)
- Freeplay: Start (tap) = exit; Start (hold) = pause menu (Phase 6)
- Pause menu's "Freeplay Mode" entry available from any scene

**Implementation impact on earlier phases**:
- Phase 1a's main loop should already check `sceneSwitchRequested()` — returns false during Phases 1–5, returns true once Phase 6 wires it
- Phase 6 itself adds ~30 lines to `pause_menu.c`
- No structural changes to `scene_freeplay.c`

---

## 23. Open implementation notes

### Naming
- Slug: `freeplay`. Final.
- BMP slot constants: define `FREEPLAY_BMP_SLOT_JOHNWALK=1`, `_GAG=2`, `_SUMMON=3`, `_PERSIST=4` to keep slot management clear.

### Random seeding
- Use existing PS1 RNG (`rand()` seeded from VBlank counter at scene start) for ambient-event timing.
- Konami input matching is exact; no randomness.

### Frame counter
- Freeplay maintains its own `gFrameCount` incremented each iteration. Used for cinematic lock, ambient cadence, animation timers, idle detection.

### Capture-ledger interaction
- Freeplay is PS1-only and play-only. The host capture pipeline is not relevant here. No capture-related code is touched by this scene.

### Fail-soft behavior
- Action handler checks BMP load result; if fail, advances to next gag in cycle and tries again. After 3 fails, fall back to bonk-head (always loaded with JOHNWALK).
- Summon load fail: silently skip and advance summon cycle pointer.

### Extensibility
- Easter-egg combo table is data-driven (struct array of `{ buttonSequence, callback, lockMs }`) so adding new secrets later doesn't require touching input code.

---

## 24. Final greenlight checklist

**Confirmed by user (2026-04-25)**:

- [x] Slug: `freeplay`
- [x] Phase split, all phases in single overnight run
- [x] Branch: new branch `freeplay-mode`, merge to main only after sign-off
- [x] One commit per phase (or per natural sub-milestone) for rollback granularity
- [x] No additional scope from MVP
- [x] Replace any nationality-tied gag name (Mexican walk → **Snazzy strut**)

Implementation ready to start. The first concrete task is Phase 0's read-only investigations, after which Phase 1a is straightforward.

---

## Changelog

- **2026-04-24**: Initial design locked at v3.1 — phase-split MVP, discovery-joy ordering, cinematic-lock semantics, pause-menu integration target, real-hardware constraints. (Then-titled `sandbox`.)
- **2026-04-25**: Renamed `sandbox` → `freeplay` throughout. Renamed "Mexican walk" → "Snazzy strut" (gag name decoupled from any nationality; sprite filename `MEXCWALK.BMP` retained as it's an asset name from the original Sierra game). API references updated to current post-cleanup names (`fgBackdropPreloadBackgrndBmp`, etc., now in `foreground_pilot.c`). Pack format references updated `.FG1` → `.FG2`. Added explicit branch `freeplay-mode` + commit-per-phase strategy. Greenlit for overnight run.
