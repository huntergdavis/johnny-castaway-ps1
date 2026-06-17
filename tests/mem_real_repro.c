/*
 * mem_real_repro.c — host-side reproduction of the PS1 scene-945
 * memory-fragmentation BSOD, driven through the REAL game code.
 *
 *   JCBSOD-FATAL CACHE exhausted: req=65536 have=88024
 *   memCacheUsed=600104   (mem_region.c)
 *
 * Links the REAL allocator (src/mem_region.c) and #includes the REAL
 * slab-pool + clean-rect-split code (src/graphics_ps1/clean_rects.c.inc).
 * Establishes the retained "stable shape" CACHE band exactly as the
 * live soak's heap map recorded it, enables slab retention, registers a
 * faithful port of foreground_pilot.c's fgCachePressureRelief tier
 * order, then replays the exact 945-scene sequence
 * (tests/soak945_sequence.h) — computing each scene's clean-rect
 * geometry with the REAL backdrop_clean.c.inc math and feeding it to the
 * REAL grSaveCleanBgRectsSplit. At scene 945 it probes whether a
 * 65536-byte CACHE request strands (largest contiguous free < 65536).
 *
 * Build (clean_rects.c.inc is #included, NOT on the command line):
 *   gcc -O2 -DPS1_MEM_FORENSICS=1 -o tests/mem_real_repro \
 *       tests/mem_real_repro.c src/mem_region.c \
 *       src/generated/pack_header_metrics.c \
 *       -Isrc -Isrc/mem_region -Isrc/platform/ps1 \
 *       -Isrc/graphics_ps1 -Isrc/core
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>

#include "core/mytypes.h"
#include "mem_region/mem_region.h"
#include "graphics_ps1/graphics_ps1.h"

#include "soak945_sequence.h"
#include "soak945_fg2bounds.h"
#include "ps1_mem_model.h"

/* ===================================================================
 * Stub preamble: graphics globals + function stubs that
 * clean_rects.c.inc references. bgTile* are NULL so the copy paths
 * short-circuit on tile->pixels.
 * =================================================================== */
PS1Surface *bgTile0 = NULL, *bgTile1 = NULL, *bgTile3 = NULL, *bgTile4 = NULL;
uint16 *bgTile0Clean = NULL, *bgTile1Clean = NULL,
       *bgTile3Clean = NULL, *bgTile4Clean = NULL;

int prevDirtyRowMinX[4][240];
int prevDirtyRowMaxX[4][240];
int prevDirtyMinY[4];
int prevDirtyMaxY[4];

int ps1PerfEnabled = 0;
uint32 gFgCleanRectMaxBytes = 96u * 1024u;

/* SPU cache alignment (src/platform/ps1/ps1_spu_cache.h). */
#ifndef PS1_SPU_CACHE_ALIGN
#define PS1_SPU_CACHE_ALIGN 64u
#endif

/* Forward decls that graphics_ps1.c provides before #including the .inc. */
static void grRestoreRectFromCleanBg(int x, int y, int width, int height);
extern void memDumpCacheMap(const char *why);

/* SCR-cache globals (static in graphics_ps1.c; owned here so the REAL
 * relief SCR tier in clean_rects.c.inc operates on the band's slab). */
static int     gFullScreenScrCacheEnabled = 1;
static uint8  *gFullScreenScrCache = NULL;
static uint32  gFullScreenScrCacheBytes = 0;
static int     gFullScreenScrCacheValid = 0;
static char    gFullScreenScrCacheName[16] = "";

void grMarkRectDirty(int a,int b,int c,int d){(void)a;(void)b;(void)c;(void)d;}
void grMarkAllTilesDirty(void){}
void grMarkPrevAllTilesDirty(void){}
void grMarkSingleColumnDirty(int a,int b,int c,int d,int e){(void)a;(void)b;(void)c;(void)d;(void)e;}
void grClearCurrDirtyState(void){}
void grEnsureDirtyRowState(void){}
void grSaveCleanBgTiles(void){}
void grDrawBackground(void){}
void ps1PerfMarkRestore(uint32 b){(void)b;}

void ps1SpuCacheInit(void){}
int  ps1SpuCacheRead(uint32 o,void*d,uint32 n){(void)o;(void)d;(void)n;return 0;}
int  ps1SpuCacheWrite(uint32 o,const void*d,uint32 n){(void)o;(void)d;(void)n;return 0;}
int  ps1SpuCacheReady(void){return 0;}
uint32 ps1SpuCacheCapacity(void){return 0;}

static inline void compositeDirectOpaqueRuns(uint16 *dst,const uint16 *src,int count)
{ (void)dst;(void)src;(void)count; }

/* ===================================================================
 * Allocator host shims. memHalt routes to ps1Bsod; we longjmp so a
 * BSOD is observable.
 * =================================================================== */
static jmp_buf g_haltJmp;
static int     g_haltArmed = 0;
static char    g_lastHaltScene[128];
static char    g_lastHaltReason[256];

void ps1DebugInit(void){}
void ps1DebugError(const char*f,...){
    /* memHalt routes here when graphics is "down" (our stub). Treat as a
     * BSOD strand: record + longjmp so fuzz/replay detect it (else the
     * real memHalt spins forever). */
    if (f) strncpy(g_lastHaltReason, f, sizeof(g_lastHaltReason)-1);
    if (g_haltArmed) longjmp(g_haltJmp, 1);
}
void ps1Bsod(const char*s,const char*r,const char*f,int l)
{
    (void)f;(void)l;
    if (s) strncpy(g_lastHaltScene, s, sizeof(g_lastHaltScene)-1);
    if (r) strncpy(g_lastHaltReason, r, sizeof(g_lastHaltReason)-1);
    if (g_haltArmed) longjmp(g_haltJmp, 1);
}
int  graphicsIsInitialized(void){return 0;}
void checkMemoryBudget(void){}
void lruEvictAllUnpinned(void){}
size_t getTotalMemoryUsed(void){return 0;}

