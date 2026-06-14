/*
 * mem_region_frag_regression.c — permanent regression test for the
 * scene-945 CACHE-fragmentation BSOD class, driving the REAL allocator
 * (src/mem_region.c).
 *
 * The blob below is a reproducing layout found by tests/mem_region_fuzz.c
 * (seed=2): 19 sub-64K CACHE blocks; freeing the evictable subset (the
 * relief pass) leaves 127,368 bytes free but the largest contiguous hole
 * is only 45,224 — so a 65,536-byte request (building7's clean-rect
 * spill) cannot be satisfied even though ~2x its size is free. This is
 * exactly the scene-945 failure (req=65536, have=88024 fragmented),
 * reproduced deterministically in microseconds.
 *
 * Asserts:
 *   1) DISEASE: after the relief pass, total free >= REQ but largest
 *      contiguous < REQ (the request would BSOD).
 *   2) FIX: a full rebuild (free-all + rewind to pristine) makes the
 *      same request satisfiable again — the mechanism the scheduled
 *      CACHE rebuild relies on.
 *
 * Build: see scripts/build-mem-fuzz.sh (same link set).
 */
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "../src/mem_region/mem_region.h"

int graphicsIsInitialized(void) { return 0; }
void checkMemoryBudget(void) {}
void lruEvictAllUnpinned(void) {}
size_t getTotalMemoryUsed(void) { return 0; }
static jmp_buf g_haltJmp;
void ps1DebugInit(void) {}
void ps1DebugError(const char *fmt, ...) { (void)fmt; longjmp(g_haltJmp, 1); }
void ps1Bsod(const char *s,const char *r,const char *f,int l){(void)s;(void)r;(void)f;(void)l;longjmp(g_haltJmp,1);}
extern size_t memCacheLargestFreeBlock(void);

#define REQ_BYTES 65536u

/* seed=2 reproducing blob: {size, evictable}. E => freed by relief. */
static const struct { unsigned size; int evict; } BLOB[] = {
    {16450,0},{22658,1},{22554,1},{45913,0},{45866,0},{31946,0},{8969,1},
    {26942,1},{41919,0},{16181,0},{29064,0},{42122,1},{38119,0},{54788,0},
    {46624,0},{59331,0},{59857,0},{37693,0},{36932,0},
};
#define BLOB_N ((int)(sizeof(BLOB)/sizeof(BLOB[0])))

int main(void){
    int fail = 0;
    memInit();
    if (setjmp(g_haltJmp)){ printf("FAIL: unexpected allocator halt\n"); return 2; }

    void *ptr[BLOB_N];
    for (int i=0;i<BLOB_N;i++)
        ptr[i] = memAlloc(MEM_REGION_CACHE, BLOB[i].size, "regr");

    /* relief pass: free the evictable subset */
    for (int i=0;i<BLOB_N;i++)
        if (BLOB[i].evict) memFree(MEM_REGION_CACHE, ptr[i]);

    size_t totalFree = MEM_CACHE_BUDGET - memRegionUsed((unsigned)MEM_REGION_CACHE);
    size_t largest   = memCacheLargestFreeBlock();
    printf("post-relief: totalFree=%lu largest=%lu req=%u\n",
           (unsigned long)totalFree,(unsigned long)largest,REQ_BYTES);

    /* (1) DISEASE: enough total free, but no contiguous REQ. */
    if (!(totalFree >= REQ_BYTES && largest < REQ_BYTES)){
        printf("FAIL: blob no longer reproduces the fragmentation state\n");
        fail = 1;
    } else {
        printf("OK: scene-945 fragmentation reproduced (req strands in free-but-fragmented CACHE)\n");
    }

    /* (2) FIX: a rebuild (free everything live + rewind) restores a
     * pristine region where the request fits. */
    for (int i=0;i<BLOB_N;i++)
        if (!BLOB[i].evict) memFree(MEM_REGION_CACHE, ptr[i]);
    memCacheRewindIfEmpty();
    size_t largestAfter = memCacheLargestFreeBlock();
    void *p = memAlloc(MEM_REGION_CACHE, REQ_BYTES, "regr-after");
    printf("post-rebuild: largest=%lu reqAlloc=%s\n",
           (unsigned long)largestAfter, p?"OK":"FAILED");
    if (!p || largestAfter < REQ_BYTES){
        printf("FAIL: rebuild did not restore a satisfiable region\n");
        fail = 1;
    } else {
        printf("OK: rebuild defragments -> request satisfiable (the fix)\n");
    }

    printf(fail?"REGRESSION TEST FAILED\n":"REGRESSION TEST PASSED\n");
    return fail;
}
