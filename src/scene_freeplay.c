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
#include "ps1_gpu_ot.h"
#include "sound_ps1.h"
#include "walk_render.h"
#include "freeplay_sprite_indices.h"

extern struct TIslandState islandState;
extern uint8 pad_buff[2][34];
extern uint32 ps1FrameCount;
extern int grDx;
extern int grDy;

#define FP_SLOT_JOHN       0
#define FP_SLOT_GAG        1
#define FP_SLOT_SUMMON     2
#define FP_SLOT_PERSIST    3

#define FP_CLEAN_RECT_COUNT 3
#define FP_OVERLAY_OT_LEN  8
#define FP_OVERLAY_PRIM_BYTES 24576
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
    FP_MODE_DIVE,
    FP_MODE_BUILD,
    FP_MODE_ONESHOT,
    FP_MODE_STRUT,
    FP_MODE_RUNAWAY,
    FP_MODE_SLEEP
};

enum TFpGag {
    FP_GAG_EAT = 0,
    FP_GAG_HOT,
    FP_GAG_IDEA,
    FP_GAG_ANGRY,
    FP_GAG_BONK,
    FP_GAG_DRUNK,
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
    uint8 flip;
    uint8 screenRelative;
};

struct TFpPersistent {
    uint8 active;
    uint8 kind;
    uint8 frame;
    sint16 x;
    sint16 y;
};

struct TFpState {
    sint16 x;
    sint16 y;
    enum TFpFace face;
    enum TFpMode mode;
    uint16 modeTimer;
    uint16 walkFrame;
    uint16 walkStepCounter;
    uint8 drunk;
    uint8 gagCursor;
    uint8 lastGag;
    uint8 summonCursor;
    uint8 helpVisible;
    uint16 helpTimer;
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
    uint32 walkingAccumFrames;
    uint32 idleFrames;
    uint32 cinematicLockUntil;
    uint32 konamiFirstFrame;
    uint16 prevButtons;
    uint16 tornadoHoldFrames;
    uint8 konamiLen;
    uint8 konami[10];
    uint8 fridayCount;
    uint8 castleStage;
    uint8 fireActive;
    sint16 fireX;
    sint16 fireY;
    uint8 fireFrame;
    uint8 fireTimer;
    struct TFpAsset gag;
    struct TFpAsset summon;
    struct TFpPersistent persist[16];
};

static struct TTtmSlot gFpSlot;
static struct TFpState gFp;
static int gFreeplayExitRequested = 0;
static int gFreeplayTelemetryLevel = 0;

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
    "DRUNKJON.BMP",
    "MEXCWALK.BMP",
    "GJRUNAWA.BMP"
};

static const uint16 kFpGagDurations[FP_GAG_COUNT] = {
    90, 90, 120, 90, 75, 90, 300, 180
};

static const uint8 kKonamiSeq[10] = {
    1, 1, 2, 2, 3, 4, 3, 4, 5, 6
};

