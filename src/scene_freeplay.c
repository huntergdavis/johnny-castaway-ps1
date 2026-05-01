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
#define FP_FISH_RIGHT_SIDE_X 440
#define FP_BANNER_X0 188
#define FP_BANNER_Y0 28
#define FP_BANNER_X1 452
#define FP_BANNER_Y1 64
#define FP_CAPTION_X0 36
#define FP_CAPTION_Y0 396
#define FP_CAPTION_X1 604
#define FP_CAPTION_Y1 474
#define FP_LOG(level, args) do { if (gFreeplayTelemetryLevel >= (level)) printf args; } while (0)

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
    uint8 fishingPhase;
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
static const char kFpCaptionFish[] =
    "Johnny fishes.";
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

static const char *fpModeName(enum TFpMode mode)
{
    switch (mode) {
    case FP_MODE_IDLE: return "idle";
    case FP_MODE_WALK: return "walk";
    case FP_MODE_FISH: return "fish";
    case FP_MODE_ONESHOT: return "oneshot";
    case FP_MODE_STRUT: return "strut";
    case FP_MODE_RUNAWAY: return "runaway";
    default: return "?";
    }
}

static const struct TFpMenuMeta *fpMetaAt(const struct TFpMenuMeta *meta,
                                          int count, int index)
{
    if (meta == NULL || count <= 0)
        return NULL;
    if (index < 0)
        index = 0;
    if (index >= count)
        index = count - 1;
    return &meta[index];
}

int freeplayGagCount(void)
{
    return FP_GAG_COUNT;
}

const char *freeplayGagTitle(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpGagMeta, FP_GAG_COUNT, index);
    return meta ? meta->title : "";
}

const char *freeplayGagDescription(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpGagMeta, FP_GAG_COUNT, index);
    return meta ? meta->description : "";
}

const char *freeplayGagBmp(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpGagMeta, FP_GAG_COUNT, index);
    return meta ? meta->bmp : "";
}

int freeplayGagFrames(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpGagMeta, FP_GAG_COUNT, index);
    return meta ? meta->frames : 0;
}

int freeplayGagMemoryKB(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpGagMeta, FP_GAG_COUNT, index);
    return meta ? meta->memoryKB : 0;
}

int freeplayVisitorCount(void)
{
    return FP_SUMMON_COUNT;
}

const char *freeplayVisitorTitle(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpSummonMeta, FP_SUMMON_COUNT, index);
    return meta ? meta->title : "";
}

const char *freeplayVisitorDescription(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpSummonMeta, FP_SUMMON_COUNT, index);
    return meta ? meta->description : "";
}

const char *freeplayVisitorBmp(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpSummonMeta, FP_SUMMON_COUNT, index);
    return meta ? meta->bmp : "";
}

int freeplayVisitorFrames(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpSummonMeta, FP_SUMMON_COUNT, index);
    return meta ? meta->frames : 0;
}

int freeplayVisitorMemoryKB(int index)
{
    const struct TFpMenuMeta *meta = fpMetaAt(kFpSummonMeta, FP_SUMMON_COUNT, index);
    return meta ? meta->memoryKB : 0;
}

static int fpLoadSlot(uint8 slot, const char *bmp)
{
    if (bmp == NULL || bmp[0] == '\0')
        return 0;
    if (slot >= MAX_BMP_SLOTS)
        return 0;
    if (gFpSlot.numSprites[slot] > 0 &&
        gFpSlot.loadedBmpNames[slot] != NULL &&
        strcmp(gFpSlot.loadedBmpNames[slot], bmp) == 0) {
        return 1;
    }
    if (gFpSlot.numSprites[slot] > 0)
        grReleaseBmp(&gFpSlot, slot);
    grLoadBmp(&gFpSlot, slot, (char *)bmp);
    if (gFpSlot.numSprites[slot] == 0) {
        gFp.assetLoadFailures++;
        FP_LOG(2, ("JCFREE asset-fail slot=%u bmp=%s failures=%lu\n",
                   (unsigned)slot, bmp, (unsigned long)gFp.assetLoadFailures));
        return 0;
    }
    FP_LOG(3, ("JCFREE asset slot=%u bmp=%s frames=%u\n",
               (unsigned)slot, bmp, (unsigned)gFpSlot.numSprites[slot]));
    return 1;
}

static void fpCancelAsset(struct TFpAsset *asset)
{
    if (asset == NULL)
        return;
    if (!asset->active) {
        memset(asset, 0, sizeof(*asset));
        return;
    }
    if (asset->slot < MAX_BMP_SLOTS && gFpSlot.numSprites[asset->slot] > 0)
        grReleaseBmp(&gFpSlot, asset->slot);
    memset(asset, 0, sizeof(*asset));
}

static void fpReleaseSlots(void)
{
    for (uint8 i = 0; i < MAX_BMP_SLOTS; i++) {
        if (gFpSlot.numSprites[i] > 0)
            grReleaseBmp(&gFpSlot, i);
    }
}

static void fpSetIslandDrawOffset(void)
{
    grDx = islandState.xPos;
    grDy = islandState.yPos;
}

static void fpSetScreenDrawOffset(void)
{
    grDx = 0;
    grDy = 0;
}

static int fpBehindTree(sint16 x, sint16 y)
{
    return (x >= 430 && x <= 478 && y >= 184 && y <= 238) ? 1 : 0;
}

static uint16 fpIdleFrame(void)
{
    switch (gFp.face) {
    case FP_FACE_W: return FP_JOHN_IDLE_W;
    case FP_FACE_N: return FP_JOHN_IDLE_N;
    case FP_FACE_S: return FP_JOHN_IDLE_S;
    case FP_FACE_E:
    default: return FP_JOHN_IDLE_E;
    }
}

