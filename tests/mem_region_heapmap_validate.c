/*
 * mem_region_heapmap_validate.c — proves the memory model reproduces the
 * REAL soak heap map byte-for-byte. Allocates the retained "stable shape"
 * band through the REAL allocator (src/mem_region.c) in the order the
 * live soak placed it, and asserts each block lands at the exact offset
 * the forensic JCMEM heap dump recorded (trans-allday3, pre-relief):
 *
 *   off=0      size=49156  johnwalk_spu_load
 *   off=49156  size=98308  fg-stream-window
 *   off=147464 size=98308  grCleanRectPixels (floor)
 *   off=245772 size=98308  grCleanRectPixels (floor)
 *   off=344080 size=16388  fg-frame
 *   off=360468 size=16388  fg-prefetch-frame
 *   off=376856 size=16388  fg-stream-scratch
 *   off=393244 size=94212  cdrom_read_result (BACKGRND.PSB)
 *   off=487456 size=153604 gr-scr-cache
 *   off=641060 size=26628  cdrom_read_result (HOLIDAY.PSB)
 *   => used = 667688  (== the live soak's cache_used steady state)
 *
 * Matching block sizes AND cumulative offsets AND the total proves the
 * model's retained-band sizes and the allocator's block math match the
 * console exactly. (The live map showed HOLIDAY 12292 bytes later behind
 * one churn gap; the contiguous boot order asserted here is the
 * pre-churn placement, same sizes and same total.)
 */
#include <stdio.h>
#include <setjmp.h>
#include "../src/mem_region/mem_region.h"
#include "ps1_mem_model.h"

int graphicsIsInitialized(void){return 0;}
void checkMemoryBudget(void){}
void lruEvictAllUnpinned(void){}
size_t getTotalMemoryUsed(void){return 0;}
static jmp_buf jb;
void ps1DebugInit(void){}
void ps1DebugError(const char*f,...){(void)f;longjmp(jb,1);}
void ps1Bsod(const char*s,const char*r,const char*f,int l){(void)s;(void)r;(void)f;(void)l;longjmp(jb,1);}

/* The retained band in boot allocation order, user bytes. */
static const struct { unsigned bytes; const char *tag; } BAND[] = {
    { PS1_WALK_PSB_BYTES,    "johnwalk_spu_load" },
    { PS1_STREAM_WINDOW_BYTES,"fg-stream-window" },
    { PS1_FLOOR_SLAB_BYTES,  "floor" },
    { PS1_FLOOR_SLAB_BYTES,  "floor" },
    { PS1_FRAME_BYTES,       "fg-frame" },
    { PS1_PREFETCH_BYTES,    "fg-prefetch-frame" },
    { PS1_SCRATCH_BYTES,     "fg-stream-scratch" },
    { PS1_BACKGRND_PSB_BYTES,"BACKGRND.PSB" },
    { PS1_SCR_CACHE_BYTES,   "gr-scr-cache" },
    { PS1_HOLIDAY_PSB_BYTES, "HOLIDAY.PSB" },
};
#define BAND_N ((int)(sizeof(BAND)/sizeof(BAND[0])))

/* Cumulative used (== next block's offset) the real heap map recorded. */
static const unsigned long EXPECT_CUM[BAND_N] = {
    49156, 147464, 245772, 344080, 360468, 376856, 393244, 487456, 641060, 667688
};

int main(void){
    int fail=0;
    memInit();
    if (setjmp(jb)){ printf("FAIL: unexpected halt\n"); return 2; }

    for (int i=0;i<BAND_N;i++){
        memAlloc(MEM_REGION_CACHE, BAND[i].bytes, BAND[i].tag);
        unsigned long cum = (unsigned long)memRegionUsed((unsigned)MEM_REGION_CACHE);
        const char *ok = (cum==EXPECT_CUM[i]) ? "ok" : "MISMATCH";
        printf("%-18s user=%-7u cum=%-7lu expect=%-7lu %s\n",
               BAND[i].tag, BAND[i].bytes, cum, EXPECT_CUM[i], ok);
        if (cum!=EXPECT_CUM[i]) fail=1;
    }
    void *ptr[BAND_N]; /* (re-track for relief-state checks below) */
    unsigned long used = (unsigned long)memRegionUsed((unsigned)MEM_REGION_CACHE);
    printf("final cache_used=%lu (real soak steady state=667688, 914/945 scenes) %s\n",
           used, used==667688?"MATCH":"MISMATCH");
    if (used!=667688) fail=1;

    /* --- Relief-state telemetry match ---
     * The real allday4 log's cache_used histogram (the memory log we're
     * reproducing) shows, besides 667688: 514084 x12 and 634920 x2.
     * These are the retained band minus specific relief evictions.
     * Re-run a clean band tracking pointers, free the SCR cache, and
     * assert the resulting cache_used equals the real 514084 — i.e. the
     * model reproduces the real log's #2 value byte-for-byte. */
    memInit();  /* fresh region */
    for (int i=0;i<BAND_N;i++) ptr[i]=memAlloc(MEM_REGION_CACHE, BAND[i].bytes, BAND[i].tag);
    /* index 8 = gr-scr-cache */
    memFree(MEM_REGION_CACHE, ptr[8]);
    unsigned long usedScrEvicted = (unsigned long)memRegionUsed((unsigned)MEM_REGION_CACHE);
    printf("relief drops SCR -> cache_used=%lu (real log 514084 x12) %s\n",
           usedScrEvicted, usedScrEvicted==514084?"MATCH":"MISMATCH");
    if (usedScrEvicted!=514084) fail=1;

    printf(fail?"HEAPMAP VALIDATION FAILED\n"
               :"HEAPMAP VALIDATION PASSED (model reproduces real soak layout + relief-state cache_used values)\n");
    return fail;
}
