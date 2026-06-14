/*
 * mem_region_fuzz.c — host fuzzer over the REAL CACHE allocator
 * (src/mem_region.c) hunting layouts that reproduce the scene-945 class
 * of BSOD: a contiguous request that fails (largest free < req) even
 * though total free >= req, AFTER a relief pass has evicted the
 * optimization-only residents. Runs millions of randomized retained-
 * band layouts in milliseconds each; every reproducing layout's exact
 * op sequence is emitted as a replayable blob for the regression corpus.
 *
 * Build: see scripts/build-mem-fuzz.sh
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "../src/mem_region/mem_region.h"

/* ---- stubs for mem_region.c's externs (host) ---- */
int graphicsIsInitialized(void) { return 0; }
void checkMemoryBudget(void) {}
void lruEvictAllUnpinned(void) {}
size_t getTotalMemoryUsed(void) { return 0; }

static jmp_buf g_haltJmp;
static char    g_haltReason[256];
void ps1DebugInit(void) {}
void ps1DebugError(const char *fmt, ...) {
    if (fmt) { strncpy(g_haltReason, fmt, sizeof(g_haltReason)-1); }
    longjmp(g_haltJmp, 1);
}
void ps1Bsod(const char *s, const char *r, const char *f, int l) {
    (void)s;(void)r;(void)f;(void)l; longjmp(g_haltJmp, 1);
}

extern size_t memCacheLargestFreeBlock(void);

/* The request whose failure defines the scene-945 class (building7's
 * clean-rect spill). */
#define REQ_BYTES 65536u

/* Realistic CACHE block inventory (user bytes) drawn from the soak heap
 * maps. `evictable` = a relief tier can free it; `pinned` survives
 * relief (BACKGRND/HOLIDAY have no tier, frames/window are grow-only). */
typedef struct { unsigned bytes; int evictable; const char *tag; } BlockSpec;
static const BlockSpec INVENTORY[] = {
    {  98304, 1, "floor" },
    {  98304, 1, "floor" },
    {  98304, 0, "window" },
    {  49152, 1, "walk" },
    {  16384, 0, "frame" },
    {  16384, 0, "prefetch" },
    {  16384, 0, "scratch" },
    {  94208, 0, "backgrnd" },
    {  26624, 0, "holiday" },
    { 153600, 1, "scr" },
    /* parked clean-rect slabs (sub-floor, evictable) — sizes the pool
     * actually holds; the fuzzer picks a random subset/order. */
    {  16384, 1, "slab16" },
    {  32768, 1, "slab32" },
    {  49152, 1, "slab48" },
    {  57344, 1, "slab56" },
    {  65536, 1, "slab64" },
    {  81920, 1, "slab80" },
};
#define INV_N ((int)(sizeof(INVENTORY)/sizeof(INVENTORY[0])))

/* One run's op log: the exact block sequence (size + pinned flag) that
 * recreates the layout, so a reproduction is fully replayable. */
#define MAXOPS 96
typedef struct {
    unsigned size[MAXOPS];   /* user bytes allocated, in order */
    int      evict[MAXOPS];  /* 1 = freed by the relief pass */
    int      n;
} RunLog;

static unsigned xs;            /* xorshift RNG (deterministic per seed) */
static unsigned rnd(void){ xs^=xs<<13; xs^=xs>>17; xs^=xs<<5; return xs; }

/* Reset CACHE to pristine between runs by freeing all tracked ptrs and
 * rewinding the bump pointer. */
static void *g_live[64]; static int g_nLive;
static void resetCache(void){
    for (int i=0;i<g_nLive;i++) if (g_live[i]) memFree(MEM_REGION_CACHE, g_live[i]);
    g_nLive=0;
    memCacheRewindIfEmpty();
}

/* Build a randomized layout, simulate relief (free evictables), and
 * test whether REQ_BYTES is satisfiable. Returns 1 if it reproduces the
 * fragmentation failure (total free >= REQ but largest free < REQ). */
/* When set, model the SEGREGATION fix: evictable (relief-droppable)
 * blocks are allocated separately from pinned ones so freeing the
 * evictables yields one contiguous span. Implemented here as a
 * two-pass placement (all pinned, then all evictable) — the address
 * outcome a two-ended/arena allocator would produce. */
static int g_fixSegregate = 0;

