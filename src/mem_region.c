/*
 * mem_region.c — memory region allocator implementation.
 *
 * See mem_region.h for the public API and
 * docs/ps1/memory-region-allocator-plan.md for the full design.
 *
 * Layout of the static region buffer:
 *
 *     0                                                         end
 *     +----------+--------------------------+-------------------+
 *     |  BOOT    |          CACHE           |     TRANSIENT     |
 *     | bump up  |  segregated free-list    |     bump down     |
 *     +----------+--------------------------+-------------------+
 *
 * - BOOT bumps up from byte 0; freezes after boot.
 * - CACHE sits in the middle; its own free-list allocator. Bounded:
 *   internal fragmentation cannot starve BOOT or TRANSIENT.
 * - TRANSIENT bumps DOWN from the top of its slot. memSceneReset
 *   rewinds the bump pointer.
 *
 * PSX-libc printf is non-allocating (uses an internal fixed buffer);
 * the project-level malloc poison would trip at link time if any
 * libc function tried to call malloc, so this is link-time-checked
 * not runtime-asserted. See plan v9 "What changed in v8 — A21".
 */

#include "mem_region.h"
#include "ps1_debug.h"          /* JC_BSOD, ps1DebugError */
#include <string.h>             /* memset, memcpy */

#ifndef PS1_BUILD
#include <stdio.h>              /* PC host build: printf for telemetry */
#endif

/* External flag — true once graphicsInit has completed. Defined in
 * graphics_ps1.c (PS1 build). On non-PS1 builds (unit tests), this
 * is a stub returning 1. */
extern int graphicsIsInitialized(void);

/* ---------------------------------------------------------------------
 * Static region buffer
 * ------------------------------------------------------------------- */

/* Region buffer is allocated dynamically in memInit() rather than
 * being a fixed BSS reservation. This was a critical correction: a
 * 1.2 MB static BSS buffer left libc with only ~30 KB heap, which
 * couldn't fit the 1.1 MB RESOURCE.001 catalog load that runs
 * during parseResourceFiles. Allocating dynamically means libc has
 * full heap until memInit fires (called AFTER parseResourceFiles in
 * main()), at which point the catalog file is closed and its buffer
 * has been returned to libc. */
static unsigned char *g_memRegionBuf = NULL;

/* Region byte ranges (set at memInit). */
static unsigned char *g_bootBase;       /* start of BOOT (byte 0) */
static unsigned char *g_cacheBase;      /* start of CACHE */
static unsigned char *g_transientBase;  /* start of TRANSIENT */
static unsigned char *g_transientEnd;   /* one past end of buffer */

/* Bump pointers. */
static unsigned char *g_bootTop;        /* next BOOT alloc here */
static unsigned char *g_transientNext;  /* next TRANSIENT alloc here (grows DOWN) */

/* High-water marks. */
static size_t g_bootPeak;
static size_t g_cacheUsed;
static size_t g_cachePeak;
static size_t g_transientPeak;

/* State flags. */
static int g_memInited      = 0;
static int g_bootFrozen     = 0;

/* TRANSIENT outstanding-allocation balance. */
static int g_sceneAllocBalance = 0;

/* Linked list of TRANSIENT allocations that overflowed to libc.
 * memSceneReset walks this list and frees them all wholesale,
 * preserving the per-scene-wipe semantics even for allocations
 * too big for the static TRANSIENT region. */
typedef struct TransientLibcEntry {
    void *ptr;
    struct TransientLibcEntry *next;
} TransientLibcEntry;
static TransientLibcEntry *g_transientLibcHead = NULL;

/* Last-resort CACHE relief hook; see mem_region.h. */
static MemCacheReliefFn g_cacheReliefHook = NULL;

void memSetCacheReliefHook(MemCacheReliefFn fn)
{
    g_cacheReliefHook = fn;
}

/* ---------------------------------------------------------------------
 * Forward declarations (internal)
 * ------------------------------------------------------------------- */

static void  *cacheAllocInternal(size_t size, const char *tag);
static void   cacheFreeInternal(void *ptr);
static size_t cacheUsedInternal(void);
static void   cacheInit_(void);
static void   memHaltFmt(const char *region, const char *what,
                         size_t required, size_t available);

/* ---------------------------------------------------------------------
 * Alignment helpers
 * ------------------------------------------------------------------- */

static size_t alignUp(size_t n)
{
    return (n + (MEM_REGION_ALIGN - 1)) & ~((size_t)MEM_REGION_ALIGN - 1);
}

/* ---------------------------------------------------------------------
 * ps1IsMainContext — runtime ISR-context predicate
 * ------------------------------------------------------------------- */

int ps1IsMainContext(void)
{
#ifdef PS1_BUILD
    /* Read CAUSE (cop0 r13) and SR (cop0 r12). We're in main context
     * if no exception is currently being handled (CAUSE.ExcCode == 0)
     * AND interrupts are enabled (SR.IEc == 1).
     *
     * Bit patterns per PSX programmer's manual:
     *   CAUSE.ExcCode = bits 2..6 (mask 0x7C). 0 means no exception.
     *   SR.IEc        = bit 0 (mask 0x01). 1 means interrupts enabled.
     *
     * Note: this is a fast probabilistic check. A future PR could
     * tighten it by also inspecting EPC/EXL on R3000A's exception
     * pipeline. See plan v9 P20 + Phase 1 unit test. */
    unsigned int cause, sr;
    __asm__ volatile ("mfc0 %0, $13" : "=r"(cause));
    __asm__ volatile ("mfc0 %0, $12" : "=r"(sr));
    return ((cause & 0x7Cu) == 0u) && ((sr & 0x01u) != 0u);
#else
    /* PC host build (unit tests): always main context. */
    return 1;
#endif
}

/* ---------------------------------------------------------------------
 * memHalt — unified halt primitive
 * ------------------------------------------------------------------- */

__attribute__((noreturn))
void memHalt(const char *scene, const char *reason)
{
    /* If graphics is up, route to the full BSOD panel. Otherwise
     * use ps1DebugError which renders a minimal text panel — that
     * depends on ps1DebugInit having run, which main() does as its
     * first thing (before memInit). */
    if (graphicsIsInitialized()) {
        ps1Bsod(scene ? scene : "(unknown)",
                reason ? reason : "(unspecified)",
                __FILE__, __LINE__);
        /* ps1Bsod is noreturn — compiler infers unreachable. */
    } else {
        (void)scene;
        ps1DebugError(reason ? reason : "(unspecified)");
        for (;;) { /* halt forever — GCC infers non-returning */ }
    }
}

/* Format a halt reason string into a single static buffer.
 * Re-entrancy guard: a depth counter prevents recursive halt-during-
 * halt from racing the buffer. Returns the same buffer or
 * "[concurrent fatal]" on re-entry. */
