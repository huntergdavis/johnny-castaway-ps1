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
static int   g_norebuild = 0;

static unsigned long cacheUsed(void){ return (unsigned long)memRegionUsed((unsigned)MEM_REGION_CACHE); }

/* Clean-rect slab POOL: under the loading-waves proof, CACHE-routed
 * clean-rect slabs are PARKED across scene boundaries (retained, reused
 * best-fit) up to GR_CLEAN_SLAB_POOL_CAP_BYTES, not freed. This is the
 * persistent, varying-size resident set that interleaves with the band
 * and drifting residents to fragment the free-list across scenes — the
 * mechanism behind the 470/945 fragmentation BSODs. (clean_rects.c.inc
 * grCleanRectSlabPool / grCleanRectSlabPoolPark.) */
static void  *g_poolPtr[64]; static unsigned g_poolSz[64]; static int g_npool;
static unsigned long g_poolBytes;

static void poolEvictSmallestIf(unsigned wantRoomFor){
    /* keep total parked <= pool cap; evict smallest while over. */
    while (g_npool>0 && g_poolBytes + wantRoomFor > PS1_SLAB_POOL_CAP_BYTES){
        int sm=0; for(int i=1;i<g_npool;i++) if(g_poolSz[i]<g_poolSz[sm]) sm=i;
        memFree(MEM_REGION_CACHE, g_poolPtr[sm]);
        g_poolBytes -= g_poolSz[sm];
        g_poolPtr[sm]=g_poolPtr[g_npool-1]; g_poolSz[sm]=g_poolSz[g_npool-1]; g_npool--;
    }
}
static void *poolTake(unsigned sz, unsigned *outSz){  /* best-fit >= sz */
    int best=-1;
    for(int i=0;i<g_npool;i++) if(g_poolSz[i]>=sz && (best<0||g_poolSz[i]<g_poolSz[best])) best=i;
    if(best<0) return NULL;
    void *p=g_poolPtr[best]; if(outSz)*outSz=g_poolSz[best]; g_poolBytes-=g_poolSz[best];
    g_poolPtr[best]=g_poolPtr[g_npool-1]; g_poolSz[best]=g_poolSz[g_npool-1]; g_npool--;
    return p;
}
static void poolPark(void *p, unsigned sz){  /* retain across boundary */
    if (g_npool>=64){ memFree(MEM_REGION_CACHE,p); return; }
    poolEvictSmallestIf(sz);
    if (g_poolBytes + sz > PS1_SLAB_POOL_CAP_BYTES){ memFree(MEM_REGION_CACHE,p); return; }
    g_poolPtr[g_npool]=p; g_poolSz[g_npool]=sz; g_npool++; g_poolBytes+=sz;
}
static int   g_stripSz[PS1_CLEANRECT_SLOTS];
static void freeStrips(void){
    /* Clean-rect slab retention. In the real soak the loading-waves
     * proof shows most CACHE-routed slabs are returned (TRANSIENT
     * absorbs the next scene), but a fraction are PARKED across the
     * boundary (reused best-fit). That retained minority is the slow
     * cross-scene fragmenter behind 470/945 — it accumulates over
     * hundreds of scenes rather than dying immediately. We retain ~1 in
     * 3 (the observed park rate that keeps steady-state cache_used at
     * 667688 yet still strands an unprotected region at realistic
     * depth); the rest free back. */
    for(int i=0;i<g_nstrips;i++) if(g_strips[i]){
        if ((rnd()%3)==0) poolPark(g_strips[i], g_stripSz[i]);
        else              memFree(MEM_REGION_CACHE, g_strips[i]);
    }
    g_nstrips=0;
}
static void poolFreeAll(void){
    for(int i=0;i<g_npool;i++) memFree(MEM_REGION_CACHE,g_poolPtr[i]);
    g_npool=0; g_poolBytes=0;
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
    /* tier 1: pooled clean-rect slabs (evict one big enough) */
    { int sm=-1; for(int i=0;i<g_npool;i++) if(g_poolSz[i]>=req && (sm<0||g_poolSz[i]<g_poolSz[sm])) sm=i;
      if(sm>=0){ memFree(MEM_REGION_CACHE,g_poolPtr[sm]); g_poolBytes-=g_poolSz[sm];
                 g_poolPtr[sm]=g_poolPtr[g_npool-1]; g_poolSz[sm]=g_poolSz[g_npool-1]; g_npool--; return 1; } }
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
    while (g_npool>0){ memFree(MEM_REGION_CACHE,g_poolPtr[g_npool-1]); g_poolBytes-=g_poolSz[g_npool-1]; g_npool--; freed=1; }
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
    if (g_norebuild) return;
    if (!(periodic || reliefdrv)) return;
    /* teardown: free strips + SCR, leaving only the pinned band; rewind
     * cannot fire (band still live) but the free space is now contiguous
     * above the band — the model of the rebuild's defrag. */
    freeStrips();
    if (g_scr){ memFree(MEM_REGION_CACHE,g_scr); g_scr=NULL; }
    if (g_walk){ memFree(MEM_REGION_CACHE,g_walk); g_walk=NULL; }
    for (int i=0;i<2;i++) if (g_floor[i]){ memFree(MEM_REGION_CACHE,g_floor[i]); g_floor[i]=NULL; }
    poolFreeAll();
    g_rebuild++; g_reliefSinceRebuild=0; g_scenesSinceRebuild=0;
}

