/*
 *  This file is part of 'Johnny Reborn' — PS1 port (walk subsystem).
 *  Copyright (C) 2026 Hunter Davis. GPL-3.0.
 *
 *  walk_pilot — story-loop walk driver. See walk_pilot.h for contract
 *  and docs/ps1/walk-implementation-plan.md § Phase 2.5 for design.
 */

#include <stddef.h>
#include <stdlib.h>
#include "mytypes.h"
#include "mem_region.h"
#include "ps1_debug.h"
#include "graphics_ps1.h"
#include "events_ps1.h"
#include "foreground_pilot.h"
#include "island.h"
#include "walk.h"
#include "walk_render.h"
#include "walk_pilot.h"
#ifdef PS1_BUILD
#include "cdrom_ps1.h"
#include "ps1_spu_cache.h"
#include "psb_registry.h"
#endif

extern struct TIslandState islandState;
extern int grDx, grDy;

#ifndef PS1_BUILD
/* Host build's eventsWaitTick / VSync; not used here, but keep the
 * signature so the file can compile on host if anyone ever lifts
 * the story-loop driver across. */
extern int eventsWaitTick(int n);
#endif

/* Per-walk frame cadence. walkAnimate() returns delays in this many
 * VBlanks between pose advances. Original walk.c sets timer=delay=6,
 * but that was tuned to PC clock-tick semantics; on PS1 at 60Hz it
 * works out to ~100ms/pose and a long SPOT_A→SPOT_F walk runs ~3s,
 * which the user reads as "very slow". 3 VBlanks (~50ms/pose) keeps
 * the leg-swing readable but cuts walk duration roughly in half. */
#define WALK_VBLANKS_PER_POSE 3

/* JOHNWALK.PSB slot — owned by walk_pilot, lazy-loaded on first walk,
 * persists across walks for the lifetime of a screensaver-loop
 * iteration to avoid CD reloads on every transition. */
static struct TTtmSlot gWalkBmpSlot;
static int gWalkBmpLoaded = 0;

#ifdef PS1_BUILD
static uint8 gWalkSpuStageEnabled = 0;
struct TWalkSpuPsbStage {
    uint8 ready;
    uint8 reading;
    uint8 *buffer;
    uint32 validBytes;
    uint32 readBytes;
    uint32 spuOffset;
    uint32 capacityBytes;
    CdlFILE file;
    const char *bmpName;
    const char *psbPath;
    const char *stageTag;
    const char *loadTag;
};

static struct TWalkSpuPsbStage gJohnwalkSpuStage;
static struct TWalkSpuPsbStage gMraftSpuStage;
#endif

/* Synthetic ttmThread used to satisfy walkAnimate's signature.
 * walk.c reads ttmSlot + ttmLayer; ttmLayer is the SDL_Surface (= PS1Surface)
 * field, NULL on PS1 since the kernel doesn't dereference it. */
static struct TTtmThread gWalkThread;

/* Persistent walk-area pristine snapshot. Holds bgTile pixels at a
 * known-clean moment (right after fgBackdropEnableWaveBackdrop, before
 * scene playback dirties them). Under PS1 spu-stage this snapshot lives
 * in SPU cold storage and restores dirty rows through a tiny scratch row;
 * the malloc fallback keeps the allocation resident once it succeeds.
 *
 * The old buffer was a fixed screen-space rect (80,150,520,180). That
 * missed Johnny whenever the randomized island y offset pushed the walk
 * path down: walk_data top-left y can reach 257 + islandState.yPos, and
 * JOHNWALK frames are up to 78 px tall. The result was lower-body residue
 * from every previous walking pose. Keep roughly the same allocation size,
 * but anchor the rect to the island so it always follows the path.
 *
 * Relative bbox covers walkData x=284..526/y=209..257 plus the largest
 * JOHNWALK frame (48x78), plus the behind-tree cover-up sprites
 * (leaves 365,122 152x69; trunk 442,148 24x145). The right edge keeps
 * a small margin beyond Johnny's widest pose while trimming empty
 * padding to preserve main-RAM headroom.
 *
 * Size: 304 x 220 x 2 = ~131KB. */
#define WALK_CLEAN_REL_X  276
#define WALK_CLEAN_REL_Y  120
#define WALK_CLEAN_W      304
#define WALK_CLEAN_H      220
#ifdef PS1_BUILD
#define WALK_CLEAN_ROW_BYTES \
    ((uint32)WALK_CLEAN_W * (uint32)sizeof(uint16))
#define WALK_CLEAN_SPU_ROW_BYTES \
    ((WALK_CLEAN_ROW_BYTES + PS1_SPU_CACHE_ALIGN - 1u) & \
     ~(PS1_SPU_CACHE_ALIGN - 1u))
#define WALK_CLEAN_SPU_BYTES \
    (WALK_CLEAN_SPU_ROW_BYTES * (uint32)WALK_CLEAN_H)
#endif

