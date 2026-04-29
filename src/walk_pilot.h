/*
 *  This file is part of 'Johnny Reborn' — PS1 port (walk subsystem).
 *  Copyright (C) 2026 Hunter Davis. GPL-3.0.
 *
 *  walk_pilot — story-loop walk driver. Wraps walk.c + walk_data.h's
 *  pre-baked spot-graph state machine and feeds the walk_render kernel
 *  one frame at a time. Called by the screensaver-loop scene picker
 *  (jc_reborn.c fgLoopNextScene) BEFORE each scene plays, to walk
 *  Johnny from his current spot/heading to the next scene's start.
 *
 *  Freeplay direct-control walks do NOT go through this file —
 *  scene_freeplay.c calls walk_render directly with D-pad-driven
 *  position. This is the story-loop side only.
 */
#ifndef WALK_PILOT_H
#define WALK_PILOT_H

#include "mytypes.h"

/*
 * Walk Johnny from (fromSpot, fromHdg) to (toSpot, toHdg) using
 * walk.c's pre-baked path data + the shared walk_render kernel.
 *
 * Returns 0 on normal completion, non-zero if the walk was aborted
 * (e.g. the caller signalled a capture/regtest abort during the loop).
 *
 * Pre-conditions:
 *   - islandState.xPos / yPos are set to the current island offset.
 *   - The island background (BACKGRND.PSB / clean-rect set) is loaded
 *     and grRestoreBgTiles() works in the current scene.
 *   - JOHNWALK.PSB is on disc (Phase 1 wired this).
 *
 * If fromSpot == toSpot AND fromHdg == toHdg, returns immediately
 * (no-op walk). If fromSpot is invalid (-1 sentinel from a
 * LEFT_ISLAND scene), returns immediately — the caller is expected
 * to pick a FIRST scene that resets position.
 */
int fgWalkRender(int fromSpot, int fromHdg, int toSpot, int toHdg);

/* Free the JOHNWALK.PSB slot. Call at scene-loop teardown if you
 * want the VRAM back; safe to skip and let the slot persist across
 * walks. */
void fgWalkRenderTeardown(void);

#endif /* WALK_PILOT_H */
