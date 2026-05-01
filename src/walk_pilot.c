/*
 *  This file is part of 'Johnny Reborn' — PS1 port (walk subsystem).
 *  Copyright (C) 2026 Hunter Davis. GPL-3.0.
 *
 *  walk_pilot — story-loop walk driver. See walk_pilot.h for contract
 *  and docs/ps1/walk-implementation-plan.md § Phase 2.5 for design.
 */

#include <stddef.h>
#include <stdlib.h>
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
 * VBlanks between pose advances. Original walk.c sets timer=delay=6,
 * but that was tuned to PC clock-tick semantics; on PS1 at 60Hz it
 * works out to ~100ms/pose and a long SPOT_A→SPOT_F walk runs ~3s,
 * which the user reads as "very slow". 3 VBlanks (~50ms/pose) keeps
 * the leg-swing readable but cuts walk duration roughly in half. */
#define WALK_VBLANKS_PER_POSE 3

/* JOHNWALK.PSB slot — owned by walk_pilot, lazy-loaded on first walk,
 * persists across walks for the lifetime of a screensaver-loop
 * iteration to avoid CD reloads on every transition. */
static struct TTtmSlot gWalkBmpSlot;
static int gWalkBmpLoaded = 0;

/* Synthetic ttmThread used to satisfy walkAnimate's signature.
 * walk.c reads ttmSlot + ttmLayer; ttmLayer is the SDL_Surface (= PS1Surface)
 * field, NULL on PS1 since the kernel doesn't dereference it. */
static struct TTtmThread gWalkThread;

/* Persistent walk-area pristine buffer. Holds bgTile pixels at a
 * known-clean moment (right after fgBackdropEnableWaveBackdrop, before
 * scene playback dirties them). Keep the allocation resident once it
 * succeeds: repeated free/malloc cycles were fragmenting the heap, and
 * later walks lost their erase baseline when this buffer could no longer
 * be reallocated.
 *
 * The old buffer was a fixed screen-space rect (80,150,520,180). That
 * missed Johnny whenever the randomized island y offset pushed the walk
 * path down: walk_data top-left y can reach 257 + islandState.yPos, and
 * JOHNWALK frames are up to 78 px tall. The result was lower-body residue
 * from every previous walking pose. Keep roughly the same allocation size,
 * but anchor the rect to the island so it always follows the path.
 *
 * Relative bbox covers walkData x=284..526/y=209..257 plus the largest
 * JOHNWALK frame (48x78), plus the behind-tree cover-up sprites
 * (leaves 365,122 152x69; trunk 442,148 24x145). It is deliberately
 * tighter than the earlier 380x250 allocation so the required resident
 * footprint is small enough to coexist with FG2 clean rects.
 *
 * Size: 340 x 224 x 2 = ~149KB. */
#define WALK_CLEAN_REL_X  260
#define WALK_CLEAN_REL_Y  120
#define WALK_CLEAN_W      340
#define WALK_CLEAN_H      224

static uint16 *gWalkCleanBuf  = NULL;
static int     gWalkCleanValid = 0;
static sint16  gWalkCleanX = 0;
static sint16  gWalkCleanY = 0;
/* State key — any field changing means the buffer's pixels are stale. */
static int gWalkCleanRaft     = -1;
static int gWalkCleanLowTide  = -1;
static int gWalkCleanNight    = -1;
static int gWalkCleanHoliday  = -1;
static int gWalkCleanXPos     = -32768;
static int gWalkCleanYPos     = -32768;


static int walkPilotCleanStateMatches(int raft, int lowTide, int night,
                                      int holidayId, int xPos, int yPos)
{
    return gWalkCleanRaft == raft &&
           gWalkCleanLowTide == lowTide &&
           gWalkCleanNight == night &&
           gWalkCleanHoliday == holidayId &&
           gWalkCleanXPos == xPos &&
           gWalkCleanYPos == yPos;
}


