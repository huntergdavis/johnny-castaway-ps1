/*
 *  This file is part of 'Johnny Reborn' - PS1 Port
 *
 *  Pause menu overlay for PS1 build
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

#include <psxgpu.h>
#include <psxpad.h>
#include <psxapi.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "mytypes.h"
#include "pause_menu.h"
#include "graphics_ps1.h"
#include "sound_ps1.h"
#include "resource.h"
#include "foreground_pilot.h"
#include "ps1_perf.h"
#include "ps1_gpu_ot.h"
#include "memcard.h"
#include "scene_explorer_data.h"
#include "holidays.h"
#include "ps1_captions.h"
#include "ps1_pad_input.h"
#include "scene_freeplay.h"
#include "scene_picker.h"

#ifndef PAUSE_MENU_DIAG_LOGS
#define PAUSE_MENU_DIAG_LOGS 0
#endif

/* ---------------------------------------------------------------------------
 *  External telemetry / debug state.
 *  ads.c and story.c were retired in the legacy-runtime cleanup
 *  (commit 7d5221e3), so the old `ps1AdsCurrent*` / `ps1StoryDbg*`
 *  symbols no longer exist. P3 will swap Scene Info for a richer
 *  Debug Info screen that reads from ps1_perf.h.
 * ------------------------------------------------------------------------- */

/* Font stream from ps1_debug.c -- reused for pause menu text. */
extern int fontID;

/* Scene render env from graphics_ps1.c — needs to be re-applied on
 * pause exit so the screen doesn't keep clearing to our pause-blue
 * after the menu closes. */
extern DRAWENV draw[2];
extern int db;

/* Sound mute helpers from sound_ps1.c. */
extern int soundMuted;
void soundMuteToggle(void);

/* RNG seed state + helpers from jc_reborn.c. ps1SeedRandom uses the
 * root-counter hash (PS1) or no-op stub (host); ps1SetSeed installs an
 * exact srand() value. ps1LastSeedKnown becomes 1 once either has run. */
extern unsigned int ps1LastSeedApplied;
extern int          ps1LastSeedKnown;
void ps1SeedRandom(void);
void ps1SetSeed(unsigned int seed);

/* Frame counter from graphics_ps1.c (incremented per scene frame). */
extern uint32 ps1FrameCount;

/* Controller pad buffer from events_ps1.c. */

/* ---------------------------------------------------------------------------
 *  Menu state
 * ------------------------------------------------------------------------- */
static int              menuVisible  = 0;
static int              menuCursor   = 0;
static enum PauseMenuState menuState = PAUSE_MENU_MAIN;
static int              menuFramebufferPrimed = 0;

/* "Next scene" request flag consumed by ads/story loop. */
int pauseMenuRequestNextScene = 0;

/* "Reset loop" flag — foreground pilot loop checks this and exits
 * early so jc_reborn's outer loop can restart from scene 0. */
int pauseMenuRequestResetLoop = 0;

/* "Freeplay Mode" flag — foreground pilot loop checks this and exits
 * early so jc_reborn's outer loop can dispatch `fgpilot freeplay`. */
int pauseMenuRequestFreeplay = 0;
int pauseMenuRequestExitFreeplay = 0;
int pauseMenuRequestFreeplayGag = -1;
int pauseMenuRequestFreeplayVisitor = -1;
int pauseMenuRequestFreeplayClear = 0;
int pauseMenuRequestFreeplayWorldRefresh = 0;
static int menuFreeplayActive = 0;

/* Scene-set framework — pause menu shows "Scene Set: <name>" and
 * cycles the active pool when the user selects it. The actual pool
 * mapping lives in jc_reborn.c. pauseMenuSceneSet is the current
 * index and pauseMenuRequestSceneSetCycle fires on each cycle so the
 * main loop knows to re-randomize. */
int pauseMenuRequestSceneSetCycle = 0;
int pauseMenuSceneSet = 0;

/* Scene Explorer — pin a scene for one-shot play (Cross) or loop
 * (Triangle). -1 = idle, 0..62 = index into gSceneExplorer[]. The
 * jc_reborn screensaver loop consumes either flag at the top of each
 * iteration. */
int pauseMenuRequestPlayScene = -1;
int pauseMenuRequestLoopScene = -1;
static int pauseMenuExplorerCursor = 0;

/* Pending preview the user is scrolling through with left/right while on
 * the Scene Set menu line. Becomes pauseMenuSceneSet only on X/START
 * commit. Resets to pauseMenuSceneSet whenever the cursor leaves the
 * Scene Set row, so unsubmitted previews never linger. */
static int pendingSceneSet = 0;

/* Did pauseMenuShow itself toggle the mute? If yes, pauseMenuHide
 * undoes it. If the user manually toggled mute via the menu item
 * while paused, this gets cleared so we don't fight them on hide. */
static int pauseMutedSound = 0;

/* OT + primitive scratch. Sized for ~30 lines × ~32 chars ≈ 960
 * SPRTs × 20 bytes each = 19200 bytes. Round up to 24 KB. */
#define PAUSE_OT_LEN 8
static uint32 pauseOt[PAUSE_OT_LEN];
static uint8  pausePrimBuf[24576];

/* Cached heap-free probe value, refreshed once per pause-show. */
static unsigned long pmCachedHeapKB = 0;

/* ---------------------------------------------------------------------------
 *  Embedded 8x8 ASCII font — chars 0x20..0x7F (96 chars).
 *  Each char is 8 bytes; each byte is one row, MSB = leftmost pixel.
 *  1 = foreground, 0 = transparent. Covers space, digits, A-Z, a-z,
 *  and basic punctuation. Unsupported chars draw blank.
 *  Source data is 1-bit-per-pixel; we expand to 4-bit-indexed at upload.
 * ------------------------------------------------------------------------- */
#define PM_FONT_FIRST 0x20
#define PM_FONT_COUNT 96
#define PM_GLYPH_W 8
#define PM_GLYPH_H 8

/* On-screen render scale — 2 = pixel-doubled (16x16 chars).
 * GPU samples each texel multiple times when SPRT WH > UV WH,
 * so this is free pixel-doubling, no extra source data. */
#define PM_SCALE 2
#define PM_DRAW_W (PM_GLYPH_W * PM_SCALE)
#define PM_DRAW_H (PM_GLYPH_H * PM_SCALE)
#define PM_LINE_STEP (PM_DRAW_H + 4)

/* The font itself. Hand-rolled minimum for menu legibility. */
static const uint8 pmFontBits[PM_FONT_COUNT][8] = {
    /* 20 ' ' */ {0,0,0,0,0,0,0,0},
    /* 21 ! */ {0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x00},
    /* 22 " */ {0x66,0x66,0x66,0x00,0x00,0x00,0x00,0x00},
    /* 23 # */ {0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00},
    /* 24 $ */ {0,0,0,0,0,0,0,0},
    /* 25 % */ {0,0,0,0,0,0,0,0},
    /* 26 & */ {0,0,0,0,0,0,0,0},
    /* 27 ' */ {0x18,0x18,0x18,0x00,0x00,0x00,0x00,0x00},
    /* 28 ( */ {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00},
    /* 29 ) */ {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00},
    /* 2A * */ {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00},
    /* 2B + */ {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00},
    /* 2C , */ {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30},
    /* 2D - */ {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00},
    /* 2E . */ {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00},
    /* 2F / */ {0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00},
    /* 30 0 */ {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00},
    /* 31 1 */ {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00},
    /* 32 2 */ {0x3C,0x66,0x06,0x1C,0x30,0x60,0x7E,0x00},
    /* 33 3 */ {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00},
    /* 34 4 */ {0x0C,0x1C,0x3C,0x6C,0x7E,0x0C,0x0C,0x00},
    /* 35 5 */ {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00},
    /* 36 6 */ {0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00},
    /* 37 7 */ {0x7E,0x06,0x0C,0x18,0x18,0x18,0x18,0x00},
    /* 38 8 */ {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00},
    /* 39 9 */ {0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00},
    /* 3A : */ {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00},
    /* 3B ; */ {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x30},
    /* 3C < */ {0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x00},
    /* 3D = */ {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00},
    /* 3E > */ {0x60,0x30,0x18,0x0C,0x18,0x30,0x60,0x00},
    /* 3F ? */ {0x3C,0x66,0x06,0x0C,0x18,0x00,0x18,0x00},
    /* 40 @ */ {0,0,0,0,0,0,0,0},
    /* 41 A */ {0x18,0x3C,0x66,0x66,0x7E,0x66,0x66,0x00},
    /* 42 B */ {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00},
    /* 43 C */ {0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00},
    /* 44 D */ {0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00},
    /* 45 E */ {0x7E,0x60,0x60,0x78,0x60,0x60,0x7E,0x00},
    /* 46 F */ {0x7E,0x60,0x60,0x78,0x60,0x60,0x60,0x00},
    /* 47 G */ {0x3C,0x66,0x60,0x6E,0x66,0x66,0x3C,0x00},
    /* 48 H */ {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00},
    /* 49 I */ {0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00},
    /* 4A J */ {0x1E,0x06,0x06,0x06,0x66,0x66,0x3C,0x00},
    /* 4B K */ {0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00},
    /* 4C L */ {0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00},
    /* 4D M */ {0xC6,0xEE,0xFE,0xD6,0xC6,0xC6,0xC6,0x00},
    /* 4E N */ {0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00},
    /* 4F O */ {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00},
    /* 50 P */ {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00},
    /* 51 Q */ {0x3C,0x66,0x66,0x66,0x6A,0x6C,0x36,0x00},
    /* 52 R */ {0x7C,0x66,0x66,0x7C,0x6C,0x66,0x66,0x00},
    /* 53 S */ {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00},
    /* 54 T */ {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00},
    /* 55 U */ {0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00},
    /* 56 V */ {0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00},
    /* 57 W */ {0xC6,0xC6,0xC6,0xD6,0xFE,0xEE,0xC6,0x00},
    /* 58 X */ {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00},
    /* 59 Y */ {0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00},
    /* 5A Z */ {0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00},
    /* 5B [ */ {0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00},
    /* 5C \ */ {0xC0,0x60,0x30,0x18,0x0C,0x06,0x02,0x00},
    /* 5D ] */ {0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00},
    /* 5E ^ */ {0x18,0x3C,0x66,0x00,0x00,0x00,0x00,0x00},
    /* 5F _ */ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF},
    /* 60 ` */ {0x30,0x18,0x0C,0x00,0x00,0x00,0x00,0x00},
    /* 61 a */ {0x00,0x00,0x3C,0x06,0x3E,0x66,0x3E,0x00},
    /* 62 b */ {0x60,0x60,0x7C,0x66,0x66,0x66,0x7C,0x00},
    /* 63 c */ {0x00,0x00,0x3C,0x66,0x60,0x66,0x3C,0x00},
    /* 64 d */ {0x06,0x06,0x3E,0x66,0x66,0x66,0x3E,0x00},
    /* 65 e */ {0x00,0x00,0x3C,0x66,0x7E,0x60,0x3C,0x00},
    /* 66 f */ {0x1C,0x30,0x7C,0x30,0x30,0x30,0x30,0x00},
    /* 67 g */ {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x3C},
    /* 68 h */ {0x60,0x60,0x7C,0x66,0x66,0x66,0x66,0x00},
    /* 69 i */ {0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00},
    /* 6A j */ {0x06,0x00,0x06,0x06,0x06,0x66,0x66,0x3C},
    /* 6B k */ {0x60,0x60,0x66,0x6C,0x78,0x6C,0x66,0x00},
    /* 6C l */ {0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00},
    /* 6D m */ {0x00,0x00,0xCC,0xFE,0xD6,0xC6,0xC6,0x00},
    /* 6E n */ {0x00,0x00,0x7C,0x66,0x66,0x66,0x66,0x00},
    /* 6F o */ {0x00,0x00,0x3C,0x66,0x66,0x66,0x3C,0x00},
    /* 70 p */ {0x00,0x00,0x7C,0x66,0x66,0x7C,0x60,0x60},
    /* 71 q */ {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x06},
    /* 72 r */ {0x00,0x00,0x7C,0x66,0x60,0x60,0x60,0x00},
    /* 73 s */ {0x00,0x00,0x3E,0x60,0x3C,0x06,0x7C,0x00},
    /* 74 t */ {0x18,0x18,0x7E,0x18,0x18,0x18,0x0E,0x00},
    /* 75 u */ {0x00,0x00,0x66,0x66,0x66,0x66,0x3E,0x00},
    /* 76 v */ {0x00,0x00,0x66,0x66,0x66,0x3C,0x18,0x00},
    /* 77 w */ {0x00,0x00,0xC6,0xC6,0xD6,0xFE,0x6C,0x00},
    /* 78 x */ {0x00,0x00,0x66,0x3C,0x18,0x3C,0x66,0x00},
    /* 79 y */ {0x00,0x00,0x66,0x66,0x66,0x3E,0x06,0x3C},
    /* 7A z */ {0x00,0x00,0x7E,0x0C,0x18,0x30,0x7E,0x00},
    /* 7B { */ {0x0E,0x18,0x18,0x70,0x18,0x18,0x0E,0x00},
    /* 7C | */ {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00},
    /* 7D } */ {0x70,0x18,0x18,0x0E,0x18,0x18,0x70,0x00},
    /* 7E ~ */ {0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 7F del */ {0,0,0,0,0,0,0,0},
};