static char        g_haltReason[256];
static volatile int g_haltDepth = 0;

static char *memAppendText(char *dst, char *end, const char *src)
{
    if (src == NULL)
        src = "?";
    while (dst < end && *src != '\0') {
        *dst++ = *src++;
    }
    *dst = '\0';
    return dst;
}

static char *memAppendUnsigned(char *dst, char *end, size_t value)
{
    char digits[24];
    int count = 0;

    do {
        digits[count++] = (char)('0' + (value % 10u));
        value /= 10u;
    } while (value != 0 && count < (int)sizeof(digits));

    while (count > 0 && dst < end)
        *dst++ = digits[--count];
    *dst = '\0';
    return dst;
}

static const char *formatHaltReason(const char *region, const char *what,
                                    size_t required, size_t available)
{
    char *p;
    char *end;

    if (g_haltDepth++ > 0) {
        return "[concurrent fatal]";
    }
    p = g_haltReason;
    end = g_haltReason + sizeof(g_haltReason) - 1;
    *p = '\0';
    p = memAppendText(p, end, region);
    p = memAppendText(p, end, " ");
    p = memAppendText(p, end, what);
    p = memAppendText(p, end, ": req=");
    p = memAppendUnsigned(p, end, required);
    p = memAppendText(p, end, " have=");
    p = memAppendUnsigned(p, end, available);
    return g_haltReason;
}

static void memHaltFmt(const char *region, const char *what,
                       size_t required, size_t available)
{
    const char *r = formatHaltReason(region, what, required, available);
    memHalt("(allocator)", r);
}

/* ---------------------------------------------------------------------
 * Boot lifecycle
 * ------------------------------------------------------------------- */

void memInit(void)
{
    extern int printf(const char *, ...);
    extern void *malloc(size_t);
    /* Allocate the region buffer from libc. After parseResourceFiles
     * streamed RESOURCE.001 (was 1.1 MB libc temp), libc has the
     * headroom to give us ~1 MB for the region. */
    g_memRegionBuf = (unsigned char *)malloc(MEM_REGION_TOTAL);
    if (g_memRegionBuf == NULL) {
        printf("JCMEM memInit: libc malloc FAILED for %lu bytes\n",
               (unsigned long)MEM_REGION_TOTAL);
        memHalt("(boot)", "memInit: libc malloc for region buffer failed");
    }
    g_bootBase      = &g_memRegionBuf[0];
    g_cacheBase     = g_bootBase  + MEM_BOOT_BUDGET;
    g_transientBase = g_cacheBase + MEM_CACHE_BUDGET;
    g_transientEnd  = g_transientBase + MEM_TRANSIENT_BUDGET;
    g_bootTop       = g_bootBase;
    g_transientNext = g_transientEnd;
    printf("JCMEM memInit: region buffer %lu KB at %p\n",
           (unsigned long)(MEM_REGION_TOTAL / 1024u),
           (void*)g_memRegionBuf);

    g_bootPeak      = 0;
    g_cacheUsed     = 0;
    g_cachePeak     = 0;
    g_transientPeak = 0;

    g_sceneAllocBalance = 0;
    g_bootFrozen        = 0;
    g_memInited         = 1;

    cacheInit_();
}

void memFreezeBoot(void)
{
    MEM_REQUIRE(g_memInited);
    g_bootFrozen = 1;
}

int memIsReady(void)
{
    return g_memInited;
}

/* ---------------------------------------------------------------------
 * memAlloc / memFree
 * ------------------------------------------------------------------- */

void *memAlloc(MemRegion region, size_t size, const char *tag)
{
    MEM_REQUIRE(g_memInited);
    MEM_REQUIRE(ps1IsMainContext());

    if (size == 0) {
        /* Define zero-size as a no-op returning a non-NULL sentinel.
         * Caller never reads from it; matches typical malloc behavior. */
        return g_bootBase;  /* harmless non-NULL */
    }

    const size_t alignedSize = alignUp(size);

    switch (region) {

    case MEM_REGION_BOOT: {
        if (g_bootFrozen) {
            memHaltFmt("BOOT", "alloc-after-freeze",
                       alignedSize, 0);
        }
        const size_t used = (size_t)(g_bootTop - g_bootBase);
        const size_t remaining = MEM_BOOT_BUDGET - used;
        if (alignedSize > remaining) {
            memHaltFmt("BOOT", "exhausted", alignedSize, remaining);
        }
        void *p = g_bootTop;
        g_bootTop += alignedSize;
        const size_t newUsed = used + alignedSize;
        if (newUsed > g_bootPeak) {
            g_bootPeak = newUsed;
        }
        return p;
    }

    case MEM_REGION_TRANSIENT: {
        const size_t used = (size_t)(g_transientEnd - g_transientNext);
        const size_t remaining = MEM_TRANSIENT_BUDGET - used;
        if (alignedSize > remaining) {
            /* TRANSIENT region exhausted — fall back to libc with
             * linked-list tracking so wholesale-wipe still works. */
            extern void *malloc(size_t);
            void *p = malloc(alignedSize);
            if (p == NULL) {
                memHaltFmt("TRANSIENT", "region+libc both exhausted",
                           alignedSize, remaining);
            }
            TransientLibcEntry *entry =
                (TransientLibcEntry *)malloc(sizeof(TransientLibcEntry));
            if (entry == NULL) {
                memHaltFmt("TRANSIENT", "libc-fallback entry failed",
                           sizeof(TransientLibcEntry), 0);
            }
            entry->ptr  = p;
            entry->next = g_transientLibcHead;
            g_transientLibcHead = entry;
            g_sceneAllocBalance++;
            return p;
        }
        g_transientNext -= alignedSize;
        const size_t newUsed = used + alignedSize;
        if (newUsed > g_transientPeak) {
            g_transientPeak = newUsed;
        }
        g_sceneAllocBalance++;
        return g_transientNext;
    }

    case MEM_REGION_CACHE: {
        /* R33o diagnostic: log large CACHE allocations so we can correlate
         * with scene events to find the 475 KB unaccounted residency. */
#if PS1_VERBOSE_DIAGNOSTICS
        /* Allocation tracing for memory-shape debugging. Soak verdicts
         * key on cache-relief/JCBSOD lines, which stay unconditional. */
        if (alignedSize >= (32u * 1024u)) {
            extern int printf(const char *, ...);
            printf("JCMEM cache-alloc-big size=%lu tag=%s\n",
                   (unsigned long)alignedSize, tag ? tag : "(?)");
        }
#endif
        void *p = cacheAllocInternal(alignedSize, tag);
        if (p == NULL) {
            extern void checkMemoryBudget(void);
            checkMemoryBudget();
            p = cacheAllocInternal(alignedSize, tag);
            if (p == NULL) {
                /* R33-soak panic mode: drop ALL unpinned LRU
                 * resources, not just down-to-budget. The
                 * fragmentation-driven BSOD at 226s came from a
                 * scenario where total-free CACHE bytes exceeded
                 * the request (113 KB free, 96 KB request) but no
                 * contiguous block was big enough. Forcing a full
                 * LRU drop here frees more individually-sized
                 * blocks; cacheCoalesce_ inside cacheFreeInternal
                 * merges adjacent ones. Retry CACHE alloc — if a
                 * contiguous span now exists, we recover. */
                extern void lruEvictAllUnpinned(void);
                lruEvictAllUnpinned();
                p = cacheAllocInternal(alignedSize, tag);
                if (p == NULL && g_cacheReliefHook != NULL &&
                    g_cacheReliefHook((unsigned long)alignedSize)) {
                    /* Optimization-only retention released (parked
                     * clean-rect slabs, idle grow-only buffers) —
                     * one more try before libc. */
                    extern int printf(const char *, ...);
                    printf("JCMEM cache-relief fired req=%lu\n",
                           (unsigned long)alignedSize);
                    p = cacheAllocInternal(alignedSize, tag);
                }
                if (p == NULL) {
                    /* CACHE region + panic eviction both insufficient
                     * — fall back to libc. memFree(CACHE) detects
                     * libc pointers by range check and frees them
                     * appropriately. */
                    extern void *malloc(size_t);
                    p = malloc(alignedSize);
                    if (p == NULL) {
                        memHaltFmt("CACHE", "exhausted (region+libc both)",
                                   alignedSize, MEM_CACHE_BUDGET - g_cacheUsed);
                    }
                }
            }
        }
        if (g_cacheUsed > g_cachePeak) {
            g_cachePeak = g_cacheUsed;
        }
        return p;
    }

    default:
        memHalt("(allocator)", "memAlloc: bad region");
    }
}