/* ===================================================================
 * Include the REAL slab-pool + clean-rect code.
 * =================================================================== */
#include "graphics_ps1/clean_rects.c.inc"

/* ===================================================================
 * Walk-slab tier model (foreground_pilot's walkPilotPsbSlab*).
 * =================================================================== */
static uint16 *g_walkSlab = NULL;
static uint32  g_walkSlabBytes = 0;

static unsigned long walkPilotPsbSlabIdleBytes(void)
{ return (unsigned long)g_walkSlabBytes; }
static int walkPilotReliefFreePsbSlab(void)
{
    if (g_walkSlab == NULL) return 0;
    memFree(MEM_REGION_CACHE, g_walkSlab);
    g_walkSlab = NULL; g_walkSlabBytes = 0;
    return 1;
}

/* Stream-window band member (relief last-resort tier). */
static uint16 *g_streamWindow = NULL;
static uint32  g_streamWindowBytes = 0;

/* Grow-only frame buffer + stream scratch (foreground_pilot.c:592/895).
 * Sized from each pack's max-entry dataSize; grow => free old, alloc
 * new in CACHE (the per-scene CACHE churn the band setup seeds at 16388
 * but the real engine grows). We track the live pointers we seeded in
 * establishBand so a growth frees them and reallocs. */
#define FG_CD_SECTOR_SIZE 2048u
static uint8 *g_frameBuf = NULL;   static uint32 g_frameBufSize = 0;
static uint8 *g_scratch  = NULL;   static uint32 g_scratchSize  = 0;
/* remaining band members (tracked so the scheduled rebuild can free the
 * WHOLE region and rewind to pristine, like the real rewound=1). */
static uint8 *g_prefetch = NULL;
static uint8 *g_backgrnd = NULL;
static uint8 *g_holiday  = NULL;

/* ===================================================================
 * Scheduled CACHE rebuild state — faithful port of the counters in
 * foreground_pilot.c:2288-2298 (fgMaybeScheduledCacheRebuild).
 * =================================================================== */
static int g_scenesSinceRebuild = 0;
static int g_reliefSinceRebuild = 0;
static int g_rebuildCooldown    = 0;
static int g_scrFailStreak      = 0;   /* grScrCacheRefillFailStreak() */
static int g_rebuilds           = 0;
static int g_forceCacheMode     = 0;   /* clean-rect CACHE routing (restored after rebuild) */
static unsigned sectorAlignUp(unsigned v)
{ return (v + FG_CD_SECTOR_SIZE - 1u) & ~(FG_CD_SECTOR_SIZE - 1u); }

static void growFrameBuffers(unsigned maxDataSize)
{
    unsigned frameBytes = sectorAlignUp(maxDataSize); /* GUARD=0 */
    if (frameBytes > g_frameBufSize) {
        if (g_frameBuf != NULL) memFree(MEM_REGION_CACHE, g_frameBuf);
        g_frameBuf = (uint8 *)memAlloc(MEM_REGION_CACHE, frameBytes, "fg-frame");
        g_frameBufSize = frameBytes;
    }
    unsigned requiredScratch = ((maxDataSize + 2047u) / 2048u) * 2048u + 2048u;
    if (requiredScratch > g_scratchSize) {
        if (g_scratch != NULL) memFree(MEM_REGION_CACHE, g_scratch);
        g_scratch = (uint8 *)memAlloc(MEM_REGION_CACHE, requiredScratch, "fg-stream-scratch");
        g_scratchSize = requiredScratch;
    }
}

/* ===================================================================
 * Faithful port of fgCachePressureRelief (foreground_pilot.c:2169),
 * using the REAL pool query/flush functions from clean_rects.c.inc.
 * =================================================================== */
static int g_reliefFires = 0;

static int fgCachePressureRelief(unsigned long requestBytes)
{
    int freed = 0;
    int largestSlab;
    g_reliefFires++;
    g_reliefSinceRebuild++;   /* gFgReliefSinceRebuild (rebuild trigger b) */

    largestSlab = grLargestPooledCleanRectSlabBytes(0);       /* tier0 sub-floor */
    if ((unsigned long)largestSlab >= requestBytes) { grFlushCleanBgRectSlabs(); return 1; }

    if (walkPilotPsbSlabIdleBytes() >= requestBytes) {        /* tier1 walk slab */
        walkPilotReliefFreePsbSlab(); return 1;
    }

    if (requestBytes <= 98304UL) {                            /* tier2 floors */
        largestSlab = grLargestPooledCleanRectSlabBytes(1);
        if ((unsigned long)largestSlab >= requestBytes) { grFlushCleanBgRectSlabsAll(); return 1; }
    }

    if ((unsigned long)grScrCacheResidentBytes() >= requestBytes) { /* tier3 SCR */
        grReliefFreeScrCache(); return 1;
    }

    /* last resort cascade */
    largestSlab = grFlushCleanBgRectSlabs(); if (largestSlab > 0) freed = 1;
    if (walkPilotReliefFreePsbSlab()) freed = 1;
    largestSlab = grReliefFreeScrCache(); if (largestSlab > 0) freed = 1;
    largestSlab = grFlushCleanBgRectSlabsAll(); if (largestSlab > 0) freed = 1;
    if ((unsigned long)largestSlab >= requestBytes) return freed;
    if (g_streamWindow != NULL) {
        memFree(MEM_REGION_CACHE, g_streamWindow);
        g_streamWindow = NULL; g_streamWindowBytes = 0;
        freed = 1;
    }
    return freed;
}

/* ===================================================================
 * Clean-rect geometry — faithful port of
 * fgBackdropSaveCleanBgRectsForPack (backdrop_clean.c.inc:187),
 * routed through the REAL grSaveCleanBgRectsSplit.
 * =================================================================== */
#define CR_WAVE_MIN_X 129
#define CR_WAVE_MIN_Y 303
#define CR_WAVE_END_X 608
#define CR_WAVE_END_Y 356
#define CR_UPPER_SPLIT_Y 190