/* VRAM placement of our font + CLUT.
 *   font texture: (640, 256), 4-bit packed, 16 cols × 6 rows of 16x16
 *     pre-doubled glyphs = 256 pixels wide × 96 pixels tall
 *     = 128 VRAM pixels × 96 lines, ending at (768, 352)
 *   CLUT: (640, 360), 16 entries × 1 line, well past texture
 */
#define PM_FONT_VRAM_X 640
#define PM_FONT_VRAM_Y 256
#define PM_CLUT_VRAM_X 640
#define PM_CLUT_VRAM_Y 360

/* Has the font been uploaded yet this run? */
static int pmFontUploaded = 0;

/* Per-frame globals so the drawXxx functions can issue text without
 * threading the buffer pointer through every call. Set by
 * pauseMenuUpdate before invoking the screen's draw helper. */
static uint8  *pmFramePrimNext = NULL;
static uint32 *pmFrameOtSlot   = NULL;

/* Upload the font texture + CLUT to VRAM. Idempotent.
 *
 * We PRE-SCALE the 8x8 source glyphs to 16x16 destination texels at
 * upload time (each source pixel becomes a 2x2 block). This way the
 * runtime SPRTs are 16x16 with a 16x16 UV step — PS1 GPU samples 1:1
 * so we get real pixel-doubled text. (Trying to scale via WH > UV at
 * runtime made each SPRT sample 2x2 chars instead of one char,
 * producing garble.)
 *
 * Output texture layout in VRAM:
 *   16 chars per row × 16 px wide each = 256 pixels = 128 VRAM pixels wide
 *   6 char rows × 16 px tall = 96 lines tall
 * → occupies (640, 256) to (768, 352) in VRAM (4-bit packed).
 */
static void pmUploadFont(void)
{
    /* Halfwords per VRAM row: 16 chars × (16 dest texels / 4 nibbles per hw)
     * = 16 × 4 = 64. Rows: 6 char rows × 16 dest lines = 96 lines.
     * Total: 64 × 96 = 6144 halfwords = 12288 bytes. */
    static uint16 fontVram[64 * 96];

    for (int gy = 0; gy < (PM_FONT_COUNT / 16); gy++) {  /* 6 glyph rows */
        for (int sy = 0; sy < PM_GLYPH_H; sy++) {        /* 8 source rows in glyph */
            for (int gx = 0; gx < 16; gx++) {            /* 16 glyph columns */
                uint8 srcRow = pmFontBits[gy * 16 + gx][sy];
                /* Build the destination 16-pixel row by doubling each
                 * source bit. 16 dest texels (4-bit each) = 4 halfwords. */
                uint16 hw[4] = {0, 0, 0, 0};
                for (int b = 0; b < 8; b++) {
                    uint8 px = (srcRow >> (7 - b)) & 1;
                    int dx0 = b * 2;       /* dest texels 2*b and 2*b+1 */
                    int dx1 = b * 2 + 1;
                    /* Each halfword packs 4 nibbles (4-bit indexed). */
                    hw[dx0 / 4] |= ((uint16)px) << ((dx0 % 4) * 4);
                    hw[dx1 / 4] |= ((uint16)px) << ((dx1 % 4) * 4);
                }
                /* Write the same halfword pattern into BOTH dest rows
                 * (vertical 2x: source row sy → dest rows 2*sy, 2*sy+1). */
                for (int rep = 0; rep < 2; rep++) {
                    int dy = sy * 2 + rep;
                    int vramRow = gy * 16 + dy;  /* 0..95 */
                    int rowBase = vramRow * 64;   /* halfwords per VRAM row */
                    int colBase = gx * 4;         /* 4 hw per glyph */
                    fontVram[rowBase + colBase + 0] = hw[0];
                    fontVram[rowBase + colBase + 1] = hw[1];
                    fontVram[rowBase + colBase + 2] = hw[2];
                    fontVram[rowBase + colBase + 3] = hw[3];
                }
            }
        }
    }

    /* Build CLUT: 16 entries.
     *   entry 0: 0x0000 = transparent (mask bit clear; PS1 treats this
     *            as "not drawn" for textured semi-trans-aware sprites
     *            with mask-on-source convention)
     *   entry 1: 0xFFFF = white with mask bit set (visible)
     *   2..15:   unused (set to white too for any glitches that
     *            reference them) */
    static uint16 clutData[16];
    clutData[0] = 0x0000;
    for (int i = 1; i < 16; i++)
        clutData[i] = 0xFFFF;

    /* Upload texture: 64 halfwords wide × 96 rows. */
    RECT texRect;
    setRECT(&texRect, PM_FONT_VRAM_X, PM_FONT_VRAM_Y, 64, 96);
    LoadImage(&texRect, (uint32*)fontVram);
    DrawSync(0);

    /* Upload CLUT: 16 halfwords × 1 row. */
    RECT clutRect;
    setRECT(&clutRect, PM_CLUT_VRAM_X, PM_CLUT_VRAM_Y, 16, 1);
    LoadImage(&clutRect, (uint32*)clutData);
    DrawSync(0);

    pmFontUploaded = 1;
#if PAUSE_MENU_DIAG_LOGS
    printf("JCPAUSE pmUploadFont done — font @ (%d,%d) clut @ (%d,%d)\n",
           PM_FONT_VRAM_X, PM_FONT_VRAM_Y, PM_CLUT_VRAM_X, PM_CLUT_VRAM_Y);
#endif
}

/* Time/date editing fields. */
static int editField  = 0;   /* 0=month,1=day,2=year,3=hour,4=min */
static int editMonth  = 6;
static int editDay    = 30;
static int editYear   = 2026;
static int editHour   = 12;
static int editMinute = 0;

/* Island-position editing fields (Set Island Pos sub-screen).
 * Mirror of the host overrides; copied in on entry, applied on confirm. */
static int editIslandX     = 0;
static int editIslandY     = 0;
static int editIslandValid = 0;
static int editIslandField = 0;   /* 0=X, 1=Y, 2=mode (AUTO/MANUAL) */

/* RNG-seed editing fields (Set RNG Seed sub-screen). */
static unsigned int editSeedValue = 1;
static int          editSeedFixed = 0;     /* 0 = AUTO, 1 = FIXED */
static int          editSeedField = 0;     /* 0 = mode, 1 = value */

/* Debounce: tracks which buttons were held last frame so we only act on
 * fresh presses (not auto-repeat while held). */
static uint16 prevButtons = 0;

/* ---------------------------------------------------------------------------
 *  Main menu item descriptors. Deeper controls live behind small
 *  sub-screens so no pause page grows past the fixed panel.
 * ------------------------------------------------------------------------- */
enum {
    MENU_RESUME,
    MENU_SCENE_SET,
    MENU_SCENE_EXPLORER,
    MENU_FREEPLAY,
    MENU_FREEPLAY_OPTIONS,
    MENU_WORLD,
    MENU_ACCESSIBILITY,
    MENU_SYSTEM,
    MENU_COUNT
};

/* Scene-set names. Order mirrors the gSceneSetPools array in
 * jc_reborn.c. Empty pools still appear in the cycle — jc_reborn
 * falls back to the All set when the picker is asked for a 0-size
 * pool. */
static const char *kSceneSetNames[] = {
    "All Scenes",
    "Fishing Only",
    "Johnny Stories",
    "Mary Visits",
    "Visitors",
    "Activities",
    "Misc & Suzy",
};
#define NUM_SCENE_SETS \
    ((int)(sizeof(kSceneSetNames) / sizeof(kSceneSetNames[0])))

const char *pauseMenuSceneSetName(int idx)
{
    if (idx < 0 || idx >= NUM_SCENE_SETS)
        return "?";
    return kSceneSetNames[idx];
}

enum {
    WORLD_DAYNIGHT,
    WORLD_TIDE,
    WORLD_RAFT,
    WORLD_HOLIDAYS,
    WORLD_ISLAND_POS,
    WORLD_BACK,
    WORLD_COUNT
};

enum {
    HOLIDAY_SET,
    HOLIDAY_SELECT,
    HOLIDAY_BACK,
    HOLIDAY_COUNT
};

enum {
    ACCESS_CAPTIONS,
    ACCESS_SOUND,
    ACCESS_OCEAN,
    ACCESS_SOUND_TEST,
    ACCESS_BACK,
    ACCESS_COUNT
};

enum {
    FPO_GAGS,
    FPO_VISITORS,
    FPO_CONTROLS,
    FPO_CLEAR,
    FPO_BACK,
    FPO_COUNT
};

enum {
    SYSTEM_SAVE,
    SYSTEM_SET_TIME,
    SYSTEM_SET_SEED,
    SYSTEM_PERF,
    SYSTEM_STORY_DAY,        /* force storyCurrentDay 1..11 — exercises Original mode dayNo gates */
    SYSTEM_RESET_LOOP,
    SYSTEM_NEXT_SCENE,
    SYSTEM_COUNT
};

/* storyCurrentDay (jc_reborn.c) is the 11-day Sierra calendar gate that
 * Original-mode picks consult via the dayNo column in storyScenes[].
 * The System sub-screen lets the user pin a day for testing without
 * waiting 11 in-game-date rollovers. */
extern int storyCurrentDay;

/* Scene Set Options sub-screen rows. */
enum {
    SSO_SCENE_SET,    /* cycle pool: All / Fishing / ... */
    SSO_PICKER,       /* cycle policy: Random / Sequential / Original */
    SSO_BACK,
    SSO_COUNT
};

static int optionsCursor = 0;
static int freeplayOptionsCursor = 0;
static int freeplayGagCursor = 0;
static int freeplayVisitorCursor = 0;
static int soundTestCursor = 0;
static int holidayCursor = 0;
static int accessCursor = 0;
static int systemCursor = 0;
static int sceneSetOptionsCursor = 0;

/* Forward decls. */
static const char *perfLevelLabel(void);

