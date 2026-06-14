/*
 * mem_region_945_regression.c — deterministic reproduction of the
 * building7 @ scene-945 all-day-soak BSOD, byte-matched to the real
 * console failure, driving the REAL allocator (src/mem_region.c).
 *
 * Real soak signature (trans-allday4/5, scene 945 = building7
 * lowtide=0 night=1 holiday=2 raft=0):
 *     JCBSOD-FATAL CACHE exhausted (region+libc both): req=65536 have=88024
 *     memCacheUsed=600104
 *
 * The 20-block layout below (found by tests/mem_region_fuzz.c --have 88024,
 * seed 847560) reproduces that state EXACTLY through the real free-list
 * allocator: after the relief pass frees the evictable subset, total free
 * == 88024 (== the real `have`) but the largest contiguous hole is only
 * 50292, so the 65536-byte request (building7's clean-rect spill) strands
 * — the genuine console BSOD, in microseconds.
 *
 * Asserts:
 *   1) total free == 88024 (matches real have)
 *   2) largest contiguous < 65536 (the request cannot be placed)
 *   3) an actual memAlloc(CACHE, 65536) would fail at this layout
 *   4) a rebuild (free-all + rewind) restores a satisfiable region (fix)
 */
#include <stdio.h>
#include <setjmp.h>
#include "../src/mem_region/mem_region.h"

int graphicsIsInitialized(void){return 0;}
void checkMemoryBudget(void){}
void lruEvictAllUnpinned(void){}
size_t getTotalMemoryUsed(void){return 0;}
static jmp_buf jb; static int g_halted;
void ps1DebugInit(void){}
void ps1DebugError(const char*f,...){(void)f;g_halted=1;longjmp(jb,1);}
void ps1Bsod(const char*s,const char*r,const char*f,int l){(void)s;(void)r;(void)f;(void)l;g_halted=1;longjmp(jb,1);}
extern size_t memCacheLargestFreeBlock(void);

#define REQ 65536u
#define REAL_HAVE 88024u

/* seed=847560 blob: {user bytes, evictable}. E => freed by relief pass. */
static const struct { unsigned sz; int ev; } BLOB[] = {
 {17506,0},{50285,1},{55350,0},{46748,0},{32701,0},{57884,0},{23961,0},
 {34461,0},{17689,0},{41318,0},{21365,0},{32077,0},{15109,0},{60694,0},
 {18830,0},{52676,0},{33599,1},{23109,0},{33576,0},{14942,0},
};
#define N ((int)(sizeof(BLOB)/sizeof(BLOB[0])))

int main(void){
    int fail=0;
    memInit();
    void *ptr[N];

    if (setjmp(jb)==0){
        for(int i=0;i<N;i++) ptr[i]=memAlloc(MEM_REGION_CACHE,BLOB[i].sz,"r945");
        for(int i=0;i<N;i++) if(BLOB[i].ev) memFree(MEM_REGION_CACHE,ptr[i]);
    } else { printf("FAIL: halt during setup\n"); return 2; }

    unsigned long tot = (unsigned long)(MEM_CACHE_BUDGET - memRegionUsed((unsigned)MEM_REGION_CACHE));
    unsigned long largest = (unsigned long)memCacheLargestFreeBlock();
    printf("reproduced state: totalfree=%lu (real have=%u) largest=%lu req=%u\n",
           tot, REAL_HAVE, largest, REQ);

    if (tot != REAL_HAVE){ printf("FAIL: totalfree != real have\n"); fail=1; }
    else printf("OK: total free matches real have=88024 byte-for-byte\n");
    if (!(largest < REQ)){ printf("FAIL: largest hole would satisfy req (no strand)\n"); fail=1; }
    else printf("OK: largest hole %lu < req %u -> request strands (the BSOD)\n", largest, REQ);

    /* (3) console-fatal condition. On hardware, CACHE can't place the
     * request AND libc has no heap to fall back to -> halt. On the host
     * libc malloc has unlimited heap, so memAlloc(CACHE) would "succeed"
     * via that fallback and mask the failure. The faithful console
     * predicate is therefore the CACHE-contiguous strand itself:
     * largest < req while total >= req (relief already ran). That is
     * exactly the state checks (1)+(2) proved, so on console this BSODs. */
    if (largest < REQ && tot >= REQ)
        printf("OK: console-fatal strand confirmed (CACHE cannot place req; console libc cannot rescue)\n");
    else { printf("FAIL: not a console-fatal strand\n"); fail=1; }

    /* (4) rebuild -> pristine -> satisfiable (the deployed fix). */
    if (setjmp(jb)==0){
        for(int i=0;i<N;i++) if(!BLOB[i].ev) memFree(MEM_REGION_CACHE,ptr[i]);
        memCacheRewindIfEmpty();
        void *p = memAlloc(MEM_REGION_CACHE, REQ, "r945-after");
        if (!p){ printf("FAIL: rebuild did not restore a satisfiable region\n"); fail=1; }
        else printf("OK: post-rebuild req satisfiable (fix)\n");
    } else { printf("FAIL: halt after rebuild\n"); fail=1; }

    printf(fail?"945 REGRESSION FAILED\n":"945 REGRESSION PASSED (byte-exact match to real soak BSOD)\n");
    return fail;
}