static int sceneSaveCleanRects(int fgX, int fgY, int fgW, int fgH, uint32 cap)
{
    sint16 fgEndX = (sint16)(fgX + fgW);
    sint16 fgEndY = (sint16)(fgY + fgH);

    if (fgW == 0 || fgH == 0) {
        sint16 xs[1]; sint16 ys[1]; uint16 ws[1]; uint16 hs[1];
        xs[0] = CR_WAVE_MIN_X; ys[0] = CR_WAVE_MIN_Y;
        ws[0] = (uint16)(CR_WAVE_END_X - CR_WAVE_MIN_X);
        hs[0] = (uint16)(CR_WAVE_END_Y - CR_WAVE_MIN_Y);
        return grSaveCleanBgRectsSplit(xs, ys, ws, hs, 1, cap) > 0;
    }

    sint16 lowerMinX = (sint16)fgX;
    sint16 lowerMinY = (sint16)(fgY >= CR_UPPER_SPLIT_Y ? fgY : CR_UPPER_SPLIT_Y);
    sint16 lowerEndX = fgEndX;
    sint16 lowerEndY = fgEndY;
    if (CR_WAVE_MIN_X < lowerMinX) lowerMinX = CR_WAVE_MIN_X;
    if (CR_WAVE_MIN_Y < lowerMinY) lowerMinY = CR_WAVE_MIN_Y;
    if (CR_WAVE_END_X > lowerEndX) lowerEndX = CR_WAVE_END_X;
    if (CR_WAVE_END_Y > lowerEndY) lowerEndY = CR_WAVE_END_Y;
    if (lowerMinX < 0) lowerMinX = 0;
    if (lowerMinY < 0) lowerMinY = 0;
    if (lowerEndX > 640) lowerEndX = 640;
    if (lowerEndY > 480) lowerEndY = 480;

    if (fgY < CR_UPPER_SPLIT_Y) {
        sint16 upperMinX = (sint16)fgX;
        sint16 upperMinY = (sint16)fgY;
        sint16 upperEndX = fgEndX;
        sint16 upperEndY = CR_UPPER_SPLIT_Y;
        sint16 xs[2]; sint16 ys[2]; uint16 ws[2]; uint16 hs[2];
        if (upperMinX < 0) upperMinX = 0;
        if (upperMinY < 0) upperMinY = 0;
        if (upperEndX > 640) upperEndX = 640;
        if (upperEndY > 480) upperEndY = 480;
        if (upperEndX <= upperMinX || upperEndY <= upperMinY ||
            lowerEndX <= lowerMinX || lowerEndY <= lowerMinY)
            return 0;
        xs[0] = lowerMinX; ys[0] = lowerMinY;
        ws[0] = (uint16)(lowerEndX - lowerMinX);
        hs[0] = (uint16)(lowerEndY - lowerMinY);
        xs[1] = upperMinX; ys[1] = upperMinY;
        ws[1] = (uint16)(upperEndX - upperMinX);
        hs[1] = (uint16)(upperEndY - upperMinY);
        return grSaveCleanBgRectsSplit(xs, ys, ws, hs, 2, cap) > 0;
    } else {
        sint16 xs[1]; sint16 ys[1]; uint16 ws[1]; uint16 hs[1];
        if (lowerEndX <= lowerMinX || lowerEndY <= lowerMinY)
            return 0;
        xs[0] = lowerMinX; ys[0] = lowerMinY;
        ws[0] = (uint16)(lowerEndX - lowerMinX);
        hs[0] = (uint16)(lowerEndY - lowerMinY);
        return grSaveCleanBgRectsSplit(xs, ys, ws, hs, 1, cap) > 0;
    }
}

/* ===================================================================
 * Band setup (retained stable shape, boot allocation order).
 * =================================================================== */
static unsigned long cacheUsed(void)
{ return (unsigned long)memRegionUsed((unsigned)MEM_REGION_CACHE); }

/* MEM_REPRO_SEGREGATED=1 lays the band in the FIXED order — walk PSB
 * reserved AFTER the sheets, contiguous with the SCR cache at the top
 * (the scene-945 segregation fix). Default = the old interleaved order
 * (walk early/low) that strands. */
static int g_segregated = 0;
static void establishBand(void)
{
    if (!g_segregated) {
        g_walkSlab = (uint16 *)memAlloc(MEM_REGION_CACHE, PS1_WALK_PSB_BYTES, "johnwalk_spu_load");
        g_walkSlabBytes = PS1_WALK_PSB_BYTES + 4u;
    }
    g_streamWindow = (uint16 *)memAlloc(MEM_REGION_CACHE,
                                        PS1_STREAM_WINDOW_BYTES, "fg-stream-window");
    g_streamWindowBytes = PS1_STREAM_WINDOW_BYTES + 4u;

    /* two floor slabs via the REAL prepark routine (alloc + park). */
    grPreparkCleanRectSlabs(2, GR_CLEAN_SLAB_FLOOR_BYTES);

    g_frameBuf = (uint8 *)memAlloc(MEM_REGION_CACHE, PS1_FRAME_BYTES, "fg-frame");
    g_frameBufSize = PS1_FRAME_BYTES;
    g_prefetch = (uint8 *)memAlloc(MEM_REGION_CACHE, PS1_PREFETCH_BYTES, "fg-prefetch-frame");
    g_scratch = (uint8 *)memAlloc(MEM_REGION_CACHE, PS1_SCRATCH_BYTES, "fg-stream-scratch");
    g_scratchSize = PS1_SCRATCH_BYTES;
    g_backgrnd = (uint8 *)memAlloc(MEM_REGION_CACHE, PS1_BACKGRND_PSB_BYTES, "cdrom_read_result");
    g_holiday = (uint8 *)memAlloc(MEM_REGION_CACHE, PS1_HOLIDAY_PSB_BYTES, "cdrom_read_result");

    if (g_segregated) {   /* walk reserved last -> adjacent to SCR (the fix) */
        g_walkSlab = (uint16 *)memAlloc(MEM_REGION_CACHE, PS1_WALK_PSB_BYTES, "johnwalk_spu_load");
        g_walkSlabBytes = PS1_WALK_PSB_BYTES + 4u;
    }
    gFullScreenScrCache = (uint8 *)memAlloc(MEM_REGION_CACHE,
                                            PS1_SCR_CACHE_BYTES, "gr-scr-cache");
    gFullScreenScrCacheBytes = PS1_SCR_CACHE_BYTES;
    gFullScreenScrCacheValid = 1;
    strcpy(gFullScreenScrCacheName, "OCEAN");
}