void memFree(MemRegion region, void *ptr)
{
    if (ptr == NULL) return;

    MEM_REQUIRE(g_memInited);
    MEM_REQUIRE(ps1IsMainContext());

    switch (region) {

    case MEM_REGION_BOOT:
        /* BOOT allocations are permanent. Freeing them is a bug
         * once boot is frozen. Before freeze, it's a no-op (e.g.,
         * legitimate startup paths that test then commit a
         * temporary). */
        if (g_bootFrozen) {
            memHalt("(allocator)",
                    "memFree(BOOT) after freeze — BOOT is permanent");
        }
        return;

    case MEM_REGION_TRANSIENT:
        /* Bytes are not reclaimed individually for region allocations;
         * memSceneReset wipes the whole region (and frees the libc-
         * fallback list). We just decrement the outstanding count. */
        if (g_sceneAllocBalance > 0) {
            g_sceneAllocBalance--;
        }
        return;

    case MEM_REGION_CACHE: {
        /* If the pointer is inside the CACHE region, use the
         * free-list. Otherwise it was libc-allocated (fallback) — use
         * libc free. */
        if (g_cacheBase != NULL &&
            (unsigned char *)ptr >= g_cacheBase &&
            (unsigned char *)ptr <  g_cacheBase + MEM_CACHE_BUDGET) {
            cacheFreeInternal(ptr);
        } else {
            extern void free(void *);
            free(ptr);
        }
        return;
    }

    default:
        memHalt("(allocator)", "memFree: bad region");
    }
}

void memSceneReset(const char *sceneName)
{
    MEM_REQUIRE(g_memInited);
    MEM_REQUIRE(ps1IsMainContext());

    /* Debug builds: assert the outstanding-allocation count returned
     * to zero. If a TRANSIENT alloc was made but never matched with a
     * memFree, the call site has a leak that needs fixing. */
#ifdef MEM_DEBUG_BALANCE
    if (g_sceneAllocBalance != 0) {
        memHalt(sceneName ? sceneName : "(unknown)",
                "memSceneReset: sceneAllocBalance != 0 — TRANSIENT leak");
    }
#endif

    /* Pin-count delta logging (plan v9 step 26 / A25). At every
     * scene transition, surface mismatched pin/unpin pairs that
     * would otherwise quietly accumulate. Compiled out in release. */
#ifdef MEM_DEBUG_PIN_DELTA
    {
        extern size_t getTotalMemoryUsed(void);
        static size_t s_prevPinned = 0;
        size_t pin = getTotalMemoryUsed();
        if (pin != s_prevPinned) {
            extern int printf(const char *, ...);
            printf("JCMEM pin-delta scene=%s prev=%lu now=%lu delta=%ld\n",
                   sceneName ? sceneName : "(unknown)",
                   (unsigned long)s_prevPinned,
                   (unsigned long)pin,
                   (long)pin - (long)s_prevPinned);
            s_prevPinned = pin;
        }
    }
#endif

    /* Telemetry on every reset — emits one JCMEM line summarizing
     * the regions, gated behind FG_HEAP_PROBE_LOGS so log volume
     * stays opt-in. */
#ifdef FG_HEAP_PROBE_LOGS
    memLogTelemetry();
#endif

    /* Free any TRANSIENT libc-fallback entries — these are allocations
     * that overflowed the static region and went to libc. Walking the
     * list at reset preserves the per-scene-wipe semantic even for
     * oversized scenes. */
    {
        extern void free(void *);
        TransientLibcEntry *e = g_transientLibcHead;
        while (e != NULL) {
            TransientLibcEntry *next = e->next;
            free(e->ptr);
            free(e);
            e = next;
        }
        g_transientLibcHead = NULL;
    }

    /* Wipe — bump the pointer back to the top of TRANSIENT (which
     * grows DOWN, so top == g_transientEnd). */
    g_transientNext     = g_transientEnd;
    g_sceneAllocBalance = 0;

#ifdef MEM_POISON_TRANSIENT
    /* Debug-only: fill with 0xCD so any code that reads
     * uninitialized TRANSIENT bytes gets a recognizable pattern. */
    memset(g_transientBase, 0xCD, MEM_TRANSIENT_BUDGET);
#endif

    (void)sceneName;
}

