/*
 * mem_map_replay.c — reconstruct a captured JCMEM heap-map byte-for-byte
 * through the REAL allocator (src/mem_region.c), then verify cache_used,
 * the largest contiguous free hole, and whether a probe request strands.
 *
 * Input: a heap-map dump as emitted by memDumpCacheMap() in a forensic
 * (PS1_MEM_FORENSICS=ON) build — the lines
 *     JCMEM map-begin why=... used=U bump=B
 *     JCMEM map off=O size=S live tag=T
 *     JCMEM map off=O size=S FREE tag=-
 *     JCMEM map-end tail=...
 * read from a file (argv[1]) or stdin. DuckStation timestamp/color
 * prefixes are tolerated (we scan for "JCMEM map").
 *
 * Method: allocate every block (live AND free) in offset order — the
 * real allocator bump-places them at the recorded offsets — then free
 * the FREE-tagged ones. That recreates the exact free-list the console
 * had at that instant. We then assert cache_used matches the recorded
 * `used`, and report the largest hole + a 65536 strand verdict.
 *
 * This turns a forensic soak's per-scene map into a deterministic,
 * offline, byte-exact regression: the scene-945 going-in / pre-relief /
 * bsod maps replay here in microseconds, no emulator.
 *
 * Build:
 *   gcc -O2 -o tests/mem_map_replay tests/mem_map_replay.c \
 *       src/mem_region.c -Isrc -Isrc/mem_region -Isrc/platform/ps1
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "../src/mem_region/mem_region.h"

int graphicsIsInitialized(void){return 0;}
void checkMemoryBudget(void){}
void lruEvictAllUnpinned(void){}
size_t getTotalMemoryUsed(void){return 0;}
static jmp_buf jb;
void ps1DebugInit(void){}
void ps1DebugError(const char*f,...){(void)f;longjmp(jb,1);}
void ps1Bsod(const char*s,const char*r,const char*f,int l){(void)s;(void)r;(void)f;(void)l;longjmp(jb,1);}
extern size_t memCacheLargestFreeBlock(void);

/* The allocator adds a 4-byte header per block: a map `size` of S
 * corresponds to a user request of S-4 (verified against the heap map:
 * walk 49156==49152+4, floor 98308==98304+4, etc.). */
#define HDR 4u

struct Blk { unsigned long off, size; int isFree; char tag[48]; void *ptr; };
static struct Blk g_blk[256];
static int g_n;
static unsigned long g_recordedUsed = 0;

static int parseMap(FILE *f)
{
    char line[512];
    int started = 0;
    while (fgets(line, sizeof line, f)) {
        char *p = strstr(line, "JCMEM map");
        if (!p) continue;
        if (strstr(p, "map-begin")) {
            char *u = strstr(p, "used=");
            if (u) g_recordedUsed = strtoul(u + 5, NULL, 10);
            started = 1; g_n = 0;
            continue;
        }
        if (strstr(p, "map-end")) break;
        if (!started) continue;
        /* "JCMEM map off=O size=S live|FREE tag=T" */
        char *o = strstr(p, "off=");
        char *s = strstr(p, "size=");
        if (!o || !s) continue;
        struct Blk *b = &g_blk[g_n];
        b->off  = strtoul(o + 4, NULL, 10);
        b->size = strtoul(s + 5, NULL, 10);
        b->isFree = (strstr(p, " FREE") != NULL) || (strstr(p, "FREE tag") != NULL);
        char *t = strstr(p, "tag=");
        if (t) { sscanf(t + 4, "%47s", b->tag); } else b->tag[0] = 0;
        b->ptr = NULL;
        if (++g_n >= 256) break;
    }
    return g_n;
}

int main(int argc, char **argv)
{
    FILE *f = stdin;
    if (argc > 1) { f = fopen(argv[1], "r"); if (!f) { perror(argv[1]); return 2; } }

    int n = parseMap(f);
    if (n == 0) { fprintf(stderr, "no map parsed\n"); return 2; }

    memInit();
    if (setjmp(jb)) { printf("FAIL: allocator halted during replay\n"); return 2; }

    /* allocate every block in offset order so the allocator bump-places
     * them at the recorded offsets; track ptrs to free the holes after. */
    int fail = 0;
    for (int i = 0; i < n; i++) {
        unsigned long req = g_blk[i].size > HDR ? g_blk[i].size - HDR : g_blk[i].size;
        void *p = memAlloc(MEM_REGION_CACHE, req, g_blk[i].tag[0] ? g_blk[i].tag : "replay");
        g_blk[i].ptr = p;
        unsigned long cum = (unsigned long)memRegionUsed((unsigned)MEM_REGION_CACHE);
        unsigned long expectEnd = g_blk[i].off + g_blk[i].size;
        if (cum != expectEnd) {
            printf("  off=%-7lu size=%-7lu %-5s -> cum=%-7lu expectEnd=%-7lu MISMATCH\n",
                   g_blk[i].off, g_blk[i].size, g_blk[i].isFree ? "FREE" : "live",
                   cum, expectEnd);
            fail = 1;
        }
    }
    /* free the holes to recreate the exact free-list */
    for (int i = 0; i < n; i++)
        if (g_blk[i].isFree && g_blk[i].ptr)
            memFree(MEM_REGION_CACHE, g_blk[i].ptr);

    unsigned long used    = (unsigned long)memRegionUsed((unsigned)MEM_REGION_CACHE);
    unsigned long largest = (unsigned long)memCacheLargestFreeBlock();
    unsigned long total   = (unsigned long)(MEM_CACHE_BUDGET - used);

    printf("blocks=%d  reconstructed cache_used=%lu (recorded=%lu) %s\n",
           n, used, g_recordedUsed,
           used == g_recordedUsed ? "MATCH" : "MISMATCH");
    printf("totalFree(have)=%lu  largestContig=%lu\n", total, largest);
    printf("65536 request: %s\n",
           largest < 65536 ? "STRANDS (largest < 65536) — the BSOD" : "fits");
    if (used != g_recordedUsed) fail = 1;

    printf(fail ? "REPLAY MISMATCH\n" : "REPLAY OK (layout reconstructed byte-for-byte)\n");
    return fail;
}
