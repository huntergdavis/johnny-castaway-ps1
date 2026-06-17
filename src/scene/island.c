/*
 *  This file is part of 'Johnny Reborn'
 *
 *  An open-source engine for the classic
 *  'Johnny Castaway' screensaver by Sierra.
 *
 *  Copyright (C) 2019 Jeremie GUILLAUME
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

/* Conditional includes for PS1 freestanding build */
#ifndef PS1_BUILD
#include <stdlib.h>
#include <stdio.h>
#else
#include <stddef.h>
extern int rand(void);
#endif

#include "mytypes.h"
/* Platform-specific graphics headers */
#ifdef PS1_BUILD
#include "graphics_ps1.h"
#else
#include "graphics.h"
#endif
#include "island.h"
#include "holidays.h"


struct TIslandState islandState = { 0, 0, 0, 0, 0, 0 };
static int gIslandSuppressBackdropTrunk = 0;
static int gIslandSuppressBackdropLeafs = 0;

/* Wave foam cache. The ocean shows 3 foam positions (high tide) / 4 (low
 * tide) simultaneously; islandAnimate advances ONE position per tick. The
 * per-frame clean-rect restore (grRestoreBgFromRects) wipes the foam
 * region every frame, so every position must be RE-DRAWN each frame or
 * only the most-recently-advanced one survives (the "only the small wave,
 * flickering on/off" regression — the slow delay just made it visible;
 * fast delay refreshed all positions before the eye noticed). Cache each
 * position's last sprite+screen-pos, indexed by wave position, and redraw
 * the whole set every frame. */
#define WAVE_CACHE_MAX 4
static PS1Surface *gWaveCacheSprite[WAVE_CACHE_MAX] = { NULL };
static sint16 gWaveCacheX[WAVE_CACHE_MAX] = { 0 };
static sint16 gWaveCacheY[WAVE_CACHE_MAX] = { 0 };
static int gIslandWaveCounter1 = 0;
static int gIslandWaveCounter2 = 0;

void islandInit(struct TTtmThread *ttmThread)
{
    struct TTtmSlot *ttmSlot = ttmThread->ttmSlot;


    if (islandState.night) {
        grLoadScreen("NIGHT.SCR");
    }
    else {
        char scrName[] = "OCEAN00.SCR";
        scrName[6] = (char)('0' + (rand() % 3));
        grLoadScreen(scrName);
    }

    ttmThread->ttmLayer = grBackgroundSfc;

    grDx = islandState.xPos;
    grDy = islandState.yPos;


    // Raft

    grLoadBmp(ttmSlot, 0, "MRAFT.BMP");

    int xRaft = (islandState.lowTide ? 529 : 512);
    int yRaft = (islandState.lowTide ? 281 : 266);

    switch (islandState.raft) {
        case 1: grDrawSprite(grBackgroundSfc, ttmSlot, xRaft, yRaft, 0, 0); break;  // raft-1
        case 2: grDrawSprite(grBackgroundSfc, ttmSlot, xRaft, yRaft, 1, 0); break;  // raft-2
        case 3: grDrawSprite(grBackgroundSfc, ttmSlot, xRaft, yRaft, 2, 0); break;  // raft-3
        case 4: grDrawSprite(grBackgroundSfc, ttmSlot, xRaft, yRaft, 3, 0); break;  // raft-4
        case 5: grDrawSprite(grBackgroundSfc, ttmSlot, xRaft, yRaft, 4, 0); break;  // raft-5
    }


    grLoadBmp(ttmSlot, 0, "BACKGRND.BMP");


    // Clouds

    int windDirection = rand() % 2;
    int numClouds;
    uint16 cloudX = 0;
    uint16 cloudY = 0;

    grDx = grDy = 0;

    if (grCaptureDir != NULL && grCaptureDir[0] != '\0') {
        /* Decorative cloud randomness adds non-scene drift to scripted captures. */
        numClouds = 0;
    }
    else if (rand() % 2)
         numClouds = 1;
    else if (rand() % 2)
        numClouds = 0;
    else if (rand() % 4)
        numClouds = 2;
    else if (rand() % 4)
        numClouds = 3;
    else if (rand() % 4)
        numClouds = 4;
    else
        numClouds = 5;

    for (int i=0; i < numClouds; i++) {

        int cloudNo = rand() % 3;

        switch (cloudNo) {

            case 0:
                cloudX = rand() % (640 - 129);
                cloudY = rand() % (135 - 36 );
                break;

            case 1:
                cloudX = rand() % (640 - 192);
                cloudY = rand() % (135 - 57 );
                break;

            case 2:
                cloudX = rand() % (640 - 264);
                cloudY = rand() % (135 - 76 );
                break;
        }

        if (windDirection)
            grDrawSprite(grBackgroundSfc, ttmSlot, cloudX, cloudY, 15 + cloudNo, 0);
        else
            grDrawSpriteFlip(grBackgroundSfc, ttmSlot, cloudX, cloudY, 15 + cloudNo, 0);
    }

    grDx = islandState.xPos;
    grDy = islandState.yPos;


    // The island itself

    grDrawSprite(grBackgroundSfc, ttmSlot, 288, 279,  0, 0);      // island
    if (!gIslandSuppressBackdropTrunk)
        grDrawSprite(grBackgroundSfc, ttmSlot, 442, 148, 13, 0);  // trunk
    if (!gIslandSuppressBackdropLeafs)
        grDrawSprite(grBackgroundSfc, ttmSlot, 365, 122, 12, 0);  // leafs
    grDrawSprite(grBackgroundSfc, ttmSlot, 396, 279, 14, 0);      // palmtree's shadow

    if (islandState.lowTide) {
        grDrawSprite(grBackgroundSfc, ttmSlot, 249, 303,  1, 0);  // low tide shore
        grDrawSprite(grBackgroundSfc, ttmSlot, 150, 328,  2, 0);  // rock
    }


    // Initial waves on the shore
    for (int i=0; i < 4; i++)
        islandAnimate(ttmThread);


    // Waves animation thread
    ttmThread->delay = ttmThread->timer = 8;
}


