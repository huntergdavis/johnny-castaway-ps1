/*
 *  This file is part of 'Johnny Reborn' - PS1 Port
 *
 *  Visual debugging system for PS1
 *  Displays text on-screen since printf() doesn't work in DuckStation
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <psxapi.h>
#include <psxpad.h>

#include "ps1_debug.h"

/* Font stream ID - not static so jc_reborn.c can access it */
int fontID = -1;

/* Screen dimensions - 640x480 interlaced high res mode */
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

/* Text buffer for accumulating debug messages */
#define MAX_DEBUG_LINES 20
#define MAX_LINE_LENGTH 80
static char debugLines[MAX_DEBUG_LINES][MAX_LINE_LENGTH];
static int numDebugLines = 0;

/* Background color for debug screen - CHANGE THIS EACH BUILD! */
/* Build 24: Light green */
static int bgR = 220, bgG = 255, bgB = 220;  /* Light green */

/*
 * Initialize visual debugging system
 */
void ps1DebugInit(void)
{
    /* Reset GPU and set video mode (do this ONCE at startup) */
    /* Use 640x480 interlaced high res mode */
    ResetGraph(0);
    SetVideoMode(MODE_NTSC);

    /* Load built-in PSX BIOS font (8x8 characters) */
    FntLoad(960, 0);

    /* Create font stream at top-left of screen */
    /* FntOpen(x, y, width, height, clear_background, max_chars) */
    fontID = FntOpen(10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20, 0, 512);

    /* Enable display */
    SetDispMask(1);

    /* Clear debug buffer */
    ps1DebugClear();
}

/*
 * Clear the debug text buffer
 */
void ps1DebugClear(void)
{
    numDebugLines = 0;
    for (int i = 0; i < MAX_DEBUG_LINES; i++) {
        debugLines[i][0] = '\0';
    }
}

/*
 * Print a line of text to the debug display
 */
void ps1DebugPrint(const char *fmt, ...)
{
    if (numDebugLines >= MAX_DEBUG_LINES) {
        return;  /* Buffer full */
    }

    va_list args;
    va_start(args, fmt);
    vsnprintf(debugLines[numDebugLines], MAX_LINE_LENGTH, fmt, args);
    va_end(args);

    numDebugLines++;
}

/*
 * Update the screen with current debug text
 */