void walkPilotCaptureCleanWalkAreaIfStale(int raft, int lowTide, int night,
                                          int holidayId, int xPos, int yPos)
{
    if (gWalkCleanValid &&
        walkPilotCleanStateMatches(raft, lowTide, night, holidayId, xPos, yPos))
        return;

    if (gWalkCleanBuf == NULL) {
        gWalkCleanBuf = (uint16 *)malloc(
            (size_t)WALK_CLEAN_W * (size_t)WALK_CLEAN_H * sizeof(uint16));
        if (gWalkCleanBuf == NULL) {
            /* Out of memory — leave invalid; walks will skip clean-area
             * restore and fall back to whatever bgTile contains. Visual
             * artifact (overpaint) but no crash. */
            extern int printf(const char *, ...);
            printf("JCWALK: walkClean buffer alloc failed\n");
            return;
        }
    }

    gWalkCleanX = (sint16)(xPos + WALK_CLEAN_REL_X);
    gWalkCleanY = (sint16)(yPos + WALK_CLEAN_REL_Y);

    grCaptureBgRect(gWalkCleanBuf, gWalkCleanX, gWalkCleanY,
                    WALK_CLEAN_W, WALK_CLEAN_H);

    gWalkCleanValid    = 1;
    gWalkCleanRaft     = raft;
    gWalkCleanLowTide  = lowTide;
    gWalkCleanNight    = night;
    gWalkCleanHoliday  = holidayId;
    gWalkCleanXPos     = xPos;
    gWalkCleanYPos     = yPos;
}


void walkPilotReleaseCleanWalkArea(void)
{
    if (gWalkCleanBuf) {
        free(gWalkCleanBuf);
        gWalkCleanBuf = NULL;
    }
    gWalkCleanValid = 0;
}


int walkPilotCleanBufferAllocated(void)
{
    return gWalkCleanBuf != NULL ? 1 : 0;
}

unsigned long walkPilotCleanBufferBytes(void)
{
    if (gWalkCleanBuf == NULL) return 0;
    return (unsigned long)WALK_CLEAN_W *
           (unsigned long)WALK_CLEAN_H *
           (unsigned long)sizeof(uint16);
}

int walkPilotJohnwalkSlotLoaded(void)
{
    return gWalkBmpLoaded ? 1 : 0;
}


int walkPilotInit(void)
{
    int ok = 1;

    if (gWalkCleanBuf == NULL) {
        gWalkCleanBuf = (uint16 *)malloc(
            (size_t)WALK_CLEAN_W * (size_t)WALK_CLEAN_H * sizeof(uint16));
        if (gWalkCleanBuf == NULL) {
            extern int printf(const char *, ...);
            printf("JCWALK: walkPilotInit clean-buf alloc failed (%u bytes)\n",
                   (unsigned)((unsigned long)WALK_CLEAN_W *
                              (unsigned long)WALK_CLEAN_H * sizeof(uint16)));
            ok = 0;
        }
        /* Mark invalid until the first scene's setup captures real
         * pristine pixels. walkPilotRestoreClean checks gWalkCleanValid
         * before copying, so an early-walk attempt is a safe no-op. */
        gWalkCleanValid = 0;
    }

    /* JOHNWALK.PSB is NOT pre-loaded at boot — the ~100KB sprite atlas
     * combined with the clean-rect/stream pre-allocs blew through PS1's
     * boot heap budget when the first scene tried to alloc its 600KB
     * bgTile (153 KB per tile, malloc returned NULL). Lazy-load on first
     * walk via walkPilotEnsureBmp instead. By the time a walk fires the
     * first scene has already played and bgTile is steady-state, so
     * the JOHNWALK alloc lands cleanly. Trade-off: ~50ms one-time CD
     * seek delay on the first walk; previously we paid that at boot. */
    return ok;
}


/* Per-walk-frame: copy persistent clean pixels back into bgTile so the
 * previous pose's Johnny is wiped before the new pose composites. The
 * underlying grRestoreBgRect respects prevDirty, so on every frame after
 * the first only Johnny's previously-touched rows actually get copied
 * — same dirty-rect economy as grRestoreBgFromRects. */