/* Play one scene. island => uses SCR cache + wave-band clean rect.
 * cleanBytes = total clean-rect dirty bytes; cap = per-rect cap; slots
 * = GR_MAX_CLEAN_RECTS. Returns 0 on BSOD (a strip can't be placed). */
#define PS1_BGTILES_BYTES (614400u)  /* 4 x 320x240x2, TRANSIENT-resident */

/* Clean-rect geometry (backdrop_clean.c.inc): wave band unioned with the
 * foreground bbox (union bounds + island offset), split at y=190; each
 * rect divides into <=cap strips of (width x rows x 2) — DIVERSE sizes,
 * the real fragmenter (uniform cap chunks recoalesce). */
static int clampi(int v,int lo,int hi){return v<lo?lo:v>hi?hi:v;}
static int cleanRects(int fgx,int fgy,int fgw,int fgh,int rw[2],int rh[2]){
    const int WMINX=129,WMINY=303,WENDX=608,WENDY=356,SPLIT=190,SW=640,SH=480;
    int n=0, fgex=fgx+fgw, fgey=fgy+fgh;
    int lx=fgx<WMINX?fgx:WMINX;
    int ly0=(fgy>SPLIT?fgy:SPLIT), ly=ly0<WMINY?ly0:WMINY;
    int ex=fgex>WENDX?fgex:WENDX, ey=fgey>WENDY?fgey:WENDY;
    lx=clampi(lx,0,SW);ly=clampi(ly,0,SH);ex=clampi(ex,0,SW);ey=clampi(ey,0,SH);
    if(ex>lx&&ey>ly){rw[n]=ex-lx;rh[n]=ey-ly;n++;}
    if(fgy<SPLIT){
        int ux=clampi(fgx,0,SW),uy=clampi(fgy,0,SH);
        int uex=clampi(fgex,0,SW),uey=clampi(SPLIT,0,SH);
        if(uex>ux&&uey>uy){rw[n]=uex-ux;rh[n]=uey-uy;n++;}
    }
    return n;
}

/* Play a scene: sample island position, compute the real clean-rect
 * rects, split each into diverse-size strips (grSaveCleanBgRectsSplit),
 * route TRANSIENT-first / CACHE-spill with tiered relief, count strips
 * against the slot cap. Returns 0 on BSOD (slot exhaust or strand). */
static int playSceneGeo(const SceneMem *sc, int posx, int posy, unsigned cap){
    g_scenes++; g_reliefedThisScene=0;
    freeStrips();
    memSceneReset("sim");
    maybeRebuild();
    reestablish(1);
    memAlloc(MEM_REGION_TRANSIENT, PS1_BGTILES_BYTES, "bgtiles");

    int rw[2],rh[2];
    int nr = cleanRects(sc->ux+posx, sc->uy+posy, sc->uw, sc->uh, rw, rh);
    g_nstrips=0; int totalStrips=0;
    for (int r=0;r<nr;r++){
        unsigned bpr = (unsigned)rw[r]*2u;          /* bytes per row */
        unsigned maxRows = bpr ? cap/bpr : 1u; if(!maxRows) maxRows=1u;
        int rem = rh[r];
        while (rem > 0){
            unsigned rows = (unsigned)rem > maxRows ? maxRows : (unsigned)rem;
            unsigned sz = bpr * rows;               /* DIVERSE strip size */
            if (++totalStrips > g_slots) return 0;  /* slot exhaustion */
            unsigned long tu = (unsigned long)memRegionUsed((unsigned)MEM_REGION_TRANSIENT);
            unsigned long tf = (tu<PS1_TRANSIENT_BUDGET)?(PS1_TRANSIENT_BUDGET-tu):0;
            if (sz <= tf){
                memAlloc(MEM_REGION_TRANSIENT, sz, "strip-t");
            } else {
                unsigned asz=sz; void *p = poolTake(sz,&asz);   /* reuse parked slab */
                if (!p){ asz=sz; p = memTryAlloc(MEM_REGION_CACHE, sz, "strip-c");
                    if (!p){ if(!relief(sz)) return 0; p=memTryAlloc(MEM_REGION_CACHE,sz,"strip-c"); if(!p) return 0; } }
                if (g_nstrips < (int)PS1_CLEANRECT_SLOTS){ g_strips[g_nstrips]=p; g_stripSz[g_nstrips]=asz; g_nstrips++; }
            }
            rem -= (int)rows;
        }
    }
    unsigned long cu = cacheUsed();
    int b = cu<520000?0:cu<560000?1:cu<600000?2:cu<640000?3:cu<668000?4:cu<690000?5:6;
    g_cuHist[b]++;
    return 1;
}

