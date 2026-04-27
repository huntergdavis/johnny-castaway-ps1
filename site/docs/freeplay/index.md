---
layout: page
title: Freeplay mode
eyebrow: Reference
subtitle: A runtime-driven scene where the player drives Johnny instead of watching him.
description: Freeplay mode in the Johnny Castaway PS1 port — a runtime-driven scene with 4-way walking, 8 self-gags, 10 summons, persistent island state, and Konami-style secrets.
---

A labor of love by Hunter Davis. *Freeplay* is the only PS1 scene where
the player drives Johnny instead of watching him. Every other scene
(`fishing1`, `fishing2`, `fishing3`, etc.) is a captured `.FG2` pack
played back as a pre-rendered foreground stream. Freeplay can't work that
way — there's no canonical frame stream because gameplay branches with
input. So freeplay lives entirely in C on the PS1, sharing the ocean-runtime
surface (background SCR, wave tick, clean-rect restore, present) but
computing the per-frame foreground sprite live from a state machine.

If you paid for this, you were cheated. Open source and free.

## Vision

A perpetual Saturday-morning Johnny Castaway: walking, fishing, building,
summoning visitors, surviving cosmic moments. The sandcastle you built
two minutes ago is still there. The fire you lit is still flickering.
Friday remembers your previous visits. King Kong might appear on the
horizon if you ask politely.

Goal: someone watching a friend play it should say "wait, I want to try"
within thirty seconds.

## Boot-token CLI

Freeplay is a foreground-pilot scene like any other, dispatched from
`foreground_pilot.c`. The boot tokens are written to
[`config/ps1/BOOTMODE.TXT`]({{ site.github_url }}/blob/main/config/ps1/BOOTMODE.TXT)
on the disc. The simplest invocation:

```text
fgpilot freeplay
```

That boots straight into freeplay at scene start. To launch it from a
live development run:

```bash
./scripts/rebuild-and-let-run.sh fgpilot freeplay
```

Variant tokens that work in other scenes also work here:

| Token                     | Effect |
|---------------------------|--------|
| `night 1`                 | Force night palette at boot. R1 + ↑ toggles it inside freeplay. |
| `lowtide 1`               | Force low-tide state at boot. R1 + ↓ toggles inside. |
| `holiday <N>`             | Pin a holiday overlay (1..36). R1 + → cycles inside. |
| `raft-stage <N>`          | Pin raft-build state (0..5). R1 + ← cycles inside. |
| `island-pos <x> <y>`      | Force a specific island position. |

Inside `foreground_pilot.c::foregroundPilotPlay()`, the dispatch is a
single equality test:

```c
if (fgSceneEquals(gForegroundPilotScene, "freeplay")) {
    freeplayRun();
    return;
}
```

`freeplayRun()` lives in `src/scene_freeplay.c`. It mirrors
`fgPlayOceanRuntimeScene`: preload `BACKGRND.BMP`, enable the wave
backdrop, save clean rects, run the main loop, tear down via
`fgBackdropRelease(1)` (keep `BACKGRND` for the next scene).

The "do not add this slug to `kProvenScenes`" rule is deliberate —
freeplay should never be picked at random by the screensaver rotation.
It's an opt-in scene only.

## Control surface

PS1 has D-pad + 4 face buttons + L1 / L2 / R1 / R2 + Select + Start.
Both pads are polled.

### Always active

| Control                  | Action |
|--------------------------|--------|
| D-pad / left analog      | 4-way walk. Analog has ~32-unit dead-zone. |
| Cross (✕)                | Context verb — does the right thing for Johnny's current zone. |
| Start (tap)              | Exit freeplay → return to screensaver random rotation. |
| Select                   | Toggle help overlay (3-sec auto-fade or until pressed again). |

### Self-expression (no modifier)

| Control      | Action |
|--------------|--------|
| Square (□)   | Cycle through 8 self-gags (see below). |
| Circle (○)   | Re-fire the gag Square just did. |

### Outside world

| Control       | Action |
|---------------|--------|
| Triangle (△)  | Cycle through 10 summons (see below). |

### Modifier combos

| Combo            | Action |
|------------------|--------|
| L1 + Square      | Bonk-head. |
| L1 + Triangle    | Summon Mary. |
| L1 + Circle      | Summon King Kong. |
| L1 + Cross       | Knock coconut on Johnny's head. |
| R1 + ↑           | Toggle day / night. |
| R1 + ↓           | Toggle low-tide / high-tide. |
| R1 + →           | Cycle holiday overlay (none → Halloween → St. Patrick → Christmas → New Year). |
| R1 + ←           | Cycle raft-build progress (0 → 1 → 2 → 3 → 4 → 5 → 0). |
| R1 + Cross       | Light / extinguish fire at Johnny's position. |
| R1 + Square      | Drop coconut. |
| R1 + Triangle    | Spawn seagull flock. |
| R1 + Circle      | Spawn cloud drift. |
| L1 + R1          | Combo prefix for easter eggs. |