/* ---------------------------------------------------------------------
 * CACHE sub-allocator — bump + first-fit free-list
 * ---------------------------------------------------------------------
 *
 * Hybrid design:
 *   - Allocation: try the free-list first (first-fit); fall back to
 *     bumping the high-water mark forward.
 *   - Free: prepend the block to the free-list. No coalescing in
 *     this implementation — fragmentation is bounded by the
 *     bump's high-water plus the free-list's outstanding blocks.
 *   - The first 4 bytes of each block store its rounded size so
 *     memFree knows how much to reclaim.
 *
 * Tradeoff vs segregated free-lists: simpler, slightly slower for
 * mixed-size workloads, no internal class waste. For the CACHE
 * pattern (a few hundred BMP/TTM/SCR blobs of varying sizes,
 * allocate-load-pin-evict), first-fit is adequate.
 *
 * Header layout: each block (allocated or free) is preceded by a
 * 4-byte size field. The user pointer is +4 bytes from the block
 * base. Sizes include the header.
 * ------------------------------------------------------------------- */

#define CACHE_HEADER_BYTES 4

typedef struct CacheFreeBlock {
    struct CacheFreeBlock *next;  /* next free block, or NULL */
    /* Block body comes after; size lives in the 4 bytes preceding
     * this struct (i.e., (uint32 *)((char *)block - CACHE_HEADER_BYTES)). */
} CacheFreeBlock;

static unsigned char *g_cacheBumpTop;
static CacheFreeBlock *g_cacheFreeList;  /* sorted by ascending address */
static const void *g_cacheDiagFreePtr;
static const void *g_cacheDiagUserPtr;
static const void *g_cacheDiagNode;
static const void *g_cacheDiagPrev;
static const void *g_cacheDiagBlockBase;
static size_t g_cacheDiagBlockSize;
static const char *g_cacheDiagFreeTag;
static const char *g_cacheDiagNodeTag;
static size_t g_cacheDiagFreeBlockSize;
static const char *g_cacheDiagOffset4Tag;
static size_t g_cacheDiagOffset4Size;

/* Minimum block size we'll leave behind when splitting a free block.
 * Must fit a 4-byte header plus a CacheFreeBlock (next pointer). */
#define CACHE_MIN_BLOCK_BYTES \
    (((size_t)(CACHE_HEADER_BYTES + sizeof(void *) + MEM_REGION_ALIGN - 1)) & \
     ~((size_t)MEM_REGION_ALIGN - 1))
#define CACHE_MIN_SPLIT_BLOCK ((size_t)(CACHE_MIN_BLOCK_BYTES + MEM_REGION_ALIGN))
#define CACHE_MAX_FREE_WALK ((size_t)((MEM_CACHE_BUDGET / CACHE_MIN_BLOCK_BYTES) + 8u))

static void cacheInit_(void)
{
    g_cacheBumpTop  = g_cacheBase;
    g_cacheFreeList = NULL;
}

static const char *cacheDiagTagForPtr_(const void *ptr)
{
    if (g_cacheBase != NULL &&
        ptr == (const void *)(g_cacheBase + CACHE_HEADER_BYTES) &&
        g_cacheDiagOffset4Tag != NULL)
        return g_cacheDiagOffset4Tag;
    return "?";
}

static void cacheDiagRememberAlloc_(const void *ptr, size_t size,
                                    const char *tag)
{
    if (g_cacheBase != NULL &&
        ptr == (const void *)(g_cacheBase + CACHE_HEADER_BYTES)) {
        g_cacheDiagOffset4Tag = tag ? tag : "?";
        g_cacheDiagOffset4Size = size;
    }
}

static void cacheDiagRememberFree_(const void *ptr)
{
    (void)ptr;
}

static unsigned int cacheReadSize_(unsigned char *blockBase)
{
    /* Block base is the header start; size is the 4-byte word at
     * blockBase. User pointer is at blockBase + 4. */
    unsigned int sz;
    memcpy(&sz, blockBase, sizeof(unsigned int));
    return sz;
}

static void cacheWriteSize_(unsigned char *blockBase, unsigned int sz)
{
    memcpy(blockBase, &sz, sizeof(unsigned int));
}

static int cachePtrAligned_(const void *ptr)
{
    return (((unsigned long)ptr & (unsigned long)(MEM_REGION_ALIGN - 1u)) == 0u);
}

static char *memAppendCacheOffset(char *dst, char *end, const void *ptr)
{
    const unsigned char *p = (const unsigned char *)ptr;

    if (ptr == NULL)
        return memAppendText(dst, end, "nil");
    if (g_cacheBase != NULL &&
        p >= g_cacheBase &&
        p <= g_cacheBase + MEM_CACHE_BUDGET)
        return memAppendUnsigned(dst, end, (size_t)(p - g_cacheBase));
    return memAppendText(dst, end, "out");
}

static const char *formatCacheCorruptReason(const char *what)
{
    char *p;
    char *end;

    if (g_haltDepth++ > 0)
        return "[concurrent fatal]";
    p = g_haltReason;
    end = g_haltReason + sizeof(g_haltReason) - 1;
    *p = '\0';
    p = memAppendText(p, end, what);
    p = memAppendText(p, end, " fp=");
    p = memAppendCacheOffset(p, end, g_cacheDiagFreePtr);
    p = memAppendText(p, end, " ft=");
    p = memAppendText(p, end, g_cacheDiagFreeTag);
    p = memAppendText(p, end, " fs=");
    p = memAppendUnsigned(p, end, g_cacheDiagFreeBlockSize);
    p = memAppendText(p, end, " up=");
    p = memAppendCacheOffset(p, end, g_cacheDiagUserPtr);
    p = memAppendText(p, end, " n=");
    p = memAppendCacheOffset(p, end, g_cacheDiagNode);
    p = memAppendText(p, end, " nt=");
    p = memAppendText(p, end, g_cacheDiagNodeTag);
    p = memAppendText(p, end, " pv=");
    p = memAppendCacheOffset(p, end, g_cacheDiagPrev);
    p = memAppendText(p, end, " bb=");
    p = memAppendCacheOffset(p, end, g_cacheDiagBlockBase);
    p = memAppendText(p, end, " sz=");
    p = memAppendUnsigned(p, end, g_cacheDiagBlockSize);
    p = memAppendText(p, end, " bump=");
    p = memAppendCacheOffset(p, end, g_cacheBumpTop);
    p = memAppendText(p, end, " used=");
    p = memAppendUnsigned(p, end, g_cacheUsed);
    return g_haltReason;
}

static void cacheCorrupt_(const char *what)
{
    memHalt("(allocator)", formatCacheCorruptReason(what));
}

static void cacheValidateUserPtr_(const void *ptr, const char *what)
{
    const unsigned char *p = (const unsigned char *)ptr;
    g_cacheDiagUserPtr = ptr;

    if (g_cacheBase == NULL || g_cacheBumpTop == NULL)
        cacheCorrupt_(what);
    if (p < g_cacheBase + CACHE_HEADER_BYTES ||
        p >= g_cacheBumpTop ||
        p >= g_cacheBase + MEM_CACHE_BUDGET)
        cacheCorrupt_(what);
    if (!cachePtrAligned_(p))
        cacheCorrupt_("CACHE pointer unaligned");
}

