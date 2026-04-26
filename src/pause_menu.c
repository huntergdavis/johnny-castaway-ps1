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
#include "memcard.h"
#include "holidays.h"

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

/* Frame counter from graphics_ps1.c (incremented per scene frame). */
extern uint32 ps1FrameCount;

/* Controller pad buffer from events_ps1.c. */

/* ---------------------------------------------------------------------------
 *  Menu state
 * ------------------------------------------------------------------------- */
static int              menuVisible  = 0;
static int              menuCursor   = 0;
static enum PauseMenuState menuState = PAUSE_MENU_MAIN;

/* "Next scene" request flag consumed by ads/story loop. */
int pauseMenuRequestNextScene = 0;

/* "Reset loop" flag — foreground pilot loop checks this and exits
 * early so jc_reborn's outer loop can restart from scene 0. */
int pauseMenuRequestResetLoop = 0;

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
    printf("JCPAUSE pmUploadFont done — font @ (%d,%d) clut @ (%d,%d)\n",
           PM_FONT_VRAM_X, PM_FONT_VRAM_Y, PM_CLUT_VRAM_X, PM_CLUT_VRAM_Y);
}

/* Time/date editing fields. */
static int editField  = 0;   /* 0=month,1=day,2=year,3=hour,4=min */
static int editMonth  = 6;
static int editDay    = 30;
static int editYear   = 2026;
static int editHour   = 12;
static int editMinute = 0;

/* Debounce: tracks which buttons were held last frame so we only act on
 * fresh presses (not auto-repeat while held). */
static uint16 prevButtons = 0;

/* ---------------------------------------------------------------------------
 *  Main menu item descriptors
 * ------------------------------------------------------------------------- */
enum {
    MENU_RESUME,
    MENU_SOUND,
    MENU_DAYNIGHT,
    MENU_HOLIDAY,
    MENU_SAVE,
    MENU_RESET_LOOP,
    MENU_NEXT_SCENE,
    MENU_PERF_TOGGLE,
    MENU_DEBUG_INFO,
    MENU_SET_TIME,
    MENU_COUNT
};

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
void pauseMenuInit(void)
{
    /* Reload BIOS font into VRAM -- graphicsInit may have overwritten it. */
    FntLoad(960, 0);

    /* Match OLD (working) commit b4476f3d: only open a stream if none
     * exists. ps1DebugInit already opened one as fontID=0; reuse that. */
    if (fontID < 0) {
        fontID = FntOpen(0, 0, 640, 480, 0, 1024);
    }
    printf("JCPAUSE pauseMenuInit fontID=%d\n", fontID);

    menuVisible = 0;
    menuCursor  = 0;
    menuState   = PAUSE_MENU_MAIN;
}

void pauseMenuShow(void)
{
    /* Reload font VRAM each time we enter the menu -- the game's
     * LoadImage uploads may have clobbered the (960,0) font area. */
    FntLoad(960, 0);

    menuVisible = 1;
    menuCursor  = 0;
    menuState   = PAUSE_MENU_MAIN;
    prevButtons = 0xFFFF;  /* Treat all buttons as "held" so the initial
                              press that opened the menu is not re-acted. */

    /* Sound stays in whatever state the user left it — defaults ON.
     * The Sound menu item lets the user mute/unmute manually. (When
     * memory-card support lands, we'll persist the user's choice.) */
    pauseMutedSound = 0;

    /* P6: one-shot JCPAUSE snapshot for log-mining. */
    printf("JCPAUSE show frame=%lu scene=%s mode=%s perfLevel=%u soundMuted=%d\n",
           (unsigned long)ps1FrameCount,
           foregroundPilotRuntimeSceneName(),
           foregroundPilotRuntimeModeName(),
           (unsigned)ps1PerfLevel,
           soundMuted);

    /* Refresh the heap-free cache once per pause-show. fgProbeLargestAlloc
     * does ~9 malloc/free cycles which fragments the heap if called every
     * pause-loop iteration; caching avoids that. */
    pmCachedHeapKB = fgProbeLargestAlloc() / 1024UL;
}

void pauseMenuHide(void)
{
    menuVisible = 0;
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
    addPrim(otSlot, sprt);
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
    addPrim(otDim, dim);

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
    addPrim(otPanel, p1);

    /* Top edge (narrower). */
    POLY_F4 *p2 = (POLY_F4*)*nextp;
    *nextp += sizeof(POLY_F4);
    setPolyF4(p2);
    setRGB0(p2, r, g, b);
    setXY4(p2, x0 + c, y0, x1 - c, y0, x0 + c, y0 + c, x1 - c, y0 + c);
    addPrim(otPanel, p2);

    /* Bottom edge (narrower). */
    POLY_F4 *p3 = (POLY_F4*)*nextp;
    *nextp += sizeof(POLY_F4);
    setPolyF4(p3);
    setRGB0(p3, r, g, b);
    setXY4(p3, x0 + c, y1 - c, x1 - c, y1 - c, x0 + c, y1, x1 - c, y1);
    addPrim(otPanel, p3);
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
    pmPrintf(" START = BACK");
}