static uint16 fpWalkFrame(void)
{
    uint16 idx;
    switch (gFp.face) {
    case FP_FACE_W:
        idx = (uint16)((gFp.walkFrame / 4) %
                       (sizeof(kFpWalkWestFrames) / sizeof(kFpWalkWestFrames[0])));
        return kFpWalkWestFrames[idx];
    case FP_FACE_N:
        idx = (uint16)((gFp.walkFrame / 5) %
                       (sizeof(kFpWalkNorthFrames) / sizeof(kFpWalkNorthFrames[0])));
        return kFpWalkNorthFrames[idx];
    case FP_FACE_S:
        idx = (uint16)((gFp.walkFrame / 5) %
                       (sizeof(kFpWalkSouthFrames) / sizeof(kFpWalkSouthFrames[0])));
        return kFpWalkSouthFrames[idx];
    case FP_FACE_E:
    default:
        idx = (uint16)((gFp.walkFrame / 4) %
                       (sizeof(kFpWalkEastFrames) / sizeof(kFpWalkEastFrames[0])));
        return kFpWalkEastFrames[idx];
    }
}

static void fpRestampTreeIfNeeded(void)
{
    struct TTtmSlot *bgSlot;
    if (!fpBehindTree(gFp.x, gFp.y))
        return;
    bgSlot = fgBackdropGetSlot();
    if (bgSlot == NULL)
        return;
    fpSetIslandDrawOffset();
    grDrawSprite(grBackgroundSfc, bgSlot, 442, 148, 13, 0);
    grDrawSprite(grBackgroundSfc, bgSlot, 365, 122, 12, 0);
}

static int fpSaveCleanRects(void)
{
    const uint32 kMaxCleanRectBytes = 96UL * 1024UL;
    sint16 islandX = (sint16)islandState.xPos;
    sint16 islandY = (sint16)islandState.yPos;
    sint16 xs[FP_CLEAN_RECT_COUNT];
    sint16 ys[FP_CLEAN_RECT_COUNT];
    uint16 ws[FP_CLEAN_RECT_COUNT];
    uint16 hs[FP_CLEAN_RECT_COUNT];
    int rc;

    xs[0] = (sint16)(islandX + 129);
    ys[0] = (sint16)(islandY + 180);
    ws[0] = 479;
    hs[0] = 220; /* island, Johnny, surf */

    xs[1] = (sint16)(islandX + 240);
    ys[1] = (sint16)(islandY + 80);
    ws[1] = 400;
    hs[1] = 128; /* sky/summons */

    xs[2] = (sint16)(islandX + 0);
    ys[2] = (sint16)(islandY + 120);
    ws[2] = 260;
    hs[2] = 96; /* upper-left Johnny smear */

    rc = grSaveCleanBgRectsSplit(xs, ys, ws, hs, FP_CLEAN_RECT_COUNT,
                                 kMaxCleanRectBytes);
    if (rc <= 0) {
        gFp.cleanFailures++;
        FP_LOG(1, ("JCFREE clean-fail rc=%d source=%d failures=%lu heapKB=%lu\n",
                   rc, FP_CLEAN_RECT_COUNT,
                   (unsigned long)gFp.cleanFailures,
                   fgProbeLargestAlloc() / 1024UL));
        return 0;
    }
    gFp.cleanRebuilds++;
    FP_LOG(2, ("JCFREE clean-ok rebuild=%lu bytes=%lu heapKB=%lu\n",
               (unsigned long)gFp.cleanRebuilds,
               grCleanBgRectsBytes(),
               fgProbeLargestAlloc() / 1024UL));
    return 1;
}

static void fpStampPersistents(void)
{
    int wantCastle = 0;
    int wantCoconut = 0;

    for (int i = 0; i < (int)(sizeof(gFp.persist) / sizeof(gFp.persist[0])); i++) {
        if (!gFp.persist[i].active)
            continue;
        if (gFp.persist[i].kind == FP_PERSIST_CASTLE)
            wantCastle = 1;
        else if (gFp.persist[i].kind == FP_PERSIST_COCONUT)
            wantCoconut = 1;
    }

    fpSetIslandDrawOffset();

    if (gFp.fireActive) {
        /* Fire is animated, not baked into the baseline. */
    }

    if (wantCastle && fpLoadSlot(FP_SLOT_PERSIST, "GJCASTLE.BMP")) {
        for (int i = 0; i < (int)(sizeof(gFp.persist) / sizeof(gFp.persist[0])); i++) {
            if (!gFp.persist[i].active || gFp.persist[i].kind != FP_PERSIST_CASTLE)
                continue;
            grDrawSprite(grBackgroundSfc, &gFpSlot,
                         gFp.persist[i].x, gFp.persist[i].y,
                         gFp.persist[i].frame, FP_SLOT_PERSIST);
        }
    }

    if (wantCoconut && fpLoadSlot(FP_SLOT_PERSIST, "COCONUTS.BMP")) {
        for (int i = 0; i < (int)(sizeof(gFp.persist) / sizeof(gFp.persist[0])); i++) {
            if (!gFp.persist[i].active || gFp.persist[i].kind != FP_PERSIST_COCONUT)
                continue;
            grDrawSprite(grBackgroundSfc, &gFpSlot,
                         gFp.persist[i].x, gFp.persist[i].y,
                         gFp.persist[i].frame, FP_SLOT_PERSIST);
        }
    }

    if (gFpSlot.numSprites[FP_SLOT_PERSIST] > 0)
        grReleaseBmp(&gFpSlot, FP_SLOT_PERSIST);
}

