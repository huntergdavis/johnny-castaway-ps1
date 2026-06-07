/*
 *  This file is part of 'Johnny Reborn' - PS1 port.
 *
 *  scene_freeplay -- PS1-only direct-control Johnny mode.
 */

#include "scene_freeplay.h"

#ifdef PS1_BUILD

#include <psxapi.h>
#include <psxgpu.h>
#include <psxpad.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mytypes.h"
#include "graphics_ps1.h"
#include "foreground_pilot.h"
#include "island.h"
#include "holidays.h"
#include "pause_menu.h"
#include "ps1_pad_input.h"
#include "ps1_captions.h"
#include "ps1_gpu_ot.h"
#include "sound_ps1.h"
#include "walk_render.h"
#include "freeplay_sprite_indices.h"

extern struct TIslandState islandState;
extern uint8 pad_buff[2][34];
extern uint32 ps1FrameCount;
extern int grDx;
extern int grDy;
extern int hostForcedNight;
extern int hostHolidayMode;
extern int hostForcedHoliday;
extern int hostForcedLowTide;
extern int hostForcedRaftStage;
extern int hostForcedIslandPosValid;
extern int hostForcedIslandX;
extern int hostForcedIslandY;
extern int ps1SoftHour;
extern int ps1SoftMonth;
extern int ps1SoftDay;
extern int ps1SoftYear;
extern int ps1SoftTimeEnabled;

#define FP_SLOT_JOHN       0
#define FP_SLOT_GAG        1
#define FP_SLOT_SUMMON     2
#define FP_SLOT_PERSIST    3

#define FP_CLEAN_RECT_COUNT 3
#define FP_OVERLAY_OT_LEN  8
#define FP_OVERLAY_PRIM_BYTES 24576
/* Flip threshold is the midpoint of the palm-tree trunk (the visual
 * center of the island). The trunk sprite is drawn at (442, 148) and is
 * 24 px wide (BACKGRND.BMP frame 13), so the trunk midpoint is x=454.
 * Past that, draw the native sprite (Johnny faces right); before it,
 * mirror so Johnny faces left. */
#define FP_FISH_TRUNK_X 454
#define FP_BANNER_X0 188
#define FP_BANNER_Y0 28
#define FP_BANNER_X1 452
#define FP_BANNER_Y1 64
#define FP_CAPTION_X0 36
#define FP_CAPTION_Y0 396
#define FP_CAPTION_X1 604
#define FP_CAPTION_Y1 474
#ifndef FREEPLAY_DIAG_LOGS
#define FREEPLAY_DIAG_LOGS 0
#endif
#ifndef PS1_VERBOSE_DIAGNOSTICS
#define PS1_VERBOSE_DIAGNOSTICS 0
#endif
#if FREEPLAY_DIAG_LOGS
#define FP_LOG(level, args) do { if (gFreeplayTelemetryLevel >= (level)) printf args; } while (0)
#else
#define FP_LOG(level, args) do { (void)(level); } while (0)
#endif

enum TFpFace {
    FP_FACE_E = 0,
    FP_FACE_W,
    FP_FACE_N,
    FP_FACE_S
};

enum TFpMode {
    FP_MODE_IDLE = 0,
    FP_MODE_WALK,
    FP_MODE_FISH,
    FP_MODE_ONESHOT,
    FP_MODE_STRUT,
    FP_MODE_RUNAWAY
};

enum TFpGag {
    FP_GAG_EAT = 0,
    FP_GAG_HOT,
    FP_GAG_IDEA,
    FP_GAG_ANGRY,
    FP_GAG_BONK,
    FP_GAG_STRUT,
    FP_GAG_RUNAWAY,
    FP_GAG_COUNT
};

enum TFpSummon {
    FP_SUMMON_SEAGULL = 0,
    FP_SUMMON_LILIPUTS,
    FP_SUMMON_BIPLANE,
    FP_SUMMON_CANOE,
    FP_SUMMON_BOAT,
    FP_SUMMON_KINGKONG,
    FP_SUMMON_MARY,
    FP_SUMMON_PIRATE,
    FP_SUMMON_FLOCK,
    FP_SUMMON_MEANWHILE,
    FP_SUMMON_CLOUD,
    FP_SUMMON_COUNT
};