static uint16 *gWalkCleanBuf  = NULL;
static int gWalkCleanInBootRegion = 0;
#ifdef PS1_BUILD
static uint8   gWalkCleanInSpu = 0;
#endif
static int     gWalkCleanValid = 0;
static sint16  gWalkCleanX = 0;
static sint16  gWalkCleanY = 0;
/* State key — any field changing means the buffer's pixels are stale. */
static int gWalkCleanRaft     = -1;
static int gWalkCleanLowTide  = -1;
static int gWalkCleanNight    = -1;
static int gWalkCleanHoliday  = -1;
static int gWalkCleanXPos     = -32768;
static int gWalkCleanYPos     = -32768;
/* The day ocean backdrop is one of THREE randomly-rolled SCRs per scene
 * (islandInit OCEAN00..02). A baseline captured over one variant and
 * restored over another paints a Johnny-shaped patch of mismatched
 * water at his walk-end pose (the "ghost Johnny" water silhouette). */
static int gWalkCleanBackdrop = -2;

static int walkPilotCleanStateMatches(int raft, int lowTide, int night,
                                      int holidayId, int xPos, int yPos)
{
    return gWalkCleanRaft == raft &&
           gWalkCleanLowTide == lowTide &&
           gWalkCleanNight == night &&
           gWalkCleanHoliday == holidayId &&
           gWalkCleanXPos == xPos &&
           gWalkCleanYPos == yPos &&
           gWalkCleanBackdrop == islandBackdropVariant();
}


#ifdef PS1_BUILD
static int walkPilotCleanSpuAvailable(void)
{
    if (!gWalkSpuStageEnabled)
        return 0;

    ps1SpuCacheInit();
    if (!ps1SpuCacheReady() ||
        WALK_CLEAN_SPU_BYTES > PS1_SPU_CACHE_WALK_CLEAN_BYTES ||
        ps1SpuCacheCapacity() < PS1_SPU_CACHE_WALK_CLEAN_OFFSET ||
        ps1SpuCacheCapacity() - PS1_SPU_CACHE_WALK_CLEAN_OFFSET <
            WALK_CLEAN_SPU_BYTES)
        return 0;

    return 1;
}
#endif


void walkPilotCaptureCleanWalkAreaIfStale(int raft, int lowTide, int night,
                                          int holidayId, int xPos, int yPos)
{
    int captured = 0;

    if (gWalkCleanValid &&
        walkPilotCleanStateMatches(raft, lowTide, night, holidayId, xPos, yPos))
        return;

    gWalkCleanX = (sint16)(xPos + WALK_CLEAN_REL_X);
    gWalkCleanY = (sint16)(yPos + WALK_CLEAN_REL_Y);

#ifdef PS1_BUILD
    if (walkPilotCleanSpuAvailable()) {
        /* Keep the BOOT-region buffer resident even when SPU capture
         * serves (it cannot be returned to BOOT anyway); only a
         * legacy libc buffer is worth releasing. */
        if (gWalkCleanBuf != NULL && !gWalkCleanInBootRegion) {
            free(gWalkCleanBuf);
            gWalkCleanBuf = NULL;
        }
        if (grCaptureBgRectToSpu(PS1_SPU_CACHE_WALK_CLEAN_OFFSET,
                                 gWalkCleanX, gWalkCleanY,
                                 WALK_CLEAN_W, WALK_CLEAN_H,
                                 WALK_CLEAN_SPU_ROW_BYTES)) {
            gWalkCleanInSpu = 1;
            captured = 1;
        } else {
            gWalkCleanInSpu = 0;
        }
    }
#endif

    if (captured == 0) {
#ifdef PS1_BUILD
        gWalkCleanInSpu = 0;
#endif
        if (gWalkCleanBuf == NULL) {
            gWalkCleanBuf = (uint16 *)malloc(
                (size_t)WALK_CLEAN_W * (size_t)WALK_CLEAN_H * sizeof(uint16));
            if (gWalkCleanBuf == NULL) {
                extern int printf(const char *, ...);
                printf("JCWALK: walkClean buffer alloc failed\n");
                return;
            }
        }

        grCaptureBgRect(gWalkCleanBuf, gWalkCleanX, gWalkCleanY,
                        WALK_CLEAN_W, WALK_CLEAN_H);
    }

    gWalkCleanValid    = 1;
    gWalkCleanRaft     = raft;
    gWalkCleanLowTide  = lowTide;
    gWalkCleanNight    = night;
    gWalkCleanHoliday  = holidayId;
    gWalkCleanXPos     = xPos;
    gWalkCleanYPos     = yPos;
    gWalkCleanBackdrop = islandBackdropVariant();
}


void walkPilotReleaseCleanWalkArea(void)
{
    if (gWalkCleanBuf && !gWalkCleanInBootRegion) {
        free(gWalkCleanBuf);
        gWalkCleanBuf = NULL;
    }
#ifdef PS1_BUILD
    gWalkCleanInSpu = 0;
#endif
    gWalkCleanValid = 0;
}