static unsigned int cacheReadSizeChecked_(unsigned char *blockBase,
                                          const char *what)
{
    unsigned int sz;

    g_cacheDiagBlockBase = blockBase;
    g_cacheDiagBlockSize = 0;
    if (g_cacheBase == NULL || g_cacheBumpTop == NULL)
        cacheCorrupt_(what);
    if (blockBase < g_cacheBase ||
        blockBase >= g_cacheBumpTop ||
        blockBase >= g_cacheBase + MEM_CACHE_BUDGET)
        cacheCorrupt_(what);
    if (!cachePtrAligned_(blockBase))
        cacheCorrupt_("CACHE header unaligned");
    if ((size_t)(g_cacheBumpTop - blockBase) < CACHE_HEADER_BYTES)
        cacheCorrupt_(what);

    sz = cacheReadSize_(blockBase);
    g_cacheDiagBlockSize = sz;
    if (sz < CACHE_MIN_BLOCK_BYTES ||
        sz > MEM_CACHE_BUDGET ||
        (sz & (unsigned int)(MEM_REGION_ALIGN - 1u)) != 0u)
        cacheCorrupt_("CACHE block size corrupt");
    if ((size_t)(g_cacheBumpTop - blockBase) < (size_t)sz)
        cacheCorrupt_("CACHE block overruns arena");

    return sz;
}

static unsigned int cacheValidateNode_(CacheFreeBlock *fb,
                                       const char *what)
{
    unsigned char *user = (unsigned char *)fb;
    unsigned char *blockBase;

    g_cacheDiagNode = fb;
    g_cacheDiagNodeTag = cacheDiagTagForPtr_(fb);
    cacheValidateUserPtr_(user, what);
    blockBase = user - CACHE_HEADER_BYTES;
    return cacheReadSizeChecked_(blockBase, what);
}

static int cacheFreeListOwnsPtr_(const void *ptr)
{
    const unsigned char *p = (const unsigned char *)ptr;
    CacheFreeBlock *prev = NULL;
    CacheFreeBlock *cur = g_cacheFreeList;
    size_t guard = 0;

    while (cur != NULL) {
        unsigned char *curUser = (unsigned char *)cur;
        unsigned char *curBase = curUser - CACHE_HEADER_BYTES;
        g_cacheDiagPrev = prev;
        unsigned int curSize = cacheValidateNode_(cur, "CACHE free-list corrupt");

        if (prev != NULL && curUser <= (unsigned char *)prev)
            cacheCorrupt_("CACHE free-list order");
        if (p == curUser)
            return 1;
        if (p > curUser && p < curBase + curSize)
            return 2;

        prev = cur;
        cur = cur->next;
        if (++guard > CACHE_MAX_FREE_WALK)
            cacheCorrupt_("CACHE free-list cycle");
    }

    return 0;
}

static void cacheValidateFreeList_(const char *what)
{
    CacheFreeBlock *prev = NULL;
    CacheFreeBlock *cur = g_cacheFreeList;
    size_t guard = 0;

    while (cur != NULL) {
        unsigned char *curUser = (unsigned char *)cur;
        g_cacheDiagPrev = prev;
        cacheValidateNode_(cur, what);
        if (prev != NULL && curUser <= (unsigned char *)prev)
            cacheCorrupt_("CACHE free-list order");
        prev = cur;
        cur = cur->next;
        if (++guard > CACHE_MAX_FREE_WALK)
            cacheCorrupt_("CACHE free-list cycle");
    }
}

/* Insert fb into the free-list at the position that keeps the list
 * sorted by ascending address. Required for coalescing to find
 * adjacent blocks. */
static void cacheInsertSorted_(CacheFreeBlock *fb)
{
    CacheFreeBlock **prev = &g_cacheFreeList;
    CacheFreeBlock *cur   = g_cacheFreeList;
    size_t guard = 0;

    cacheValidateNode_(fb, "CACHE insert bad node");
    if (cacheFreeListOwnsPtr_(fb) != 0)
        cacheCorrupt_("CACHE double free");

    while (cur != NULL && (unsigned char *)cur < (unsigned char *)fb) {
        g_cacheDiagPrev = NULL;
        cacheValidateNode_(cur, "CACHE insert list corrupt");
        prev = &cur->next;
        cur  = cur->next;
        if (++guard > CACHE_MAX_FREE_WALK)
            cacheCorrupt_("CACHE free-list cycle");
    }
    fb->next = cur;
    *prev = fb;
}

/* Walk the sorted free-list and merge each pair of physically
 * adjacent free blocks. Two blocks A and B are adjacent iff
 * A_base + A_size == B_base, where A_base is the header start
 * (user_ptr - CACHE_HEADER_BYTES). The merged block keeps A's
 * header position; its size becomes A_size + B_size; B is dropped
 * from the list. */
static void cacheCoalesce_(void)
{
    CacheFreeBlock *cur = g_cacheFreeList;
    size_t guard = 0;

    cacheValidateFreeList_("CACHE coalesce list corrupt");
    while (cur != NULL && cur->next != NULL) {
        unsigned char *curBase  = (unsigned char *)cur  - CACHE_HEADER_BYTES;
        unsigned char *nextBase = (unsigned char *)cur->next - CACHE_HEADER_BYTES;
        unsigned int   curSize  = cacheReadSizeChecked_(curBase, "CACHE coalesce cur");
        unsigned int   nextSize;

        g_cacheDiagPrev = cur;
        cacheValidateNode_(cur->next, "CACHE coalesce next");
        if ((unsigned char *)cur->next <= (unsigned char *)cur)
            cacheCorrupt_("CACHE free-list order");
        if (curBase + curSize > nextBase)
            cacheCorrupt_("CACHE free-list overlap");

        if (curBase + curSize == nextBase) {
            nextSize = cacheReadSizeChecked_(nextBase, "CACHE coalesce next size");
            cacheWriteSize_(curBase, curSize + nextSize);
            cur->next = cur->next->next;
            /* Don't advance; the merged block might be adjacent to
             * the *new* next. */
        } else {
            cur = cur->next;
        }
        if (++guard > CACHE_MAX_FREE_WALK)
            cacheCorrupt_("CACHE free-list cycle");
    }
}