static int fpRebuildBackdrop(void)
{
    grDeactivateCleanBgRects();
    fgBackdropPrepareIslandRuntimePublic();
    fpStampPersistents();
    fgBackdropStampHolidayPublic();
    if (!fpSaveCleanRects())
        return 0;
    grForceFullRedrawNextFrame();
    return 1;
}

static uint16 fpReadButtons(void)
{
    uint16 buttons = 0;
    for (int i = 0; i < 2; i++) {
        PADTYPE *pad = (PADTYPE *)pad_buff[i];
        buttons |= ps1PadButtonsWithAnalog(pad);
    }
    return buttons;
}

static int fpClampJohnny(void)
{
    sint16 oldX = gFp.x;
    sint16 oldY = gFp.y;
    if (gFp.x < 245) gFp.x = 245;
    if (gFp.x > 535) gFp.x = 535;
    if (gFp.y < 205) gFp.y = 205;
    if (gFp.y > 322) gFp.y = 322;
    if (gFp.y < 235 && gFp.x < 300) gFp.x = 300;
    if (gFp.y < 220 && gFp.x > 500) gFp.x = 500;
    if (oldX != gFp.x || oldY != gFp.y) {
        gFp.clampCount++;
        return 1;
    }
    return 0;
}

static void fpSetBanner(const char *text, uint16 frames)
{
    strncpy(gFp.banner, text, sizeof(gFp.banner) - 1);
    gFp.banner[sizeof(gFp.banner) - 1] = '\0';
    gFp.bannerTimer = frames;
}

static void fpCancelTransientAction(void)
{
    fpCancelAsset(&gFp.gag);
    fpCancelAsset(&gFp.summon);
    gFp.mode = FP_MODE_IDLE;
    gFp.modeTimer = 0;
    gFp.fishingPhase = 0;
    gFp.idleFrames = 0;
    grForceFullRedrawNextFrame();
}

static void fpClearScreen(void)
{
    fpCancelTransientAction();
    fpSetBanner("CLEARED", 90);
    captionsShowText(kFpCaptionClear, 120);
    grShowMeanwhileLoadingFrame((uint16)(gFp.frame & 7UL));
    if (!fpRebuildBackdrop())
        gFreeplayExitRequested = 1;
}

static void fpStartGag(enum TFpGag gag)
{
    gFp.actionCount++;
    gFp.lastGag = (uint8)gag;
    gFp.fishingPhase = 0;

    fpCancelAsset(&gFp.gag);
    fpCancelAsset(&gFp.summon);
    if (!fpLoadSlot(FP_SLOT_GAG, kFpGagBmps[gag])) {
        fpSetBanner("ACTION UNAVAILABLE", 60);
        return;
    }
    strncpy(gFp.gag.bmp, kFpGagBmps[gag], sizeof(gFp.gag.bmp) - 1);
    gFp.gag.slot = FP_SLOT_GAG;
    gFp.gag.active = 1;
    gFp.gag.frameDelay = kFpGagFrameDelays[gag];
    gFp.gag.ttl = kFpGagDurations[gag];
    gFp.gag.x = (sint16)(gFp.x + kFpGagXOffsets[gag]);
    gFp.gag.y = (sint16)(gFp.y + kFpGagYOffsets[gag]);
    gFp.gag.firstFrame = 0;
    gFp.gag.frameCount = kFpGagFrameCounts[gag];
    if (gFp.gag.frameCount > gFpSlot.numSprites[FP_SLOT_GAG])
        gFp.gag.frameCount = gFpSlot.numSprites[FP_SLOT_GAG];
    if (gFp.gag.frameCount == 0)
        gFp.gag.frameCount = 1;
    gFp.gag.flip = (kFpGagFlipWithFace[gag] && gFp.face == FP_FACE_W) ? 1 : 0;
    gFp.gag.loop = (gag == FP_GAG_STRUT || gag == FP_GAG_RUNAWAY) ? 1 : 0;
    gFp.gag.screenRelative = 0;
    gFp.mode = (gag == FP_GAG_STRUT) ? FP_MODE_STRUT :
               (gag == FP_GAG_RUNAWAY) ? FP_MODE_RUNAWAY :
               FP_MODE_ONESHOT;
    gFp.modeTimer = gFp.gag.ttl;
    fpSetBanner(kFpGagLabels[gag], 90);
    captionsShowText(kFpCaptionAction, 150);

    if (gag == FP_GAG_BONK) soundPlay(5);
    else if (gag == FP_GAG_EAT) soundPlay(8);
    else if (gag == FP_GAG_HOT) soundPlay(4);
}

static void fpStartAssetModeEx(const char *bmp, uint16 ttl, enum TFpMode mode,
                               sint16 x, sint16 y, uint16 firstFrame,
                               uint16 frameCount, uint16 frameDelay,
                               uint8 flip, int soundId)
{
    uint16 availableFrames;

    gFp.actionCount++;
    if (mode != FP_MODE_FISH)
        gFp.fishingPhase = 0;
    fpCancelAsset(&gFp.gag);
    fpCancelAsset(&gFp.summon);
    if (!fpLoadSlot(FP_SLOT_GAG, bmp)) {
        fpSetBanner("ACTION SKIPPED", 60);
        return;
    }
    strncpy(gFp.gag.bmp, bmp, sizeof(gFp.gag.bmp) - 1);
    gFp.gag.slot = FP_SLOT_GAG;
    gFp.gag.active = 1;
    gFp.gag.frameDelay = frameDelay ? frameDelay : 6;
    gFp.gag.ttl = ttl;
    gFp.gag.x = x;
    gFp.gag.y = y;
    availableFrames = gFpSlot.numSprites[FP_SLOT_GAG];
    if (firstFrame >= availableFrames)
        firstFrame = 0;
    gFp.gag.firstFrame = firstFrame;
    if (frameCount == 0 || firstFrame + frameCount > availableFrames)
        frameCount = (uint16)(availableFrames - firstFrame);
    gFp.gag.frameCount = frameCount ? frameCount : 1;
    gFp.gag.flip = flip;
    gFp.gag.loop = 0;
    gFp.mode = mode;
    gFp.modeTimer = ttl;
    if (soundId >= 0)
        soundPlay(soundId);
}