/* Clamp helpers */
static int clampInt(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static int daysInMonth(int m, int y)
{
    static const int dim[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (m < 1 || m > 12) return 30;
    int d = dim[m];
    if (m == 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)) d = 29;
    return d;
}

/* ---------------------------------------------------------------------------
 *  Pad reading helpers
 * ------------------------------------------------------------------------- */

/* Returns buttons that are newly pressed this frame (not held from last). */
static uint16 pmNewPress(uint16 cur)
{
    uint16 fresh = cur & ~prevButtons;
    return fresh;
}

/* ---------------------------------------------------------------------------
 *  Public API
 * ------------------------------------------------------------------------- */
void pauseMenuEnsureFontUploaded(void)
{
    if (!pmFontUploaded)
        pmUploadFont();
}

void pauseMenuInit(void)
{
    /* Reload BIOS font into VRAM -- graphicsInit may have overwritten it. */
    FntLoad(960, 0);

    /* Match OLD (working) commit b4476f3d: only open a stream if none
     * exists. ps1DebugInit already opened one as fontID=0; reuse that. */
    if (fontID < 0) {
        fontID = FntOpen(0, 0, 640, 480, 0, 1024);
    }
#if PAUSE_MENU_DIAG_LOGS
    printf("JCPAUSE pauseMenuInit fontID=%d\n", fontID);
#endif

    menuVisible = 0;
    menuCursor  = 0;
    menuState   = PAUSE_MENU_MAIN;
    menuFramebufferPrimed = 0;
}

void pauseMenuSetFreeplayActive(int active)
{
    menuFreeplayActive = active ? 1 : 0;
}

void pauseMenuShow(void)
{
    /* Reload font VRAM each time we enter the menu -- the game's
     * LoadImage uploads may have clobbered the (960,0) font area. */
    FntLoad(960, 0);

    menuVisible           = 1;
    menuCursor            = 0;
    optionsCursor         = 0;
    holidayCursor         = 0;
    accessCursor          = 0;
    systemCursor          = 0;
    pendingSceneSet       = pauseMenuSceneSet;  /* fresh preview each open */
    menuState             = PAUSE_MENU_MAIN;
    menuFramebufferPrimed = 0;
    prevButtons           = 0xFFFF;  /* Treat all buttons as "held" so the
                                        initial press that opened the menu
                                        is not re-acted. */

    /* Sound stays in whatever state the user left it — defaults ON.
     * The Sound menu item lets the user mute/unmute manually. (When
     * memory-card support lands, we'll persist the user's choice.) */
    pauseMutedSound = 0;

#if PAUSE_MENU_DIAG_LOGS
    /* One-shot JCPAUSE snapshot for log-mining. */
    printf("JCPAUSE show frame=%lu scene=%s mode=%s perfLevel=%u soundMuted=%d\n",
           (unsigned long)ps1FrameCount,
           foregroundPilotRuntimeSceneName(),
           foregroundPilotRuntimeModeName(),
           (unsigned)ps1PerfLevel,
           soundMuted);
#endif
    if (ps1PadScriptVerboseLogEnabled()) {
        printf("JCPAUSE show state=%d cursor=%d\n",
               (int)menuState, menuCursor);
    }

    /* Refresh the heap-free cache once per pause-show. fgProbeLargestAlloc
     * does ~9 malloc/free cycles which fragments the heap if called every
     * pause-loop iteration; caching avoids that. */
    pmCachedHeapKB = fgProbeLargestAlloc() / 1024UL;
}

void pauseMenuHide(void)
{
    if (ps1PadScriptVerboseLogEnabled()) {
        printf("JCPAUSE hide state=%d cursor=%d\n",
               (int)menuState, menuCursor);
    }
    menuVisible = 0;
    menuFramebufferPrimed = 0;
    /* Sound stays in user's chosen state — no auto-restore. */
}

int pauseMenuIsVisible(void)
{
    return menuVisible;
}

enum PauseMenuState pauseMenuGetState(void)
{
    return menuState;
}

void pauseMenuSetState(enum PauseMenuState state)
{
    menuState = state;
    if (state == PAUSE_MENU_MAIN)
        menuCursor = 0;
    prevButtons = 0xFFFF;  /* suppress stale press */
}

/* ---------------------------------------------------------------------------
 *  Drawing helpers -- all use FntPrint into the existing fontID stream
 * ------------------------------------------------------------------------- */

/* MANUAL SPRT TEXT RENDERER.
 *
 * Bypasses PSn00bSDK's broken FntFlush by building SPRT primitives
 * ourselves, pointing at the BIOS font texture that FntLoad(960, 0)
 * uploaded to VRAM. The BIOS font is 16 chars wide × 16 chars tall,
 * 8x8 each (256 chars total), stored 4-bit packed.
 *
 * ASCII char N → texture coords (u, v):
 *   col = N % 16, row = N / 16
 *   u = col * 8, v = row * 8
 *
 * TPage covers a 256-texel-wide × 256-line region. The BIOS font at
 * (960, 0) sits at tpageX = 960/64 = 15, tpageY = 0/256 = 0.
 * BIOS font CLUT is conventionally at (x, y+128) → (960, 128).
 */
static int pmTextX = 60;
static int pmTextY = 100;
static int pmPrintfX = 80;

static void pmTextStart(int x, int y)
{
    pmTextX = x;
    pmTextY = y;
}

static void pmTextDrawChar(uint8 **nextp, uint32 *otSlot, char c)
{
    if (c == '\n') {
        pmTextX = pmPrintfX;
        pmTextY += PM_LINE_STEP;
        return;
    }
    unsigned char uc = (unsigned char)c;
    if (uc < PM_FONT_FIRST || uc >= PM_FONT_FIRST + PM_FONT_COUNT) {
        pmTextX += PM_GLYPH_W;
        return;
    }
    int idx = uc - PM_FONT_FIRST;
    int col = idx % 16;
    int row = idx / 16;

    SPRT *sprt = (SPRT*)(*nextp);
    *nextp += sizeof(SPRT);
    setSprt(sprt);
    setXY0(sprt, pmTextX, pmTextY);
    /* SPRT samples 1:1; on-screen size == texel size. Both 16x16 since
     * we pre-doubled the font at upload time. */
    setWH(sprt, PM_DRAW_W, PM_DRAW_H);
    setUV0(sprt, col * PM_DRAW_W, row * PM_DRAW_H);
    setClut(sprt, PM_CLUT_VRAM_X, PM_CLUT_VRAM_Y);
    setRGB0(sprt, 128, 128, 128);
    ps1GpuOtAddPrim(otSlot, sprt);
    pmTextX += PM_DRAW_W;
}

static void pmTextDrawStr(uint8 **nextp, uint32 *otSlot, const char *s)
{
    while (*s) {
        pmTextDrawChar(nextp, otSlot, *s);
        s++;
    }
}

/* printf-style helper that uses the per-frame globals. Newline at end
 * advances pmTextY by PM_LINE_STEP and resets pmTextX to the column
 * the line started in (set by the caller before this call). */
static void pmPrintf(const char *fmt, ...)
{
    char buf[80];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    /* Strip trailing newlines — pmPrintf advances Y itself. */
    int len = (int)strlen(buf);
    while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r'))
        buf[--len] = '\0';
    /* Reset X to line start. */
    pmTextX = pmPrintfX;
    pmTextDrawStr(&pmFramePrimNext, pmFrameOtSlot, buf);
    pmTextY += PM_LINE_STEP;
}

/* Panel geometry — rounded-rect look via 3 overlapping rectangles
 * with 8x8 corner cutouts that show the dim layer through. */
#define PM_PANEL_X0 60
#define PM_PANEL_Y0 60
#define PM_PANEL_X1 580
#define PM_PANEL_Y1 420
#define PM_CORNER 12

/* Build dim + 3 panel quads. Caller threads the buffer pointer +
 * separate OT slots so the chain order matches GPU draw order:
 *   slot N-2: dim   (drawn after TPAGE at N-1)
 *   slot N-3: panel quads
 *   slot N-4: text
 */
static void pmBuildPanelQuads(uint8 **nextp, uint32 *otDim, uint32 *otPanel)
{
    if (otDim != NULL) {
        /* Dim — full screen, semi-trans 50% black. Halves what's behind. */
        POLY_F4 *dim = (POLY_F4*)*nextp;
        *nextp += sizeof(POLY_F4);
        setPolyF4(dim);
        setSemiTrans(dim, 1);
        setRGB0(dim, 0, 0, 0);
        setXY4(dim,   0,   0,
                    640,   0,
                      0, 480,
                    640, 480);
        ps1GpuOtAddPrim(otDim, dim);
    }

    /* Panel: 3 rectangles. Middle full width, top/bottom narrower so
     * the corners stay as dim background — fakes rounded corners. */
    int x0 = PM_PANEL_X0, y0 = PM_PANEL_Y0;
    int x1 = PM_PANEL_X1, y1 = PM_PANEL_Y1;
    int c  = PM_CORNER;
    uint8 r = 0x40, g = 0x10, b = 0x60;  /* dark purple */

    /* Middle (full width, no corners). */
    POLY_F4 *p1 = (POLY_F4*)*nextp;
    *nextp += sizeof(POLY_F4);
    setPolyF4(p1);
    setRGB0(p1, r, g, b);
    setXY4(p1, x0, y0 + c, x1, y0 + c, x0, y1 - c, x1, y1 - c);
    ps1GpuOtAddPrim(otPanel, p1);

    /* Top edge (narrower). */
    POLY_F4 *p2 = (POLY_F4*)*nextp;
    *nextp += sizeof(POLY_F4);
    setPolyF4(p2);
    setRGB0(p2, r, g, b);
    setXY4(p2, x0 + c, y0, x1 - c, y0, x0 + c, y0 + c, x1 - c, y0 + c);
    ps1GpuOtAddPrim(otPanel, p2);

    /* Bottom edge (narrower). */
    POLY_F4 *p3 = (POLY_F4*)*nextp;
    *nextp += sizeof(POLY_F4);
    setPolyF4(p3);
    setRGB0(p3, r, g, b);
    setXY4(p3, x0 + c, y1 - c, x1 - c, y1 - c, x0 + c, y1, x1 - c, y1);
    ps1GpuOtAddPrim(otPanel, p3);
}

static void drawSeparator(void)
{
    pmPrintf("----------------------------\n");
}

/* ---------------------------------------------------------------------------
 *  Sub-screen: Debug Info
 * ------------------------------------------------------------------------- */
static void drawSceneInfo(void)
{
    const char *scene = foregroundPilotRuntimeSceneName();
    const char *mode  = foregroundPilotRuntimeModeName();
    uint16 fIdx = foregroundPilotRuntimeFrameIndex();
    uint16 fCnt = foregroundPilotRuntimeFrameCount();
    uint32 uptimeSec = ps1FrameCount / 60;

    pmPrintf("    DEBUG INFO");
    drawSeparator();
    pmPrintf(" Scene:  %s",   scene && *scene ? scene : "(none)");
    pmPrintf(" Mode:   %s",   mode  ? mode  : "?");
    pmPrintf(" Frame:  %u/%u", (unsigned)fIdx, (unsigned)fCnt);
    pmPrintf(" Heap:   %lu KB free", pmCachedHeapKB);
    pmPrintf(" Uptime: %lu:%02lu",
             (unsigned long)(uptimeSec / 60), (unsigned long)(uptimeSec % 60));
    pmPrintf(" Build:  %s", __DATE__);
    pmPrintf(" Perf:   %s", perfLevelLabel());
    pmPrintf(" Sound:  %s", soundMuted ? "MUTED" : "ON");
    pmPrintf(" Save:   %s", memcardLastStatus ? memcardLastStatus : "(none)");
    drawSeparator();
    pmPrintf(" O/START = BACK");
}

/* ---------------------------------------------------------------------------
 *  Sub-screen: Controls
 * ------------------------------------------------------------------------- */
static void drawControls(void)
{
    pmPrintf("       CONTROLS\n");
    drawSeparator();
    pmPrintf(" D-PAD/L-STICK  Walk\n");
    pmPrintf(" L2 / R2        Slow / Fast\n");
    pmPrintf(" CIRCLE         Fish\n");
    pmPrintf(" R1+UP          Day/Night\n");
    pmPrintf(" R1+DOWN        Tide\n");
    pmPrintf(" R1+LEFT        Raft\n");
    pmPrintf(" R1+RIGHT       Holiday\n");
    pmPrintf(" START          Pause\n");
    drawSeparator();
    pmPrintf(" O/START = back\n");
}

static void drawFreeplayOptions(void)
{
    pmPrintf("    FREEPLAY OPTIONS\n");
    drawSeparator();
    pmPrintf(" %s Gags...\n",
             freeplayOptionsCursor == FPO_GAGS ? ">" : " ");
    pmPrintf(" %s Visitors...\n",
             freeplayOptionsCursor == FPO_VISITORS ? ">" : " ");
    pmPrintf(" %s Controls\n",
             freeplayOptionsCursor == FPO_CONTROLS ? ">" : " ");
    pmPrintf(" %s Clear Screen\n",
             freeplayOptionsCursor == FPO_CLEAR ? ">" : " ");
    pmPrintf(" %s Back\n",
             freeplayOptionsCursor == FPO_BACK ? ">" : " ");
    drawSeparator();
    if (!menuFreeplayActive)
        pmPrintf(" Start freeplay to spawn.\n");
    pmPrintf(" X = select   O/START = back\n");
}

/* ---------------------------------------------------------------------------
 *  Scene Set Options sub-screen — pool selector + picker policy in one
 *  page. Up/Down navigates rows, Left/Right cycles values inline,
 *  Cross commits any pending Scene Set change and backs out, Circle
 *  backs out without committing.
 * ------------------------------------------------------------------------- */
static void drawSceneSetOptions(void)
{
    pmPrintf("   SCENE SET OPTIONS\n");
    drawSeparator();

    /* Scene Set row: shows pending value with arrows when focused. */
    {
        int showIdx = (sceneSetOptionsCursor == SSO_SCENE_SET)
                          ? pendingSceneSet : pauseMenuSceneSet;
        int dirty   = (pendingSceneSet != pauseMenuSceneSet);
        if (showIdx < 0 || showIdx >= NUM_SCENE_SETS) showIdx = 0;
        pmPrintf(" %s Set:    %s%s%s%s\n",
                 sceneSetOptionsCursor == SSO_SCENE_SET ? ">" : " ",
                 sceneSetOptionsCursor == SSO_SCENE_SET ? "<" : " ",
                 kSceneSetNames[showIdx],
                 sceneSetOptionsCursor == SSO_SCENE_SET ? ">" : " ",
                 dirty ? "*" : "");
    }

    /* Picker policy row: cycles immediately on Left/Right (no commit
     * step — the policy switch is cheap and reversible). */
    {
        int policy = pickerGetPolicy();
        pmPrintf(" %s Picker: %s%s%s\n",
                 sceneSetOptionsCursor == SSO_PICKER ? ">" : " ",
                 sceneSetOptionsCursor == SSO_PICKER ? "<" : " ",
                 pickerPolicyName(policy),
                 sceneSetOptionsCursor == SSO_PICKER ? ">" : " ");
    }

    pmPrintf(" %s Back\n",
             sceneSetOptionsCursor == SSO_BACK ? ">" : " ");

    drawSeparator();
    pmPrintf(" L/R cycle   X commit\n");
    pmPrintf(" O/START = back\n");
}

static void drawFreeplayGagCatalog(void)
{
    int count = freeplayGagCount();
    if (count <= 0)
        count = 1;
    if (freeplayGagCursor >= count)
        freeplayGagCursor = 0;
    if (freeplayGagCursor < 0)
        freeplayGagCursor = count - 1;

    pmPrintf("      FREEPLAY GAG\n");
    drawSeparator();
    pmPrintf(" %02d/%02d  %.22s\n",
             freeplayGagCursor + 1, count,
             freeplayGagTitle(freeplayGagCursor));
    pmPrintf(" BMP: %.18s\n", freeplayGagBmp(freeplayGagCursor));
    pmPrintf(" Frames: %-3d  RAM: ~%d KB\n",
             freeplayGagFrames(freeplayGagCursor),
             freeplayGagMemoryKB(freeplayGagCursor));
    pmPrintf(" %.28s\n", freeplayGagDescription(freeplayGagCursor));
    drawSeparator();
    pmPrintf(" UP/DOWN choose\n");
    pmPrintf(menuFreeplayActive ? " X spawn now\n" : " Start freeplay first\n");
    pmPrintf(" O/START = back\n");
}

static void drawFreeplayVisitorCatalog(void)
{
    int count = freeplayVisitorCount();
    if (count <= 0)
        count = 1;
    if (freeplayVisitorCursor >= count)
        freeplayVisitorCursor = 0;
    if (freeplayVisitorCursor < 0)
        freeplayVisitorCursor = count - 1;

    pmPrintf("    FREEPLAY VISITOR\n");
    drawSeparator();
    pmPrintf(" %02d/%02d  %.22s\n",
             freeplayVisitorCursor + 1, count,
             freeplayVisitorTitle(freeplayVisitorCursor));
    pmPrintf(" BMP: %.18s\n", freeplayVisitorBmp(freeplayVisitorCursor));
    pmPrintf(" Frames: %-3d  RAM: ~%d KB\n",
             freeplayVisitorFrames(freeplayVisitorCursor),
             freeplayVisitorMemoryKB(freeplayVisitorCursor));
    pmPrintf(" %.28s\n", freeplayVisitorDescription(freeplayVisitorCursor));
    drawSeparator();
    pmPrintf(" UP/DOWN choose\n");
    pmPrintf(menuFreeplayActive ? " X spawn now\n" : " Start freeplay first\n");
    pmPrintf(" O/START = back\n");
}

/* ---------------------------------------------------------------------------
 *  Sub-screen: Set Time/Date
 * ------------------------------------------------------------------------- */
static void drawSetTime(void)
{
    const char *fieldNames[] = {"Month","Day","Year","Hour","Min"};

    pmPrintf("\n");
    drawSeparator();
    pmPrintf("   SET TIME AND DATE\n");
    drawSeparator();
    pmPrintf("\n");

    /* Month */
    pmPrintf(" %s Month: %s%02d%s\n",
             editField == 0 ? ">" : " ",
             editField == 0 ? "[" : " ",
             editMonth,
             editField == 0 ? "]" : " ");

    /* Day */
    pmPrintf(" %s Day:   %s%02d%s\n",
             editField == 1 ? ">" : " ",
             editField == 1 ? "[" : " ",
             editDay,
             editField == 1 ? "]" : " ");

    /* Year */
    pmPrintf(" %s Year:  %s%04d%s\n",
             editField == 2 ? ">" : " ",
             editField == 2 ? "[" : " ",
             editYear,
             editField == 2 ? "]" : " ");

    /* Hour */
    pmPrintf(" %s Hour:  %s%02d%s\n",
             editField == 3 ? ">" : " ",
             editField == 3 ? "[" : " ",
             editHour,
             editField == 3 ? "]" : " ");

    /* Minute */
    pmPrintf(" %s Min:   %s%02d%s\n",
             editField == 4 ? ">" : " ",
             editField == 4 ? "[" : " ",
             editMinute,
             editField == 4 ? "]" : " ");

    {
        int dateHoliday = holidayForDate(editYear, editMonth, editDay);
        pmPrintf("   Holiday: %s\n",
                 dateHoliday ? holidayShortName(dateHoliday) : "NONE");
        if (dateHoliday)
            pmPrintf("   %.28s\n", holidayTitle(dateHoliday));
    }

    pmPrintf("\n");
    pmPrintf(" UP/DOWN select field\n");
    pmPrintf(" LEFT/RIGHT adjust value\n");
    pmPrintf(" X to confirm, O/START back\n");

    (void)fieldNames;
}

/* ---------------------------------------------------------------------------
 *  Sub-screen: Set Island Pos
 * ---------------------------------------------------------------------------
 *  Three fields: X offset, Y offset, and a Mode toggle (AUTO vs MANUAL).
 *  AUTO = let the runtime pick (random varpos for fishing scenes,
 *         (0, 0) otherwise).
 *  MANUAL = use the X/Y values from this screen on every scene.
 *  Range guards: X clamped to -250..+250, Y to -100..+200. Real
 *  observed varpos from fgLoopRandomVarPos lives well inside that.
 *  X confirms back to main; Circle/START goes back without applying.
 * ------------------------------------------------------------------------- */
static void drawIslandPos(void)
{
    pmPrintf("\n");
    drawSeparator();
    pmPrintf("    SET ISLAND POSITION\n");
    drawSeparator();
    pmPrintf("\n");

    pmPrintf(" %s X offset:  %s%+4d%s\n",
             editIslandField == 0 ? ">" : " ",
             editIslandField == 0 ? "[" : " ",
             editIslandX,
             editIslandField == 0 ? "]" : " ");
    pmPrintf(" %s Y offset:  %s%+4d%s\n",
             editIslandField == 1 ? ">" : " ",
             editIslandField == 1 ? "[" : " ",
             editIslandY,
             editIslandField == 1 ? "]" : " ");
    pmPrintf(" %s Mode:      %s%s%s\n",
             editIslandField == 2 ? ">" : " ",
             editIslandField == 2 ? "[" : " ",
             editIslandValid ? "MANUAL" : "AUTO",
             editIslandField == 2 ? "]" : " ");

    pmPrintf("\n");
    pmPrintf(" UP/DOWN  select field\n");
    pmPrintf(" LEFT/RIGHT adjust X/Y by 4\n");
    pmPrintf("           toggle mode\n");
    pmPrintf(" X = confirm   O/START = back\n");

    if (!editIslandValid && (editIslandX != 0 || editIslandY != 0)) {
        pmPrintf("\n");
        pmPrintf(" (X/Y kept; AUTO ignores them)\n");
    }
}

/* ---------------------------------------------------------------------------
 *  Sub-screen: Set RNG Seed
 * ---------------------------------------------------------------------------
 *  Two fields: Mode (AUTO / FIXED) and Value (unsigned 32-bit).
 *  AUTO  = on confirm, re-seed via ps1SeedRandom() (root counters).
 *  FIXED = on confirm, srand(value).
 *  LEFT/RIGHT on the value field nudges by 1; L1/R1 by 100; L2/R2 by 10000.
 *  X = confirm, Circle/START = back without applying.
 * ------------------------------------------------------------------------- */
static void drawSetSeed(void)
{
    pmPrintf("\n");
    drawSeparator();
    pmPrintf("    SET RNG SEED\n");
    drawSeparator();
    pmPrintf("\n");

    pmPrintf(" %s Mode:   %s%s%s\n",
             editSeedField == 0 ? ">" : " ",
             editSeedField == 0 ? "[" : " ",
             editSeedFixed ? "FIXED" : "AUTO",
             editSeedField == 0 ? "]" : " ");

    pmPrintf(" %s Value:  %s%10u%s\n",
             editSeedField == 1 ? ">" : " ",
             editSeedField == 1 ? "[" : " ",
             editSeedValue,
             editSeedField == 1 ? "]" : " ");

    pmPrintf("\n");
    if (ps1LastSeedKnown) {
        pmPrintf(" Current seed: %u\n", ps1LastSeedApplied);
    } else {
        pmPrintf(" Current seed: (unknown)\n");
    }

    pmPrintf("\n");
    pmPrintf(" UP/DOWN  select field\n");
    pmPrintf(" LEFT/RIGHT  +/-1\n");
    pmPrintf(" L1/R1       +/-100\n");
    pmPrintf(" L2/R2       +/-10000\n");
    pmPrintf(" X = confirm   O/START = back\n");
}

/* ---------------------------------------------------------------------------
 *  Sub-screen: Credits
 * ---------------------------------------------------------------------------
 *  A labor-of-love attribution + open-source notice + the "if you paid
 *  you were cheated" disclaimer + the upstream URL. Circle/START = back.
 *  Lines are tuned to ~30 chars max so they fit the panel width.
 * ------------------------------------------------------------------------- */
static void drawCredits(void)
{
    pmPrintf(" CREDITS  (O/START to return)\n");
    drawSeparator();
    pmPrintf(" A labor of love by\n");
    pmPrintf(" Hunter Davis.\n");
    pmPrintf("\n");
    pmPrintf(" Hunter does not own or have\n");
    pmPrintf(" a license to the Johnny\n");
    pmPrintf(" Castaway character. The\n");
    pmPrintf(" original creator generously\n");
    pmPrintf(" allows fan ports.\n");
    pmPrintf("\n");
    pmPrintf(" If you paid for this, you\n");
    pmPrintf(" were cheated.\n");
    pmPrintf(" Open source and free:\n");
    pmPrintf(" github.com/huntergdavis/\n");
    pmPrintf(" Johnny-Castaway-PS1\n");
}

/* ---------------------------------------------------------------------------
 *  Main menu drawing
 * ------------------------------------------------------------------------- */
static const char *perfLevelLabel(void)
{
    switch (ps1PerfLevel) {
    case 0: return "OFF";
    case 1: return "SUMMARY";
    case 2: return "DETAIL";
    case 3: return "DEBUG";
    default: return "?";
    }
}

/* Externs from jc_reborn.c: -1 = auto/random, else forced. */
extern int hostForcedNight;
extern int hostHolidayMode;
extern int hostForcedHoliday;
extern int hostForcedLowTide;
extern int hostForcedRaftStage;
extern int hostForcedIslandPosValid;
extern int hostForcedIslandX;
extern int hostForcedIslandY;
extern int ps1SoftHour;
extern int ps1SoftMinute;
extern int ps1SoftMonth;
extern int ps1SoftDay;
extern int ps1SoftYear;
extern int ps1SoftTimeEnabled;

static const char *daynightLabel(void)
{
    switch (hostForcedNight) {
    case -1: return "AUTO";
    case 0:  return "DAY";
    case 1:  return "NIGHT";
    default: return "?";
    }
}

static const char *holidayLabel(void)
{
    if (hostHolidayMode == HOLIDAY_MODE_AUTO_ORIGINAL4 ||
        hostHolidayMode == HOLIDAY_MODE_AUTO_ALL)
        return "AUTO";
    if (hostHolidayMode == HOLIDAY_MODE_NONE) return "NONE";
    if (hostForcedHoliday == 0) return "NONE";
    return holidayShortName(hostForcedHoliday);
}

static int holidaySetMode(void)
{
    if (hostHolidayMode < 0 || hostHolidayMode >= HOLIDAY_MODE_COUNT)
        return HOLIDAY_MODE_AUTO_ORIGINAL4;
    return hostHolidayMode;
}

static const char *holidaySetLabel(void)
{
    switch (holidaySetMode()) {
    case HOLIDAY_MODE_AUTO_ORIGINAL4: return "AUTO DATE:ORIG4";
    case HOLIDAY_MODE_AUTO_ALL:       return "AUTO DATE";
    case HOLIDAY_MODE_NONE:           return "NONE";
    case HOLIDAY_MODE_MANUAL_ORIG4:   return "ORIGINAL 4";
    case HOLIDAY_MODE_MANUAL_EXPANDED:return "EXPANDED";
    default: return "?";
    }
}

static int firstExpandedHoliday(void)
{
    return holidayFirstExpandedId();
}

static int nextHolidayInRange(int current, int minId, int maxId, int dir)
{
    int best = 0;
    int found = 0;
    if (dir >= 0) {
        for (int i = 0; i < gHolidayCount; i++) {
            int id = gHolidays[i].id;
            if (id < minId || id > maxId)
                continue;
            if (!best)
                best = id;
            if (found)
                return id;
            if (id == current)
                found = 1;
        }
        return best;
    }

    for (int i = gHolidayCount - 1; i >= 0; i--) {
        int id = gHolidays[i].id;
        if (id < minId || id > maxId)
            continue;
        if (!best)
            best = id;
        if (found)
            return id;
        if (id == current)
            found = 1;
    }
    return best;
}

static void cycleHolidaySet(int dir)
{
    int mode = holidaySetMode();
    mode += (dir >= 0) ? 1 : -1;
    if (mode >= HOLIDAY_MODE_COUNT) mode = 0;
    if (mode < 0) mode = HOLIDAY_MODE_COUNT - 1;

    hostHolidayMode = mode;
    switch (mode) {
    case HOLIDAY_MODE_AUTO_ORIGINAL4:
    case HOLIDAY_MODE_AUTO_ALL:
    case HOLIDAY_MODE_NONE:
        hostForcedHoliday = 0;
        break;
    case HOLIDAY_MODE_MANUAL_ORIG4:
        if (hostForcedHoliday < 1 || hostForcedHoliday > 4)
            hostForcedHoliday = 1;
        break;
    case HOLIDAY_MODE_MANUAL_EXPANDED:
        if (hostForcedHoliday <= 4)
            hostForcedHoliday = firstExpandedHoliday();
        break;
    }
    if (menuFreeplayActive)
        pauseMenuRequestFreeplayWorldRefresh = 1;
}

static void cycleHolidaySelection(int dir)
{
    int oldHoliday = hostForcedHoliday;
    if (hostHolidayMode == HOLIDAY_MODE_MANUAL_ORIG4) {
        hostForcedHoliday = nextHolidayInRange(hostForcedHoliday, 1, 4, dir);
    } else if (hostHolidayMode == HOLIDAY_MODE_MANUAL_EXPANDED) {
        hostForcedHoliday = nextHolidayInRange(hostForcedHoliday, 5,
                                               holidayMaxId(), dir);
    }
    if (menuFreeplayActive && hostForcedHoliday != oldHoliday)
        pauseMenuRequestFreeplayWorldRefresh = 1;
}

static void cycleDaynight(int dir)
{
    /* AUTO(-1) -> DAY(0) -> NIGHT(1) -> AUTO */
    hostForcedNight += dir;
    if (hostForcedNight > 1)  hostForcedNight = -1;
    if (hostForcedNight < -1) hostForcedNight = 1;
    if (menuFreeplayActive)
        pauseMenuRequestFreeplayWorldRefresh = 1;
}

static void cycleHoliday(int dir)
{
    cycleHolidaySelection(dir);
}

static const char *tideLabel(void)
{
    switch (hostForcedLowTide) {
    case -1: return "AUTO";
    case  0: return "HIGH";
    case  1: return "LOW";
    default: return "?";
    }
}

static void cycleTide(int dir)
{
    /* AUTO(-1) -> HIGH(0) -> LOW(1) -> AUTO */
    hostForcedLowTide += dir;
    if (hostForcedLowTide > 1)  hostForcedLowTide = -1;
    if (hostForcedLowTide < -1) hostForcedLowTide = 1;
    if (menuFreeplayActive)
        pauseMenuRequestFreeplayWorldRefresh = 1;
}

static const char *raftLabel(void)
{
    switch (hostForcedRaftStage) {
    case -1: return "AUTO";
    case  0: return "NONE";
    case  1: return "1";
    case  2: return "2";
    case  3: return "3";
    case  4: return "4";
    case  5: return "5";
    default: return "?";
    }
}

static void cycleRaft(int dir)
{
    /* AUTO(-1) -> NONE(0) -> 1..5 -> AUTO */
    hostForcedRaftStage += dir;
    if (hostForcedRaftStage > 5)  hostForcedRaftStage = -1;
    if (hostForcedRaftStage < -1) hostForcedRaftStage = 5;
    if (menuFreeplayActive)
        pauseMenuRequestFreeplayWorldRefresh = 1;
}

static void cyclePerf(int dir)
{
    /* OFF(0) <-> SUMMARY(1) <-> DETAIL(2) <-> DEBUG(3) wrapping */
    int lvl = ((int)ps1PerfLevel + dir) & 0x3;
    ps1PerfSetLevel((uint32)lvl);
}

static const char *captionsLabel(void)
{
    return captionsGetEnabled() ? "ON" : "OFF";
}

static void cycleCaptions(int dir)
{
    /* Two states; either direction toggles. */
    (void)dir;
    captionsSetEnabled(!captionsGetEnabled());
}

static void drawMainMenu(void)
{
    pmPrintf(" JOHNNY CASTAWAY  - PAUSED -\n");
    drawSeparator();

    pmPrintf(" %s Resume\n",
             menuCursor == MENU_RESUME ? ">" : " ");
    /* Scene Set Options is a sub-screen; the inline cycler moved into
     * it alongside the new Scene Picker policy selector. The active
     * set name lives inside the sub-screen — appending it here would
     * overflow the panel for the longer set names ("Misc & Suzy",
     * "Johnny Stories", etc.). */
    pmPrintf(" %s Scene Set Options...\n",
             menuCursor == MENU_SCENE_SET ? ">" : " ");
    pmPrintf(" %s Scene Explorer\n",
             menuCursor == MENU_SCENE_EXPLORER ? ">" : " ");
    pmPrintf(" %s Freeplay: %s\n",
             menuCursor == MENU_FREEPLAY ? ">" : " ",
             menuFreeplayActive ? "ON" : "OFF");
    pmPrintf(" %s Freeplay Options\n",
             menuCursor == MENU_FREEPLAY_OPTIONS ? ">" : " ");
    pmPrintf(" %s World Options\n",
             menuCursor == MENU_WORLD ? ">" : " ");
    pmPrintf(" %s Accessibility\n",
             menuCursor == MENU_ACCESSIBILITY ? ">" : " ");
    pmPrintf(" %s System\n",
             menuCursor == MENU_SYSTEM ? ">" : " ");

    drawSeparator();
    pmPrintf("  X = select   O/START = resume\n");
}

/* ---------------------------------------------------------------------------
 *  Sub-screen: Options (sound, day/night, tide, raft, holiday, perf log)
 * ------------------------------------------------------------------------- */
static void drawOptions(void)
{
    pmPrintf("     WORLD OPTIONS\n");
    drawSeparator();

    pmPrintf(" %s Day/Night: %s\n",
             optionsCursor == WORLD_DAYNIGHT ? ">" : " ", daynightLabel());
    pmPrintf(" %s Tide:      %s\n",
             optionsCursor == WORLD_TIDE ? ">" : " ", tideLabel());
    pmPrintf(" %s Raft:      %s\n",
             optionsCursor == WORLD_RAFT ? ">" : " ", raftLabel());
    pmPrintf(" %s Holidays...\n",
             optionsCursor == WORLD_HOLIDAYS ? ">" : " ");
    pmPrintf(" %s Set Island Pos...\n",
             optionsCursor == WORLD_ISLAND_POS ? ">" : " ");
    pmPrintf(" %s Back\n",
             optionsCursor == WORLD_BACK ? ">" : " ");

    drawSeparator();
    pmPrintf(" LEFT/RIGHT or X adjust\n");
    pmPrintf(" O/START = back\n");
}

static void drawHolidayMenu(void)
{
    pmPrintf("        HOLIDAYS\n");
    drawSeparator();
    pmPrintf(" %s Set:     %s\n",
             holidayCursor == HOLIDAY_SET ? ">" : " ", holidaySetLabel());
    pmPrintf(" %s Holiday: %s\n",
             holidayCursor == HOLIDAY_SELECT ? ">" : " ", holidayLabel());
    pmPrintf(" %s Back\n",
             holidayCursor == HOLIDAY_BACK ? ">" : " ");
    drawSeparator();
    if (holidayModeIsManual(hostHolidayMode) &&
        hostForcedHoliday > 0 && holidayById(hostForcedHoliday)) {
        pmPrintf("   %s\n", holidayDateLabel(hostForcedHoliday));
        pmPrintf("   %.28s\n", holidayTitle(hostForcedHoliday));
    } else if ((hostHolidayMode == HOLIDAY_MODE_AUTO_ORIGINAL4 ||
                hostHolidayMode == HOLIDAY_MODE_AUTO_ALL) &&
               ps1SoftTimeEnabled) {
        int dateHoliday = (hostHolidayMode == HOLIDAY_MODE_AUTO_ORIGINAL4)
                        ? holidayForDateOriginal4(ps1SoftYear, ps1SoftMonth, ps1SoftDay)
                        : holidayForDate(ps1SoftYear, ps1SoftMonth, ps1SoftDay);
        pmPrintf("   Date picker: %s\n",
                 dateHoliday ? holidayShortName(dateHoliday) : "NONE");
    } else if (hostHolidayMode == HOLIDAY_MODE_AUTO_ORIGINAL4) {
        pmPrintf("   Random original 4 until\n");
        pmPrintf("   Set Time/Date is saved.\n");
    } else if (hostHolidayMode == HOLIDAY_MODE_AUTO_ALL) {
        pmPrintf("   Random expanded holiday\n");
        pmPrintf("   until Set Time/Date.\n");
    }
    pmPrintf(" O/START = back\n");
}

static void drawAccessibilityMenu(void)
{
    extern int oceanAmbientEnabled;
    const char *soundLabel = soundMuted ? "MUTED" : "ON";
    const char *oceanLabel = oceanAmbientEnabled ? "ON" : "OFF";

    pmPrintf("     ACCESSIBILITY\n");
    drawSeparator();
    pmPrintf(" %s Captions:  %s\n",
             accessCursor == ACCESS_CAPTIONS ? ">" : " ", captionsLabel());
    pmPrintf(" %s Sound:     %s\n",
             accessCursor == ACCESS_SOUND ? ">" : " ", soundLabel);
    pmPrintf(" %s Ocean:     %s\n",
             accessCursor == ACCESS_OCEAN ? ">" : " ", oceanLabel);
    pmPrintf(" %s Sound Test...\n",
             accessCursor == ACCESS_SOUND_TEST ? ">" : " ");
    pmPrintf(" %s Back\n",
             accessCursor == ACCESS_BACK ? ">" : " ");
    drawSeparator();
    pmPrintf(" LEFT/RIGHT or X adjust\n");
    pmPrintf(" O/START = back\n");
}

static const char *soundTestName(int index)
{
    static const char *const names[] = {
        "SOUND00", "SOUND01", "SOUND02", "SOUND03", "SOUND04",
        "SOUND05", "SOUND06", "SOUND07", "SOUND08", "SOUND09",
        "SOUND10", "SOUND11", "SOUND12", "SOUND13", "SOUND14",
        "SOUND15", "SOUND16", "SOUND17", "SOUND18", "SOUND19",
        "SOUND20", "SOUND21", "SOUND22", "SOUND23", "SOUND24"
    };
    if (index < 0 || index >= (int)(sizeof(names) / sizeof(names[0])))
        return "SOUND??";
    return names[index];
}

static void drawSoundTestMenu(void)
{
    int count = soundEffectCount();
    if (count <= 0)
        count = 1;
    if (soundTestCursor < 0)
        soundTestCursor = count - 1;
    if (soundTestCursor >= count)
        soundTestCursor = 0;

    pmPrintf("       SOUND TEST\n");
    drawSeparator();
    pmPrintf(" %02d/%02d  %s\n",
             soundTestCursor + 1, count, soundTestName(soundTestCursor));
    pmPrintf(" Status: %s\n",
             soundEffectLoaded(soundTestCursor) ? "READY" : "MISSING");
    pmPrintf(" Size:   %lu KB\n",
             (soundEffectSizeBytes(soundTestCursor) + 1023UL) / 1024UL);
    pmPrintf(" Rate:   %d Hz\n",
             soundEffectSampleRate(soundTestCursor));
    pmPrintf(" Output: %s\n", soundMuted ? "MUTED" : "ON");
    drawSeparator();
    pmPrintf(" UP/DOWN choose\n");
    pmPrintf(" X play   O/START back\n");
}

static void drawSystemMenu(void)
{
    pmPrintf("        SYSTEM\n");
    drawSeparator();
    pmPrintf(" %s Save Settings\n",
             systemCursor == SYSTEM_SAVE ? ">" : " ");
    pmPrintf(" %s Set Time/Date...\n",
             systemCursor == SYSTEM_SET_TIME ? ">" : " ");
    pmPrintf(" %s Set RNG Seed...\n",
             systemCursor == SYSTEM_SET_SEED ? ">" : " ");
    pmPrintf(" %s Perf Log: %s\n",
             systemCursor == SYSTEM_PERF ? ">" : " ", perfLevelLabel());
    {
        int day = storyCurrentDay;
        if (day < 1 || day > 11) day = 1;
        pmPrintf(" %s Story Day: %s%2d/11%s\n",
                 systemCursor == SYSTEM_STORY_DAY ? ">" : " ",
                 systemCursor == SYSTEM_STORY_DAY ? "<" : " ",
                 day,
                 systemCursor == SYSTEM_STORY_DAY ? ">" : " ");
    }
    pmPrintf(" %s Reset Scene\n",
             systemCursor == SYSTEM_RESET_LOOP ? ">" : " ");
    pmPrintf(" %s Next Scene\n",
             systemCursor == SYSTEM_NEXT_SCENE ? ">" : " ");
    drawSeparator();
    pmPrintf(" O/START = back\n");
}

/* ---------------------------------------------------------------------------
 *  Scene Explorer — text-only sub-screen for v1. Thumbnails (320x208 SCR
 *  per scene) load on cursor change in a follow-up step. Layout fits on
 *  the current 8x8 panel without a thumbnail by using all rows for text.
 * ------------------------------------------------------------------------- */

static int sceneExplorerFamilyForCursor(int cursor)
{
    int i;
    int family = 0;
    for (i = 0; i < gSceneExplorerFamilyCount; i++) {
        if (gSceneExplorerFamilyStart[i] <= cursor) {
            family = i;
        }
    }
    return family;
}

/* Track which thumbnail is currently in VRAM. -1 forces a load on the
 * next sceneExplorerEnsureThumbnail call. Reset to -1 on entry to the
 * explorer state so the first cursor lands fresh; then advances on
 * each cursor change. */
static int sceneExplorerLoadedCursor = -1;

static void sceneExplorerEnsureThumbnail(void)
{
    int cur = pauseMenuExplorerCursor;
    if (cur < 0 || cur >= gSceneExplorerCount) return;
    if (cur == sceneExplorerLoadedCursor) return;

    /* grLoadSceneExplorerThumbnail LoadImages the new thumbnail to the
     * framebuffer at (0,0,320,240). We do NOT touch menuFramebufferPrimed
     * here — re-priming would have grDrawBackground repaint the paused
     * scene's bgTile* over our thumbnail. The chrome strip's OT primitives
     * draw on top of the LoadImage'd pixels each frame. */
    if (grLoadSceneExplorerThumbnail(gSceneExplorer[cur].slug)) {
        sceneExplorerLoadedCursor = cur;
    }
    /* If the load failed (missing thumbnail file on disc — e.g., a
     * not-yet-captured scene), leave fb alone. The chrome strip + text
     * still communicate the metadata; the previous thumbnail (or main
     * menu's bg) shows above. */
}

static void drawSceneExplorer(void)
{
    int cur = pauseMenuExplorerCursor;
    if (cur < 0) cur = 0;
    if (cur >= gSceneExplorerCount) cur = gSceneExplorerCount - 1;

    /* Stream the current cursor's thumbnail SCR into the framebuffer
     * if it isn't already there. The OT primitives below add the
     * top + bottom chrome bands and the text. */
    sceneExplorerEnsureThumbnail();

    const struct TSceneExplorerEntry *e = &gSceneExplorer[cur];

    /* Truncate display name so it fits in the 38-char top-band budget
     * (640 logical px / 16 logical per char ≈ 40 chars; we leave a
     * small right margin). */
    char title[40];
    int max = 38;
    int len = 0;
    while (e->display_name[len] != '\0' && len < max) {
        title[len] = e->display_name[len];
        len++;
    }
    if (e->display_name[len] != '\0' && len > 3) {
        title[len-3] = '.'; title[len-2] = '.'; title[len-1] = '.';
    }
    title[len] = '\0';

    /* Top band — title + position + display name + family/frames.
     * First line starts at y=24 logical so the title isn't clipped by
     * NTSC overscan at the very top of the framebuffer. */
    pmTextStart(pmPrintfX, 24);
    pmPrintf("       SCENE EXPLORER\n");
    drawSeparator();
    pmPrintf(" %d/%d   %s\n",
             cur + 1, gSceneExplorerCount,
             e->validated ? "* validated" : "? pending");
    pmPrintf(" %s\n", title);
    pmPrintf(" Family: %s   Frames: %u\n",
             e->family, (unsigned)e->frame_count);

    /* Bottom band — pack name, separator, nav hints (X/△/O on one
     * line so the band fits in 4 lines instead of 5 and we stay clear
     * of bottom NTSC overscan). */
    pmTextStart(pmPrintfX, 376);
    pmPrintf(" Pack: %s\n", e->pack);
    drawSeparator();
    pmPrintf(" <- -> scene     L1/R1 family\n");
    pmPrintf(" X play  /\\ loop  O back\n");
}

static int handleSceneExplorerInput(uint16 pressed)
{
    if (pressed & PAD_LEFT) {
        pauseMenuExplorerCursor--;
        if (pauseMenuExplorerCursor < 0)
            pauseMenuExplorerCursor = gSceneExplorerCount - 1;
    }
    if (pressed & PAD_RIGHT) {
        pauseMenuExplorerCursor++;
        if (pauseMenuExplorerCursor >= gSceneExplorerCount)
            pauseMenuExplorerCursor = 0;
    }
    if (pressed & PAD_L1) {
        int fam = sceneExplorerFamilyForCursor(pauseMenuExplorerCursor);
        /* Jump to the start of the previous family. If we're already at
         * a family boundary, go back one further. */
        if (pauseMenuExplorerCursor == gSceneExplorerFamilyStart[fam] && fam > 0)
            fam--;
        else if (pauseMenuExplorerCursor == gSceneExplorerFamilyStart[fam])
            fam = gSceneExplorerFamilyCount - 1;
        else
            ; /* fam already points at the current family's start */
        pauseMenuExplorerCursor = gSceneExplorerFamilyStart[fam];
    }
    if (pressed & PAD_R1) {
        int fam = sceneExplorerFamilyForCursor(pauseMenuExplorerCursor);
        fam = (fam + 1) % gSceneExplorerFamilyCount;
        pauseMenuExplorerCursor = gSceneExplorerFamilyStart[fam];
    }
    if (pressed & PAD_CROSS) {
        pauseMenuRequestPlayScene = pauseMenuExplorerCursor;
        pauseMenuRequestLoopScene = -1;
        sceneExplorerLoadedCursor = -1;
        grFreeSceneExplorerThumbnailBuffer();
        return 0;       /* close menu — scene plays on next iteration */
    }
    if (pressed & PAD_TRIANGLE) {
        pauseMenuRequestLoopScene = pauseMenuExplorerCursor;
        pauseMenuRequestPlayScene = -1;
        sceneExplorerLoadedCursor = -1;
        grFreeSceneExplorerThumbnailBuffer();
        return 0;
    }
    if (pressed & (PAD_CIRCLE | PAD_START)) {
        menuState = PAUSE_MENU_MAIN;
        menuCursor = MENU_SCENE_EXPLORER;
        prevButtons = 0xFFFF;
        sceneExplorerLoadedCursor = -1;
        grFreeSceneExplorerThumbnailBuffer();
        /* Force the main menu to re-draw its dim+panel over whatever
         * thumbnail pixels are sitting in the framebuffer. */
        menuFramebufferPrimed = 0;
    }
    return 1;
}

/* ---------------------------------------------------------------------------
 *  Input handling per sub-screen
 * ------------------------------------------------------------------------- */

/* Handle main menu input.  Returns 0 if user chose Resume. */
static int handleMainInput(uint16 pressed)
{
    if (pressed & PAD_UP) {
        menuCursor--;
        if (menuCursor < 0) menuCursor = MENU_COUNT - 1;
    }
    if (pressed & PAD_DOWN) {
        menuCursor++;
        if (menuCursor >= MENU_COUNT) menuCursor = 0;
    }

    /* Scene Set inline cycling moved into the Scene Set Options
     * sub-screen alongside the Scene Picker policy. Keep pending in
     * sync with the committed value so the sub-screen starts on the
     * current set, not a stale preview. */
    pendingSceneSet = pauseMenuSceneSet;

    /* X = select current item */
    if (pressed & PAD_CROSS) {
        switch (menuCursor) {
        case MENU_RESUME:
            return 0;  /* close menu */

        case MENU_FREEPLAY:
            if (menuFreeplayActive) {
                pauseMenuRequestExitFreeplay = 1;
            } else {
                pauseMenuRequestFreeplay = 1;
            }
            return 0;

        case MENU_FREEPLAY_OPTIONS:
            menuState = PAUSE_MENU_FREEPLAY_OPTIONS;
            prevButtons = 0xFFFF;
            break;

        case MENU_SCENE_SET:
            /* Open the Scene Set Options sub-screen. Cursor lands on
             * the Scene Set row (most-used) by default. */
            menuState             = PAUSE_MENU_SCENE_SET_OPTIONS;
            sceneSetOptionsCursor = SSO_SCENE_SET;
            prevButtons           = 0xFFFF;
            break;

        case MENU_SCENE_EXPLORER:
            menuState = PAUSE_MENU_SCENE_EXPLORER;
            sceneExplorerLoadedCursor = -1;  /* force load on first draw */
            prevButtons = 0xFFFF;
            break;

        case MENU_WORLD:
            menuState = PAUSE_MENU_OPTIONS;
            prevButtons = 0xFFFF;
            break;

        case MENU_ACCESSIBILITY:
            menuState = PAUSE_MENU_ACCESSIBILITY;
            prevButtons = 0xFFFF;
            break;

        case MENU_SYSTEM:
            menuState = PAUSE_MENU_SYSTEM;
            prevButtons = 0xFFFF;
            break;

        default:
            break;
        }
    }

    /* START / Circle close the menu — the sub-screen owns its own
     * commit semantics now (X commits + leaves, Circle backs out). */
    if (pressed & PAD_START)
        return 0;
    if (pressed & PAD_CIRCLE)
        return 0;

    return 1;  /* keep menu open */
}

static int handleFreeplayOptionsInput(uint16 pressed)
{
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_MAIN;
        menuCursor = MENU_FREEPLAY_OPTIONS;
        prevButtons = 0xFFFF;
        return 1;
    }
    if (pressed & PAD_UP) {
        freeplayOptionsCursor--;
        if (freeplayOptionsCursor < 0) freeplayOptionsCursor = FPO_COUNT - 1;
    }
    if (pressed & PAD_DOWN) {
        freeplayOptionsCursor++;
        if (freeplayOptionsCursor >= FPO_COUNT) freeplayOptionsCursor = 0;
    }
    if (pressed & PAD_CROSS) {
        switch (freeplayOptionsCursor) {
        case FPO_GAGS:
            menuState = PAUSE_MENU_FREEPLAY_GAGS;
            prevButtons = 0xFFFF;
            break;
        case FPO_VISITORS:
            menuState = PAUSE_MENU_FREEPLAY_VISITORS;
            prevButtons = 0xFFFF;
            break;
        case FPO_CONTROLS:
            menuState = PAUSE_MENU_CONTROLS;
            prevButtons = 0xFFFF;
            break;
        case FPO_CLEAR:
            if (menuFreeplayActive) {
                pauseMenuRequestFreeplayClear = 1;
                return 0;
            }
            break;
        case FPO_BACK:
            menuState = PAUSE_MENU_MAIN;
            menuCursor = MENU_FREEPLAY_OPTIONS;
            prevButtons = 0xFFFF;
            break;
        }
    }
    return 1;
}