/* ---------------------------------------------------------------------------
 *  Sub-screen: Controls
 * ------------------------------------------------------------------------- */
static void drawControls(void)
{
    pmPrintf("\n");
    drawSeparator();
    pmPrintf("     CONTROLS\n");
    drawSeparator();
    pmPrintf("\n");
    pmPrintf(" START      Pause / Resume\n");
    pmPrintf(" X          Next Scene\n");
    pmPrintf(" CIRCLE     Max Speed\n");
    pmPrintf(" TRIANGLE   Frame Advance\n");
    pmPrintf(" SELECT     Quit\n");
    pmPrintf("\n");
    pmPrintf(" (START to go back)\n");
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
    pmPrintf(" X to confirm, START back\n");

    (void)fieldNames;
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
extern int hostForcedHoliday;
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
    if (hostForcedHoliday < 0) return "AUTO";
    if (hostForcedHoliday == 0) return "NONE";
    return holidayShortName(hostForcedHoliday);
}

static void cycleDaynight(void)
{
    /* AUTO(-1) -> DAY(0) -> NIGHT(1) -> AUTO */
    hostForcedNight = hostForcedNight + 1;
    if (hostForcedNight > 1) hostForcedNight = -1;
}

static void cycleHoliday(void)
{
    /* AUTO(-1) -> NONE(0) -> holidays in YAML/calendar order -> AUTO */
    hostForcedHoliday = holidayNextId(hostForcedHoliday);
}

static void drawMainMenu(void)
{
    const char *soundLabel = soundMuted ? "MUTED" : "ON";

    pmPrintf(" JOHNNY CASTAWAY  - PAUSED -\n");
    drawSeparator();

    pmPrintf(" %s Resume\n",
             menuCursor == MENU_RESUME ? ">" : " ");
    pmPrintf(" %s Sound: %s\n",
             menuCursor == MENU_SOUND ? ">" : " ", soundLabel);
    pmPrintf(" %s Day/Night: %s\n",
             menuCursor == MENU_DAYNIGHT ? ">" : " ", daynightLabel());
    pmPrintf(" %s Holiday: %s\n",
             menuCursor == MENU_HOLIDAY ? ">" : " ", holidayLabel());
    if (menuCursor == MENU_HOLIDAY) {
        if (hostForcedHoliday > 0 && holidayById(hostForcedHoliday)) {
            pmPrintf("   %s\n", holidayDateLabel(hostForcedHoliday));
            pmPrintf("   %.28s\n", holidayTitle(hostForcedHoliday));
        } else if (hostForcedHoliday < 0 && ps1SoftTimeEnabled) {
            int dateHoliday = holidayForDate(ps1SoftYear, ps1SoftMonth, ps1SoftDay);
            pmPrintf("   Date picker: %s\n",
                     dateHoliday ? holidayShortName(dateHoliday) : "NONE");
        } else if (hostForcedHoliday < 0) {
            pmPrintf("   Random each scene\n");
        }
    }
    pmPrintf(" %s Save Settings to Memcard\n",
             menuCursor == MENU_SAVE ? ">" : " ");
    pmPrintf(" %s Reset Screensaver Loop\n",
             menuCursor == MENU_RESET_LOOP ? ">" : " ");
    pmPrintf(" %s Next Scene\n",
             menuCursor == MENU_NEXT_SCENE ? ">" : " ");
    pmPrintf(" %s TTY Perf Log: %s\n",
             menuCursor == MENU_PERF_TOGGLE ? ">" : " ", perfLevelLabel());
    pmPrintf(" %s Debug Info\n",
             menuCursor == MENU_DEBUG_INFO ? ">" : " ");
    pmPrintf(" %s Set Time/Date\n",
             menuCursor == MENU_SET_TIME ? ">" : " ");

    drawSeparator();
    pmPrintf("  X = select   START = resume\n");
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

    /* X = select current item */
    if (pressed & PAD_CROSS) {
        switch (menuCursor) {
        case MENU_RESUME:
            return 0;  /* close menu */

        case MENU_SOUND:
            /* Toggle real SPU mute (silences active voices + blocks new). */
            soundMuteToggle();
            /* User chose mute state — don't auto-undo on hide. */
            pauseMutedSound = 0;
            break;

        case MENU_DAYNIGHT:
            cycleDaynight();
            break;

        case MENU_HOLIDAY:
            cycleHoliday();
            break;

        case MENU_SAVE:
            memcardSaveSettings();
            break;

        case MENU_RESET_LOOP:
            pauseMenuRequestResetLoop = 1;
            return 0;  /* close menu so the foreground pilot loop sees it */

        case MENU_NEXT_SCENE:
            pauseMenuRequestNextScene = 1;
            return 0;

        case MENU_PERF_TOGGLE:
            /* Cycle: OFF → SUMMARY → DETAIL → DEBUG → OFF. */
            ps1PerfSetLevel((ps1PerfLevel + 1) & 0x3);
            break;

        case MENU_DEBUG_INFO:
            menuState = PAUSE_MENU_SCENE_INFO;
            prevButtons = 0xFFFF;
            break;

        case MENU_SET_TIME:
            /* Sync edit fields from current soft-time state so the
             * sub-screen reflects what was loaded from memcard (or
             * what's been previously set). */
            editMonth = ps1SoftMonth;
            editDay   = ps1SoftDay;
            editYear  = ps1SoftYear;
            editHour  = ps1SoftHour;
            editMinute = ps1SoftMinute;
            menuState = PAUSE_MENU_SET_TIME;
            editField = 0;
            prevButtons = 0xFFFF;
            break;

        default:
            break;
        }
    }

    /* START on main menu = resume */
    if (pressed & PAD_START)
        return 0;

    return 1;  /* keep menu open */
}

