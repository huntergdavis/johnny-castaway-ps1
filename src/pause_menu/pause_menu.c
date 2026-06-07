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
#define SCENE_EXPLORER_DATA_DEFINE
#include "scene_explorer_data.h"
#undef SCENE_EXPLORER_DATA_DEFINE
#include "holidays.h"
#include "ps1_captions.h"
#include "ps1_pad_input.h"
#include "scene_freeplay.h"
#include "scene_picker.h"
#include "config/ps1/build_date_embedded.h"

#ifndef PAUSE_MENU_DIAG_LOGS
#define PAUSE_MENU_DIAG_LOGS 0
#endif
#ifndef PS1_VERBOSE_DIAGNOSTICS
#define PS1_VERBOSE_DIAGNOSTICS 0
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
#include "pause_menu/lifecycle.c.inc"
#include "pause_menu/text_panel.c.inc"
#include "pause_menu/presentation.c.inc"
#include "pause_menu/option_helpers.c.inc"
#include "pause_menu/scene_explorer.c.inc"
#include "pause_menu/input.c.inc"
#include "pause_menu/update.c.inc"