static int handleSceneSetOptionsInput(uint16 pressed)
{
    /* Circle / Start: back out. If a Scene Set preview was pending,
     * silently drop it — Circle = "I changed my mind." */
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        pendingSceneSet      = pauseMenuSceneSet;
        menuState            = PAUSE_MENU_MAIN;
        menuCursor           = MENU_SCENE_SET;
        prevButtons          = 0xFFFF;
        return 1;
    }

    if (pressed & PAD_UP) {
        sceneSetOptionsCursor--;
        if (sceneSetOptionsCursor < 0) sceneSetOptionsCursor = SSO_COUNT - 1;
    }
    if (pressed & PAD_DOWN) {
        sceneSetOptionsCursor++;
        if (sceneSetOptionsCursor >= SSO_COUNT) sceneSetOptionsCursor = 0;
    }

    /* Left/Right cycles the focused row's value. Scene Set uses the
     * same pending-preview pattern as before; Picker commits inline
     * because the policy switch is cheap and reversible. */
    if (sceneSetOptionsCursor == SSO_SCENE_SET) {
        if (pressed & PAD_LEFT) {
            pendingSceneSet--;
            if (pendingSceneSet < 0) pendingSceneSet = NUM_SCENE_SETS - 1;
        }
        if (pressed & PAD_RIGHT) {
            pendingSceneSet++;
            if (pendingSceneSet >= NUM_SCENE_SETS) pendingSceneSet = 0;
        }
    } else if (sceneSetOptionsCursor == SSO_PICKER) {
        int policy = pickerGetPolicy();
        if (pressed & PAD_LEFT) {
            policy = (policy + SCENE_PICKER_COUNT - 1) % SCENE_PICKER_COUNT;
            pickerSetPolicy(policy);
        }
        if (pressed & PAD_RIGHT) {
            policy = (policy + 1) % SCENE_PICKER_COUNT;
            pickerSetPolicy(policy);
        }
    }

    /* X commits a pending Scene Set and returns to main; on Picker it
     * just returns (policy is already applied); on Back it returns. */
    if (pressed & PAD_CROSS) {
        if (pendingSceneSet != pauseMenuSceneSet) {
            pauseMenuSceneSet             = pendingSceneSet;
            pauseMenuRequestSceneSetCycle = 1;
        }
        if (sceneSetOptionsCursor == SSO_BACK) {
            menuState   = PAUSE_MENU_MAIN;
            menuCursor  = MENU_SCENE_SET;
            prevButtons = 0xFFFF;
            return 1;
        }
        /* SSO_SCENE_SET commit: close the menu so the cycle banner
         * shows immediately. SSO_PICKER X: also close — user picked,
         * we're done. Mirrors the old "X commits Scene Set + closes"
         * UX from the legacy main-menu inline cycler. */
        return 0;
    }

    return 1;
}