int walkPilotCleanBufferAllocated(void)
{
#ifdef PS1_BUILD
    if (gWalkCleanInSpu)
        return 1;
#endif
    return gWalkCleanBuf != NULL ? 1 : 0;
}

unsigned long walkPilotCleanBufferBytes(void)
{
#ifdef PS1_BUILD
    if (gWalkCleanInSpu)
        return (unsigned long)WALK_CLEAN_W *
               (unsigned long)WALK_CLEAN_H *
               (unsigned long)sizeof(uint16);
#endif
    if (gWalkCleanBuf == NULL) return 0;
    return (unsigned long)WALK_CLEAN_W *
           (unsigned long)WALK_CLEAN_H *
           (unsigned long)sizeof(uint16);
}

int walkPilotJohnwalkSlotLoaded(void)
{
    return gWalkBmpLoaded ? 1 : 0;
}

#ifdef PS1_BUILD
static uint32 walkPilotAlignUp(uint32 value, uint32 align)
{
    return (value + (align - 1u)) & ~(align - 1u);
}

static void walkPilotInitSpuStageDescriptors(void)
{
    if (gJohnwalkSpuStage.bmpName != NULL)
        return;

    gJohnwalkSpuStage.spuOffset = PS1_SPU_CACHE_WALK_PSB_OFFSET;
    gJohnwalkSpuStage.capacityBytes = PS1_SPU_CACHE_WALK_PSB_BYTES;
    gJohnwalkSpuStage.bmpName = "JOHNWALK.BMP";
    gJohnwalkSpuStage.psbPath = "PSB\\JOHNWALK.PSB";
    gJohnwalkSpuStage.stageTag = "johnwalk_spu_stage";
    gJohnwalkSpuStage.loadTag = "johnwalk_spu_load";

    gMraftSpuStage.spuOffset = PS1_SPU_CACHE_MRAFT_PSB_OFFSET;
    gMraftSpuStage.capacityBytes = PS1_SPU_CACHE_MRAFT_PSB_BYTES;
    gMraftSpuStage.bmpName = "MRAFT.BMP";
    gMraftSpuStage.psbPath = "PSB\\MRAFT.PSB";
    gMraftSpuStage.stageTag = "mraft_spu_stage";
    gMraftSpuStage.loadTag = "mraft_spu_load";
}

/* Persistent SPU->RAM load slab. The walk runs between every scene
 * pair and loads the same 48 KB JOHNWALK.PSB each time; allocating and
 * freeing that block per walk churns CACHE between long-lived blocks
 * and was the residual fragmentation source after the clean-rect slab
 * fix (4th-transition walk BSOD: req=49152, 97 KB free, none
 * contiguous). Allocated grow-only on first use, handed to the sprite
 * slot caller-owned so grReleaseBmp drops the reference without
 * freeing. JOHNWALK only — MRAFT's 12 KB load is rarer, same-size, and
 * its slot release point lives in scene code where the slab's busy
 * flag cannot be cleared reliably. */
static uint8 *gWalkPsbLoadSlab = NULL;
static uint32 gWalkPsbLoadSlabSize = 0;
static int gWalkPsbLoadSlabBusy = 0;

static void walkPilotFreeSpuStageBuffer(struct TWalkSpuPsbStage *stage)
{
    if (stage == NULL || stage->buffer == NULL)
        return;
    if (memIsReady())
        memFree(MEM_REGION_CACHE, stage->buffer);
    else
        free(stage->buffer);
    stage->buffer = NULL;
}

static void walkPilotInvalidateSpuStage(struct TWalkSpuPsbStage *stage,
                                        int dropReady)
{
    if (stage == NULL)
        return;
    if (stage->reading) {
        while (ps1_streamAsyncReadPoll() == PS1_CD_ASYNC_PENDING)
            VSync(0);
    }
    stage->reading = 0;
    walkPilotFreeSpuStageBuffer(stage);
    if (dropReady) {
        stage->ready = 0;
        stage->validBytes = 0;
        stage->readBytes = 0;
    }
}

void walkPilotSetSpuStage(int enabled)
{
    walkPilotInitSpuStageDescriptors();
    gWalkSpuStageEnabled = enabled ? 1 : 0;
    if (!gWalkSpuStageEnabled) {
        walkPilotInvalidateSpuStage(&gJohnwalkSpuStage, 1);
        walkPilotInvalidateSpuStage(&gMraftSpuStage, 1);
    }
}

int walkPilotReliefFreePsbSlab(void)
{
    if (gWalkPsbLoadSlab == NULL || gWalkPsbLoadSlabBusy)
        return 0;
    memFree(MEM_REGION_CACHE, gWalkPsbLoadSlab);
    gWalkPsbLoadSlab = NULL;
    gWalkPsbLoadSlabSize = 0;
    return 1;
}