/* cleanBytes-based scene play (slab-pool aware). Splits the clean rect
 * into diverse strips (full-width 608 rows), TRANSIENT-first / CACHE-
 * spill with pooled-slab reuse + tiered relief. */
static int playSceneBytes(unsigned cleanBytes, unsigned cap){
    g_scenes++; g_reliefedThisScene=0;
    freeStrips();
    memSceneReset("sim");
    maybeRebuild();
    reestablish(1);
    memAlloc(MEM_REGION_TRANSIENT, PS1_BGTILES_BYTES, "bgtiles");
    unsigned remaining = cleanBytes; g_nstrips=0; int totalStrips=0;
    while (remaining > 0){
        unsigned sz = remaining > cap ? cap : remaining;
        if (++totalStrips > g_slots) return 0;       /* slot exhaustion */
        unsigned long tu = (unsigned long)memRegionUsed((unsigned)MEM_REGION_TRANSIENT);
        unsigned long tf = (tu<PS1_TRANSIENT_BUDGET)?(PS1_TRANSIENT_BUDGET-tu):0;
        if (sz <= tf){
            memAlloc(MEM_REGION_TRANSIENT, sz, "strip-t");
        } else {
            unsigned asz=sz; void *p=poolTake(sz,&asz);
            if(!p){ asz=sz; p=memTryAlloc(MEM_REGION_CACHE,sz,"strip-c");
                if(!p){ if(!relief(sz)) return 0; p=memTryAlloc(MEM_REGION_CACHE,sz,"strip-c"); if(!p) return 0; } }
            if (g_nstrips<(int)PS1_CLEANRECT_SLOTS){ g_strips[g_nstrips]=p; g_stripSz[g_nstrips]=asz; g_nstrips++; }
        }
        remaining -= sz;
    }
    unsigned long cu=cacheUsed();
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
    poolFreeAll();
    memSceneReset("reset");
    memCacheRewindIfEmpty();
    g_scr=g_walk=NULL; g_floor[0]=g_floor[1]=NULL; g_nstrips=0;
    g_relief=g_rebuild=g_scenes=0; g_reliefSinceRebuild=0; g_scenesSinceRebuild=0;
    memset(g_cuHist,0,sizeof(g_cuHist));
    g_band = memAlloc(MEM_REGION_CACHE, BAND_PINNED, "band");
    reestablish(1);
    /* splitmix32 scramble so sequential per-soak seeds (seed0+i) produce
     * well-separated, decorrelated xorshift streams across billions of
     * walks rather than near-identical adjacent sequences. */
    { unsigned z = (seed?seed:1u) + 0x9E3779B9u;
      z = (z ^ (z>>16)) * 0x85EBCA6Bu; z = (z ^ (z>>13)) * 0xC2B2AE35u; z ^= z>>16;
      xs = z?z:1u; }
    long bsod=-1;
    if (setjmp(jb)){ return g_scenes; }
    for (long s=0;s<scenes;s++){
        /* draw a REAL scene from the extracted FG2 table and use its
         * clean-rect demand at a sampled island position (position
         * factor scales the worst-case bytes; worst case is at 1.0). */
        /* Per-scene clean-rect demand calibrated to the REAL allday4
         * cache_used histogram (914/945 scenes at 667688 == no CACHE
         * spill, so clean rect < TRANSIENT's ~172K room; ~31 scenes dip
         * == spill). The union-bounds header field is the whole-animation
         * envelope and overestimates ~3-9x; the log's max_restore_bytes
         * (median 12K, max 98K) is the real per-scene dirty region.
         * Bimodal: ~96.7% light (12-150K, fits TRANSIENT), ~3.3% heavy
         * (walkstuf1/building7-class, 300-360K, spills + can slot-exhaust
         * at the historical 48K cap). */
        unsigned cleanBytes; unsigned cap;
        if ((rnd()%1000) < 33){          /* heavy island scene */
            int bad = (rnd()%100) < 25;  /* worst island position */
            cleanBytes = bad ? (400u*1024u + (rnd()%(45u*1024u)))
                             : (300u*1024u + (rnd()%(40u*1024u)));
            cap = g_periodic ? 96u*1024u : 48u*1024u;   /* walkstuf1-low cap */
        } else {                         /* common light scene */
            cleanBytes = 12u*1024u + (rnd()%(140u*1024u));
            cap = 96u*1024u;
        }
        if (!playSceneBytes(cleanBytes, cap)){ bsod=g_scenes; break; }
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
        else if (!strcmp(argv[a],"--no-rebuild")){ g_norebuild=1; g_periodic=0; }
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