static int handleFreeplayGagInput(uint16 pressed)
{
    int count = freeplayGagCount();
    if (count <= 0)
        count = 1;
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_FREEPLAY_OPTIONS;
        freeplayOptionsCursor = FPO_GAGS;
        prevButtons = 0xFFFF;
        return 1;
    }
    if (pressed & (PAD_UP | PAD_LEFT)) {
        freeplayGagCursor--;
        if (freeplayGagCursor < 0) freeplayGagCursor = count - 1;
    }
    if (pressed & (PAD_DOWN | PAD_RIGHT)) {
        freeplayGagCursor++;
        if (freeplayGagCursor >= count) freeplayGagCursor = 0;
    }
    if ((pressed & PAD_CROSS) && menuFreeplayActive) {
        pauseMenuRequestFreeplayGag = freeplayGagCursor;
        return 0;
    }
    return 1;
}

static int handleFreeplayVisitorInput(uint16 pressed)
{
    int count = freeplayVisitorCount();
    if (count <= 0)
        count = 1;
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_FREEPLAY_OPTIONS;
        freeplayOptionsCursor = FPO_VISITORS;
        prevButtons = 0xFFFF;
        return 1;
    }
    if (pressed & (PAD_UP | PAD_LEFT)) {
        freeplayVisitorCursor--;
        if (freeplayVisitorCursor < 0) freeplayVisitorCursor = count - 1;
    }
    if (pressed & (PAD_DOWN | PAD_RIGHT)) {
        freeplayVisitorCursor++;
        if (freeplayVisitorCursor >= count) freeplayVisitorCursor = 0;
    }
    if ((pressed & PAD_CROSS) && menuFreeplayActive) {
        pauseMenuRequestFreeplayVisitor = freeplayVisitorCursor;
        return 0;
    }
    return 1;
}

