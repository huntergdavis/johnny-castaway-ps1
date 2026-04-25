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
#include <string.h>

#include "mytypes.h"
#include "pause_menu.h"
#include "graphics_ps1.h"
#include "sound_ps1.h"
#include "resource.h"
#include "foreground_pilot.h"
#include "ps1_perf.h"

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

/* OT + primitive scratch for the dim/panel POLY_F4 quads. Tiny —
 * just a tpage + 2 quads = 60 bytes, OT length 4 is plenty. */
#define PAUSE_OT_LEN 4
static uint32 pauseOt[PAUSE_OT_LEN];
static uint8  pausePrimBuf[128];

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
    MENU_RESET_LOOP,
    MENU_NEXT_SCENE,
    MENU_PERF_TOGGLE,
    MENU_DEBUG_INFO,
    MENU_SET_TIME,
    MENU_CONTROLS,
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

    /* We keep the fontID from ps1_debug.c if still valid; otherwise open a
     * new stream. FntOpen returns a small integer stream id. */
    if (fontID < 0) {
        fontID = FntOpen(80, 100, 480, 280, 0, 1024);
    }

    menuVisible = 0;
    menuCursor  = 0;
    menuState   = PAUSE_MENU_MAIN;

    /* We piggyback on the events_ps1 pad buffers (extern pad_buff). */
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

    /* P6: mute SPU on pause-show. Track that WE caused the mute so
     * we can undo it on hide (unless the user manually toggled it). */
    if (!soundMuted) {
        soundMuteToggle();
        pauseMutedSound = 1;
    } else {
        pauseMutedSound = 0;
    }

    /* P6: one-shot JCPAUSE snapshot for log-mining. */
    printf("JCPAUSE show frame=%lu scene=%s mode=%s perfLevel=%u soundMuted=%d\n",
           (unsigned long)ps1FrameCount,
           foregroundPilotRuntimeSceneName(),
           foregroundPilotRuntimeModeName(),
           (unsigned)ps1PerfLevel,
           soundMuted);
}