enum TFpPersistentKind {
    FP_PERSIST_CASTLE = 0,
    FP_PERSIST_COCONUT
};

struct TFpAsset {
    int active;
    char bmp[16];
    uint8 slot;
    uint16 frame;
    uint16 frameDelay;
    uint16 frameTimer;
    sint16 x;
    sint16 y;
    sint16 vx;
    sint16 vy;
    uint16 ttl;
    uint16 firstFrame;
    uint16 frameCount;
    uint16 loopStart;     /* When loop=1, wrap to firstFrame+loopStart (not 0) */
    sint16 bodyAnchorX;   /* If >0, asset->x is the body screen-x, not sprite top-left.
                           * fpDrawAsset converts to top-left per-frame using sprite
                           * fullWidth + this offset. Use 0 for legacy top-left mode. */
    uint8 flip;
    uint8 screenRelative;
    uint8 loop;
};

struct TFpPersistent {
    uint8 active;
    uint8 kind;
    uint8 frame;
    sint16 x;
    sint16 y;
};

struct TFpMenuMeta {
    const char *title;
    const char *description;
    const char *bmp;
    uint16 frames;
    uint16 memoryKB;
};

struct TFpState {
    sint16 x;
    sint16 y;
    enum TFpFace face;
    enum TFpMode mode;
    uint16 modeTimer;
    uint16 walkFrame;
    uint16 walkStepCounter;
    uint8 gagCursor;
    uint8 lastGag;
    uint8 summonCursor;
    uint16 bannerTimer;
    char banner[32];
    uint32 frame;
    uint32 inputFrames;
    uint32 walkFrames;
    uint32 actionCount;
    uint32 summonCount;
    uint32 cleanRebuilds;
    uint32 cleanFailures;
    uint32 assetLoadFailures;
    uint32 clampCount;
    uint32 ambientNextFrame;
    uint32 idleFrames;
    uint16 prevButtons;
    uint8 fridayCount;
    uint8 castleStage;
    uint8 fireActive;
    sint16 fireX;
    sint16 fireY;
    uint8 fireFrame;
    uint8 fireTimer;
    uint8 overlayWasVisible;
    struct TFpAsset gag;
    struct TFpAsset summon;
    struct TFpPersistent persist[16];
};

static struct TTtmSlot gFpSlot;
static struct TFpState gFp;
static int gFreeplayExitRequested = 0;
static int gFreeplayTelemetryLevel = 0;

static const char kFpCaptionStart[] =
    "Freeplay mode is on.\nWalk Johnny around the island.";
static const char kFpCaptionAction[] =
    "Johnny does a quick gag.";
static const char kFpCaptionSummon[] =
    "A visitor appears on the island.";
static const char kFpCaptionPause[] =
    "Freeplay is paused.";
static const char kFpCaptionClear[] =
    "Freeplay screen cleared.";

static uint32 fpOverlayOt[FP_OVERLAY_OT_LEN];
static uint8  fpOverlayPrim[FP_OVERLAY_PRIM_BYTES];
static uint8 *fpOverlayNext;
static uint32 *fpOverlayTextOt;
static int fpTextX;
static int fpTextY;

static const char *const kFpGagBmps[FP_GAG_COUNT] = {
    "GJFFFOOD.BMP",
    "GJHOT.BMP",
    "LITEBULB.BMP",
    "GJANGRY.BMP",
    "COCOHEAD.BMP",
    "MEXCWALK.BMP",
    "GJRUNAWA.BMP"
};

static const uint16 kFpGagDurations[FP_GAG_COUNT] = {
    210, 130, 80, 160, 80, 300, 180
};

static const uint16 kFpGagFrameCounts[FP_GAG_COUNT] = {
    49, 23, 5, 49, 10, 8, 14
};

static const uint16 kFpGagFrameDelays[FP_GAG_COUNT] = {
    4, 5, 12, 3, 6, 4, 4
};

