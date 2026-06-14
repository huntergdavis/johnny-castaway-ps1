/*
 * mem_sim.c — scene-sequence memory simulator over the REAL allocator
 * (src/mem_region.c). Plays a stream of scenes through the genuine
 * CACHE free-list + relief + rebuild logic and reproduces the
 * observable memory behaviour of a real soak: the cache_used
 * oscillation, relief cadence, rebuild cadence, and the BSOD.
 *
 * Model (all sizes from tests/ps1_mem_model.h, sourced file:line):
 *   - Retained band minus SCR is one pinned block: 514084 bytes
 *     (walk+window+2 floors+3 frames+BACKGRND+HOLIDAY) — the value the
 *     real log shows when the SCR cache is absent.
 *   - SCR cache (153604 block) is resident for island scenes; pinned+SCR
 *     == 667688, the real steady-state cache_used.
 *   - Each scene allocates clean-rect strips (split by its cap); island
 *     scenes also (re)admit the SCR cache. Under pressure, relief evicts
 *     the SCR cache (cache_used -> 514084) then re-admits next island
 *     scene (-> 667688) — the exact oscillation in the real log.
 *   - The scheduled rebuild fires on the modeled triggers.
 *
 * Determinism: a per-run xorshift seed picks scene + island position, so
 * millions of distinct soak sequences run in seconds. With the logged
 * real sequence it replays a specific soak.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include "../src/mem_region/mem_region.h"
#include "ps1_mem_model.h"
#include "scene_mem_table.h"  /* real per-scene clean-rect demand (extract-scene-mem.py) */

int graphicsIsInitialized(void){return 0;}
void checkMemoryBudget(void){}
void lruEvictAllUnpinned(void){}
size_t getTotalMemoryUsed(void){return 0;}
static jmp_buf jb;
void ps1DebugInit(void){}
void ps1DebugError(const char*f,...){(void)f;longjmp(jb,1);}
void ps1Bsod(const char*s,const char*r,const char*ff,int l){(void)s;(void)r;(void)ff;(void)l;longjmp(jb,1);}
extern size_t memCacheLargestFreeBlock(void);

/* Pinned band (no relief tier frees these): BACKGRND 94212 + HOLIDAY
 * 26628 + window 98308 + 3 frames 49164 = 268312. */
#define BAND_PINNED 268312u

static unsigned xs;
static unsigned rnd(void){ xs^=xs<<13; xs^=xs>>17; xs^=xs<<5; return xs; }

/* engine state */
static void *g_band;            /* pinned 268312 */
static void *g_scr;             /* SCR cache 153604 (evictable, island) */
static void *g_walk;            /* walk PSB slab 49156 (evictable) */
static void *g_floor[2];        /* floor slabs 98308 each (evictable) */
static void *g_strips[PS1_CLEANRECT_SLOTS];
static int   g_nstrips;
static long  g_relief, g_rebuild, g_scenes;
static long  g_reliefSinceRebuild, g_scenesSinceRebuild;
static long  g_cuHist[8];
static int   g_slots = (int)PS1_CLEANRECT_SLOTS;
static int   g_periodic = 1;

static unsigned long cacheUsed(void){ return (unsigned long)memRegionUsed((unsigned)MEM_REGION_CACHE); }

static void freeStrips(void){
    for(int i=0;i<g_nstrips;i++) if(g_strips[i]) memFree(MEM_REGION_CACHE,g_strips[i]);
    g_nstrips=0;
}

/* Faithful tiered relief (fgCachePressureRelief): free cheapest-first,
 * each tier only if its yield >= request, until the retry can succeed.
 * Returns 1 if anything was freed. */
static int g_reliefedThisScene;
static int relief(unsigned req){
    int freed=0;
    /* count one relief EVENT per scene that needs it (matches the real
     * log's per-scene relief cadence), though it may free several tiers. */
    if (!g_reliefedThisScene){ g_relief++; g_reliefSinceRebuild++; g_reliefedThisScene=1; }
    /* tier 2: walk slab (49156) */
    if (g_walk && 49156u >= req){ memFree(MEM_REGION_CACHE,g_walk); g_walk=NULL; return 1; }
    /* tier 3: floor slabs (98308) when req <= 98304 */
    if (req <= 98304u){
        for (int i=0;i<2;i++) if (g_floor[i]){ memFree(MEM_REGION_CACHE,g_floor[i]); g_floor[i]=NULL; return 1; }
    }
    /* tier 4: SCR cache (153604) */
    if (g_scr && 153604u >= req){ memFree(MEM_REGION_CACHE,g_scr); g_scr=NULL; return 1; }
    /* last resort: free everything evictable cheapest-first */
    if (g_walk){ memFree(MEM_REGION_CACHE,g_walk); g_walk=NULL; freed=1; }
    for (int i=0;i<2;i++) if (g_floor[i]){ memFree(MEM_REGION_CACHE,g_floor[i]); g_floor[i]=NULL; freed=1; }
    if (g_scr){ memFree(MEM_REGION_CACHE,g_scr); g_scr=NULL; freed=1; }
    return freed;
}