void pauseMenuHide(void)
{
    menuVisible = 0;

    /* P6: undo our auto-mute. If user toggled sound while paused,
     * pauseMutedSound was cleared and we leave their choice alone. */
    if (pauseMutedSound) {
        soundMuteToggle();
        pauseMutedSound = 0;
    }
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

/* P2: build POLY_F4 dim+panel quads each pause frame.
 *
 * Order in OT (back→front):
 *   priority N   : dim quad — full screen, semi-trans 50% black
 *   priority N-1 : panel quad — centered, opaque dark blue
 * Text from FntFlush draws on top via its own primitive list.
 *
 * The bgTile RAM is never modified — on resume we just stop drawing
 * the quads and the next scene frame's grDrawBackground re-uploads
 * the bg as if pause never happened (well, we still need
 * grForceFullRedrawNextFrame on exit because our quads modified
 * VRAM directly during pause). */
static void drawPauseQuads(void)
{
    ClearOTagR(pauseOt, PAUSE_OT_LEN);
    uint8 *next = pausePrimBuf;

    /* TPAGE: set abr=0 (50% blend) for any subsequent semi-trans prim. */
    DR_TPAGE *tp = (DR_TPAGE*)next;
    next += sizeof(DR_TPAGE);
    setDrawTPage(tp, 0, 1, getTPage(0, 0, 0, 0));
    addPrim(&pauseOt[PAUSE_OT_LEN - 1], tp);

    /* Dim quad — full 640x480, semi-trans black → halves what's behind. */
    POLY_F4 *dim = (POLY_F4*)next;
    next += sizeof(POLY_F4);
    setPolyF4(dim);
    setSemiTrans(dim, 1);
    setRGB0(dim, 0, 0, 0);
    setXY4(dim,   0,   0,
                640,   0,
                  0, 480,
                640, 480);
    addPrim(&pauseOt[PAUSE_OT_LEN - 2], dim);

    /* Panel quad — centered at (60,80)–(580,400), opaque dark blue. */
    POLY_F4 *panel = (POLY_F4*)next;
    next += sizeof(POLY_F4);
    setPolyF4(panel);
    setRGB0(panel, 0x10, 0x18, 0x40);
    setXY4(panel,  60,  80,
                  580,  80,
                   60, 400,
                  580, 400);
    addPrim(&pauseOt[PAUSE_OT_LEN - 3], panel);

    DrawSync(0);
    DrawOTag(&pauseOt[PAUSE_OT_LEN - 1]);
    DrawSync(0);
}

static void drawSeparator(void)
{
    FntPrint(fontID, "----------------------------\n");
}

/* ---------------------------------------------------------------------------
 *  Sub-screen: Debug Info
 * ------------------------------------------------------------------------- */
static void drawSceneInfo(void)
{
    size_t used   = getTotalMemoryUsed();
    size_t budget = getMemoryBudget();
    const char *scene = foregroundPilotRuntimeSceneName();
    const char *mode  = foregroundPilotRuntimeModeName();
    uint16 fIdx = foregroundPilotRuntimeFrameIndex();
    uint16 fCnt = foregroundPilotRuntimeFrameCount();
    uint32 uptimeSec = ps1FrameCount / 60;

    FntPrint(fontID, "       DEBUG INFO\n");
    drawSeparator();
    FntPrint(fontID, " Scene:  %s\n", scene ? scene : "?");
    FntPrint(fontID, " Mode:   %s\n", mode  ? mode  : "?");
    FntPrint(fontID, " Frame:  %u / %u\n",
             (unsigned)fIdx, (unsigned)fCnt);
    FntPrint(fontID, " Mem:    %u KB / %u KB\n",
             (unsigned)(used / 1024), (unsigned)(budget / 1024));
    FntPrint(fontID, " Uptime: %lu:%02lu\n",
             (unsigned long)(uptimeSec / 60), (unsigned long)(uptimeSec % 60));
    FntPrint(fontID, " Build:  %s\n", __DATE__);
    FntPrint(fontID, " Perf:   %s\n", perfLevelLabel());
    FntPrint(fontID, " Sound:  %s\n", soundMuted ? "MUTED" : "ON");
    drawSeparator();
    FntPrint(fontID, "  START = back\n");
}

/* ---------------------------------------------------------------------------
 *  Sub-screen: Controls
 * ------------------------------------------------------------------------- */
static void drawControls(void)
{
    FntPrint(fontID, "\n");
    drawSeparator();
    FntPrint(fontID, "     CONTROLS\n");
    drawSeparator();
    FntPrint(fontID, "\n");
    FntPrint(fontID, " START      Pause / Resume\n");
    FntPrint(fontID, " X          Next Scene\n");
    FntPrint(fontID, " CIRCLE     Max Speed\n");
    FntPrint(fontID, " TRIANGLE   Frame Advance\n");
    FntPrint(fontID, " SELECT     Quit\n");
    FntPrint(fontID, "\n");
    FntPrint(fontID, " (START to go back)\n");
}

/* ---------------------------------------------------------------------------
 *  Sub-screen: Set Time/Date
 * ------------------------------------------------------------------------- */
static void drawSetTime(void)
{
    const char *fieldNames[] = {"Month","Day","Year","Hour","Min"};

    FntPrint(fontID, "\n");
    drawSeparator();
    FntPrint(fontID, "   SET TIME AND DATE\n");
    drawSeparator();
    FntPrint(fontID, "\n");

    /* Month */
    FntPrint(fontID, " %s Month: %s%02d%s\n",
             editField == 0 ? ">" : " ",
             editField == 0 ? "[" : " ",
             editMonth,
             editField == 0 ? "]" : " ");

    /* Day */
    FntPrint(fontID, " %s Day:   %s%02d%s\n",
             editField == 1 ? ">" : " ",
             editField == 1 ? "[" : " ",
             editDay,
             editField == 1 ? "]" : " ");

    /* Year */
    FntPrint(fontID, " %s Year:  %s%04d%s\n",
             editField == 2 ? ">" : " ",
             editField == 2 ? "[" : " ",
             editYear,
             editField == 2 ? "]" : " ");

    /* Hour */
    FntPrint(fontID, " %s Hour:  %s%02d%s\n",
             editField == 3 ? ">" : " ",
             editField == 3 ? "[" : " ",
             editHour,
             editField == 3 ? "]" : " ");

    /* Minute */
    FntPrint(fontID, " %s Min:   %s%02d%s\n",
             editField == 4 ? ">" : " ",
             editField == 4 ? "[" : " ",
             editMinute,
             editField == 4 ? "]" : " ");

    FntPrint(fontID, "\n");
    FntPrint(fontID, " UP/DOWN select field\n");
    FntPrint(fontID, " LEFT/RIGHT adjust value\n");
    FntPrint(fontID, " X to confirm, START back\n");

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

static void drawMainMenu(void)
{
    const char *soundLabel = soundMuted ? "MUTED" : "ON";

    FntPrint(fontID, " JOHNNY CASTAWAY  - PAUSED -\n");
    drawSeparator();

    FntPrint(fontID, " %s Resume\n",
             menuCursor == MENU_RESUME ? ">" : " ");
    FntPrint(fontID, " %s Sound: %s\n",
             menuCursor == MENU_SOUND ? ">" : " ", soundLabel);
    FntPrint(fontID, " %s Reset Screensaver Loop\n",
             menuCursor == MENU_RESET_LOOP ? ">" : " ");
    FntPrint(fontID, " %s Next Scene\n",
             menuCursor == MENU_NEXT_SCENE ? ">" : " ");
    FntPrint(fontID, " %s Perf Counters: %s\n",
             menuCursor == MENU_PERF_TOGGLE ? ">" : " ", perfLevelLabel());
    FntPrint(fontID, " %s Debug Info\n",
             menuCursor == MENU_DEBUG_INFO ? ">" : " ");
    FntPrint(fontID, " %s Set Time/Date\n",
             menuCursor == MENU_SET_TIME ? ">" : " ");
    FntPrint(fontID, " %s Controls\n",
             menuCursor == MENU_CONTROLS ? ">" : " ");

    drawSeparator();
    FntPrint(fontID, "  X = select   START = resume\n");
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
            menuState = PAUSE_MENU_SET_TIME;
            editField = 0;
            prevButtons = 0xFFFF;
            break;

        case MENU_CONTROLS:
            menuState = PAUSE_MENU_CONTROLS;
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
     * utils.c, which return hard-coded values on PS1.  We update those stubs
     * through extern globals exposed specifically for the pause menu. */
    if (pressed & PAD_CROSS) {
        /* Export to the global overrides (defined in utils.c PS1 section). */
        extern int ps1SoftHour;
        extern int ps1SoftMonth;
        extern int ps1SoftDay;

        ps1SoftHour  = editHour;
        ps1SoftMonth = editMonth;
        ps1SoftDay   = editDay;
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

    /* P2: re-upload bg every pause frame so the dim quad doesn't
     * compound over time (each pass would otherwise re-halve VRAM). */
    grForceFullRedrawNextFrame();
    grDrawBackground();
    DrawSync(0);

    /* P2: draw the translucent dim + opaque panel quads on top of bg. */
    drawPauseQuads();

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

    /* Draw the appropriate screen. */
    switch (menuState) {
    case PAUSE_MENU_MAIN:       drawMainMenu();  break;
    case PAUSE_MENU_SCENE_INFO: drawSceneInfo(); break;
    case PAUSE_MENU_CONTROLS:   drawControls();  break;
    case PAUSE_MENU_SET_TIME:   drawSetTime();   break;
    }

    /* Flush the font stream to GPU -- renders text primitives this frame. */
    FntFlush(fontID);

    /* VSync to pace at 60 fps while paused. */
    VSync(0);

    return 1;
}