static int runOnce(unsigned seed, RunLog *log){
    xs = seed ? seed : 0x9e3779b9u;
    resetCache();
    log->n = 0;

    /* Faithful scene-945 state: the region is nearly full of SUB-64K
     * blocks — building7's already-placed clean-rect strips and the
     * retained band (pinned), interleaved with small parked/evictable
     * slabs. NO freeable >=64K block exists (the floors were consumed
     * into live strips). We fill CACHE to near-full with a random mix,
     * then free the evictables and ask whether a 64K request can still
     * be assembled. Sizes 8K..60K guarantee no single block is itself
     * the >=64K rescue, so reproduction depends purely on whether the
     * freed evictables coalesce — the real coalescing logic decides. */
    /* First decide the block set (sizes + evict flags) deterministically. */
    unsigned sizes[MAXOPS]; int evs[MAXOPS]; int n=0;
    size_t used=0;
    while (n < MAXOPS-1){
        size_t room = MEM_CACHE_BUDGET - used;
        if (room < 12u*1024u) break;
        unsigned sz = 8192u + (rnd() % (52u*1024u));
        if (sz > room - 4096u) sz = (unsigned)(room - 4096u);
        if (sz < 4096u) break;
        sizes[n]=sz; evs[n]=((rnd()%100)<35);
        used += sz + 8u;             /* approx; refined by real alloc below */
        n++;
    }

    void *ptr[MAXOPS];
    for(int i=0;i<n;i++) ptr[i]=NULL;
    /* Placement order determines address layout. DISEASE: original
     * (interleaved) order. FIX (segregation): all pinned first, then
     * all evictable — so the evictables occupy one contiguous span and
     * freeing them yields one big hole. */
    if (!g_fixSegregate){
        for(int i=0;i<n;i++) ptr[i]=memAlloc(MEM_REGION_CACHE,sizes[i],"fuzz");
    } else {
        for(int i=0;i<n;i++) if(!evs[i]) ptr[i]=memAlloc(MEM_REGION_CACHE,sizes[i],"fuzz");
        for(int i=0;i<n;i++) if( evs[i]) ptr[i]=memAlloc(MEM_REGION_CACHE,sizes[i],"fuzz");
    }
    for(int i=0;i<n;i++){ if(ptr[i]&&g_nLive<64) g_live[g_nLive++]=ptr[i];
        log->size[i]=sizes[i]; log->evict[i]=evs[i]; }
    log->n = n;

    /* Relief pass: free every evictable block. */
    for(int i=0;i<n;i++){
        if (evs[i] && ptr[i]){
            memFree(MEM_REGION_CACHE, ptr[i]);
            for(int j=0;j<g_nLive;j++) if(g_live[j]==ptr[i]) g_live[j]=NULL;
        }
    }

    size_t totalFree = MEM_CACHE_BUDGET - memRegionUsed((unsigned)MEM_REGION_CACHE);
    size_t largest   = memCacheLargestFreeBlock();
    return (totalFree >= REQ_BYTES && largest < REQ_BYTES);
}

int main(int argc, char **argv){
    long N = (argc>1)? atol(argv[1]) : 1000000L;
    for (int a=2;a<argc;a++) if (!strcmp(argv[a],"--fixed")) g_fixSegregate=1;
    memInit();
    if (setjmp(g_haltJmp)){ fprintf(stderr,"HALT during fuzz: %s\n", g_haltReason); return 2; }

    long repros=0; unsigned firstSeed=0; RunLog log, firstLog;
    /* histogram of largest-free at reproduction, to see "types" */
    long bucket[8]={0}; /* <16k,<32k,<48k,<56k,<60k,<64k... */

    FILE *corpus = fopen("tests/mem_region_fuzz_corpus.txt","w");
    fprintf(corpus, "# scene-945-class reproductions: req=%u\n", REQ_BYTES);
    fprintf(corpus, "# format: seed=<n> order=<inventory indices in alloc order>\n");

    for(long s=1;s<=N;s++){
        if (runOnce((unsigned)s, &log)){
            repros++;
            /* classify by the largest hole left */
            size_t largest = memCacheLargestFreeBlock();
            int b = largest<16384?0:largest<32768?1:largest<49152?2:
                    largest<57344?3:largest<60000?4:5;
            bucket[b]++;
            if (repros==1){ firstSeed=(unsigned)s; firstLog=log; }
            if (repros<=2000){ /* cap corpus size */
                size_t tot = MEM_CACHE_BUDGET - memRegionUsed((unsigned)MEM_REGION_CACHE);
                fprintf(corpus,"seed=%ld largest=%lu totalfree=%lu nblocks=%d\n",
                        s,(unsigned long)largest,(unsigned long)tot,log.n);
                fprintf(corpus,"  blocks=");
                for(int i=0;i<log.n;i++)
                    fprintf(corpus,"%u%s ", log.size[i], log.evict[i]?"E":"P");
                fprintf(corpus,"\n");
            }
        }
    }
    fclose(corpus);

    printf("runs=%ld repros=%ld (%.4f%%)\n", N, repros, 100.0*repros/N);
    printf("largest-hole histogram: <16k=%ld <32k=%ld <48k=%ld <56k=%ld <60k=%ld <64k=%ld\n",
           bucket[0],bucket[1],bucket[2],bucket[3],bucket[4],bucket[5]);
    if (repros){
        printf("first repro: seed=%u nblocks=%d\n", firstSeed, firstLog.n);
    }
    return repros>0 ? 0 : 1;
}