### Held / continuous

| Held         | Effect |
|--------------|--------|
| L2 (held)    | Sprint: walk speed 2 px / frame. |
| R2 (held)    | Tiptoe: walk speed 0.5 px / frame. |

## Self-gag catalog (Square cycle)

Mundane → escalating, so first impressions feel chill and discovery
rewards repetition.

| # | Gag           | Sprite source                  | Vibe        | Duration |
|--:|---------------|--------------------------------|-------------|----------|
| 1 | Eat           | `GJFFFOOD.BMP`                 | domestic    | 90 vb |
| 2 | Wipe brow     | `GJHOT.BMP`                    | domestic    | 90 vb |
| 3 | Idea          | `LITEBULB.BMP` over idle Johnny | first "aha" | 120 vb |
| 4 | Angry         | `GJANGRY.BMP`                  | first emote | 90 vb |
| 5 | Bonk head     | `JOHNWALK.BMP`                 | slapstick   | 75 vb |
| 6 | Drunk toggle  | `DRUNKJON.BMP`                 | silly state | sticky |
| 7 | Snazzy strut  | `MEXCWALK.BMP`                 | charm       | 5 sec auto-revert |
| 8 | Run away      | `GJRUNAWA.BMP`                 | action peak | 3-sec dash |

Cycle wraps. Drunk is a flag, not a mode — it modifies all subsequent
walking. Circle re-fires whatever Square just did. The original sprite
filename `MEXCWALK.BMP` is retained from the Sierra assets; the gag name
was renamed from "Mexican walk" to "Snazzy strut" so the gag name is
decoupled from any nationality.

## Summon catalog (Triangle cycle)

Ten summons, one slot at a time. Pressing Triangle during an active
summon either queues or interrupts (tuneable).

| # | Summon              | Behavior                                                    | Duration |
|--:|---------------------|-------------------------------------------------------------|----------|
| 1 | Seagull (single)    | Figure-8 across upper sky. SFX: distant cry.                | 6 sec |
| 2 | Liliput parade      | 5 tiny natives cross beach in single file.                  | 8 sec |
| 3 | Biplane fly-by      | Plane banks across sky; banner reads "JOHNNY".              | 6 sec |
| 4 | Native canoe        | Friday paddles past. After friendship counter ≥ 1 he lands. After ≥ 3, full greeting. | 10 sec |
| 5 | Visitor boat        | Boat lands at right shore, visitor disembarks, leaves.      | 12 sec |
| 6 | King Kong           | Massive sprite on top-left horizon. Auto-triggers `GJRUNAWA`. | 8 sec |
| 7 | Mary the mermaid    | Surfaces at left shore; if Johnny walks within 50 px, hearts. | 10 sec |
| 8 | Pirate cameo        | Walks across upper beach, plants flag, leaves.              | 8 sec |
| 9 | Seagull flock       | 3 birds on independent figure-8s.                            | 10 sec |
|10 | "Meanwhile…" panel  | Full-screen comic panel flashes (~600 ms), dismisses.       | 1.5 sec |

L1 + button shortcuts skip directly to specific summons without disturbing
the cycle pointer. Summons do **not** lock input — Johnny stays
controllable while seagulls / Mary / King Kong play out.

## Persistent island state

Once placed, things stay until the player exits freeplay. State resets on
each fresh entry into freeplay (no memory-card persistence).

| Object        | Trigger                        | Limit                        | Restore handling |
|---------------|--------------------------------|------------------------------|------------------|
| Sandcastle    | Cross on beach center, 5th press finalizes | Up to 3 simultaneous | Stamped into clean baseline at place time. Sub-rect re-snapshot. |
| Lit fire      | R1 + Cross                     | One at a time               | Animated `FIRE1-5.BMP` cycle, redrawn per frame. |
| Raft stage    | R1 + ←                         | Stage 0..5                  | Stamped at canonical raft position. Re-snapshot on change. |
| Coconut pile  | knocked from tree              | Up to 5 piled                | Stamped at impact position. 6th triggers explosion easter egg. |
| Day / night   | R1 + ↑                         | Either                       | Re-stamps all persistents into new clean baseline, then re-snapshots. |
| Holiday       | R1 + →                         | One of 5                     | Loads `HOLIDAY.BMP` lazily; stamped per-frame. |
| Friday-friendship | Native Canoe summon counter | 0..3 sticky during session   | Affects Native Canoe summon behavior. |

The "freeplay-owned clean-rect array" is the core implementation pattern.
Freeplay maintains `gFreeplayOwnedRects[]` (up to 8 entries). When
persistent state changes, freeplay rebuilds the list, stamps new state
into the bg tiles, then calls `grSaveCleanBgRects(...)` with the full
set. This is the same pattern the ocean runtime uses for its wave +
Johnny rect.