static const sint16 kFpGagXOffsets[FP_GAG_COUNT] = {
    -18, 0, 2, -4, 8, 0, 0
};

static const sint16 kFpGagYOffsets[FP_GAG_COUNT] = {
    -6, 0, -44, -8, -34, 0, 0
};

static const uint8 kFpGagFlipWithFace[FP_GAG_COUNT] = {
    1, 1, 0, 1, 0, 1, 1
};

static const char *const kFpGagLabels[FP_GAG_COUNT] = {
    "SCRATCH",
    "HOT",
    "IDEA",
    "ANGRY",
    "BONK",
    "STRUT",
    "RUNAWAY"
};

static const struct TFpMenuMeta kFpGagMeta[FP_GAG_COUNT] = {
    {"Scratch","Johnny scratches and settles in.",     "GJFFFOOD.BMP",49,135},
    {"Hot",    "Johnny wipes out in the heat.",        "GJHOT.BMP",   23, 33},
    {"Idea",   "A light bulb appears over Johnny.",    "LITEBULB.BMP", 5,  5},
    {"Angry",  "Johnny throws a small tantrum.",       "GJANGRY.BMP", 49, 38},
    {"Bonk",   "A coconut clocks Johnny.",             "COCOHEAD.BMP",10,  4},
    {"Strut",  "Johnny takes a proud little walk.",    "MEXCWALK.BMP", 8, 11},
    {"Runaway","Johnny bolts across the island.",      "GJRUNAWA.BMP",14, 42}
};

static const struct TFpMenuMeta kFpSummonMeta[FP_SUMMON_COUNT] = {
    {"Seagull",   "A small bird visits the palm.",      "GJGULL1.BMP", 1, 32},
    {"Liliputs",  "Tiny visitors cross the sand.",      "LILIPUTS.BMP",4, 64},
    {"Biplane",   "A plane sweeps past the island.",    "GJBIPLAN.BMP",1, 48},
    {"Canoe",     "A native canoe stops by.",           "GJNAT1.BMP",  1, 48},
    {"Boat",      "A boat appears offshore.",           "BOAT.BMP",    1, 48},
    {"King Kong", "A skyline-sized visitor looms.",     "GJKINGKO.BMP",1, 80},
    {"Mary",      "Mary drops into the scene.",         "MJBATH.BMP",  6, 96},
    {"Pirate",    "The fish-man pirate appears.",       "FISHMAN.BMP", 1, 64},
    {"Flock",     "A small flock passes overhead.",     "GJGULL3.BMP", 3, 64},
    {"Meanwhile", "A comic meanwhile card appears.",    "MEANWHIL.BMP",1, 48},
    {"Cloud",     "A cloud floats over the island.",    "CLOUDS.BMP",  1, 64}
};

#include "scene_freeplay/catalog.c.inc"
#include "scene_freeplay/backdrop.c.inc"
#include "scene_freeplay/actions.c.inc"
#include "scene_freeplay/render.c.inc"
#include "scene_freeplay/runtime.c.inc"
#else

void freeplayRun(void)
{
}

int freeplayExitRequested(void)
{
    return 0;
}

void freeplayClearExitRequest(void)
{
}


void freeplaySetTelemetryLevel(int level)
{
    (void)level;
}

int freeplayGetTelemetryLevel(void)
{
    return 0;
}

int freeplayGagCount(void) { return 0; }
const char *freeplayGagTitle(int index) { (void)index; return ""; }
const char *freeplayGagDescription(int index) { (void)index; return ""; }
const char *freeplayGagBmp(int index) { (void)index; return ""; }
int freeplayGagFrames(int index) { (void)index; return 0; }
int freeplayGagMemoryKB(int index) { (void)index; return 0; }

int freeplayVisitorCount(void) { return 0; }
const char *freeplayVisitorTitle(int index) { (void)index; return ""; }
const char *freeplayVisitorDescription(int index) { (void)index; return ""; }
const char *freeplayVisitorBmp(int index) { (void)index; return ""; }
int freeplayVisitorFrames(int index) { (void)index; return 0; }
int freeplayVisitorMemoryKB(int index) { (void)index; return 0; }

#endif