static void fpStartFishing(void)
{
    int rightSide = (gFp.x >= FP_FISH_RIGHT_SIDE_X);
    sint16 fx = rightSide ? (sint16)(gFp.x - 118) : (sint16)(gFp.x - 28);
    sint16 fy = (sint16)(gFp.y - 42);

    if (gFp.y < 260)
        fy = 196;

    gFp.face = rightSide ? FP_FACE_W : FP_FACE_E;
    fpSetBanner("FISHING", 90);
    captionsShowText(kFpCaptionFish, 150);
    fpStartAssetModeEx("MJFISH1.BMP", 115, FP_MODE_FISH,
                       fx, fy, 0, 19, 6, rightSide ? 1 : 0, 4);
    gFp.fishingPhase = 1;
}

static void fpStartFishingScratch(void)
{
    int rightSide = (gFp.x >= FP_FISH_RIGHT_SIDE_X);
    sint16 fx = rightSide ? (sint16)(gFp.x - 88) : (sint16)(gFp.x - 20);
    sint16 fy = (sint16)(gFp.y - 32);

    if (gFp.y < 260)
        fy = 206;

    fpStartAssetModeEx("MJFISH2.BMP", 90, FP_MODE_FISH,
                       fx, fy, 10, 5, 8, rightSide ? 1 : 0, -1);
    gFp.fishingPhase = 2;
}

static void fpStartSummon(enum TFpSummon kind)
{
    const char *bmp = "GJGULL1.BMP";
    sint16 x = 452, y = 154, vx = 0, vy = 0;
    uint16 ttl = 420;
    uint16 delay = 10;
    uint16 frameCount = 1;
    uint8 screenRelative = 0;
    int soundId = -1;

    switch (kind) {
    case FP_SUMMON_SEAGULL:  bmp = "GJGULL1.BMP"; x = 452; y = 154; ttl = 420; soundId = 4; break;
    case FP_SUMMON_LILIPUTS: bmp = "LILIPUTS.BMP"; x = 255; y = 292; ttl = 480; frameCount = 4; break;
    case FP_SUMMON_BIPLANE:  bmp = "GJBIPLAN.BMP"; x = 420; y = 128; ttl = 300; break;
    case FP_SUMMON_CANOE:    bmp = "GJNAT1.BMP"; x = 506; y = 292; ttl = 420; gFp.fridayCount++; break;
    case FP_SUMMON_BOAT:     bmp = "BOAT.BMP"; x = 505; y = 276; ttl = 420; soundId = 16; break;
    case FP_SUMMON_KINGKONG: bmp = "GJKINGKO.BMP"; x = 265; y = 120; ttl = 360; soundId = 5; break;
    case FP_SUMMON_MARY:     bmp = "MJBATH.BMP"; x = 255; y = 300; ttl = 420; frameCount = 6; break;
    case FP_SUMMON_PIRATE:   bmp = "FISHMAN.BMP"; x = 500; y = 235; ttl = 360; break;
    case FP_SUMMON_FLOCK:    bmp = "GJGULL3.BMP"; x = 444; y = 150; ttl = 360; frameCount = 3; break;
    case FP_SUMMON_MEANWHILE:bmp = "MEANWHIL.BMP"; x = 270; y = 120; ttl = 90; delay = 10; break;
    case FP_SUMMON_CLOUD:    bmp = "CLOUDS.BMP"; x = 420; y = 96; ttl = 480; break;
    default: break;
    }

    fpCancelAsset(&gFp.gag);
    fpCancelAsset(&gFp.summon);
    gFp.fishingPhase = 0;
    if (!fpLoadSlot(FP_SLOT_SUMMON, bmp)) {
        fpSetBanner("SUMMON SKIPPED", 60);
        return;
    }
    strncpy(gFp.summon.bmp, bmp, sizeof(gFp.summon.bmp) - 1);
    gFp.summon.slot = FP_SLOT_SUMMON;
    gFp.summon.active = 1;
    gFp.summon.x = x;
    gFp.summon.y = y;
    gFp.summon.vx = vx;
    gFp.summon.vy = vy;
    gFp.summon.ttl = ttl;
    gFp.summon.frameDelay = delay;
    gFp.summon.firstFrame = 0;
    gFp.summon.frameCount = frameCount;
    gFp.summon.loop = 1;
    gFp.summon.screenRelative = screenRelative;
    gFp.summonCount++;
    captionsShowText(kFpCaptionSummon, 180);
    if (soundId >= 0)
        soundPlay(soundId);

    if (kind == FP_SUMMON_MARY && gFp.fridayCount >= 3)
        fpSetBanner("FRIDAY REMEMBERS", 150);
    else if (kind == FP_SUMMON_KINGKONG)
        fpSetBanner("KING KONG!", 120);
}