/* Unconditional walk-slab teardown for the full-cache-reset. Unlike the relief
 * variant above, this does NOT honor the busy flag: the entire CACHE region is
 * about to be wiped, so the sprite slot that points into the slab must be
 * dropped (grReleaseBmp on the caller-owned slot detaches without freeing the
 * slab), busy cleared, and the slab freed + NULLed here. If left busy-gated,
 * gWalkPsbLoadSlab survives the wipe as a dangling pointer that a later free
 * corrupts — the deep-soak BSOD signature (fp=344092, johnwalkSlotLoaded=0,
 * CACHE block size corrupt ~12 min after a reset). */
void walkPilotForceDropPsbSlab(void)
{
    if (gWalkBmpLoaded) {
        grReleaseBmp(&gWalkBmpSlot, 0);
        gWalkBmpLoaded = 0;
    }
    gWalkPsbLoadSlabBusy = 0;
    if (gWalkPsbLoadSlab != NULL) {
        memFree(MEM_REGION_CACHE, gWalkPsbLoadSlab);
        gWalkPsbLoadSlab = NULL;
        gWalkPsbLoadSlabSize = 0;
    }
    walkRenderResetCache();
}

unsigned long walkPilotPsbSlabIdleBytes(void)
{
    if (gWalkPsbLoadSlab == NULL || gWalkPsbLoadSlabBusy)
        return 0;
    return (unsigned long)gWalkPsbLoadSlabSize;
}

void walkPilotReservePsbSlab(unsigned long bytes)
{
    if (!gWalkSpuStageEnabled || gWalkPsbLoadSlab != NULL || !memIsReady())
        return;
    /* MEM_REGION_RATIONALE: session-lifetime walk PSB load slab,
     * reserved at boot as part of the stable CACHE shape. No-halt: a
     * relief cycle can free this slab and ask to re-reserve it under a
     * fragmented CACHE (deep-soak BSOD 2026-06-23: req=49152 have=98268,
     * none contiguous). A failed reservation leaves the slab NULL and
     * the consume path retries / the walk degrades to a teleport. */
    memSetCacheAllocNoHalt(1);
    gWalkPsbLoadSlab = (uint8 *)memAlloc(MEM_REGION_CACHE, bytes,
                                         "johnwalk_spu_load");
    memSetCacheAllocNoHalt(0);
    if (gWalkPsbLoadSlab != NULL)
        gWalkPsbLoadSlabSize = (uint32)bytes;
}

static int walkPilotCompletePsbSpuRead(struct TWalkSpuPsbStage *stage)
{
    if (stage == NULL ||
        stage->buffer == NULL ||
        stage->readBytes == 0) {
        walkPilotInvalidateSpuStage(stage, 1);
        return 0;
    }

    ps1SpuCacheInit();
    if (!ps1SpuCacheReady() ||
        ps1SpuCacheCapacity() < stage->spuOffset ||
        ps1SpuCacheCapacity() - stage->spuOffset < stage->readBytes ||
        !ps1SpuCacheWrite(stage->spuOffset,
                          stage->buffer,
                          stage->readBytes)) {
        walkPilotInvalidateSpuStage(stage, 1);
        return 0;
    }

    walkPilotFreeSpuStageBuffer(stage);
    stage->reading = 0;
    stage->ready = 1;
    return 1;
}

static int walkPilotStagePsbSpuTick(struct TWalkSpuPsbStage *stage)
{
    uint32 validBytes;
    uint32 readBytes;
    int asyncResult;

    if (stage == NULL || !gWalkSpuStageEnabled || stage->ready)
        return 0;

    if (stage->reading) {
        asyncResult = ps1_streamAsyncReadPoll();
        if (asyncResult == PS1_CD_ASYNC_PENDING)
            return 1;
        if (asyncResult != PS1_CD_ASYNC_DONE) {
            walkPilotInvalidateSpuStage(stage, 1);
            return 0;
        }
        return walkPilotCompletePsbSpuRead(stage);
    }

    validBytes = psbRegistryLookup(stage->bmpName);
    if (validBytes == 0)
        return 0;
    readBytes = walkPilotAlignUp(validBytes, 2048u);
    readBytes = walkPilotAlignUp(readBytes, PS1_SPU_CACHE_ALIGN);
    if (stage->capacityBytes != 0 && readBytes > stage->capacityBytes)
        return 0;

    ps1SpuCacheInit();
    if (!ps1SpuCacheReady() ||
        ps1SpuCacheCapacity() < stage->spuOffset ||
        ps1SpuCacheCapacity() - stage->spuOffset < readBytes)
        return 0;

    if (memIsReady())
        stage->buffer = (uint8 *)memAlloc(MEM_REGION_CACHE,
                                          readBytes,
                                          stage->stageTag);
    else
        stage->buffer = (uint8 *)malloc(readBytes);
    if (stage->buffer == NULL)
        return 0;

    if (!ps1_streamResolveFile(stage->psbPath, &stage->file) ||
        !ps1_streamAsyncReadAlignedBegin(&stage->file,
                                         0,
                                         readBytes,
                                         stage->buffer)) {
        walkPilotInvalidateSpuStage(stage, 1);
        return 0;
    }

    stage->validBytes = validBytes;
    stage->readBytes = readBytes;
    stage->reading = 1;
    return 1;
}