/* Teardown to pristine + re-establish the band — the body of the
 * scheduled rebuild, also reused to reset the engine between fuzz
 * soaks. Frees every CACHE resident, rewinds (O(1) defrag), re-builds
 * the canonical stable shape. */
static void engineTeardownRebuild(void)
{
    grSetCleanBgRectsForceCache(0);
    grFreeCleanBgRects();              /* release LIVE clean-rect pixels */
    grSetCleanBgRectsSlabRetain(0);   /* flushes the pool on disable */
    grCleanRectSlabPoolFlush();        /* belt-and-suspenders: floors too */
    if (g_streamWindow) { memFree(MEM_REGION_CACHE, g_streamWindow); g_streamWindow = NULL; g_streamWindowBytes = 0; }
    if (g_walkSlab)     { memFree(MEM_REGION_CACHE, g_walkSlab); g_walkSlab = NULL; g_walkSlabBytes = 0; }
    if (g_frameBuf)     { memFree(MEM_REGION_CACHE, g_frameBuf); g_frameBuf = NULL; g_frameBufSize = 0; }
    if (g_scratch)      { memFree(MEM_REGION_CACHE, g_scratch); g_scratch = NULL; g_scratchSize = 0; }
    if (g_prefetch)     { memFree(MEM_REGION_CACHE, g_prefetch); g_prefetch = NULL; }
    if (g_backgrnd)     { memFree(MEM_REGION_CACHE, g_backgrnd); g_backgrnd = NULL; }
    if (g_holiday)      { memFree(MEM_REGION_CACHE, g_holiday); g_holiday = NULL; }
    grReliefFreeScrCache();
    memCacheRewindIfEmpty();
    grSetCleanBgRectsSlabRetain(1);
    grSetCleanBgRectsForceCache(g_forceCacheMode);
    establishBand();
}

/* ===================================================================
 * Faithful port of fgMaybeScheduledCacheRebuild (foreground_pilot.c:2300).
 * Teardown: drop retain (flush pool incl. floors), release stream +
 * sheets + walk + SCR + all grow buffers + live clean rects, then
 * memCacheRewindIfEmpty (the O(1) defrag), then re-establish the band —
 * exactly the real begin/done that the soak log shows ending
 * 'rewound=1 largest=688128'. Same three triggers + 20-scene cooldown. */
#define FG_REBUILD_SCENE_CAP 40
static int g_noRebuild = 0;   /* MEM_REPRO_NOREBUILD=1 disables the scheduled rebuild */
static void fgMaybeScheduledCacheRebuild(void)
{
    size_t largest;
    int streak, scrAbsent;

    if (g_noRebuild) return;
    g_scenesSinceRebuild++;
    if (g_rebuildCooldown > 0) { g_rebuildCooldown--; return; }

    largest   = memCacheLargestFreeBlock();
    streak    = g_scrFailStreak;
    scrAbsent = (grScrCacheResidentBytes() == 0);

    if (!((scrAbsent && streak >= 3 && largest < 160u * 1024u) ||
          (g_reliefSinceRebuild >= 2 && largest < 98304u) ||
          (g_scenesSinceRebuild >= FG_REBUILD_SCENE_CAP)))
        return;

#if PS1_MEM_FORENSICS
    printf("JCMEM rebuild-begin largest=%lu streak=%d\n",
           (unsigned long)largest, streak);
#endif
    engineTeardownRebuild();
#if PS1_MEM_FORENSICS
    printf("JCMEM rebuild-done rewound=1 largest=%lu\n",
           (unsigned long)memCacheLargestFreeBlock());
#endif
    g_reliefSinceRebuild = 0;
    g_scenesSinceRebuild = 0;
    g_rebuildCooldown    = 20;
    g_scrFailStreak      = 0;
    g_rebuilds++;
}

static void scrCacheTryReadmit(void)
{
    if (gFullScreenScrCache != NULL) return;
    void *p = memTryAlloc(MEM_REGION_CACHE, PS1_SCR_CACHE_BYTES, "gr-scr-cache");
    if (p != NULL) {
        gFullScreenScrCache = (uint8 *)p;
        gFullScreenScrCacheBytes = PS1_SCR_CACHE_BYTES;
        gFullScreenScrCacheValid = 1;
        strcpy(gFullScreenScrCacheName, "OCEAN");
        g_scrFailStreak = 0;       /* re-hosted: streak resets */
    } else {
        g_scrFailStreak++;         /* grScrCacheRefillFailStreak() climbs */
    }
}

static void walkSlabTryReadmit(void)
{
    if (g_walkSlab != NULL) return;
    void *p = memTryAlloc(MEM_REGION_CACHE, PS1_WALK_PSB_BYTES, "johnwalk_spu_load");
    if (p != NULL) { g_walkSlab = (uint16 *)p; g_walkSlabBytes = PS1_WALK_PSB_BYTES + 4u; }
}

/* per-scene clean-rect cap override (ps1_mem_model.h PS1_SCENES). */
static uint32 sceneCap(const char *name, int lowtide)
{
    /* test hook: emulate the stale-cap leak (walkstuf1-high left 64K and
     * a staged-hit building7 inherited it instead of resetting to 96K). */
    if (getenv("MEM_REPRO_CAP64")) return 64u * 1024u;
    if (strcmp(name, "visitor3") == 0 && !lowtide) return 64u * 1024u;
    if (strcmp(name, "building2") == 0 && lowtide)  return 80u * 1024u;
    return 96u * 1024u;
}