static int fpNextHolidayId(int current)
{
    int maxId = holidayMaxId();
    int next = current;
    for (int i = 0; i <= maxId; i++) {
        next++;
        if (next > maxId)
            next = 0;
        if (next == 0 || holidayById(next) != NULL)
            return next;
    }
    return 0;
}

static int fpApplyMenuWorldOverrides(void)
{
    int changed = 0;
    int desiredNight = islandState.night;
    int desiredLowTide = islandState.lowTide;
    int desiredRaft = islandState.raft;
    int desiredHoliday = islandState.holiday;
    int desiredX = islandState.xPos;
    int desiredY = islandState.yPos;

    if (hostForcedNight >= 0) {
        desiredNight = hostForcedNight;
    } else if (ps1SoftTimeEnabled) {
        desiredNight = (ps1SoftHour < 6 || ps1SoftHour >= 20) ? 1 : 0;
    }

    if (hostForcedLowTide >= 0)
        desiredLowTide = hostForcedLowTide;
    if (hostForcedRaftStage >= 0)
        desiredRaft = hostForcedRaftStage;

    if (hostForcedHoliday >= 0) {
        desiredHoliday = hostForcedHoliday;
    } else if (ps1SoftTimeEnabled) {
        desiredHoliday = holidayForDate(ps1SoftYear, ps1SoftMonth, ps1SoftDay);
    }

    if (hostForcedIslandPosValid) {
        desiredX = hostForcedIslandX;
        desiredY = hostForcedIslandY;
    }

    if (islandState.night != desiredNight) {
        islandState.night = desiredNight;
        changed = 1;
    }
    if (islandState.lowTide != desiredLowTide) {
        islandState.lowTide = desiredLowTide;
        changed = 1;
    }
    if (islandState.raft != desiredRaft) {
        islandState.raft = desiredRaft;
        changed = 1;
    }
    if (islandState.holiday != desiredHoliday) {
        islandState.holiday = desiredHoliday;
        changed = 1;
    }
    if (islandState.xPos != desiredX || islandState.yPos != desiredY) {
        islandState.xPos = (sint16)desiredX;
        islandState.yPos = (sint16)desiredY;
        changed = 1;
    }

    if (changed) {
        fpCancelTransientAction();
        fpSetBanner("WORLD UPDATED", 90);
        grShowMeanwhileLoadingFrame((uint16)(gFp.frame & 7UL));
        if (!fpRebuildBackdrop())
            gFreeplayExitRequested = 1;
    }

    return changed;
}

static void fpHandleEnvironment(uint16 pressed)
{
    if (pressed & PAD_UP) {
        islandState.night = !islandState.night;
        fpSetBanner(islandState.night ? "NIGHT" : "DAY", 90);
        fpRebuildBackdrop();
    } else if (pressed & PAD_DOWN) {
        islandState.lowTide = !islandState.lowTide;
        fpSetBanner(islandState.lowTide ? "LOW TIDE" : "HIGH TIDE", 90);
        fpRebuildBackdrop();
    } else if (pressed & PAD_RIGHT) {
        islandState.holiday = fpNextHolidayId(islandState.holiday);
        fpSetBanner(islandState.holiday ? "HOLIDAY" : "NO HOLIDAY", 90);
        fpRebuildBackdrop();
    } else if (pressed & PAD_LEFT) {
        islandState.raft++;
        if (islandState.raft > 5)
            islandState.raft = 0;
        fpSetBanner("RAFT STAGE", 90);
        fpRebuildBackdrop();
    }
}

static void fpOpenPauseMenu(void)
{
    captionsShowText(kFpCaptionPause, 120);
    pauseMenuShow();
    while (pauseMenuIsVisible()) {
        pauseMenuUpdate();
    }
    if (pauseMenuRequestExitFreeplay ||
        pauseMenuRequestNextScene ||
        pauseMenuRequestResetLoop) {
        pauseMenuRequestExitFreeplay = 0;
        pauseMenuRequestFreeplayWorldRefresh = 0;
        gFreeplayExitRequested = 1;
    } else {
        int gagRequest = pauseMenuRequestFreeplayGag;
        int visitorRequest = pauseMenuRequestFreeplayVisitor;
        int clearRequest = pauseMenuRequestFreeplayClear;
        int worldRequest = pauseMenuRequestFreeplayWorldRefresh;

        pauseMenuRequestFreeplayGag = -1;
        pauseMenuRequestFreeplayVisitor = -1;
        pauseMenuRequestFreeplayClear = 0;
        pauseMenuRequestFreeplayWorldRefresh = 0;

        if (worldRequest) {
            fpApplyMenuWorldOverrides();
        } else if (clearRequest) {
            fpClearScreen();
        } else if (gagRequest >= 0 && gagRequest < FP_GAG_COUNT) {
            fpStartGag((enum TFpGag)gagRequest);
        } else if (visitorRequest >= 0 && visitorRequest < FP_SUMMON_COUNT) {
            fpStartSummon((enum TFpSummon)visitorRequest);
        }
    }
    gFp.prevButtons = 0xffff;
    grForceFullRedrawNextFrame();
}