int walkPilotStageJohnwalkSpuTick(void)
{
    walkPilotInitSpuStageDescriptors();
    return walkPilotStagePsbSpuTick(&gJohnwalkSpuStage);
}

static int walkPilotFinishPsbSpuStage(struct TWalkSpuPsbStage *stage)
{
    while (stage != NULL && stage->reading) {
        if (!walkPilotStagePsbSpuTick(stage))
            return 0;
        if (stage->reading)
            VSync(0);
    }
    return (stage != NULL && stage->ready) ? 1 : 0;
}

static int walkPilotPrimePsbSpuBlocking(struct TWalkSpuPsbStage *stage)
{
    if (stage == NULL || !gWalkSpuStageEnabled || stage->ready)
        return (stage != NULL && stage->ready) ? 1 : 0;

    do {
        if (!walkPilotStagePsbSpuTick(stage))
            return stage->ready ? 1 : 0;
        if (stage->reading)
            VSync(0);
    } while (!stage->ready);

    return 1;
}

int walkPilotPrimeSpuAssetsBlocking(void)
{
    int ok = 1;
    walkPilotInitSpuStageDescriptors();
    if (!walkPilotPrimePsbSpuBlocking(&gJohnwalkSpuStage))
        ok = 0;
    if (!walkPilotPrimePsbSpuBlocking(&gMraftSpuStage))
        ok = 0;
    return ok;
}

static int walkPilotLoadPsbFromSpu(struct TWalkSpuPsbStage *stage,
                                   struct TTtmSlot *slot,
                                   uint16 slotNo)
{
    uint8 *psbBuf;
    int callerOwned = 0;

    if (stage == NULL || !gWalkSpuStageEnabled)
        return 0;
    if (!stage->ready && !walkPilotFinishPsbSpuStage(stage))
        return 0;
    if (stage->validBytes == 0 || stage->readBytes == 0)
        return 0;

    if (memIsReady() && !gWalkPsbLoadSlabBusy &&
        stage == &gJohnwalkSpuStage) {
        if (gWalkPsbLoadSlab != NULL &&
            gWalkPsbLoadSlabSize < stage->readBytes) {
            memFree(MEM_REGION_CACHE, gWalkPsbLoadSlab);
            gWalkPsbLoadSlab = NULL;
            gWalkPsbLoadSlabSize = 0;
        }
        if (gWalkPsbLoadSlab == NULL) {
            /* MEM_REGION_RATIONALE: grow-only persistent slab; same
             * block reused for every inter-scene walk PSB load to keep
             * CACHE churn-free across boundaries. No-halt: under deep-soak
             * fragmentation a NULL leaves the slab unset so we fall to the
             * per-load alloc below and ultimately to a teleport. */
            memSetCacheAllocNoHalt(1);
            gWalkPsbLoadSlab = (uint8 *)memAlloc(MEM_REGION_CACHE,
                                                 stage->readBytes,
                                                 stage->loadTag);
            memSetCacheAllocNoHalt(0);
            if (gWalkPsbLoadSlab != NULL)
                gWalkPsbLoadSlabSize = stage->readBytes;
        }
    }

    if (memIsReady() && gWalkPsbLoadSlab != NULL && !gWalkPsbLoadSlabBusy &&
        stage == &gJohnwalkSpuStage &&
        gWalkPsbLoadSlabSize >= stage->readBytes) {
        psbBuf = gWalkPsbLoadSlab;
        callerOwned = 1;
    } else if (memIsReady()) {
        /* No-halt: a fragmented deep-soak CACHE returns NULL here and the
         * walk/raft load degrades gracefully (return 0) rather than
         * BSOD'ing the allocator. */
        memSetCacheAllocNoHalt(1);
        psbBuf = (uint8 *)memAlloc(MEM_REGION_CACHE,
                                   stage->readBytes,
                                   stage->loadTag);
        memSetCacheAllocNoHalt(0);
    } else {
        psbBuf = (uint8 *)malloc(stage->readBytes);
    }
    if (psbBuf == NULL)
        return 0;

    ps1SpuCacheInit();
    if (!ps1SpuCacheReady() ||
        !ps1SpuCacheRead(stage->spuOffset, psbBuf, stage->readBytes)) {
        if (!callerOwned) {
            if (memIsReady())
                memFree(MEM_REGION_CACHE, psbBuf);
            else
                free(psbBuf);
        }
        return 0;
    }

    if (callerOwned) {
        int loaded = grLoadPsbBufferCallerOwned(slot, slotNo,
                                                (char *)stage->bmpName,
                                                psbBuf, stage->validBytes);
        /* The slab stays "busy" while the walk sprite slot points into
         * it so nothing can overwrite live sprite data;
         * fgWalkRenderTeardown clears it at every scene boundary. */
        gWalkPsbLoadSlabBusy = loaded ? 1 : 0;
        return loaded;
    }

    return grLoadPsbBuffer(slot, slotNo, (char *)stage->bmpName,
                           psbBuf, stage->validBytes);
}

