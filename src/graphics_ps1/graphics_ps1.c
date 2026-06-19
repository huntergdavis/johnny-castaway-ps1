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
#include "ps1_spu_cache.h"

#ifndef GRAPHICS_PS1_DIAG_LOGS
#define GRAPHICS_PS1_DIAG_LOGS 0
#endif
#ifndef PS1_VERBOSE_DIAGNOSTICS
#define PS1_VERBOSE_DIAGNOSTICS 0
#endif

#if GRAPHICS_PS1_DIAG_LOGS
#define GR_DIAG_PRINTF(...) do { if (debugMode) printf(__VA_ARGS__); } while (0)
#else
#define GR_DIAG_PRINTF(...) do { } while (0)
#endif

static int grBuildPath3(char *out, size_t outSize,
                        const char *a, const char *b, const char *c)
{
    size_t pos = 0;
    const char *parts[3];
    int part;

    if (out == NULL || outSize == 0)
        return 0;

    parts[0] = a ? a : "";
    parts[1] = b ? b : "";
    parts[2] = c ? c : "";
    for (part = 0; part < 3; part++) {
        const char *s = parts[part];
        while (*s != '\0') {
            if (pos + 1 >= outSize) {
                out[0] = '\0';
                return 0;
            }
            out[pos++] = *s++;
        }
    }
    out[pos] = '\0';
    return 1;
}

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

/* One-shot "the next framebuffer upload MUST be all 4 tiles in full" flag.
 * grForceFullRedrawNextFrame sets it; grDrawBackground honors it with an
 * unconditional full-tile LoadImage and clears it. This makes the forced
 * full upload immune to any intervening dirty-state clear — notably the
 * residual-clean present paths (grBeginResidualCleanBgFrame) that wipe
 * prevDirty/currDirty between the force call and the actual upload, which
 * otherwise leaves a stale backdrop (e.g. the previous island surviving a
 * transition into an all-black scene like johnny6 "the end"). */
static int grForcedFullRedrawPending = 0;

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

#include "graphics_ps1/frame.c.inc"
#include "graphics_ps1/loaders.c.inc"
#include "graphics_ps1/composite.c.inc"
#include "graphics_ps1/draw_primitives.c.inc"
#include "graphics_ps1/background_tiles.c.inc"
#include "graphics_ps1/clean_rects.c.inc"
#include "graphics_ps1/background_screen.c.inc"
#include "graphics_ps1/screen_load.c.inc"
#include "graphics_ps1/capture_zones.c.inc"