/* Re-establish the evictable stable-shape residents at a scene boundary
 * (the real reserve/re-admit): floors + walk always, SCR for island
 * scenes. memTryAlloc so a tight region just leaves them absent. */
static void reestablish(int island){
    for (int i=0;i<2;i++) if(!g_floor[i]) g_floor[i]=memTryAlloc(MEM_REGION_CACHE,PS1_FLOOR_SLAB_BYTES,"floor");
    if(!g_walk) g_walk=memTryAlloc(MEM_REGION_CACHE,PS1_WALK_PSB_BYTES,"walk");
    if(island && !g_scr) g_scr=memTryAlloc(MEM_REGION_CACHE,PS1_SCR_CACHE_BYTES,"scr");
}

/* Scheduled rebuild: tear the region down to the pinned band and rewind
 * (the real rebuild's O(1) defrag), mirroring the trigger conditions. */
static void maybeRebuild(void){
    unsigned long largest;
    if (g_scenesSinceRebuild < 0) return;
    g_scenesSinceRebuild++;
    largest = memCacheLargestFreeBlock();
    int periodic = g_periodic && (g_scenesSinceRebuild >= PS1_REBUILD_SCENE_CAP);
    int reliefdrv = (g_reliefSinceRebuild >= PS1_REBUILD_RELIEF_MIN &&
                     largest < PS1_REBUILD_RELIEF_LARGEST);
    if (!(periodic || reliefdrv)) return;
    /* teardown: free strips + SCR, leaving only the pinned band; rewind
     * cannot fire (band still live) but the free space is now contiguous
     * above the band — the model of the rebuild's defrag. */
    freeStrips();
    if (g_scr){ memFree(MEM_REGION_CACHE,g_scr); g_scr=NULL; }
    if (g_walk){ memFree(MEM_REGION_CACHE,g_walk); g_walk=NULL; }
    for (int i=0;i<2;i++) if (g_floor[i]){ memFree(MEM_REGION_CACHE,g_floor[i]); g_floor[i]=NULL; }
    g_rebuild++; g_reliefSinceRebuild=0; g_scenesSinceRebuild=0;
}

/* Play one scene. island => uses SCR cache + wave-band clean rect.
 * cleanBytes = total clean-rect dirty bytes; cap = per-rect cap; slots
 * = GR_MAX_CLEAN_RECTS. Returns 0 on BSOD (a strip can't be placed). */
#define PS1_BGTILES_BYTES (614400u)  /* 4 x 320x240x2, TRANSIENT-resident */

static int playScene(int island, unsigned cleanBytes, unsigned cap){
    g_scenes++; g_reliefedThisScene=0;
    freeStrips();              /* prev scene's CACHE-spill strips released */
    memSceneReset("sim");      /* wipe TRANSIENT (bg-tiles + transient strips) */
    maybeRebuild();
    reestablish(island);       /* re-admit floors/walk/SCR if absent */

    /* bg-tiles occupy TRANSIENT; clean-rect strips route TRANSIENT-first
     * and spill to CACHE only when TRANSIENT lacks room (the real
     * grSaveCleanBgRects dynamic routing). */
    memAlloc(MEM_REGION_TRANSIENT, PS1_BGTILES_BYTES, "bgtiles");

    /* The clean-rect split (grSaveCleanBgRectsSplit) caps the TOTAL
     * strip count at GR_MAX_CLEAN_RECTS regardless of region — exceeding
     * it returns 0 -> the "TRANSIENT budget shortfall" BSOD (walkstuf1
     * @612 was 8 strips of a 336640 region at a 48K cap). */
    unsigned remaining = cleanBytes; g_nstrips=0; int totalStrips=0;
    while (remaining > 0){
        unsigned sz = remaining > cap ? cap : remaining;
        if (++totalStrips > g_slots) return 0;   /* slot exhaustion (all regions) */
        unsigned long transUsed = (unsigned long)memRegionUsed((unsigned)MEM_REGION_TRANSIENT);
        unsigned long transFree = (transUsed < PS1_TRANSIENT_BUDGET) ?
                                  (PS1_TRANSIENT_BUDGET - transUsed) : 0;
        if (sz <= transFree){
            memAlloc(MEM_REGION_TRANSIENT, sz, "strip-t");  /* TRANSIENT-first */
        } else {
            void *p = memTryAlloc(MEM_REGION_CACHE, sz, "strip-c");  /* spill */
            if (!p){
                if (!relief(sz)) return 0;
                p = memTryAlloc(MEM_REGION_CACHE, sz, "strip-c");
                if (!p) return 0;
            }
            if (g_nstrips < (int)PS1_CLEANRECT_SLOTS) g_strips[g_nstrips++] = p;
        }
        remaining -= sz;
    }

    unsigned long cu = cacheUsed();
    int b = cu<520000?0:cu<560000?1:cu<600000?2:cu<640000?3:cu<668000?4:cu<690000?5:6;
    g_cuHist[b]++;
    return 1;
}