static int walkPilotLoadJohnwalkFromSpu(void)
{
    walkPilotInitSpuStageDescriptors();
    return walkPilotLoadPsbFromSpu(&gJohnwalkSpuStage, &gWalkBmpSlot, 0);
}

int walkPilotLoadMraftFromSpu(struct TTtmSlot *slot, uint16 slotNo)
{
    walkPilotInitSpuStageDescriptors();
    return walkPilotLoadPsbFromSpu(&gMraftSpuStage, slot, slotNo);
}
#else
void walkPilotSetSpuStage(int enabled)
{
    (void)enabled;
}

int walkPilotStageJohnwalkSpuTick(void)
{
    return 0;
}

int walkPilotPrimeSpuAssetsBlocking(void)
{
    return 0;
}

int walkPilotLoadMraftFromSpu(struct TTtmSlot *slot, uint16 slotNo)
{
    (void)slot;
    (void)slotNo;
    return 0;
}

int walkPilotReliefFreePsbSlab(void)
{
    return 0;
}

void walkPilotForceDropPsbSlab(void)
{
}

unsigned long walkPilotPsbSlabIdleBytes(void)
{
    return 0;
}

void walkPilotReservePsbSlab(unsigned long bytes)
{
    (void)bytes;
}
#endif


int walkPilotInit(void)
{
    int ok = 1;

    /* The clean buffer is allocated ONCE here, from the BOOT region,
     * and never freed (BOOT freezes after boot; memFree(BOOT) is a
     * no-op). It was previously a lazy libc malloc that the SPU-stage
     * path skipped at init and re-created mid-soak whenever SPU
     * capture was unavailable — the churning 130 KB libc resident
     * ("walkCleanKB=130") that starved the CACHE libc-fallback path
     * in every deep-soak BSOD. Allocating up front, before
     * memFreezeBoot and graphicsInit, makes its placement
     * deterministic and returns ~134 KB of contiguous libc headroom
     * to the spike paths. */
    if (gWalkCleanBuf == NULL) {
        gWalkCleanBuf = (uint16 *)malloc(
            (size_t)WALK_CLEAN_W * (size_t)WALK_CLEAN_H * sizeof(uint16));
        gWalkCleanInBootRegion = 1;  /* persistent: allocated once at
                                      * boot, never freed (see header
                                      * comment at the declaration) */
        if (gWalkCleanBuf == NULL) {
            extern int printf(const char *, ...);
            printf("JCWALK: walkPilotInit clean-buf alloc failed\n");
            ok = 0;
        }
        /* Mark invalid until the first scene's setup captures real
         * pristine pixels. walkPilotRestoreClean checks gWalkCleanValid
         * before copying, so an early-walk attempt is a safe no-op. */
        gWalkCleanValid = 0;
    }

#ifdef PS1_BUILD
    if (gWalkSpuStageEnabled) {
        gWalkCleanValid = 0;
        return ok;
    }
#endif

    /* JOHNWALK.PSB is not pre-loaded into main RAM. Under spu-stage it is
     * cold-cached in SPU at boot and copied back only for a walk; without
     * spu-stage it lazy-loads from CD on first use. */
    return ok;
}


/* Per-walk-frame: copy persistent clean pixels back into bgTile so the
 * previous pose's Johnny is wiped before the new pose composites. The
 * underlying grRestoreBgRect respects prevDirty, so on every frame after
 * the first only Johnny's previously-touched rows actually get copied
 * — same dirty-rect economy as grRestoreBgFromRects. */
static void walkPilotRestoreClean(void)
{
    if (!gWalkCleanValid) return;
#ifdef PS1_BUILD
    if (gWalkCleanInSpu) {
        grRestoreBgRectFromSpu(PS1_SPU_CACHE_WALK_CLEAN_OFFSET,
                               gWalkCleanX, gWalkCleanY,
                               WALK_CLEAN_W, WALK_CLEAN_H,
                               WALK_CLEAN_SPU_ROW_BYTES);
        return;
    }
#endif
    if (gWalkCleanBuf == NULL) return;
    grRestoreBgRect(gWalkCleanBuf, gWalkCleanX, gWalkCleanY,
                    WALK_CLEAN_W, WALK_CLEAN_H);
}


/* Returns 1 on success (sprites present in slot), 0 on silent load
 * failure. grLoadBmp doesn't return a status, so we check the slot's
 * sprite count afterward — the PSB and BMP fallback paths both leave
 * numSprites[0] == 0 when their allocations fail. Without this check,
 * a malloc failure in mid-soak (heap fragmentation after many scene
 * cycles) marked the BMP as loaded anyway and fgWalkRender entered
 * walkAnimate with no sprite data, which spins forever waiting for a
 * scene-end that never fires. Caught 2026-05-13: two consecutive long
 * runs hung at the 6th JOHNWALK load (runtime ~444s, scene activity9
 * after 4 prior JCMEM clean-retry events). */