/* TRANSIENT clean-rect headroom at snapshot time. MEASURED from the real
 * soak's cache-alloc-big distribution: clean-rect strips of 98304/81920/
 * 65536 spill to CACHE (grCleanRectPixels), but NO strip <=49152 ever
 * does — i.e. TRANSIENT had room for a <=49K strip but not a >=64K one at
 * snapshot. So the snapshot-time free window sits in [49152, 65536). We
 * occupy TRANSIENT to leave this headroom, then the REAL TRANSIENT-first
 * routing in grSaveCleanBgRectsSplit spills exactly the >=64K strips —
 * reproducing the measured envelope without force-cache or guesswork. */
static unsigned g_transientHeadroom = 57344u;   /* 56K: >=64K spills, <=49K fits */

/* Fill TRANSIENT (per-scene working set: pack data, frames, prefetch the
 * real engine loads) until under ~headroom remains, so the REAL
 * grSaveCleanBgRectsSplit TRANSIENT-first routing spills strips >=64K to
 * CACHE — the measured envelope. memRegionUsed(TRANSIENT) accounting and
 * the per-alloc cap are quirky (bump-down, ~655K usable, libc fallback),
 * so fill GREEDILY by probing instead of computing from budget: keep
 * grabbing chunks while a headroom-sized alloc still succeeds. TRANSIENT
 * is wiped by memSceneReset each scene, so no tracking/free needed. */
static void occupyTransientToHeadroom(void)
{
    /* NOTE: memTryAlloc ignores its region arg and always hits CACHE
     * (it's the SCR-refill helper, mem_region.c:453). Use memAlloc with
     * MEM_REGION_TRANSIENT to actually consume the TRANSIENT bump region.
     * Leave g_transientHeadroom free so grSaveCleanBgRectsSplit's
     * TRANSIENT-first routing spills strips >= headroom to CACHE. */
    unsigned long used   = (unsigned long)memRegionUsed((unsigned)MEM_REGION_TRANSIENT);
    unsigned long budget = PS1_TRANSIENT_BUDGET;
    if (used + g_transientHeadroom < budget) {
        unsigned long fill = budget - g_transientHeadroom - used;
        memAlloc(MEM_REGION_TRANSIENT, (size_t)fill, "scene-working-set");
    }
}

/* play one scene through the real allocator/pool/relief/rebuild path. */
static void playScene(const Soak945Scene *s, const Soak945Bounds *b)
{
    memSceneReset("scene");
    grFreeCleanBgRects();              /* park previous overlay (real boundary) */
    fgMaybeScheduledCacheRebuild();    /* scheduled O(1) defrag */
    walkSlabTryReadmit();
    scrCacheTryReadmit();
    grTopUpCleanRectSlabFloor();
    growFrameBuffers((unsigned)b->maxDataSize);
    occupyTransientToHeadroom();       /* real snapshot-time TRANSIENT pressure */
    sceneSaveCleanRects(b->ux + s->posx, b->uy + s->posy,
                        b->uw, b->uh, sceneCap(s->name, s->lowtide));
}

/* ===================================================================
 * Fuzz driver — run millions of randomized scene soaks through the REAL
 * slab pool + allocator + relief + rebuild. A strand surfaces as a halt
 * (longjmp). Returns the scene index that stranded, or -1 if survived.
 * =================================================================== */
static unsigned g_xs;
static unsigned frand(void){ g_xs^=g_xs<<13; g_xs^=g_xs>>17; g_xs^=g_xs<<5; return g_xs; }
static int g_curScene;

static int runFuzzSoak(unsigned seed, int nScenes)
{
    engineTeardownRebuild();           /* reset to pristine band */
    g_reliefSinceRebuild = g_scenesSinceRebuild = 0;
    g_rebuildCooldown = g_scrFailStreak = 0;
    g_xs = seed ? seed : 1u;

    int catalog = (int)(sizeof(SOAK945_SEQ)/sizeof(SOAK945_SEQ[0]));
    if (catalog > SOAK945_BOUNDS_COUNT) catalog = SOAK945_BOUNDS_COUNT;

    g_haltArmed = 1;
    if (setjmp(g_haltJmp)) { g_haltArmed = 0; return g_curScene; }  /* stranded */
    for (int i = 0; i < nScenes; i++) {
        int idx = (int)(frand() % (unsigned)catalog);
        /* Explore the real demand space: randomize island position
         * (VARPOS_OK range, host/story.c) and tide. Position shifts the
         * foreground bbox -> different clean-rect rects -> different strip
         * sizes (the 65536/81920 strips the real soak produced come from
         * specific position/tide combos), and tide flips the cap. This is
         * what makes the fuzzer reach the stranding states a fixed replay
         * never visits. */
        Soak945Scene s = SOAK945_SEQ[idx];
        s.posx = PS1_POS_X_MIN + (int)(frand() % (unsigned)(PS1_POS_X_MAX - PS1_POS_X_MIN + 1));
        s.posy = PS1_POS_Y_MIN + (int)(frand() % (unsigned)(PS1_POS_Y_MAX - PS1_POS_Y_MIN + 1));
        s.lowtide = (int)(frand() & 1u);
        g_curScene = i;
        playScene(&s, &SOAK945_BOUNDS[idx]);
    }
    g_haltArmed = 0;
    return -1;                          /* survived */
}