static void fpApplyInput(uint16 cur, uint16 pressed)
{
    int dx = 0;
    int dy = 0;

    if (cur)
        gFp.inputFrames++;

    if (pressed & PAD_START) {
        fpOpenPauseMenu();
        return;
    }

    if (cur & PAD_R1) {
        fpHandleEnvironment(pressed);
        return;
    }

    if (cur & PAD_LEFT)      { dx = -1; gFp.face = FP_FACE_W; }
    else if (cur & PAD_RIGHT){ dx =  1; gFp.face = FP_FACE_E; }
    else if (cur & PAD_UP)   { dy = -1; gFp.face = FP_FACE_N; }
    else if (cur & PAD_DOWN) { dy =  1; gFp.face = FP_FACE_S; }

    if (dx || dy) {
        int speed = 1;
        if (cur & PAD_R2)
            speed = 2;
        else if (cur & PAD_L2)
            speed = (gFp.frame & 1) ? 1 : 0;
        if (gFp.gag.active || gFp.summon.active || gFp.mode != FP_MODE_WALK)
            fpCancelTransientAction();
        gFp.x = (sint16)(gFp.x + dx * speed);
        gFp.y = (sint16)(gFp.y + dy * speed);
        fpClampJohnny();
        gFp.walkFrame++;
        gFp.walkFrames++;
        gFp.idleFrames = 0;
        gFp.mode = FP_MODE_WALK;
        return;
    } else if (gFp.mode == FP_MODE_WALK) {
        gFp.mode = FP_MODE_IDLE;
    }

    if (pressed & PAD_SELECT) {
        fpClearScreen();
        return;
    }

    if (pressed & PAD_CIRCLE) {
        fpStartFishing();
        return;
    }
}

static void fpTickAsset(struct TFpAsset *asset)
{
    if (!asset->active)
        return;
    if (asset->ttl > 0)
        asset->ttl--;
    if (asset->ttl == 0) {
        fpCancelAsset(asset);
        return;
    }
    asset->x = (sint16)(asset->x + asset->vx);
    asset->y = (sint16)(asset->y + asset->vy);
    asset->frameTimer++;
    if (asset->frameTimer >= asset->frameDelay) {
        asset->frameTimer = 0;
        if (asset->frameCount > 1) {
            if (asset->loop)
                asset->frame = (uint16)((asset->frame + 1) % asset->frameCount);
            else if (asset->frame + 1 < asset->frameCount)
                asset->frame++;
        }
    }
}

static void fpTick(void)
{
    fpTickAsset(&gFp.gag);
    if (gFp.fishingPhase == 1 && !gFp.gag.active) {
        fpStartFishingScratch();
    } else if (gFp.fishingPhase == 2 && !gFp.gag.active) {
        gFp.fishingPhase = 0;
    }
    fpTickAsset(&gFp.summon);

    if (gFp.modeTimer > 0) {
        gFp.modeTimer--;
        if (gFp.modeTimer == 0)
            gFp.mode = FP_MODE_IDLE;
    }
    if (gFp.bannerTimer > 0)
        gFp.bannerTimer--;

    if (gFp.mode == FP_MODE_IDLE)
        gFp.idleFrames++;

    if (gFp.fireActive) {
        gFp.fireTimer++;
        if (gFp.fireTimer >= 6) {
            gFp.fireTimer = 0;
            gFp.fireFrame++;
        }
    }

}

static void fpDrawAsset(const struct TFpAsset *asset)
{
    uint16 drawFrame;
    int slotFrames;

    if (!asset->active)
        return;
    if (asset->slot >= MAX_BMP_SLOTS)
        return;
    slotFrames = gFpSlot.numSprites[asset->slot];
    if (slotFrames <= 0)
        return;
    drawFrame = (uint16)(asset->firstFrame + asset->frame);
    if (drawFrame >= (uint16)slotFrames)
        drawFrame = (uint16)(slotFrames - 1);

    if (asset->screenRelative)
        fpSetScreenDrawOffset();
    else
        fpSetIslandDrawOffset();
    if (asset->flip)
        grDrawSpriteFlip(grBackgroundSfc, &gFpSlot,
                         asset->x, asset->y,
                         drawFrame, asset->slot);
    else
        grDrawSprite(grBackgroundSfc, &gFpSlot,
                     asset->x, asset->y,
                     drawFrame, asset->slot);
}

static void fpDrawFire(void)
{
    char bmp[12];
    if (!gFp.fireActive)
        return;
    snprintf(bmp, sizeof(bmp), "FIRE%u.BMP", (unsigned)((gFp.fireFrame % 5) + 1));
    if (!fpLoadSlot(FP_SLOT_PERSIST, bmp))
        return;
    fpSetIslandDrawOffset();
    grDrawSprite(grBackgroundSfc, &gFpSlot,
                 gFp.fireX, gFp.fireY, 0, FP_SLOT_PERSIST);
}

static void fpDrawJohnny(void)
{
    if (gFp.gag.active) {
        fpDrawAsset(&gFp.gag);
        fpRestampTreeIfNeeded();
        return;
    }

    fpSetIslandDrawOffset();
    if (gFp.mode == FP_MODE_WALK || gFp.mode == FP_MODE_STRUT) {
        int flip = (gFp.face == FP_FACE_W) ? 1 : 0;
        walkRenderFrame(grBackgroundSfc, &gFpSlot, fgBackdropGetSlot(),
                        gFp.x, gFp.y, fpWalkFrame(), flip,
                        fpBehindTree(gFp.x, gFp.y), 0);
    } else {
        int flip = (gFp.face == FP_FACE_W) ? 1 : 0;
        if (flip)
            grDrawSpriteFlip(grBackgroundSfc, &gFpSlot,
                             gFp.x, gFp.y, fpIdleFrame(), FP_SLOT_JOHN);
        else
            grDrawSprite(grBackgroundSfc, &gFpSlot,
                         gFp.x, gFp.y, fpIdleFrame(), FP_SLOT_JOHN);
        fpRestampTreeIfNeeded();
    }
}

