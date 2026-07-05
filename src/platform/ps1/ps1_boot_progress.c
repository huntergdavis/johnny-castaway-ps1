/*
 *  This file is part of 'Johnny Reborn' - PS1 Port
 *
 *  Boot loading indicator: the full-size Lilliputian galleon sails in
 *  from the left edge; its BOWSPRIT is the progress needle, reaching
 *  the right screen edge at 100%. A chunky blue bar runs beneath it.
 *
 *  Milestone map (which stage a frozen ship points at):
 *      6%  title image visible, CD catalog parse starting
 *     10%  RESOURCE.MAP/001 parsed
 *     14%  memory regions initialized
 *     18%  memory-card settings loaded
 *  20..70%  SPU sample uploads (2% per VAG — the long stretch)
 *     74%  ocean ambience uploaded
 *     80%  SPU cache / walk assets primed
 *     86%  staged-transition shape reserved
 *     92%  scene explorer data loaded
 *    100%  first scene staged (band is cleared)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#include <psxgpu.h>
#include <string.h>

#include "mytypes.h"
#include "ps1_boot_progress.h"
#include "ps1_ship_sprite.h"
#include "graphics_ps1.h"
#include "cdrom_ps1.h"

#define BAR_H        8
#define BAR_R        40
#define BAR_G        120
#define BAR_B        255
#define SHIP_Y       (480 - PS1_SHIP_SPRITE_H - BAR_H - 2)
#define BAR_Y        (SHIP_Y + PS1_SHIP_SPRITE_H + 1)
/* Bow starts just peeking in at the left edge and touches x=640 at 100%. */
#define BOW_MIN      18
#define BOW_MAX      640

static int    gBootBarActive = 0;
static int    gBootBarPrevX  = -32768;    /* ship-left of previous draw */
/* The ship is stream-decoded from RLE on every tick, one row at a time,
 * through these two small static buffers. Deliberately NO heap use: the
 * indicator lives across parseResourceFiles and memInit, and holding
 * heap blocks there fragments the arena right where memInit needs its
 * contiguous 1.44 MB region buffer (that failure shipped once). */
/* All ship working memory lives in the borrowed 10240-byte explorer
 * chunk buffer (idle during boot; see grSceneExplorerChunkBorrow):
 *   [0    .. 4902)  SHIP.DAT RLE payload
 *   [6144 .. 6408)  row decode scratch   (132 px * 2)
 *   [6656 .. 6920)  aligned upload slice (132 px * 2)
 *   [8192 .. 10240) sound-upload verify window (sound_ps1.c)
 * Keeping these out of BSS matters: the diagnostics-heavy regtest
 * builds sit ~2 KB under the static-image ceiling. NULL = bar-only. */
static const uint8 *gShipRle   = NULL;
static uint16     *gShipRow    = NULL;
static uint16     *gShipSlice  = NULL;

/* The indicator drives the GPU DIRECTLY through GP0 port writes — no
 * PSn00bSDK draw queue, no DMA2. Mixing immediate DrawPrim fills with
 * queued LoadImage transfers wedged the GPU FIFO here: DrawSync can
 * return before the final row DMA drains, and the next fill's command
 * words then land inside the pixel stream (frozen boot at a random
 * tick, fills silently eaten, stray diagonal pixel smears). Direct
 * port writes are serialized by construction. Callers drain the SDK
 * queue once (DrawSync) before the first direct write of each tick. */
#define BOOT_GP0     (*(volatile uint32_t *)0xBF801810)
#define BOOT_GPUSTAT (*(volatile uint32_t *)0xBF801814)

static void bootGpuPace(uint32 readyBit)
{
    int i;
    for (i = 0; i < 400000; i++) {
        if (BOOT_GPUSTAT & readyBit)
            return;
    }
    /* Ceiling hit: proceed anyway — a stalled write is caught by the
     * next pace; bailing mid-command would desync the FIFO for real. */
}

static void bootFill(int x, int y, int w, int h,
                     uint8 r, uint8 g, uint8 b)
{
    if (w <= 0 || h <= 0)
        return;
    bootGpuPace(1u << 26);              /* ready for command */
    BOOT_GP0 = 0x02000000u |
               ((uint32)b << 16) | ((uint32)g << 8) | (uint32)r;
    BOOT_GP0 = ((uint32)(uint16)y << 16) | (uint16)x;
    BOOT_GP0 = ((uint32)(uint16)h << 16) | (uint16)w;
}

/* One-row CPU->VRAM upload, w even. */
static void bootBlitRow(int x, int y, int w, const uint16 *src)
{
    int i, words = w / 2;

    bootGpuPace(1u << 26);
    BOOT_GP0 = 0xA0000000u;
    BOOT_GP0 = ((uint32)(uint16)y << 16) | (uint16)x;
    BOOT_GP0 = (1u << 16) | (uint16)w;
    for (i = 0; i < words; i++) {
        if ((i & 15) == 0)
            bootGpuPace(1u << 28);      /* ready for data block */
        BOOT_GP0 = ((uint32)src[i * 2 + 1] << 16) | src[i * 2];
    }
}

/* Stream-decode the RLE sprite and upload the horizontally-clipped
 * slice of each row as it completes. xs/wVis are the visible span;
 * srcOff is how many left columns are clipped off. */