static void *cacheAllocInternal(size_t size, const char *tag)
{
    /* Lazy init on first use. */
    if (g_cacheBumpTop == NULL) {
        cacheInit_();
    }
    /* Total block size includes the 4-byte header. Round up to align. */
    size_t blockSize = (size + CACHE_HEADER_BYTES + MEM_REGION_ALIGN - 1)
                        & ~((size_t)MEM_REGION_ALIGN - 1);
    if (blockSize < CACHE_MIN_BLOCK_BYTES)
        blockSize = CACHE_MIN_BLOCK_BYTES;
    if (g_cacheBumpTop < g_cacheBase ||
        g_cacheBumpTop > g_cacheBase + MEM_CACHE_BUDGET)
        cacheCorrupt_("CACHE bump corrupt");

    /* Try free-list first (first-fit, with splitting). */
    CacheFreeBlock **prev = &g_cacheFreeList;
    CacheFreeBlock *cur = g_cacheFreeList;
    size_t guard = 0;
    cacheValidateFreeList_("CACHE alloc free-list corrupt");
    while (cur != NULL) {
        unsigned char *blockBase = (unsigned char *)cur - CACHE_HEADER_BYTES;
        g_cacheDiagPrev = NULL;
        g_cacheDiagNode = cur;
        unsigned int  freeSize   = cacheReadSizeChecked_(blockBase,
                                                         "CACHE alloc node");
        if (freeSize >= blockSize) {
            CacheFreeBlock *next = cur->next;
            size_t allocatedBlockSize;
            /* Remove from free-list. */
            *prev = next;
            if (freeSize >= blockSize + CACHE_MIN_SPLIT_BLOCK) {
                /* Split: keep front blockSize bytes; leave the tail
                 * as a new free block. */
                cacheWriteSize_(blockBase, (unsigned int)blockSize);
                unsigned char *tailBase = blockBase + blockSize;
                unsigned int   tailSize = (unsigned int)(freeSize - blockSize);
                cacheWriteSize_(tailBase, tailSize);
                CacheFreeBlock *tailFb =
                    (CacheFreeBlock *)(tailBase + CACHE_HEADER_BYTES);
                cacheInsertSorted_(tailFb);
                g_cacheUsed += blockSize;
                allocatedBlockSize = blockSize;
            } else {
                /* Whole-block take — caller pays for the unused tail. */
                g_cacheUsed += freeSize;
                allocatedBlockSize = freeSize;
            }
            cacheDiagRememberAlloc_(cur, allocatedBlockSize, tag);
            return (void *)cur;
        }
        prev = &cur->next;
        cur  = cur->next;
        if (++guard > CACHE_MAX_FREE_WALK)
            cacheCorrupt_("CACHE free-list cycle");
    }

    /* No free-list match; bump forward. */
    const size_t used = (size_t)(g_cacheBumpTop - g_cacheBase);
    const size_t remaining = MEM_CACHE_BUDGET - used;
    if (blockSize > remaining) {
        /* CACHE exhausted. Caller invokes LRU evictor + retries. */
        return NULL;
    }
    unsigned char *blockBase = g_cacheBumpTop;
    cacheWriteSize_(blockBase, (unsigned int)blockSize);
    g_cacheBumpTop += blockSize;
    g_cacheUsed += blockSize;
    cacheDiagRememberAlloc_(blockBase + CACHE_HEADER_BYTES, blockSize, tag);
    return (void *)(blockBase + CACHE_HEADER_BYTES);
}

static void cacheFreeInternal(void *ptr)
{
    if (ptr == NULL) return;
    g_cacheDiagFreePtr = ptr;
    g_cacheDiagUserPtr = ptr;
    g_cacheDiagNode = NULL;
    g_cacheDiagPrev = NULL;
    g_cacheDiagBlockBase = NULL;
    g_cacheDiagBlockSize = 0;
    g_cacheDiagFreeTag = cacheDiagTagForPtr_(ptr);
    g_cacheDiagNodeTag = "?";
    g_cacheDiagFreeBlockSize = 0;
    /* User pointer is +4 bytes from the block header. */
    cacheValidateUserPtr_(ptr, "CACHE free bad pointer");
    unsigned char *blockBase = (unsigned char *)ptr - CACHE_HEADER_BYTES;
    /* Update g_cacheUsed by the block's size. */
    unsigned int blockSize = cacheReadSizeChecked_(blockBase,
                                                   "CACHE free bad block");
    g_cacheDiagFreeBlockSize = blockSize;
    int ownership = cacheFreeListOwnsPtr_(ptr);
    if (ownership == 1)
        cacheCorrupt_("CACHE double free");
    if (ownership == 2)
        cacheCorrupt_("CACHE stale interior free");
    if ((size_t)blockSize > g_cacheUsed)
        cacheCorrupt_("CACHE used underflow");
    g_cacheUsed -= blockSize;
    /* Insert into the sorted free-list, then merge adjacent blocks.
     * Both ops are O(n) in the free-list length; n stays small (a
     * few hundred at most) so the cost is dwarfed by the CD read
     * that motivated the alloc. */
    CacheFreeBlock *fb = (CacheFreeBlock *)ptr;
    cacheInsertSorted_(fb);
    cacheCoalesce_();
    cacheDiagRememberFree_(ptr);
}

static size_t cacheUsedInternal(void)
{
    /* g_cacheUsed tracks the *live* (unfreed) bytes; that's the
     * accurate value for diagnostics. The bump high-water can be
     * higher because freed blocks haven't been compacted. */
    return g_cacheUsed;
}

/* R33-soak diagnostic: dump CACHE bump high-water and free-list summary
 * to TTY. Helps identify CACHE residency not covered by LRU array walks. */
void memDumpCacheStats(const char *prefix)
{
    if (!g_memInited) return;
    extern int printf(const char *, ...);
    size_t bumpOffset = (size_t)(g_cacheBumpTop - g_cacheBase);
    size_t freeListBlocks = 0;
    size_t freeListBytes = 0;
    CacheFreeBlock *cur = g_cacheFreeList;
    cacheValidateFreeList_("CACHE stats free-list corrupt");
    while (cur != NULL) {
        unsigned char *blockBase = (unsigned char *)cur - CACHE_HEADER_BYTES;
        unsigned int blockSize = cacheReadSizeChecked_(blockBase,
                                                       "CACHE stats node");
        freeListBlocks++;
        freeListBytes += blockSize;
        cur = cur->next;
    }
    printf("%s bumpOffset=%lu free=%lu/%lu blocks live=%lu\n",
           prefix ? prefix : "JCMEM cache-stats",
           (unsigned long)bumpOffset,
           (unsigned long)freeListBytes,
           (unsigned long)freeListBlocks,
           (unsigned long)g_cacheUsed);
}