int main(void)
{
    memInit();

    { const char *e = getenv("MEM_REPRO_SEGREGATED"); if (e && e[0]=='1') g_segregated = 1; }
    establishBand();
    unsigned long steady = cacheUsed();
    printf("=== BAND ESTABLISHED cache_used=%lu (expect 667688) %s ===\n",
           steady, steady == 667688 ? "MATCH" : "MISMATCH");

    grSetCleanBgRectsSlabRetain(1);
    memSetCacheReliefHook(fgCachePressureRelief);

    /* Clean-rect CACHE routing. The live proof routes per-scene
     * clean-rect snapshots TRANSIENT-first (grSaveCleanBgRects), but
     * under the loading-waves proof at depth they spill to CACHE, which
     * is what fragments the retained band. Set MEM_REPRO_FORCE_CACHE=1
     * to force the CACHE route (faithful to the fragmenting soak); the
     * default TRANSIENT route shows the band staying pristine. */
    {
        const char *e = getenv("MEM_REPRO_FORCE_CACHE");
        if (e && e[0] == '1') g_forceCacheMode = 1;
    }
    grSetCleanBgRectsForceCache(g_forceCacheMode);
    printf("=== clean-rect CACHE routing: %s ===\n",
           g_forceCacheMode ? "FORCED (faithful fragmenting path)"
                      : "TRANSIENT-first (default; band stays pristine)");

    /* Fuzz mode: MEM_REPRO_FUZZ=<soaks>[,<scenesEach>[,<seed0>]] runs
     * millions of randomized scene soaks through the REAL pool/allocator
     * and tallies strands. Over-stressed force-cache routing makes this a
     * CONSERVATIVE gate: surviving it implies surviving the milder real
     * spill cadence. */
    {
        { const char *nr = getenv("MEM_REPRO_NOREBUILD"); if (nr && nr[0]=='1') g_noRebuild = 1; }
        /* 945PATH: reconstruct the EXACT scene-945 going-in layout (band
         * in the real map order, 2 floors parked, SCR resident) then play
         * building7-high's clean rect through the real grSaveCleanBgRects
         * — the organic reproduction. Tests a fix when MEM_REPRO_FIX set. */
        if (getenv("MEM_REPRO_945PATH")) {
            extern size_t memCacheLargestFreeBlock(void);
            /* main already established the canonical band + hook; tear it
             * down to pristine so we can build the EXACT going-in order. */
            memSetCacheReliefHook(NULL);
            engineTeardownRebuild();   /* frees all + rewinds + re-establishes... */
            /* ...then free that re-established band back to truly pristine */
            if (g_streamWindow){memFree(MEM_REGION_CACHE,g_streamWindow);g_streamWindow=NULL;}
            if (g_walkSlab){memFree(MEM_REGION_CACHE,g_walkSlab);g_walkSlab=NULL;}
            if (g_frameBuf){memFree(MEM_REGION_CACHE,g_frameBuf);g_frameBuf=NULL;g_frameBufSize=0;}
            if (g_scratch){memFree(MEM_REGION_CACHE,g_scratch);g_scratch=NULL;g_scratchSize=0;}
            if (g_prefetch){memFree(MEM_REGION_CACHE,g_prefetch);g_prefetch=NULL;}
            if (g_backgrnd){memFree(MEM_REGION_CACHE,g_backgrnd);g_backgrnd=NULL;}
            if (g_holiday){memFree(MEM_REGION_CACHE,g_holiday);g_holiday=NULL;}
            grReliefFreeScrCache(); grCleanRectSlabPoolFlush(); grFreeCleanBgRects();
            memCacheRewindIfEmpty();
            /* going-in order (tests/fixtures/soak945_goingin.map), user bytes */
            g_streamWindow = (uint16*)memAlloc(MEM_REGION_CACHE, PS1_STREAM_WINDOW_BYTES, "fg-stream-window");
            grPreparkCleanRectSlabs(2, GR_CLEAN_SLAB_FLOOR_BYTES);     /* 2 floors -> pool */
            g_walkSlab = (uint16*)memAlloc(MEM_REGION_CACHE, PS1_WALK_PSB_BYTES, "johnwalk_spu_load");
            g_walkSlabBytes = PS1_WALK_PSB_BYTES+4u;
            g_frameBuf = (uint8*)memAlloc(MEM_REGION_CACHE, PS1_FRAME_BYTES, "fg-frame"); g_frameBufSize=PS1_FRAME_BYTES;
            g_prefetch = (uint8*)memAlloc(MEM_REGION_CACHE, PS1_PREFETCH_BYTES, "fg-prefetch-frame");
            g_scratch  = (uint8*)memAlloc(MEM_REGION_CACHE, PS1_SCRATCH_BYTES, "fg-stream-scratch"); g_scratchSize=PS1_SCRATCH_BYTES;
            g_backgrnd = (uint8*)memAlloc(MEM_REGION_CACHE, PS1_BACKGRND_PSB_BYTES, "cdrom_read_result");
            g_holiday  = (uint8*)memAlloc(MEM_REGION_CACHE, PS1_HOLIDAY_PSB_BYTES, "cdrom_read_result");
            gFullScreenScrCache = (uint8*)memAlloc(MEM_REGION_CACHE, PS1_SCR_CACHE_BYTES, "gr-scr-cache");
            gFullScreenScrCacheBytes=PS1_SCR_CACHE_BYTES; gFullScreenScrCacheValid=1; strcpy(gFullScreenScrCacheName,"OCEAN");
            grSetCleanBgRectsSlabRetain(1); memSetCacheReliefHook(fgCachePressureRelief);
            printf("945PATH going-in: cache_used=%lu largest=%lu (expect 667688 / 12292)\n",
                   cacheUsed(), (unsigned long)memCacheLargestFreeBlock());
            /* building7-high draw bounds (per-entry union) + real island pos */
            Soak945Bounds b = SOAK945_BOUNDS[944];
            Soak945Scene  s = SOAK945_SEQ[944];
            memSceneReset("945path-scene");

            /* THEORY C — demand-gated proactive defrag at the boundary.
             * building7-high's fresh CACHE clean-rect demand is ~3x 64K =
             * 196608 contiguous. If the largest contiguous hole can't
             * serve it, rewind to pristine and re-band MANDATORY blocks
             * only, WITHHOLDING the optional evictables (SCR/walk/HOLIDAY)
             * so the remaining contiguous hole covers the demand. Safe
             * here: it's the boundary, nothing of this scene is live, so
             * memCacheRewindIfEmpty can reach a true O(1) defrag. SCR/walk
             * re-admit next boundary. */
            if (getenv("MEM_REPRO_FIX")) {
                const unsigned long FRESH_DEMAND = 196608u;  /* 3x 64K */
                if ((unsigned long)memCacheLargestFreeBlock() < FRESH_DEMAND) {
                    unsigned long before = (unsigned long)memCacheLargestFreeBlock();
                    grFreeCleanBgRects();
                    if (g_streamWindow){memFree(MEM_REGION_CACHE,g_streamWindow);g_streamWindow=NULL;}
                    if (g_frameBuf){memFree(MEM_REGION_CACHE,g_frameBuf);g_frameBuf=NULL;g_frameBufSize=0;}
                    if (g_scratch){memFree(MEM_REGION_CACHE,g_scratch);g_scratch=NULL;g_scratchSize=0;}
                    if (g_prefetch){memFree(MEM_REGION_CACHE,g_prefetch);g_prefetch=NULL;}
                    if (g_backgrnd){memFree(MEM_REGION_CACHE,g_backgrnd);g_backgrnd=NULL;}
                    if (g_holiday){memFree(MEM_REGION_CACHE,g_holiday);g_holiday=NULL;}
                    if (g_walkSlab){memFree(MEM_REGION_CACHE,g_walkSlab);g_walkSlab=NULL;g_walkSlabBytes=0;}
                    grReliefFreeScrCache(); grCleanRectSlabPoolFlush();
                    memCacheRewindIfEmpty();           /* O(1) defrag -> pristine */
                    /* re-band MANDATORY only; withhold SCR/walk/HOLIDAY */
                    g_streamWindow=(uint16*)memAlloc(MEM_REGION_CACHE,PS1_STREAM_WINDOW_BYTES,"fg-stream-window");
                    grPreparkCleanRectSlabs(2,GR_CLEAN_SLAB_FLOOR_BYTES);
                    g_frameBuf=(uint8*)memAlloc(MEM_REGION_CACHE,PS1_FRAME_BYTES,"fg-frame");g_frameBufSize=PS1_FRAME_BYTES;
                    g_prefetch=(uint8*)memAlloc(MEM_REGION_CACHE,PS1_PREFETCH_BYTES,"fg-prefetch-frame");
                    g_scratch=(uint8*)memAlloc(MEM_REGION_CACHE,PS1_SCRATCH_BYTES,"fg-stream-scratch");g_scratchSize=PS1_SCRATCH_BYTES;
                    g_backgrnd=(uint8*)memAlloc(MEM_REGION_CACHE,PS1_BACKGRND_PSB_BYTES,"cdrom_read_result");
                    gFullScreenScrCache=NULL; gFullScreenScrCacheValid=0;  /* SCR withheld */
                    printf("945PATH FIX: demand=%lu largest %lu -> %lu (withhold SCR/walk/HOLIDAY)\n",
                           FRESH_DEMAND, before, (unsigned long)memCacheLargestFreeBlock());
                }
            }

            growFrameBuffers((unsigned)b.maxDataSize);     /* frame growth (16388->?) */
            occupyTransientToHeadroom();
            g_haltArmed=1;
            if (setjmp(g_haltJmp)) {
                printf("945PATH: BSOD reproduced — building7 clean rect stranded (reason=%s)\n", g_lastHaltReason);
                printf("  cache_used=%lu have=%lu largest=%lu\n", cacheUsed(),
                       (unsigned long)(MEM_CACHE_BUDGET-cacheUsed()), (unsigned long)memCacheLargestFreeBlock());
                return 0;
            }
            int rc = sceneSaveCleanRects(b.ux+s.posx, b.uy+s.posy, b.uw, b.uh, sceneCap(s.name,s.lowtide));
            printf("945PATH: building7 clean rect SAVED rc=%d nrects=%d cache_used=%lu largest=%lu have=%lu reliefFires=%d\n",
                   rc, grCleanBgRectsCount(), cacheUsed(), (unsigned long)memCacheLargestFreeBlock(),
                   (unsigned long)(MEM_CACHE_BUDGET-cacheUsed()), g_reliefFires);
            printf("945PATH: NO STRAND (building7 placed its clean rect)\n");
            return 0;
        }
        /* PROBE: play one heavy scene (johnny4-class, 4 big strips at the
         * worst position) from the canonical band, tracing every step:
         * TRANSIENT occupancy, per-strip routing, CACHE largest hole. */
        if (getenv("MEM_REPRO_PROBE")) {
            extern size_t memCacheLargestFreeBlock(void);
            Soak945Bounds b = {64,43,403,310,16384};   /* johnny4 draw-bounds */
            Soak945Scene s = {"johnny4","FG/JOHNNY4.FG2",0,0,0,1,2,1,-110,55};
            printf("PROBE pre: cache_used=%lu largest=%lu trans_used=%lu\n",
                   cacheUsed(), (unsigned long)memCacheLargestFreeBlock(),
                   (unsigned long)memRegionUsed((unsigned)MEM_REGION_TRANSIENT));
            memSceneReset("probe");
            grFreeCleanBgRects();
            walkSlabTryReadmit(); scrCacheTryReadmit(); grTopUpCleanRectSlabFloor();
            growFrameBuffers((unsigned)b.maxDataSize);
            occupyTransientToHeadroom();
            printf("PROBE after occupy: trans_used=%lu (budget=%lu, headroom left=%lu)  cache largest=%lu\n",
                   (unsigned long)memRegionUsed((unsigned)MEM_REGION_TRANSIENT), (unsigned long)PS1_TRANSIENT_BUDGET,
                   (unsigned long)(PS1_TRANSIENT_BUDGET - memRegionUsed((unsigned)MEM_REGION_TRANSIENT)),
                   (unsigned long)memCacheLargestFreeBlock());
            g_haltArmed=1;
            if (setjmp(g_haltJmp)) { printf("PROBE: HALT (strand) reason=%s\n", g_lastHaltReason); return 0; }
            int rc = sceneSaveCleanRects(b.ux+s.posx, b.uy+s.posy, b.uw, b.uh, sceneCap(s.name,s.lowtide));
            printf("PROBE post: saved_rc=%d nrects=%d cache_used=%lu largest=%lu reliefFires=%d\n",
                   rc, grCleanBgRectsCount(), cacheUsed(),
                   (unsigned long)memCacheLargestFreeBlock(), g_reliefFires);
            return 0;
        }
        const char *fz = getenv("MEM_REPRO_FUZZ");
        if (fz && fz[0]) {
            long soaks = 0, scenesEach = 945; unsigned seed0 = 1;
            soaks = strtol(fz, NULL, 10);
            const char *c1 = strchr(fz, ','); if (c1) scenesEach = strtol(c1+1, NULL, 10);
            const char *c2 = c1 ? strchr(c1+1, ',') : NULL; if (c2) seed0 = (unsigned)strtoul(c2+1, NULL, 10);
            if (soaks < 1) soaks = 1;
            long strands = 0; long firstStrand = -1;
            for (long k = 0; k < soaks; k++) {
                int r = runFuzzSoak(seed0 + (unsigned)k, (int)scenesEach);
                if (r >= 0) { strands++; if (firstStrand < 0) firstStrand = k; }
            }
            printf("=== FUZZ %s: soaks=%ld scenesEach=%ld -> STRANDS=%ld (%.4f%%)%s ===\n",
                   g_forceCacheMode ? "force-cache(conservative)" : "transient-first",
                   soaks, scenesEach, strands, 100.0 * strands / soaks,
                   strands ? "" : "  [clean]");
            if (strands) printf("    first strand at soak %ld (seed %u)\n",
                                firstStrand, seed0 + (unsigned)firstStrand);
            return strands ? 1 : 0;
        }
    }

    g_haltArmed = 1;
    int halted = 0;
    if (setjmp(g_haltJmp)) {
        printf("\n*** HALT during replay: scene=%s reason=%s ***\n",
               g_lastHaltScene, g_lastHaltReason);
        halted = 1;
    }

    int n = (int)(sizeof(SOAK945_SEQ)/sizeof(SOAK945_SEQ[0]));
    if (n > SOAK945_BOUNDS_COUNT) n = SOAK945_BOUNDS_COUNT;
    const int TRACE_FROM = n - 25;

    for (int i = 0; i < n && !halted; i++) {
        const Soak945Scene *s = &SOAK945_SEQ[i];
        const Soak945Bounds *b = &SOAK945_BOUNDS[i];

        memSceneReset("scene");
        /* Scene-boundary teardown of the previous scene's clean-rect
         * overlay (foreground_pilot.c:1334 grFreeCleanBgRects): in retain
         * mode this PARKS the previous CACHE-routed slabs into the pool so
         * the next scene reuses them best-fit — the mechanism that keeps
         * the real region compact (largest=12292 steady) and relief rare.
         * Without it the pool starves and every scene allocates fresh. */
        grFreeCleanBgRects();
        fgMaybeScheduledCacheRebuild();   /* scheduled O(1) defrag (real cadence) */
        walkSlabTryReadmit();
        scrCacheTryReadmit();
        grTopUpCleanRectSlabFloor();
        /* grow-only frame buffer + scratch (real per-scene CACHE churn) */
        growFrameBuffers((unsigned)b->maxDataSize);
        occupyTransientToHeadroom();   /* measured snapshot-time TRANSIENT pressure */

        int fgX = b->ux + s->posx;
        int fgY = b->uy + s->posy;
        int fgW = b->uw;
        int fgH = b->uh;
        uint32 cap = sceneCap(s->name, s->lowtide);

        int trace = (i >= TRACE_FROM);
        if (trace)
            printf("\n--- scene %d (%s %s low=%d raft=%d pos=%d,%d) "
                   "cache_used=%lu largestFree=%lu ---\n",
                   i + 1, s->name, s->pack, s->lowtide, s->raft,
                   s->posx, s->posy, cacheUsed(),
                   (unsigned long)memCacheLargestFreeBlock());

        if (i == 944) {
            printf("\n############ SCENE 945 (building7) PRE-SETUP ############\n");
            printf("cache_used=%lu totalFree=%lu largestFree=%lu\n",
                   cacheUsed(), (unsigned long)(MEM_CACHE_BUDGET - cacheUsed()),
                   (unsigned long)memCacheLargestFreeBlock());
            memDumpCacheMap("scene945-pre");
        }

        int ok = sceneSaveCleanRects(fgX, fgY, fgW, fgH, cap);

        if (trace)
            printf("    saved=%d count=%d cache_used=%lu largestFree=%lu "
                   "totalFree=%lu reliefFires=%d\n",
                   ok, grCleanBgRectsCount(), cacheUsed(),
                   (unsigned long)memCacheLargestFreeBlock(),
                   (unsigned long)(MEM_CACHE_BUDGET - cacheUsed()),
                   g_reliefFires);

        if (i == 944) {
            unsigned long largest   = (unsigned long)memCacheLargestFreeBlock();
            unsigned long totalFree = (unsigned long)(MEM_CACHE_BUDGET - cacheUsed());
            printf("\n############ SCENE 945 65536-REQUEST PROBE ############\n");
            printf("cache_used=%lu totalFree=%lu largestContig=%lu req=65536\n",
                   cacheUsed(), totalFree, largest);
            printf("STRAND=%s (largest %s 65536)\n",
                   largest < 65536 ? "YES" : "no",
                   largest < 65536 ? "<" : ">=");
            printf("have==88024 ? %s   cache_used==600104 ? %s\n",
                   totalFree == 88024 ? "YES" : "no",
                   cacheUsed() == 600104 ? "YES" : "no");
            memDumpCacheMap("scene945-post");
        }
    }

    g_haltArmed = 0;
    printf("\n=== REPLAY DONE  reliefFires=%d  finalHalt=%s ===\n",
           g_reliefFires, halted ? "YES" : "no");
    return 0;
}