/* Apply a +1 / -1 cycle to the field at `optionsCursor`. */
static void optionsCycle(int dir)
{
    switch (optionsCursor) {
    case WORLD_DAYNIGHT: cycleDaynight(dir); break;
    case WORLD_TIDE:     cycleTide(dir);     break;
    case WORLD_RAFT:     cycleRaft(dir);     break;
    default: break;
    }
}

/* Options sub-screen input. UP/DOWN move cursor; on cycle rows
 * LEFT/RIGHT or X cycle the value; on launcher rows X opens the
 * editor sub-screen. Circle/START goes back to the main menu. */
static int handleOptionsInput(uint16 pressed)
{
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_MAIN;
        menuCursor = MENU_WORLD;   /* return cursor to where we came from */
        prevButtons = 0xFFFF;
        return 1;
    }

    if (pressed & PAD_UP) {
        optionsCursor--;
        if (optionsCursor < 0) optionsCursor = WORLD_COUNT - 1;
    }
    if (pressed & PAD_DOWN) {
        optionsCursor++;
        if (optionsCursor >= WORLD_COUNT) optionsCursor = 0;
    }

    if (pressed & (PAD_RIGHT | PAD_CROSS)) {
        switch (optionsCursor) {
        case WORLD_DAYNIGHT:
        case WORLD_TIDE:
        case WORLD_RAFT:
            optionsCycle(+1);
            if (menuFreeplayActive)
                return 0;
            break;
        case WORLD_HOLIDAYS:
            menuState = PAUSE_MENU_HOLIDAYS;
            prevButtons = 0xFFFF;
            break;
        case WORLD_ISLAND_POS:
            editIslandX     = hostForcedIslandX;
            editIslandY     = hostForcedIslandY;
            editIslandValid = hostForcedIslandPosValid;
            editIslandField = 0;
            menuState   = PAUSE_MENU_ISLAND_POS;
            prevButtons = 0xFFFF;
            break;
        case WORLD_BACK:
            menuState = PAUSE_MENU_MAIN;
            menuCursor = MENU_WORLD;
            prevButtons = 0xFFFF;
            break;
        }
    } else if (pressed & PAD_LEFT) {
        switch (optionsCursor) {
        case WORLD_DAYNIGHT:
        case WORLD_TIDE:
        case WORLD_RAFT:
            optionsCycle(-1);
            if (menuFreeplayActive)
                return 0;
            break;
        default:
            break;
        }
    }

    return 1;
}