void memDebugValidateCache(const char *phase)
{
    if (!g_memInited || g_cacheBase == NULL || g_cacheBumpTop == NULL)
        return;

    g_cacheDiagFreePtr = g_cacheDiagOffset4Tag != NULL
        ? (const void *)(g_cacheBase + CACHE_HEADER_BYTES)
        : NULL;
    g_cacheDiagFreeTag = g_cacheDiagOffset4Tag != NULL
        ? g_cacheDiagOffset4Tag
        : "?";
    g_cacheDiagFreeBlockSize = g_cacheDiagOffset4Size;
    g_cacheDiagUserPtr = NULL;
    g_cacheDiagNode = NULL;
    g_cacheDiagPrev = NULL;
    g_cacheDiagBlockBase = NULL;
    g_cacheDiagBlockSize = 0;
    g_cacheDiagNodeTag = "?";
    cacheValidateFreeList_(phase ? phase : "CACHE debug validate");
}

/* R33-soak final fix: rewind the CACHE bump pointer to base if there
 * are zero live bytes. Called from fgRuntimeReset after release+evict
 * has freed every CACHE allocation it knows about; if g_cacheUsed is
 * zero at that point, no live block references CACHE bytes, so the
 * free-list (which may have many fragmented entries from years of
 * scene transitions) can be discarded and bump_top reset to base.
 *
 * Defragments CACHE entirely without needing to relocate live blocks —
 * because there are no live blocks at the moment we run. The next
 * scene's allocations start fresh from the bump pointer, so the
 * cycle of "alloc-fragment-fail" can never accumulate across many
 * scenes.
 *
 * Cost: the unpinned-LRU drop and grow-only buffer release that
 * precede this call collectively re-allocate ~250–400 KB of CACHE
 * for the next scene. LRU re-loads its resources from CD on demand
 * (slower scene setup); grow-only buffers re-allocate from the now-
 * empty CACHE. The CD reload is bounded — each scene only pulls in
 * what its TTM/BMP/SCR/ADS references demand.
 *
 * Returns 1 if rewind happened, 0 if some allocations remain live. */
int memCacheRewindIfEmpty(void)
{
    MEM_REQUIRE(g_memInited);
    MEM_REQUIRE(ps1IsMainContext());

    if (g_cacheUsed != 0) {
        /* Something is still live in CACHE. Don't rewind — that would
         * dangle the live pointer(s). The caller's eviction pass
         * either missed something or LRU has pinned resources. */
        return 0;
    }

    /* No live bytes. The free-list points into now-irrelevant memory;
     * dropping it and rewinding bump_top defragments CACHE in O(1). */
    g_cacheFreeList = NULL;
    g_cacheBumpTop = g_cacheBase;
    /* Keep g_cachePeak as the historical max — useful for telemetry.
     * If we reset it to 0 here, the "peak" reading after a rewind
     * would lie about how much CACHE the workload actually demanded. */
    return 1;
}

void memCachePreEvictForNextScene(const char *effectiveSceneName)
{
    (void)effectiveSceneName;  /* metric lookup elided — see comment below */
    MEM_REQUIRE(g_memInited);
    MEM_REQUIRE(ps1IsMainContext());

    /* Pre-emptively shed unpinned LRU resources before the next
     * scene starts allocating from CACHE. Runs in the already-paused
     * transition window (after the scene pick, before walk-to-scene),
     * so the eviction's ~3-5 ms scan doesn't block the first frame
     * (per plan v9 PR7).
     *
     * Projected-demand lookup against kPackHeaderMetrics is currently
     * disabled — the pack names in the metrics are "ADS:tag" format
     * (e.g., "MARY.ADS:1") while the scene name passed here is the
     * picker slug (e.g., "mary1"). A slug→pack-name resolver lives
     * in story_data.h but isn't wired up yet. For now we trust
     * checkMemoryBudget's existing pinned-bytes comparison — which
     * is idempotent: if pinned bytes already fit the budget, the
     * call is a near-no-op. */
    extern void checkMemoryBudget(void);
    checkMemoryBudget();
}

/* ---------------------------------------------------------------------
 * Diagnostics (consumed by ps1_debug.c via mem_region_extern.h)
 * ------------------------------------------------------------------- */

size_t memRegionUsed(unsigned int region)
{
    if (!g_memInited) return 0;
    switch ((MemRegion)region) {
    case MEM_REGION_BOOT:      return (size_t)(g_bootTop - g_bootBase);
    case MEM_REGION_CACHE:     return cacheUsedInternal();
    case MEM_REGION_TRANSIENT: return (size_t)(g_transientEnd - g_transientNext);
    default: return 0;
    }
}

size_t memRegionPeak(unsigned int region)
{
    if (!g_memInited) return 0;
    switch ((MemRegion)region) {
    case MEM_REGION_BOOT:      return g_bootPeak;
    case MEM_REGION_CACHE:     return g_cachePeak;
    case MEM_REGION_TRANSIENT: return g_transientPeak;
    default: return 0;
    }
}

size_t memSafeRead(unsigned int region)
{
    /* Defensive clamp: bounds the value to [0, region budget] so the
     * BSOD dump can't crash on corrupted metadata. */
    size_t used;
    size_t budget;
    switch ((MemRegion)region) {
    case MEM_REGION_BOOT:      used = memRegionUsed((unsigned int)MEM_REGION_BOOT);
                                budget = MEM_BOOT_BUDGET; break;
    case MEM_REGION_CACHE:     used = memRegionUsed((unsigned int)MEM_REGION_CACHE);
                                budget = MEM_CACHE_BUDGET; break;
    case MEM_REGION_TRANSIENT: used = memRegionUsed((unsigned int)MEM_REGION_TRANSIENT);
                                budget = MEM_TRANSIENT_BUDGET; break;
    default: return 0;
    }
    if (used > budget) used = budget;
    return used;
}

int sceneAllocBalanceGet(void)
{
    return g_sceneAllocBalance;
}

void memLogTelemetry(void)
{
    /* JCMEM line — emitted by callers gated behind FG_HEAP_PROBE_LOGS. */
    extern int printf(const char *, ...);
    printf("JCMEM boot=%lu/%lu cache=%lu/%lu transient=%lu/%lu (peaks %lu %lu %lu) balance=%d\n",
           (unsigned long)memRegionUsed((unsigned int)MEM_REGION_BOOT),
           (unsigned long)MEM_BOOT_BUDGET,
           (unsigned long)memRegionUsed((unsigned int)MEM_REGION_CACHE),
           (unsigned long)MEM_CACHE_BUDGET,
           (unsigned long)memRegionUsed((unsigned int)MEM_REGION_TRANSIENT),
           (unsigned long)MEM_TRANSIENT_BUDGET,
           (unsigned long)g_bootPeak,
           (unsigned long)g_cachePeak,
           (unsigned long)g_transientPeak,
           g_sceneAllocBalance);
}