static void walkPilotRestoreClean(void)
{
    if (!gWalkCleanValid || gWalkCleanBuf == NULL) return;
    grRestoreBgRect(gWalkCleanBuf, gWalkCleanX, gWalkCleanY,
                    WALK_CLEAN_W, WALK_CLEAN_H);
}


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

    /* Suppress runtime compose for the entire walk. Without this, every
     * frame's grUpdateDisplay call re-stamps the previous FG2 scene's
     * baked-in Johnny on top of the walking sprite, looking exactly
     * like Johnny painting over himself. */
    foregroundPilotSuppressCompose(1);

    /* Force a full-tile dirty so the first walk frame's clean-area
     * restore copies the entire walk-area buffer (not just the rows
     * scene N's last composite happened to dirty). */
    grForceFullRedrawNextFrame();

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
     * to "Johnny visible through tree" until the slot is populated. */
    struct TTtmSlot *bgSlot = fgBackdropGetSlot();

    /* Set walk.c's static state up for this transition. */
    walkInit(fromSpot, fromHdg, toSpot, toHdg);

    grCurrentThread = NULL;     /* kernel's redraw cache covers
                                 * inter-tick frames; thread replay
                                 * mechanism not used. */

    /* Pre-walk dwell: hold scene N's last frame for ~12 VBlanks (~200ms)
     * before the walk's first composite. The framebuffer already shows
     * scene N's final pose at this point, so all we need is to wait
     * VSyncs without redrawing — that lets the eye register the scene's
     * end state before Johnny "switches" into walking pose. Without the
     * dwell, a turn-in-place walk reads as a hard pose-swap at the same
     * spot ("teleported into a new position"). */
    for (int dwell = 0; dwell < 12; dwell++) {
        VSync(0);
    }

    int timerLeft = 1;
    int walkDone  = 0;

    while (!walkDone) {
        grBeginFrame();
        /* Start from the previous scene's clean rects first. This clears
         * foreground leftovers and shoreline wave residue, and mirrors the
         * normal frame-start path's currDirty reset before the walk-specific
         * restore below handles Johnny's full route. */
        grRestoreBgFromRects();
        /* Clear the previous walk pose by restoring pixels from the
         * persistent walk-area pristine buffer. Falls through (no-op)
         * if the capture hasn't been done yet — only the first scene
         * before its setup runs is in this state, which can't have
         * fired a walk anyway. */
        walkPilotRestoreClean();

        /* Wave animation — keep the ocean moving during walks so
         * scene→walk→scene looks seamless. */
        fgBackdropTickWavesPublic();

        if (timerLeft <= 0) {
            /* Tick: walkAnimate advances pose + calls walkRenderFrame
             * (which stamps Johnny + updates the redraw cache). */
            int next = walkAnimate(&gWalkThread, bgSlot);
            if (next <= 0) {
                walkRedrawLastFrame(NULL, &gWalkBmpSlot, bgSlot);
                walkDone = 1;
                timerLeft = 0;
            } else {
                timerLeft = next - 1;   /* current frame counts as 1 */
            }
        } else {
            /* Inter-tick frame: redraw the cached pose. */
            walkRedrawLastFrame(NULL, &gWalkBmpSlot, bgSlot);
            timerLeft--;
        }

        fgBackdropStampHolidayPublic();
        grUpdateDisplay(NULL, NULL, NULL);
    }

    /* Hold-frame: keep the final pose visible briefly before yielding
     * to the next scene. 12 VBlanks ≈ 200ms for actual position changes.
     *
     * Turn-in-place walks (same spot, just rotating) don't benefit from
     * the hold — Johnny was already at this position, the visible change
     * is just heading. Cut the hold to 0 so back-to-back same-spot
     * scenes feel snappy. */
    int holdFrames = (fromSpot == toSpot) ? 0 : 12;
    for (int hold = 0; hold < holdFrames; hold++) {
        grBeginFrame();
        grRestoreBgFromRects();
        walkPilotRestoreClean();
        fgBackdropTickWavesPublic();
        walkRedrawLastFrame(NULL, &gWalkBmpSlot, bgSlot);
        fgBackdropStampHolidayPublic();
        grUpdateDisplay(NULL, NULL, NULL);
    }

    walkRenderResetCache();
    fgWalkRenderTeardown();
    foregroundPilotSuppressCompose(0);
    return 0;
}