static const char *fpModeName(enum TFpMode mode)
{
    switch (mode) {
    case FP_MODE_IDLE: return "idle";
    case FP_MODE_WALK: return "walk";
    case FP_MODE_FISH: return "fish";
    case FP_MODE_DIVE: return "dive";
    case FP_MODE_BUILD: return "build";
    case FP_MODE_ONESHOT: return "oneshot";
    case FP_MODE_STRUT: return "strut";
    case FP_MODE_RUNAWAY: return "runaway";
    case FP_MODE_SLEEP: return "sleep";
    default: return "?";
    }
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
    return (x >= 420 && x <= 488 && y >= 210 && y <= 265) ? 1 : 0;
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
    sint16 xs[FP_CLEAN_RECT_COUNT];
    sint16 ys[FP_CLEAN_RECT_COUNT];
    uint16 ws[FP_CLEAN_RECT_COUNT];
    uint16 hs[FP_CLEAN_RECT_COUNT];
    int rc;

    xs[0] = 40;  ys[0] = 48;  ws[0] = 560; hs[0] = 150; /* sky summons */
    xs[1] = 220; ys[1] = 180; ws[1] = 370; hs[1] = 170; /* island + Johnny */
    xs[2] = 129; ys[2] = 303; ws[2] = 479; hs[2] =  53; /* surf animation */

    rc = grSaveCleanBgRects(xs, ys, ws, hs, FP_CLEAN_RECT_COUNT);
    if (rc != FP_CLEAN_RECT_COUNT) {
        gFp.cleanFailures++;
        FP_LOG(1, ("JCFREE clean-fail rc=%d need=%d failures=%lu heapKB=%lu\n",
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
    fpSetIslandDrawOffset();

    if (gFp.fireActive) {
        /* Fire is animated, not baked into the baseline. */
    }

    if (fpLoadSlot(FP_SLOT_PERSIST, "GJCASTLE.BMP")) {
        for (int i = 0; i < (int)(sizeof(gFp.persist) / sizeof(gFp.persist[0])); i++) {
            if (!gFp.persist[i].active || gFp.persist[i].kind != FP_PERSIST_CASTLE)
                continue;
            grDrawSprite(grBackgroundSfc, &gFpSlot,
                         gFp.persist[i].x, gFp.persist[i].y,
                         gFp.persist[i].frame, FP_SLOT_PERSIST);
        }
    }

    if (fpLoadSlot(FP_SLOT_PERSIST, "COCONUTS.BMP")) {
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
        buttons |= (uint16)(~pad->btn);
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

static int fpAddPersistent(uint8 kind, sint16 x, sint16 y, uint8 frame)
{
    for (int i = 0; i < (int)(sizeof(gFp.persist) / sizeof(gFp.persist[0])); i++) {
        if (gFp.persist[i].active)
            continue;
        gFp.persist[i].active = 1;
        gFp.persist[i].kind = kind;
        gFp.persist[i].x = x;
        gFp.persist[i].y = y;
        gFp.persist[i].frame = frame;
        fpRebuildBackdrop();
        return 1;
    }
    fpSetBanner("ISLAND IS FULL", 90);
    return 0;
}

static int fpNear(sint16 x, sint16 y, sint16 tx, sint16 ty, sint16 r)
{
    sint16 dx = (sint16)(x - tx);
    sint16 dy = (sint16)(y - ty);
    return ((int)dx * (int)dx + (int)dy * (int)dy) <= ((int)r * (int)r);
}

static int fpAtFishingShore(void)
{
    return ((gFp.x >= 240 && gFp.x <= 295 && gFp.y >= 275 && gFp.y <= 310) ||
            (gFp.x >= 505 && gFp.x <= 545 && gFp.y >= 275 && gFp.y <= 310));
}

static int fpAtWater(void)
{
    return (gFp.x >= 240 && gFp.x <= 545 && gFp.y >= 305 && gFp.y <= 330);
}

static int fpAtBuildZone(void)
{
    return (gFp.x >= 310 && gFp.x <= 470 && gFp.y >= 235 && gFp.y <= 292);
}

static int fpAtPalm(void)
{
    return (gFp.x >= 420 && gFp.x <= 490 && gFp.y >= 210 && gFp.y <= 265);
}

static void fpStartGag(enum TFpGag gag)
{
    gFp.actionCount++;
    gFp.lastGag = (uint8)gag;

    if (gag == FP_GAG_DRUNK) {
        gFp.drunk = !gFp.drunk;
        fpSetBanner(gFp.drunk ? "DRUNK WALK ON" : "DRUNK WALK OFF", 90);
        soundPlay(5);
        return;
    }

    memset(&gFp.gag, 0, sizeof(gFp.gag));
    if (!fpLoadSlot(FP_SLOT_GAG, kFpGagBmps[gag])) {
        fpSetBanner("GAG SKIPPED", 60);
        return;
    }
    strncpy(gFp.gag.bmp, kFpGagBmps[gag], sizeof(gFp.gag.bmp) - 1);
    gFp.gag.slot = FP_SLOT_GAG;
    gFp.gag.active = 1;
    gFp.gag.frameDelay = 4;
    gFp.gag.ttl = kFpGagDurations[gag];
    gFp.gag.x = gFp.x;
    gFp.gag.y = (sint16)(gFp.y - ((gag == FP_GAG_IDEA) ? 24 : 0));
    gFp.gag.screenRelative = 0;
    gFp.mode = (gag == FP_GAG_STRUT) ? FP_MODE_STRUT :
               (gag == FP_GAG_RUNAWAY) ? FP_MODE_RUNAWAY :
               FP_MODE_ONESHOT;
    gFp.modeTimer = gFp.gag.ttl;

    if (gag == FP_GAG_BONK) soundPlay(5);
    else if (gag == FP_GAG_EAT) soundPlay(8);
    else if (gag == FP_GAG_HOT) soundPlay(4);
}

static void fpStartAssetMode(const char *bmp, uint16 ttl, enum TFpMode mode,
                             sint16 x, sint16 y, int soundId)
{
    gFp.actionCount++;
    memset(&gFp.gag, 0, sizeof(gFp.gag));
    if (!fpLoadSlot(FP_SLOT_GAG, bmp)) {
        fpSetBanner("ACTION SKIPPED", 60);
        return;
    }
    strncpy(gFp.gag.bmp, bmp, sizeof(gFp.gag.bmp) - 1);
    gFp.gag.slot = FP_SLOT_GAG;
    gFp.gag.active = 1;
    gFp.gag.frameDelay = 4;
    gFp.gag.ttl = ttl;
    gFp.gag.x = x;
    gFp.gag.y = y;
    gFp.mode = mode;
    gFp.modeTimer = ttl;
    if (soundId >= 0)
        soundPlay(soundId);
}

static void fpStartSummon(enum TFpSummon kind)
{
    const char *bmp = "GJGULL1.BMP";
    sint16 x = 640, y = 120, vx = -2, vy = 0;
    uint16 ttl = 360;
    uint16 delay = 5;
    uint8 screenRelative = 1;
    int soundId = -1;

    switch (kind) {
    case FP_SUMMON_SEAGULL:  bmp = "GJGULL1.BMP"; x = 620; y = 125; vx = -2; ttl = 360; soundId = 4; break;
    case FP_SUMMON_LILIPUTS: bmp = "LILIPUTS.BMP"; x = 245; y = 292; vx = 1; ttl = 480; screenRelative = 0; break;
    case FP_SUMMON_BIPLANE:  bmp = "GJBIPLAN.BMP"; x = 640; y = 68; vx = -3; ttl = 360; break;
    case FP_SUMMON_CANOE:    bmp = "GJNAT1.BMP"; x = 600; y = 292; vx = -1; ttl = 600; screenRelative = 0; gFp.fridayCount++; break;
    case FP_SUMMON_BOAT:     bmp = "BOAT.BMP"; x = 560; y = 268; vx = -1; ttl = 540; screenRelative = 0; soundId = 16; break;
    case FP_SUMMON_KINGKONG: bmp = "GJKINGKO.BMP"; x = 90; y = 92; vx = 0; ttl = 480; soundId = 5; break;
    case FP_SUMMON_MARY:     bmp = "MJBATH.BMP"; x = 255; y = 300; vx = 0; ttl = 600; screenRelative = 0; break;
    case FP_SUMMON_PIRATE:   bmp = "FISHMAN.BMP"; x = 560; y = 220; vx = -1; ttl = 420; screenRelative = 0; break;
    case FP_SUMMON_FLOCK:    bmp = "GJGULL3.BMP"; x = 620; y = 96; vx = -2; ttl = 600; break;
    case FP_SUMMON_MEANWHILE:bmp = "MEANWHIL.BMP"; x = 270; y = 120; vx = 0; ttl = 90; delay = 3; break;
    case FP_SUMMON_CLOUD:    bmp = "CLOUDS.BMP"; x = 640; y = 88; vx = -1; ttl = 900; break;
    default: break;
    }

    memset(&gFp.summon, 0, sizeof(gFp.summon));
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
    gFp.summon.screenRelative = screenRelative;
    gFp.summonCount++;
    if (soundId >= 0)
        soundPlay(soundId);

    if (kind == FP_SUMMON_MARY && gFp.fridayCount >= 3)
        fpSetBanner("FRIDAY REMEMBERS", 150);
    else if (kind == FP_SUMMON_KINGKONG)
        fpSetBanner("KING KONG!", 120);
}

static void fpTriggerCarnival(void)
{
    fpSetBanner("SECRET FOUND", 240);
    gFp.cinematicLockUntil = gFp.frame + 420;
    fpStartSummon(FP_SUMMON_KINGKONG);
    fpStartGag(FP_GAG_STRUT);
    soundPlay(16);
}

static void fpPushKonami(uint8 code)
{
    if (gFp.konamiLen == 0)
        gFp.konamiFirstFrame = gFp.frame;
    if (gFp.frame - gFp.konamiFirstFrame > 240) {
        gFp.konamiLen = 0;
        gFp.konamiFirstFrame = gFp.frame;
    }
    if (gFp.konamiLen < sizeof(gFp.konami)) {
        gFp.konami[gFp.konamiLen++] = code;
    } else {
        memmove(gFp.konami, gFp.konami + 1, sizeof(gFp.konami) - 1);
        gFp.konami[sizeof(gFp.konami) - 1] = code;
    }
    if (gFp.konamiLen >= sizeof(kKonamiSeq) &&
        memcmp(gFp.konami + gFp.konamiLen - sizeof(kKonamiSeq),
               kKonamiSeq, sizeof(kKonamiSeq)) == 0) {
        gFp.konamiLen = 0;
        fpTriggerCarnival();
    }
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

static void fpHandleContext(void)
{
    static uint32 fishWindowStart = 0;
    static uint8 fishPresses = 0;

    if (fpAtFishingShore()) {
        if (gFp.frame - fishWindowStart > 120) {
            fishWindowStart = gFp.frame;
            fishPresses = 0;
        }
        fishPresses++;
        if (fishPresses >= 7) {
            fpSetBanner("THE BIG ONE!", 180);
            fpStartAssetMode("GJCATCH3.BMP", 240, FP_MODE_FISH,
                             (sint16)(gFp.x - 16), (sint16)(gFp.y - 28), 16);
            fishPresses = 0;
        } else {
            fpStartAssetMode("GJCATCH1.BMP", 120, FP_MODE_FISH,
                             (sint16)(gFp.x - 16), (sint16)(gFp.y - 24), 4);
        }
        return;
    }

    if (fpAtWater()) {
        fpStartAssetMode("GJDIVE.BMP", 120, FP_MODE_DIVE,
                         (sint16)(gFp.x - 24), (sint16)(gFp.y - 20), 4);
        return;
    }

    if (fpAtBuildZone()) {
        gFp.castleStage++;
        if (gFp.castleStage >= 5) {
            fpAddPersistent(FP_PERSIST_CASTLE,
                            (sint16)(gFp.x - 8), (sint16)(gFp.y + 16), 4);
            gFp.castleStage = 0;
            fpSetBanner("SANDCASTLE!", 120);
            soundPlay(8);
        } else {
            fpStartAssetMode("GJCASTLE.BMP", 60, FP_MODE_BUILD,
                             (sint16)(gFp.x - 8), (sint16)(gFp.y + 12),
                             8);
        }
        return;
    }

    if (fpAtPalm()) {
        fpAddPersistent(FP_PERSIST_COCONUT,
                        (sint16)(gFp.x + 12), (sint16)(gFp.y + 48), 0);
        fpStartGag(FP_GAG_BONK);
        return;
    }

    if (gFp.fireActive && fpNear(gFp.x, gFp.y, gFp.fireX, gFp.fireY, 60)) {
        fpStartGag(FP_GAG_EAT);
        fpSetBanner("CAMPFIRE SNACK", 90);
        return;
    }

    fpStartGag(FP_GAG_IDEA);
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
    } else if (pressed & PAD_CROSS) {
        if (gFp.fireActive && fpNear(gFp.x, gFp.y, gFp.fireX, gFp.fireY, 42)) {
            gFp.fireActive = 0;
            fpSetBanner("FIRE OUT", 90);
        } else {
            gFp.fireActive = 1;
            gFp.fireX = (sint16)(gFp.x + 16);
            gFp.fireY = (sint16)(gFp.y + 28);
            fpSetBanner("FIRE LIT", 90);
        }
    } else if (pressed & PAD_SQUARE) {
        fpAddPersistent(FP_PERSIST_COCONUT,
                        (sint16)(gFp.x + 18), (sint16)(gFp.y + 42), 0);
        fpSetBanner("COCONUT", 80);
    } else if (pressed & PAD_TRIANGLE) {
        fpStartSummon(FP_SUMMON_FLOCK);
    } else if (pressed & PAD_CIRCLE) {
        fpStartSummon(FP_SUMMON_CLOUD);
    }
}

static void fpApplyInput(uint16 cur, uint16 pressed)
{
    const uint16 tornadoMask = (uint16)(PAD_L1 | PAD_R1 | PAD_L2 | PAD_R2);
    int dx = 0;
    int dy = 0;

    if (pressed & PAD_UP)       fpPushKonami(1);
    if (pressed & PAD_DOWN)     fpPushKonami(2);
    if (pressed & PAD_LEFT)     fpPushKonami(3);
    if (pressed & PAD_RIGHT)    fpPushKonami(4);
    if (pressed & PAD_SQUARE)   fpPushKonami(5);
    if (pressed & PAD_CROSS)    fpPushKonami(6);

    if (cur)
        gFp.inputFrames++;

    if ((cur & tornadoMask) == tornadoMask)
        gFp.tornadoHoldFrames++;
    else
        gFp.tornadoHoldFrames = 0;
    if (gFp.tornadoHoldFrames == 120) {
        fpSetBanner("TORNADO!", 180);
        fpTriggerCarnival();
    }

    if (gFp.frame < gFp.cinematicLockUntil)
        return;

    if (pressed & PAD_START) {
        gFreeplayExitRequested = 1;
        return;
    }

    if (pressed & PAD_SELECT) {
        gFp.helpVisible = !gFp.helpVisible;
        gFp.helpTimer = gFp.helpVisible ? 300 : 0;
    }

    if ((cur & PAD_L1) && (pressed & PAD_SQUARE)) {
        fpStartGag(FP_GAG_BONK);
        return;
    }
    if ((cur & PAD_L1) && (pressed & PAD_TRIANGLE)) {
        fpStartSummon(FP_SUMMON_MARY);
        return;
    }
    if ((cur & PAD_L1) && (pressed & PAD_CIRCLE)) {
        fpStartSummon(FP_SUMMON_KINGKONG);
        return;
    }
    if ((cur & PAD_L1) && (pressed & PAD_CROSS)) {
        fpAddPersistent(FP_PERSIST_COCONUT,
                        (sint16)(gFp.x + 8), (sint16)(gFp.y + 40), 0);
        fpStartGag(FP_GAG_BONK);
        return;
    }

    if (cur & PAD_R1) {
        fpHandleEnvironment(pressed);
        return;
    }

    if (pressed & PAD_CROSS) {
        fpHandleContext();
        return;
    }
    if (pressed & PAD_SQUARE) {
        fpStartGag((enum TFpGag)gFp.gagCursor);
        gFp.gagCursor = (uint8)((gFp.gagCursor + 1) % FP_GAG_COUNT);
        return;
    }
    if (pressed & PAD_CIRCLE) {
        fpStartGag((enum TFpGag)gFp.lastGag);
        return;
    }
    if (pressed & PAD_TRIANGLE) {
        fpStartSummon((enum TFpSummon)gFp.summonCursor);
        gFp.summonCursor = (uint8)((gFp.summonCursor + 1) % FP_SUMMON_CLOUD);
        return;
    }

    if (cur & PAD_LEFT)      { dx = -1; gFp.face = FP_FACE_W; }
    else if (cur & PAD_RIGHT){ dx =  1; gFp.face = FP_FACE_E; }
    else if (cur & PAD_UP)   { dy = -1; gFp.face = FP_FACE_N; }
    else if (cur & PAD_DOWN) { dy =  1; gFp.face = FP_FACE_S; }

    if (dx || dy) {
        int speed = (cur & PAD_L2) ? 2 : 1;
        if (cur & PAD_R2)
            speed = (gFp.frame & 1) ? 1 : 0;
        if (gFp.drunk && (gFp.frame & 0x10))
            dx = -dx;
        gFp.x = (sint16)(gFp.x + dx * speed);
        gFp.y = (sint16)(gFp.y + dy * speed);
        fpClampJohnny();
        gFp.walkFrame++;
        gFp.walkFrames++;
        gFp.walkingAccumFrames++;
        gFp.idleFrames = 0;
        gFp.mode = FP_MODE_WALK;
    } else if (gFp.mode == FP_MODE_WALK) {
        gFp.mode = FP_MODE_IDLE;
    }
}

static void fpTickAsset(struct TFpAsset *asset)
{
    if (!asset->active)
        return;
    if (asset->ttl > 0)
        asset->ttl--;
    if (asset->ttl == 0) {
        asset->active = 0;
        return;
    }
    asset->x = (sint16)(asset->x + asset->vx);
    asset->y = (sint16)(asset->y + asset->vy);
    asset->frameTimer++;
    if (asset->frameTimer >= asset->frameDelay) {
        asset->frameTimer = 0;
        asset->frame++;
    }
}

static void fpTick(void)
{
    fpTickAsset(&gFp.gag);
    fpTickAsset(&gFp.summon);

    if (gFp.modeTimer > 0) {
        gFp.modeTimer--;
        if (gFp.modeTimer == 0)
            gFp.mode = FP_MODE_IDLE;
    }
    if (gFp.helpTimer > 0) {
        gFp.helpTimer--;
        if (gFp.helpTimer == 0)
            gFp.helpVisible = 0;
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

    if (gFp.frame >= gFp.ambientNextFrame &&
        gFp.mode == FP_MODE_IDLE &&
        !gFp.summon.active) {
        fpStartSummon((rand() & 1) ? FP_SUMMON_CLOUD : FP_SUMMON_SEAGULL);
        gFp.ambientNextFrame = gFp.frame + 1800 + (uint32)(rand() % 2400);
    }

    if (gFp.walkingAccumFrames > 0 &&
        gFp.walkingAccumFrames % (90U * 60U) == 0 &&
        gFp.mode == FP_MODE_WALK) {
        fpStartGag(FP_GAG_HOT);
    }

    if (gFp.idleFrames == 60U * 60U && gFp.mode == FP_MODE_IDLE) {
        fpStartAssetMode("ZZZZS.BMP", 240, FP_MODE_SLEEP,
                         (sint16)(gFp.x + 20), (sint16)(gFp.y - 20), -1);
    }
}

static void fpDrawAsset(const struct TFpAsset *asset)
{
    if (!asset->active)
        return;
    if (asset->screenRelative)
        fpSetScreenDrawOffset();
    else
        fpSetIslandDrawOffset();
    if (asset->flip)
        grDrawSpriteFlip(grBackgroundSfc, &gFpSlot,
                         asset->x, asset->y,
                         asset->frame, asset->slot);
    else
        grDrawSprite(grBackgroundSfc, &gFpSlot,
                     asset->x, asset->y,
                     asset->frame, asset->slot);
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
                 gFp.fireX, gFp.fireY,
                 gFp.fireFrame, FP_SLOT_PERSIST);
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

    if (gFp.helpVisible) {
        panel = (POLY_F4 *)fpOverlayNext;
        fpOverlayNext += sizeof(POLY_F4);
        setPolyF4(panel);
        setSemiTrans(panel, 1);
        setRGB0(panel, 0, 0, 0);
        setXY4(panel, 16, 16, 390, 16, 16, 178, 390, 178);
        ps1GpuOtAddPrim(&fpOverlayOt[FP_OVERLAY_OT_LEN - 2], panel);

        fpTextStart(24, 26);
        fpText("FREEPLAY\n");
        fpText("D-PAD WALK   X DO THING\n");
        fpText("SQUARE GAG   CIRCLE REPLAY\n");
        fpText("TRIANGLE SUMMON\n");
        fpText("R1+D-PAD ENV  R1+X FIRE\n");
        fpText("L1+BUTTON DIRECT GAGS\n");
        fpText("START EXIT   SELECT HELP\n");
    }

    if (gFp.bannerTimer > 0) {
        panel = (POLY_F4 *)fpOverlayNext;
        fpOverlayNext += sizeof(POLY_F4);
        setPolyF4(panel);
        setSemiTrans(panel, 1);
        setRGB0(panel, 0, 0, 0);
        setXY4(panel, 188, 404, 452, 404, 188, 440, 452, 440);
        ps1GpuOtAddPrim(&fpOverlayOt[FP_OVERLAY_OT_LEN - 3], panel);
        fpTextStart(210, 414);
        fpText(gFp.banner);
    }

    if (gFreeplayTelemetryLevel >= 2) {
        char line[64];
        snprintf(line, sizeof(line), "FP %s X%d Y%d R%lu A%lu",
                 fpModeName(gFp.mode), (int)gFp.x, (int)gFp.y,
                 (unsigned long)gFp.cleanRebuilds,
                 (unsigned long)gFp.assetLoadFailures);
        fpTextStart(12, 456);
        fpText(line);
    }
}

static void fpPresent(void)
{
    ps1FrameCount++;
    VSync(0);
    grDrawBackground();
    if (gFp.helpVisible || gFp.bannerTimer > 0 || gFreeplayTelemetryLevel >= 2) {
        pauseMenuEnsureFontUploaded();
        fpBuildOverlay();
        DrawOTag(&fpOverlayOt[FP_OVERLAY_OT_LEN - 1]);
    }
    DrawSync(0);
}

static void fpFrame(void)
{
    grBeginFrame();
    grRestoreBgFromRects();
    fgBackdropTickWavesPublic();
    fpDrawFire();
    fpDrawAsset(&gFp.summon);
    fpDrawJohnny();
    fgBackdropStampHolidayPublic();
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
    gFp.ambientNextFrame = 1800 + (uint32)(rand() % 1200);
    gFp.lastGag = FP_GAG_BONK;
    fpSetBanner("FREEPLAY", 120);
}

void freeplayRun(void)
{
    uint16 cur;
    uint16 pressed;

    gFreeplayExitRequested = 0;
    fpInitState();
    memset(&gFpSlot, 0, sizeof(gFpSlot));

    FP_LOG(1, ("JCFREE start heapKB=%lu night=%d low=%d raft=%d holiday=%d\n",
               fgProbeLargestAlloc() / 1024UL,
               islandState.night, islandState.lowTide,
               islandState.raft, islandState.holiday));

    if (!fpLoadSlot(FP_SLOT_JOHN, "JOHNWALK.BMP")) {
        FP_LOG(1, ("JCFREE fatal missing JOHNWALK\n"));
        gFreeplayExitRequested = 1;
        return;
    }

    if (!fpRebuildBackdrop()) {
        gFreeplayExitRequested = 1;
        fpReleaseSlots();
        fgBackdropReleasePublic(1);
        return;
    }

    grForceFullRedrawNextFrame();

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

    grDeactivateCleanBgRects();
    fpReleaseSlots();
    fgBackdropReleasePublic(1);
    walkRenderResetCache();
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

#endif
