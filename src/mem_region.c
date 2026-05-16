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

static unsigned char g_memRegionBuf[MEM_REGION_TOTAL];

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

/* ---------------------------------------------------------------------
 * Forward declarations (internal)
 * ------------------------------------------------------------------- */

static void  *cacheAllocInternal(size_t size);
static void   cacheFreeInternal(void *ptr);
static size_t cacheUsedInternal(void);
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
        ps1DebugError("%s: %s",
                      scene  ? scene  : "(boot)",
                      reason ? reason : "(unspecified)");
        for (;;) { /* halt forever — GCC infers non-returning */ }
    }
}

/* Format a halt reason string into a single static buffer.
 * Re-entrancy guard: a depth counter prevents recursive halt-during-
 * halt from racing the buffer. Returns the same buffer or
 * "[concurrent fatal]" on re-entry. */
static char        g_haltReason[160];
static volatile int g_haltDepth = 0;

static const char *formatHaltReason(const char *region, const char *what,
                                    size_t required, size_t available)
{
    if (g_haltDepth++ > 0) {
        return "[concurrent fatal]";
    }
    /* PSX libc printf is non-allocating per project invariant. */
#ifdef PS1_BUILD
    extern int snprintf(char *, size_t, const char *, ...);
#else
    /* host stdio.h supplies snprintf */
#endif
    snprintf(g_haltReason, sizeof(g_haltReason),
             "%s %s: req=%lu have=%lu",
             region ? region : "?",
             what   ? what   : "?",
             (unsigned long)required,
             (unsigned long)available);
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
    /* Region layout: BOOT | CACHE | TRANSIENT */
    g_bootBase      = &g_memRegionBuf[0];
    g_cacheBase     = g_bootBase  + MEM_BOOT_BUDGET;
    g_transientBase = g_cacheBase + MEM_CACHE_BUDGET;
    g_transientEnd  = g_transientBase + MEM_TRANSIENT_BUDGET;

    g_bootTop       = g_bootBase;
    g_transientNext = g_transientEnd;  /* grows DOWN */

    g_bootPeak      = 0;
    g_cacheUsed     = 0;
    g_cachePeak     = 0;
    g_transientPeak = 0;

    g_sceneAllocBalance = 0;
    g_bootFrozen        = 0;
    g_memInited         = 1;

    /* Initialize CACHE sub-allocator (segregated free-list). */
    /* See cacheAllocInternal et al. — placeholder for now. */
}

void memFreezeBoot(void)
{
    MEM_REQUIRE(g_memInited);
    g_bootFrozen = 1;
}

/* ---------------------------------------------------------------------
 * memAlloc / memFree
 * ------------------------------------------------------------------- */

