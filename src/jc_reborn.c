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

/* PS1 Build - needs special header handling */
#ifdef PS1_BUILD
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>  /* Provides exit(), atoi(), malloc(), etc. */
#include <string.h>
#ifndef _FILE_DEFINED
#define _FILE_DEFINED
typedef struct _FILE FILE;
#endif
#define stderr ((FILE*)2)  /* PSn00bSDK doesn't define stderr */
#define fprintf(stream, ...) printf(__VA_ARGS__)  /* Redirect to printf */
/* Declare functions implemented in ps1_stubs.c */
void exit(int status);
int atoi(const char *str);
FILE *fopen(const char *pathname, const char *mode);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
int fclose(FILE *stream);
#else
/* Standard SDL build */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#endif

#include "mytypes.h"
#include "utils.h"
#include "resource.h"

/* Platform-specific headers */
#ifdef PS1_BUILD
#include <psxgpu.h>
#include <psxgte.h>
#include <psxcd.h>
#include <psxapi.h>
#include "graphics_ps1.h"
#include "events_ps1.h"
#include "sound_ps1.h"
#include "cdrom_ps1.h"
#include "ps1_debug.h"
#include "config/ps1/bootmode_embedded.h"
#else
#include "graphics.h"
#include "events.h"
#include "sound.h"
#endif

#include "island.h"
#include "foreground_pilot.h"
#include "ps1_perf.h"

#ifndef PS1_BUILD
#include "dump.h"
#include "ttm.h"
#include "ads.h"
#include "story.h"
#endif

/* Root counters are exposed by PSn00bSDK on PS1 builds. */
#ifdef PS1_BUILD
static void ps1SeedRandom(void)
{
    uint32 seed = 0x9e3779b9u;

    for (int i = 0; i < 32; i++) {
        uint32 t0 = (uint32)GetRCnt(RCntCNT0);
        uint32 t1 = (uint32)GetRCnt(RCntCNT1);
        uint32 t2 = (uint32)GetRCnt(RCntCNT2);
        seed ^= (t0 << (i & 7)) ^ (t1 << ((i + 3) & 7)) ^ (t2 << ((i + 5) & 7));
        seed = (seed << 5) | (seed >> 27);
        seed += 0x7f4a7c15u + (uint32)i;
    }

    if (seed == 0)
        seed = 1;
    srand(seed);
}
#endif


#ifndef PS1_BUILD
static int  argDump     = 0;
static int  argBench    = 0;
static int  argTtm      = 0;
static int  argAds      = 0;
static int  argPlayAll  = 0;
static int  argIsland   = 0;
#endif
static int  argForegroundPilot = 0;

static char *args[3];
static int  numArgs  = 0;

#ifndef PS1_BUILD
static int hostForcedSeed = -1;
static int hostForcedStoryDay = -1;
static int hostBootDirectSceneIndex = -1;
#endif

static int hostForcedIslandPosValid = 0;
static int hostForcedIslandX = 0;
static int hostForcedIslandY = 0;
static int hostForcedLowTide = -1;
static int hostForcedRaftStage = -1;
static int hostForcedNight = -1;     /* -1=unset, 0|1 forces islandState.night */
static int hostForcedHoliday = -1;   /* -1=unset, 0..4 forces islandState.holiday */

/* Screensaver loop: fgpilot mode replays the scene forever with randomized
 * variant params per iteration, unless the `noloop` boot token is set.
 * Variant fields that were explicitly forced via BOOTMODE (night, lowtide,
 * raft-stage, holiday) stay forced across iterations. */
static int screensaverLoopDisabled = 0;

/* Scenes that have reached the "fully validated" bar in
 * docs/ps1/scene-status.md. Expand as scenes are signed off. */
static const char *kProvenScenes[] = { "fishing1", "fishing2" };
#define NUM_PROVEN_SCENES ((int)(sizeof(kProvenScenes) / sizeof(kProvenScenes[0])))

#ifndef PS1_BUILD
static int hostForcedSceneOffsetValid = 0;
static int hostForcedSceneOffsetX = 0;
static int hostForcedSceneOffsetY = 0;
static int hostCapturePreludeFrame = 0;
#endif

/* Pick the scene to play on this screensaver-loop iteration. If the user
 * explicitly named a scene on the fgpilot command line, we replay THAT
 * scene every iteration with random variants; if no scene was named we
 * free-select from the kProvenScenes array (so `fgpilot` alone cycles
 * through every validated scene). */
static const char *fgLoopNextScene(const char *explicitScene)
{
    if (explicitScene && explicitScene[0] != '\0')
        return explicitScene;
    return kProvenScenes[rand() % NUM_PROVEN_SCENES];
}