/* Returns 0 if user goes back to main. */
static int handleSubInput(uint16 pressed)
{
    if (pressed & PAD_START) {
        menuState = PAUSE_MENU_MAIN;
        menuCursor = 0;
        prevButtons = 0xFFFF;
        return 1;
    }
    return 1;
}

/* Set Time input -- LEFT/RIGHT adjust value, UP/DOWN change field. */
static int handleSetTimeInput(uint16 pressed)
{
    if (pressed & PAD_START) {
        menuState = PAUSE_MENU_MAIN;
        menuCursor = 0;
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
        hostForcedHoliday = -1;  /* date picker drives holiday while in AUTO */
        /* getDayOfYear() in utils.c computes from ps1SoftMonth/ps1SoftDay */

        /* Return to main menu after confirming. */
        menuState = PAUSE_MENU_MAIN;
        menuCursor = 0;
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
    if (!menuVisible) return 0;

    /* Re-upload scene bg every pause frame so the dim quad doesn't
     * compound (each frame's semi-trans 50% would otherwise re-halve
     * VRAM). */
    grForceFullRedrawNextFrame();
    grDrawBackground();
    DrawSync(0);

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
    addPrim(&pauseOt[PAUSE_OT_LEN - 1], tp);

    pmBuildPanelQuads(&next,
                      &pauseOt[PAUSE_OT_LEN - 2],
                      &pauseOt[PAUSE_OT_LEN - 3]);

    /* Globals consumed by pmPrintf for text. */
    pmFramePrimNext = next;
    pmFrameOtSlot   = &pauseOt[PAUSE_OT_LEN - 4];
    pmPrintfX       = PM_PANEL_X0 + 24;
    pmTextStart(pmPrintfX, PM_PANEL_Y0 + 24);

    /* Read pad through the game's shared pad buffer (events_ps1.c owns
     * InitPAD, so we just peek at its buffer via the extern). */
    extern uint8 pad_buff[2][34];
    uint16 cur = 0;
    {
        PADTYPE *pad = (PADTYPE *)pad_buff[0];
        /* Don't gate on pad->stat — DualShock analog reports non-zero
         * stat values in some modes and pad->btn is still valid. */
        cur = ~(pad->btn);
    }
    uint16 pressed = pmNewPress(cur);
    prevButtons = cur;

    /* Dispatch to current sub-screen input handler. */
    int keepOpen = 1;
    switch (menuState) {
    case PAUSE_MENU_MAIN:
        keepOpen = handleMainInput(pressed);
        break;
    case PAUSE_MENU_SET_TIME:
        keepOpen = handleSetTimeInput(pressed);
        break;
    case PAUSE_MENU_SCENE_INFO:
    case PAUSE_MENU_CONTROLS:
        keepOpen = handleSubInput(pressed);
        break;
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

    /* Draw the appropriate screen — pmPrintf adds SPRT primitives to
     * pauseOt via the pmFrame* globals. */
    switch (menuState) {
    case PAUSE_MENU_MAIN:       drawMainMenu();  break;
    case PAUSE_MENU_SCENE_INFO: drawSceneInfo(); break;
    case PAUSE_MENU_CONTROLS:   drawControls();  break;
    case PAUSE_MENU_SET_TIME:   drawSetTime();   break;
    }

    /* Submit the OT to GPU. */
    DrawOTag(&pauseOt[PAUSE_OT_LEN - 1]);

    /* JCPAUSE per-frame diag. */
    {
        static uint32 pauseFrameDbg = 0;
        if ((pauseFrameDbg++ % 60) == 0) {
            printf("JCPAUSE flush #%lu fontID=%d state=%d cursor=%d\n",
                   (unsigned long)pauseFrameDbg, fontID, (int)menuState, menuCursor);
        }
    }

    /* VSync to pace at 60 Hz while paused. */
    VSync(0);

    return 1;
}