static void bootDrawShip(int xs, int wVis, int srcOff)
{
    uint32 i;
    int col = 0, row = 0;

    for (i = 0; i + 1u < PS1_SHIP_SPRITE_RLE_LEN && row < PS1_SHIP_SPRITE_H;
         i += 2) {
        uint8 count = gShipRle[i];
        uint8 value = gShipRle[i + 1];
        uint8 c;
        for (c = 0; c < count; c++) {
            gShipRow[col++] = gPs1ShipPalette[(value >> 4) & 0xF];
            gShipRow[col++] = gPs1ShipPalette[value & 0xF];
            if (col >= PS1_SHIP_SPRITE_W) {
                memcpy(gShipSlice, gShipRow + srcOff, (uint32)wVis * 2u);
                bootBlitRow(xs, SHIP_Y + row, wVis, gShipSlice);
                col = 0;
                row++;
                if (row >= PS1_SHIP_SPRITE_H)
                    break;
            }
        }
    }
}

void ps1BootProgressBegin(void)
{
    uint32 bufBytes = 0;
    void *buf = grSceneExplorerChunkBorrow(&bufBytes);
    CdlFILE f;

    gShipRle = NULL;
    if (bufBytes >= 8192u &&
        (PS1_SHIP_SPRITE_RLE_LEN + 2047u & ~2047u) <= 6144u &&
        ps1_streamResolveFile("SHIP.DAT", &f) &&
        ps1_streamReadAlignedIntoFile(&f, 0,
                                      (PS1_SHIP_SPRITE_RLE_LEN + 2047u) & ~2047u,
                                      (uint8 *)buf)) {
        gShipRle   = (const uint8 *)buf;
        gShipRow   = (uint16 *)((uint8 *)buf + 6144u);
        gShipSlice = (uint16 *)((uint8 *)buf + 6656u);
    }
    /* Load failure -> bar-only indicator; never block boot on the ship. */

    gBootBarActive = 1;
    gBootBarPrevX  = -32768;
    ps1BootProgress(1);
}

void ps1BootProgress(uint8 pct)
{
    int bow, x, xs, wVis;

    if (!gBootBarActive)
        return;
    if (pct > 100)
        pct = 100;
    /* Drain anything the SDK queued before our direct GP0 writes.
     * DrawSync(0) alone is NOT enough: the dock call (pct=100) runs at
     * the first presented scene frame, when the scene's initial
     * LoadImages sit QUEUED-not-started — invisible to DrawSync(0)'s
     * early return, they auto-start from the DMA IRQ mid-fill (console:
     * garbled bottom line on scene start, healed by the menu repaint).
     * Same queue-first discipline as the explorer quiesce. */
    {
        int i;
        for (i = 0; i < 2000000 && DrawSync(1) > 0; i++)
            ;
    }
    DrawSync(0);
    bow = BOW_MIN + ((BOW_MAX - BOW_MIN) * (int)pct) / 100;
    x = bow - PS1_SHIP_SPRITE_W;          /* ship-left; negative = clipped */
    if (x < gBootBarPrevX)
        x = gBootBarPrevX;                /* the ship never sails backwards */

    /* Progress bar: left edge to the bowsprit. */
    bootFill(0, BAR_Y, bow > 640 ? 640 : bow, BAR_H, BAR_R, BAR_G, BAR_B);

    /* Erase the trail the ship vacated BEFORE redrawing. This must be
     * an exact pixel upload, not a GPU FILL: FILL operates on 16px-
     * aligned columns only, and the per-tick trail is ~12px — FILL
     * rounded it to zero width and never erased, leaving the previous
     * stern slice visible at the ship's left ("drawn twice"). Erasing
     * first is safe because the ship redraw stamps over any overlap. */
    xs = (x < 0) ? 0 : x;
    if (gBootBarPrevX > -32768 && x > gBootBarPrevX) {
        int es = ((gBootBarPrevX < 0) ? 0 : gBootBarPrevX) & ~1;
        int ew = (xs - es + 1) & ~1;
        if (ew > PS1_SHIP_SPRITE_W)
            ew = PS1_SHIP_SPRITE_W;
        if (ew > 0) {
            int r;
            memset(gShipSlice, 0, (uint32)ew * 2u);
            for (r = 0; r < PS1_SHIP_SPRITE_H; r++)
                bootBlitRow(es, SHIP_Y + r, ew, gShipSlice);
        }
    }

    /* Ship, width forced even (the GPU transfers pixel pairs). */
    wVis = (x + PS1_SHIP_SPRITE_W - xs) & ~1;
    if (wVis > 0 && gShipRle != NULL)
        bootDrawShip(xs, wVis, xs - x);
    bootGpuPace(1u << 26);              /* last command fully consumed */

    gBootBarPrevX = x;
}

void ps1BootProgressFinish(void)
{
    if (!gBootBarActive)
        return;
    /* Queue-first drain — see ps1BootProgress. */
    {
        int i;
        for (i = 0; i < 2000000 && DrawSync(1) > 0; i++)
            ;
    }
    DrawSync(0);
    /* Clear the whole band (ship + bar) back to black. */
    bootFill(0, SHIP_Y - 2, 640, PS1_SHIP_SPRITE_H + BAR_H + 6, 0, 0, 0);
    bootGpuPace(1u << 26);
    gBootBarActive = 0;
}