static void fpTextStart(int x, int y)
{
    fpTextX = x;
    fpTextY = y;
}

static void fpTextChar(char c)
{
    int idx;
    int col;
    int row;
    SPRT *sprt;

    if (c == '\n') {
        fpTextX = 24;
        fpTextY += PAUSE_GLYPH_DRAW_H + 2;
        return;
    }
    if (c < PAUSE_GLYPH_FIRST ||
        c >= (PAUSE_GLYPH_FIRST + PAUSE_GLYPH_COUNT)) {
        c = '?';
    }
    if (fpOverlayNext + sizeof(SPRT) >= fpOverlayPrim + FP_OVERLAY_PRIM_BYTES)
        return;

    idx = c - PAUSE_GLYPH_FIRST;
    col = idx & 15;
    row = idx >> 4;
    sprt = (SPRT *)fpOverlayNext;
    fpOverlayNext += sizeof(SPRT);
    setSprt(sprt);
    setXY0(sprt, fpTextX, fpTextY);
    setWH(sprt, PAUSE_GLYPH_DRAW_W, PAUSE_GLYPH_DRAW_H);
    setUV0(sprt, (uint8)(col * PAUSE_GLYPH_DRAW_W),
           (uint8)(row * PAUSE_GLYPH_DRAW_H));
    setClut(sprt, PAUSE_CLUT_VRAM_X, PAUSE_CLUT_VRAM_Y);
    setRGB0(sprt, 128, 128, 128);
    ps1GpuOtAddPrim(fpOverlayTextOt, sprt);
    fpTextX += PAUSE_GLYPH_DRAW_W;
}

static void fpText(const char *text)
{
    while (text && *text)
        fpTextChar(*text++);
}

static void fpBuildOverlay(void)
{
    DR_TPAGE *tp;
    POLY_F4 *panel;

    ClearOTagR(fpOverlayOt, FP_OVERLAY_OT_LEN);
    fpOverlayNext = fpOverlayPrim;
    fpOverlayTextOt = &fpOverlayOt[FP_OVERLAY_OT_LEN - 4];

    tp = (DR_TPAGE *)fpOverlayNext;
    fpOverlayNext += sizeof(DR_TPAGE);
    setDrawTPage(tp, 0, 1, getTPage(0, 0, PAUSE_FONT_VRAM_X, PAUSE_FONT_VRAM_Y));
    ps1GpuOtAddPrim(&fpOverlayOt[FP_OVERLAY_OT_LEN - 1], tp);

    if (gFp.bannerTimer > 0) {
        panel = (POLY_F4 *)fpOverlayNext;
        fpOverlayNext += sizeof(POLY_F4);
        setPolyF4(panel);
        setSemiTrans(panel, 1);
        setRGB0(panel, 0, 0, 0);
        setXY4(panel,
               FP_BANNER_X0, FP_BANNER_Y0,
               FP_BANNER_X1, FP_BANNER_Y0,
               FP_BANNER_X0, FP_BANNER_Y1,
               FP_BANNER_X1, FP_BANNER_Y1);
        ps1GpuOtAddPrim(&fpOverlayOt[FP_OVERLAY_OT_LEN - 3], panel);
        fpTextStart(210, 38);
        fpText(gFp.banner);
    }

    if (gFreeplayTelemetryLevel >= 3) {
        char line[64];
        snprintf(line, sizeof(line), "FP %s X%d Y%d R%lu A%lu",
                 fpModeName(gFp.mode), (int)gFp.x, (int)gFp.y,
                 (unsigned long)gFp.cleanRebuilds,
                 (unsigned long)gFp.assetLoadFailures);
        fpTextStart(12, 204);
        fpText(line);
    }
}

static int fpOverlayVisible(void)
{
    return (gFp.bannerTimer > 0 ||
            gFreeplayTelemetryLevel >= 3 ||
            captionsIsVisible()) ? 1 : 0;
}

static void fpMarkOverlayDirty(void)
{
    int visible = fpOverlayVisible();
    if (visible || gFp.overlayWasVisible) {
        grMarkScreenRectDirty(FP_BANNER_X0 - 4, FP_BANNER_Y0 - 4,
                              FP_BANNER_X1 + 4, FP_BANNER_Y1 + 4);
        grMarkScreenRectDirty(FP_CAPTION_X0, FP_CAPTION_Y0,
                              FP_CAPTION_X1, FP_CAPTION_Y1);
    }
    gFp.overlayWasVisible = (uint8)visible;
}

static void fpPresent(void)
{
    ps1FrameCount++;
    VSync(0);
    grDrawBackground();
    if (gFp.bannerTimer > 0 || gFreeplayTelemetryLevel >= 3) {
        pauseMenuEnsureFontUploaded();
        fpBuildOverlay();
        DrawOTag(&fpOverlayOt[FP_OVERLAY_OT_LEN - 1]);
    }
    if (ps1CaptionsEnabled)
        captionsRender();
    DrawSync(0);
}

static void fpFrame(void)
{
    grBeginFrame();
    grRestoreBgFromRects();
    if ((gFp.frame & 3UL) == 0)
        fgBackdropTickWavesPublic();
    fpDrawFire();
    fpDrawAsset(&gFp.summon);
    fpDrawJohnny();
    fgBackdropStampHolidayPublic();
    fpMarkOverlayDirty();
    fpPresent();
}

static void fpInitState(void)
{
    memset(&gFp, 0, sizeof(gFp));
    gFp.x = 392;
    gFp.y = 238;
    gFp.face = FP_FACE_E;
    gFp.mode = FP_MODE_IDLE;
    gFp.prevButtons = 0;
    gFp.ambientNextFrame = 0xffffffffUL;
    gFp.lastGag = FP_GAG_BONK;
    fpSetBanner("FREEPLAY", 120);
}