/* one full soak; returns the BSOD scene (or -1 if it survived). */
static long runSoak(unsigned seed, long scenes){
    /* reset allocator to pristine between soaks (memInit is called ONCE
     * in main; re-calling it would leak 1.44MB/soak and OOM at scale). */
    freeStrips();
    if (g_scr){ memFree(MEM_REGION_CACHE,g_scr); g_scr=NULL; }
    if (g_walk){ memFree(MEM_REGION_CACHE,g_walk); g_walk=NULL; }
    for (int i=0;i<2;i++) if(g_floor[i]){ memFree(MEM_REGION_CACHE,g_floor[i]); g_floor[i]=NULL; }
    if (g_band){ memFree(MEM_REGION_CACHE,g_band); g_band=NULL; }
    memSceneReset("reset");
    memCacheRewindIfEmpty();
    g_scr=g_walk=NULL; g_floor[0]=g_floor[1]=NULL; g_nstrips=0;
    g_relief=g_rebuild=g_scenes=0; g_reliefSinceRebuild=0; g_scenesSinceRebuild=0;
    memset(g_cuHist,0,sizeof(g_cuHist));
    g_band = memAlloc(MEM_REGION_CACHE, BAND_PINNED, "band");
    reestablish(1);
    xs = seed?seed:1u;
    long bsod=-1;
    if (setjmp(jb)){ return g_scenes; }
    for (long s=0;s<scenes;s++){
        /* draw a REAL scene from the extracted FG2 table and use its
         * clean-rect demand at a sampled island position (position
         * factor scales the worst-case bytes; worst case is at 1.0). */
        const SceneMem *sc = &SCENE_MEM[rnd() % SCENE_MEM_COUNT];
        int island = 1;  /* the heavy clean-rect scenes are the island set */
        unsigned posFactorPct = 35 + (rnd()%66);          /* 35..100% of worst */
        unsigned cleanBytes = (unsigned)((unsigned long)sc->cleanBytes * posFactorPct / 100u);
        /* historical walkstuf/visitor used a 48K/64K cap; the fix set
         * uses 96K for all. */
        unsigned cap = g_periodic ? 96u*1024u
                     : (sc->strips48 > sc->strips64 ? 48u*1024u : 64u*1024u);
        if (!playScene(island, cleanBytes, cap)){ bsod=g_scenes; break; }
    }
    return bsod;
}

int main(int argc, char**argv){
    long scenes = 3000; long soaks = 1; unsigned seed0 = 1;
    for (int a=1;a<argc;a++){
        if (!strcmp(argv[a],"--scenes")&&a+1<argc) scenes=atol(argv[++a]);
        else if (!strcmp(argv[a],"--soaks")&&a+1<argc) soaks=atol(argv[++a]);
        else if (!strcmp(argv[a],"--seed")&&a+1<argc) seed0=(unsigned)strtoul(argv[++a],0,10);
        else if (!strcmp(argv[a],"--historical")){ g_slots=8; g_periodic=0; }
        else if (!strcmp(argv[a],"--slots")&&a+1<argc) g_slots=atoi(argv[++a]);
        else if (a==1) scenes=atol(argv[a]);   /* positional back-comat */
        else if (a==2) seed0=(unsigned)strtoul(argv[a],0,10);
    }
    memInit();
    if (soaks==1){
        long b=runSoak(seed0,scenes);
        printf("config=%s slots=%d periodic=%d  seed=%u scenes=%ld relief=%ld rebuild=%ld bsod_scene=%ld\n",
               g_periodic?"fixed":"historical", g_slots, g_periodic,
               seed0, g_scenes, g_relief, g_rebuild, b);
        printf("cache_used: <520k=%ld <560k=%ld <600k=%ld <640k=%ld 667688=%ld <690k=%ld 690k+=%ld\n",
               g_cuHist[0],g_cuHist[1],g_cuHist[2],g_cuHist[3],g_cuHist[4],g_cuHist[5],g_cuHist[6]);
        return b>=0?1:0;
    }
    /* multi-soak: run `soaks` independent full soaks, count BSODs. */
    long bsods=0, minScene=1L<<30;
    for (long i=0;i<soaks;i++){
        long b=runSoak(seed0+(unsigned)i, scenes);
        if (b>=0){ bsods++; if(b<minScene) minScene=b; }
    }
    printf("config=%s slots=%d periodic=%d  soaks=%ld scenes_each=%ld -> BSODs=%ld (%.4f%%)%s\n",
           g_periodic?"fixed":"historical", g_slots, g_periodic, soaks, scenes,
           bsods, 100.0*bsods/soaks, bsods?"":"  [clean across all soaks]");
    if (bsods) printf("earliest BSOD scene=%ld\n", minScene);
    return bsods?1:0;
}