static int handleHolidayInput(uint16 pressed)
{
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_OPTIONS;
        optionsCursor = WORLD_HOLIDAYS;
        prevButtons = 0xFFFF;
        return 1;
    }
    if (pressed & PAD_UP) {
        holidayCursor--;
        if (holidayCursor < 0) holidayCursor = HOLIDAY_COUNT - 1;
    }
    if (pressed & PAD_DOWN) {
        holidayCursor++;
        if (holidayCursor >= HOLIDAY_COUNT) holidayCursor = 0;
    }
    if (pressed & (PAD_RIGHT | PAD_CROSS)) {
        if (holidayCursor == HOLIDAY_SET) {
            cycleHolidaySet(+1);
            if (menuFreeplayActive)
                return 0;
        } else if (holidayCursor == HOLIDAY_SELECT) {
            cycleHoliday(+1);
            if (menuFreeplayActive && pauseMenuRequestFreeplayWorldRefresh)
                return 0;
        } else if (holidayCursor == HOLIDAY_BACK) {
            menuState = PAUSE_MENU_OPTIONS;
            optionsCursor = WORLD_HOLIDAYS;
            prevButtons = 0xFFFF;
        }
    } else if (pressed & PAD_LEFT) {
        if (holidayCursor == HOLIDAY_SET) {
            cycleHolidaySet(-1);
            if (menuFreeplayActive)
                return 0;
        } else if (holidayCursor == HOLIDAY_SELECT) {
            cycleHoliday(-1);
            if (menuFreeplayActive && pauseMenuRequestFreeplayWorldRefresh)
                return 0;
        }
    }
    return 1;
}

static int handleAccessibilityInput(uint16 pressed)
{
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_MAIN;
        menuCursor = MENU_ACCESSIBILITY;
        prevButtons = 0xFFFF;
        return 1;
    }
    if (pressed & PAD_UP) {
        accessCursor--;
        if (accessCursor < 0) accessCursor = ACCESS_COUNT - 1;
    }
    if (pressed & PAD_DOWN) {
        accessCursor++;
        if (accessCursor >= ACCESS_COUNT) accessCursor = 0;
    }
    if (pressed & (PAD_LEFT | PAD_RIGHT | PAD_CROSS)) {
        switch (accessCursor) {
        case ACCESS_CAPTIONS:
            cycleCaptions(+1);
            break;
        case ACCESS_SOUND:
            soundMuteToggle();
            pauseMutedSound = 0;
            break;
        case ACCESS_OCEAN: {
            extern int oceanAmbientEnabled;
            extern void oceanAmbientStart(void);
            extern void oceanAmbientStop(void);
            oceanAmbientEnabled = !oceanAmbientEnabled;
            if (oceanAmbientEnabled) oceanAmbientStart();
            else                     oceanAmbientStop();
            break;
        }
        case ACCESS_SOUND_TEST:
            menuState = PAUSE_MENU_SOUND_TEST;
            prevButtons = 0xFFFF;
            break;
        case ACCESS_BACK:
            menuState = PAUSE_MENU_MAIN;
            menuCursor = MENU_ACCESSIBILITY;
            prevButtons = 0xFFFF;
            break;
        }
    }
    return 1;
}

static int handleSoundTestInput(uint16 pressed)
{
    int count = soundEffectCount();
    if (count <= 0)
        count = 1;
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_ACCESSIBILITY;
        accessCursor = ACCESS_SOUND_TEST;
        prevButtons = 0xFFFF;
        return 1;
    }
    if (pressed & (PAD_UP | PAD_LEFT)) {
        soundTestCursor--;
        if (soundTestCursor < 0) soundTestCursor = count - 1;
    }
    if (pressed & (PAD_DOWN | PAD_RIGHT)) {
        soundTestCursor++;
        if (soundTestCursor >= count) soundTestCursor = 0;
    }
    if (pressed & PAD_CROSS)
        soundPlay(soundTestCursor);
    return 1;
}

static int handleSystemInput(uint16 pressed)
{
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_MAIN;
        menuCursor = MENU_SYSTEM;
        prevButtons = 0xFFFF;
        return 1;
    }
    if (pressed & PAD_UP) {
        systemCursor--;
        if (systemCursor < 0) systemCursor = SYSTEM_COUNT - 1;
    }
    if (pressed & PAD_DOWN) {
        systemCursor++;
        if (systemCursor >= SYSTEM_COUNT) systemCursor = 0;
    }
    if (pressed & PAD_RIGHT) {
        if (systemCursor == SYSTEM_PERF)
            cyclePerf(+1);
        else if (systemCursor == SYSTEM_STORY_DAY) {
            storyCurrentDay++;
            if (storyCurrentDay > 11) storyCurrentDay = 1;
            if (storyCurrentDay < 1)  storyCurrentDay = 1;
        }
    } else if (pressed & PAD_LEFT) {
        if (systemCursor == SYSTEM_PERF)
            cyclePerf(-1);
        else if (systemCursor == SYSTEM_STORY_DAY) {
            storyCurrentDay--;
            if (storyCurrentDay < 1) storyCurrentDay = 11;
        }
    } else if (pressed & PAD_CROSS) {
        switch (systemCursor) {
        case SYSTEM_SAVE:
            memcardSaveSettings();
            break;
        case SYSTEM_SET_TIME:
            editMonth  = ps1SoftMonth;
            editDay    = ps1SoftDay;
            editYear   = ps1SoftYear;
            editHour   = ps1SoftHour;
            editMinute = ps1SoftMinute;
            editField  = 0;
            menuState   = PAUSE_MENU_SET_TIME;
            prevButtons = 0xFFFF;
            break;
        case SYSTEM_SET_SEED:
            editSeedValue = ps1LastSeedKnown ? ps1LastSeedApplied : 1u;
            editSeedFixed = 0;
            editSeedField = 0;
            menuState   = PAUSE_MENU_SET_SEED;
            prevButtons = 0xFFFF;
            break;
        case SYSTEM_PERF:
            cyclePerf(+1);
            break;
        case SYSTEM_RESET_LOOP:
            pauseMenuRequestResetLoop = 1;
            return 0;
        case SYSTEM_NEXT_SCENE:
            pauseMenuRequestNextScene = 1;
            return 0;
        }
    }
    return 1;
}

/* Returns 0 if user goes back to main. */
static int handleSubInput(uint16 pressed)
{
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        enum PauseMenuState fromState = menuState;
        if (fromState == PAUSE_MENU_CONTROLS) {
            menuState = PAUSE_MENU_FREEPLAY_OPTIONS;
            freeplayOptionsCursor = FPO_CONTROLS;
            prevButtons = 0xFFFF;
            return 1;
        }
        menuState = PAUSE_MENU_MAIN;
        if (fromState == PAUSE_MENU_CREDITS)
            menuCursor = MENU_SYSTEM;
        else
            menuCursor = MENU_RESUME;
        prevButtons = 0xFFFF;
        return 1;
    }
    return 1;
}

/* Set Time input -- LEFT/RIGHT adjust value, UP/DOWN change field. */
static int handleSetTimeInput(uint16 pressed)
{
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_SYSTEM;
        systemCursor = SYSTEM_SET_TIME;
        prevButtons = 0xFFFF;
        return 1;
    }

    if (pressed & PAD_UP) {
        editField--;
        if (editField < 0) editField = 4;
    }
    if (pressed & PAD_DOWN) {
        editField++;
        if (editField > 4) editField = 0;
    }

    int delta = 0;
    if (pressed & PAD_RIGHT) delta = 1;
    if (pressed & PAD_LEFT)  delta = -1;

    if (delta) {
        switch (editField) {
        case 0: editMonth  = clampInt(editMonth + delta, 1, 12); break;
        case 1: editDay    = clampInt(editDay + delta, 1, daysInMonth(editMonth, editYear)); break;
        case 2: editYear   = clampInt(editYear + delta, 1990, 2099); break;
        case 3: editHour   = clampInt(editHour + delta, 0, 23); break;
        case 4: editMinute = clampInt(editMinute + delta, 0, 59); break;
        }
    }

    /* X = confirm -- write values into the PS1 time stubs.
     * The game reads time via getHour()/getMonthAndDay()/getDayOfYear() in
     * utils.c.  We update those stubs through extern globals exposed
     * specifically for the pause menu. */
    if (pressed & PAD_CROSS) {
        extern int ps1SoftHour;
        extern int ps1SoftMinute;
        extern int ps1SoftMonth;
        extern int ps1SoftDay;
        extern int ps1SoftYear;
        extern int ps1SoftTimeEnabled;
        ps1SoftTimeEnabled = 1;  /* future scene picks honor user-set date */

        ps1SoftHour  = editHour;
        ps1SoftMinute = editMinute;
        ps1SoftMonth = editMonth;
        ps1SoftDay   = editDay;
        ps1SoftYear  = editYear;
        if (hostHolidayMode != HOLIDAY_MODE_AUTO_ORIGINAL4 &&
            hostHolidayMode != HOLIDAY_MODE_AUTO_ALL)
            hostHolidayMode = HOLIDAY_MODE_AUTO_ORIGINAL4;
        hostForcedHoliday = 0;  /* date picker drives holiday while in AUTO */
        /* getDayOfYear() in utils.c computes from ps1SoftMonth/ps1SoftDay */

        /* Return to System after confirming. */
        menuState = PAUSE_MENU_SYSTEM;
        systemCursor = SYSTEM_SET_TIME;
        prevButtons = 0xFFFF;
        if (menuFreeplayActive) {
            pauseMenuRequestFreeplayWorldRefresh = 1;
            return 0;
        }
    }

    return 1;
}

/* Set Island Pos input. UP/DOWN selects field; LEFT/RIGHT adjusts;
 * X confirms (write through to host overrides) and goes back; Circle/START
 * goes back without applying changes. */
static int handleIslandPosInput(uint16 pressed)
{
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_OPTIONS;
        optionsCursor = WORLD_ISLAND_POS;
        prevButtons = 0xFFFF;
        return 1;
    }

    if (pressed & PAD_UP) {
        editIslandField--;
        if (editIslandField < 0) editIslandField = 2;
    }
    if (pressed & PAD_DOWN) {
        editIslandField++;
        if (editIslandField > 2) editIslandField = 0;
    }

    int delta = 0;
    if (pressed & PAD_RIGHT) delta = +1;
    if (pressed & PAD_LEFT)  delta = -1;
    if (delta) {
        switch (editIslandField) {
        case 0:
            editIslandX = clampInt(editIslandX + delta * 4, -250, 250);
            /* Editing X implies the user wants MANUAL mode. */
            editIslandValid = 1;
            break;
        case 1:
            editIslandY = clampInt(editIslandY + delta * 4, -100, 200);
            editIslandValid = 1;
            break;
        case 2:
            editIslandValid = !editIslandValid;
            break;
        }
    }

    if (pressed & PAD_CROSS) {
        hostForcedIslandPosValid = editIslandValid;
        hostForcedIslandX        = editIslandX;
        hostForcedIslandY        = editIslandY;
        menuState  = PAUSE_MENU_OPTIONS;
        optionsCursor = WORLD_ISLAND_POS;
        prevButtons = 0xFFFF;
        if (menuFreeplayActive) {
            pauseMenuRequestFreeplayWorldRefresh = 1;
            return 0;
        }
    }

    return 1;
}