void *memAlloc(MemRegion region, size_t size, const char *tag)
{
    (void)tag;  /* tag is for telemetry only; not stored */

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
            memHaltFmt("TRANSIENT", "exhausted",
                       alignedSize, remaining);
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
        /* cacheAllocInternal updates g_cacheUsed itself (per-block,
         * including the 4-byte header). NULL return means the
         * free-list missed and the bump's headroom was insufficient. */
        void *p = cacheAllocInternal(alignedSize);
        if (p == NULL) {
            /* CACHE exhausted: call the LRU evictor (resource.c
             * provides it as a weak symbol so the link stays clean
             * even when the evictor isn't compiled in). The evictor
             * walks LRU and calls memFree(CACHE, ...) on unpinned
             * resources, returning freed bytes to the free-list. */
            extern void checkMemoryBudget(void);
            checkMemoryBudget();
            p = cacheAllocInternal(alignedSize);
            if (p == NULL) {
                memHaltFmt("CACHE", "exhausted (eviction insufficient)",
                           alignedSize, MEM_CACHE_BUDGET - g_cacheUsed);
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
        /* Bytes are not reclaimed individually; memSceneReset wipes
         * the whole region. We just decrement the outstanding count
         * so memSceneReset can assert balance == 0. */
        if (g_sceneAllocBalance > 0) {
            g_sceneAllocBalance--;
        }
        return;

    case MEM_REGION_CACHE:
        /* Real release. Used only by the LRU evictor — game code
         * touches CACHE via pinResource/unpinResource. */
        cacheFreeInternal(ptr);
        /* g_cacheUsed accounting is updated inside cacheFreeInternal. */
        return;

    default:
        memHalt("(allocator)", "memFree: bad region");
    }
}

void memSceneReset(const char *sceneName)
{
    (void)sceneName;
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

    /* Wipe — bump the pointer back to the top of TRANSIENT (which
     * grows DOWN, so top == g_transientEnd). */
    g_transientNext     = g_transientEnd;
    g_sceneAllocBalance = 0;

#ifdef MEM_POISON_TRANSIENT
    /* Debug-only: fill with 0xCD so any code that reads
     * uninitialized TRANSIENT bytes gets a recognizable pattern. */
    memset(g_transientBase, 0xCD, MEM_TRANSIENT_BUDGET);
#endif
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
static CacheFreeBlock *g_cacheFreeList;

static void cacheInit_(void)
{
    g_cacheBumpTop  = g_cacheBase;
    g_cacheFreeList = NULL;
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

static void *cacheAllocInternal(size_t size)
{
    /* Lazy init on first use. */
    if (g_cacheBumpTop == NULL) {
        cacheInit_();
    }
    /* Total block size includes the 4-byte header. Round up to align. */
    const size_t blockSize = (size + CACHE_HEADER_BYTES + MEM_REGION_ALIGN - 1)
                              & ~((size_t)MEM_REGION_ALIGN - 1);

    /* Try free-list first (first-fit). */
    CacheFreeBlock **prev = &g_cacheFreeList;
    CacheFreeBlock *cur = g_cacheFreeList;
    while (cur != NULL) {
        unsigned char *blockBase = (unsigned char *)cur - CACHE_HEADER_BYTES;
        unsigned int  freeSize   = cacheReadSize_(blockBase);
        if (freeSize >= blockSize) {
            /* Remove from free-list. */
            *prev = cur->next;
            /* The free block's reported size may be larger than what
             * we need; we honor the full block (no splitting in this
             * implementation). g_cacheUsed adds back the full block. */
            g_cacheUsed += freeSize;
            return (void *)cur;
        }
        prev = &cur->next;
        cur  = cur->next;
    }

    /* No free-list match; bump forward. */
    const size_t used = (size_t)(g_cacheBumpTop - g_cacheBase);
    const size_t remaining = MEM_CACHE_BUDGET - used;
    if (blockSize > remaining) {
        /* CACHE exhausted. TODO(phase-1): integrate LRU eviction
         * before returning NULL — caller halts in that case. */
        return NULL;
    }
    unsigned char *blockBase = g_cacheBumpTop;
    cacheWriteSize_(blockBase, (unsigned int)blockSize);
    g_cacheBumpTop += blockSize;
    g_cacheUsed += blockSize;
    return (void *)(blockBase + CACHE_HEADER_BYTES);
}

static void cacheFreeInternal(void *ptr)
{
    if (ptr == NULL) return;
    /* User pointer is +4 bytes from the block header. */
    unsigned char *blockBase = (unsigned char *)ptr - CACHE_HEADER_BYTES;
    /* Update g_cacheUsed by the block's size. */
    unsigned int blockSize = cacheReadSize_(blockBase);
    if (blockSize <= g_cacheUsed) {
        g_cacheUsed -= blockSize;
    } else {
        g_cacheUsed = 0;  /* defensive on counter underflow */
    }
    /* Prepend to free-list. The block's body is reinterpreted as a
     * CacheFreeBlock; first sizeof(void *) bytes hold `next`. */
    CacheFreeBlock *fb = (CacheFreeBlock *)ptr;
    fb->next = g_cacheFreeList;
    g_cacheFreeList = fb;
}

static size_t cacheUsedInternal(void)
{
    /* g_cacheUsed tracks the *live* (unfreed) bytes; that's the
     * accurate value for diagnostics. The bump high-water can be
     * higher because freed blocks haven't been compacted. */
    return g_cacheUsed;
}

void memCachePreEvictForNextScene(const char *effectiveSceneName)
{
    (void)effectiveSceneName;
    MEM_REQUIRE(g_memInited);
    MEM_REQUIRE(ps1IsMainContext());

    /* TODO(phase-1): consult pack_header_metrics.h for the next
     * scene's cachePinnedWorstCase, subtract the current pinned
     * working set, and evict unpinned resources until the delta
     * fits. For the stub, this is a no-op. */
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

    /* Compile-time constants for fixed BOOT residents (walk buf,
     * surface pool, structs). Re-evaluated as Phase 1 progresses. */
    const size_t fixedBoot =
        (149u * 1024u) +      /* walk clean buffer */
        (300u * 1024u) +      /* grBackgroundSfc backing */
        (32u  * 1024u);       /* assorted structs + audio + font */

    const size_t bootEstimate = maxFrame + maxPrefetch + maxWindow +
                                maxScratch + fixedBoot;

    if (bootEstimate > MEM_BOOT_BUDGET) {
        memHaltFmt("BOOT", "verify-budget", bootEstimate, MEM_BOOT_BUDGET);
    }
}

void memVerifyAllScenesFitTransient(void)
{
    for (size_t i = 0; i < kPackHeaderMetricsCount; i++) {
        const size_t need = kPackHeaderMetrics[i].transientWorstCase;
        if (need > MEM_TRANSIENT_BUDGET) {
            memHaltFmt("TRANSIENT", kPackHeaderMetrics[i].packName,
                       need, MEM_TRANSIENT_BUDGET);
        }
    }
}

void memVerifyAllScenesPinnedFitCache(void)
{
    for (size_t i = 0; i < kPackHeaderMetricsCount; i++) {
        const size_t need = kPackHeaderMetrics[i].cachePinnedWorstCase;
        if (need > MEM_CACHE_BUDGET) {
            memHaltFmt("CACHE", kPackHeaderMetrics[i].packName,
                       need, MEM_CACHE_BUDGET);
        }
    }
}

#ifdef JC_VERIFY_PACK_HASHES
void memVerifyPackHashes(void)
{
    /* TODO(phase-1): for each entry in kPackHeaderMetrics, read the
     * pack's header bytes from CD, CRC-32 them, compare against
     * .headerCrc. memHalt on mismatch. ~9 seconds total CD work. */
}
#endif