static uint16 islandAdvanceWaveSelection(sint16 *waveX, sint16 *waveY)
{
    uint16 waveSpriteNo = 0;

    if (islandState.lowTide) {

        gIslandWaveCounter2++;
        gIslandWaveCounter2 %= 4;

        switch (gIslandWaveCounter2) {
            case 0: *waveX = 129; *waveY = 340; waveSpriteNo = 39+gIslandWaveCounter1; break;  // rock waves (40)
            case 1: *waveX = 233; *waveY = 323; waveSpriteNo = 30+gIslandWaveCounter1; break;  // low tide waves - left (31)
            case 2: *waveX = 367; *waveY = 356; waveSpriteNo = 33+gIslandWaveCounter1; break;  // low tide waves - center (33)
            case 3: *waveX = 558; *waveY = 323; waveSpriteNo = 36+gIslandWaveCounter1; break;  // low tide waves - right (36)
        }
    }
    else {

        gIslandWaveCounter2++;
        gIslandWaveCounter2 %= 3;

        switch (gIslandWaveCounter2) {
            case 0: *waveX = 270; *waveY = 306; waveSpriteNo = 3+gIslandWaveCounter1; break;  // high tide waves - left (3)
            case 1: *waveX = 364; *waveY = 319; waveSpriteNo = 6+gIslandWaveCounter1; break;  // high tide waves - center (6)
            case 2: *waveX = 518; *waveY = 303; waveSpriteNo = 9+gIslandWaveCounter1; break;  // high tide waves - right (9)
        }
    }

    if (!gIslandWaveCounter2) {
        gIslandWaveCounter1++;
        gIslandWaveCounter1 %= 3;
    }

    return waveSpriteNo;
}

void islandAnimate(struct TTtmThread *ttmThread)
{
    struct TTtmSlot *ttmSlot = ttmThread->ttmSlot;

    sint16 waveX = 0, waveY = 0;
    uint16 waveSpriteNo = 0;

    grDx = islandState.xPos;
    grDy = islandState.yPos;

    waveSpriteNo = islandAdvanceWaveSelection(&waveX, &waveY);

    grDrawSprite(grBackgroundSfc, ttmSlot, waveX, waveY, waveSpriteNo, 0);

    /* Cache this position's sprite (indexed by wave position) so the
     * per-frame redraw can repaint the WHOLE ocean, not just this one. */
    uint16 actualSpriteNo = waveSpriteNo % ttmSlot->numSprites[0];
    int pos = gIslandWaveCounter2;
    if (pos >= 0 && pos < WAVE_CACHE_MAX) {
        gWaveCacheSprite[pos] = ttmSlot->sprites[0][actualSpriteNo];
        gWaveCacheX[pos] = waveX + grDx;
        gWaveCacheY[pos] = waveY + grDy;
    }
    /* Repaint the full foam set this frame (the other positions were
     * wiped by the clean-rect restore). */
    islandRedrawWave(ttmThread);
}


/*
 * Replay last wave sprite on non-firing frames.
 * Same pattern as the thread sprite replay system.
 */
void islandRedrawWave(struct TTtmThread *ttmThread)
{
    (void)ttmThread;
    /* Redraw the WHOLE foam set (all cached positions), not just the last
     * one — the clean-rect restore wiped the foam region this frame. Bound
     * to the current tide's position count (3 high / 4 low) so a stale
     * 4th-position sprite from a prior low-tide scene can't paint a ghost
     * wave after switching to high tide. */
    int n = islandState.lowTide ? 4 : 3;
    if (n > WAVE_CACHE_MAX) n = WAVE_CACHE_MAX;
    for (int i = 0; i < n; i++) {
        if (gWaveCacheSprite[i])
            grCompositeToBackground(gWaveCacheSprite[i], gWaveCacheX[i], gWaveCacheY[i]);
    }
}


/*
 * Clear cached wave sprite pointer.
 * Must be called before freeing the background slot's BMP sprites
 * (e.g., in adsReleaseIsland) to prevent dangling pointer use.
 */
void islandClearWaveCache(void)
{
    for (int i = 0; i < WAVE_CACHE_MAX; i++)
        gWaveCacheSprite[i] = NULL;
}


void islandInitHoliday(struct TTtmThread *ttmThread)
{
    struct TTtmSlot *ttmSlot = ttmThread->ttmSlot;
    const struct Holiday *holiday = holidayById(islandState.holiday);

    if (holiday) {

        ttmThread->ttmLayer  = grNewLayer();
        ttmThread->isRunning = 3;

        grDx = islandState.xPos;
        grDy = islandState.yPos;

        grLoadBmp(ttmSlot, 0, "HOLIDAY.BMP");
        grDrawSprite(ttmThread->ttmLayer, ttmSlot,
                     holiday->island_x, holiday->island_y,
                     (uint16)holiday->sprite_index, 0);

        grReleaseBmp(ttmSlot,0);
    }
    else {
        ttmThread->isRunning = 0;
    }
}

void islandSetSuppressBackdropTrunk(int suppress)
{
    gIslandSuppressBackdropTrunk = suppress ? 1 : 0;
}


void islandSetSuppressBackdropLeafs(int suppress)
{
    gIslandSuppressBackdropLeafs = suppress ? 1 : 0;
}
