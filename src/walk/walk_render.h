/*
 *  This file is part of 'Johnny Reborn' — PS1 port (walk subsystem).
 *  Copyright (C) 2026 Hunter Davis. GPL-3.0.
 *
 *  walk_render — the shared kernel that draws ONE walking sprite frame
 *  against the restored island background. Used by both:
 *    - story-loop walks (walk_pilot.c, driving walk.c's pre-baked
 *      path-table state machine)
 *    - freeplay direct-control walks (scene_freeplay.c, driving from
 *      D-pad input per VBlank)
 *
 *  The kernel owns no walk state. It's pure draw + optional behind-tree
 *  cover-up. Callers manage their own position, sprite-frame index,
 *  heading, and timing.
 *
 *  Architecture detail in docs/ps1/walk-implementation-plan.md § 3.5.
 */
#ifndef WALK_RENDER_H
#define WALK_RENDER_H

#include "mytypes.h"

#ifdef PS1_BUILD
#include "graphics_ps1.h"
#else
#include "graphics.h"
#endif

struct TTtmSlot;

/*
 * Draw one walking-frame sprite against the restored background.
 *
 *   sfc           — host-build SDL surface; ignored on PS1 (pass NULL
 *                   or whatever the caller has — both work).
 *   johnwalkSlot  — TTtmSlot loaded with JOHNWALK.BMP/PSB (caller owns).
 *   islandBgSlot  — TTtmSlot for the island background (used when
 *                   behindTree is set, to stamp trunk + leaf cover-up
 *                   sprites). May be NULL only when behindTree=0.
 *   x, y          — island-relative screen coords (caller adds
 *                   islandState.xPos/yPos before calling). Order
 *                   matches grDrawSprite's native (x, y, sprite) form.
 *   spriteIdx     — index into JOHNWALK frames (the (*data)[3] value
 *                   from walk_data.h, or freeplay's frame counter).
 *   flip          — 0 for normal, 1 for horizontal flip (JOHNWALK
 *                   stores east-facing frames; west-facing reuses them
 *                   flipped).
 *   behindTree    — 1 if Johnny is between SPOT_3 and SPOT_4 (tree
 *                   z-region). Stamps trunk+leaf cover-up after the
 *                   walking sprite so the tree visibly covers him.
 */
void walkRenderFrame(SDL_Surface *sfc,
                     struct TTtmSlot *johnwalkSlot,
                     struct TTtmSlot *islandBgSlot,
                     sint16 x, sint16 y, uint16 spriteIdx,
                     int flip, int behindTree);

/* Re-draw the most recent walkRenderFrame() pose against a fresh
 * frame envelope. The story-loop driver uses this between
 * walkAnimate() advance ticks (typically every 6 VBlanks) to keep
 * Johnny visible during the in-between frames.
 *
 * Freeplay does NOT use this — it calls walkRenderFrame every
 * VBlank with live D-pad-driven coords.
 *
 * Caller still owns grBeginFrame / grRestoreBgTiles / grEndFrame. */
void walkRedrawLastFrame(SDL_Surface *sfc,
                         struct TTtmSlot *johnwalkSlot,
                         struct TTtmSlot *islandBgSlot);

/* Reset cached last-frame state. Call when a walk sequence ends so
 * the next sequence doesn't accidentally redraw a stale pose. */
void walkRenderResetCache(void);

#endif /* WALK_RENDER_H */
