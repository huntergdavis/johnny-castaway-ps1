/*
 *  This file is part of 'Johnny Reborn' - PS1 Port
 *
 *  PlayStation 1 graphics implementation using PSn00bSDK
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <psxapi.h>

#include "mytypes.h"

#include "utils.h"
#include "graphics_ps1.h"
#include "ads.h"
#include "mem_region.h"
#include "foreground_pilot.h"
#include "ps1_perf.h"
#include "resource.h"
#include "events_ps1.h"
#include "cdrom_ps1.h"
#include "psb_format.h"
#include "psb_registry.h"
#include "ps1_gpu_ot.h"
#include "ps1_captions.h"

#ifndef GRAPHICS_PS1_DIAG_LOGS
#define GRAPHICS_PS1_DIAG_LOGS 0
#endif

#if GRAPHICS_PS1_DIAG_LOGS
#define GR_DIAG_PRINTF(...) do { if (debugMode) printf(__VA_ARGS__); } while (0)
#else
#define GR_DIAG_PRINTF(...) do { } while (0)
#endif

/* Primitive buffer for GPU commands */
#define PRIMITIVE_BUFFER_SIZE 32768
uint8 *primitiveBuffer[2];  /* Malloc'd, not static array! */
uint32 primitiveIndex[2];
uint8 *nextPrimitive[2];

/* PS1 Display and drawing environments */
static DISPENV disp[2];
DRAWENV draw[2];
int db = 0;  /* Double buffer index */

/* Per-scene-frame counter, used by pause_menu.c for uptime display. */
uint32 ps1FrameCount = 0;

/* Ordering tables for GPU command queueing */
#define OT_LENGTH 8
unsigned long ot[2][OT_LENGTH];

/* Palette (16 colors, matching original TTM format) - exported for jc_reborn.c */
uint16 ttmPalette[16];

/* Layer management */
static PS1Surface *grSavedZonesLayer = NULL;
PS1Surface *grBackgroundSfc = NULL;

/* Background tiles for pixel-perfect 640x480 rendering
 * Top row: 3 tiles (256+256+128 = 640 pixels wide, 240 tall)
 * Bottom row will be added later */
#define BG_TILE_HEIGHT 240
#define SCR_STREAM_ROWS 64
#define SCR_STREAM_ROW_BYTES 320  /* 640 packed 4bpp pixels */
/* Top row tiles (stored in VRAM texture area) - exported for dirty rectangle wiping */
PS1Surface *bgTile0 = NULL;  /* x=0-255,   srcX=0 */
PS1Surface *bgTile1 = NULL;  /* x=256-511, srcX=256 */
static PS1Surface *bgTile2a = NULL; /* x=512-575, srcX=512, width=64 */
static PS1Surface *bgTile2b = NULL; /* x=576-639, srcX=576, width=64 */

/* Bottom row tiles - exported for dirty rectangle wiping */
PS1Surface *bgTile3 = NULL;  /* y=240, x=0-255 */
PS1Surface *bgTile4 = NULL;  /* y=240, x=256-511 */
static PS1Surface *bgTile5a = NULL; /* y=240, x=512-575 */
static PS1Surface *bgTile5b = NULL; /* y=240, x=576-637 */

/* Clean copies of background tiles for composite pattern.
 * Each frame: restore from clean → composite sprites → upload to framebuffer.
 * This avoids multiple LoadImage calls per frame (DMA conflict issue). */
static uint16 *bgTile0Clean = NULL;
static uint16 *bgTile1Clean = NULL;
static uint16 *bgTile3Clean = NULL;
static uint16 *bgTile4Clean = NULL;
static int grSaveCleanOnScreenLoad = 1;
static uint8 gScrStreamRows[SCR_STREAM_ROWS * SCR_STREAM_ROW_BYTES];
static int gFullScreenScrCacheEnabled = 0;
static uint8 *gFullScreenScrCache = NULL;
static uint32 gFullScreenScrCacheBytes = 0;
static int gFullScreenScrCacheValid = 0;
static char gFullScreenScrCacheName[16] = "";

/* Dirty-rect tracking: per-tile row-granularity restore/upload.
 * Index: 0=bgTile0, 1=bgTile1, 2=bgTile3, 3=bgTile4.
 * -1 means clean (no rows modified). */
static int currDirtyMinY[4] = {-1, -1, -1, -1};
static int currDirtyMaxY[4] = {-1, -1, -1, -1};
static int currDirtyMinX[4] = {-1, -1, -1, -1};
static int currDirtyMaxX[4] = {-1, -1, -1, -1};
static int prevDirtyMinY[4] = {-1, -1, -1, -1};
static int prevDirtyMaxY[4] = {-1, -1, -1, -1};
static int prevDirtyMinX[4] = {-1, -1, -1, -1};
static int prevDirtyMaxX[4] = {-1, -1, -1, -1};
static sint16 currDirtyRowMinX[4][BG_TILE_HEIGHT];
static sint16 currDirtyRowMaxX[4][BG_TILE_HEIGHT];
static sint16 prevDirtyRowMinX[4][BG_TILE_HEIGHT];
static sint16 prevDirtyRowMaxX[4][BG_TILE_HEIGHT];
static int dirtyRowStateInitialized = 0;

/* Byte-pair palette lookup tables (256 entries × 4 bytes = 1KB each).
 * Each entry packs two resolved 16-bit colors for a packed byte:
 *   low16 = even pixel color, high16 = odd pixel color.
 * palLutSierra: high nibble = even (Sierra BMP format).
 * palLutPsb:    low nibble = even (PSB pre-transcoded format). */
static uint32 palLutSierra[256];
static uint32 palLutPsb[256];
static int grPresentDuringScreenLoad = 1;

static void grDrawRectColor15(sint16 x, sint16 y, uint16 width, uint16 height, uint16 bgColor);
static void grRestoreRectFromCleanBg(int x, int y, int width, int height);
static void grCommitRectToCleanBg(int x, int y, int width, int height);
static void freeBgTile(PS1Surface **tile);
static uint32 grRestoreCleanBgSpanFromRects(int x, int y, int width);

void grSetPresentDuringScreenLoad(int enabled)
{
    grPresentDuringScreenLoad = enabled ? 1 : 0;
}

static void grClearDirtyRows(sint16 rowsMinX[4][BG_TILE_HEIGHT],
                             sint16 rowsMaxX[4][BG_TILE_HEIGHT])
{
    for (int i = 0; i < 4; i++) {
        for (int y = 0; y < BG_TILE_HEIGHT; y++) {
            rowsMinX[i][y] = -1;
            rowsMaxX[i][y] = -1;
        }
    }
}

static void grEnsureDirtyRowState(void)
{
    if (dirtyRowStateInitialized)
        return;

    grClearDirtyRows(currDirtyRowMinX, currDirtyRowMaxX);
    grClearDirtyRows(prevDirtyRowMinX, prevDirtyRowMaxX);
    dirtyRowStateInitialized = 1;
}

static void grClearDirtyRowRange(sint16 rowsMinX[4][BG_TILE_HEIGHT],
                                 sint16 rowsMaxX[4][BG_TILE_HEIGHT],
                                 int idx,
                                 int minY,
                                 int maxY)
{
    if (idx < 0 || idx >= 4)
        return;
    if (minY < 0)
        minY = 0;
    if (maxY >= BG_TILE_HEIGHT)
        maxY = BG_TILE_HEIGHT - 1;
    if (minY > maxY)
        return;

    for (int y = minY; y <= maxY; y++) {
        rowsMinX[idx][y] = -1;
        rowsMaxX[idx][y] = -1;
    }
}

static void grClearCurrDirtyState(void)
{
    grEnsureDirtyRowState();
    for (int i = 0; i < 4; i++) {
        grClearDirtyRowRange(currDirtyRowMinX, currDirtyRowMaxX,
                             i, currDirtyMinY[i], currDirtyMaxY[i]);
        currDirtyMinX[i] = -1;
        currDirtyMaxX[i] = -1;
        currDirtyMinY[i] = -1;
        currDirtyMaxY[i] = -1;
    }
}

static void grPromoteCurrDirtyToPrev(void)
{
    grEnsureDirtyRowState();
    for (int i = 0; i < 4; i++) {
        int minY;
        int maxY;

        grClearDirtyRowRange(prevDirtyRowMinX, prevDirtyRowMaxX,
                             i, prevDirtyMinY[i], prevDirtyMaxY[i]);

        prevDirtyMinX[i] = currDirtyMinX[i];
        prevDirtyMaxX[i] = currDirtyMaxX[i];
        prevDirtyMinY[i] = currDirtyMinY[i];
        prevDirtyMaxY[i] = currDirtyMaxY[i];

        minY = currDirtyMinY[i];
        maxY = currDirtyMaxY[i];
        if (minY < 0)
            continue;
        if (maxY >= BG_TILE_HEIGHT)
            maxY = BG_TILE_HEIGHT - 1;
        for (int y = minY; y <= maxY; y++) {
            prevDirtyRowMinX[i][y] = currDirtyRowMinX[i][y];
            prevDirtyRowMaxX[i][y] = currDirtyRowMaxX[i][y];
        }
    }
}

static void grClearPrevDirtyState(void)
{
    grEnsureDirtyRowState();
    for (int i = 0; i < 4; i++) {
        grClearDirtyRowRange(prevDirtyRowMinX, prevDirtyRowMaxX,
                             i, prevDirtyMinY[i], prevDirtyMaxY[i]);
        prevDirtyMinX[i] = -1;
        prevDirtyMaxX[i] = -1;
        prevDirtyMinY[i] = -1;
        prevDirtyMaxY[i] = -1;
    }
}

static void grMarkDirtyRows(sint16 rowsMinX[4][BG_TILE_HEIGHT],
                            sint16 rowsMaxX[4][BG_TILE_HEIGHT],
                            int idx,
                            int minX,
                            int maxX,
                            int minY,
                            int maxY)
{
    if (idx < 0 || idx >= 4)
        return;
    if (minX < 0) minX = 0;
    if (maxX > 319) maxX = 319;
    if (minY < 0) minY = 0;
    if (maxY > 239) maxY = 239;
    if (minX > maxX || minY > maxY)
        return;

    grEnsureDirtyRowState();
    for (int y = minY; y <= maxY; y++) {
        if (rowsMinX[idx][y] < 0) {
            rowsMinX[idx][y] = (sint16)minX;
            rowsMaxX[idx][y] = (sint16)maxX;
        } else {
            if (minX < rowsMinX[idx][y]) rowsMinX[idx][y] = (sint16)minX;
            if (maxX > rowsMaxX[idx][y]) rowsMaxX[idx][y] = (sint16)maxX;
        }
    }
}

static void grMarkPrevTileDirtyRect(int idx, int minX, int maxX, int minY, int maxY)
{
    if (idx < 0 || idx >= 4)
        return;
    if (minX < 0) minX = 0;
    if (maxX > 319) maxX = 319;
    if (minY < 0) minY = 0;
    if (maxY > 239) maxY = 239;
    if (minX > maxX || minY > maxY)
        return;

    prevDirtyMinX[idx] = minX;
    prevDirtyMaxX[idx] = maxX;
    prevDirtyMinY[idx] = minY;
    prevDirtyMaxY[idx] = maxY;
    grMarkDirtyRows(prevDirtyRowMinX, prevDirtyRowMaxX, idx, minX, maxX, minY, maxY);
}

static void grMarkPrevAllTilesDirty(void)
{
    grEnsureDirtyRowState();
    grClearDirtyRows(prevDirtyRowMinX, prevDirtyRowMaxX);
    for (int i = 0; i < 4; i++)
        grMarkPrevTileDirtyRect(i, 0, 319, 0, 239);
}

static inline void markTileDirtyRect(int idx, int minX, int maxX, int minY, int maxY)
{
    if (idx < 0 || idx >= 4)
        return;
    if (minX < 0) minX = 0;
    if (maxX > 319) maxX = 319;
    if (minY < 0) minY = 0;
    if (maxY > 239) maxY = 239;
    if (minX > maxX || minY > maxY)
        return;

    if (currDirtyMinY[idx] < 0) {
        currDirtyMinX[idx] = minX;
        currDirtyMaxX[idx] = maxX;
        currDirtyMinY[idx] = minY;
        currDirtyMaxY[idx] = maxY;
    } else {
        if (minX < currDirtyMinX[idx]) currDirtyMinX[idx] = minX;
        if (maxX > currDirtyMaxX[idx]) currDirtyMaxX[idx] = maxX;
        if (minY < currDirtyMinY[idx]) currDirtyMinY[idx] = minY;
        if (maxY > currDirtyMaxY[idx]) currDirtyMaxY[idx] = maxY;
    }
    grMarkDirtyRows(currDirtyRowMinX, currDirtyRowMaxX, idx, minX, maxX, minY, maxY);
}

static inline void markTileDirty(int idx, int minY, int maxY)
{
    markTileDirtyRect(idx, 0, 319, minY, maxY);
}

static inline void grMarkSingleColumnDirty(int tileBaseX,
                                           int x,
                                           int width,
                                           int y0,
                                           int y1Exclusive)
{
    int idxBase = (tileBaseX == 0) ? 0 : 1;
    int minX = x - tileBaseX;
    int maxX = minX + width - 1;

    if (width <= 0)
        return;

    if (y0 < 240) {
        int minY = y0;
        int maxY = (y1Exclusive < 240) ? (y1Exclusive - 1) : 239;
        markTileDirtyRect(idxBase, minX, maxX, minY, maxY);
    }

    if (y1Exclusive > 240) {
        int minY = (y0 > 240) ? (y0 - 240) : 0;
        int maxY = y1Exclusive - 241;
        if (maxY > 239)
            maxY = 239;
        markTileDirtyRect(idxBase + 2, minX, maxX, minY, maxY);
    }
}

void grMarkAllTilesDirty(void)
{
    grEnsureDirtyRowState();
    grClearDirtyRows(currDirtyRowMinX, currDirtyRowMaxX);
    for (int i = 0; i < 4; i++) {
        currDirtyMinX[i] = 0;
        currDirtyMaxX[i] = 319;
        currDirtyMinY[i] = 0;
        currDirtyMaxY[i] = 239;
        grMarkDirtyRows(currDirtyRowMinX, currDirtyRowMaxX, i, 0, 319, 0, 239);
    }
}

/* Force the next grDrawBackground call to upload all 4 tiles in full.
 * Sets BOTH currDirty and prevDirty because grRestoreBgTiles resets
 * currDirty at the start of every frame. prevDirty survives that
 * reset and feeds into grDrawBackground's union(prev, curr) upload
 * range. Mirrors the pattern in grFadeOut / grFreeCleanBgTiles. */
void grForceFullRedrawNextFrame(void)
{
    grEnsureDirtyRowState();
    grClearDirtyRows(currDirtyRowMinX, currDirtyRowMaxX);
    grClearDirtyRows(prevDirtyRowMinX, prevDirtyRowMaxX);
    for (int i = 0; i < 4; i++) {
        currDirtyMinX[i] = 0;
        currDirtyMaxX[i] = 319;
        currDirtyMinY[i] = 0;
        currDirtyMaxY[i] = 239;
        prevDirtyMinX[i] = 0;
        prevDirtyMaxX[i] = 319;
        prevDirtyMinY[i] = 0;
        prevDirtyMaxY[i] = 239;
        grMarkDirtyRows(currDirtyRowMinX, currDirtyRowMaxX, i, 0, 319, 0, 239);
        grMarkDirtyRows(prevDirtyRowMinX, prevDirtyRowMaxX, i, 0, 319, 0, 239);
    }
}

/* Mark dirty region from a screen-space rectangle (x0,y0)-(x1,y1) exclusive */
static void grMarkRectDirty(int x0, int y0, int x1, int y1)
{
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > 640) x1 = 640;
    if (y1 > 480) y1 = 480;
    if (x0 >= x1 || y0 >= y1) return;

    /* Top row tiles (screen y 0-239) */
    if (y0 < 240) {
        int ty0 = y0;
        int ty1 = (y1 < 240) ? y1 - 1 : 239;
        if (x0 < 320) {
            int tx1 = (x1 < 320) ? x1 - 1 : 319;
            markTileDirtyRect(0, x0, tx1, ty0, ty1);
        }
        if (x1 > 320) {
            int tx0 = (x0 > 320) ? x0 - 320 : 0;
            int tx1 = x1 - 321;
            markTileDirtyRect(1, tx0, tx1, ty0, ty1);
        }
    }
    /* Bottom row tiles (screen y 240-479) */
    if (y1 > 240) {
        int ty0 = (y0 > 240) ? y0 - 240 : 0;
        int ty1 = y1 - 240 - 1;
        if (ty1 > 239) ty1 = 239;
        if (x0 < 320) {
            int tx1 = (x1 < 320) ? x1 - 1 : 319;
            markTileDirtyRect(2, x0, tx1, ty0, ty1);
        }
        if (x1 > 320) {
            int tx0 = (x0 > 320) ? x0 - 320 : 0;
            int tx1 = x1 - 321;
            markTileDirtyRect(3, tx0, tx1, ty0, ty1);
        }
    }
}

void grMarkScreenRectDirty(int x0, int y0, int x1, int y1)
{
    grMarkRectDirty(x0, y0, x1, y1);
}

static void grRebuildPaletteLuts(void)
{
    for (int i = 0; i < 256; i++) {
        /* Sierra: high nibble = even pixel, low nibble = odd pixel */
        uint16 pe = ttmPalette[(i >> 4) & 0x0F];
        uint16 po = ttmPalette[i & 0x0F];
        palLutSierra[i] = (uint32)pe | ((uint32)po << 16);
        /* PSB: low nibble = even pixel, high nibble = odd pixel */
        pe = ttmPalette[i & 0x0F];
        po = ttmPalette[(i >> 4) & 0x0F];
        palLutPsb[i] = (uint32)pe | ((uint32)po << 16);
    }
}

struct TPs1SavedZone {
    uint16 x;
    uint16 y;
    uint16 width;
    uint16 height;
    uint8 valid;
};

static struct TPs1SavedZone grPs1SavedZone = {0, 0, 0, 0, 0};

/* Global variables matching original implementation */
int grDx = 0;
int grDy = 0;
int grWindowed = 0;  /* PS1 is always fullscreen, but keep for compatibility */
int grUpdateDelay = 0;

/* Frame capture - stubbed for PS1 */
int grCaptureFrameNumber = -1;
int grCaptureForegroundOnly = 0;
char *grCaptureFilename = NULL;
char *grCaptureDir = NULL;
char *grCaptureMetaDir = NULL;
int grCaptureInterval = 0;
int grCaptureStartFrame = 0;
int grCaptureEndFrame = -1;
int grCaptureOverlay = 0;
int grCaptureOverlayMaskOnly = 0;
int grCaptureForegroundIncludeStaticBase = 0;
int grCaptureForegroundSkipVisibilityMask = 0;
char *grCaptureSoundEventsPath = NULL;

/* Flag to track if GPU was already initialized (e.g., by loadTitleScreenEarly) */
int grGpuAlreadyInitialized = 0;

/* Current thread being played - used to record sprite draws for replay */
struct TTtmThread *grCurrentThread = NULL;
int grPs1TelemetryEnabled = 0;

/* Persistent debug counters for sprite/frame clipping diagnostics. */
static uint32 gStatThreadDrops = 0;
static uint32 gStatBmpFrameCapHits = 0;
static uint32 gStatBmpShortLoads = 0;
static uint16 gStatBmpMaxRequested = 0;
static uint16 gStatBmpMinLoaded = 0xFFFF;
static uint16 gStatLastBmpSlot = 0;
static uint16 gStatLastBmpFrames = 0;
static uint16 gStatLastBmpStatus = 0;

/* Story transition diagnostics from story.c */
extern uint16 ps1StoryDbgPhase;
extern uint16 ps1StoryDbgSceneTag;
extern uint16 ps1StoryDbgAdsSig;
extern uint16 ps1StoryDbgPrevSpot;
extern uint16 ps1StoryDbgPrevHdg;
extern uint16 ps1StoryDbgNextSpot;
extern uint16 ps1StoryDbgNextHdg;
extern uint16 ps1StoryDbgSeq;
extern uint16 ps1AdsDbgActiveThreads;
extern uint16 ps1AdsDbgMini;
extern uint16 ps1AdsDbgRunningThreads;
extern uint16 ps1AdsDbgTerminatedThreads;
extern uint16 ps1AdsDbgSceneSlot;
extern uint16 ps1AdsDbgSceneTag;
extern uint16 ps1AdsDbgReplayCount;
extern uint16 ps1AdsDbgReplayTryFrame;
extern uint16 ps1AdsDbgReplayDrawFrame;
extern uint16 ps1AdsDbgMergeCarryFrame;
extern uint16 ps1AdsDbgNoDrawThreadsFrame;
extern uint16 ps1AdsDbgPlayedThreadsFrame;
extern uint16 ps1AdsDbgRecordedSpritesFrame;
extern uint16 ps1AdsDbgLastStopThread;
extern uint16 ps1AdsDbgLastStopSceneSig;
extern uint16 ps1AdsDbgLastReapThread;
extern uint16 ps1AdsDbgLastReapSceneSig;
extern uint16 ps1AdsDbgLastAddThread;
extern uint16 ps1AdsDbgLastAddSceneSig;
extern uint16 ps1PilotDbgActivePack;
extern uint16 ps1PilotDbgHits;
extern uint16 ps1PilotDbgFallbacks;
extern uint16 ps1PilotDbgLastHitEntry;
extern uint16 ps1PilotDbgLastFallbackEntry;
extern uint16 ps1PilotDbgFallbackWhilePackActive;

void grPs1StatThreadDrop(void)
{
    if (gStatThreadDrops < 0xFFFFFFFFU) gStatThreadDrops++;
}

void grPs1StatBmpFrameCap(uint16 requested, uint16 cap)
{
    (void)cap;
    if (gStatBmpFrameCapHits < 0xFFFFFFFFU) gStatBmpFrameCapHits++;
    if (requested > gStatBmpMaxRequested) gStatBmpMaxRequested = requested;
}

void grPs1StatBmpShortLoad(uint16 requested, uint16 loaded)
{
    if (gStatBmpShortLoads < 0xFFFFFFFFU) gStatBmpShortLoads++;
    if (requested > gStatBmpMaxRequested) gStatBmpMaxRequested = requested;
    if (loaded < gStatBmpMinLoaded) gStatBmpMinLoaded = loaded;
}

void grPs1SetLastBmpTelemetry(uint16 slot, uint16 frames, uint16 status)
{
    gStatLastBmpSlot = slot;
    gStatLastBmpFrames = frames;
    gStatLastBmpStatus = status;
}

/* VRAM allocation tracking
 * VRAM Layout for 640x480 interlaced:
 * (0,0)-(639,479): Framebuffer (single buffer for 640x480)
 * (640,0)-(656,1): CLUT (16 colors)
 * (640,2)-(895,2): CLUT 256 (grayscale)
 * (640,4) onwards: Textures
 */
static uint16 nextVRAMX = 640;  /* Start to the right of framebuffer */
static uint16 nextVRAMY = 4;    /* Below CLUTs */

static void grResetVramCursor(void)
{
    nextVRAMX = 640;
    nextVRAMY = 4;
}


/*
 * Initialize PS1 graphics subsystem
 */
