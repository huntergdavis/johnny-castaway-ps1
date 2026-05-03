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

/* Compile-gated diagnostic — emits one printf per kernel call. Off by
 * default to keep the runtime code phase stable. Flip to 1 when
 * debugging overdraw / sprite-index / position.
 *
 * Each printf to PS1 TTY is ~1ms/char via SIO; with a ~70-char message
 * per frame the loop slows from 60 Hz to ~8 Hz, which by itself looks
 * indistinguishable from "walk is stuck" / "Johnny is painting trails"
 * in the visual. KEEP THIS OFF for any visual-correctness debugging. */
#define WALK_RENDER_DIAG 0

#if WALK_RENDER_DIAG
extern int printf(const char *, ...);
static unsigned long sDiagFrameNo = 0;
#endif

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
                     int flip, int behindTree)
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

    /* Update redraw cache. Always — every fresh frame becomes the
     * "current pose" the story-loop driver may want to redraw on
     * inter-tick VBlanks. */
    sCacheValid     = 1;
    sCacheX         = x;
    sCacheY         = y;
    sCacheSpriteIdx = spriteIdx;
    sCacheFlip      = flip;
    sCacheBehindTree = behindTree;

#if WALK_RENDER_DIAG
    printf("JCWALK frame=%lu kind=tick x=%d y=%d sprite=%u flip=%d behind=%d slot_n=%u\n",
           sDiagFrameNo++, (int)x, (int)y, (unsigned)spriteIdx,
           flip, behindTree,
           johnwalkSlot ? (unsigned)johnwalkSlot->numSprites[0] : 0u);
#endif
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

#if WALK_RENDER_DIAG
    printf("JCWALK frame=%lu kind=redraw x=%d y=%d sprite=%u flip=%d behind=%d\n",
           sDiagFrameNo++, (int)sCacheX, (int)sCacheY,
           (unsigned)sCacheSpriteIdx, sCacheFlip, sCacheBehindTree);
#endif
}