/* ---------------------------------------------------------------------
 * Boot proof — verify scenes fit at boot
 * ---------------------------------------------------------------------
 *
 * These functions consult src/generated/pack_header_metrics.h for
 * per-scene worst-case footprints. The metrics header is generated
 * offline by scripts/generate-pack-metrics.py (see Phase 1 step 5).
 *
 * The Phase 1 PR includes a minimal stub header that satisfies the
 * link; a follow-up commit replaces it with the real generated
 * content once the generator is wired up.
 * ------------------------------------------------------------------- */

#include "generated/pack_header_metrics.h"

void memVerifyBootBudget(void)
{
    /* R34: when MEM_BOOT_BUDGET is 0 the BOOT region is disabled —
     * no callers allocate from it and the headroom went to CACHE. The
     * pack-metrics sum below is a historical worst-case estimate from
     * when frame/prefetch/window buffers lived in BOOT; with those
     * migrated to CACHE the check is meaningless against a zero
     * budget and would silently halt (graphics not yet up → text
     * panel + infinite loop, no JCBSOD telemetry). Skip cleanly. */
    if (MEM_BOOT_BUDGET == 0u) {
        return;
    }
    /* Sum the worst-case BOOT-region footprints across all packs.
     * The metrics header pre-computes each pack's max frame buffer,
     * prefetch buffer, stream window, stream scratch demands; we
     * take the maximum (since BOOT pre-sizes for the worst case). */
    size_t maxFrame    = 0;
    size_t maxPrefetch = 0;
    size_t maxWindow   = 0;
    size_t maxScratch  = 0;

    for (size_t i = 0; i < kPackHeaderMetricsCount; i++) {
        if (kPackHeaderMetrics[i].maxFrameBytes        > maxFrame)
            maxFrame    = kPackHeaderMetrics[i].maxFrameBytes;
        if (kPackHeaderMetrics[i].maxPrefetchBytes     > maxPrefetch)
            maxPrefetch = kPackHeaderMetrics[i].maxPrefetchBytes;
        if (kPackHeaderMetrics[i].maxStreamWindowBytes > maxWindow)
            maxWindow   = kPackHeaderMetrics[i].maxStreamWindowBytes;
        if (kPackHeaderMetrics[i].maxStreamScratchBytes > maxScratch)
            maxScratch  = kPackHeaderMetrics[i].maxStreamScratchBytes;
    }

    /* BOOT region is mostly idle (the big BOOT migrations were
     * reverted to libc; walk buf, GPU primitives etc. stay there).
     * Small safety estimate. */
    const size_t fixedBoot = 4u * 1024u;
    const size_t bootEstimate = maxFrame + maxPrefetch + maxWindow +
                                maxScratch + fixedBoot;
    if (bootEstimate > MEM_BOOT_BUDGET) {
        memHaltFmt("BOOT", "verify-budget", bootEstimate, MEM_BOOT_BUDGET);
    }
}

void memVerifyAllScenesFitTransient(void)
{
    /* TRANSIENT has libc fallback for overflowing scenes; only warn. */
    extern int printf(const char *, ...);
    int overflows = 0;
    for (size_t i = 0; i < kPackHeaderMetricsCount; i++) {
        const size_t need = kPackHeaderMetrics[i].transientWorstCase;
        if (need > MEM_TRANSIENT_BUDGET) {
            if (overflows < 3) {
                printf("JCMEM WARN: TRANSIENT overflow scene=%s need=%lu budget=%lu (libc fallback)\n",
                       kPackHeaderMetrics[i].packName,
                       (unsigned long)need,
                       (unsigned long)MEM_TRANSIENT_BUDGET);
            }
            overflows++;
        }
    }
    if (overflows > 0) {
        printf("JCMEM WARN: %d scene(s) exceed TRANSIENT budget; libc fallback active\n",
               overflows);
    }
}

void memVerifyAllScenesPinnedFitCache(void)
{
    /* CACHE has LRU eviction + libc fallback; only halt on truly
     * pathological cases (>2x budget). */
    extern int printf(const char *, ...);
    for (size_t i = 0; i < kPackHeaderMetricsCount; i++) {
        const size_t need = kPackHeaderMetrics[i].cachePinnedWorstCase;
        if (need > MEM_CACHE_BUDGET * 2u) {
            memHaltFmt("CACHE", kPackHeaderMetrics[i].packName,
                       need, MEM_CACHE_BUDGET);
        }
        if (need > MEM_CACHE_BUDGET) {
            printf("JCMEM WARN: CACHE pinned overflow scene=%s need=%lu budget=%lu (LRU evict)\n",
                   kPackHeaderMetrics[i].packName,
                   (unsigned long)need,
                   (unsigned long)MEM_CACHE_BUDGET);
        }
    }
}

#ifdef JC_VERIFY_PACK_HASHES
/* CRC-32 via Sarwate's algorithm. 1 KB lookup table in .rodata
 * (BOOT-region budget unaffected). Polynomial 0xEDB88320 (reflected
 * IEEE 802.3 standard CRC32). */
static uint32_t crc32_lookup[256];
static int      crc32_table_inited = 0;

static void crc32_init_table(void)
{
    uint32_t c;
    int i, j;
    for (i = 0; i < 256; i++) {
        c = (uint32_t)i;
        for (j = 0; j < 8; j++) {
            c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        }
        crc32_lookup[i] = c;
    }
    crc32_table_inited = 1;
}

static uint32_t crc32_compute(const uint8_t *data, size_t len)
{
    if (!crc32_table_inited) crc32_init_table();
    uint32_t c = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; i++) {
        c = crc32_lookup[(c ^ data[i]) & 0xFFu] ^ (c >> 8);
    }
    return c ^ 0xFFFFFFFFu;
}

void memVerifyPackHashes(void)
{
    /* For each pack in kPackHeaderMetrics with a non-zero headerCrc,
     * read its header bytes from CD and verify the CRC. memHalt on
     * mismatch with the pack name. ~150 ms per pack at PSX CD
     * speeds (seek + read header + CRC), so ~9 sec total for 63
     * packs. Gated by JC_VERIFY_PACK_HASHES build flag (default off
     * in release; on in dev/QA).
     *
     * NOTE: kPackHeaderMetrics currently has headerCrc = 0 for all
     * entries (the generator doesn't emit CRCs yet — analyzer JSON
     * doesn't include them). When the generator is extended to
     * compute CRCs offline, this verifier becomes meaningful. For
     * now it's a no-op even with the flag on. */
    for (size_t i = 0; i < kPackHeaderMetricsCount; i++) {
        if (kPackHeaderMetrics[i].headerCrc == 0) continue;
        /* TODO: read the pack header bytes from CD; compute CRC via
         * crc32_compute(); compare against kPackHeaderMetrics[i].headerCrc;
         * memHalt on mismatch. CD-read path requires hooking
         * cdrom_ps1.c — deferred. The crc32_compute helper above
         * is ready to use. */
    }
}
#endif