static int walkPilotEnsureBmp(void)
{
    if (gWalkBmpLoaded) return 1;
#ifdef PS1_BUILD
    /* A deep-soak CACHE can be too fragmented for the 48 KB JOHNWALK load
     * (SPU slab or BMP fallback) to find a contiguous run. Arm no-halt so
     * a failed load returns an empty slot and we degrade to a teleport
     * (skip-walk) instead of halting the allocator. */
    memSetCacheAllocNoHalt(1);
    if (walkPilotLoadJohnwalkFromSpu()) {
        memSetCacheAllocNoHalt(0);
        gWalkBmpLoaded = 1;
        return 1;
    }
#endif
    grLoadBmp(&gWalkBmpSlot, 0, "JOHNWALK.BMP");
#ifdef PS1_BUILD
    memSetCacheAllocNoHalt(0);
#endif
    if (gWalkBmpSlot.numSprites[0] == 0) {
        extern int printf(const char *, ...);
        printf("JCWALK: ensureBmp JOHNWALK load failed numSprites=0; "
               "skipping walk (teleport)\n");
        return 0;
    }
    gWalkBmpLoaded = 1;
    return 1;
}


void fgWalkRenderTeardown(void)
{
    if (!gWalkBmpLoaded) return;
    grReleaseBmp(&gWalkBmpSlot, 0);
    gWalkBmpLoaded = 0;
#ifdef PS1_BUILD
    /* The walk sprite slot no longer points into the persistent PSB
     * load slab; the next walk's SPU load may reuse it. */
    gWalkPsbLoadSlabBusy = 0;
#endif
    walkRenderResetCache();
}