## Ambient life

The island has a heartbeat. These tick continuously regardless of input:

| Event           | Cadence                       | What |
|-----------------|-------------------------------|------|
| Cloud drift     | every 25–45 sec (random)      | A `CLOUDS.BMP` cloud crosses sky. |
| Solo seagull    | every 30–90 sec               | One bird crosses (silent, low-key). |
| Random coconut  | every 60–180 sec, low chance  | Coconut drops from tree. |
| Auto-thirst     | after 90 sec of walking       | Johnny auto-fires the "hot" gag once. |
| Auto-fish-bite  | after 30 sec fishing          | Auto-triggers the catch sequence with kingfish. |
| Auto-sleep      | 60 sec idle                   | Johnny yawns, sits down, "Z" sprite. Wakes on any input. |

Auto-events only fire while Johnny is in `IDLE` mode (or `WALK`, for
thirst). They never interrupt cinematics, gag cycles, or active summons.

## Konami code & secrets

**↑ ↑ ↓ ↓ ← → ← → Square Cross** within 4 seconds triggers the **Castaway
Cove Carnival**: every summon spawns simultaneously, banner reads "PARTY!",
King Kong roars, layered SFX (capped at 8 simultaneous SPU voices). Lasts
~12 seconds; input is locked. After: everyone exits in their normal
patterns and Johnny does one celebratory snazzy-strut.

Other secrets include "The Big One" (Cross 7 times within 2 sec at fishing
shore), Sleep walker, Coconut stack-pocalypse, Friday's friend, The
Tornado, Holiday speedrun, Strut forever, and Drunken master. Easter-egg
combo detection is data-driven — a struct array of
`{ buttonSequence, callback, lockMs }` so adding new secrets later
doesn't touch input code.

## Memory budget

Comfortably under the 2 MB ceiling.

| Item                                | Size   | Notes |
|-------------------------------------|--------|-------|
| `BACKGRND.BMP` slot 0               | ~93 KB | Sticky across screensaver loops. |
| `HOLIDAY.BMP` variant slot          | ~30 KB | Lazy on first holiday-toggle. |
| `JOHNWALK.BMP` slot 1               | ~100 KB | Always loaded. |
| Active gag BMP, slot 2              | ≤ 50 KB | LRU; one at a time. |
| Active summon BMP, slot 3           | ≤ 80 KB | LRU; King Kong is the largest. |
| Persistent BMPs (FIRE, COCONUTS, MRAFT, GJCASTLE) | ≤ 60 KB | Concat where possible. |
| `LITEBULB` overlay                  | ≤ 5 KB | tiny |
| bg tiles (4)                        | 614 KB | |
| Clean rects                         | ~280 KB | Slightly larger than fishing3 due to summon corridors. |
| Freeplay state struct + arrays      | < 4 KB | |
| **Total**                           | **~1.3 MB** | |

If memory pressure surfaces: drop a couple of summons or aggressively
LRU more slots.

## Real-hardware constraints

- Only PSn00bSDK APIs. No DuckStation-specific shortcuts. No emulator-only
  behavior.
- VRAM addressing strict — freeplay reuses the existing infrastructure
  and introduces no new VRAM math.
- No CD streaming per-frame (no `.FG2`); all BMPs load once at scene
  start. Safer than fishing scenes from a CD-timing standpoint.
- No interrupt blocking; long animations advance frame-by-frame.
- Cap simultaneous SPU voices at 8 (PS1 SPU has 24, headroom reserved).
- Standard digital + analog pad reads only.

## Pause-menu integration

Freeplay is also reachable from the [pause menu]({{ '/docs/pause-menu/' | relative_url }}).
Selecting "Freeplay Mode" sets a scene-switch flag which the current
scene's main loop checks via `sceneSwitchRequested()`; the loop exits
cleanly and the screensaver outer loop calls `foregroundPilotPlay()` again,
which dispatches to freeplay.

Inside freeplay:

- Start (tap) = exit to screensaver random rotation.
- Long-press Start (>1 sec) = open pause menu inside freeplay.

## Related pages

- [Pause menu]({{ '/docs/pause-menu/' | relative_url }}) — entry point
  from in-screensaver.
- [Holidays]({{ '/docs/holidays/' | relative_url }}) — the holiday
  overlay cycle uses `gHolidays[]`.
- [Development workflow]({{ '/docs/dev-workflow/' | relative_url }}) — how
  freeplay differs from a captured-pack scene during bring-up.

## View source on GitHub

- [`docs/ps1/freeplay-mode-design.md`]({{ site.github_url }}/blob/main/docs/ps1/freeplay-mode-design.md) — locked design.
- [`src/foreground_pilot.c`]({{ site.github_url }}/blob/main/src/foreground_pilot.c) — dispatch entry point.