void graphicsInit()
{
    /* Skip GPU reset if already initialized (e.g., by loadTitleScreenEarly)
     * CRITICAL: Calling ResetGraph(0) after GPU is already initialized
     * conflicts with the existing GPU state and causes hangs */
    if (!grGpuAlreadyInitialized) {
        GR_DIAG_PRINTF("GPU: Resetting GPU...\n");

        /* Reset GPU and set video mode */
        ResetGraph(0);
        SetVideoMode(MODE_NTSC);

        GR_DIAG_PRINTF("GPU: Initializing GTE...\n");

        /* Initialize geometry transformation engine */
        InitGeom();

        GR_DIAG_PRINTF("GPU: Setting up display buffers (%dx%d)...\n", SCREEN_WIDTH, SCREEN_HEIGHT);

        /* Setup display environments for 640x480 interlaced mode
         * Single buffer mode since 2x640x480 won't fit in VRAM
         * Both buffers point to same location - no flipping needed */
        SetDefDispEnv(&disp[0], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        SetDefDispEnv(&disp[1], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        /* Enable interlaced mode for 640x480 */
        disp[0].isinter = 1;
        disp[1].isinter = 1;

        /* Setup drawing environments - both draw to same buffer */
        SetDefDrawEnv(&draw[0], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        SetDefDrawEnv(&draw[1], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        /* Set background clear color - but DON'T enable clear!
         * We use grDrawBackground() LoadImage to paint the background each frame,
         * so isbg=0 preserves it for OT primitives to draw on top */
        setRGB0(&draw[0], 0, 0, 0);
        setRGB0(&draw[1], 0, 0, 0);
        draw[0].isbg = 0;  /* Don't clear - grDrawBackground handles it */
        draw[1].isbg = 0;

        GR_DIAG_PRINTF("GPU: Enabling display...\n");

        /* Enable display */
        SetDispMask(1);

        /* Apply first buffer */
        PutDispEnv(&disp[db]);
        PutDrawEnv(&draw[db]);
    } else {
        GR_DIAG_PRINTF("GPU: Skipping ResetGraph - already initialized\n");

        /* Still need to setup display/draw environments for rendering
         * GPU is already on, but we need proper environment structs */
        SetDefDispEnv(&disp[0], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        SetDefDispEnv(&disp[1], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        disp[0].isinter = 1;  /* Interlaced mode for 640x480 */
        disp[1].isinter = 1;

        SetDefDrawEnv(&draw[0], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        SetDefDrawEnv(&draw[1], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        setRGB0(&draw[0], 0, 0, 0);
        setRGB0(&draw[1], 0, 0, 0);
        draw[0].isbg = 0;  /* Don't clear - grDrawBackground handles it */
        draw[1].isbg = 0;

        /* Apply environments immediately (like the non-ELSE branch) */
        PutDispEnv(&disp[db]);
        PutDrawEnv(&draw[db]);
    }

    GR_DIAG_PRINTF("GPU: Initializing ordering tables...\n");

    /* Clear ordering tables */
    ClearOTagR(ot[0], OT_LENGTH);
    ClearOTagR(ot[1], OT_LENGTH);

    /* Reverted to libc — 2x32 KB exceeds the reduced BOOT budget. */
    primitiveBuffer[0] = (uint8*)malloc(PRIMITIVE_BUFFER_SIZE);
    primitiveBuffer[1] = (uint8*)malloc(PRIMITIVE_BUFFER_SIZE);
    if (!primitiveBuffer[0] || !primitiveBuffer[1]) {
        printf("ERROR: Failed to allocate primitive buffers\n");
        while(1);
    }

    /* Initialize primitive buffers */
    nextPrimitive[0] = primitiveBuffer[0];
    nextPrimitive[1] = primitiveBuffer[1];
    primitiveIndex[0] = 0;
    primitiveIndex[1] = 0;

    GR_DIAG_PRINTF("GPU: Loading default palette...\n");

    /* Load default palette with distinct, bright colors for testing
     * PS1 uses BGR555 format: (B << 10) | (G << 5) | R
     * Each channel is 5 bits (0-31) */
    ttmPalette[0]  = (0 << 10)  | (0 << 5)  | 0;   /* 0: Black */
    ttmPalette[1]  = (0 << 10)  | (0 << 5)  | 31;  /* 1: Red */
    ttmPalette[2]  = (0 << 10)  | (31 << 5) | 0;   /* 2: Green */
    ttmPalette[3]  = (31 << 10) | (0 << 5)  | 0;   /* 3: Blue */
    ttmPalette[4]  = (0 << 10)  | (31 << 5) | 31;  /* 4: Yellow */
    ttmPalette[5]  = (31 << 10) | (0 << 5)  | 31;  /* 5: Magenta */
    ttmPalette[6]  = (31 << 10) | (31 << 5) | 0;   /* 6: Cyan */
    ttmPalette[7]  = (31 << 10) | (31 << 5) | 31;  /* 7: White */
    ttmPalette[8]  = (16 << 10) | (16 << 5) | 16;  /* 8: Gray */
    ttmPalette[9]  = (0 << 10)  | (0 << 5)  | 20;  /* 9: Dark Red */
    ttmPalette[10] = (0 << 10)  | (20 << 5) | 0;   /* 10: Dark Green */
    ttmPalette[11] = (20 << 10) | (0 << 5)  | 0;   /* 11: Dark Blue */
    ttmPalette[12] = (0 << 10)  | (20 << 5) | 20;  /* 12: Orange */
    ttmPalette[13] = (20 << 10) | (0 << 5)  | 20;  /* 13: Purple */
    ttmPalette[14] = (20 << 10) | (20 << 5) | 0;   /* 14: Teal */
    ttmPalette[15] = (20 << 10) | (20 << 5) | 20;  /* 15: Light Gray */

    /* Upload 16-color CLUT for primitives at (640, 0) - right of framebuffer */
    RECT clutRect16;
    setRECT(&clutRect16, 640, 0, 16, 1);  /* 16 colors, 1 row */
    LoadImage(&clutRect16, (uint32*)ttmPalette);

    /* Create and upload 256-color grayscale CLUT for SCR textures at (640, 2) */
    static uint16 clut256[256];
    for (int i = 0; i < 256; i++) {
        /* Convert 8-bit grayscale to BGR555 */
        uint8 val = (i >> 3) & 0x1F;  /* Scale 0-255 to 0-31 */
        clut256[i] = (val << 10) | (val << 5) | val;  /* Grayscale */
    }
    RECT clutRect256;
    setRECT(&clutRect256, 640, 2, 256, 1);  /* 256 colors, 1 row */
    LoadImage(&clutRect256, (uint32*)clut256);

    GR_DIAG_PRINTF("GPU: Initializing event system...\n");

    /* Initialize event system */
    eventsInit();
    GR_DIAG_PRINTF("GPU: Graphics initialization complete!\n");

    /* Mark graphics ready — used by memHalt to decide between the
     * full BSOD panel (ps1Bsod) and the minimal pre-graphics text
     * panel (ps1DebugError). See plan v9 "Failure UX". */
    extern void memSetGraphicsReady(int ready);
    memSetGraphicsReady(1);
}

/* Mem-region allocator queries this to choose its halt path. Defined
 * here so the flag lives next to graphicsInit; the allocator uses
 * `extern int graphicsIsInitialized(void)` via mem_region.c. */
static int gGraphicsReady = 0;

int graphicsIsInitialized(void) {
    return gGraphicsReady;
}

void memSetGraphicsReady(int ready) {
    gGraphicsReady = ready ? 1 : 0;
}

/*
 * Shutdown graphics subsystem
 */
void graphicsEnd()
{
    /* Free allocated surfaces */
    if (grBackgroundSfc != NULL) {
        grFreeLayer(grBackgroundSfc);
        grBackgroundSfc = NULL;
    }

    if (grSavedZonesLayer != NULL) {
        grFreeLayer(grSavedZonesLayer);
        grSavedZonesLayer = NULL;
    }

    SetDispMask(0);
}

/*
 * Load palette from resource
 */
void grLoadPalette(struct TPalResource *palResource)
{
    if (palResource == NULL) {
        fatalError("NULL palette\n");
    }

    /* Convert VGA 6-bit RGB to PS1 15-bit RGB (5-5-5)
     * Magenta (0xa8, 0, 0xa8) = VGA 6-bit (42, 0, 42) is the transparent color.
     * Match SDL version: only EXACT magenta (168, 0, 168 in 8-bit) is transparent. */
    for (int i = 0; i < 16; i++) {
        uint8 vgaR = palResource->colors[i].r;  /* 6-bit VGA values (0-63) */
        uint8 vgaG = palResource->colors[i].g;
        uint8 vgaB = palResource->colors[i].b;

        /* Convert to 8-bit like SDL version */
        uint8 r8 = vgaR << 2;
        uint8 g8 = vgaG << 2;
        uint8 b8 = vgaB << 2;

        /* Check for EXACT magenta (0xa8=168, 0, 0xa8=168) only - matches SDL behavior */
        int isMagenta = (r8 == 0xa8) && (g8 == 0) && (b8 == 0xa8);

        if (isMagenta) {
            /* Magenta = 0x0000 = fully transparent on PS1 */
            ttmPalette[i] = 0x0000;
        } else {
            uint8 r = r8 >> 3;  /* 8-bit to 5-bit */
            uint8 g = g8 >> 3;
            uint8 b = b8 >> 3;
            uint16 color = (b << 10) | (g << 5) | r;

            /* IMPORTANT: If color is 0x0000 (black), use 0x0001 instead
             * because 0x0000 is reserved for transparency on PS1 */
            if (color == 0x0000) {
                color = 0x0001;  /* Very dark blue, nearly black */
            }
            ttmPalette[i] = color;
        }
    }

    /* Upload CLUT (Color Lookup Table) to VRAM */
    RECT clutRect;
    setRECT(&clutRect, 640, 0, 16, 1);  /* 16 colors, 1 row, at (640, 0) */
    LoadImage(&clutRect, (uint32*)ttmPalette);

    /* Rebuild byte-pair LUTs for compositing span functions */
    grRebuildPaletteLuts();
}

/*
 * Swap display buffers
 */
void grRefreshDisplay()
{
    /* Wait for GPU DMA to finish (LoadImage operations) */
    DrawSync(0);

    /* VSync is called in grUpdateDisplay before LoadImage to prevent tearing.
     * This function just ensures DMA is complete for standalone use cases. */
}

/*
 * Toggle fullscreen (no-op on PS1, always fullscreen)
 */
void grToggleFullScreen()
{
    /* PS1 is always fullscreen */
    grWindowed = !grWindowed;  /* Keep variable for compatibility */
}

/*
 * Per-frame initialisation for the remaining primitive-buffer path.
 */
void grBeginFrame(void)
{
    /* Reset OT and primitive buffer each frame.
     * Required because VRAM-based sprites (from grLoadBmp) still emit
     * GPU primitives into the OT — without reset the buffer overflows. */
    ClearOTagR(ot[0], OT_LENGTH);
    nextPrimitive[0] = primitiveBuffer[0];
    primitiveIndex[0] = 0;
}

/*
 * Replay a previously-drawn sprite via GPU upload.
 * Falls back to software composite if GPU path fails.
 */
void grReplaySprite(struct TDrawnSprite *ds)
{
    if (!ds || !ds->indexedPixels) return;

    PS1Surface tmpSfc = {0};
    tmpSfc.indexedPixels = ds->indexedPixels;
    tmpSfc.width = ds->width;
    tmpSfc.height = ds->height;
    tmpSfc.psbNibbles = ds->psbNibbles;
    if (ds->flip)
        grCompositeToBackgroundFlip(&tmpSfc, ds->x, ds->y);
    else
        grCompositeToBackground(&tmpSfc, ds->x, ds->y);
}

/*
 * Update display with all layers
 */
__attribute__((optimize("Os")))
void grUpdateDisplay(struct TTtmThread *ttmBackgroundThread,
                     struct TTtmThread *ttmThreads,
                     struct TTtmThread *ttmHolidayThread)
{
    int perfDetail = ps1PerfEnabled ? ps1PerfDetailEnabled() : 0;
    uint32 perfTick = 0;

    /* PS1 uses RAM-based compositing - sprites are drawn to bgTile buffers
     * via grCompositeToBackground(). By this point:
     * - grRestoreBgTiles was called before ttmPlay (at frame start)
     * - ttmPlay drew sprites to bgTile via grCompositeToBackground
     * Now we just need to upload and display.
     */

    ps1FrameCount++;

    /* Wait for VSync BEFORE uploading to framebuffer.
     * This ensures we write during vertical blank when display isn't scanning. */
    if (perfDetail)
        perfTick = ps1PerfTick();
    VSync(0);
    if (perfDetail)
        ps1PerfMarkRenderPhase(PS1_PERF_RENDER_PRESENT_WAIT,
                               ps1PerfElapsedVBlanks(perfTick));

    if (perfDetail)
        perfTick = ps1PerfTick();
    if (foregroundPilotRuntimeActive())
        foregroundPilotRuntimeCompose();
    if (perfDetail)
        ps1PerfMarkRenderPhase(PS1_PERF_RENDER_COMPOSE,
                               ps1PerfElapsedVBlanks(perfTick));

    /* Upload background tiles (with sprites composited in software) to framebuffer */
    if (perfDetail)
        perfTick = ps1PerfTick();
    grDrawBackground();
    if (perfDetail)
        ps1PerfMarkRenderPhase(PS1_PERF_RENDER_UPLOAD,
                               ps1PerfElapsedVBlanks(perfTick));

    /* Closed captions overlay — drawn AFTER the scene LoadImage so the
     * dark band + text land on top of the frame the user sees this
     * VSync. No-op when captions are off or no text is queued. */
    if (ps1CaptionsEnabled)
        captionsRender();

    /* Handle frame timing */
    if (perfDetail)
        perfTick = ps1PerfTick();
    eventsWaitTick(grUpdateDelay);
    if (perfDetail)
        ps1PerfMarkRenderPhase(PS1_PERF_RENDER_EVENT_WAIT,
                               ps1PerfElapsedVBlanks(perfTick));

}

/*
 * Create a new empty background surface
 */
PS1Surface *grNewEmptyBackground()
{
    /* Kept on libc malloc — this is called from island.c at scene
     * runtime (via grNewLayer for TTM threads), not strictly at boot.
     * Migrating to BOOT would block memFreezeBoot enablement.
     * Future: route through MEM_REGION_CACHE with corresponding
     * memFree in the TTM-thread shutdown path. */
    PS1Surface *sfc = (PS1Surface*)malloc(sizeof(PS1Surface));
    if (sfc == NULL) {
        fatalError("Failed to allocate PS1Surface");
    }

    sfc->width = SCREEN_WIDTH;
    sfc->height = SCREEN_HEIGHT;
    sfc->x = nextVRAMX;
    sfc->y = nextVRAMY;
    sfc->pixels = NULL;  /* Will be allocated in VRAM */
    sfc->indexedPixels = NULL;
    sfc->indexedOwned = 0;
    sfc->psbNibbles = 0;
    sfc->nextTile = NULL;

    /* Update VRAM allocation tracking */
    nextVRAMY += SCREEN_HEIGHT;
    if (nextVRAMY >= 512) {  /* VRAM height limit */
        nextVRAMX += SCREEN_WIDTH;
        nextVRAMY = 0;
    }

    return sfc;
}

/*
 * Create a new layer surface for TTM animations
 */
PS1Surface *grNewLayer()
{
    return grNewEmptyBackground();
}

/*
 * Free a layer surface
 */
void grFreeLayer(PS1Surface *sfc)
{
    while (sfc != NULL) {
        PS1Surface *next = sfc->nextTile;
        if (sfc->indexedPixels && sfc->indexedOwned) free(sfc->indexedPixels);
        if (sfc->pixels) free(sfc->pixels);
        free(sfc);
        sfc = next;
    }
}

/*
 * Load BMP sprite sheet into slot
 */
void grLoadBmp(struct TTtmSlot *ttmSlot, uint16 slotNo, char *strArg)
{
    grLoadBmpRAM(ttmSlot, slotNo, strArg);
}

/*
 * Release BMP sprite sheet from slot
 */
void grReleaseBmp(struct TTtmSlot *ttmSlot, uint16 bmpSlotNo)
{
    if (ttmSlot == NULL || bmpSlotNo >= MAX_BMP_SLOTS) {
        return;
    }

    /* Replay records keep raw indexedPixels pointers into BMP/PSB storage.
     * If this slot is being replaced during the active thread's TTM tick,
     * those records must be dropped before the backing storage is freed. */
    if (grCurrentThread != NULL && grCurrentThread->ttmSlot == ttmSlot) {
        uint8 writeIdx = 0;
        for (uint8 readIdx = 0; readIdx < grCurrentThread->numDrawnSprites; readIdx++) {
            struct TDrawnSprite *ds = &grCurrentThread->drawnSprites[readIdx];
            if (ds->imageNo == bmpSlotNo)
                continue;
            if (writeIdx != readIdx)
                grCurrentThread->drawnSprites[writeIdx] = *ds;
            writeIdx++;
        }
        grCurrentThread->numDrawnSprites = writeIdx;
        if (grCurrentThread->replayWriteCursor > writeIdx)
            grCurrentThread->replayWriteCursor = writeIdx;
    }

    /* Invalidate replay records that reference previous contents of this slot. */
    ttmSlot->spriteGen[bmpSlotNo]++;
    ttmSlot->loadedBmp[bmpSlotNo] = NULL;
    ttmSlot->loadedBmpNames[bmpSlotNo] = NULL;

    /* Free all sprites in this slot (PS1Surface structs only;
     * indexedPixels with indexedOwned=0 are NOT freed here because
     * they point into either the BMP resource data or the PSB buffer). */
    for (int i = 0; i < ttmSlot->numSprites[bmpSlotNo]; i++) {
        if (ttmSlot->sprites[bmpSlotNo][i] != NULL) {
            grFreeLayer(ttmSlot->sprites[bmpSlotNo][i]);
            ttmSlot->sprites[bmpSlotNo][i] = NULL;
        }
    }

    /* Free PSB data buffer if this slot was loaded from a PSB file.
     * Must happen AFTER freeing sprites since they pointed into it. */
    if (ttmSlot->psbData[bmpSlotNo] != NULL) {
        /* R33-soak fix: PSB buffer comes from ps1PilotLoadPsb /
         * ps1_streamReadCache → CACHE region (post-memInit). The prior
         * libc free() was a silent no-op on CACHE pointers, leaving
         * the ~80–120 KB PSB block leaked in CACHE forever. memFree(CACHE)
         * range-checks the pointer and routes correctly to either the
         * CACHE free-list or libc free. The R33m diagnostic showed
         * cacheUsed=512 KB with LRU empty — that 512 KB was leaked PSB
         * buffers from prior scenes. */
        if (memIsReady())
            memFree(MEM_REGION_CACHE, ttmSlot->psbData[bmpSlotNo]);
        else
            free(ttmSlot->psbData[bmpSlotNo]);
        ttmSlot->psbData[bmpSlotNo] = NULL;
    }

    ttmSlot->numSprites[bmpSlotNo] = 0;
}

/*
 * Try to load a pre-transcoded PSB (PS1 Sprite Bundle) file for a BMP resource.
 * PSB files have nibbles pre-swapped to PS1 order, eliminating runtime swap.
 *
 * Uses the compile-time PSB registry for O(log N) lookup — no CD probe needed
 * for BMPs that don't have PSB versions.
 *
 * On success: sprites loaded into slot, psbData stored for lifecycle management,
 * bmpResource stored in loadedBmp for dedup.
 * Returns 1 on success, 0 if no PSB available or load failed.
 */
static int grTryLoadPsb(struct TTtmSlot *ttmSlot, uint16 slotNo,
                         char *strArg, struct TBmpResource *bmpResource)
{
    char psbPath[64];
    char psbName[32];
    int nameLen;
    int i;
    uint32 psbSize;
    uint8 *psbBuf;
    PSBHeader *hdr;
    PSBFrame *frames;
    uint8 *pixelBase;
    int numToLoad;
    int framesLoaded;
    uint32 frameTableEnd;

    /* JOHNWALK.BMP — Phase 1+ of the walk plan promotes this to the
     * PSB-backed sprite route. Earlier guard removed 2026-04-29; the
     * registry lookup below pulls JOHNWALK.PSB's size from cd_layout. */

    /* Fast registry lookup — avoids any CD access for unknown BMPs. */
    psbSize = psbRegistryLookup(strArg);
    if (psbSize == 0) return 0;

    /* Build PSB name: JOHNWALK.BMP -> JOHNWALK.PSB */
    nameLen = strlen(strArg);
    if (nameLen < 5 || nameLen > 28) return 0;
    memcpy(psbName, strArg, nameLen + 1);
    /* Replace .BMP extension with .PSB */
    if (psbName[nameLen-4] == '.' &&
        (psbName[nameLen-3] == 'B' || psbName[nameLen-3] == 'b') &&
        (psbName[nameLen-2] == 'M' || psbName[nameLen-2] == 'm') &&
        (psbName[nameLen-1] == 'P' || psbName[nameLen-1] == 'p')) {
        psbName[nameLen-3] = 'P';
        psbName[nameLen-2] = 'S';
        psbName[nameLen-1] = 'B';
    } else {
        return 0;
    }

    /* Try loading PSB from the active scene pack first (offset-based,
     * no CdSearchFile needed — much faster than standalone file lookup). */
    psbBuf = ps1PilotLoadPsb(psbName, &psbSize);

    /* Fallback: load standalone PSB file from CD PSB/ directory.
     * Both the pack path (ps1PilotLoadPsb) and this fallback now
     * return MEM_REGION_CACHE-allocated buffers — free uniformly
     * via memFree(CACHE) below. */
    if (psbBuf == NULL) {
        snprintf(psbPath, sizeof(psbPath), "PSB\\%s", psbName);
        psbBuf = ps1_streamReadCache(psbPath, 0, psbSize);
        if (psbBuf == NULL) return 0;
    }

    /* Validate PSB header */
    if (psbSize < sizeof(PSBHeader)) {
        if (memIsReady()) memFree(MEM_REGION_CACHE, psbBuf); else free(psbBuf);
        return 0;
    }

    hdr = (PSBHeader *)psbBuf;
    if (hdr->magic != PSB_MAGIC || hdr->version != PSB_VERSION) {
        if (memIsReady()) memFree(MEM_REGION_CACHE, psbBuf); else free(psbBuf);
        return 0;
    }

    if (hdr->numFrames == 0 || hdr->dataOffset > psbSize) {
        if (memIsReady()) memFree(MEM_REGION_CACHE, psbBuf); else free(psbBuf);
        return 0;
    }

    /* Cross-check totalSize against actual buffer size */
    if (hdr->totalSize > psbSize) {
        if (memIsReady()) memFree(MEM_REGION_CACHE, psbBuf); else free(psbBuf);
        return 0;
    }

    /* Verify frame table fits */
    frameTableEnd = sizeof(PSBHeader) + (uint32)hdr->numFrames * sizeof(PSBFrame);
    if (frameTableEnd > hdr->dataOffset) {
        if (memIsReady()) memFree(MEM_REGION_CACHE, psbBuf); else free(psbBuf);
        return 0;
    }

    /* Release any existing sprites in this slot */
    if (ttmSlot->numSprites[slotNo])
        grReleaseBmp(ttmSlot, slotNo);

    frames = (PSBFrame *)(psbBuf + sizeof(PSBHeader));
    pixelBase = psbBuf + hdr->dataOffset;
    numToLoad = hdr->numFrames;
    if (numToLoad > MAX_SPRITES_PER_BMP)
        numToLoad = MAX_SPRITES_PER_BMP;

    framesLoaded = 0;
    for (i = 0; i < numToLoad; i++) {
        PSBFrame *fr = &frames[i];
        PS1Surface *surface;

        /* Validate frame bounds against actual buffer */
        if (hdr->dataOffset + fr->offset + fr->size > psbSize) break;

        /* Validate frame dimensions — reject corrupt entries that would
         * waste RAM or confuse the compositing path (max 640x480). */
        if (fr->width == 0 || fr->height == 0 ||
            fr->width > 640 || fr->height > 480) break;

        /* Kept on libc malloc — sprite frame descriptors live with
         * their owning resource (BMP/PSB). Lifecycle is "until LRU
         * eviction." Migrating to BOOT would block memFreezeBoot.
         * Future: route through MEM_REGION_CACHE with companion frees
         * inside grReleaseBmp / the LRU evictor. */
        surface = (PS1Surface*)malloc(sizeof(PS1Surface));
        if (!surface) break;

        surface->width = fr->width;
        surface->height = fr->height;
        surface->x = 0;  /* RAM-based, not in VRAM */
        surface->y = 0;
        surface->clutX = 0;
        surface->clutY = 0;
        surface->nextTile = NULL;
        surface->pixels = NULL;

        /* Point directly into PSB data buffer (zero-copy).
         * PSB data is in PS1 nibble order — no runtime swap needed. */
        surface->indexedPixels = pixelBase + fr->offset;
        surface->indexedOwned = 0;  /* Don't free per-frame; whole buffer freed on release */
        surface->psbNibbles = 1;    /* Flag: PS1 nibble order */

        /* Multi-tile fields (not used for PSB, but init for safety) */
        surface->fullWidth = fr->width;
        surface->fullHeight = fr->height;
        surface->tileOffsetX = 0;
        surface->tileOffsetY = 0;

        ttmSlot->sprites[slotNo][i] = surface;
        framesLoaded++;
    }

    ttmSlot->numSprites[slotNo] = framesLoaded;

    /* If no frames loaded (corruption or OOM), free everything and fall back. */
    if (framesLoaded == 0) {
        if (memIsReady()) memFree(MEM_REGION_CACHE, psbBuf); else free(psbBuf);
        return 0;
    }

    /* NOTE: The PSB header + frame table (~16 + 12*N bytes) remain in the
     * buffer.  Trimming them via memmove + realloc + pointer fixup was
     * considered (TODO 3), but the savings are tiny (e.g. 1.5KB for 120
     * frames) vs. the memmove cost on a 33MHz R3000 and the fragility of
     * post-realloc pointer adjustment.  Keeping the header in-place is
     * simpler and safer. */

    /* Lifecycle: store PSB buffer in slot so grReleaseBmp can free it. */
    ttmSlot->psbData[slotNo] = psbBuf;

    /* Dedup: store the BMP resource pointer so repeated loads of the same
     * BMP into the same slot are detected and skipped by grLoadBmpRAM. */
    ttmSlot->loadedBmp[slotNo] = bmpResource;
    ttmSlot->loadedBmpNames[slotNo] = bmpResource->resName;

    return 1;
}

/*
 * Load BMP sprites into RAM as 4-bit indexed data (compact storage)
 * Stores raw 4-bit packed pixels in indexedPixels, palette lookup at composite time.
 * This uses 4x less memory than the previous 15-bit direct color approach.
 *
 * Load order:
 * 1. Dedup check — if this exact BMP is already loaded in the slot, keep it.
 * 2. PSB fast path — if a pre-transcoded PSB exists on CD, use it (zero-copy,
 *    no runtime nibble swap, compile-time registry lookup).
 * 3. BMP fallback — decompress from RESOURCE.001 / extracted BMP files.
 */
void grLoadBmpRAM(struct TTtmSlot *ttmSlot, uint16 slotNo, char *strArg)
{
    struct TBmpResource *bmpResource = findBmpResource(strArg);
    gStatLastBmpSlot = (uint16)(slotNo + 1U);
    gStatLastBmpFrames = 0;
    gStatLastBmpStatus = 1;  /* attempt */
    if (!bmpResource) {
        gStatLastBmpStatus = 2;  /* resource missing */
        return;
    }

    /* Dedup: if slot already has this exact BMP loaded (via PSB or BMP), keep it. */
    if (ttmSlot->numSprites[slotNo] > 0 && ttmSlot->loadedBmp[slotNo] == bmpResource) {
        gStatLastBmpFrames = ttmSlot->numSprites[slotNo];
        gStatLastBmpStatus = 3;  /* reused existing slot */
        return;
    }

    /* PSB fast path: try pre-transcoded PSB file (skips nibble swap).
     * Pass bmpResource so PSB loader can set loadedBmp for dedup. */
    if (grTryLoadPsb(ttmSlot, slotNo, strArg, bmpResource)) {
        return;
    }

    /* BMP fallback path */
    if (ttmSlot->numSprites[slotNo])
        grReleaseBmp(ttmSlot, slotNo);

    /* On-demand loading: load BMP data if not already loaded */
    if (!bmpResource->uncompressedData) {
        ps1_loadBmpData(bmpResource);
    }

    if (!bmpResource->uncompressedData) {
        gStatLastBmpStatus = 4;  /* no BMP bytes after load */
        return;
    }
    if (bmpResource->numImages < 1) {
        gStatLastBmpStatus = 5;  /* zero image metadata */
        return;
    }

    {
        int numToLoad = bmpResource->numImages;
        uint8 *srcPtr = bmpResource->uncompressedData;
        int framesLoaded = 0;

        if (numToLoad > MAX_SPRITES_PER_BMP) {
            grPs1StatBmpFrameCap((uint16)bmpResource->numImages, MAX_SPRITES_PER_BMP);
            fatalError("BMP frame overflow: %s has %d frames, MAX_SPRITES_PER_BMP=%d",
                       strArg, numToLoad, MAX_SPRITES_PER_BMP);
        }

        for (int frameIdx = 0; frameIdx < numToLoad; frameIdx++) {
            uint16 width = bmpResource->widths[frameIdx];
            uint16 height = bmpResource->heights[frameIdx];
            uint32 indexedSize = ((uint32)width * (uint32)height + 1) / 2;

            /* Kept on libc malloc — same rationale as PSB frame
             * surfaces above: lifecycle is "until LRU eviction,"
             * not strict boot. */
            PS1Surface *surface = (PS1Surface*)malloc(sizeof(PS1Surface));
            if (!surface) {
                gStatLastBmpStatus = 6;  /* allocation failure / partial install */
                break;
            }

            surface->width = width;
            surface->height = height;
            surface->x = 0;  /* Not in VRAM - RAM only */
            surface->y = 0;
            surface->clutX = 0;
            surface->clutY = 0;
            surface->nextTile = NULL;
            surface->pixels = NULL;  /* Not using 15-bit direct color */

            /* Zero-copy indexed frame: reference BMP resource memory directly.
             * This removes per-frame malloc/memcpy churn and cuts fragmentation. */
            surface->indexedPixels = srcPtr;
            surface->indexedOwned = 0;
            surface->psbNibbles = 0;

            /* Advance source pointer for next frame */
            srcPtr += indexedSize;

            /* Store in slot */
            ttmSlot->sprites[slotNo][frameIdx] = surface;
            framesLoaded++;
        }

        ttmSlot->numSprites[slotNo] = framesLoaded;
        ttmSlot->loadedBmp[slotNo] = bmpResource;
        ttmSlot->loadedBmpNames[slotNo] = bmpResource->resName;
        gStatLastBmpFrames = (uint16)framesLoaded;
        gStatLastBmpStatus = (framesLoaded == numToLoad) ? 7 : 8;  /* ok / short install */
        if (framesLoaded < numToLoad) {
            grPs1StatBmpShortLoad((uint16)numToLoad, (uint16)framesLoaded);
        }
    }
}

/*
 * Blit a RAM-stored sprite directly to framebuffer using LoadImage
 * Sprite must have been loaded with grLoadBmpRAM (15-bit direct color)
 * NOTE: No transparency support yet - black pixels will show
 */
void grBlitToFramebuffer(PS1Surface *sprite, sint16 screenX, sint16 screenY)
{
    if (!sprite || !sprite->pixels) return;

    /* Clip to screen bounds */
    sint16 srcX = 0, srcY = 0;
    uint16 blitW = sprite->width;
    uint16 blitH = sprite->height;

    /* Left edge clipping */
    if (screenX < 0) {
        srcX = -screenX;
        blitW -= srcX;
        screenX = 0;
    }
    /* Top edge clipping */
    if (screenY < 0) {
        srcY = -screenY;
        blitH -= srcY;
        screenY = 0;
    }
    /* Right edge clipping */
    if (screenX + blitW > 640) {
        blitW = 640 - screenX;
    }
    /* Bottom edge clipping */
    if (screenY + blitH > 480) {
        blitH = 480 - screenY;
    }

    /* Nothing to draw if fully clipped */
    if (blitW <= 0 || blitH <= 0) return;

    /* If no clipping needed, direct LoadImage */
    if (srcX == 0 && srcY == 0 && blitW == sprite->width && blitH == sprite->height) {
        RECT dstRect;
        setRECT(&dstRect, screenX, screenY, sprite->width, sprite->height);
        LoadImage(&dstRect, (uint32*)sprite->pixels);
        /* No DrawSync here - let main loop handle sync */
    } else {
        /* MEM_REGION_RATIONALE: per-blit clipped-region temp buffer.
         * Lives ~one frame; freed inline. CACHE region: memFree(CACHE)
         * actually reclaims bytes via the coalescing free-list, so
         * per-blit allocs of any size can be reused. TRANSIENT was
         * the wrong fit because its memFree only decrements balance;
         * bytes persist until memSceneReset, and large blits (up to
         * 70 KB on activity-high variants) overflow the budget.
         * INIT_FULL_WRITE — populated by the memcpy loop below. */
        uint16 *tempBuf = (uint16*)memAlloc(MEM_REGION_CACHE,
                                            blitW * blitH * 2,
                                            "grBlitTempBuf");
        uint16 *src = sprite->pixels;
        uint16 *dst = tempBuf;

        for (uint16 y = 0; y < blitH; y++) {
            memcpy(dst, &src[(srcY + y) * sprite->width + srcX], blitW * 2);
            dst += blitW;
        }

        RECT dstRect;
        setRECT(&dstRect, screenX, screenY, blitW, blitH);
        LoadImage(&dstRect, (uint32*)tempBuf);
        DrawSync(0);  /* Must sync before "releasing" buffer - LoadImage is async! */

        memFree(MEM_REGION_CACHE, tempBuf);
    }
}

/* Indexed compositing helpers: simple per-byte decode.
 * Keep this path conservative; fishing regressions started after the later
 * LUT/unrolled fast path landed. */
static inline void compositeIndexedSpanFwd(uint16 *dst, const uint8 *src,
                                           uint32 pixelIdx, int count,
                                           const uint16 *pal)
{
    int di = 0;
    if (count <= 0) return;

    if (pixelIdx & 1) {
        uint8 packed = src[pixelIdx >> 1];
        uint16 p = pal[packed & 0x0F];
        if (p) dst[di] = p;
        di++;
        pixelIdx++;
        count--;
    }

    if (count >= 2) {
        while (count >= 2) {
            uint8 packed = src[pixelIdx >> 1];
            uint16 p0 = pal[(packed >> 4) & 0x0F];
            uint16 p1 = pal[packed & 0x0F];
            if (p0) dst[di] = p0;
            if (p1) dst[di + 1] = p1;
            di += 2;
            pixelIdx += 2;
            count -= 2;
        }
    }

    if (count) {
        uint8 packed = src[pixelIdx >> 1];
        uint16 p = pal[(packed >> 4) & 0x0F];
        if (p) dst[di] = p;
    }
}

static inline void compositeIndexedSpanRev(uint16 *dst, const uint8 *src,
                                           uint32 pixelIdx, int count,
                                           const uint16 *pal)
{
    int di = 0;
    if (count <= 0) return;

    if ((pixelIdx & 1) == 0) {
        uint8 packed = src[pixelIdx >> 1];
        uint16 p = pal[(packed >> 4) & 0x0F];
        if (p) dst[di] = p;
        di++;
        pixelIdx--;
        count--;
    }

    if (count >= 2) {
        while (count >= 2) {
            uint8 packed = src[pixelIdx >> 1];
            uint16 p0 = pal[packed & 0x0F];
            uint16 p1 = pal[(packed >> 4) & 0x0F];
            if (p0) dst[di] = p0;
            if (p1) dst[di + 1] = p1;
            di += 2;
            pixelIdx -= 2;
            count -= 2;
        }
    }

    if (count) {
        uint8 packed = src[pixelIdx >> 1];
        uint16 p = pal[packed & 0x0F];
        if (p) dst[di] = p;
    }
}

/* PS1-nibble-order compositing helpers for PSB sprites.
 * Keep these simple as well while chasing fishing sprite loss. */
static inline void compositePsbSpanFwd(uint16 *dst, const uint8 *src,
                                       uint32 pixelIdx, int count,
                                       const uint16 *pal)
{
    int di = 0;
    if (count <= 0) return;

    if (pixelIdx & 1) {
        uint8 packed = src[pixelIdx >> 1];
        uint16 p = pal[(packed >> 4) & 0x0F];
        if (p) dst[di] = p;
        di++;
        pixelIdx++;
        count--;
    }

    if (count >= 2) {
        while (count >= 2) {
            uint8 packed = src[pixelIdx >> 1];
            uint16 p0 = pal[packed & 0x0F];
            uint16 p1 = pal[(packed >> 4) & 0x0F];
            if (p0) dst[di] = p0;
            if (p1) dst[di + 1] = p1;
            di += 2;
            pixelIdx += 2;
            count -= 2;
        }
    }

    if (count) {
        uint8 packed = src[pixelIdx >> 1];
        uint16 p = pal[packed & 0x0F];
        if (p) dst[di] = p;
    }
}

static inline void compositePsbSpanRev(uint16 *dst, const uint8 *src,
                                       uint32 pixelIdx, int count,
                                       const uint16 *pal)
{
    int di = 0;
    if (count <= 0) return;

    if ((pixelIdx & 1) == 0) {
        uint8 packed = src[pixelIdx >> 1];
        uint16 p = pal[packed & 0x0F];
        if (p) dst[di] = p;
        di++;
        pixelIdx--;
        count--;
    }

    if (count >= 2) {
        while (count >= 2) {
            uint8 packed = src[pixelIdx >> 1];
            uint16 p0 = pal[(packed >> 4) & 0x0F];
            uint16 p1 = pal[packed & 0x0F];
            if (p0) dst[di] = p0;
            if (p1) dst[di + 1] = p1;
            di += 2;
            pixelIdx -= 2;
            count -= 2;
        }
    }

    if (count) {
        uint8 packed = src[pixelIdx >> 1];
        uint16 p = pal[(packed >> 4) & 0x0F];
        if (p) dst[di] = p;
    }
}

static inline void compositeDirectOpaqueRuns(uint16 *dst, const uint16 *src, int count)
{
    int runStart = -1;

    for (int i = 0; i < count; i++) {
        if (src[i] != 0x0000) {
            if (runStart < 0)
                runStart = i;
        } else if (runStart >= 0) {
            memcpy(dst + runStart, src + runStart, (size_t)(i - runStart) * sizeof(uint16));
            runStart = -1;
        }
    }

    if (runStart >= 0)
        memcpy(dst + runStart, src + runStart, (size_t)(count - runStart) * sizeof(uint16));
}

static void grCompositeDirectOpaqueSingleColumn(const uint16 *srcPixels,
                                                uint16 srcWidth,
                                                int screenX,
                                                int screenY,
                                                int height,
                                                int tileBaseX,
                                                PS1Surface *topTile,
                                                PS1Surface *bottomTile)
{
    int tileLocalX;
    int topRows;
    int bottomRows;

    if (srcPixels == NULL || topTile == NULL || bottomTile == NULL ||
        topTile->pixels == NULL || bottomTile->pixels == NULL) {
        return;
    }

    tileLocalX = screenX - tileBaseX;
    topRows = 240 - screenY;
    if (topRows < 0)
        topRows = 0;
    if (topRows > height)
        topRows = height;
    bottomRows = height - topRows;

    for (int row = 0; row < topRows; row++) {
        uint16 *dst = topTile->pixels + ((screenY + row) * (int)topTile->width) + tileLocalX;
        const uint16 *src = srcPixels + ((uint32)row * (uint32)srcWidth);
        compositeDirectOpaqueRuns(dst, src, srcWidth);
    }

    for (int row = 0; row < bottomRows; row++) {
        uint16 *dst = bottomTile->pixels + (row * (int)bottomTile->width) + tileLocalX;
        const uint16 *src = srcPixels + ((uint32)(topRows + row) * (uint32)srcWidth);
        compositeDirectOpaqueRuns(dst, src, srcWidth);
    }
}

/*
 * Composite a RAM-stored sprite into the background tile buffers WITH TRANSPARENCY
 * Skips pixels with value 0x0000 (transparent)
 * This modifies the bgTile RAM buffers so grDrawBackground() renders with transparency
 *
 * Background tile layout:
 * - bgTile0: x=0-319, y=0-239
 * - bgTile1: x=320-639, y=0-239
 * - bgTile3: x=0-319, y=240-479
 * - bgTile4: x=320-639, y=240-479
 */
void __attribute__((optimize("Os")))
grCompositeToBackground(PS1Surface *sprite, sint16 screenX, sint16 screenY)
{
    if (!sprite) return;
    if (!sprite->pixels && !sprite->indexedPixels) return;

    int sprW = sprite->width;
    int sprH = sprite->height;

    /* Sanity check - prevent hang from corrupt/freed sprite data */
    if (sprW == 0 || sprH == 0 || sprW > 640 || sprH > 480) return;

    /* Choose indexed or direct color path */
    int useIndexed = (sprite->indexedPixels != NULL);
    int usePsb = (useIndexed && sprite->psbNibbles);

    int startSy = 0;
    int endSy = sprH;
    if (screenY < 0) startSy = -screenY;
    if (screenY + endSy > 480) endSy = 480 - screenY;
    if (startSy >= endSy) return;

    int startSx = 0;
    int endSx = sprW;
    if (screenX < 0) startSx = -screenX;
    if (screenX + endSx > 640) endSx = 640 - screenX;
    if (startSx >= endSx) return;

    /* Mark dirty region for this sprite */
    grMarkRectDirty(screenX + startSx, screenY + startSy,
                    screenX + endSx, screenY + endSy);

    const uint16 *pal = ttmPalette;

    for (int sy = startSy; sy < endSy; sy++) {
        int destY = screenY + sy;

        /* Determine tile row once per scanline */
        PS1Surface *tileLeft, *tileRight;
        int tileLocalY;
        if (destY < 240) {
            tileLocalY = destY;
            tileLeft = bgTile0;
            tileRight = bgTile1;
        } else {
            tileLocalY = destY - 240;
            tileLeft = bgTile3;
            tileRight = bgTile4;
        }

        uint16 *rowLeft = (tileLeft && tileLeft->pixels) ? (tileLeft->pixels + tileLocalY * (int)tileLeft->width) : NULL;
        uint16 *rowRight = (tileRight && tileRight->pixels) ? (tileRight->pixels + tileLocalY * (int)tileRight->width) : NULL;
        uint32 srcRowBase = (uint32)sy * (uint32)sprW;
        int destStartX = screenX + startSx;
        int destEndX = screenX + endSx;

        if (usePsb) {
            /* PSB (PS1 nibble order): skip nibble swap — data is pre-transcoded */
            if (rowLeft && destStartX < 320) {
                int lx0 = destStartX;
                int lx1 = (destEndX < 320) ? destEndX : 320;
                int srcX = startSx + (lx0 - destStartX);
                int span = lx1 - lx0;
                compositePsbSpanFwd(rowLeft + lx0, sprite->indexedPixels,
                                    srcRowBase + (uint32)srcX, span, pal);
            }

            if (rowRight && destEndX > 320) {
                int rx0 = (destStartX > 320) ? destStartX : 320;
                int rx1 = destEndX;
                int srcX = startSx + (rx0 - destStartX);
                int span = rx1 - rx0;
                compositePsbSpanFwd(rowRight + (rx0 - 320), sprite->indexedPixels,
                                    srcRowBase + (uint32)srcX, span, pal);
            }
        } else if (useIndexed) {
            if (rowLeft && destStartX < 320) {
                int lx0 = destStartX;
                int lx1 = (destEndX < 320) ? destEndX : 320;
                int srcX = startSx + (lx0 - destStartX);
                int span = lx1 - lx0;
                compositeIndexedSpanFwd(rowLeft + lx0, sprite->indexedPixels,
                                        srcRowBase + (uint32)srcX, span, pal);
            }

            if (rowRight && destEndX > 320) {
                int rx0 = (destStartX > 320) ? destStartX : 320;
                int rx1 = destEndX;
                int srcX = startSx + (rx0 - destStartX);
                int span = rx1 - rx0;
                compositeIndexedSpanFwd(rowRight + (rx0 - 320), sprite->indexedPixels,
                                        srcRowBase + (uint32)srcX, span, pal);
            }
        } else {
            if (rowLeft && destStartX < 320) {
                int lx0 = destStartX;
                int lx1 = (destEndX < 320) ? destEndX : 320;
                int srcX = startSx + (lx0 - destStartX);
                int dx = lx0;
                for (; dx + 1 < lx1; dx += 2, srcX += 2) {
                    uint16 p0 = sprite->pixels[srcRowBase + (uint32)srcX];
                    uint16 p1 = sprite->pixels[srcRowBase + (uint32)srcX + 1];
                    if (p0 != 0x0000) rowLeft[dx] = p0;
                    if (p1 != 0x0000) rowLeft[dx + 1] = p1;
                }
                if (dx < lx1) {
                    uint16 p = sprite->pixels[srcRowBase + (uint32)srcX];
                    if (p != 0x0000) rowLeft[dx] = p;
                }
            }

            if (rowRight && destEndX > 320) {
                int rx0 = (destStartX > 320) ? destStartX : 320;
                int rx1 = destEndX;
                int srcX = startSx + (rx0 - destStartX);
                int dx = rx0;
                for (; dx + 1 < rx1; dx += 2, srcX += 2) {
                    uint16 p0 = sprite->pixels[srcRowBase + (uint32)srcX];
                    uint16 p1 = sprite->pixels[srcRowBase + (uint32)srcX + 1];
                    if (p0 != 0x0000) rowRight[dx - 320] = p0;
                    if (p1 != 0x0000) rowRight[dx - 319] = p1;
                }
                if (dx < rx1) {
                    uint16 p = sprite->pixels[srcRowBase + (uint32)srcX];
                    if (p != 0x0000) rowRight[dx - 320] = p;
                }
            }
        }
    }
}

void grCompositeDirect16ToBackground(const uint16 *srcPixels, uint16 srcWidth, uint16 srcHeight,
                                     sint16 screenX, sint16 screenY)
{
    int rectEndX;
    int rectEndY;
    int startSy;
    int endSy;
    int startSx;
    int endSx;

    if (srcPixels == NULL || srcWidth == 0 || srcHeight == 0)
        return;
    if (srcWidth > 640 || srcHeight > 480)
        return;

    rectEndX = screenX + (int)srcWidth;
    rectEndY = screenY + (int)srcHeight;

    if (screenX >= 0 && screenY >= 0 &&
        rectEndX <= 640 && rectEndY <= 480) {
        int tileBaseX = (screenX >= 320) ? 320 : 0;
        if (rectEndX <= tileBaseX + 320) {
            if (screenY < 240 && rectEndY > 240) {
                grMarkSingleColumnDirty(tileBaseX, screenX, srcWidth, screenY, rectEndY);
                if (tileBaseX == 0) {
                    grCompositeDirectOpaqueSingleColumn(srcPixels, srcWidth,
                                                        screenX, screenY, srcHeight,
                                                        tileBaseX, bgTile0, bgTile3);
                } else {
                    grCompositeDirectOpaqueSingleColumn(srcPixels, srcWidth,
                                                        screenX, screenY, srcHeight,
                                                        tileBaseX, bgTile1, bgTile4);
                }
                return;
            }

            int tileLocalX = screenX - tileBaseX;

            grMarkSingleColumnDirty(tileBaseX, screenX, srcWidth, screenY, rectEndY);

            {
                PS1Surface *tile;
                int startLocalY;

                if (screenY < 240) {
                    tile = (tileBaseX == 0) ? bgTile0 : bgTile1;
                    startLocalY = screenY;
                } else {
                    tile = (tileBaseX == 0) ? bgTile3 : bgTile4;
                    startLocalY = screenY - 240;
                }

                if (tile == NULL || tile->pixels == NULL)
                    return;

                for (int sy = 0; sy < (int)srcHeight; sy++) {
                    uint16 *row = tile->pixels + ((startLocalY + sy) * (int)tile->width);
                    const uint16 *srcRow = srcPixels + ((uint32)sy * (uint32)srcWidth);
                    compositeDirectOpaqueRuns(row + tileLocalX, srcRow, srcWidth);
                }
            }
            return;
        }
    }

    startSy = 0;
    endSy = srcHeight;
    if (screenY < 0)
        startSy = -screenY;
    if (screenY + endSy > 480)
        endSy = 480 - screenY;
    if (startSy >= endSy)
        return;

    startSx = 0;
    endSx = srcWidth;
    if (screenX < 0)
        startSx = -screenX;
    if (screenX + endSx > 640)
        endSx = 640 - screenX;
    if (startSx >= endSx)
        return;

    grMarkRectDirty(screenX + startSx, screenY + startSy,
                    screenX + endSx, screenY + endSy);

    for (int sy = startSy; sy < endSy; sy++) {
        int destY = screenY + sy;
        PS1Surface *tileLeft;
        PS1Surface *tileRight;
        int tileLocalY;
        uint16 *rowLeft;
        uint16 *rowRight;
        const uint16 *srcRow = srcPixels + ((uint32)sy * (uint32)srcWidth);
        int destStartX = screenX + startSx;
        int destEndX = screenX + endSx;

        if (destY < 240) {
            tileLocalY = destY;
            tileLeft = bgTile0;
            tileRight = bgTile1;
        } else {
            tileLocalY = destY - 240;
            tileLeft = bgTile3;
            tileRight = bgTile4;
        }

        rowLeft = (tileLeft && tileLeft->pixels) ? (tileLeft->pixels + tileLocalY * (int)tileLeft->width) : NULL;
        rowRight = (tileRight && tileRight->pixels) ? (tileRight->pixels + tileLocalY * (int)tileRight->width) : NULL;

        if (rowLeft && destStartX < 320) {
            int lx0 = destStartX;
            int lx1 = (destEndX < 320) ? destEndX : 320;
            int srcX = startSx + (lx0 - destStartX);
            compositeDirectOpaqueRuns(rowLeft + lx0, srcRow + srcX, lx1 - lx0);
        }

        if (rowRight && destEndX > 320) {
            int rx0 = (destStartX > 320) ? destStartX : 320;
            int rx1 = destEndX;
            int srcX = startSx + (rx0 - destStartX);
            compositeDirectOpaqueRuns(rowRight + (rx0 - 320), srcRow + srcX, rx1 - rx0);
        }
    }
}

static uint16 grReadPackedSpanU16(const uint8 *p)
{
    return (uint16)((uint16)p[0] | ((uint16)p[1] << 8));
}

static uint8 grPacked4SpanMetadataCompact = 0;

static inline int
grReadCompactSpanU16(const uint8 *data,
                     uint32 dataSize,
                     uint32 *offset,
                     uint16 *outValue)
{
    uint8 value;

    if (*offset >= dataSize)
        return 0;

    value = data[*offset];
    *offset = *offset + 1u;
    if (value != 0xffu) {
        *outValue = value;
        return 1;
    }

    if (*offset + 2u > dataSize)
        return 0;
    *outValue = grReadPackedSpanU16(data + *offset);
    *offset = *offset + 2u;
    return 1;
}

static inline void grCompositePacked4OpaqueRun(uint16 *dst,
                                               const uint8 *packedPixels,
                                               int srcPixel,
                                               int count,
                                               const uint16 *palette)
{
    int out = 0;

    if (count <= 0)
        return;

    /* FG2 PAL4 spans contain only visible pixels; index 0 never appears
     * inside a span. The packer splits around no-diff pixels up front. */
    if (srcPixel & 1) {
        uint8 packed = packedPixels[srcPixel >> 1];
        dst[out++] = palette[packed & 0x0F];
        srcPixel++;
        count--;
    }

    while (count >= 2) {
        uint8 packed = packedPixels[srcPixel >> 1];
        dst[out] = palette[packed >> 4];
        dst[out + 1] = palette[packed & 0x0F];
        out += 2;
        srcPixel += 2;
        count -= 2;
    }

    if (count) {
        uint8 packed = packedPixels[srcPixel >> 1];
        dst[out] = palette[packed >> 4];
    }
}

static void grCompositePacked4SpanToBackground(const uint8 *packedPixels,
                                               uint16 pixelCount,
                                               const uint16 *palette,
                                               int destX,
                                               int destY)
{
    int start = 0;
    int end = (int)pixelCount;
    int destStartX;
    int destEndX;
    int tileLocalY;
    PS1Surface *tileLeft;
    PS1Surface *tileRight;

    if (packedPixels == NULL || palette == NULL || pixelCount == 0)
        return;
    if (destY < 0 || destY >= 480)
        return;
    if (destX < 0)
        start = -destX;
    if (destX + end > 640)
        end = 640 - destX;
    if (start >= end)
        return;

    destStartX = destX + start;
    destEndX = destX + end;
    if (destY < 240) {
        tileLocalY = destY;
        tileLeft = bgTile0;
        tileRight = bgTile1;
    } else {
        tileLocalY = destY - 240;
        tileLeft = bgTile3;
        tileRight = bgTile4;
    }

    if (tileLeft != NULL && tileLeft->pixels != NULL && destStartX < 320) {
        int lx0 = destStartX;
        int lx1 = (destEndX < 320) ? destEndX : 320;
        int srcStart = start + (lx0 - destStartX);
        int count = lx1 - lx0;
        uint16 *dst = tileLeft->pixels + (tileLocalY * (int)tileLeft->width) + lx0;

        grCompositePacked4OpaqueRun(dst, packedPixels, srcStart, count, palette);
    }

    if (tileRight != NULL && tileRight->pixels != NULL && destEndX > 320) {
        int rx0 = (destStartX > 320) ? destStartX : 320;
        int rx1 = destEndX;
        int srcStart = start + (rx0 - destStartX);
        int count = rx1 - rx0;
        uint16 *dst = tileRight->pixels + (tileLocalY * (int)tileRight->width) + (rx0 - 320);

        grCompositePacked4OpaqueRun(dst, packedPixels, srcStart, count, palette);
    }
}

void grCompositePacked4SpansToBackground(const uint8 *spanData, uint32 spanDataSize,
                                         const uint16 *palette,
                                         sint16 screenX, sint16 screenY)
{
    uint32 offset = 0;
    uint16 rowCount;
    uint16 perfSpans = 0;
    uint32 perfPixels = 0;
    int perfTrack = ps1PerfEnabled;
    int compactMetadata = grPacked4SpanMetadataCompact;

    if (spanData == NULL || palette == NULL || spanDataSize < 2)
        return;

    rowCount = grReadPackedSpanU16(spanData);
    offset = 2;

    for (uint16 row = 0; row < rowCount; row++) {
        uint16 relY;
        uint16 spanCount;
        int rowScreenY;
        uint16 *rowLeftPixels = NULL;
        uint16 *rowRightPixels = NULL;
        /* Dirty rows are tracked per 320px tile; do not merge across the tile boundary. */
        int rowLeftDirtyStart = 320;
        int rowLeftDirtyEnd = -1;
        int rowRightDirtyStart = 640;
        int rowRightDirtyEnd = 319;

        if (compactMetadata) {
            if (!grReadCompactSpanU16(spanData, spanDataSize, &offset, &relY) ||
                !grReadCompactSpanU16(spanData, spanDataSize, &offset, &spanCount))
                return;
        } else {
            if (offset + 4u > spanDataSize)
                return;
            relY = grReadPackedSpanU16(spanData + offset);
            spanCount = grReadPackedSpanU16(spanData + offset + 2u);
            offset += 4u;
        }
        rowScreenY = (int)screenY + (int)relY;
        if (rowScreenY >= 0 && rowScreenY < 480) {
            int tileLocalY;
            PS1Surface *tileLeft;
            PS1Surface *tileRight;

            if (rowScreenY < 240) {
                tileLocalY = rowScreenY;
                tileLeft = bgTile0;
                tileRight = bgTile1;
            } else {
                tileLocalY = rowScreenY - 240;
                tileLeft = bgTile3;
                tileRight = bgTile4;
            }
            if (tileLeft != NULL && tileLeft->pixels != NULL)
                rowLeftPixels = tileLeft->pixels + (tileLocalY * (int)tileLeft->width);
            if (tileRight != NULL && tileRight->pixels != NULL)
                rowRightPixels = tileRight->pixels + (tileLocalY * (int)tileRight->width);
        }

        for (uint16 span = 0; span < spanCount; span++) {
            uint16 relX;
            uint16 pixelCount;
            uint32 packedBytes;
            int spanX;
            int spanEndX;
            int dirtyStart;
            int dirtyEnd;

            if (compactMetadata) {
                if (!grReadCompactSpanU16(spanData, spanDataSize, &offset, &relX) ||
                    !grReadCompactSpanU16(spanData, spanDataSize, &offset, &pixelCount))
                    return;
            } else {
                if (offset + 4u > spanDataSize)
                    return;
                relX = grReadPackedSpanU16(spanData + offset);
                pixelCount = grReadPackedSpanU16(spanData + offset + 2u);
                offset += 4u;
            }
            if (perfTrack) {
                perfSpans++;
                perfPixels += pixelCount;
            }

            packedBytes = ((uint32)pixelCount + 1u) >> 1;
            if (offset + packedBytes > spanDataSize)
                return;

            spanX = (int)screenX + (int)relX;
            spanEndX = spanX + (int)pixelCount;
            dirtyStart = spanX;
            dirtyEnd = spanEndX;
            if (rowScreenY >= 0 && rowScreenY < 480) {
                if (dirtyStart < 320 && dirtyEnd > 0) {
                    int leftStart = dirtyStart;
                    int leftEnd = dirtyEnd;
                    if (leftStart < 0)
                        leftStart = 0;
                    if (leftEnd > 320)
                        leftEnd = 320;
                    if (leftStart < leftEnd) {
                        if (leftStart < rowLeftDirtyStart)
                            rowLeftDirtyStart = leftStart;
                        if (leftEnd > rowLeftDirtyEnd)
                            rowLeftDirtyEnd = leftEnd;
                    }
                }
                if (dirtyEnd > 320 && dirtyStart < 640) {
                    int rightStart = dirtyStart;
                    int rightEnd = dirtyEnd;
                    if (rightStart < 320)
                        rightStart = 320;
                    if (rightEnd > 640)
                        rightEnd = 640;
                    if (rightStart < rightEnd) {
                        if (rightStart < rowRightDirtyStart)
                            rowRightDirtyStart = rightStart;
                        if (rightEnd > rowRightDirtyEnd)
                            rowRightDirtyEnd = rightEnd;
                    }
                }
            }

            if (spanX >= 0 && spanEndX <= 320 && rowLeftPixels != NULL) {
                grCompositePacked4OpaqueRun(rowLeftPixels + spanX,
                                            spanData + offset,
                                            0,
                                            pixelCount,
                                            palette);
            } else if (spanX >= 320 && spanEndX <= 640 && rowRightPixels != NULL) {
                grCompositePacked4OpaqueRun(rowRightPixels + (spanX - 320),
                                            spanData + offset,
                                            0,
                                            pixelCount,
                                            palette);
            } else {
                grCompositePacked4SpanToBackground(spanData + offset,
                                                   pixelCount,
                                                   palette,
                                                   spanX,
                                                   rowScreenY);
            }
            offset += packedBytes;
        }
        if (rowLeftDirtyStart < rowLeftDirtyEnd)
            grMarkRectDirty(rowLeftDirtyStart, rowScreenY, rowLeftDirtyEnd, rowScreenY + 1);
        if (rowRightDirtyStart < rowRightDirtyEnd)
            grMarkRectDirty(rowRightDirtyStart, rowScreenY, rowRightDirtyEnd, rowScreenY + 1);
    }
    if (perfTrack)
        ps1PerfMarkCompose(rowCount, perfSpans, perfPixels, spanDataSize);
}

static void grCompositePacked4CompactSpansToBackground(const uint8 *spanData,
                                                       uint32 spanDataSize,
                                                       const uint16 *palette,
                                                       sint16 screenX,
                                                       sint16 screenY)
{
    grPacked4SpanMetadataCompact = 1;
    grCompositePacked4SpansToBackground(spanData, spanDataSize, palette, screenX, screenY);
    grPacked4SpanMetadataCompact = 0;
}

void grBeginResidualCleanBgFrame(void)
{
    grClearCurrDirtyState();
    grClearPrevDirtyState();
}

void grBeginResidualCleanBgFirstFrame(void)
{
    grClearCurrDirtyState();
}

void grCompositePacked4TemporalResidualToBackground(const uint8 *spanData, uint32 spanDataSize,
                                                    const uint16 *palette,
                                                    sint16 screenX, sint16 screenY)
{
    uint32 offset = 0;
    uint32 restoredBytes = 0;
    uint16 cleanupRows;

    if (spanData == NULL || palette == NULL || spanDataSize < 2)
        return;

    cleanupRows = grReadPackedSpanU16(spanData);
    offset = 2;
    for (uint16 row = 0; row < cleanupRows; row++) {
        uint16 relY;
        uint16 spanCount;
        int rowScreenY;

        if (offset + 4u > spanDataSize)
            return;
        relY = grReadPackedSpanU16(spanData + offset);
        spanCount = grReadPackedSpanU16(spanData + offset + 2u);
        offset += 4u;
        rowScreenY = (int)screenY + (int)relY;

        for (uint16 span = 0; span < spanCount; span++) {
            uint16 relX;
            uint16 pixelCount;

            if (offset + 4u > spanDataSize)
                return;
            relX = grReadPackedSpanU16(spanData + offset);
            pixelCount = grReadPackedSpanU16(spanData + offset + 2u);
            offset += 4u;
            restoredBytes += grRestoreCleanBgSpanFromRects((int)screenX + (int)relX,
                                                           rowScreenY,
                                                           (int)pixelCount);
        }
    }

    if (ps1PerfEnabled)
        ps1PerfMarkRestore(restoredBytes);
    if (offset < spanDataSize)
        grCompositePacked4SpansToBackground(spanData + offset,
                                            spanDataSize - offset,
                                            palette,
                                            screenX,
                                            screenY);
}

void __attribute__((optimize("Os")))
grCompositePacked4CompactTemporalResidualToBackground(const uint8 *spanData,
                                                      uint32 spanDataSize,
                                                      const uint16 *palette,
                                                      sint16 screenX,
                                                      sint16 screenY)
{
    uint32 offset = 0;
    uint32 restoredBytes = 0;
    uint16 cleanupRows;

    if (spanData == NULL || palette == NULL || spanDataSize < 2)
        return;

    cleanupRows = grReadPackedSpanU16(spanData);
    offset = 2;
    for (uint16 row = 0; row < cleanupRows; row++) {
        uint16 relY;
        uint16 spanCount;
        int rowScreenY;

        if (!grReadCompactSpanU16(spanData, spanDataSize, &offset, &relY) ||
            !grReadCompactSpanU16(spanData, spanDataSize, &offset, &spanCount))
            return;
        rowScreenY = (int)screenY + (int)relY;

        for (uint16 span = 0; span < spanCount; span++) {
            uint16 relX;
            uint16 pixelCount;

            if (!grReadCompactSpanU16(spanData, spanDataSize, &offset, &relX) ||
                !grReadCompactSpanU16(spanData, spanDataSize, &offset, &pixelCount))
                return;
            restoredBytes += grRestoreCleanBgSpanFromRects((int)screenX + (int)relX,
                                                           rowScreenY,
                                                           (int)pixelCount);
        }
    }

    if (ps1PerfEnabled)
        ps1PerfMarkRestore(restoredBytes);
    if (offset < spanDataSize)
        grCompositePacked4CompactSpansToBackground(spanData + offset,
                                                   spanDataSize - offset,
                                                   palette,
                                                   screenX,
                                                   screenY);
}

static inline void grCompositeIndexed8OpaqueRun(uint16 *dst,
                                                const uint8 *indexedPixels,
                                                int count,
                                                const uint16 *palette)
{
    for (int i = 0; i < count; i++)
        dst[i] = palette[indexedPixels[i]];
}

static void grCompositeIndexed8SpanToBackground(const uint8 *indexedPixels,
                                                uint16 pixelCount,
                                                const uint16 *palette,
                                                int destX,
                                                int destY)
{
    int start = 0;
    int end = (int)pixelCount;
    int destStartX;
    int destEndX;
    int tileLocalY;
    PS1Surface *tileLeft;
    PS1Surface *tileRight;

    if (indexedPixels == NULL || palette == NULL || pixelCount == 0)
        return;
    if (destY < 0 || destY >= 480)
        return;
    if (destX < 0)
        start = -destX;
    if (destX + end > 640)
        end = 640 - destX;
    if (start >= end)
        return;

    destStartX = destX + start;
    destEndX = destX + end;
    grMarkRectDirty(destStartX, destY, destEndX, destY + 1);

    if (destY < 240) {
        tileLocalY = destY;
        tileLeft = bgTile0;
        tileRight = bgTile1;
    } else {
        tileLocalY = destY - 240;
        tileLeft = bgTile3;
        tileRight = bgTile4;
    }

    if (tileLeft != NULL && tileLeft->pixels != NULL && destStartX < 320) {
        int lx0 = destStartX;
        int lx1 = (destEndX < 320) ? destEndX : 320;
        int srcStart = start + (lx0 - destStartX);
        uint16 *dst = tileLeft->pixels + (tileLocalY * (int)tileLeft->width) + lx0;
        grCompositeIndexed8OpaqueRun(dst, indexedPixels + srcStart, lx1 - lx0, palette);
    }

    if (tileRight != NULL && tileRight->pixels != NULL && destEndX > 320) {
        int rx0 = (destStartX > 320) ? destStartX : 320;
        int rx1 = destEndX;
        int srcStart = start + (rx0 - destStartX);
        uint16 *dst = tileRight->pixels + (tileLocalY * (int)tileRight->width) + (rx0 - 320);
        grCompositeIndexed8OpaqueRun(dst, indexedPixels + srcStart, rx1 - rx0, palette);
    }
}

void grCompositeIndexed8SpansToBackground(const uint8 *spanData, uint32 spanDataSize,
                                          const uint16 *palette,
                                          sint16 screenX, sint16 screenY)
{
    uint32 offset = 0;
    uint16 rowCount;
    uint16 perfSpans = 0;
    uint32 perfPixels = 0;
    int perfTrack = ps1PerfEnabled;

    if (spanData == NULL || palette == NULL || spanDataSize < 2)
        return;

    rowCount = grReadPackedSpanU16(spanData);
    offset = 2;

    for (uint16 row = 0; row < rowCount; row++) {
        uint16 relY;
        uint16 spanCount;
        int rowScreenY;
        uint16 *rowLeftPixels = NULL;
        uint16 *rowRightPixels = NULL;
        /* Dirty rows are tracked per 320px tile; keep indexed8 aligned with PAL4. */
        int rowLeftDirtyStart = 320;
        int rowLeftDirtyEnd = -1;
        int rowRightDirtyStart = 640;
        int rowRightDirtyEnd = 319;

        if (offset + 4u > spanDataSize)
            return;
        relY = grReadPackedSpanU16(spanData + offset);
        spanCount = grReadPackedSpanU16(spanData + offset + 2u);
        offset += 4u;
        rowScreenY = (int)screenY + (int)relY;
        if (rowScreenY >= 0 && rowScreenY < 480) {
            int tileLocalY;
            PS1Surface *tileLeft;
            PS1Surface *tileRight;

            if (rowScreenY < 240) {
                tileLocalY = rowScreenY;
                tileLeft = bgTile0;
                tileRight = bgTile1;
            } else {
                tileLocalY = rowScreenY - 240;
                tileLeft = bgTile3;
                tileRight = bgTile4;
            }
            if (tileLeft != NULL && tileLeft->pixels != NULL)
                rowLeftPixels = tileLeft->pixels + (tileLocalY * (int)tileLeft->width);
            if (tileRight != NULL && tileRight->pixels != NULL)
                rowRightPixels = tileRight->pixels + (tileLocalY * (int)tileRight->width);
        }

        for (uint16 span = 0; span < spanCount; span++) {
            uint16 relX;
            uint16 pixelCount;
            int spanX;
            int spanEndX;

            if (offset + 4u > spanDataSize)
                return;
            relX = grReadPackedSpanU16(spanData + offset);
            pixelCount = grReadPackedSpanU16(spanData + offset + 2u);
            offset += 4u;
            if (perfTrack) {
                perfSpans++;
                perfPixels += pixelCount;
            }

            if (offset + (uint32)pixelCount > spanDataSize)
                return;

            spanX = (int)screenX + (int)relX;
            spanEndX = spanX + (int)pixelCount;
            if (spanX >= 0 && spanEndX <= 320 && rowLeftPixels != NULL) {
                if (spanX < rowLeftDirtyStart)
                    rowLeftDirtyStart = spanX;
                if (spanEndX > rowLeftDirtyEnd)
                    rowLeftDirtyEnd = spanEndX;
                grCompositeIndexed8OpaqueRun(rowLeftPixels + spanX,
                                             spanData + offset,
                                             pixelCount,
                                             palette);
            } else if (spanX >= 320 && spanEndX <= 640 && rowRightPixels != NULL) {
                if (spanX < rowRightDirtyStart)
                    rowRightDirtyStart = spanX;
                if (spanEndX > rowRightDirtyEnd)
                    rowRightDirtyEnd = spanEndX;
                grCompositeIndexed8OpaqueRun(rowRightPixels + (spanX - 320),
                                             spanData + offset,
                                             pixelCount,
                                             palette);
            } else {
                grCompositeIndexed8SpanToBackground(spanData + offset,
                                                    pixelCount,
                                                    palette,
                                                    spanX,
                                                    rowScreenY);
            }
            offset += (uint32)pixelCount;
        }
        if (rowLeftDirtyStart < rowLeftDirtyEnd)
            grMarkRectDirty(rowLeftDirtyStart, rowScreenY, rowLeftDirtyEnd, rowScreenY + 1);
        if (rowRightDirtyStart < rowRightDirtyEnd)
            grMarkRectDirty(rowRightDirtyStart, rowScreenY, rowRightDirtyEnd, rowScreenY + 1);
    }
    if (perfTrack)
        ps1PerfMarkCompose(rowCount, perfSpans, perfPixels, spanDataSize);
}

void grCompositeIndexed8TemporalResidualToBackground(const uint8 *spanData, uint32 spanDataSize,
                                                     const uint16 *palette,
                                                     sint16 screenX, sint16 screenY)
{
    uint32 offset = 0;
    uint32 restoredBytes = 0;
    uint16 cleanupRows;

    if (spanData == NULL || palette == NULL || spanDataSize < 2)
        return;

    cleanupRows = grReadPackedSpanU16(spanData);
    offset = 2;
    for (uint16 row = 0; row < cleanupRows; row++) {
        uint16 relY;
        uint16 spanCount;
        int rowScreenY;

        if (offset + 4u > spanDataSize)
            return;
        relY = grReadPackedSpanU16(spanData + offset);
        spanCount = grReadPackedSpanU16(spanData + offset + 2u);
        offset += 4u;
        rowScreenY = (int)screenY + (int)relY;

        for (uint16 span = 0; span < spanCount; span++) {
            uint16 relX;
            uint16 pixelCount;

            if (offset + 4u > spanDataSize)
                return;
            relX = grReadPackedSpanU16(spanData + offset);
            pixelCount = grReadPackedSpanU16(spanData + offset + 2u);
            offset += 4u;
            restoredBytes += grRestoreCleanBgSpanFromRects((int)screenX + (int)relX,
                                                           rowScreenY,
                                                           (int)pixelCount);
        }
    }

    if (ps1PerfEnabled)
        ps1PerfMarkRestore(restoredBytes);
    if (offset < spanDataSize)
        grCompositeIndexed8SpansToBackground(spanData + offset,
                                             spanDataSize - offset,
                                             palette,
                                             screenX,
                                             screenY);
}

/*
 * Set clipping rectangle
 */
void grSetClipZone(PS1Surface *sfc, sint16 x1, sint16 y1, sint16 x2, sint16 y2)
{
    /* Set clip rectangle in draw environment */
    draw[db].clip.x = x1;
    draw[db].clip.y = y1;
    draw[db].clip.w = x2 - x1;
    draw[db].clip.h = y2 - y1;
    PutDrawEnv(&draw[db]);
}

/*
 * Draw pixel
 */
void grDrawPixel(PS1Surface *sfc, sint16 x, sint16 y, uint8 color)
{
    /* TODO: Implement pixel drawing using GPU primitive */
    /* For now, stub */
}

/*
 * Draw line — software composite to background tiles.
 * (GPU primitives are reserved for the sprite OT rendered by DrawOTag.)
 */
void grDrawLine(PS1Surface *sfc, sint16 x1, sint16 y1, sint16 x2, sint16 y2, uint8 color)
{
    /* Stub — TTM line draws are rare and cosmetic (e.g. fishing line).
     * Previously these GPU primitives were silently accumulated but never
     * rendered (no DrawOTag).  TODO: implement software line rasterizer. */
    (void)sfc; (void)x1; (void)y1; (void)x2; (void)y2; (void)color;
}

/*
 * Draw filled rectangle — software composite to background tiles.
 * Used by TTM DRAW_RECT opcode for screen clears and overlays.
 */
static void grDrawRectColor15(sint16 x, sint16 y, uint16 width, uint16 height, uint16 bgColor)
{
    /* Software fill directly into bgTile buffers (matching composite approach).
     * This replaces the GPU POLY_F3 path that was never rendered before. */
    sint16 x2 = x + (sint16)width;
    sint16 y2 = y + (sint16)height;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x2 > 640) x2 = 640;
    if (y2 > 480) y2 = 480;
    if (x >= x2 || y >= y2) return;

    /* Mark dirty region */
    grMarkRectDirty(x, y, x2, y2);

    /* Pack two pixels into uint32 for word-fill */
    uint32 bgColor32 = (uint32)bgColor | ((uint32)bgColor << 16);

    for (sint16 py = y; py < y2; py++) {
        int tileLocalY = (py < 240) ? py : py - 240;
        PS1Surface *tileLeft = (py < 240) ? bgTile0 : bgTile3;
        PS1Surface *tileRight = (py < 240) ? bgTile1 : bgTile4;

        if (x < 320 && tileLeft && tileLeft->pixels) {
            sint16 fillStart = x;
            sint16 fillEnd = (x2 < 320) ? x2 : 320;
            uint16 *dst = tileLeft->pixels + (tileLocalY * (int)tileLeft->width) + fillStart;
            int fillCount = fillEnd - fillStart;
            /* Word-align: handle odd start pixel */
            if (fillCount > 0 && ((uintptr_t)dst & 2)) {
                *dst++ = bgColor;
                fillCount--;
            }
            /* Fill 2 pixels per uint32 store */
            uint32 *dst32 = (uint32 *)dst;
            while (fillCount >= 2) {
                *dst32++ = bgColor32;
                fillCount -= 2;
            }
            /* Handle trailing pixel */
            if (fillCount) {
                *(uint16 *)dst32 = bgColor;
            }
        }

        if (x2 > 320 && tileRight && tileRight->pixels) {
            sint16 fillStart = (x > 320) ? x : 320;
            sint16 fillEnd = x2;
            uint16 *dst = tileRight->pixels + (tileLocalY * (int)tileRight->width) + (fillStart - 320);
            int fillCount = fillEnd - fillStart;
            if (fillCount > 0 && ((uintptr_t)dst & 2)) {
                *dst++ = bgColor;
                fillCount--;
            }
            uint32 *dst32 = (uint32 *)dst;
            while (fillCount >= 2) {
                *dst32++ = bgColor32;
                fillCount -= 2;
            }
            if (fillCount) {
                *(uint16 *)dst32 = bgColor;
            }
        }
    }
}

void grDrawRect(PS1Surface *sfc, sint16 x, sint16 y, uint16 width, uint16 height, uint8 color)
{
    (void)sfc;
    grDrawRectColor15(x, y, width, height, ttmPalette[color & 0xF]);
}

/*
 * Draw circle (hollow)
 */
void grDrawCircle(PS1Surface *sfc, sint16 x1, sint16 y1, uint16 width, uint16 height,
                  uint8 fgColor, uint8 bgColor)
{
    /* TODO: Implement circle/ellipse drawing using line primitives */
    /* This requires Bresenham's ellipse algorithm */
}

/*
 * Draw sprite from BMP slot
 */
static void grRecordReplaySprite(struct TTtmThread *thread,
                                 PS1Surface *sprite, sint16 x, sint16 y,
                                 uint16 spriteNo, uint16 imageNo, uint8 flip)
{
    if (!thread || !sprite || !sprite->indexedPixels) return;

    /* Deduplicate exact same draw within the frame.
     * Fast-path: check last 8 entries first (draws are sequential). */
    int scanStart = (thread->numDrawnSprites > 8) ? thread->numDrawnSprites - 8 : 0;
    for (int pass = 0; pass < 2; pass++) {
        int lo = (pass == 0) ? scanStart : 0;
        int hi = (pass == 0) ? thread->numDrawnSprites : scanStart;
        for (int i = lo; i < hi; i++) {
            struct TDrawnSprite *ds = &thread->drawnSprites[i];
            if (ds->imageNo == imageNo &&
                ds->spriteNo == spriteNo &&
                ds->flip == flip &&
                ds->x == x &&
                ds->y == y &&
                ds->sceneEpoch == thread->sceneEpoch) {
                ds->indexedPixels = sprite->indexedPixels;
                ds->width = sprite->width;
                ds->height = sprite->height;
                ds->psbNibbles = sprite->psbNibbles;
                ds->bmpName = thread->ttmSlot ? thread->ttmSlot->loadedBmpNames[imageNo] : NULL;
                return;
            }
        }
    }

    uint16 recIdx;
    if (thread->numDrawnSprites >= MAX_DRAWN_SPRITES) {
        /* Keep most recent draws when scene density exceeds replay capacity.
         * Dropping new records causes actor vanish (Johnny lost behind props). */
        recIdx = thread->replayWriteCursor;
        thread->replayWriteCursor++;
        if (thread->replayWriteCursor >= MAX_DRAWN_SPRITES)
            thread->replayWriteCursor = 0;
    } else {
        recIdx = thread->numDrawnSprites++;
    }

    struct TDrawnSprite *ds = &thread->drawnSprites[recIdx];
    ds->indexedPixels = sprite->indexedPixels;
    ds->width = sprite->width;
    ds->height = sprite->height;
    ds->x = x;
    ds->y = y;
    ds->spriteNo = spriteNo;
    ds->imageNo = imageNo;
    ds->sceneEpoch = thread->sceneEpoch;
    ds->flip = flip;
    ds->psbNibbles = sprite->psbNibbles;
    ds->bmpName = thread->ttmSlot ? thread->ttmSlot->loadedBmpNames[imageNo] : NULL;
}

void grDrawSprite(PS1Surface *sfc, struct TTtmSlot *ttmSlot, sint16 x, sint16 y,
                  uint16 spriteNo, uint16 imageNo)
{
    x += grDx;
    y += grDy;

    /* Validate imageNo bounds */
    if (imageNo >= MAX_BMP_SLOTS || ttmSlot->numSprites[imageNo] == 0) {
        return;
    }

    /* Wrap sprite index to available frames (handles frame cap) */
    uint16 actualSpriteNo = spriteNo % ttmSlot->numSprites[imageNo];

    PS1Surface *sprite = ttmSlot->sprites[imageNo][actualSpriteNo];
    if (sprite == NULL) {
        return;
    }

    /* RAM-based sprites (loaded via grLoadBmpRAM) have x=0, y=0 with valid pixel data. */
    if (sprite->x == 0 && sprite->y == 0 && (sprite->pixels != NULL || sprite->indexedPixels != NULL)) {
        grCompositeToBackground(sprite, x, y);
        grRecordReplaySprite(grCurrentThread, sprite, x, y, spriteNo, imageNo, 0);
        return;
    }

    /* Draw all tiles in this sprite's linked list */
    PS1Surface *tile = sprite;
    while (tile != NULL) {
        /* Allocate DR_TPAGE + SPRT primitives from buffer */
        if (primitiveIndex[db] + sizeof(DR_TPAGE) + sizeof(SPRT) > PRIMITIVE_BUFFER_SIZE) {
            GR_DIAG_PRINTF("Warning: Primitive buffer full!\n");
            return;
        }

        /* Calculate screen position for this tile */
        sint16 tileX = x + tile->tileOffsetX;
        sint16 tileY = y + tile->tileOffsetY;

        /* Add texture page primitive first - tells GPU where texture data is */
        DR_TPAGE *tpage = (DR_TPAGE*)nextPrimitive[db];
        nextPrimitive[db] += sizeof(DR_TPAGE);
        primitiveIndex[db] += sizeof(DR_TPAGE);

        /* Calculate texture page from tile VRAM position
         * tpage X: in 64-pixel units (tile->x / 64)
         * tpage Y: in 256-pixel units (tile->y / 256)
         * Color mode: 0 = 4-bit CLUT (16 colors) */
        uint16 tpageX = tile->x / 64;
        uint16 tpageY = tile->y / 256;
        setDrawTPage(tpage, 0, 0, getTPage(0, 0, tpageX * 64, tpageY * 256));
        ps1GpuOtAddPrim(&ot[db][0], tpage);

        SPRT *sprt = (SPRT*)nextPrimitive[db];
        nextPrimitive[db] += sizeof(SPRT);
        primitiveIndex[db] += sizeof(SPRT);

        /* Initialize sprite primitive */
        setSprt(sprt);
        setXY0(sprt, tileX, tileY);
        setWH(sprt, tile->width, tile->height);
        /* UV coords are relative to texture page (0-255 range)
         * For 4-bit textures: U = ((vram_x % 64) * 4) & 0xFF
         * This is because each texture page is 64 VRAM pixels = 256 texture pixels */
        setUV0(sprt, ((tile->x % 64) * 4) & 0xFF, (tile->y % 256) & 0xFF);
        setClut(sprt, tile->clutX, tile->clutY);
        setRGB0(sprt, 128, 128, 128);  /* Normal brightness */

        /* Add to ordering table */
        ps1GpuOtAddPrim(&ot[db][0], sprt);

        GR_DIAG_PRINTF("Draw tile: pos=(%d,%d) size=%dx%d VRAM=(%d,%d)\n",
                       tileX, tileY, tile->width, tile->height, tile->x, tile->y);

        tile = tile->nextTile;
    }
}

/*
 * Composite sprite to background tiles with horizontal flip
 */
void __attribute__((optimize("Os")))
grCompositeToBackgroundFlip(PS1Surface *sprite, sint16 screenX, sint16 screenY)
{
    if (!sprite) return;
    if (!sprite->pixels && !sprite->indexedPixels) return;

    int sprW = sprite->width;
    int sprH = sprite->height;

    /* Sanity check - prevent hang from corrupt/freed sprite data */
    if (sprW == 0 || sprH == 0 || sprW > 640 || sprH > 480) return;

    /* Choose indexed or direct color path */
    int useIndexed = (sprite->indexedPixels != NULL);
    int usePsb = (useIndexed && sprite->psbNibbles);

    int startSy = 0;
    int endSy = sprH;
    if (screenY < 0) startSy = -screenY;
    if (screenY + endSy > 480) endSy = 480 - screenY;
    if (startSy >= endSy) return;

    int startDestX = screenX < 0 ? 0 : screenX;
    int endDestX = screenX + sprW;
    if (endDestX > 640) endDestX = 640;
    if (startDestX >= endDestX) return;

    /* Mark dirty region for this flipped sprite */
    grMarkRectDirty(startDestX, screenY + startSy, endDestX, screenY + endSy);

    const uint16 *pal = ttmPalette;

    /* Iterate over each visible pixel in the sprite (flipped horizontally) */
    for (int sy = startSy; sy < endSy; sy++) {
        int destY = screenY + sy;

        /* Determine tile row once per scanline */
        PS1Surface *tileLeft, *tileRight;
        int tileLocalY;
        if (destY < 240) {
            tileLocalY = destY;
            tileLeft = bgTile0;
            tileRight = bgTile1;
        } else {
            tileLocalY = destY - 240;
            tileLeft = bgTile3;
            tileRight = bgTile4;
        }

        uint16 *rowLeft = (tileLeft && tileLeft->pixels) ? (tileLeft->pixels + tileLocalY * (int)tileLeft->width) : NULL;
        uint16 *rowRight = (tileRight && tileRight->pixels) ? (tileRight->pixels + tileLocalY * (int)tileRight->width) : NULL;
        uint32 srcRowBase = (uint32)sy * (uint32)sprW;

        if (usePsb) {
            /* PSB (PS1 nibble order): pre-transcoded, no runtime swap */
            if (rowLeft && startDestX < 320) {
                int lx0 = startDestX;
                int lx1 = (endDestX < 320) ? endDestX : 320;
                int srcX = sprW - 1 - (lx0 - screenX);
                int span = lx1 - lx0;
                compositePsbSpanRev(rowLeft + lx0, sprite->indexedPixels,
                                    srcRowBase + (uint32)srcX, span, pal);
            }

            if (rowRight && endDestX > 320) {
                int rx0 = (startDestX > 320) ? startDestX : 320;
                int rx1 = endDestX;
                int srcX = sprW - 1 - (rx0 - screenX);
                int span = rx1 - rx0;
                compositePsbSpanRev(rowRight + (rx0 - 320), sprite->indexedPixels,
                                    srcRowBase + (uint32)srcX, span, pal);
            }
        } else if (useIndexed) {
            if (rowLeft && startDestX < 320) {
                int lx0 = startDestX;
                int lx1 = (endDestX < 320) ? endDestX : 320;
                int srcX = sprW - 1 - (lx0 - screenX);
                int span = lx1 - lx0;
                compositeIndexedSpanRev(rowLeft + lx0, sprite->indexedPixels,
                                        srcRowBase + (uint32)srcX, span, pal);
            }

            if (rowRight && endDestX > 320) {
                int rx0 = (startDestX > 320) ? startDestX : 320;
                int rx1 = endDestX;
                int srcX = sprW - 1 - (rx0 - screenX);
                int span = rx1 - rx0;
                compositeIndexedSpanRev(rowRight + (rx0 - 320), sprite->indexedPixels,
                                        srcRowBase + (uint32)srcX, span, pal);
            }
        } else {
            if (rowLeft && startDestX < 320) {
                int lx0 = startDestX;
                int lx1 = (endDestX < 320) ? endDestX : 320;
                int srcX = sprW - 1 - (lx0 - screenX);
                int dx = lx0;
                for (; dx + 1 < lx1; dx += 2, srcX -= 2) {
                    uint16 p0 = sprite->pixels[srcRowBase + (uint32)srcX];
                    uint16 p1 = sprite->pixels[srcRowBase + (uint32)srcX - 1];
                    if (p0 != 0x0000) rowLeft[dx] = p0;
                    if (p1 != 0x0000) rowLeft[dx + 1] = p1;
                }
                if (dx < lx1) {
                    uint16 p = sprite->pixels[srcRowBase + (uint32)srcX];
                    if (p != 0x0000) rowLeft[dx] = p;
                }
            }

            if (rowRight && endDestX > 320) {
                int rx0 = (startDestX > 320) ? startDestX : 320;
                int rx1 = endDestX;
                int srcX = sprW - 1 - (rx0 - screenX);
                int dx = rx0;
                for (; dx + 1 < rx1; dx += 2, srcX -= 2) {
                    uint16 p0 = sprite->pixels[srcRowBase + (uint32)srcX];
                    uint16 p1 = sprite->pixels[srcRowBase + (uint32)srcX - 1];
                    if (p0 != 0x0000) rowRight[dx - 320] = p0;
                    if (p1 != 0x0000) rowRight[dx - 319] = p1;
                }
                if (dx < rx1) {
                    uint16 p = sprite->pixels[srcRowBase + (uint32)srcX];
                    if (p != 0x0000) rowRight[dx - 320] = p;
                }
            }
        }
    }
}

/*
 * Draw horizontally flipped sprite
 */
void grDrawSpriteFlip(PS1Surface *sfc, struct TTtmSlot *ttmSlot, sint16 x, sint16 y,
                      uint16 spriteNo, uint16 imageNo)
{
    x += grDx;
    y += grDy;

    /* Validate imageNo bounds */
    if (imageNo >= MAX_BMP_SLOTS || ttmSlot->numSprites[imageNo] == 0) {
        return;
    }

    /* Wrap sprite index to available frames (handles frame cap) */
    uint16 actualSpriteNo = spriteNo % ttmSlot->numSprites[imageNo];

    PS1Surface *sprite = ttmSlot->sprites[imageNo][actualSpriteNo];
    if (sprite == NULL) {
        return;
    }

    /* RAM-based sprites (loaded via grLoadBmpRAM) have x=0, y=0 with valid pixel data. */
    if (sprite->x == 0 && sprite->y == 0 && (sprite->pixels != NULL || sprite->indexedPixels != NULL)) {
        grCompositeToBackgroundFlip(sprite, x, y);
        grRecordReplaySprite(grCurrentThread, sprite, x, y, spriteNo, imageNo, 1);
        return;
    }

    /* Draw all tiles in this sprite's linked list (flipped) */
    PS1Surface *tile = sprite;
    while (tile != NULL) {
        /* Allocate POLY_FT4 primitive from buffer */
        /* PS1 doesn't have hardware flip, so we use textured quad with reversed UVs */
        if (primitiveIndex[db] + sizeof(POLY_FT4) > PRIMITIVE_BUFFER_SIZE) {
            GR_DIAG_PRINTF("Warning: Primitive buffer full!\n");
            return;
        }

        /* Calculate flipped screen position for this tile
         * For horizontal flip: tile at offsetX goes to (fullWidth - offsetX - tileWidth) */
        sint16 tileX = x + (sprite->fullWidth - tile->tileOffsetX - tile->width);
        sint16 tileY = y + tile->tileOffsetY;

        POLY_FT4 *poly = (POLY_FT4*)nextPrimitive[db];
        nextPrimitive[db] += sizeof(POLY_FT4);
        primitiveIndex[db] += sizeof(POLY_FT4);

        /* Initialize textured quad */
        setPolyFT4(poly);

        /* Set screen coordinates (normal quad, flipping happens in UV) */
        setXY4(poly,
               tileX, tileY,                              /* Top-left */
               tileX + tile->width, tileY,                /* Top-right */
               tileX, tileY + tile->height,               /* Bottom-left */
               tileX + tile->width, tileY + tile->height); /* Bottom-right */

        /* Calculate texture page from tile VRAM position (4-bit mode) */
        uint16 tpageX = tile->x / 64;
        uint16 tpageY = tile->y / 256;
        poly->tpage = getTPage(0, 0, tpageX * 64, tpageY * 256);

        /* Set UV coordinates (flipped horizontally, relative to texture page)
         * For 4-bit textures: U = ((vram_x % 64) * 4) & 0xFF */
        uint8 baseU = ((tile->x % 64) * 4) & 0xFF;
        uint8 baseV = (tile->y % 256) & 0xFF;
        uint8 u0 = baseU + tile->width;  /* Right edge */
        uint8 u1 = baseU;                 /* Left edge */
        uint8 v0 = baseV;
        uint8 v1 = baseV + tile->height;

        setUV4(poly, u0, v0, u1, v0, u0, v1, u1, v1);  /* Flipped U coords */

        setClut(poly, tile->clutX, tile->clutY);
        setRGB0(poly, 128, 128, 128);  /* Normal brightness */

        /* Add to ordering table */
        ps1GpuOtAddPrim(&ot[db][0], poly);

        GR_DIAG_PRINTF("Draw flipped tile: pos=(%d,%d) size=%dx%d\n",
                       tileX, tileY, tile->width, tile->height);

        tile = tile->nextTile;
    }
}

/*
 * Extended sprite drawing - allows caller to provide their own OT and primitive buffer
 * Walks linked list of tiles for multi-tile sprites (sprites > 64 pixels)
 * Returns 0 on success, -1 on failure
 */
int grDrawSpriteExt(unsigned long *extOT, char **nextPri, PS1Surface *sprite, sint16 x, sint16 y)
{
    if (sprite == NULL || extOT == NULL || nextPri == NULL) {
        return -1;
    }

    x += grDx;
    y += grDy;

    /* RAM-based sprites (loaded via grLoadBmpRAM) have x=0, y=0 with valid pixel data.
     * Composite to background tiles with transparency (0x0000 = transparent).
     * grDrawBackground() will upload the composited tiles later this frame. */
    if (sprite->x == 0 && sprite->y == 0 && (sprite->pixels != NULL || sprite->indexedPixels != NULL)) {
        grCompositeToBackground(sprite, x, y);
        return 0;
    }

    /* Calculate texture page ONCE from first tile - all tiles share same tpage */
    uint16 tpageX = sprite->x / 64;
    uint16 tpageY = sprite->y / 256;

    /* Walk linked list of tiles */
    PS1Surface *tile = sprite;
    while (tile != NULL) {
        /* Screen position for this tile */
        sint16 tileX = x + tile->tileOffsetX;
        sint16 tileY = y + tile->tileOffsetY;

        /* Allocate DR_TPAGE primitive */
        DR_TPAGE *tpage = (DR_TPAGE*)(*nextPri);
        *nextPri += sizeof(DR_TPAGE);

        /* Use SAME texture page for all tiles (calculated from first tile) */
        setDrawTPage(tpage, 0, 0, getTPage(0, 0, tpageX * 64, tpageY * 256));

        /* Allocate SPRT primitive */
        SPRT *sprt = (SPRT*)(*nextPri);
        *nextPri += sizeof(SPRT);

        /* Initialize sprite primitive */
        setSprt(sprt);
        setXY0(sprt, tileX, tileY);
        setWH(sprt, tile->width, tile->height);
        /* UV coords are relative to texture page (0-255 range)
         * For 4-bit textures: U = ((vram_x % 64) * 4) & 0xFF */
        setUV0(sprt, ((tile->x % 64) * 4) & 0xFF, (tile->y % 256) & 0xFF);
        setClut(sprt, tile->clutX, tile->clutY);
        setRGB0(sprt, 128, 128, 128);  /* Normal brightness */

        /* Add to ordering table - sprt FIRST so tpage renders BEFORE it
         * (addPrim adds to HEAD, so last added = first rendered) */
        ps1GpuOtAddPrim(extOT, sprt);
        ps1GpuOtAddPrim(extOT, tpage);

        tile = tile->nextTile;
    }

    return 0;
}

/*
 * Create an empty background tile (black, for RAM compositing)
 */
static PS1Surface *createEmptyBgTileRAM(uint16 width, uint16 height)
{
    /* MEM_REGION_RATIONALE (Round 33): persistent-per-scene BG tile
     * surface (320x240, ~150 KB pixel buffer). 4 tiles × 150 KB =
     * 600 KB worst case — the largest single CACHE pressure source
     * across all 63 scenes. Moved to TRANSIENT because:
     *   - Pixel contents are overwritten by grLoadScreen at scene
     *     start anyway (no cross-scene content reuse).
     *   - Re-allocation via the TRANSIENT bump pointer is ~3
     *     instructions vs the prior O(n) CACHE free-list walk.
     *   - 600 KB out of CACHE eliminates the visitor3-class BSOD
     *     (req=97280 byte clean-rect fails to find a contiguous
     *     CACHE block when bg-tile + LRU + frame buffers occupy it).
     * grBackgroundTilesAssumeWiped (called from fgRuntimeReset right
     * after memSceneReset) NULLs the slot pointers so the next scene's
     * grLoadScreen re-allocates fresh in the new TRANSIENT frame. */
    /* MEM_REGION_RATIONALE: TRANSIENT background tile descriptor, paired
     * with per-scene pixel storage and wiped by memSceneReset. */
    PS1Surface *tile = memIsReady()
        ? (PS1Surface*)memAlloc(MEM_REGION_TRANSIENT, sizeof(PS1Surface), "bgtile-struct")
        : (PS1Surface*)safe_malloc(sizeof(PS1Surface));
    tile->width = width;
    tile->height = height;
    tile->x = 0;
    tile->y = 0;
    tile->indexedPixels = NULL;
    tile->indexedOwned = 0;
    tile->psbNibbles = 0;
    tile->nextTile = NULL;
    /* MEM_REGION_RATIONALE: TRANSIENT background tile pixels, overwritten
     * by each scene's screen load and not reused across scene boundaries. */
    tile->pixels = memIsReady()
        ? (uint16*)memAlloc(MEM_REGION_TRANSIENT, (size_t)width * height * 2u, "bgtile-pixels")
        : (uint16*)safe_malloc(width * height * 2);
    /* Fill with black (0x0000 = transparent/black) */
    for (uint32 i = 0; i < width * height; i++) {
        tile->pixels[i] = 0x0000;
    }
    return tile;
}

static void resetBgTileRAMFields(PS1Surface *tile, uint16 width, uint16 height)
{
    tile->width = width;
    tile->height = height;
    tile->x = 0;
    tile->y = 0;
    tile->indexedPixels = NULL;
    tile->indexedOwned = 0;
    tile->psbNibbles = 0;
    tile->nextTile = NULL;
}

static PS1Surface *ensureEmptyBgTileRAM(PS1Surface **slot, uint16 width, uint16 height)
{
    uint32 pixelCount = (uint32)width * (uint32)height;

    if (*slot == NULL || (*slot)->pixels == NULL ||
        (*slot)->width != width || (*slot)->height != height) {
        freeBgTile(slot);
        *slot = createEmptyBgTileRAM(width, height);
        return *slot;
    }

    resetBgTileRAMFields(*slot, width, height);
    memset((*slot)->pixels, 0, pixelCount * sizeof(uint16));
    return *slot;
}

static PS1Surface *ensureBgTileRAM(PS1Surface **slot, uint16 width, uint16 height)
{
    if (*slot == NULL || (*slot)->pixels == NULL ||
        (*slot)->width != width || (*slot)->height != height) {
        freeBgTile(slot);
        /* MEM_REGION_RATIONALE (Round 33): TRANSIENT bg-tile, see
         * createEmptyBgTileRAM rationale. The freeBgTile call above is
         * a defensive no-op after a scene wipe (slot is already NULL
         * thanks to grBackgroundTilesAssumeWiped); it handles the
         * cold-boot and width/height-change paths. */
        /* MEM_REGION_RATIONALE: TRANSIENT background tile descriptor, same
         * per-scene lifetime as createEmptyBgTileRAM. */
        *slot = memIsReady()
            ? (PS1Surface*)memAlloc(MEM_REGION_TRANSIENT, sizeof(PS1Surface), "bgtile-struct")
            : (PS1Surface*)safe_malloc(sizeof(PS1Surface));
        resetBgTileRAMFields(*slot, width, height);
        /* MEM_REGION_RATIONALE: TRANSIENT background tile pixels, same
         * per-scene lifetime as createEmptyBgTileRAM. */
        (*slot)->pixels = memIsReady()
            ? (uint16*)memAlloc(MEM_REGION_TRANSIENT, (uint32)width * height * 2u, "bgtile-pixels")
            : (uint16*)safe_malloc((uint32)width * height * 2);
    } else {
        resetBgTileRAMFields(*slot, width, height);
    }

    return *slot;
}

/*
 * Initialize empty background
 */
void grInitEmptyBackground()
{
    if (grBackgroundSfc != NULL) {
        /* grLoadScreen sets grBackgroundSfc = bgTile0, so freeing it
         * would leave bgTile0 as a dangling pointer. Null out any match. */
        if (grBackgroundSfc == bgTile0) bgTile0 = NULL;
        if (grBackgroundSfc == bgTile1) bgTile1 = NULL;
        if (grBackgroundSfc == bgTile3) bgTile3 = NULL;
        if (grBackgroundSfc == bgTile4) bgTile4 = NULL;
        grFreeLayer(grBackgroundSfc);
    }

    /* Reset VRAM allocation cursor before grNewEmptyBackground bumps it.
     * Without this the screensaver loop walks the cursor past the PS1 VRAM
     * width (1024 px) after ~2 iterations, and the 3rd scene's background
     * surface lands at an invalid VRAM coordinate. */
    grResetVramCursor();

    grBackgroundSfc = grNewEmptyBackground();

    /* Create empty RAM tiles for sprite compositing (needed by grCompositeToBackground)
     * If tiles already exist (e.g. after grFadeOut darkened them), zero their pixels */
    if (bgTile0 == NULL) bgTile0 = createEmptyBgTileRAM(320, 240);
    else memset(bgTile0->pixels, 0, 320 * 240 * 2);
    if (bgTile1 == NULL) bgTile1 = createEmptyBgTileRAM(320, 240);
    else memset(bgTile1->pixels, 0, 320 * 240 * 2);
    if (bgTile3 == NULL) bgTile3 = createEmptyBgTileRAM(320, 240);
    else memset(bgTile3->pixels, 0, 320 * 240 * 2);
    if (bgTile4 == NULL) bgTile4 = createEmptyBgTileRAM(320, 240);
    else memset(bgTile4->pixels, 0, 320 * 240 * 2);
}

/* Frames the frog-clock animation runs for. The original single-call
 * variant painted one frame and returned; looping here makes the hand
 * fly around the way it does in JOHNNY 6 before the next load step
 * takes over the screen. */
#define MEANWHILE_ANIMATE_VBLANKS  36

/* Per-sprite anchor table for MEANWHIL.BMP, lifted from MEANWHIL.TTM
 * DRAW_SPRITE opcodes. Sprite 0 is the card; sprites 5..16 are the
 * 12 hour-hand positions (one per hour); sprites 1..4 are the 4
 * minute-hand positions. Each (x, y) is the script's draw position
 * for that frame — the bounding boxes vary because each rotated hand
 * has a different bbox, so a fixed top-left would visibly drift. */
static const sint16 kMeanwhilePos[17][2] = {
    {254, 100},  /* 0  card */
    {300, 152},  /* 1  minute */
    {275, 152},  /* 2  minute */
    {291, 161},  /* 3  minute */
    {290, 137},  /* 4  minute */
    {297, 139},  /* 5  hour */
    {301, 143},  /* 6  hour */
    {301, 149},  /* 7  hour */
    {299, 159},  /* 8  hour */
    {299, 161},  /* 9  hour */
    {300, 162},  /* 10 hour */
    {294, 162},  /* 11 hour */
    {285, 163},  /* 12 hour */
    {278, 162},  /* 13 hour */
    {278, 157},  /* 14 hour */
    {281, 147},  /* 15 hour */
    {287, 141},  /* 16 hour */
};

void grShowMeanwhileLoadingFrame(uint16 tick)
{
    struct TTtmSlot slot;
    int i;

    memset(&slot, 0, sizeof(slot));
    grInitEmptyBackground();   /* once — fresh bg surface, zeros bgTile */
    grDx = 0;
    grDy = 0;
    grLoadBmp(&slot, 0, "MEANWHIL.BMP");

    /* Loop, painting card + hour hand + minute hand each VBlank. The
     * MEANWHILE card is opaque enough that re-stamping it before the
     * hands each frame covers the previous hands.
     *
     * NOTE: this leaves bgTile* zeroed when it returns. Callers that
     * follow with code expecting a populated bg (e.g., the walk
     * subsystem) must either reload the bg themselves or set
     * fgLoopSequenceJustReset so the walk is skipped. */
    for (i = 0; i < MEANWHILE_ANIMATE_VBLANKS; i++) {
        if (slot.numSprites[0] > 0) {
            int kf = (int)tick + i;
            int hourSprite   = 5 + ((kf >> 2) % 12);   /* 5..16 */
            int minuteSprite = 1 + (kf & 3);           /* 1..4 */

            /* Card. */
            grDrawSprite(grBackgroundSfc, &slot,
                         kMeanwhilePos[0][0], kMeanwhilePos[0][1], 0, 0);
            /* Hour hand (advances every 4 frames). */
            if (hourSprite < slot.numSprites[0]) {
                grDrawSprite(grBackgroundSfc, &slot,
                             kMeanwhilePos[hourSprite][0],
                             kMeanwhilePos[hourSprite][1],
                             (uint16)hourSprite, 0);
            }
            /* Minute hand (cycles every frame). */
            if (minuteSprite < slot.numSprites[0]) {
                grDrawSprite(grBackgroundSfc, &slot,
                             kMeanwhilePos[minuteSprite][0],
                             kMeanwhilePos[minuteSprite][1],
                             (uint16)minuteSprite, 0);
            }
        }
        grForceFullRedrawNextFrame();
        VSync(0);
        grDrawBackground();
        DrawSync(0);
    }

    grReleaseBmp(&slot, 0);
}

/*
 * Save clean copies of background tiles (after loading background, before any sprite compositing).
 * Used by composite pattern to restore pristine background each frame.
 *
 * Called from grLoadScreen after background is loaded/composited.
 * Always updates ALL clean copies to current tile state.
 */
void grSaveCleanBgTiles(void)
{
    uint32 tileSize = 320 * 240 * 2;  /* 320x240 @ 16-bit = 153,600 bytes per tile */

    /* Reuse existing buffers when possible to avoid 600KB free+malloc churn.
     * Only allocate if buffer doesn't exist yet; always overwrite content. */
    if (bgTile0 && bgTile0->pixels) {
        if (!bgTile0Clean) bgTile0Clean = (uint16*)malloc(tileSize);
        if (bgTile0Clean) memcpy(bgTile0Clean, bgTile0->pixels, tileSize);
    } else if (bgTile0Clean) { free(bgTile0Clean); bgTile0Clean = NULL; }

    if (bgTile1 && bgTile1->pixels) {
        if (!bgTile1Clean) bgTile1Clean = (uint16*)malloc(tileSize);
        if (bgTile1Clean) memcpy(bgTile1Clean, bgTile1->pixels, tileSize);
    } else if (bgTile1Clean) { free(bgTile1Clean); bgTile1Clean = NULL; }

    /* Bottom tiles - for partial height images (like ISLETEMP), the bottom
     * tiles have been composited with scene data over the ocean base. */
    if (bgTile3 && bgTile3->pixels) {
        if (!bgTile3Clean) bgTile3Clean = (uint16*)malloc(tileSize);
        if (bgTile3Clean) memcpy(bgTile3Clean, bgTile3->pixels, tileSize);
    } else if (bgTile3Clean) { free(bgTile3Clean); bgTile3Clean = NULL; }

    if (bgTile4 && bgTile4->pixels) {
        if (!bgTile4Clean) bgTile4Clean = (uint16*)malloc(tileSize);
        if (bgTile4Clean) memcpy(bgTile4Clean, bgTile4->pixels, tileSize);
    } else if (bgTile4Clean) { free(bgTile4Clean); bgTile4Clean = NULL; }

    /* New clean baseline: mark all tiles dirty so first frame uploads everything.
     * Set prevDirty too since the framebuffer may not match the new background. */
    grMarkAllTilesDirty();
    grMarkPrevAllTilesDirty();
}

void grSetSaveCleanOnScreenLoad(int enabled)
{
    grSaveCleanOnScreenLoad = enabled ? 1 : 0;
}

void grSetFullScreenScrCacheEnabled(int enabled)
{
    gFullScreenScrCacheEnabled = enabled ? 1 : 0;
    if (!gFullScreenScrCacheEnabled) {
        if (gFullScreenScrCache != NULL) {
            memFree(MEM_REGION_CACHE, gFullScreenScrCache);
            gFullScreenScrCache = NULL;
        }
        gFullScreenScrCacheBytes = 0;
        gFullScreenScrCacheValid = 0;
        gFullScreenScrCacheName[0] = '\0';
    }
}

/* ---- Rect-based clean-pixel backup (option B). Alternative to full-tile
 *      clean copies: scene declares one or more rectangles that cover its
 *      dynamic regions; only those rects are backed up + restored. Massive
 *      memory savings for scenes that only animate a small portion of the
 *      screen (fishing1: wave strip + Johnny area ≈ 181 KB instead of 614
 *      KB for all four full-size clean tiles). */

#define GR_MAX_CLEAN_RECTS 8

struct TGrCleanRect {
    sint16 x, y;
    uint16 width, height;
    uint16 *pixels;
    uint32 capacityBytes;
    /* MEM_REGION tag for the pixels allocation (0=TRANSIENT,
     * 1=CACHE). Round 13 introduced dynamic routing: clean-rect
     * snapshots prefer TRANSIENT (right semantic) but spill to
     * CACHE when TRANSIENT can't fit. memFree must use the same
     * region the alloc came from. */
    uint8 pixelsRegion;
};

static struct TGrCleanRect gGrCleanRects[GR_MAX_CLEAN_RECTS];
static int gGrCleanRectCount = 0;
static int gGrCleanBgBlackMode = 0;
static int gGrCleanBgRectsForceCache = 0;

/* Copy a rectangle's pixels out of the 4 bg tiles into a flat buffer. */
static void grCleanRectCopyOut(struct TGrCleanRect *r)
{
    int sy;
    if (r->pixels == NULL || r->width == 0 || r->height == 0) return;
    for (sy = 0; sy < (int)r->height; sy++) {
        int destY = r->y + sy;
        if (destY < 0 || destY >= 480) continue;
        {
            PS1Surface *tileLeft, *tileRight;
            int tileLocalY;
            if (destY < 240) { tileLocalY = destY; tileLeft = bgTile0; tileRight = bgTile1; }
            else             { tileLocalY = destY - 240; tileLeft = bgTile3; tileRight = bgTile4; }
            uint16 *dstRow = r->pixels + (uint32)sy * (uint32)r->width;
            int xStart = r->x;
            int xEnd   = r->x + (int)r->width;
            if (xStart < 0) xStart = 0;
            if (xEnd > 640) xEnd = 640;
            if (tileLeft && tileLeft->pixels && xStart < 320) {
                int lx0 = xStart;
                int lx1 = (xEnd < 320) ? xEnd : 320;
                uint16 *src = tileLeft->pixels + (tileLocalY * (int)tileLeft->width) + lx0;
                memcpy(dstRow + (lx0 - r->x), src,
                       (size_t)(lx1 - lx0) * sizeof(uint16));
            }
            if (tileRight && tileRight->pixels && xEnd > 320) {
                int rx0 = (xStart > 320) ? xStart : 320;
                int rx1 = xEnd;
                uint16 *src = tileRight->pixels + (tileLocalY * (int)tileRight->width) + (rx0 - 320);
                memcpy(dstRow + (rx0 - r->x), src,
                       (size_t)(rx1 - rx0) * sizeof(uint16));
            }
        }
    }
}

/* Copy only the previous frame's dirty rows from a clean-rect backup. Upload
 * is already driven by prevDirty in grDrawBackground, matching tile-mode. */
static void grCleanRectCopyIn(const struct TGrCleanRect *r)
{
    int sy;
    uint32 copiedBytes = 0;
    int perfTrack = ps1PerfEnabled;
    if (r->pixels == NULL || r->width == 0 || r->height == 0) return;
    grEnsureDirtyRowState();
    for (sy = 0; sy < (int)r->height; sy++) {
        int destY = r->y + sy;
        if (destY < 0 || destY >= 480) continue;
        {
            PS1Surface *tileLeft, *tileRight;
            int tileLocalY;
            if (destY < 240) { tileLocalY = destY; tileLeft = bgTile0; tileRight = bgTile1; }
            else             { tileLocalY = destY - 240; tileLeft = bgTile3; tileRight = bgTile4; }
            const uint16 *srcRow = r->pixels + (uint32)sy * (uint32)r->width;
            int xStart = r->x;
            int xEnd   = r->x + (int)r->width;
            int leftIdx = (destY < 240) ? 0 : 2;
            int rightIdx = leftIdx + 1;
            if (xStart < 0) xStart = 0;
            if (xEnd > 640) xEnd = 640;
            if (tileLeft && tileLeft->pixels && xStart < 320 &&
                prevDirtyRowMinX[leftIdx][tileLocalY] >= 0) {
                int lx0 = xStart;
                int lx1 = (xEnd < 320) ? xEnd : 320;
                if (lx0 < prevDirtyRowMinX[leftIdx][tileLocalY])
                    lx0 = prevDirtyRowMinX[leftIdx][tileLocalY];
                if (lx1 > prevDirtyRowMaxX[leftIdx][tileLocalY] + 1)
                    lx1 = prevDirtyRowMaxX[leftIdx][tileLocalY] + 1;
                if (lx0 < lx1) {
                    uint16 *dst = tileLeft->pixels + (tileLocalY * (int)tileLeft->width) + lx0;
                    size_t bytes = (size_t)(lx1 - lx0) * sizeof(uint16);
                    memcpy(dst, srcRow + (lx0 - r->x), bytes);
                    if (perfTrack)
                        copiedBytes += (uint32)bytes;
                }
            }
            if (tileRight && tileRight->pixels && xEnd > 320 &&
                prevDirtyRowMinX[rightIdx][tileLocalY] >= 0) {
                int rx0 = (xStart > 320) ? (xStart - 320) : 0;
                int rx1 = xEnd - 320;
                if (rx0 < prevDirtyRowMinX[rightIdx][tileLocalY])
                    rx0 = prevDirtyRowMinX[rightIdx][tileLocalY];
                if (rx1 > prevDirtyRowMaxX[rightIdx][tileLocalY] + 1)
                    rx1 = prevDirtyRowMaxX[rightIdx][tileLocalY] + 1;
                if (rx0 < rx1) {
                    uint16 *dst = tileRight->pixels + (tileLocalY * (int)tileRight->width) + rx0;
                    size_t bytes = (size_t)(rx1 - rx0) * sizeof(uint16);
                    memcpy(dst, srcRow + ((rx0 + 320) - r->x), bytes);
                    if (perfTrack)
                        copiedBytes += (uint32)bytes;
                }
            }
        }
    }
    if (perfTrack)
        ps1PerfMarkRestore(copiedBytes);
}

static void grCleanRectCopyInFull(const struct TGrCleanRect *r)
{
    int sy;
    if (r->pixels == NULL || r->width == 0 || r->height == 0) return;
    for (sy = 0; sy < (int)r->height; sy++) {
        int destY = r->y + sy;
        if (destY < 0 || destY >= 480) continue;
        {
            PS1Surface *tileLeft, *tileRight;
            int tileLocalY;
            const uint16 *srcRow = r->pixels + (uint32)sy * (uint32)r->width;
            int xStart = r->x;
            int xEnd = r->x + (int)r->width;
            if (destY < 240) {
                tileLocalY = destY;
                tileLeft = bgTile0;
                tileRight = bgTile1;
            } else {
                tileLocalY = destY - 240;
                tileLeft = bgTile3;
                tileRight = bgTile4;
            }
            if (xStart < 0) xStart = 0;
            if (xEnd > 640) xEnd = 640;
            if (tileLeft && tileLeft->pixels && xStart < 320) {
                int lx0 = xStart;
                int lx1 = (xEnd < 320) ? xEnd : 320;
                if (lx0 < lx1) {
                    uint16 *dst = tileLeft->pixels +
                        (tileLocalY * (int)tileLeft->width) + lx0;
                    memcpy(dst, srcRow + (lx0 - r->x),
                           (size_t)(lx1 - lx0) * sizeof(uint16));
                    grMarkRectDirty(lx0, destY, lx1, destY + 1);
                }
            }
            if (tileRight && tileRight->pixels && xEnd > 320) {
                int rx0 = (xStart > 320) ? (xStart - 320) : 0;
                int rx1 = xEnd - 320;
                if (rx0 < rx1) {
                    uint16 *dst = tileRight->pixels +
                        (tileLocalY * (int)tileRight->width) + rx0;
                    memcpy(dst, srcRow + ((rx0 + 320) - r->x),
                           (size_t)(rx1 - rx0) * sizeof(uint16));
                    grMarkRectDirty(rx0 + 320, destY, rx1 + 320, destY + 1);
                }
            }
        }
    }
}

static uint32 grRestoreCleanBgSpanFromRects(int x, int y, int width)
{
    int xEnd;
    uint32 copiedBytes = 0;

    if (width <= 0 || y < 0 || y >= 480)
        return 0;
    xEnd = x + width;
    if (x < 0)
        x = 0;
    if (xEnd > 640)
        xEnd = 640;
    if (x >= xEnd)
        return 0;

    if (gGrCleanBgBlackMode) {
        PS1Surface *tileLeft;
        PS1Surface *tileRight;
        int tileLocalY;

        if (y < 240) {
            tileLocalY = y;
            tileLeft = bgTile0;
            tileRight = bgTile1;
        } else {
            tileLocalY = y - 240;
            tileLeft = bgTile3;
            tileRight = bgTile4;
        }

        if (tileLeft != NULL && tileLeft->pixels != NULL && x < 320) {
            int lx0 = x;
            int lx1 = (xEnd < 320) ? xEnd : 320;
            if (lx0 < lx1) {
                uint16 *dst = tileLeft->pixels + (tileLocalY * (int)tileLeft->width) + lx0;
                size_t bytes = (size_t)(lx1 - lx0) * sizeof(uint16);
                memset(dst, 0, bytes);
                grMarkRectDirty(lx0, y, lx1, y + 1);
                copiedBytes += (uint32)bytes;
            }
        }
        if (tileRight != NULL && tileRight->pixels != NULL && xEnd > 320) {
            int rx0 = (x > 320) ? (x - 320) : 0;
            int rx1 = xEnd - 320;
            if (rx0 < rx1) {
                uint16 *dst = tileRight->pixels + (tileLocalY * (int)tileRight->width) + rx0;
                size_t bytes = (size_t)(rx1 - rx0) * sizeof(uint16);
                memset(dst, 0, bytes);
                grMarkRectDirty(rx0 + 320, y, rx1 + 320, y + 1);
                copiedBytes += (uint32)bytes;
            }
        }

        return copiedBytes;
    }

    for (int i = 0; i < gGrCleanRectCount; i++) {
        const struct TGrCleanRect *r = &gGrCleanRects[i];
        int rx0;
        int rx1;
        int sy;

        if (r->pixels == NULL || r->width == 0 || r->height == 0)
            continue;
        if (y < r->y || y >= r->y + (int)r->height)
            continue;

        rx0 = x;
        rx1 = xEnd;
        if (rx0 < r->x)
            rx0 = r->x;
        if (rx1 > r->x + (int)r->width)
            rx1 = r->x + (int)r->width;
        if (rx0 >= rx1)
            continue;

        sy = y - r->y;
        {
            PS1Surface *tileLeft;
            PS1Surface *tileRight;
            int tileLocalY;
            const uint16 *srcRow = r->pixels + (uint32)sy * (uint32)r->width;

            if (y < 240) {
                tileLocalY = y;
                tileLeft = bgTile0;
                tileRight = bgTile1;
            } else {
                tileLocalY = y - 240;
                tileLeft = bgTile3;
                tileRight = bgTile4;
            }

            if (tileLeft != NULL && tileLeft->pixels != NULL && rx0 < 320) {
                int lx0 = rx0;
                int lx1 = (rx1 < 320) ? rx1 : 320;
                if (lx0 < lx1) {
                    uint16 *dst = tileLeft->pixels + (tileLocalY * (int)tileLeft->width) + lx0;
                    size_t bytes = (size_t)(lx1 - lx0) * sizeof(uint16);
                    memcpy(dst, srcRow + (lx0 - r->x), bytes);
                    grMarkRectDirty(lx0, y, lx1, y + 1);
                    copiedBytes += (uint32)bytes;
                }
            }
            if (tileRight != NULL && tileRight->pixels != NULL && rx1 > 320) {
                int rxLocal0 = (rx0 > 320) ? (rx0 - 320) : 0;
                int rxLocal1 = rx1 - 320;
                if (rxLocal0 < rxLocal1) {
                    uint16 *dst = tileRight->pixels + (tileLocalY * (int)tileRight->width) + rxLocal0;
                    size_t bytes = (size_t)(rxLocal1 - rxLocal0) * sizeof(uint16);
                    memcpy(dst, srcRow + ((rxLocal0 + 320) - r->x), bytes);
                    grMarkRectDirty(rxLocal0 + 320, y, rxLocal1 + 320, y + 1);
                    copiedBytes += (uint32)bytes;
                }
            }
        }
    }

    return copiedBytes;
}

static void grResetCleanBgRects(int releasePixels)
{
    int i;
    for (i = 0; i < GR_MAX_CLEAN_RECTS; i++) {
        if (releasePixels && gGrCleanRects[i].pixels) {
            /* Use the recorded region; dynamic routing in
             * grSaveCleanBgTiles may have placed pixels in
             * EITHER TRANSIENT or CACHE. */
            MemRegion freeRegion = gGrCleanRects[i].pixelsRegion
                ? MEM_REGION_CACHE : MEM_REGION_TRANSIENT;
            memFree(freeRegion, gGrCleanRects[i].pixels);
            gGrCleanRects[i].pixels = NULL;
            gGrCleanRects[i].capacityBytes = 0;
            gGrCleanRects[i].pixelsRegion = 0;
        }
        gGrCleanRects[i].x = 0;
        gGrCleanRects[i].y = 0;
        gGrCleanRects[i].width = 0;
        gGrCleanRects[i].height = 0;
    }
    gGrCleanRectCount = 0;
}

void grFreeCleanBgRects(void)
{
    grResetCleanBgRects(1);
    gGrCleanBgBlackMode = 0;
}

void grDeactivateCleanBgRects(void)
{
    grResetCleanBgRects(0);
    gGrCleanBgBlackMode = 0;
}

void grSetCleanBgBlackMode(int enabled)
{
    gGrCleanBgBlackMode = enabled ? 1 : 0;
}

void grPreallocCleanBgRects(const uint32 *capBytes, int n)
{
    /* Pre-allocation is unnecessary under the new memory-region allocator.
     * Clean-rect snapshots now live in TRANSIENT, which is wiped between
     * scenes — every grSaveCleanBgRects call allocates fresh from a
     * deterministic region. There's no fragmentation to pre-empt and no
     * benefit to reserving capacity in advance.
     *
     * Kept as a public symbol so existing boot code that calls this
     * compiles. The boot caller could be deleted in Phase 2 of the
     * mem-region rollout. */
    (void)capBytes;
    (void)n;
}

int grCleanBgRectsCount(void)
{
    return gGrCleanRectCount;
}

void grSetCleanBgRectsForceCache(int enabled)
{
    gGrCleanBgRectsForceCache = enabled ? 1 : 0;
}

void grRestoreBgRectsFull(void)
{
    int i;
    for (i = 0; i < gGrCleanRectCount; i++)
        grCleanRectCopyInFull(&gGrCleanRects[i]);
}

unsigned long grCleanBgRectsBytes(void)
{
    unsigned long total = 0;
    int i;
    for (i = 0; i < gGrCleanRectCount; i++)
        total += (unsigned long)gGrCleanRects[i].width *
                 (unsigned long)gGrCleanRects[i].height *
                 (unsigned long)sizeof(uint16);
    return total;
}

/* Caller-owned rect copy wrappers. Used by walk_pilot's persistent
 * walk-area buffer — same per-tile splitting logic as the gGrCleanRects
 * snapshots, but the buffer is owned outside of graphics_ps1's rect
 * machinery (so grSaveCleanBgRects/grFreeCleanBgRects don't touch it).
 * The dst/src buffer must be sized w * h * sizeof(uint16). */
void grCaptureBgRect(uint16 *dst, sint16 x, sint16 y, uint16 w, uint16 h)
{
    struct TGrCleanRect r;
    if (dst == NULL || w == 0 || h == 0)
        return;
    r.x = x;
    r.y = y;
    r.width = w;
    r.height = h;
    r.pixels = dst;
    r.capacityBytes = (uint32)w * (uint32)h * sizeof(uint16);
    grCleanRectCopyOut(&r);
}

void grRestoreBgRect(const uint16 *src, sint16 x, sint16 y, uint16 w, uint16 h)
{
    struct TGrCleanRect r;
    if (src == NULL || w == 0 || h == 0)
        return;
    r.x = x;
    r.y = y;
    r.width = w;
    r.height = h;
    r.pixels = (uint16 *)src;
    r.capacityBytes = (uint32)w * (uint32)h * sizeof(uint16);
    grCleanRectCopyIn(&r);
}

/* Set up rect-based clean backup. Drops any existing rects first (also any
 * full-tile clean copies from grSaveCleanBgTiles, via the caller's intent).
 * Returns count of rects successfully allocated. */
int grSaveCleanBgRects(const sint16 *xArr, const sint16 *yArr,
                       const uint16 *wArr, const uint16 *hArr, int n)
{
    int i;
    uint32 requiredBytes[GR_MAX_CLEAN_RECTS];
    int allocatedThisCall[GR_MAX_CLEAN_RECTS];

    grDeactivateCleanBgRects();
    gGrCleanBgBlackMode = 0;
    grFreeCleanBgTiles();  /* mutually exclusive: rect-mode replaces tile-mode */

    if (xArr == NULL || yArr == NULL || wArr == NULL || hArr == NULL || n <= 0)
        return 0;
    if (n > GR_MAX_CLEAN_RECTS) n = GR_MAX_CLEAN_RECTS;

    for (i = 0; i < GR_MAX_CLEAN_RECTS; i++) {
        requiredBytes[i] = 0;
        allocatedThisCall[i] = 0;
    }

    /* Atomic allocation phase: all requested rect buffers must exist before
     * any rect becomes active. Otherwise a partial clean-restore set can both
     * leak and leave stale pixels from a prior foreground frame. */
    /* Compute required bytes for every rect first, then allocate in
     * size-DESCENDING order. Round 15 fix: smaller rects allocated
     * first can fragment TRANSIENT such that a later large rect
     * doesn't fit, even though TRANSIENT had enough space at start.
     * Visitor5 specifically: 97 KB rect last in iteration order would
     * fail when 60 KB was left after smaller rects took TRANSIENT
     * first. Sorting biggest-first gives the large rect first crack
     * at the contiguous TRANSIENT space.
     *
     * Selection sort by index — n ≤ 8 so O(n²) is fine. */
    int sortedIdx[GR_MAX_CLEAN_RECTS];
    for (i = 0; i < n; i++) {
        requiredBytes[i] = (uint32)wArr[i] * (uint32)hArr[i] * (uint32)sizeof(uint16);
        if (requiredBytes[i] == 0)
            goto fail;
        sortedIdx[i] = i;
    }
    /* Sort indices by requiredBytes descending. */
    for (int a = 0; a < n - 1; a++) {
        int maxJ = a;
        for (int b = a + 1; b < n; b++) {
            if (requiredBytes[sortedIdx[b]] > requiredBytes[sortedIdx[maxJ]])
                maxJ = b;
        }
        if (maxJ != a) {
            int tmp = sortedIdx[a];
            sortedIdx[a] = sortedIdx[maxJ];
            sortedIdx[maxJ] = tmp;
        }
    }
    /* Allocate in size-descending order. The rect at original index
     * `idx` is still stored at gGrCleanRects[idx]. */
    for (int s = 0; s < n; s++) {
        int idx = sortedIdx[s];
        /* MEM_REGION_RATIONALE: per-scene clean-rect snapshot (one of
         * 6 atomic slots; see comment block above the for loop).
         * Dynamic routing (Round 14): prefer TRANSIENT for correct
         * wholesale-wipe semantics; spill to CACHE when TRANSIENT
         * lacks contiguous space. Each rect records which region
         * its pixels came from so the matching memFree is used
         * (memFree's range-check would otherwise mismatch and
         * double-free via the TransientLibcEntry list). */
        const size_t transRemaining = MEM_TRANSIENT_BUDGET -
            memRegionUsed((unsigned int)MEM_REGION_TRANSIENT);
        /* Round 33-soak update: drop the TRANSIENT_RESERVE that
         * previously forced clean-rects into CACHE when TRANSIENT
         * was within 16 KB of full. CACHE is NOT wiped per-scene,
         * so spilling a 70–100 KB clean-rect into it across many
         * scene transitions accumulates fragmentation that
         * eventually breaks a later CACHE alloc (R33 soak BSOD at
         * 226s on stand6 with CACHE peak 568 KB and 113 KB free
         * but fragmented below the 71 KB request). The "other
         * TRANSIENT allocs" (sound events, setup segment) that
         * the reserve was protecting have their own libc-fallback
         * via TransientLibcEntry — they don't lose correctness
         * if TRANSIENT fills, just transparently spill to libc and
         * still get per-scene-wiped at memSceneReset. */
        MemRegion target;
        if (gGrCleanBgRectsForceCache) {
            target = MEM_REGION_CACHE;
        } else if (requiredBytes[idx] <= transRemaining) {
            target = MEM_REGION_TRANSIENT;
        } else {
            target = MEM_REGION_CACHE;
        }
        /* MEM_REGION_RATIONALE: clean-rect pixels prefer TRANSIENT for
         * scene-lifetime snapshots and spill to CACHE only when needed. */
        gGrCleanRects[idx].pixels = (uint16 *)memAlloc(target,
                                                     requiredBytes[idx],
                                                     "grCleanRectPixels");
        gGrCleanRects[idx].pixelsRegion =
            (target == MEM_REGION_CACHE) ? 1u : 0u;
        gGrCleanRects[idx].capacityBytes = requiredBytes[idx];
        allocatedThisCall[idx] = 1;
    }

    for (i = 0; i < n; i++) {
        gGrCleanRects[i].x = xArr[i];
        gGrCleanRects[i].y = yArr[i];
        gGrCleanRects[i].width = wArr[i];
        gGrCleanRects[i].height = hArr[i];
        grCleanRectCopyOut(&gGrCleanRects[i]);
    }
    gGrCleanRectCount = n;

    /* Force a full first-frame upload. FG2 restores clean rects after setup,
     * but static pixels outside those rects still need an initial refresh. */
    grMarkAllTilesDirty();
    grMarkPrevAllTilesDirty();
    return gGrCleanRectCount;

fail:
    for (i = 0; i < n; i++) {
        if (allocatedThisCall[i] && gGrCleanRects[i].pixels != NULL) {
            /* Use the recorded region (dynamic routing). */
            MemRegion freeRegion = gGrCleanRects[i].pixelsRegion
                ? MEM_REGION_CACHE : MEM_REGION_TRANSIENT;
            memFree(freeRegion, gGrCleanRects[i].pixels);
            gGrCleanRects[i].pixels = NULL;
            gGrCleanRects[i].capacityBytes = 0;
            gGrCleanRects[i].pixelsRegion = 0;
        }
    }
    grDeactivateCleanBgRects();
    return 0;
}

int grSaveCleanBgRectsSplit(const sint16 *xArr, const sint16 *yArr,
                            const uint16 *wArr, const uint16 *hArr, int n,
                            uint32 maxBytesPerRect)
{
    sint16 sx[GR_MAX_CLEAN_RECTS];
    sint16 sy[GR_MAX_CLEAN_RECTS];
    uint16 sw[GR_MAX_CLEAN_RECTS];
    uint16 sh[GR_MAX_CLEAN_RECTS];
    int outCount = 0;
    int i;

    if (maxBytesPerRect == 0)
        return grSaveCleanBgRects(xArr, yArr, wArr, hArr, n);
    if (xArr == NULL || yArr == NULL || wArr == NULL || hArr == NULL || n <= 0)
        return 0;

    for (i = 0; i < n; i++) {
        uint16 remaining;
        sint16 curY;
        uint32 bytesPerRow;
        uint16 maxRows;

        if (wArr[i] == 0 || hArr[i] == 0)
            return 0;

        bytesPerRow = (uint32)wArr[i] * (uint32)sizeof(uint16);
        maxRows = (uint16)(maxBytesPerRect / bytesPerRow);
        if (maxRows == 0)
            maxRows = 1;

        remaining = hArr[i];
        curY = yArr[i];
        while (remaining > 0) {
            uint16 stripH = remaining;
            if (stripH > maxRows)
                stripH = maxRows;
            if (outCount >= GR_MAX_CLEAN_RECTS)
                return 0;
            sx[outCount] = xArr[i];
            sy[outCount] = curY;
            sw[outCount] = wArr[i];
            sh[outCount] = stripH;
            outCount++;
            curY = (sint16)(curY + stripH);
            remaining = (uint16)(remaining - stripH);
        }
    }

    return grSaveCleanBgRects(sx, sy, sw, sh, outCount);
}

/* Per-frame: restore each clean rect from its saved buffer into bg tiles.
 * Caller uses this INSTEAD of grRestoreBgTiles when in rect-mode. */
void grRestoreBgFromRects(void)
{
    int i;
    /* Clear currDirty at start of new frame, mirroring grRestoreBgTiles. */
    grClearCurrDirtyState();
    for (i = 0; i < gGrCleanRectCount; i++) {
        if (gGrCleanRects[i].pixels)
            grCleanRectCopyIn(&gGrCleanRects[i]);
    }
}

/*
 * Free clean tile copies to reclaim memory (~600KB).
 * Called when switching to non-island (black) backgrounds where clean copies aren't needed.
 */
void grFreeCleanBgTiles(void)
{
    if (bgTile0Clean) { free(bgTile0Clean); bgTile0Clean = NULL; }
    if (bgTile1Clean) { free(bgTile1Clean); bgTile1Clean = NULL; }
    if (bgTile3Clean) { free(bgTile3Clean); bgTile3Clean = NULL; }
    if (bgTile4Clean) { free(bgTile4Clean); bgTile4Clean = NULL; }

    /* No clean copies → force full upload on next frame */
    grMarkAllTilesDirty();
    grMarkPrevAllTilesDirty();
}

/*
 * Ensure clean background copies exist before frame restore/composite.
 */
void grEnsureCleanBgTiles(void)
{
    if (bgTile0Clean && bgTile1Clean && bgTile3Clean && bgTile4Clean) {
        return;
    }
    grSaveCleanBgTiles();
}

/*
 * Restore background tiles from clean copies (call at start of each frame).
 * Only restores rows that were dirtied by the previous frame's compositing.
 * prevDirty is preserved for grDrawBackground's union upload.
 */
void grRestoreBgTiles(void)
{
    PS1Surface *tiles[4] = { bgTile0, bgTile1, bgTile3, bgTile4 };
    const uint16 *clean[4] = { bgTile0Clean, bgTile1Clean, bgTile3Clean, bgTile4Clean };
    uint32 restoredBytes = 0;
    int perfTrack = ps1PerfEnabled;

    /* Clear currDirty for new frame's compositing */
    grClearCurrDirtyState();

    for (int i = 0; i < 4; i++) {
        if (!tiles[i] || !tiles[i]->pixels || !clean[i]) continue;

        int minY = prevDirtyMinY[i];
        int maxY = prevDirtyMaxY[i];
        uint32 w = tiles[i]->width;
        if (minY < 0) continue;  /* tile was clean last frame */

        for (int row = minY; row <= maxY; row++) {
            int minX = prevDirtyRowMinX[i][row];
            int maxX = prevDirtyRowMaxX[i][row];
            int copyWidth;
            uint32 copyBytes;
            uint16 *dst;
            const uint16 *src;

            if (minX < 0)
                continue;
            if (maxX < minX || maxX >= (int)w) {
                minX = 0;
                maxX = (int)w - 1;
            }

            copyWidth = maxX - minX + 1;
            copyBytes = (uint32)copyWidth * sizeof(uint16);
            dst = tiles[i]->pixels + (row * (int)w) + minX;
            src = clean[i] + (row * (int)w) + minX;
            memcpy(dst, src, (size_t)copyBytes);
            if (perfTrack)
                restoredBytes += copyBytes;
        }
    }

    if (perfTrack)
        ps1PerfMarkRestore(restoredBytes);
}

void grRestoreBackgroundRectForFrame(int x, int y, int width, int height)
{
    grClearCurrDirtyState();

    if (width <= 0 || height <= 0)
        return;

    grRestoreRectFromCleanBg(x, y, width, height);
}

void grRestoreAndCompositeDirect16BackgroundRectForFrame(int x, int y, int width, int height,
                                                         const uint16 *srcPixels)
{
    int rectEndX;
    int rectEndY;

    grClearCurrDirtyState();

    if (srcPixels == NULL || width <= 0 || height <= 0)
        return;

    if (x < 0) {
        width += x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        y = 0;
    }
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT)
        return;
    if (x + width > SCREEN_WIDTH)
        width = SCREEN_WIDTH - x;
    if (y + height > SCREEN_HEIGHT)
        height = SCREEN_HEIGHT - y;
    if (width <= 0 || height <= 0)
        return;

    rectEndX = x + width;
    rectEndY = y + height;

    if (x >= 0 && y >= 0 && rectEndX <= 640 && rectEndY <= 480) {
        int tileBaseX = (x >= 320) ? 320 : 0;
        if (rectEndX <= tileBaseX + 320) {
            grMarkSingleColumnDirty(tileBaseX, x, width, y, rectEndY);
            if (y < 240 && rectEndY > 240) {
                PS1Surface *topTile = (tileBaseX == 0) ? bgTile0 : bgTile1;
                PS1Surface *bottomTile = (tileBaseX == 0) ? bgTile3 : bgTile4;
                const uint16 *topClean = (tileBaseX == 0) ? bgTile0Clean : bgTile1Clean;
                const uint16 *bottomClean = (tileBaseX == 0) ? bgTile3Clean : bgTile4Clean;
                int tileLocalX = x - tileBaseX;
                int topRows = 240 - y;
                if (topRows < 0)
                    topRows = 0;
                if (topRows > height)
                    topRows = height;

                grEnsureCleanBgTiles();

                if (topTile != NULL && topTile->pixels != NULL && topClean != NULL) {
                    for (int row = 0; row < topRows; row++) {
                        uint16 *dst = topTile->pixels + ((y + row) * (int)topTile->width) + tileLocalX;
                        const uint16 *clean = topClean + ((y + row) * (int)topTile->width) + tileLocalX;
                        const uint16 *src = srcPixels + ((uint32)row * (uint32)width);
                        memcpy(dst, clean, (size_t)width * sizeof(uint16));
                        compositeDirectOpaqueRuns(dst, src, width);
                    }
                }
                if (bottomTile != NULL && bottomTile->pixels != NULL && bottomClean != NULL) {
                    for (int row = 0; row < (height - topRows); row++) {
                        uint16 *dst = bottomTile->pixels + (row * (int)bottomTile->width) + tileLocalX;
                        const uint16 *clean = bottomClean + (row * (int)bottomTile->width) + tileLocalX;
                        const uint16 *src = srcPixels + ((uint32)(topRows + row) * (uint32)width);
                        memcpy(dst, clean, (size_t)width * sizeof(uint16));
                        compositeDirectOpaqueRuns(dst, src, width);
                    }
                }
                return;
            }

            {
                PS1Surface *tile;
                const uint16 *clean;
                int tileLocalX = x - tileBaseX;
                int startLocalY;

                if (y < 240) {
                    tile = (tileBaseX == 0) ? bgTile0 : bgTile1;
                    clean = (tileBaseX == 0) ? bgTile0Clean : bgTile1Clean;
                    startLocalY = y;
                } else {
                    tile = (tileBaseX == 0) ? bgTile3 : bgTile4;
                    clean = (tileBaseX == 0) ? bgTile3Clean : bgTile4Clean;
                    startLocalY = y - 240;
                }

                grEnsureCleanBgTiles();

                if (tile == NULL || tile->pixels == NULL || clean == NULL)
                    return;

                for (int row = 0; row < height; row++) {
                    uint16 *dst = tile->pixels + ((startLocalY + row) * (int)tile->width) + tileLocalX;
                    const uint16 *cleanRow = clean + ((startLocalY + row) * (int)tile->width) + tileLocalX;
                    const uint16 *srcRow = srcPixels + ((uint32)row * (uint32)width);
                    memcpy(dst, cleanRow, (size_t)width * sizeof(uint16));
                    compositeDirectOpaqueRuns(dst, srcRow, width);
                }
            }
            return;
        }
    }

    grEnsureCleanBgTiles();
    grMarkRectDirty(x, y, rectEndX, rectEndY);
    for (int row = y; row < rectEndY; row++) {
        int tileLocalY;
        PS1Surface *tileLeft;
        PS1Surface *tileRight;
        const uint16 *cleanLeft;
        const uint16 *cleanRight;
        int destStartX = x;
        int destEndX = rectEndX;
        int srcRow = row - y;

        if (row < 240) {
            tileLocalY = row;
            tileLeft = bgTile0;
            tileRight = bgTile1;
            cleanLeft = bgTile0Clean;
            cleanRight = bgTile1Clean;
        } else {
            tileLocalY = row - 240;
            tileLeft = bgTile3;
            tileRight = bgTile4;
            cleanLeft = bgTile3Clean;
            cleanRight = bgTile4Clean;
        }

        if (tileLeft != NULL && tileLeft->pixels != NULL && cleanLeft != NULL && destStartX < 320) {
            int lx0 = destStartX;
            int lx1 = (destEndX < 320) ? destEndX : 320;
            uint16 *dst = tileLeft->pixels + (tileLocalY * (int)tileLeft->width) + lx0;
            const uint16 *clean = cleanLeft + (tileLocalY * (int)tileLeft->width) + lx0;
            const uint16 *src = srcPixels + ((uint32)srcRow * (uint32)width) + (uint32)(lx0 - x);
            int span = lx1 - lx0;
            memcpy(dst, clean, (size_t)span * sizeof(uint16));
            compositeDirectOpaqueRuns(dst, src, span);
        }

        if (tileRight != NULL && tileRight->pixels != NULL && cleanRight != NULL && destEndX > 320) {
            int rx0 = (destStartX > 320) ? destStartX : 320;
            int rx1 = destEndX;
            uint16 *dst = tileRight->pixels + (tileLocalY * (int)tileRight->width) + (rx0 - 320);
            const uint16 *clean = cleanRight + (tileLocalY * (int)tileRight->width) + (rx0 - 320);
            const uint16 *src = srcPixels + ((uint32)srcRow * (uint32)width) + (uint32)(rx0 - x);
            int span = rx1 - rx0;
            memcpy(dst, clean, (size_t)span * sizeof(uint16));
            compositeDirectOpaqueRuns(dst, src, span);
        }
    }
}

static void grRestoreTileRect(PS1Surface *dstTile,
                              const uint16 *srcClean,
                              int tileScreenX,
                              int tileScreenY,
                              int rectX,
                              int rectY,
                              int rectW,
                              int rectH)
{
    int copyStartX;
    int copyStartY;
    int copyEndX;
    int copyEndY;
    int row;

    if (dstTile == NULL || dstTile->pixels == NULL || srcClean == NULL)
        return;
    if (rectW <= 0 || rectH <= 0)
        return;

    copyStartX = (rectX > tileScreenX) ? rectX : tileScreenX;
    copyStartY = (rectY > tileScreenY) ? rectY : tileScreenY;
    copyEndX = rectX + rectW;
    copyEndY = rectY + rectH;

    if (copyEndX > tileScreenX + (int)dstTile->width)
        copyEndX = tileScreenX + (int)dstTile->width;
    if (copyEndY > tileScreenY + (int)dstTile->height)
        copyEndY = tileScreenY + (int)dstTile->height;

    if (copyStartX >= copyEndX || copyStartY >= copyEndY)
        return;

    for (row = copyStartY; row < copyEndY; row++) {
        int tileRow = row - tileScreenY;
        int tileCol = copyStartX - tileScreenX;
        int copyWidth = copyEndX - copyStartX;
        uint16 *dst = dstTile->pixels + (tileRow * dstTile->width) + tileCol;
        const uint16 *src = srcClean + (tileRow * dstTile->width) + tileCol;
        memcpy(dst, src, (size_t)copyWidth * sizeof(uint16));
    }
}

static void grRestoreRectSingleColumn(const uint16 *srcCleanTop,
                                      PS1Surface *dstTop,
                                      const uint16 *srcCleanBottom,
                                      PS1Surface *dstBottom,
                                      int tileBaseX,
                                      int x,
                                      int y,
                                      int width,
                                      int height)
{
    int tileLocalX;
    int topRows;
    int bottomRows;

    if (dstTop == NULL || dstBottom == NULL ||
        dstTop->pixels == NULL || dstBottom->pixels == NULL ||
        srcCleanTop == NULL || srcCleanBottom == NULL) {
        return;
    }

    tileLocalX = x - tileBaseX;
    topRows = 240 - y;
    if (topRows < 0)
        topRows = 0;
    if (topRows > height)
        topRows = height;
    bottomRows = height - topRows;

    for (int row = 0; row < topRows; row++) {
        int tileLocalY = y + row;
        uint16 *dst = dstTop->pixels + (tileLocalY * (int)dstTop->width) + tileLocalX;
        const uint16 *src = srcCleanTop + (tileLocalY * (int)dstTop->width) + tileLocalX;
        memcpy(dst, src, (size_t)width * sizeof(uint16));
    }

    for (int row = 0; row < bottomRows; row++) {
        uint16 *dst = dstBottom->pixels + (row * (int)dstBottom->width) + tileLocalX;
        const uint16 *src = srcCleanBottom + (row * (int)dstBottom->width) + tileLocalX;
        memcpy(dst, src, (size_t)width * sizeof(uint16));
    }
}

static void grRestoreRectFromCleanBg(int x, int y, int width, int height)
{
    int rectEndX;
    int rectEndY;

    if (width <= 0 || height <= 0)
        return;

    if (x < 0) {
        width += x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        y = 0;
    }
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT)
        return;
    if (x + width > SCREEN_WIDTH)
        width = SCREEN_WIDTH - x;
    if (y + height > SCREEN_HEIGHT)
        height = SCREEN_HEIGHT - y;

    rectEndX = x + width;
    rectEndY = y + height;

    /* Tile pixels are being modified — mark dirty for upload */
    if (x >= 0 && y >= 0 && rectEndX <= 640 && rectEndY <= 480) {
        int tileBaseX = (x >= 320) ? 320 : 0;
        if (rectEndX <= tileBaseX + 320) {
            grMarkSingleColumnDirty(tileBaseX, x, width, y, rectEndY);
            if (width <= 0 || height <= 0)
                return;

            grEnsureCleanBgTiles();
            if (y < 240 && rectEndY > 240) {
                if (tileBaseX == 0) {
                    grRestoreRectSingleColumn(bgTile0Clean, bgTile0,
                                              bgTile3Clean, bgTile3,
                                              tileBaseX, x, y, width, height);
                } else {
                    grRestoreRectSingleColumn(bgTile1Clean, bgTile1,
                                              bgTile4Clean, bgTile4,
                                              tileBaseX, x, y, width, height);
                }
                return;
            }

            {
                PS1Surface *tile;
                const uint16 *clean;
                int tileLocalX = x - tileBaseX;
                int startLocalY;

                if (y < 240) {
                    tile = (tileBaseX == 0) ? bgTile0 : bgTile1;
                    clean = (tileBaseX == 0) ? bgTile0Clean : bgTile1Clean;
                    startLocalY = y;
                } else {
                    tile = (tileBaseX == 0) ? bgTile3 : bgTile4;
                    clean = (tileBaseX == 0) ? bgTile3Clean : bgTile4Clean;
                    startLocalY = y - 240;
                }

                if (tile == NULL || tile->pixels == NULL || clean == NULL)
                    return;

                for (int row = 0; row < height; row++) {
                    int tileLocalY = startLocalY + row;
                    uint16 *dst = tile->pixels + (tileLocalY * (int)tile->width) + tileLocalX;
                    const uint16 *src = clean + (tileLocalY * (int)tile->width) + tileLocalX;
                    memcpy(dst, src, (size_t)width * sizeof(uint16));
                }
            }
            return;
        }
    }

    grMarkRectDirty(x, y, rectEndX, rectEndY);
    if (width <= 0 || height <= 0)
        return;

    grEnsureCleanBgTiles();

    grRestoreTileRect(bgTile0, bgTile0Clean, 0, 0, x, y, width, height);
    grRestoreTileRect(bgTile1, bgTile1Clean, 320, 0, x, y, width, height);
    grRestoreTileRect(bgTile3, bgTile3Clean, 0, 240, x, y, width, height);
    grRestoreTileRect(bgTile4, bgTile4Clean, 320, 240, x, y, width, height);
}

static void grCommitTileRectToClean(PS1Surface *srcTile,
                                    uint16 *dstClean,
                                    int tileScreenX,
                                    int tileScreenY,
                                    int rectX,
                                    int rectY,
                                    int rectW,
                                    int rectH)
{
    int copyStartX;
    int copyStartY;
    int copyEndX;
    int copyEndY;
    int row;

    if (srcTile == NULL || srcTile->pixels == NULL || dstClean == NULL)
        return;
    if (rectW <= 0 || rectH <= 0)
        return;

    copyStartX = (rectX > tileScreenX) ? rectX : tileScreenX;
    copyStartY = (rectY > tileScreenY) ? rectY : tileScreenY;
    copyEndX = rectX + rectW;
    copyEndY = rectY + rectH;

    if (copyEndX > tileScreenX + (int)srcTile->width)
        copyEndX = tileScreenX + (int)srcTile->width;
    if (copyEndY > tileScreenY + (int)srcTile->height)
        copyEndY = tileScreenY + (int)srcTile->height;

    if (copyStartX >= copyEndX || copyStartY >= copyEndY)
        return;

    for (row = copyStartY; row < copyEndY; row++) {
        int tileRow = row - tileScreenY;
        int tileCol = copyStartX - tileScreenX;
        int copyWidth = copyEndX - copyStartX;
        const uint16 *src = srcTile->pixels + (tileRow * srcTile->width) + tileCol;
        uint16 *dst = dstClean + (tileRow * srcTile->width) + tileCol;
        memcpy(dst, src, (size_t)copyWidth * sizeof(uint16));
    }
}

static void grCommitRectToCleanBg(int x, int y, int width, int height)
{
    if (width <= 0 || height <= 0)
        return;

    if (x < 0) {
        width += x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        y = 0;
    }
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT)
        return;
    if (x + width > SCREEN_WIDTH)
        width = SCREEN_WIDTH - x;
    if (y + height > SCREEN_HEIGHT)
        height = SCREEN_HEIGHT - y;
    if (width <= 0 || height <= 0)
        return;

    grEnsureCleanBgTiles();
    grCommitTileRectToClean(bgTile0, bgTile0Clean, 0, 0, x, y, width, height);
    grCommitTileRectToClean(bgTile1, bgTile1Clean, 320, 0, x, y, width, height);
    grCommitTileRectToClean(bgTile3, bgTile3Clean, 0, 240, x, y, width, height);
    grCommitTileRectToClean(bgTile4, bgTile4Clean, 320, 240, x, y, width, height);
}

/*
 * Clear screen to black
 */
void grClearScreen(PS1Surface *sfc)
{
    /* Clear entire screen using TILE primitive */
    grDrawRect(sfc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
}

/*
 * Draw background surface to screen
 * Re-LoadImages all tiles to framebuffer to restore background each frame.
 * Required when isbg=0 to erase previous frame's sprites.
 */
__attribute__((optimize("Os")))
void grDrawBackground(void)
{
    enum {
        GR_MAX_UPLOAD_RECTS = 8,
        GR_UPLOAD_BAND_MERGE_GAP = 0
    };
    /* Upload only dirty rows: union(prevDirty, currDirty) per tile.
     * prevDirty = rows restored at frame start (framebuffer still has old content).
     * currDirty = rows composited this frame (framebuffer has clean/old content). */
    PS1Surface *tiles[4] = { bgTile0, bgTile1, bgTile3, bgTile4 };
    RECT rect;
    int minYs[4];
    int maxYs[4];
    int bandTile[GR_MAX_UPLOAD_RECTS];
    int bandMinY[GR_MAX_UPLOAD_RECTS];
    int bandMaxY[GR_MAX_UPLOAD_RECTS];
    int bandCount = 0;
    int capped = 0;
    int dirtyCount = 0;
    uint16 uploadRects = 0;
    uint16 uploadRows = 0;
    uint32 uploadBytes = 0;
    uint32 perfStartTick = 0;

    for (int i = 0; i < 4; i++) {
        int minY = -1;
        int maxY = -1;

        minYs[i] = -1;
        maxYs[i] = -1;
        if (!tiles[i] || !tiles[i]->pixels)
            continue;

        /* Compute upload range = union(prevDirty, currDirty) */
        if (prevDirtyMinY[i] >= 0) {
            minY = prevDirtyMinY[i];
            maxY = prevDirtyMaxY[i];
        }
        if (currDirtyMinY[i] >= 0) {
            if (minY < 0) {
                minY = currDirtyMinY[i];
                maxY = currDirtyMaxY[i];
            } else {
                if (currDirtyMinY[i] < minY) minY = currDirtyMinY[i];
                if (currDirtyMaxY[i] > maxY) maxY = currDirtyMaxY[i];
            }
        }
        if (minY < 0) continue;  /* tile is fully clean — skip upload */

        minYs[i] = minY;
        maxYs[i] = maxY;
        dirtyCount++;
    }

    for (int i = 0; i < 4 && !capped; i++) {
        int y;

        if (minYs[i] < 0)
            continue;

        y = minYs[i];
        while (y <= maxYs[i]) {
            int startY;

            while (y <= maxYs[i] &&
                   prevDirtyRowMinX[i][y] < 0 &&
                   currDirtyRowMinX[i][y] < 0) {
                y++;
            }
            if (y > maxYs[i])
                break;

            startY = y;
            {
                int scanY = y + 1;
                int lastDirtyY = y;
                int cleanGap = 0;

                while (scanY <= maxYs[i]) {
                    if (prevDirtyRowMinX[i][scanY] >= 0 ||
                        currDirtyRowMinX[i][scanY] >= 0) {
                        lastDirtyY = scanY;
                        cleanGap = 0;
                    } else {
                        cleanGap++;
                        if (cleanGap > GR_UPLOAD_BAND_MERGE_GAP)
                            break;
                    }
                    scanY++;
                }
                y = lastDirtyY;
            }

            if (bandCount >= GR_MAX_UPLOAD_RECTS) {
                capped = 1;
                break;
            }
            bandTile[bandCount] = i;
            bandMinY[bandCount] = startY;
            bandMaxY[bandCount] = y;
            bandCount++;
            y++;
        }
    }

    if (ps1PerfEnabled && dirtyCount > 0)
        perfStartTick = ps1PerfTick();

    if (!capped && bandCount > 0) {
        for (int b = 0; b < bandCount; b++) {
            int i = bandTile[b];
            int minY = bandMinY[b];
            int h = bandMaxY[b] - minY + 1;
            uint32 w = tiles[i]->width;

            setRECT(&rect, (i & 1) ? 320 : 0, ((i & 2) ? 240 : 0) + minY, w, h);
            uploadRects++;
            uploadRows = (uint16)(uploadRows + (uint16)h);
            uploadBytes += (uint32)w * (uint32)h * sizeof(uint16);
            LoadImage(&rect, (uint32 *)(tiles[i]->pixels + minY * w));
        }

        DrawSync(0);
    } else {
        for (int i = 0; i < 4; i++) {
            if (minYs[i] < 0)
                continue;

            int minY = minYs[i];
            int h = maxYs[i] - minY + 1;
            uint32 w = tiles[i]->width;

            setRECT(&rect, (i & 1) ? 320 : 0, ((i & 2) ? 240 : 0) + minY, w, h);
            uploadRects++;
            uploadRows = (uint16)(uploadRows + (uint16)h);
            uploadBytes += (uint32)w * (uint32)h * sizeof(uint16);
            LoadImage(&rect, (uint32 *)(tiles[i]->pixels + minY * w));
        }

        if (dirtyCount > 0)
            DrawSync(0);
    }

    if (ps1PerfEnabled && dirtyCount > 0) {
        ps1PerfMarkUploadDirty(uploadRects, uploadRows, uploadBytes,
                               ps1PerfElapsedVBlanks(perfStartTick));
    }

    /* Advance dirty state: this frame's compositing becomes next frame's restore set. */
    grPromoteCurrDirtyToPrev();
}

/*
 * Fade out effect
 */
void grFadeOut()
{
    /* Force full dirty for fade — modifies all pixels */
    grMarkAllTilesDirty();
    grMarkPrevAllTilesDirty();

    /* 16 fade steps, ~2 frames each = ~0.5 sec at 60fps.
     * Uses (c >> 1) & 0x3DEF to halve all 3 color channels simultaneously:
     * the mask prevents bit leakage between R/G/B fields and clears STP. */
    for (int step = 0; step < 16; step++) {
        PS1Surface *tiles[] = { bgTile0, bgTile1, bgTile3, bgTile4 };
        for (int t = 0; t < 4; t++) {
            if (!tiles[t] || !tiles[t]->pixels) continue;
            uint32 count = tiles[t]->width * tiles[t]->height;
            uint32 *px32 = (uint32 *)tiles[t]->pixels;
            uint32 count32 = count >> 1;
            /* Process 2 pixels per uint32.
             * (c >> 1) & 0x3DEF works per-pixel; the mask at bit 15
             * also prevents leakage between the two packed pixels. */
            for (uint32 i = 0; i < count32; i++) {
                px32[i] = (px32[i] >> 1) & 0x3DEF3DEFu;
            }
        }

        /* Mark all dirty for each step's upload */
        grMarkAllTilesDirty();

        VSync(0);
        grDrawBackground();
    }
}

/* Helper to free a tile.
 *
 * Round 33: pixels + struct live in MEM_REGION_TRANSIENT post-Round-33.
 * memFree(TRANSIENT, ptr) only decrements the scene-alloc balance — the
 * underlying bytes survive until the next memSceneReset wipes the whole
 * region wholesale. That's intentional: between explicit freeBgTile calls
 * and the next scene boundary, freed tiles' bytes are dead but the
 * TRANSIENT bump pointer doesn't reclaim them (this is normal TRANSIENT
 * semantics). The bump pointer rewinds on memSceneReset.
 *
 * Pre-memInit allocations (the libc-fallback safe_malloc branch in
 * createEmptyBgTileRAM) are never actually hit in practice — bg-tiles
 * are first allocated by graphicsInit(), which runs after memInit().
 * The `else free()` path remains as defensive code for any future
 * pre-memInit caller. */
static void freeBgTile(PS1Surface **tile)
{
    if (*tile != NULL) {
        if ((*tile)->pixels) {
            if (memIsReady()) memFree(MEM_REGION_TRANSIENT, (*tile)->pixels);
            else free((*tile)->pixels);
        }
        if (memIsReady()) memFree(MEM_REGION_TRANSIENT, *tile);
        else free(*tile);
        *tile = NULL;
    }
}

void grReleaseBackgroundTiles(void)
{
    grFreeCleanBgTiles();
    grFreeCleanBgRects();

    freeBgTile(&bgTile0);
    freeBgTile(&bgTile1);
    freeBgTile(&bgTile2a);
    freeBgTile(&bgTile2b);
    freeBgTile(&bgTile3);
    freeBgTile(&bgTile4);
    freeBgTile(&bgTile5a);
    freeBgTile(&bgTile5b);

    grBackgroundSfc = NULL;

    if (grSavedZonesLayer != NULL) {
        grFreeLayer(grSavedZonesLayer);
        grSavedZonesLayer = NULL;
    }

    grForceFullRedrawNextFrame();
}

/* Round 33: per-scene TRANSIENT wipe hook.
 *
 * The bg-tile struct AND pixel buffer both live in MEM_REGION_TRANSIENT
 * (post-Round-33 migration), so memSceneReset has already reclaimed
 * every byte. The static slot pointers still hold the dangling
 * addresses; we NULL them so the next grLoadScreen / grInitEmptyBackground
 * sees them as "uninitialised" and re-allocates fresh in the new
 * TRANSIENT bump frame.
 *
 * Do NOT call freeBgTile here — that would dereference the dangling
 * struct (to read its pixels pointer) and call memFree(TRANSIENT, ...)
 * which would decrement the scene-alloc balance that memSceneReset
 * already zeroed. */
void grBackgroundTilesAssumeWiped(void)
{
    bgTile0 = NULL;
    bgTile1 = NULL;
    bgTile2a = NULL;
    bgTile2b = NULL;
    bgTile3 = NULL;
    bgTile4 = NULL;
    bgTile5a = NULL;
    bgTile5b = NULL;
    /* grBackgroundSfc points at one of the bgTile slots when a scene
     * is active. After the wipe the bytes behind it are gone — NULL it
     * so callers see "no background yet" and grLoadScreen / friends
     * repopulate it. */
    grBackgroundSfc = NULL;
    /* Force a full first-frame upload after the new scene's tiles
     * are populated; we know every pixel just got wiped. */
    grForceFullRedrawNextFrame();
}

/*
 * Helper: Create a background tile stored in RAM only (no VRAM upload)
 * For use with LoadImage directly to framebuffer
 * srcHeight parameter allows partial source data (rest filled with black)
 */
static void fillBgTileRAMPartial(PS1Surface *tile, uint8 *src,
                                 uint16 srcWidth, uint16 srcHeight,
                                 uint16 srcStartX, uint16 srcStartY,
                                 uint16 tileWidth)
{
    uint16 *dst = tile->pixels;

    /* Process 2 pixels per byte using palette LUT, with bounds checking.
     * Row base increment avoids per-pixel multiply. */
    for (uint16 y = 0; y < BG_TILE_HEIGHT; y++) {
        uint32 srcY = srcStartY + y;
        uint16 *dstRow = dst + (uint32)y * tileWidth;
        if (srcY >= srcHeight) {
            /* Entire row is outside source bounds — fill with black */
            memset(dstRow, 0, tileWidth * sizeof(uint16));
            continue;
        }
        uint32 srcOff = srcY * (uint32)srcWidth + srcStartX;
        /* Calculate how many pixels are within source bounds */
        uint16 validW = (srcStartX + tileWidth <= srcWidth) ? tileWidth
                        : (srcStartX < srcWidth ? srcWidth - srcStartX : 0);
        uint16 x = 0;
        /* Handle odd start pixel */
        if ((srcStartX & 1) && x < validW) {
            uint8 packed = src[srcOff >> 1];
            dstRow[0] = ttmPalette[packed & 0x0F];
            x = 1;
            srcOff++;
        }
        /* Process 2 pixels per byte */
        for (; x + 1 < validW; x += 2, srcOff += 2) {
            uint32 pair = palLutSierra[src[srcOff >> 1]];
            dstRow[x]     = (uint16)pair;
            dstRow[x + 1] = (uint16)(pair >> 16);
        }
        /* Handle trailing valid pixel */
        if (x < validW) {
            uint8 packed = src[srcOff >> 1];
            dstRow[x] = ttmPalette[(packed >> 4) & 0x0F];
            x++;
        }
        /* Fill remaining with black */
        if (x < tileWidth) {
            memset(&dstRow[x], 0, (tileWidth - x) * sizeof(uint16));
        }
    }

}

static PS1Surface *ensureBgTileRAMPartial(PS1Surface **slot, uint8 *src,
                                           uint16 srcWidth, uint16 srcHeight,
                                           uint16 srcStartX, uint16 srcStartY,
                                           uint16 tileWidth)
{
    if (*slot == NULL || (*slot)->pixels == NULL ||
        (*slot)->width != tileWidth || (*slot)->height != BG_TILE_HEIGHT) {
        ensureBgTileRAM(slot, tileWidth, BG_TILE_HEIGHT);
    } else {
        resetBgTileRAMFields(*slot, tileWidth, BG_TILE_HEIGHT);
    }

    fillBgTileRAMPartial(*slot, src, srcWidth, srcHeight,
                         srcStartX, srcStartY, tileWidth);
    return *slot;
}

static void grExpandScrPackedRow(uint16 *dst, const uint8 *src, uint16 byteCount)
{
    uint16 x = 0;
    for (uint16 i = 0; i < byteCount; i++, x += 2) {
        uint32 pair = palLutSierra[src[i]];
        dst[x] = (uint16)pair;
        dst[x + 1] = (uint16)(pair >> 16);
    }
}

static int grIsFullScreenScrResource(const struct TScrResource *scrResource)
{
    return scrResource != NULL &&
           scrResource->width == 640 &&
           scrResource->height == 480 &&
           scrResource->uncompressedSize == 153600UL;
}

static void grCopyFullScreenScrName(char *dst, uint32 dstBytes,
                                    const char *src)
{
    uint32 i;

    if (dst == NULL || dstBytes == 0)
        return;
    if (src == NULL) {
        dst[0] = '\0';
        return;
    }
    for (i = 0; i + 1 < dstBytes && src[i] != '\0'; i++)
        dst[i] = src[i];
    dst[i] = '\0';
}

static int grEnsureFullScreenScrCache(const char *resName, uint32 bytes)
{
    if (!gFullScreenScrCacheEnabled ||
        resName == NULL ||
        bytes == 0)
        return 0;

    if (gFullScreenScrCache != NULL &&
        gFullScreenScrCacheBytes >= bytes)
        return 1;

    if (gFullScreenScrCache != NULL) {
        memFree(MEM_REGION_CACHE, gFullScreenScrCache);
        gFullScreenScrCache = NULL;
    }
    gFullScreenScrCacheBytes = 0;
    gFullScreenScrCacheValid = 0;
    gFullScreenScrCacheName[0] = '\0';

    /* MEM_REGION_RATIONALE: optional packed full-screen SCR cache for the
     * async-loading branch. Stores 4bpp source bytes (150KB), not the 600KB
     * expanded bg tiles, and is freed when the proof toggle is disabled. */
    gFullScreenScrCache = (uint8 *)memAlloc(MEM_REGION_CACHE, bytes,
                                            "gr-scr-cache");
    if (gFullScreenScrCache == NULL)
        return 0;
    gFullScreenScrCacheBytes = bytes;
    grCopyFullScreenScrName(gFullScreenScrCacheName,
                            sizeof(gFullScreenScrCacheName),
                            resName);
    return 1;
}

static void grExpandFullScreenScrPackedToTiles(const uint8 *packed)
{
    uint16 y;

    ensureBgTileRAM(&bgTile0, 320, BG_TILE_HEIGHT);
    ensureBgTileRAM(&bgTile1, 320, BG_TILE_HEIGHT);
    ensureBgTileRAM(&bgTile3, 320, BG_TILE_HEIGHT);
    ensureBgTileRAM(&bgTile4, 320, BG_TILE_HEIGHT);
    bgTile2a = NULL;
    bgTile2b = NULL;
    bgTile5a = NULL;
    bgTile5b = NULL;

    for (y = 0; y < 480; y++) {
        const uint8 *srcRow = packed + ((uint32)y * SCR_STREAM_ROW_BYTES);
        if (y < BG_TILE_HEIGHT) {
            grExpandScrPackedRow(bgTile0->pixels + ((uint32)y * 320),
                                 srcRow, 160);
            grExpandScrPackedRow(bgTile1->pixels + ((uint32)y * 320),
                                 srcRow + 160, 160);
        } else {
            uint16 tileY = (uint16)(y - BG_TILE_HEIGHT);
            grExpandScrPackedRow(bgTile3->pixels + ((uint32)tileY * 320),
                                 srcRow, 160);
            grExpandScrPackedRow(bgTile4->pixels + ((uint32)tileY * 320),
                                 srcRow + 160, 160);
        }
    }
}

static void grUploadFullScreenBgTilesIfNeeded(void)
{
    RECT rect0, rect1, rect3, rect4;

    if (!grPresentDuringScreenLoad)
        return;

    if (bgTile0 && bgTile0->pixels) {
        setRECT(&rect0, 0, 0, bgTile0->width, bgTile0->height);
        LoadImage(&rect0, (uint32*)bgTile0->pixels);
    }
    if (bgTile1 && bgTile1->pixels) {
        setRECT(&rect1, 320, 0, bgTile1->width, bgTile1->height);
        LoadImage(&rect1, (uint32*)bgTile1->pixels);
    }
    if (bgTile3 && bgTile3->pixels) {
        setRECT(&rect3, 0, 240, bgTile3->width, bgTile3->height);
        LoadImage(&rect3, (uint32*)bgTile3->pixels);
    }
    if (bgTile4 && bgTile4->pixels) {
        setRECT(&rect4, 320, 240, bgTile4->width, bgTile4->height);
        LoadImage(&rect4, (uint32*)bgTile4->pixels);
    }
    DrawSync(0);
}

static int grLoadFullScreenScrCached(struct TScrResource *scrResource)
{
    if (!grIsFullScreenScrResource(scrResource) ||
        !gFullScreenScrCacheEnabled ||
        !gFullScreenScrCacheValid ||
        gFullScreenScrCache == NULL ||
        gFullScreenScrCacheBytes < scrResource->uncompressedSize ||
        strcmp(gFullScreenScrCacheName, scrResource->resName) != 0)
        return 0;

    grExpandFullScreenScrPackedToTiles(gFullScreenScrCache);
    grUploadFullScreenBgTilesIfNeeded();
    grBackgroundSfc = bgTile0;
    if (grSaveCleanOnScreenLoad)
        grSaveCleanBgTiles();
    printf("JCSCREEN scr-cache-apply %s bytes=%lu\n",
           scrResource->resName,
           (unsigned long)scrResource->uncompressedSize);
    return 1;
}

static int grLoadFullScreenScrStreamed(struct TScrResource *scrResource)
{
    CdlFILE cdfile;
    char path[32];
    uint16 y;
    int cacheTarget = 0;

    if (!grIsFullScreenScrResource(scrResource)) {
        return 0;
    }

    snprintf(path, sizeof(path), "SCR\\%s", scrResource->resName);
    if (!ps1_streamResolveFile(path, &cdfile))
        return 0;

    ensureBgTileRAM(&bgTile0, 320, BG_TILE_HEIGHT);
    ensureBgTileRAM(&bgTile1, 320, BG_TILE_HEIGHT);
    ensureBgTileRAM(&bgTile3, 320, BG_TILE_HEIGHT);
    ensureBgTileRAM(&bgTile4, 320, BG_TILE_HEIGHT);
    bgTile2a = NULL;
    bgTile2b = NULL;
    bgTile5a = NULL;
    bgTile5b = NULL;
    cacheTarget = grEnsureFullScreenScrCache(scrResource->resName,
                                             scrResource->uncompressedSize);

    for (y = 0; y < 480; y = (uint16)(y + SCR_STREAM_ROWS)) {
        uint16 rows = (uint16)((480 - y) > SCR_STREAM_ROWS ? SCR_STREAM_ROWS : (480 - y));
        uint32 offset = (uint32)y * SCR_STREAM_ROW_BYTES;
        uint32 bytes = (uint32)rows * SCR_STREAM_ROW_BYTES;

        if (!ps1_streamReadAlignedIntoFile(&cdfile, offset, bytes,
                                           gScrStreamRows)) {
            if (cacheTarget)
                gFullScreenScrCacheValid = 0;
            return 0;
        }
        if (cacheTarget)
            memcpy(gFullScreenScrCache + offset, gScrStreamRows, bytes);

        for (uint16 row = 0; row < rows; row++) {
            uint16 screenY = (uint16)(y + row);
            const uint8 *srcRow = gScrStreamRows + ((uint32)row * SCR_STREAM_ROW_BYTES);
            if (screenY < BG_TILE_HEIGHT) {
                grExpandScrPackedRow(bgTile0->pixels + ((uint32)screenY * 320), srcRow, 160);
                grExpandScrPackedRow(bgTile1->pixels + ((uint32)screenY * 320), srcRow + 160, 160);
            } else {
                uint16 tileY = (uint16)(screenY - BG_TILE_HEIGHT);
                grExpandScrPackedRow(bgTile3->pixels + ((uint32)tileY * 320), srcRow, 160);
                grExpandScrPackedRow(bgTile4->pixels + ((uint32)tileY * 320), srcRow + 160, 160);
            }
        }
    }

    if (cacheTarget) {
        gFullScreenScrCacheValid = 1;
        printf("JCSCREEN scr-cache-fill %s bytes=%lu\n",
               scrResource->resName,
               (unsigned long)scrResource->uncompressedSize);
    }
    return 1;
}

/*
 * Load a Scene Explorer thumbnail SCR — bypasses the Sierra
 * scrResources registry so we don't have to register synthetic SCR
 * resources for the 27 thumbnails (and grow as scenes validate).
 *
 * Format: raw 320x240 16-bit RGB555 LE, 153,600 bytes. The file is
 * streamed in 16-row chunks directly to the framebuffer so Scene
 * Explorer no longer needs a 153 KB heap allocation while paused. That
 * allocation was fragile after the runtime grew persistent scene caches
 * and could fail from heap fragmentation, leaving the explorer text-only.
 *
 * Returns 1 on success, 0 if the file isn't on disc.
 */
enum {
    SCENE_EXPLORER_THUMB_W = 320,
    SCENE_EXPLORER_THUMB_H = 240,
    SCENE_EXPLORER_THUMB_ROWS_PER_READ = 16,
};

static uint16 gSceneExplorerThumbChunk[
    SCENE_EXPLORER_THUMB_W * SCENE_EXPLORER_THUMB_ROWS_PER_READ
];

static void grClearSceneExplorerRect(int x, int y, int w, int h)
{
    RECT rect;

    memset(gSceneExplorerThumbChunk, 0, sizeof(gSceneExplorerThumbChunk));

    while (h > 0) {
        int rows = h;
        int clearX = x;
        int remainingW = w;

        if (rows > SCENE_EXPLORER_THUMB_ROWS_PER_READ)
            rows = SCENE_EXPLORER_THUMB_ROWS_PER_READ;

        while (remainingW > 0) {
            int chunkW = remainingW;
            if (chunkW > SCENE_EXPLORER_THUMB_W)
                chunkW = SCENE_EXPLORER_THUMB_W;

            setRECT(&rect, clearX, y, chunkW, rows);
            LoadImage(&rect, (uint32 *)gSceneExplorerThumbChunk);
            DrawSync(0);

            clearX += chunkW;
            remainingW -= chunkW;
        }

        y += rows;
        h -= rows;
    }
}

int grLoadSceneExplorerThumbnail(const char *slug)
{
    CdlFILE cdfile;
    char path[24];
    char family[16];
    int i, fam_len;
    const char *digits;
    const char *abbrev;
    RECT rect;

    if (slug == NULL || slug[0] == '\0')
        return 0;

    /* Split slug into family prefix + numeric tag.
     * "fishing1" -> family="fishing", digits="1" */
    fam_len = 0;
    while (slug[fam_len] >= 'a' && slug[fam_len] <= 'z'
           && fam_len < (int)sizeof(family) - 1)
        fam_len++;
    if (fam_len == 0 || fam_len >= (int)sizeof(family))
        return 0;
    for (i = 0; i < fam_len; i++) family[i] = slug[i];
    family[fam_len] = '\0';
    digits = slug + fam_len;
    if (*digits < '0' || *digits > '9')
        return 0;

    /* Family -> 2-letter abbrev. Must match FAMILY_ABBREV in
     * scripts/build-scene-explorer-thumbnails.py. */
    if      (!strcmp(family, "fishing"))  abbrev = "FI";
    else if (!strcmp(family, "johnny"))   abbrev = "JO";
    else if (!strcmp(family, "mary"))     abbrev = "MA";
    else if (!strcmp(family, "visitor"))  abbrev = "VI";
    else if (!strcmp(family, "activity")) abbrev = "AC";
    else if (!strcmp(family, "suzy"))     abbrev = "SU";
    else if (!strcmp(family, "miscgag"))  abbrev = "MG";
    else if (!strcmp(family, "stand"))    abbrev = "ST";
    else if (!strcmp(family, "walkstuf")) abbrev = "WK";
    else if (!strcmp(family, "building")) abbrev = "BL";
    else return 0;

    snprintf(path, sizeof(path), "SCR\\SX%s%s.SCR", abbrev, digits);
    if (!ps1_streamResolveFile(path, &cdfile))
        return 0;

    /* Clear the four bands around the centered thumbnail to black so
     * the previous menu/scene pixels don't show through. Widths greater
     * than 320 are split because the reusable chunk buffer is 320 pixels
     * wide. */
    grClearSceneExplorerRect(0, 0, 640, 120);
    grClearSceneExplorerRect(0, 360, 640, 120);
    grClearSceneExplorerRect(0, 120, 160, 240);
    grClearSceneExplorerRect(480, 120, 160, 240);

    for (int y = 0; y < SCENE_EXPLORER_THUMB_H;
         y += SCENE_EXPLORER_THUMB_ROWS_PER_READ) {
        uint32 offset = (uint32)y * SCENE_EXPLORER_THUMB_W * 2UL;
        uint32 bytes = SCENE_EXPLORER_THUMB_W *
                       SCENE_EXPLORER_THUMB_ROWS_PER_READ * 2UL;

        if (!ps1_streamReadAlignedIntoFile(&cdfile, offset, bytes,
                                           (uint8 *)gSceneExplorerThumbChunk))
            return 0;

        setRECT(&rect, 160, 120 + y,
                SCENE_EXPLORER_THUMB_W,
                SCENE_EXPLORER_THUMB_ROWS_PER_READ);
        LoadImage(&rect, (uint32 *)gSceneExplorerThumbChunk);
        DrawSync(0);
    }

    return 1;
}

void grFreeSceneExplorerThumbnailBuffer(void)
{
    /* Thumbnails now stream through a static 16-row chunk buffer, so
     * there is no per-entry heap state to release. Keep the function as
     * the pause-menu cleanup hook. */
}

/*
 * Load background screen
 */
void grLoadScreen(char *strArg)
{
    struct TScrResource *scrResource = findScrResource(strArg);
    if (scrResource == NULL) return;

    /* Determine partial height from metadata (available without loading data) */
    uint16 srcHeight = scrResource->height;
    int isPartialHeight = (srcHeight < 480);

    /* Drop clean copies before loading SCR data. Keep the 320x240 bg tile
     * buffers resident and refill them in place; repeated 153600-byte
     * free+malloc cycles fragment the PS1 heap during long screensaver runs. */
    grFreeCleanBgTiles();

    /* Retired split-tile layout; release any stale remnants but preserve the
     * active 320px tiles (bgTile0/1/3/4) for reuse. */
    freeBgTile(&bgTile2a);
    freeBgTile(&bgTile2b);

    freeBgTile(&bgTile5a);
    freeBgTile(&bgTile5b);

    grBackgroundSfc = NULL;

    if (grSavedZonesLayer != NULL) {
        grFreeLayer(grSavedZonesLayer);
        grSavedZonesLayer = NULL;
    }

    /* Full-screen ocean/night SCR files are fixed-size raw 4bpp payloads.
     * Stream them directly into bg tiles so the scene loop never needs a
     * 153600-byte temporary SCR heap allocation. */
    if (scrResource->uncompressedData == NULL &&
        grLoadFullScreenScrCached(scrResource)) {
        return;
    }

    if (scrResource->uncompressedData == NULL &&
        grLoadFullScreenScrStreamed(scrResource)) {
        grUploadFullScreenBgTilesIfNeeded();
        grBackgroundSfc = bgTile0;
        if (grSaveCleanOnScreenLoad)
            grSaveCleanBgTiles();
        return;
    }

    /* Partial/non-standard SCR files still use the legacy temporary load. */
    if (scrResource->uncompressedData == NULL) {
        ps1_loadScrData(scrResource);
    }
    if (scrResource->uncompressedData == NULL &&
        (bgTile0 != NULL || bgTile1 != NULL || bgTile3 != NULL || bgTile4 != NULL)) {
        /* Boot/menu paths can be tighter than steady-state scene playback.
         * Retry once with tile RAM released; steady-state FG2 should not
         * normally take this path. */
        freeBgTile(&bgTile0);
        freeBgTile(&bgTile1);
        freeBgTile(&bgTile3);
        freeBgTile(&bgTile4);
        ps1_loadScrData(scrResource);
    }
    if (scrResource->uncompressedData == NULL) {
        /* Failed to load — recreate empty tiles so rendering doesn't crash */
        ensureEmptyBgTileRAM(&bgTile0, 320, 240);
        ensureEmptyBgTileRAM(&bgTile1, 320, 240);
        if (!isPartialHeight) {
            ensureEmptyBgTileRAM(&bgTile3, 320, 240);
            ensureEmptyBgTileRAM(&bgTile4, 320, 240);
        }
        grBackgroundSfc = bgTile0;
        return;
    }

    if ((scrResource->width % 2) == 1) {
        GR_DIAG_PRINTF("Warning: grLoadScreen(): can't manage odd widths\n");
    }

    if (scrResource->width > 640 || scrResource->height > 480) {
        fatalError("grLoadScreen(): can't manage more than 640x480 resolutions");
    }

    uint16 srcWidth  = scrResource->width;
    uint8 *src = scrResource->uncompressedData;

    /* Create tiles for top row (y=0-239)
     * VRAM layout:
     * - Tile 0  (256x240) at VRAM(640, 4)   - srcX=0,   y=4-243
     * - Tile 1  (256x240) at VRAM(640, 244) - srcX=256, y=244-483
     * DEBUG: Test single 64px tile at x=896 to isolate VRAM issue
     */
    /* Top row: LoadImage directly to framebuffer at init
     * Use 2 tiles of 320px each to cover 640px total */
    bgTile0  = ensureBgTileRAMPartial(&bgTile0, src, srcWidth, srcHeight, 0,   0, 320);   /* top row, x=0-319 */
    bgTile1  = ensureBgTileRAMPartial(&bgTile1, src, srcWidth, srcHeight, 320, 0, 320);   /* top row, x=320-639 */
    bgTile2a = NULL;
    bgTile2b = NULL;

    if (grPresentDuringScreenLoad) {
        /* LoadImage top row directly to framebuffer
         * Use separate RECTs - LoadImage is async and may read RECT after return */
        RECT topRect0, topRect1;
        if (bgTile0 && bgTile0->pixels) {
            setRECT(&topRect0, 0, 0, bgTile0->width, bgTile0->height);
            LoadImage(&topRect0, (uint32*)bgTile0->pixels);
        }
        if (bgTile1 && bgTile1->pixels) {
            setRECT(&topRect1, 320, 0, bgTile1->width, bgTile1->height);
            LoadImage(&topRect1, (uint32*)bgTile1->pixels);
        }
        DrawSync(0);
    }

    /* Bottom row: Handle based on image height and existing tiles
     * For partial height images (like ISLETEMP 640x350), preserve existing ocean tiles */

    /* Calculate how many lines we can read for bottom half (y=240+) */
    uint16 bottomRowLines = (srcHeight > 240) ? (srcHeight - 240) : 0;

    if (bottomRowLines >= 240) {
        /* Full 640x480 image - create bottom row tiles (2x320 to match top row) */
        bgTile3  = ensureBgTileRAMPartial(&bgTile3, src, srcWidth, srcHeight, 0,   240, 320);
        bgTile4  = ensureBgTileRAMPartial(&bgTile4, src, srcWidth, srcHeight, 320, 240, 320);
        bgTile5a = NULL;
        bgTile5b = NULL;
    } else if (isPartialHeight && bgTile3 != NULL && bgTile4 != NULL &&
               bgTile3->pixels != NULL && bgTile4->pixels != NULL &&
               bottomRowLines > 0) {
        /* Partial height image with existing bottom tiles (ocean) - composite on top!
         * Copy the available rows (240 to srcHeight-1) onto existing tiles */
        uint16 *dst3 = bgTile3->pixels;
        uint16 *dst4 = bgTile4->pixels;

        for (uint16 y = 0; y < bottomRowLines && y < 240; y++) {
            uint16 srcY = 240 + y;
            for (uint16 x = 0; x < 320; x++) {
                /* Left tile (bgTile3) */
                uint32 srcOffset = (srcY * srcWidth + x) / 2;
                uint8 palIndex;
                if (x & 1) {
                    palIndex = src[srcOffset] & 0x0F;
                } else {
                    palIndex = (src[srcOffset] >> 4) & 0x0F;
                }
                dst3[y * 320 + x] = ttmPalette[palIndex & 0x0F];

                /* Right tile (bgTile4) */
                uint16 srcX = 320 + x;
                srcOffset = (srcY * srcWidth + srcX) / 2;
                if (srcX & 1) {
                    palIndex = src[srcOffset] & 0x0F;
                } else {
                    palIndex = (src[srcOffset] >> 4) & 0x0F;
                }
                dst4[y * 320 + x] = ttmPalette[palIndex & 0x0F];
            }
        }
    } else if (bottomRowLines > 0) {
        /* Partial bottom row with no existing tiles - create with partial data */
        bgTile3  = ensureBgTileRAMPartial(&bgTile3, src, srcWidth, srcHeight, 0,   240, 320);
        bgTile4  = ensureBgTileRAMPartial(&bgTile4, src, srcWidth, srcHeight, 320, 240, 320);
        bgTile5a = NULL;
        bgTile5b = NULL;
    } else {
        /* SCR is only 240 lines or less - create empty tiles for sprite compositing */
        ensureEmptyBgTileRAM(&bgTile3, 320, 240);
        ensureEmptyBgTileRAM(&bgTile4, 320, 240);
        bgTile5a = NULL;
        bgTile5b = NULL;
    }

    if (grPresentDuringScreenLoad) {
        /* Disable display during bottom row LoadImage to avoid tearing/corruption */
        SetDispMask(0);

        /* LoadImage bottom row tiles directly to framebuffer (2x320 layout)
         * Use separate RECTs - LoadImage is async and may read RECT after return */
        RECT botRect3, botRect4;

        if (bgTile3 && bgTile3->pixels) {
            setRECT(&botRect3, 0, 240, bgTile3->width, bgTile3->height);
            LoadImage(&botRect3, (uint32*)bgTile3->pixels);
        }
        if (bgTile4 && bgTile4->pixels) {
            setRECT(&botRect4, 320, 240, bgTile4->width, bgTile4->height);
            LoadImage(&botRect4, (uint32*)bgTile4->pixels);
        }

        DrawSync(0);  /* Sync bottom row uploads */
        SetDispMask(1);
    }

    /* Set grBackgroundSfc to first tile for compatibility with existing code */
    grBackgroundSfc = bgTile0;

    /* Free SCR data after converting - saves memory. The buffer comes
     * from ps1_streamReadCache → ps1_streamReadFromCdFile → memAlloc(CACHE)
     * post-memInit, so libc free() is a silent no-op for that range —
     * route through memFree(CACHE), which range-checks and dispatches
     * to free() for pre-memInit libc fallback pointers. Plugging this
     * leak is the difference between a soak that hits CACHE-exhaustion
     * after ~5 scene transitions and one that stays bounded. */
    if (scrResource->uncompressedData) {
        memFree(MEM_REGION_CACHE, scrResource->uncompressedData);
        scrResource->uncompressedData = NULL;
    }

    /* Save clean background tiles immediately after loading unless the
     * caller is about to use rect-mode snapshots. Freeplay loads an island
     * backdrop and then saves its own sparse clean rects; allocating four
     * full-tile backups here can exhaust the contiguous PS1 heap. */
    if (grSaveCleanOnScreenLoad)
        grSaveCleanBgTiles();
}

/*
 * Copy zone operations - minimal PS1 implementation.
 */
void grCopyZoneToBg(PS1Surface *sfc, uint16 x, uint16 y, uint16 width, uint16 height)
{
    int screenX;
    int screenY;

    (void)sfc;

    screenX = (int)x + grDx;
    screenY = (int)y + grDy;

    /* PS1 draws directly into the composited background tiles instead of
     * keeping a separate saved-zones overlay layer. Treat COPY_ZONE_TO_BG as
     * committing the current rectangle into the clean restore baseline. */
    grCommitRectToCleanBg(screenX, screenY, (int)width + 2, height);
}
void grSaveImage1(PS1Surface *sfc, uint16 x, uint16 y, uint16 width, uint16 height)
{
    /* Johnny's current use of SAVE_IMAGE1 is the same class of operation as
     * COPY_ZONE_TO_BG: define a bounded region that should survive subsequent
     * frame restores. Keep the PS1 behavior explicit and deterministic by
     * committing the current rectangle into the clean background baseline. */
    grCopyZoneToBg(sfc, x, y, width, height);
}
void grSaveZone(PS1Surface *sfc, uint16 x, uint16 y, uint16 width, uint16 height)
{
    (void)sfc;
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    grPs1SavedZone.valid = 1;
}

void grRestoreZone(PS1Surface *sfc, uint16 x, uint16 y, uint16 width, uint16 height)
{
    int screenX;
    int screenY;

    (void)sfc;

    if (grPs1SavedZone.valid) {
        grRestoreRectFromCleanBg(grPs1SavedZone.x,
                                 grPs1SavedZone.y,
                                 grPs1SavedZone.width,
                                 grPs1SavedZone.height);
        grPs1SavedZone.valid = 0;
        return;
    }

    if (width == 0 || height == 0)
        return;

    screenX = (int)x + grDx;
    screenY = (int)y + grDy;
    grRestoreRectFromCleanBg(screenX, screenY, width, height);
}

/*
 * Frame capture (not implemented on PS1)
 */
int grCaptureFrame(const char *filename)
{
    /* Frame capture not supported on PS1 hardware */
    return -1;
}

int grCaptureSequenceComplete(void)
{
    return 0;
}

void grCaptureSetSceneLabel(const char *sceneLabel)
{
    (void)sceneLabel;
}

void grCaptureSoundEvent(int sampleNo)
{
    (void)sampleNo;
}
