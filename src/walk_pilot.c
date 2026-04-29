/*
 *  This file is part of 'Johnny Reborn' — PS1 port (walk subsystem).
 *  Copyright (C) 2026 Hunter Davis. GPL-3.0.
 *
 *  walk_pilot — story-loop walk driver. See walk_pilot.h for contract
 *  and docs/ps1/walk-implementation-plan.md § Phase 2.5 for design.
 */

#include <stddef.h>
#include "mytypes.h"
#include "graphics_ps1.h"
#include "events_ps1.h"
#include "foreground_pilot.h"
#include "island.h"
#include "walk.h"
#include "walk_data.h"
#include "walk_render.h"
#include "walk_pilot.h"

extern struct TIslandState islandState;
extern int grDx, grDy;

#ifndef PS1_BUILD
/* Host build's eventsWaitTick / VSync; not used here, but keep the
 * signature so the file can compile on host if anyone ever lifts
 * the story-loop driver across. */
extern int eventsWaitTick(int n);
#endif

/* Per-walk frame cadence. walkAnimate() returns delays in this many
 * VBlanks between pose advances. Original walk.c sets timer=delay=6
 * (matches original Sierra game pacing). */
#define WALK_VBLANKS_PER_POSE 6

/* JOHNWALK.PSB slot — owned by walk_pilot, lazy-loaded on first walk,
 * persists across walks for the lifetime of a screensaver-loop
 * iteration to avoid CD reloads on every transition. */
static struct TTtmSlot gWalkBmpSlot;
static int gWalkBmpLoaded = 0;

/* Synthetic ttmThread used to satisfy walkAnimate's signature.
 * walk.c reads ttmSlot + ttmLayer; ttmLayer is the SDL_Surface (= PS1Surface)
 * field, NULL on PS1 since the kernel doesn't dereference it. */
static struct TTtmThread gWalkThread;


static void walkPilotEnsureBmp(void)
{
    if (gWalkBmpLoaded) return;
    grLoadBmp(&gWalkBmpSlot, 0, "JOHNWALK.BMP");
    gWalkBmpLoaded = 1;
}


void fgWalkRenderTeardown(void)
{
    if (!gWalkBmpLoaded) return;
    grReleaseBmp(&gWalkBmpSlot, 0);
    gWalkBmpLoaded = 0;
    walkRenderResetCache();
}


/* Single VBlank of redraw using the kernel's last-frame cache.
 * Used during the inter-tick frames (when walkAnimate hasn't ticked
 * yet but we still need to present a frame so VSync stays honest). */
static void walkPilotPresentInterTick(struct TTtmSlot *bgSlot)
{
    grBeginFrame();
    grRestoreBgTiles();
    walkRedrawLastFrame(NULL, &gWalkBmpSlot, bgSlot);
    grUpdateDisplay(NULL, NULL, NULL);
}


int fgWalkRender(int fromSpot, int fromHdg, int toSpot, int toHdg)
{
    /* No-op walks: invalid prev (LEFT_ISLAND scenes set prevSpot=-1)
     * or same-spot-same-heading. Same-spot-different-heading still
     * walks (Johnny turns in place via walk.c's turn-table logic). */
    if (fromSpot < 0 || toSpot < 0)
        return 0;
    if (fromSpot == toSpot && fromHdg == toHdg)
        return 0;

    walkPilotEnsureBmp();
    walkRenderResetCache();

    /* Match the original engine's island-offset wiring (ads.c:2222). */
    grDx = (int)islandState.xPos;
    grDy = (int)islandState.yPos;

    /* Set up the synthetic thread walkAnimate needs. ttmLayer stays
     * NULL — kernel passes it to grDrawSprite which doesn't deref. */
    gWalkThread.ttmSlot   = &gWalkBmpSlot;
    gWalkThread.ttmLayer  = NULL;
    gWalkThread.timer     = WALK_VBLANKS_PER_POSE;
    gWalkThread.delay     = WALK_VBLANKS_PER_POSE;

    /* The island background slot for behind-tree cover-up. May be
     * NULL during very early scene-loop init; cover-up just degrades
     * to "Johnny visible through tree" until the slot is populated.
     * Phase 5 of the walk plan tightens this; for now NULL is safe. */
    struct TTtmSlot *bgSlot = fgBackdropGetSlot();

    /* Set walk.c's static state up for this transition. */
    walkInit(fromSpot, fromHdg, toSpot, toHdg);

    /* Initial pose tick: walkAnimate computes the first pose, calls
     * the kernel (populates the redraw cache), and returns the next
     * delay. We then present that pose for `delay` VBlanks before
     * ticking again. */
    grCurrentThread = NULL;     /* walk_pilot doesn't use the thread
                                 * replay-sprites mechanism; redraw
                                 * cache covers inter-tick frames. */
    int delay = walkAnimate(&gWalkThread, bgSlot);

    while (delay > 0) {
        /* Present the current pose for `delay` VBlanks. */
        for (int i = 0; i < delay; i++) {
            walkPilotPresentInterTick(bgSlot);
            VSync(0);
        }
        /* Tick walkAnimate to compute the next pose. The kernel call
         * inside walkAnimate updates the redraw cache. */
        delay = walkAnimate(&gWalkThread, bgSlot);
    }

    walkRenderResetCache();
    return 0;
}