static int fgLoopSceneUsesVarPos(const char *sceneName)
{
    return sceneName != NULL &&
           (!strcmp(sceneName, "fishing1") ||
            !strcmp(sceneName, "fishing2") ||
            !strcmp(sceneName, "fishing3"));
}

static void fgLoopRandomVarPos(int *outX, int *outY)
{
    if (rand() % 2) {
        *outX = -222 + (rand() % 109);
        *outY = -44  + (rand() % 128);
    } else if (rand() % 2) {
        *outX = -114 + (rand() % 134);
        *outY = -14  + (rand() % 99);
    } else {
        *outX = -114 + (rand() % 119);
        *outY = -73  + (rand() % 60);
    }
}

/* Set islandState variant fields for one iteration. Fields explicitly
 * forced via BOOTMODE (hostForced* >= 0) stay forced; unforced fields
 * get a fresh random value each call. Position policy is scene-specific:
 * current validated fgpilot scenes are VARPOS_OK in story_data.h, so their
 * FG2 scene-relative overlays must follow the original random island offset. */
static void fgLoopApplyVariant(const char *sceneName)
{
    islandState.night   = (hostForcedNight     >= 0) ? hostForcedNight     : (rand() & 1);
    islandState.lowTide = (hostForcedLowTide   >= 0) ? hostForcedLowTide   : (rand() & 1);
    islandState.holiday = (hostForcedHoliday   >= 0) ? hostForcedHoliday   : (rand() % 5);
    islandState.raft    = (hostForcedRaftStage >= 0) ? hostForcedRaftStage : (rand() % 6);

    if (hostForcedIslandPosValid) {
        islandState.xPos = hostForcedIslandX;
        islandState.yPos = hostForcedIslandY;
    } else if (fgLoopSceneUsesVarPos(sceneName)) {
        fgLoopRandomVarPos(&islandState.xPos, &islandState.yPos);
    } else {
        islandState.xPos = 0;
        islandState.yPos = 0;
    }
}

#ifdef PS1_BUILD
#define PS1_BOOT_OVERRIDE_FILE "BOOTMODE.TXT"

static int ps1BootForcedSeed = -1;  /* -1 = use hardware RNG */
static char ps1BootArgStorage[3][32];
static char ps1BootForegroundOverlayScene[32];
static char ps1BootCaptureMetaDirStorage[32];
static char ps1BootCaptureSceneLabelStorage[64];
volatile uint16 ps1BootDbgCaptureMode = 0;
static int ps1BootPrintfTest = 0;

