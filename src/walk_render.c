/*
 *  This file is part of 'Johnny Reborn' — PS1 port (walk subsystem).
 *  Copyright (C) 2026 Hunter Davis. GPL-3.0.
 *
 *  walk_render — the shared draw kernel. See walk_render.h for the
 *  contract.
 */

#include "mytypes.h"

#ifdef PS1_BUILD
#include "graphics_ps1.h"
#else
#include "graphics.h"
#include "sound.h"
#endif

#include "walk_render.h"

/* Footsteps default ON. Pause-menu Options sub-screen flips this; the
 * setting is persisted to memcard alongside soundMuted etc. (Phase 4.3).
 *
 * If walkStepSamples table for the active edge is empty, fireFootstep
 * is harmlessly a no-op even with this flag ON — see Phase 4.1 audit. */
int footstepsEnabled = 1;

/* Last-frame cache: stores the most recent walkRenderFrame() params
 * so the story-loop driver can re-draw the same pose against a new
 * frame envelope without ticking walkAnimate(). This is a redraw
 * cache, NOT walk state — walk state (current spot, heading, path)
 * stays in walk.c. */
static int    sCacheValid    = 0;
static sint16 sCacheX         = 0;
static sint16 sCacheY         = 0;
static uint16 sCacheSpriteIdx = 0;
static int    sCacheFlip      = 0;
static int    sCacheBehindTree = 0;


void walkRenderResetCache(void)
{
    sCacheValid = 0;
}


void walkRenderFrame(SDL_Surface *sfc,
                     struct TTtmSlot *johnwalkSlot,
                     struct TTtmSlot *islandBgSlot,
                     sint16 x, sint16 y, uint16 spriteIdx,
                     int flip, int behindTree, int fireFootstep)
{
    /* The walking sprite. flip indicates the original walk_data row
     * stored a flipped pose (east-facing frames are reused for
     * west-facing via grDrawSpriteFlip). */
    if (flip)
        grDrawSpriteFlip(sfc, johnwalkSlot, x, y, spriteIdx, 0);
    else
        grDrawSprite    (sfc, johnwalkSlot, x, y, spriteIdx, 0);

    /* Behind-tree cover-up: stamp trunk + leaf cluster from the island
     * background slot AFTER the walking sprite so the GPU OT renders
     * them on top, hiding Johnny at the right pixels. The coordinates
     * and sprite indices match the original walk.c:174-175 cover-up
     * path verbatim — the tree's bbox doesn't move, so they're
     * constants. */
    if (behindTree && islandBgSlot != NULL) {
        grDrawSprite(sfc, islandBgSlot, 442, 148, 13, 0);  /* trunk    */
        grDrawSprite(sfc, islandBgSlot, 365, 122, 12, 0);  /* leaves   */
    }

    /* Footstep trigger. Phase 4.2 wires the actual soundPlay() call
     * with a sample id resolved from walkStepSamples[fromSpot][toSpot]
     * (or freeplay's per-step counter). Until then, the trigger is a
     * documented no-op so the kernel API surface is stable for the
     * driver code being written in parallel.
     *
     * The footstepsEnabled gate stays here — the kernel is the
     * single point of audio policy enforcement, even when the actual
     * playback call lands later. */
    if (fireFootstep && footstepsEnabled) {
        /* Phase 4.2: soundPlay(walkStepSampleId); */
    }

    /* Update redraw cache. Always — every fresh frame becomes the
     * "current pose" the story-loop driver may want to redraw on
     * inter-tick VBlanks. */
    sCacheValid     = 1;
    sCacheX         = x;
    sCacheY         = y;
    sCacheSpriteIdx = spriteIdx;
    sCacheFlip      = flip;
    sCacheBehindTree = behindTree;
}


void walkRedrawLastFrame(SDL_Surface *sfc,
                         struct TTtmSlot *johnwalkSlot,
                         struct TTtmSlot *islandBgSlot)
{
    if (!sCacheValid) return;

    if (sCacheFlip)
        grDrawSpriteFlip(sfc, johnwalkSlot, sCacheX, sCacheY, sCacheSpriteIdx, 0);
    else
        grDrawSprite    (sfc, johnwalkSlot, sCacheX, sCacheY, sCacheSpriteIdx, 0);

    if (sCacheBehindTree && islandBgSlot != NULL) {
        grDrawSprite(sfc, islandBgSlot, 442, 148, 13, 0);  /* trunk */
        grDrawSprite(sfc, islandBgSlot, 365, 122, 12, 0);  /* leaves */
    }
    /* No footstep on redraw — that's a once-per-step trigger. */
}