void ps1DebugFlush(void)
{
    /* Set up draw environment with background color */
    DRAWENV draw;
    SetDefDrawEnv(&draw, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    setRGB0(&draw, bgR, bgG, bgB);
    draw.isbg = 1;  /* Enable background clear */
    PutDrawEnv(&draw);

    /* Print all accumulated debug lines */
    for (int i = 0; i < numDebugLines; i++) {
        FntPrint(fontID, "%s\n", debugLines[i]);
    }

    /* Flush font buffer to VRAM */
    FntFlush(fontID);

    /* DON'T use DrawSync or VSync - they can hang after CdInit()! */
    /* The GPU will render when it's ready */
}

/*
 * Wait for user input (SELECT button) before continuing
 */
void ps1DebugWait(void)
{
    /* Show "Press SELECT to continue" message */
    FntPrint(fontID, "\nPress SELECT to continue...\n");
    FntFlush(fontID);

    /* Initialize pad */
    uint8_t pad_buff[2][34];
    InitPAD(pad_buff[0], 34, pad_buff[1], 34);
    StartPAD();
    ChangeClearPAD(0);

    /* Wait for SELECT button press */
    int waiting = 1;
    while (waiting) {
        PADTYPE *pad = (PADTYPE*)pad_buff[0];

        if (pad->stat == 0) {
            /* Pad connected */
            if ((pad->type == 0x4) || (pad->type == 0x5) || (pad->type == 0x7)) {
                /* Digital pad, DualShock, or Analog */
                if (!(pad->btn & PAD_SELECT)) {
                    /* SELECT button pressed */
                    waiting = 0;
                }
            }
        }

        VSync(0);  /* Wait for next frame */
    }

    /* Wait for button release */
    while (1) {
        PADTYPE *pad = (PADTYPE*)pad_buff[0];

        if (pad->stat == 0) {
            if ((pad->type == 0x4) || (pad->type == 0x5) || (pad->type == 0x7)) {
                if (pad->btn & PAD_SELECT) {
                    /* Button released */
                    break;
                }
            }
        }

        VSync(0);
    }

    StopPAD();
}

/*
 * Show an error screen with text and wait
 */
void ps1DebugError(const char *fmt, ...)
{
    /* Re-initialize GPU and font - the game overwrites font VRAM area */
    ResetGraph(0);
    SetVideoMode(MODE_NTSC);
    FntLoad(960, 0);
    fontID = FntOpen(10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20, 0, 512);
    SetDispMask(1);

    /* Set error background color (dark red) */
    bgR = 64;
    bgG = 0;
    bgB = 0;

    /* Clear and add error header */
    ps1DebugClear();
    ps1DebugPrint("=== ERROR ===");
    ps1DebugPrint("");

    /* Add error message */
    char errorMsg[MAX_LINE_LENGTH];
    va_list args;
    va_start(args, fmt);
    vsnprintf(errorMsg, MAX_LINE_LENGTH, fmt, args);
    va_end(args);

    ps1DebugPrint("%s", errorMsg);

    /* Flush and wait */
    ps1DebugFlush();
    ps1DebugWait();

    /* Reset background color */
    bgR = 0;
    bgG = 0;
    bgB = 64;
}


/*
 * Windows-3.1-style fatal-error screen. Solid blue background with white
 * BIOS-font diagnostic info. Halts forever — caller must never return.
 *
 * Tries hard to surface enough state to diagnose:
 *   - the scene slug that was loading
 *   - a short human reason
 *   - file:line of the call site
 *   - heap state (largest contiguous malloc available right now)
 *
 * The intent is "should never fire in production but if it does, give
 * the developer everything they'd ask for next". On a deterministic
 * test build any BSOD = code/data bug — fail loud and stop.
 */
#include "mem_region_extern.h"
/* MemRegion enum values — match mem_region.h: BOOT=0, CACHE=1, TRANSIENT=2 */
#define BSOD_REGION_BOOT      0u
#define BSOD_REGION_CACHE     1u
#define BSOD_REGION_TRANSIENT 2u

__attribute__((noreturn))
void ps1Bsod(const char *scene, const char *reason,
             const char *file, int line)
{
    /* Log to TTY first. ps1Bsod halts forever, so this is the last
     * chance for an external observer (DuckStation TTY console,
     * regtest harness, perf-trace agent) to capture the failure.
     *
     * Log-tag scheme — designed to be grep-friendly for test scripts
     * that want to detect the fatal end state and bail out of a long
     * run:
     *   JCBSOD-FATAL  — single-line sentinel for "trip the alarm".
     *                   Match this once and you know a BSOD fired.
     *   JCBSOD <k>=<v> — detail lines, one key/value per line, for
     *                   collecting heap state, scene name, etc.
     *   JCBSOD-HALT   — single-line sentinel for "log block complete".
     *                   Match this to know all detail has flushed
     *                   before tearing down the test process. */
    {
        extern int printf(const char *, ...);
        extern int           walkPilotCleanBufferAllocated(void);
        extern unsigned long walkPilotCleanBufferBytes(void);
        extern int           walkPilotJohnwalkSlotLoaded(void);

        /* Mem-region state (replaces the old fgProbeLargestAlloc /
         * fgGetFrameBufferBytes / fgGetPrefetchFrameBufferBytes
         * heap-probe lines — those queried libc malloc, which is
         * unreachable under the new allocator). The new state covers
         * the same diagnostic ground via region used/peak counters
         * + the TRANSIENT outstanding-allocation balance.
         *
         * memSafeRead clamps the value to [0, region budget] so a
         * corrupted metadata word can't crash the halt screen itself
         * (PR9). See plan v9 "Failure UX". */
        unsigned long memBootUsed       = (unsigned long)memSafeRead(BSOD_REGION_BOOT);
        unsigned long memBootPeak       = (unsigned long)memRegionPeak(BSOD_REGION_BOOT);
        unsigned long memCacheUsed      = (unsigned long)memSafeRead(BSOD_REGION_CACHE);
        unsigned long memCachePeak      = (unsigned long)memRegionPeak(BSOD_REGION_CACHE);
        unsigned long memTransientUsed  = (unsigned long)memSafeRead(BSOD_REGION_TRANSIENT);
        unsigned long memTransientPeak  = (unsigned long)memRegionPeak(BSOD_REGION_TRANSIENT);
        int           sceneAllocBalance = sceneAllocBalanceGet();
        unsigned long walkCleanKB       = walkPilotCleanBufferBytes() / 1024UL;
        int           walkCleanOK       = walkPilotCleanBufferAllocated();
        int           johnwalkOK        = walkPilotJohnwalkSlotLoaded();

        /* Strip the build-tree directory prefix from __FILE__ for the
         * log line — same treatment the on-screen panel does. */
        const char *fileBase = file ? file : "(nofile)";
        if (file) {
            for (const char *p = file; *p; p++) {
                if (*p == '/' || *p == '\\') fileBase = p + 1;
            }
        }

        printf("\n");
        printf("JCBSOD-FATAL %s\n", reason ? reason : "(unspecified)");
        printf("JCBSOD scene=%s\n",
               (scene && scene[0]) ? scene : "(unknown)");
        printf("JCBSOD where=%s:%d\n", fileBase, line);
        printf("JCBSOD memBootUsed=%lu memBootPeak=%lu\n",
               memBootUsed, memBootPeak);
        printf("JCBSOD memCacheUsed=%lu memCachePeak=%lu\n",
               memCacheUsed, memCachePeak);
        printf("JCBSOD memTransientUsed=%lu memTransientPeak=%lu\n",
               memTransientUsed, memTransientPeak);
        printf("JCBSOD sceneAllocBalance=%d\n", sceneAllocBalance);
        printf("JCBSOD walkCleanAlloc=%d walkCleanKB=%lu\n",
               walkCleanOK, walkCleanKB);
        printf("JCBSOD johnwalkSlotLoaded=%d\n", johnwalkOK);
        printf("JCBSOD note=Reset console to recover.\n");
        printf("JCBSOD-HALT\n");
    }

    /* Reinit GPU. The game has been mutating display + VRAM and may have
     * been in 640x480 interlaced mode, but the BIOS font helper reads
     * cleanest at standard 320x240 NTSC — and at that resolution PSn00bSDK
     * shows ~40 columns of the 8x8 BIOS font, which lines up with our
     * panel design. Set an explicit display + draw env so we don't
     * inherit anything weird from whatever crashed. */
    #define BSOD_W 320
    #define BSOD_H 240
    ResetGraph(0);
    SetVideoMode(MODE_NTSC);

    DISPENV disp;
    DRAWENV draw;
    SetDefDispEnv(&disp, 0, 0, BSOD_W, BSOD_H);
    disp.isinter = 0;
    SetDefDrawEnv(&draw, 0, 0, BSOD_W, BSOD_H);
    setRGB0(&draw, 0, 0, 168);   /* Win 3.1 BSOD blue */
    draw.isbg = 1;               /* solid-fill the framebuffer */
    PutDispEnv(&disp);
    PutDrawEnv(&draw);
    SetDispMask(1);

    /* Open a font stream that fits inside the 320x240 viewport with a
     * comfortable margin. 304x224 inset by 8 px — gives ~38 columns,
     * 28 rows. */
    FntLoad(960, 0);
    fontID = FntOpen(8, 8, BSOD_W - 16, BSOD_H - 16, 0, 1024);

    if (scene == NULL || scene[0] == '\0') scene = "(unknown)";
    if (reason == NULL || reason[0] == '\0') reason = "(unspecified)";
    if (file == NULL) file = "(no file)";

    /* Read mem-region state for the on-screen panel. Under the new
     * allocator there's no fragmented heap to probe — the regions
     * are deterministic, so we report TRANSIENT used as the closest
     * analogue (it's the dynamic region per scene). memSafeRead
     * clamps so a metadata bug can't crash the panel itself (PR9). */
    unsigned long memTrUsed   = (unsigned long)memSafeRead(BSOD_REGION_TRANSIENT);
    unsigned long memTrPeak   = (unsigned long)memRegionPeak(BSOD_REGION_TRANSIENT);
    unsigned long heapKB      = memTrUsed / 1024UL;
    (void)memTrPeak;  /* available if panel layout adds it */

    /* Strip directory prefix from __FILE__ — keeps the on-screen line
     * tight (the build path is usually long enough to wrap on its own). */
    const char *fileBase = file;
    for (const char *p = file; *p; p++) {
        if (*p == '/' || *p == '\\') fileBase = p + 1;
    }

    /* Word-wrap the reason into the BSOD panel's 38-col layout. The
     * 8x8 BIOS font + 304 px text width = 38 columns; reasons like
     * "CACHE exhausted (region+libc both): req=65280 have=107276" are
     * 57 chars and previously got clipped at the right margin. Wrap
     * at the last space before column WRAP_COL, hard-break if a single
     * word exceeds the column width. Indent continuation lines so the
     * structure stays readable. */
    {
        char reasonBuf[256];
        size_t outLen = 0;
        const size_t indentChars = 3;     /* "   " prefix on each line */
        const size_t bufMax = sizeof(reasonBuf) - 1;
        const int WRAP_COL = 36;          /* leave a 1-col safety margin */
        const char *r = reason;
        while (*r && outLen < bufMax) {
            /* Emit indent for this line. */
            for (size_t i = 0; i < indentChars && outLen < bufMax; i++)
                reasonBuf[outLen++] = ' ';
            /* Find the end of the chunk that fits. */
            size_t chunkChars = 0;
            size_t lastSpace = 0;       /* offset in r where last space was */
            size_t lastSpaceChunk = 0;  /* chunkChars at that point */
            size_t cap = (size_t)WRAP_COL - indentChars;
            while (r[chunkChars] && chunkChars < cap) {
                if (r[chunkChars] == ' ') {
                    lastSpace = chunkChars;
                    lastSpaceChunk = chunkChars;
                }
                chunkChars++;
            }
            /* If we hit the cap mid-word and saw a space, break there. */
            size_t copyLen = chunkChars;
            if (r[chunkChars] && r[chunkChars] != ' ' && lastSpace > 0) {
                copyLen = lastSpaceChunk;
            }
            /* Copy the chunk. */
            for (size_t i = 0; i < copyLen && outLen < bufMax; i++)
                reasonBuf[outLen++] = r[i];
            if (outLen < bufMax)
                reasonBuf[outLen++] = '\n';
            r += copyLen;
            /* Skip a single leading space on the next line so the
             * indented continuation lines don't double-space. */
            if (*r == ' ') r++;
        }
        reasonBuf[outLen] = '\0';

        FntPrint(fontID,
            " JOHNNY CASTAWAY  SYSTEM ERROR\n"
            " ==============================\n"
            "\n"
            " A fatal error has occurred.\n"
            " The screensaver has halted to\n"
            " prevent further problems.\n"
            "\n"
            " Reason:\n"
            "%s"
            "\n"
            " Scene:  %s\n"
            " At:     %s:%d\n"
            "\n"
            " Diagnostics:\n"
            "   Largest free block: %lu KB\n"
            "\n"
            " Reset the console to recover.\n"
            " (Photograph this screen for\n"
            "  the bug report.)\n",
            reasonBuf, scene, fileBase, line, heapKB);
    }

    FntFlush(fontID);
    DrawSync(0);
    VSync(0);

    /* Park forever. The user must reset to recover. */
    for (;;) {
        VSync(0);
    }
}