void freeplayRun(void)
{
    uint16 cur;
    uint16 pressed;

    gFreeplayExitRequested = 0;
    pauseMenuRequestExitFreeplay = 0;
    pauseMenuRequestFreeplayGag = -1;
    pauseMenuRequestFreeplayVisitor = -1;
    pauseMenuRequestFreeplayClear = 0;
    pauseMenuRequestFreeplayWorldRefresh = 0;
    pauseMenuSetFreeplayActive(1);
    fpInitState();
    memset(&gFpSlot, 0, sizeof(gFpSlot));
    captionsShowText(kFpCaptionStart, 240);

    FP_LOG(1, ("JCFREE start heapKB=%lu night=%d low=%d raft=%d holiday=%d\n",
               fgProbeLargestAlloc() / 1024UL,
               islandState.night, islandState.lowTide,
               islandState.raft, islandState.holiday));

    FP_LOG(1, ("JCFREE boot backdrop-first heapKB=%lu\n",
               fgProbeLargestAlloc() / 1024UL));
    FP_LOG(1, ("JCFREE rebuild backdrop begin heapKB=%lu\n",
               fgProbeLargestAlloc() / 1024UL));
    if (!fpRebuildBackdrop()) {
        gFreeplayExitRequested = 1;
        fpReleaseSlots();
        grFreeCleanBgRects();
        fgBackdropReleasePublic(1);
        captionsClear();
        pauseMenuSetFreeplayActive(0);
        return;
    }
    FP_LOG(1, ("JCFREE backdrop ready heapKB=%lu rectKB=%lu\n",
               fgProbeLargestAlloc() / 1024UL,
               grCleanBgRectsBytes() / 1024UL));
    FP_LOG(1, ("JCFREE rebuild backdrop done heapKB=%lu rectKB=%lu\n",
               fgProbeLargestAlloc() / 1024UL,
               grCleanBgRectsBytes() / 1024UL));

    FP_LOG(1, ("JCFREE load johnwalk heapKB=%lu\n",
               fgProbeLargestAlloc() / 1024UL));
    if (!fpLoadSlot(FP_SLOT_JOHN, "JOHNWALK.BMP")) {
        FP_LOG(1, ("JCFREE fatal missing JOHNWALK\n"));
        gFreeplayExitRequested = 1;
        fpReleaseSlots();
        grFreeCleanBgRects();
        fgBackdropReleasePublic(1);
        captionsClear();
        pauseMenuSetFreeplayActive(0);
        return;
    }
    FP_LOG(1, ("JCFREE johnwalk loaded heapKB=%lu\n",
               fgProbeLargestAlloc() / 1024UL));

    grForceFullRedrawNextFrame();
    gFp.prevButtons = fpReadButtons();

    while (!gFreeplayExitRequested) {
        cur = fpReadButtons();
        pressed = (uint16)(cur & ~gFp.prevButtons);
        gFp.prevButtons = cur;

        fpApplyInput(cur, pressed);
        fpTick();
        fpFrame();

        gFp.frame++;
        if (gFreeplayTelemetryLevel >= 1 && (gFp.frame % 300U) == 0) {
            FP_LOG(1,
                   ("JCFREE summary frame=%lu mode=%s x=%d y=%d walk=%lu action=%lu summon=%lu clean=%lu cleanFail=%lu assetFail=%lu rectKB=%lu heapKB=%lu\n",
                    (unsigned long)gFp.frame,
                    fpModeName(gFp.mode),
                    (int)gFp.x, (int)gFp.y,
                    (unsigned long)gFp.walkFrames,
                    (unsigned long)gFp.actionCount,
                    (unsigned long)gFp.summonCount,
                    (unsigned long)gFp.cleanRebuilds,
                    (unsigned long)gFp.cleanFailures,
                    (unsigned long)gFp.assetLoadFailures,
                    grCleanBgRectsBytes() / 1024UL,
                    fgProbeLargestAlloc() / 1024UL));
        }
    }

    while (fpReadButtons() & PAD_START)
        VSync(0);

    FP_LOG(1,
           ("JCFREE end frame=%lu walk=%lu action=%lu summon=%lu clean=%lu cleanFail=%lu assetFail=%lu heapKB=%lu\n",
            (unsigned long)gFp.frame,
            (unsigned long)gFp.walkFrames,
            (unsigned long)gFp.actionCount,
            (unsigned long)gFp.summonCount,
            (unsigned long)gFp.cleanRebuilds,
            (unsigned long)gFp.cleanFailures,
            (unsigned long)gFp.assetLoadFailures,
            fgProbeLargestAlloc() / 1024UL));

    fpReleaseSlots();
    grFreeCleanBgRects();
    fgBackdropReleasePublic(1);
    walkRenderResetCache();
    captionsClear();
    grShowMeanwhileLoadingFrame((uint16)(gFp.frame & 7UL));
    pauseMenuSetFreeplayActive(0);
    grForceFullRedrawNextFrame();
}

int freeplayExitRequested(void)
{
    return gFreeplayExitRequested;
}

void freeplayClearExitRequest(void)
{
    gFreeplayExitRequested = 0;
}

void freeplaySetTelemetryLevel(int level)
{
    if (level < 0) level = 0;
    if (level > 3) level = 3;
    gFreeplayTelemetryLevel = level;
}

int freeplayGetTelemetryLevel(void)
{
    return gFreeplayTelemetryLevel;
}

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