/* Set RNG Seed input. */
static int handleSetSeedInput(uint16 pressed)
{
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_SYSTEM;
        systemCursor = SYSTEM_SET_SEED;
        prevButtons = 0xFFFF;
        return 1;
    }

    if (pressed & PAD_UP) {
        editSeedField--;
        if (editSeedField < 0) editSeedField = 1;
    }
    if (pressed & PAD_DOWN) {
        editSeedField++;
        if (editSeedField > 1) editSeedField = 0;
    }

    /* Determine step: +/-1, +/-100, or +/-10000 depending on
     * which shoulder button is held alongside LEFT/RIGHT.
     * (LEFT/RIGHT alone = 1; L1/R1 alone = 100; L2/R2 alone = 10000.) */
    int step = 0;
    int dir  = 0;
    if (pressed & PAD_RIGHT)   { dir = +1; step = 1; }
    if (pressed & PAD_LEFT)    { dir = -1; step = 1; }
    if (pressed & PAD_R1)      { dir = +1; step = 100; }
    if (pressed & PAD_L1)      { dir = -1; step = 100; }
    if (pressed & PAD_R2)      { dir = +1; step = 10000; }
    if (pressed & PAD_L2)      { dir = -1; step = 10000; }

    if (dir != 0) {
        if (editSeedField == 0) {
            editSeedFixed = !editSeedFixed;
        } else {
            unsigned int delta = (unsigned int)step;
            if (dir > 0) editSeedValue += delta;
            else         editSeedValue -= delta;
        }
    }

    if (pressed & PAD_CROSS) {
        if (editSeedFixed) {
            ps1SetSeed(editSeedValue);
        } else {
            ps1SeedRandom();   /* re-seed via root counters */
        }
        menuState   = PAUSE_MENU_SYSTEM;
        systemCursor = SYSTEM_SET_SEED;
        prevButtons = 0xFFFF;
    }

    return 1;
}

/* ---------------------------------------------------------------------------
 *  pauseMenuUpdate -- one frame of the overlay
 *
 *  Called in a tight VSync loop while paused.  Reads pad, updates state,
 *  draws text, uploads background.  Returns 1 to stay paused, 0 to resume.
 * ------------------------------------------------------------------------- */
int pauseMenuUpdate(void)
{
    int drawDim;

    if (!menuVisible) return 0;

    /* Match the scene renderer: wait first, then touch the single
     * framebuffer. Waiting at the end caused the next pause iteration to
     * erase the completed menu immediately after VBlank, making the display
     * alternate between raw scene and menu. */
    VSync(0);

    drawDim = !menuFramebufferPrimed;

    if (!menuFramebufferPrimed) {
        /* Restore the raw scene once, then draw the semi-transparent dim
         * once. Re-uploading the raw scene every pause frame fights the
         * single framebuffer and produces visible scene/menu flicker. */
        grForceFullRedrawNextFrame();
        grDrawBackground();
        DrawSync(0);
    }

    /* Upload our font on first frame. */
    if (!pmFontUploaded)
        pmUploadFont();

    /* Build OT, slots high-to-low = drawn first-to-last:
     *   N-1: TPAGE (font tpage with abr=0 for 50% blend)
     *   N-2: dim quad (semi-trans full-screen black)
     *   N-3: panel quads (3, with corner cutouts for rounded look)
     *   N-4: text SPRTs
     */
    ClearOTagR(pauseOt, PAUSE_OT_LEN);
    uint8 *next = pausePrimBuf;

    DR_TPAGE *tp = (DR_TPAGE*)next;
    next += sizeof(DR_TPAGE);
    setDrawTPage(tp, 0, 1, getTPage(0, 0, PM_FONT_VRAM_X, PM_FONT_VRAM_Y));
    ps1GpuOtAddPrim(&pauseOt[PAUSE_OT_LEN - 1], tp);

    /* Panel-quad build is intentionally deferred until AFTER the input
     * handler runs (below) — it depends on the menu state the input
     * handler may transition to. Otherwise the first-frame transition
     * into Scene Explorer (or any sub-screen) would still build panels
     * for the OUTGOING state and cover the new screen's content. */

    /* Read pad through the game's shared pad buffer (events_ps1.c owns
     * InitPAD, so we just peek at its buffer via the extern). */
    extern uint8 pad_buff[2][34];
    uint16 cur = 0;
    for (int i = 0; i < 2; i++) {
        PADTYPE *pad = (PADTYPE *)pad_buff[i];
        /* Don't gate on pad->stat — DualShock analog reports non-zero
         * stat values in some modes and pad->btn is still valid. */
        cur |= ps1PadButtonsWithAnalog(pad);
    }
    uint16 pressed = pmNewPress(cur);
    prevButtons = cur;

    /* Dispatch to current sub-screen input handler. */
    enum PauseMenuState oldState = menuState;
    int oldMenuCursor = menuCursor;
    int oldOptionsCursor = optionsCursor;
    int oldFreeplayOptionsCursor = freeplayOptionsCursor;
    int oldAccessCursor = accessCursor;
    int oldSystemCursor = systemCursor;
    int keepOpen = 1;
    switch (menuState) {
    case PAUSE_MENU_MAIN:
        keepOpen = handleMainInput(pressed);
        break;
    case PAUSE_MENU_FREEPLAY_OPTIONS:
        keepOpen = handleFreeplayOptionsInput(pressed);
        break;
    case PAUSE_MENU_FREEPLAY_GAGS:
        keepOpen = handleFreeplayGagInput(pressed);
        break;
    case PAUSE_MENU_FREEPLAY_VISITORS:
        keepOpen = handleFreeplayVisitorInput(pressed);
        break;
    case PAUSE_MENU_OPTIONS:
        keepOpen = handleOptionsInput(pressed);
        break;
    case PAUSE_MENU_HOLIDAYS:
        keepOpen = handleHolidayInput(pressed);
        break;
    case PAUSE_MENU_ACCESSIBILITY:
        keepOpen = handleAccessibilityInput(pressed);
        break;
    case PAUSE_MENU_SOUND_TEST:
        keepOpen = handleSoundTestInput(pressed);
        break;
    case PAUSE_MENU_SYSTEM:
        keepOpen = handleSystemInput(pressed);
        break;
    case PAUSE_MENU_SET_TIME:
        keepOpen = handleSetTimeInput(pressed);
        break;
    case PAUSE_MENU_ISLAND_POS:
        keepOpen = handleIslandPosInput(pressed);
        break;
    case PAUSE_MENU_SET_SEED:
        keepOpen = handleSetSeedInput(pressed);
        break;
    case PAUSE_MENU_SCENE_INFO:
    case PAUSE_MENU_CONTROLS:
    case PAUSE_MENU_CREDITS:
        keepOpen = handleSubInput(pressed);
        break;
    case PAUSE_MENU_SCENE_EXPLORER:
        keepOpen = handleSceneExplorerInput(pressed);
        break;
    case PAUSE_MENU_SCENE_SET_OPTIONS:
        keepOpen = handleSceneSetOptionsInput(pressed);
        break;
    }

    /* If the state transitioned this frame (e.g. out of Scene Explorer
     * back to main menu), the framebuffer is full of the OUTGOING
     * screen's pixels. Re-prime now: re-upload the paused scene's bg
     * tiles, then let the panel/text draws below paint chrome on top.
     * Without this the main menu would draw over thumbnail+bands and
     * leave bits of them peeking out around the panel edges. */
    if (oldState != menuState && keepOpen) {
        grForceFullRedrawNextFrame();
        grDrawBackground();
        DrawSync(0);
        drawDim = 1;          /* re-dim on transition */
        menuFramebufferPrimed = 0;
    }

    if (ps1PadScriptVerboseLogEnabled() &&
        (pressed ||
         oldState != menuState ||
         oldMenuCursor != menuCursor ||
         oldOptionsCursor != optionsCursor ||
         oldFreeplayOptionsCursor != freeplayOptionsCursor ||
         oldAccessCursor != accessCursor ||
         oldSystemCursor != systemCursor ||
         !keepOpen)) {
        printf("JCPAUSE input state=%d->%d main=%d->%d opt=%d->%d fp=%d->%d acc=%d->%d sys=%d->%d cur=%04x pressed=%04x keep=%d\n",
               (int)oldState, (int)menuState,
               oldMenuCursor, menuCursor,
               oldOptionsCursor, optionsCursor,
               oldFreeplayOptionsCursor, freeplayOptionsCursor,
               oldAccessCursor, accessCursor,
               oldSystemCursor, systemCursor,
               (unsigned)cur,
               (unsigned)pressed,
               keepOpen);
    }

    if (!keepOpen) {
        pauseMenuHide();
        /* Force a full bgTile re-upload on the first scene frame after
         * resume — our dim/panel quads painted into VRAM and we need
         * the scene to fully repaint over them. prevDirty survives
         * grRestoreBgTiles' currDirty reset; setting both is the
         * standard pattern (see grFadeOut/grFreeCleanBgTiles). */
        grForceFullRedrawNextFrame();
        return 0;
    }

    /* Build panel quads NOW (post-input-handler) so the menu state we
     * transition into this frame controls the panel layout, not the
     * outgoing state. */
    if (menuState == PAUSE_MENU_SCENE_EXPLORER) {
        /* Scene Explorer chrome — two opaque dark-purple bands sitting
         * ABOVE and BELOW the centered 320x240 thumbnail (which lives at
         * fb (160, 120, 320, 240) per grLoadSceneExplorerThumbnail).
         * The thumbnail rect is left untouched.
         *
         *      +------+------+------+
         *      |    top band         |   y=0..120 logical, 5 lines
         *      +------+------+------+
         *      | left | thumb| rite |   y=120..360, image
         *      +------+------+------+
         *      |   bottom band       |   y=360..480, 5 lines
         *      +------+------+------+
         */
        POLY_F4 *pTop = (POLY_F4*)next;
        next += sizeof(POLY_F4);
        setPolyF4(pTop);
        setRGB0(pTop, 0x10, 0x08, 0x20);
        setXY4(pTop,   0,   0, 640,   0,
                       0, 120, 640, 120);
        ps1GpuOtAddPrim(&pauseOt[PAUSE_OT_LEN - 3], pTop);

        POLY_F4 *pBot = (POLY_F4*)next;
        next += sizeof(POLY_F4);
        setPolyF4(pBot);
        setRGB0(pBot, 0x10, 0x08, 0x20);
        setXY4(pBot,   0, 360, 640, 360,
                       0, 480, 640, 480);
        ps1GpuOtAddPrim(&pauseOt[PAUSE_OT_LEN - 3], pBot);

        pmFramePrimNext = next;
        pmFrameOtSlot   = &pauseOt[PAUSE_OT_LEN - 4];
        pmPrintfX       = 8;
        pmTextStart(pmPrintfX, 8);   /* first line lands in top band */
    } else {
        pmBuildPanelQuads(&next,
                          drawDim ? &pauseOt[PAUSE_OT_LEN - 2] : NULL,
                          &pauseOt[PAUSE_OT_LEN - 3]);

        pmFramePrimNext = next;
        pmFrameOtSlot   = &pauseOt[PAUSE_OT_LEN - 4];
        pmPrintfX       = PM_PANEL_X0 + 24;
        pmTextStart(pmPrintfX, PM_PANEL_Y0 + 24);
    }

    /* Draw the appropriate screen — pmPrintf adds SPRT primitives to
     * pauseOt via the pmFrame* globals. */
    switch (menuState) {
    case PAUSE_MENU_MAIN:       drawMainMenu();  break;
    case PAUSE_MENU_FREEPLAY_OPTIONS: drawFreeplayOptions(); break;
    case PAUSE_MENU_FREEPLAY_GAGS: drawFreeplayGagCatalog(); break;
    case PAUSE_MENU_FREEPLAY_VISITORS: drawFreeplayVisitorCatalog(); break;
    case PAUSE_MENU_OPTIONS:    drawOptions();   break;
    case PAUSE_MENU_HOLIDAYS:   drawHolidayMenu(); break;
    case PAUSE_MENU_ACCESSIBILITY: drawAccessibilityMenu(); break;
    case PAUSE_MENU_SOUND_TEST: drawSoundTestMenu(); break;
    case PAUSE_MENU_SYSTEM:     drawSystemMenu(); break;
    case PAUSE_MENU_SCENE_INFO: drawSceneInfo(); break;
    case PAUSE_MENU_CONTROLS:   drawControls();  break;
    case PAUSE_MENU_SET_TIME:   drawSetTime();   break;
    case PAUSE_MENU_ISLAND_POS: drawIslandPos(); break;
    case PAUSE_MENU_SET_SEED:   drawSetSeed();   break;
    case PAUSE_MENU_CREDITS:    drawCredits();   break;
    case PAUSE_MENU_SCENE_EXPLORER: drawSceneExplorer(); break;
    case PAUSE_MENU_SCENE_SET_OPTIONS: drawSceneSetOptions(); break;
    }

    /* Submit the OT to GPU. */
    DrawOTag(&pauseOt[PAUSE_OT_LEN - 1]);
    DrawSync(0);
    menuFramebufferPrimed = 1;

#if PAUSE_MENU_DIAG_LOGS
    /* JCPAUSE per-frame diag. */
    {
        static uint32 pauseFrameDbg = 0;
        if ((pauseFrameDbg++ % 60) == 0) {
            printf("JCPAUSE flush #%lu fontID=%d state=%d cursor=%d\n",
                   (unsigned long)pauseFrameDbg, fontID, (int)menuState, menuCursor);
        }
    }
#endif

    return 1;
}