static int ps1IsSpace(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static void ps1ResetBootArgs(void)
{
    argForegroundPilot = 0;
    numArgs = 0;

    for (int i = 0; i < 3; i++) {
        args[i] = NULL;
        ps1BootArgStorage[i][0] = '\0';
    }
    ps1BootForegroundOverlayScene[0] = '\0';
    ps1BootCaptureMetaDirStorage[0] = '\0';
    ps1BootCaptureSceneLabelStorage[0] = '\0';

    grCaptureMetaDir = NULL;
    grCaptureOverlay = 0;
    grCaptureOverlayMaskOnly = 0;
    grCaptureSetSceneLabel("");
    foregroundPilotSetHeapProbe(0);
    ps1PerfSetEnabled(0);
    ps1BootDbgCaptureMode = 0;
    ps1BootForcedSeed = -1;
    ps1BootPrintfTest = 0;
    hostForcedIslandPosValid = 0;
    hostForcedIslandX = 0;
    hostForcedIslandY = 0;
    hostForcedLowTide = -1;
    hostForcedRaftStage = -1;
    hostForcedNight = -1;
    hostForcedHoliday = -1;
}

static int ps1CopyBootArg(int index, const char *src)
{
    size_t len;
    char *copy;

    if (index < 0 || index >= 3 || !src) {
        return 0;
    }

    strncpy(ps1BootArgStorage[index], src, sizeof(ps1BootArgStorage[index]) - 1);
    ps1BootArgStorage[index][sizeof(ps1BootArgStorage[index]) - 1] = '\0';

    len = strlen(ps1BootArgStorage[index]);
    copy = safe_malloc(len + 1);
    memcpy(copy, ps1BootArgStorage[index], len + 1);
    args[index] = copy;
    return 1;
}

static char *ps1CopyBootString(char *dst, size_t dstSize, const char *src)
{
    if (dst == NULL || dstSize == 0 || src == NULL) {
        return NULL;
    }

    strncpy(dst, src, dstSize - 1);
    dst[dstSize - 1] = '\0';
    return dst;
}

static void ps1ApplyBootOverride(char *buffer)
{
    char *tokens[32];
    int tokenCount = 0;
    char *cursor = buffer;
    int tokenBase = 0;

    while (*cursor && tokenCount < (int)(sizeof(tokens) / sizeof(tokens[0]))) {
        while (*cursor && ps1IsSpace(*cursor)) {
            cursor++;
        }

        if (*cursor == '\0' || *cursor == '#') {
            break;
        }

        tokens[tokenCount++] = cursor;

        while (*cursor && !ps1IsSpace(*cursor) && *cursor != '#') {
            cursor++;
        }

        if (*cursor == '#') {
            *cursor = '\0';
            break;
        }

        if (*cursor == '\0') {
            break;
        }

        *cursor = '\0';
        cursor++;
    }

    if (tokenCount == 0) {
        return;
    }

    /* Scan for trailing "seed N" parameter anywhere in the token list */
    for (int i = 0; i + 1 < tokenCount; i++) {
        if (!strcmp(tokens[i], "seed")) {
            ps1BootForcedSeed = atoi(tokens[i + 1]);
            break;
        }
    }

    for (int i = 0; i < tokenCount; i++) {
        if (!strcmp(tokens[i], "capture-overlay-mask")) {
            grCaptureOverlay = 1;
            grCaptureOverlayMaskOnly = 1;
            ps1BootDbgCaptureMode = 2;
        } else if (!strcmp(tokens[i], "capture-overlay")) {
            grCaptureOverlay = 1;
            if (ps1BootDbgCaptureMode == 0)
                ps1BootDbgCaptureMode = 1;
        } else if (!strcmp(tokens[i], "fgoverlay") && (i + 1) < tokenCount) {
            ps1CopyBootString(
                ps1BootForegroundOverlayScene,
                sizeof(ps1BootForegroundOverlayScene),
                tokens[i + 1]
            );
            i++;
        } else if (!strcmp(tokens[i], "capture-meta-dir") && (i + 1) < tokenCount) {
            grCaptureMetaDir = ps1CopyBootString(
                ps1BootCaptureMetaDirStorage,
                sizeof(ps1BootCaptureMetaDirStorage),
                tokens[i + 1]
            );
            i++;
        } else if (!strcmp(tokens[i], "capture-range") && (i + 2) < tokenCount) {
            grCaptureStartFrame = atoi(tokens[i + 1]);
            grCaptureEndFrame = atoi(tokens[i + 2]);
            i += 2;
        } else if (!strcmp(tokens[i], "capture-interval") && (i + 1) < tokenCount) {
            grCaptureInterval = atoi(tokens[i + 1]);
            i++;
        } else if (!strcmp(tokens[i], "capture-scene-label") && (i + 1) < tokenCount) {
            grCaptureSetSceneLabel(ps1CopyBootString(
                ps1BootCaptureSceneLabelStorage,
                sizeof(ps1BootCaptureSceneLabelStorage),
                tokens[i + 1]
            ));
            i++;
        } else if (!strcmp(tokens[i], "island-pos") && (i + 2) < tokenCount) {
            hostForcedIslandX = atoi(tokens[i + 1]);
            hostForcedIslandY = atoi(tokens[i + 2]);
            hostForcedIslandPosValid = 1;
            i += 2;
        } else if (!strcmp(tokens[i], "lowtide") && (i + 1) < tokenCount) {
            hostForcedLowTide = atoi(tokens[i + 1]) ? 1 : 0;
            i++;
        } else if (!strcmp(tokens[i], "raft-stage") && (i + 1) < tokenCount) {
            hostForcedRaftStage = atoi(tokens[i + 1]);
            i++;
        } else if (!strcmp(tokens[i], "night") && (i + 1) < tokenCount) {
            hostForcedNight = atoi(tokens[i + 1]) ? 1 : 0;
            i++;
        } else if (!strcmp(tokens[i], "holiday") && (i + 1) < tokenCount) {
            hostForcedHoliday = atoi(tokens[i + 1]);
            if (hostForcedHoliday < 0) hostForcedHoliday = 0;
            if (hostForcedHoliday > 4) hostForcedHoliday = 4;
            i++;
        } else if (!strcmp(tokens[i], "noloop")) {
            screensaverLoopDisabled = 1;
        } else if (!strcmp(tokens[i], "heap-probe")) {
            foregroundPilotSetHeapProbe(1);
        } else if (!strcmp(tokens[i], "perf-log") || !strcmp(tokens[i], "perf")) {
            ps1PerfSetLevel(PS1_PERF_LEVEL_SUMMARY);
        } else if (!strcmp(tokens[i], "perf-detail")) {
            ps1PerfSetLevel(PS1_PERF_LEVEL_DETAIL);
        } else if (!strcmp(tokens[i], "perf-debug")) {
            ps1PerfSetLevel(PS1_PERF_LEVEL_DEBUG);
        } else if (!strcmp(tokens[i], "printf-test") || !strcmp(tokens[i], "logtest")) {
            ps1BootPrintfTest = 1;
        }
    }

    if (!strcmp(tokens[0], "island")) {
        tokenBase = 1;
    }

    if (tokenBase >= tokenCount) {
        return;
    }

    if (!strcmp(tokens[tokenBase], "fgpilot")) {
        if ((tokenBase + 1) < tokenCount && ps1CopyBootArg(0, tokens[tokenBase + 1]))
            numArgs = 1;
        argForegroundPilot = 1;
        return;
    }
}

static void ps1LoadBootOverride(void)
{
    PS1File *file;
    char buffer[512];
    size_t readCount = 0;
    uint32 rawSize = 0;
    uint8 *rawData;

    ps1ResetBootArgs();

    file = ps1_fopen(PS1_BOOT_OVERRIDE_FILE, "rb");
    if (file != NULL) {
        readCount = ps1_fread(buffer, 1, sizeof(buffer) - 1, file);
        ps1_fclose(file);
        buffer[readCount] = '\0';
        ps1ApplyBootOverride(buffer);
        return;
    }

    rawData = ps1_loadRawFile("\\BOOTMODE.TXT;1", &rawSize);
    if (rawData != NULL) {
        readCount = (rawSize < (sizeof(buffer) - 1)) ? rawSize : (sizeof(buffer) - 1);
        memcpy(buffer, rawData, readCount);
        free(rawData);
        buffer[readCount] = '\0';
        ps1ApplyBootOverride(buffer);
        return;
    }

    if (PS1_EMBEDDED_BOOT_OVERRIDE[0] != '\0') {
        strncpy(buffer, PS1_EMBEDDED_BOOT_OVERRIDE, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
        ps1ApplyBootOverride(buffer);
        return;
    }
}

static void ps1PrintfProbe(const char *phase, const char *sceneName)
{
    if (!ps1BootPrintfTest) {
        return;
    }

    printf(
        "JCLOG phase=%s scene=%s fgpilot=%d seed=%d lowtide=%d night=%d holiday=%d raft=%d pos=%d,%d loop=%d\n",
        phase ? phase : "?",
        sceneName ? sceneName : "?",
        argForegroundPilot,
        ps1BootForcedSeed,
        islandState.lowTide,
        islandState.night,
        islandState.holiday,
        islandState.raft,
        islandState.xPos,
        islandState.yPos,
        screensaverLoopDisabled
    );
}

/* Load and display title screen from raw file on CD */
/* This runs BEFORE resource parsing for instant visual feedback */
static void initTitleDisplayEarly(void)
{
    /* Initialize graphics for 640x480 interlaced */
    ResetGraph(0);
    SetVideoMode(MODE_NTSC);
    grGpuAlreadyInitialized = 1;

    /* Set up display environment for 640x480 */
    DISPENV disp;
    DRAWENV draw;
    SetDefDispEnv(&disp, 0, 0, 640, 480);
    SetDefDrawEnv(&draw, 0, 0, 640, 480);
    disp.isinter = 1;  /* Interlaced mode */
    draw.isbg = 0;     /* Don't clear - we'll load image directly */
    PutDispEnv(&disp);
    PutDrawEnv(&draw);

    /* Enable display */
    SetDispMask(1);
}

static void loadTitleScreenEarly(void)
{
    initTitleDisplayEarly();

    /* Allocate buffer for full title screen (640x480 x 2 bytes = 614400) */
    int totalBytes = 640 * 480 * 2;  /* 614400 bytes */
    uint8 *screenBuffer = (uint8*)malloc(totalBytes);
    if (!screenBuffer) {
        return;  /* Can't show title, continue anyway */
    }

    /* Load TITLE.RAW using direct CD calls */
    CdlFILE fileInfo;
    if (!CdSearchFile(&fileInfo, "\\TITLE.RAW;1")) {
        free(screenBuffer);
        return;  /* File not found, continue anyway */
    }

    /* Calculate sectors needed (2048 bytes per sector) */
    int totalSectors = (totalBytes + 2047) / 2048;

    /* Seek to file location */
    CdControl(CdlSetloc, (uint8*)&fileInfo.pos, 0);

    /* Read data */
    CdRead(totalSectors, (uint32*)screenBuffer, CdlModeSpeed);
    CdReadSync(0, 0);

    /* Upload to framebuffer in strips (GPU DMA works better with smaller chunks) */
    int stripHeight = 60;
    int numStrips = 480 / stripHeight;  /* 8 strips */

    for (int strip = 0; strip < numStrips; strip++) {
        int yOffset = strip * stripHeight;
        uint8 *stripData = screenBuffer + (yOffset * 640 * 2);

        RECT rect;
        setRECT(&rect, 0, yOffset, 640, stripHeight);
        LoadImage(&rect, (uint32*)stripData);
        DrawSync(0);
    }

    free(screenBuffer);

    /* Reset CD state for subsequent resource loading */
    /* This ensures ps1_fopen works correctly after direct CD calls */
    cdromResetState();

}


#endif

#ifndef PS1_BUILD
static void usage()
{
        printf("\n");
        printf(" Usage :\n");
        printf("         jc_reborn\n");
        printf("         jc_reborn help\n");
        printf("         jc_reborn version\n");
        printf("         jc_reborn dump\n");
        printf("         jc_reborn [<options>] bench\n");
        printf("         jc_reborn [<options>] ttm <TTM name>\n");
        printf("         jc_reborn [<options>] ads <ADS name> <ADS tag no>\n");
        printf("         jc_reborn [<options>] fgpilot <scene>\n");
        printf("\n");
        printf(" Available options are:\n");
        printf("         window          - play in windowed mode\n");
        printf("         nosound         - quiet mode\n");
        printf("         island          - display the island as background for ADS play\n");
        printf("         debug           - print some debug info on stdout\n");
        printf("         hotkeys         - enable hot keys\n");
        printf("         capture-frame N - capture frame N to file (for visual testing)\n");
        printf("         capture-output FILE - specify output file for captured frame\n");
        printf("         capture-dir DIR - capture a frame sequence into DIR/frame_XXXXX.bmp\n");
        printf("         capture-meta-dir DIR - emit per-frame sprite metadata JSON into DIR\n");
        printf("         capture-range START END - capture inclusive frame range; END=-1 means until exit\n");
        printf("         capture-interval N - capture every Nth frame in the active range\n");
        printf("         capture-overlay - embed a machine-readable debug overlay in captures\n");
        printf("         capture-overlay-mask - draw overlay background only for paired baseline captures\n");
        printf("         capture-foreground-only - capture composited non-background layers over magenta key\n");
        printf("         noloop          - disable the fgpilot screensaver loop (single-shot play)\n");
        printf("         capture-sound-events FILE - append {frame,sample} JSONL for every PLAY_SAMPLE opcode\n");
        printf("         capture-scene-label TEXT - annotate metadata with the scene label\n");
        printf("         seed N          - force deterministic RNG seed for host runs\n");
        printf("         story-day N     - force story day 1..11 for host story runs\n");
        printf("         island-pos X Y  - force island position for host story/island runs\n");
        printf("         lowtide 0|1     - force low tide state for host story/island runs\n");
        printf("         raft-stage N    - force raft stage 0..5 for host story/island runs\n");
        printf("         scene-offset X Y - force thread-layer scene offset for host story runs\n");
        printf("         capture-prelude-frame - capture one establishing frame before forced non-final story scenes\n");
        printf("\n");
        printf(" While-playing hot-keys (if enabled):\n");
        printf("         Esc        - Terminate immediately\n");
        printf("         Alt+Return - Toggle full screen / windowed mode\n");
        printf("         Space      - Toggle pause / unpause\n");
        printf("         Return     - When paused, advance one frame\n");
        printf("         <M>        - toggle max / normal speed\n");
        printf("\n");
        exit(1);
}


static void version()
{
        printf("\n");
        printf("    Johnny Reborn, an open-source engine for\n");
        printf("    the classic Johnny Castaway screensaver by Sierra.\n");
        printf("    Development version Copyright (C) 2019 Jeremie GUILLAUME\n");
        printf("\n");
        exit(1);
}


static void parseArgs(int argc, char **argv)
{
    int numExpectedArgs = 0;

    for (int i=1; i < argc; i++) {

        if (numExpectedArgs) {
            args[numArgs++] = argv[i];
            numExpectedArgs--;
        }
        else {
            if (!strcmp(argv[i], "help")) {
                usage();
            }
            if (!strcmp(argv[i], "version")) {
                version();
            }
            else if (!strcmp(argv[i], "dump")) {
                argDump = 1;
            }
            else if (!strcmp(argv[i], "bench")) {
                argBench = 1;
            }
            else if (!strcmp(argv[i], "story")) {
                if (i + 2 < argc && !strcmp(argv[i + 1], "single")) {
                    storySetBootSingleSceneIndex(atoi(argv[i + 2]));
                    i += 2;
                }
                else if (i + 2 < argc && !strcmp(argv[i + 1], "direct")) {
                    hostBootDirectSceneIndex = atoi(argv[i + 2]);
                    i += 2;
                }
                else if (i + 2 < argc &&
                         (!strcmp(argv[i + 1], "scene") || !strcmp(argv[i + 1], "index"))) {
                    storySetBootSceneIndex(atoi(argv[i + 2]));
                    i += 2;
                }
                else if (i + 3 < argc && !strcmp(argv[i + 1], "ads")) {
                    storySetBootScene(argv[i + 2], (uint16)atoi(argv[i + 3]));
                    i += 3;
                }
                else {
                    fprintf(stderr, "Error: unsupported story boot form\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "ttm")) {
                argTtm = 1;
                numExpectedArgs = 1;
            }
            else if (!strcmp(argv[i], "ads")) {
                argAds = 1;
                numExpectedArgs = 2;
            }
            else if (!strcmp(argv[i], "fgpilot")) {
                argForegroundPilot = 1;
                argPlayAll = 0;
                numExpectedArgs = 1;
            }
            else if (!strcmp(argv[i], "window")) {
                grWindowed = 1;
            }
            else if (!strcmp(argv[i], "nosound")) {
                soundDisabled = 1;
            }
            else if (!strcmp(argv[i], "island")) {
                argIsland = 1;
            }
            else if (!strcmp(argv[i], "debug")) {
                debugMode = 1;
            }
            else if (!strcmp(argv[i], "hotkeys")) {
                evHotKeysEnabled = 1;
            }
            else if (!strcmp(argv[i], "capture-frame")) {
                if (i + 1 < argc) {
                    grCaptureFrameNumber = atoi(argv[++i]);
                    if (grCaptureFrameNumber < 0) {
                        fprintf(stderr, "Error: capture-frame must be >= 0\n");
                        usage();
                    }
                } else {
                    fprintf(stderr, "Error: capture-frame requires a frame number\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "capture-output")) {
                if (i + 1 < argc) {
                    grCaptureFilename = argv[++i];
                } else {
                    fprintf(stderr, "Error: capture-output requires a filename\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "capture-dir")) {
                if (i + 1 < argc) {
                    grCaptureDir = argv[++i];
                } else {
                    fprintf(stderr, "Error: capture-dir requires a directory\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "capture-meta-dir")) {
                if (i + 1 < argc) {
                    grCaptureMetaDir = argv[++i];
                } else {
                    fprintf(stderr, "Error: capture-meta-dir requires a directory\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "capture-range")) {
                if (i + 2 < argc) {
                    grCaptureStartFrame = atoi(argv[++i]);
                    grCaptureEndFrame = atoi(argv[++i]);
                } else {
                    fprintf(stderr, "Error: capture-range requires START and END\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "capture-interval")) {
                if (i + 1 < argc) {
                    grCaptureInterval = atoi(argv[++i]);
                    if (grCaptureInterval <= 0) {
                        fprintf(stderr, "Error: capture-interval must be > 0\n");
                        usage();
                    }
                } else {
                    fprintf(stderr, "Error: capture-interval requires N\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "capture-overlay")) {
                grCaptureOverlay = 1;
            }
            else if (!strcmp(argv[i], "capture-overlay-mask")) {
                grCaptureOverlay = 1;
                grCaptureOverlayMaskOnly = 1;
            }
            else if (!strcmp(argv[i], "capture-foreground-only")) {
                grCaptureForegroundOnly = 1;
            }
            else if (!strcmp(argv[i], "noloop")) {
                screensaverLoopDisabled = 1;
            }
            else if (!strcmp(argv[i], "capture-sound-events")) {
                if (i + 1 < argc) {
                    grCaptureSoundEventsPath = argv[++i];
                } else {
                    fprintf(stderr, "Error: capture-sound-events requires a file path\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "capture-scene-label")) {
                if (i + 1 < argc) {
                    grCaptureSetSceneLabel(argv[++i]);
                } else {
                    fprintf(stderr, "Error: capture-scene-label requires text\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "seed")) {
                if (i + 1 < argc) {
                    hostForcedSeed = atoi(argv[++i]);
                } else {
                    fprintf(stderr, "Error: seed requires a value\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "story-day")) {
                if (i + 1 < argc) {
                    hostForcedStoryDay = atoi(argv[++i]);
                    if (hostForcedStoryDay < 1 || hostForcedStoryDay > 11) {
                        fprintf(stderr, "Error: story-day must be in range 1..11\n");
                        usage();
                    }
                } else {
                    fprintf(stderr, "Error: story-day requires a value\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "island-pos")) {
                if (i + 2 < argc) {
                    hostForcedIslandX = atoi(argv[++i]);
                    hostForcedIslandY = atoi(argv[++i]);
                    hostForcedIslandPosValid = 1;
                } else {
                    fprintf(stderr, "Error: island-pos requires X and Y\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "lowtide")) {
                if (i + 1 < argc) {
                    hostForcedLowTide = atoi(argv[++i]) ? 1 : 0;
                } else {
                    fprintf(stderr, "Error: lowtide requires 0 or 1\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "raft-stage")) {
                if (i + 1 < argc) {
                    hostForcedRaftStage = atoi(argv[++i]);
                    if (hostForcedRaftStage < 0 || hostForcedRaftStage > 5) {
                        fprintf(stderr, "Error: raft-stage must be in range 0..5\n");
                        usage();
                    }
                } else {
                    fprintf(stderr, "Error: raft-stage requires a value\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "night")) {
                if (i + 1 < argc) {
                    hostForcedNight = atoi(argv[++i]) ? 1 : 0;
                } else {
                    fprintf(stderr, "Error: night requires 0 or 1\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "holiday")) {
                if (i + 1 < argc) {
                    hostForcedHoliday = atoi(argv[++i]);
                    if (hostForcedHoliday < 0) hostForcedHoliday = 0;
                    if (hostForcedHoliday > 4) hostForcedHoliday = 4;
                } else {
                    fprintf(stderr, "Error: holiday requires a value 0..4\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "scene-offset")) {
                if (i + 2 < argc) {
                    hostForcedSceneOffsetX = atoi(argv[++i]);
                    hostForcedSceneOffsetY = atoi(argv[++i]);
                    hostForcedSceneOffsetValid = 1;
                } else {
                    fprintf(stderr, "Error: scene-offset requires X and Y\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "capture-prelude-frame")) {
                hostCapturePreludeFrame = 1;
            }
        }
    }

    if (numExpectedArgs)
        usage();

    if (argDump + argBench + argTtm + argAds + argForegroundPilot > 1)
        usage();

    if (argDump + argBench + argTtm + argAds + argForegroundPilot == 0)
        argPlayAll = 1;
}
#endif


int main(int argc, char **argv)
{
#ifdef PS1_BUILD
    /* Initialize debug system FIRST, before any CD operations */
    /* FntLoad must happen before CdInit or it causes hangs */
    ps1DebugInit();

    /* Initialize CD-ROM subsystem */
    if (cdromInit() < 0) {
        ps1DebugError("CD-ROM init failed!");
        while(1);
    }

    debugMode = 0;  /* Keep PS1 debug chatter opt-in; use BOOTMODE probes for logs. */

    /* Load boot override BEFORE seeding RNG so "seed N" can override. */
    ps1LoadBootOverride();
    if (!argForegroundPilot) {
        argForegroundPilot = 1;
    }
    ps1PrintfProbe("boot-override-loaded", NULL);

    loadTitleScreenEarly();
    ps1PrintfProbe("title-loaded", NULL);

    /* Parse resource files from CD - needed for background and sprites */
    parseResourceFiles("RESOURCE.MAP");
    ps1PrintfProbe("resources-loaded", NULL);

    /* Seed RNG — use forced seed if specified in BOOTMODE, else hardware RNG. */
    if (ps1BootForcedSeed >= 0) {
        srand((unsigned int)ps1BootForcedSeed);
    } else {
        ps1SeedRandom();
    }
#else
    /* Non-PS1: normal flow */
    parseArgs(argc, argv);

    if (argDump)
        debugMode = 1;

    parseResourceFiles("RESOURCE.MAP");

    storySetForcedCurrentDay(hostForcedStoryDay);
    storySetIslandOverrides(
        hostForcedIslandPosValid,
        hostForcedIslandX,
        hostForcedIslandY,
        hostForcedLowTide >= 0,
        hostForcedLowTide,
        hostForcedRaftStage >= 0,
        hostForcedRaftStage
    );
    storySetSceneOffsetOverride(
        hostForcedSceneOffsetValid,
        hostForcedSceneOffsetX,
        hostForcedSceneOffsetY
    );
    storySetCapturePreludeFrame(hostCapturePreludeFrame);
    if (argForegroundPilot && numArgs >= 1)
        foregroundPilotSetScene(args[0]);
    else
        foregroundPilotSetScene(NULL);

    if (hostForcedSeed >= 0)
        srand((unsigned int)hostForcedSeed);
    else
        srand((unsigned int)time(NULL));
#endif

#ifdef PS1_BUILD
    /* Resource counts available via extern */
    extern int numPalResources;
    extern struct TPalResource *palResources[];
#endif

    /* Initialize LRU cache for memory management */
    initLRUCache();

#ifdef PS1_BUILD
    if (ps1BootForegroundOverlayScene[0] != '\0')
        foregroundPilotSetScene(ps1BootForegroundOverlayScene);
    else if (argForegroundPilot && numArgs >= 1)
        foregroundPilotSetScene(args[0]);
    else
        foregroundPilotSetScene(NULL);

    graphicsInit();
    ps1PrintfProbe("graphics-init", NULL);
    soundInit();
    ps1PrintfProbe("sound-init", NULL);

    if (numPalResources > 0 && palResources[0]) {
        grLoadPalette(palResources[0]);
    }

    if (hostForcedLowTide >= 0)
        islandState.lowTide = hostForcedLowTide;
    if (hostForcedRaftStage >= 0)
        islandState.raft = hostForcedRaftStage;
    if (hostForcedNight >= 0)
        islandState.night = hostForcedNight;
    if (hostForcedHoliday >= 0)
        islandState.holiday = hostForcedHoliday;
    if (hostForcedIslandPosValid) {
        islandState.xPos = hostForcedIslandX;
        islandState.yPos = hostForcedIslandY;
    }

    /* PS1 is now FG2-scene-playback only. Host ADS/TTM/story engines stay
     * available for capture tooling, but they are no longer linked into the
     * console executable. */
    const char *explicitScene = (ps1BootForegroundOverlayScene[0] != '\0')
                                ? ps1BootForegroundOverlayScene
                                : ((numArgs >= 1) ? args[0] : NULL);
    do {
        const char *loopScene = fgLoopNextScene(explicitScene);
        fgLoopApplyVariant(loopScene);
        foregroundPilotSetScene(loopScene);
        ps1PerfBeginScene(loopScene);
        ps1PrintfProbe("scene-start", loopScene);
        foregroundPilotPlay();
        ps1PerfEndScene(loopScene);
        ps1PrintfProbe("scene-end", loopScene);
    } while (!screensaverLoopDisabled);

    soundEnd();
    graphicsEnd();
    return 0;
#endif

#ifndef PS1_BUILD
    if (hostBootDirectSceneIndex >= 0) {
        printf("Initializing graphics...\n");
        graphicsInit();
        printf("Graphics initialized\n");

        printf("Initializing sound...\n");
        soundInit();
        printf("Sound initialized\n");

        printf("Starting direct story scene %d...\n", hostBootDirectSceneIndex);
        storyPlayBootSceneDirect(hostBootDirectSceneIndex);

        printf("Shutting down sound...\n");
        soundEnd();
        printf("Shutting down graphics...\n");
        graphicsEnd();
        printf("Shutdown complete\n");
    }

    else if (argPlayAll) {
        printf("Initializing graphics...\n");
        graphicsInit();
        printf("Graphics initialized\n");

        printf("Initializing sound...\n");
        soundInit();
        printf("Sound initialized\n");

        printf("Starting story mode...\n");
        storyPlay();

        printf("Shutting down sound...\n");
        soundEnd();
        printf("Shutting down graphics...\n");
        graphicsEnd();
        printf("Shutdown complete\n");
    }

    else if (argDump) {
        dumpAllResources();
    }

    else if (argBench) {
        graphicsInit();
        adsPlayBench();
        graphicsEnd();
    }

    else if (argTtm) {
        graphicsInit();

#ifdef PS1_BUILD
        /* PS1: Simple render test - bypass TTM logic for now */
        printf("PS1: Starting simple render test (300 frames)...\n");

        int frame_count = 0;
        while (frame_count < 300) {  /* Run for 5 seconds at 60fps */
            grRefreshDisplay();

            frame_count++;
            if ((frame_count % 60) == 0) {
                printf("Frame %d\n", frame_count);
            }
        }
        printf("PS1: Render test complete\n");
#else
        soundInit();
        adsPlaySingleTtm(args[0], (numArgs >= 2) ? (uint16)atoi(args[1]) : 0);
        soundEnd();
#endif

        graphicsEnd();
    }

    else if (argAds) {

        graphicsInit();
        soundInit();
        adsInit();

        if (hostForcedLowTide >= 0)
            islandState.lowTide = hostForcedLowTide;
        if (hostForcedRaftStage >= 0)
            islandState.raft = hostForcedRaftStage;
        if (hostForcedIslandPosValid) {
            islandState.xPos = hostForcedIslandX;
            islandState.yPos = hostForcedIslandY;
        }

        if (argIsland)
            adsInitIsland();
        else
            adsNoIsland();

        adsPlay(args[0], atoi(args[1]));

        soundEnd();
        graphicsEnd();
    }

    else if (argForegroundPilot) {
        printf("Initializing graphics...\n");
        graphicsInit();
        printf("Graphics initialized\n");
        /* Host fgpilot path: same screensaver loop as PS1. See comment
         * on the PS1 branch above. */
        const char *explicitScene = (numArgs >= 1) ? args[0] : NULL;
        do {
            const char *loopScene = fgLoopNextScene(explicitScene);
            fgLoopApplyVariant(loopScene);
            foregroundPilotSetScene(loopScene);
            foregroundPilotPlay();
        } while (!screensaverLoopDisabled);
        graphicsEnd();
        printf("Shutdown complete\n");
    }
#endif

    return 0;
}