int fgWalkRender(int fromSpot, int fromHdg, int toSpot, int toHdg)
{
    /* No-op walks: invalid prev (LEFT_ISLAND scenes set prevSpot=-1)
     * or same-spot-same-heading. Same-spot-different-heading still
     * walks (Johnny turns in place via walk.c's turn-table logic). */
    if (fromSpot < 0 || toSpot < 0)
        return 0;
    if (fromSpot == toSpot && fromHdg == toHdg)
        return 0;

    if (!walkPilotEnsureBmp()) {
        /* Plan v9 manifest item #19. Under the deterministic allocator,
         * walkPilotEnsureBmp can only fail on a data bug (missing or
         * malformed JOHNWALK.PSB/BMP); halt loudly so the issue is
         * caught at the symptom site rather than masquerading as a
         * teleport. */
        JC_BSOD("walk", "walkPilotEnsureBmp failed (missing JOHNWALK asset)");
    }
    walkRenderResetCache();

    /* Suppress runtime compose for the entire walk. Without this, every
     * frame's grUpdateDisplay call re-stamps the previous FG2 scene's
     * baked-in Johnny on top of the walking sprite, looking exactly
     * like Johnny painting over himself. */
    foregroundPilotSuppressCompose(1);

    /* Force a full-tile dirty so the first walk frame's clean-area
     * restore copies the entire walk-area buffer (not just the rows
     * scene N's last composite happened to dirty). */
    grForceFullRedrawNextFrame();

    /* Match the original engine's island-offset wiring (ads.c:2222). */
    grDx = (int)islandState.xPos;
    grDy = (int)islandState.yPos;

    /* Set up the synthetic thread walkAnimate needs. ttmLayer stays
     * NULL — kernel passes it to grDrawSprite which doesn't deref. */
    gWalkThread.ttmSlot   = &gWalkBmpSlot;
    gWalkThread.ttmLayer  = NULL;
    gWalkThread.timer     = WALK_VBLANKS_PER_POSE;
    gWalkThread.delay     = WALK_VBLANKS_PER_POSE;

    /* The island background slot for behind-tree cover-up. May be
     * NULL during very early scene-loop init; cover-up just degrades
     * to "Johnny visible through tree" until the slot is populated. */
    struct TTtmSlot *bgSlot = fgBackdropGetSlot();

    /* Set walk.c's static state up for this transition. */
    walkInit(fromSpot, fromHdg, toSpot, toHdg);

    grCurrentThread = NULL;     /* kernel's redraw cache covers
                                 * inter-tick frames; thread replay
                                 * mechanism not used. */

    /* Pre-walk dwell: hold scene N's last frame for ~12 VBlanks (~200ms)
     * before the walk's first composite. The framebuffer already shows
     * scene N's final pose at this point, so all we need is to wait
     * VSyncs without redrawing — that lets the eye register the scene's
     * end state before Johnny "switches" into walking pose. Without the
     * dwell, a turn-in-place walk reads as a hard pose-swap at the same
     * spot ("teleported into a new position"). */
    for (int dwell = 0; dwell < 12; dwell++) {
        /* CD is idle for the whole walk: keep the next scene's stage
         * payload read moving (one async chunk per frame). */
        foregroundPilotStageWalkTick();
        VSync(0);
    }

    /* timerLeft starts at 0 so the FIRST loop iteration calls
     * walkAnimate and stamps walk pose 1 in the same frame whose
     * restore wipes the scene's baked-in Johnny. Starting at 1 took
     * the redraw-cached-pose branch with an empty cache: one visible
     * frame with no Johnny at all (the single-frame "disappear" flash
     * reported at the first watched transition). */
    int timerLeft = 0;
    int walkDone  = 0;
    /* Hard cap on walk duration. A legitimate walk is bounded by Sierra's
     * pre-baked route (max ~8 spots × ~30 VBlanks per pose ≈ 240 VBlanks).
     * 600 VBlanks (~10s) is well past that. The cap exists because long
     * soaks deterministically deadlock somewhere inside walkAnimate /
     * grBeginFrame on certain transitions (observed: building7 →
     * activity9 at the 6th walk in a row, runtime ~444s); without a
     * cap the screensaver hangs forever. The visual cost on a real hang
     * is a brief frozen pose; on legitimate walks the cap never fires.
     *
     * Pose pacing and the cap both count ACTUAL elapsed VBlanks, not
     * loop iterations: now that walks are visible (deferred TRANSIENT
     * wipe), each iteration carries real composite+upload work and can
     * span 2+ VBlanks. Counting iterations doubled every pose duration
     * and stretched walks to 9-17 s. */
    int walkVBlanksUsed = 0;
    const int WALK_VBLANK_LIMIT = 600;
    int walkPrevTick = VSync(-1);

    while (!walkDone) {
        if (walkVBlanksUsed > WALK_VBLANK_LIMIT) {
            extern int printf(const char *, ...);
            printf("JCWALK: vblank-cap hit at %d, forcing walkDone\n",
                   walkVBlanksUsed);
            walkDone = 1;
            break;
        }
        grBeginFrame();
        /* Start from the previous scene's clean rects first. This clears
         * foreground leftovers and shoreline wave residue, and mirrors the
         * normal frame-start path's currDirty reset before the walk-specific
         * restore below handles Johnny's full route. */
        grRestoreBgFromRects();
        /* Clear the previous walk pose by restoring pixels from the
         * persistent walk-area pristine buffer. Falls through (no-op)
         * if the capture hasn't been done yet — only the first scene
         * before its setup runs is in this state, which can't have
         * fired a walk anyway. */
        walkPilotRestoreClean();

        /* Wave animation — keep the ocean moving during walks so
         * scene→walk→scene looks seamless. */
        fgBackdropTickWavesPublic();

        /* Holiday decorations are part of the island backdrop during walks.
         * Stamp them before Johnny so the walking sprite is never hidden by
         * a menu/default holiday overlay. Scene packs still bake holiday on
         * top where the original z-order calls for it. */
        fgBackdropStampHolidayPublic();

        if (timerLeft <= 0) {
            /* Tick: walkAnimate advances pose + calls walkRenderFrame
             * (which stamps Johnny + updates the redraw cache). */
            int next = walkAnimate(&gWalkThread, bgSlot);
            if (next <= 0) {
                walkRedrawLastFrame(NULL, &gWalkBmpSlot, bgSlot);
                walkDone = 1;
                timerLeft = 0;
            } else {
                timerLeft = next;
            }
        } else {
            /* Inter-tick frame: redraw the cached pose. */
            walkRedrawLastFrame(NULL, &gWalkBmpSlot, bgSlot);
        }

        foregroundPilotStageWalkTick();
        grUpdateDisplay(NULL, NULL, NULL);

        {
            /* Charge pose timing + the duration cap with the VBlanks
             * this frame actually consumed (>= 1 after the VSync inside
             * grUpdateDisplay). */
            int nowTick = VSync(-1);
            int took = nowTick - walkPrevTick;
            if (took < 1)
                took = 1;
            walkPrevTick = nowTick;
            timerLeft -= took;
            walkVBlanksUsed += took;
        }
    }

    /* Hold-frame: keep the final pose visible briefly before yielding
     * to the next scene. 12 VBlanks ≈ 200ms for actual position changes.
     *
     * Turn-in-place walks (same spot, just rotating) don't benefit from
     * the hold — Johnny was already at this position, the visible change
     * is just heading. Cut the hold to 0 so back-to-back same-spot
     * scenes feel snappy. */
    int holdFrames = (fromSpot == toSpot) ? 0 : 12;
    for (int hold = 0; hold < holdFrames; hold++) {
        grBeginFrame();
        grRestoreBgFromRects();
        walkPilotRestoreClean();
        fgBackdropTickWavesPublic();
        fgBackdropStampHolidayPublic();
        walkRedrawLastFrame(NULL, &gWalkBmpSlot, bgSlot);
        foregroundPilotStageWalkTick();
        grUpdateDisplay(NULL, NULL, NULL);
    }

    walkRenderResetCache();
    fgWalkRenderTeardown();
    foregroundPilotSuppressCompose(0);
    return 0;
}
