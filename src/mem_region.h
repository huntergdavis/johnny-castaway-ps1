/*
 * mem_region.h — memory region allocator for the PS1 port of jc_reborn.
 *
 * Deterministic, three-region allocator backed by one static buffer.
 * Allocations cannot fail at runtime: if a region cannot satisfy a
 * request, the program halts via memHalt (which routes to JC_BSOD
 * after graphics is up, or fatalError before).
 *
 * Three regions, by lifetime:
 *   BOOT      — allocated once at startup, never freed. Bump-up.
 *               Frozen after memFreezeBoot(); any later BOOT alloc
 *               is a memHalt.
 *   CACHE     — LRU-managed resource cache (BMP/TTM/SCR/ADS data).
 *               Segregated free-list with non-recursive eviction.
 *   TRANSIENT — per-scene scratch; wiped wholesale at memSceneReset.
 *               Bump-down.
 *
 * See:
 *   docs/ps1/memory-region-allocator-plan.md   — full plan
 *   docs/ps1/mem-region-decision-tree.md       — which region to use
 *   docs/ps1/mem-region-README.md              — doc index
 */

#ifndef MEM_REGION_H
#define MEM_REGION_H

#include <stddef.h>
#include "mem_region_extern.h"

/* Region identifiers. Defined here as a complete enum; the forward
 * declaration in mem_region_extern.h is the same name. */
typedef enum MemRegion {
    MEM_REGION_BOOT,        /* bump up; freezes after boot */
    MEM_REGION_CACHE,       /* LRU; eviction allowed */
    MEM_REGION_TRANSIENT,   /* bump down; wipes between scenes */
    MEM_REGION_COUNT_       /* sentinel, do not use as a region */
} MemRegion;

/* All allocations are aligned to MEM_REGION_ALIGN. R3000A only needs
 * 4-byte alignment for the scalars we use (uint32, pointer). The
 * largest scalar we use is uint32 / pointer; no double-precision. */
#define MEM_REGION_ALIGN 4

/* Region budgets (compile-time constants). Sized against the v9 plan's
 * budget table:
 *   - BOOT       ~350 KB (engine + frame buffers + walk buf + structs)
 *   - CACHE      ~600 KB (resource working set, MARY peak 568 KB)
 *   - TRANSIENT  ~250 KB (clean-rect ≤181 KB + per-scene scratch)
 *   - Total      1.2 MB (fits under usable RAM per linker map)
 */
/* Region budgets — set to 0 (all-libc-backed mode).
 *
 * Empirical testing revealed that ANY static region buffer of
 * useful size (≥600 KB) collides with this codebase's existing
 * libc demand during scene playback. Key existing libc allocations:
 *   - OCEAN.SCR background: 150 KB
 *   - FG2 pack body + sectorBuffer scratch: 300 KB peak temp
 *   - Other graphics surfaces / walk PSB / etc.: 100+ KB
 * Even with RESOURCE.001 streamed, a 600 KB region leaves libc with
 * insufficient contiguous space for the OCEAN load, etc.
 *
 * All-libc-backed mode: the allocator API runs but every allocation
 * routes through libc malloc. TRANSIENT still preserves its
 * wholesale-wipe semantic via a linked list of libc pointers freed
 * at memSceneReset — the central architectural benefit (per-scene
 * cleanup invariant) is delivered without competing for the static
 * buffer pool.
 *
 * To enable static-buffer mode in the future, the prerequisites are:
 *   1. RESOURCE.001 streamed (DONE — ps1_fopen_stream)
 *   2. OCEAN/scene-pack loads migrated to region or streamed
 *   3. graphics-surface allocations migrated to region
 *   4. sector-buffer scratch migrated to TRANSIENT
 * After those, libc's transient demand drops below ~200 KB and a
 * 1 MB+ static region becomes feasible. */
/* Static region budgets — re-enabled after migrating the big libc
 * allocations (ps1_fopen file buffer, ps1_streamReadFromCdFile
 * sectorBuffer + result) into CACHE/TRANSIENT. With those migrated,
 * libc demand post-memInit drops to ~50 KB (small struct allocations
 * + audio init scratch), leaving the rest of usable RAM for the
 * region buffer.
 *
 *   BOOT: 32 KB (placeholder — most BOOT residents still libc)
 *   CACHE: 700 KB (MARY pinned 568 KB + file buffer + result peaks)
 *   TRANSIENT: 280 KB (sectorBuffer 300 KB peak split + scene scratch)
 *   Total: 1012 KB (~1 MB) */
/* Red-team measurements drove these budgets (Rounds 10-13):
 *   - R10: CACHE 700→800 KB after mary1 peak hit 716 KB
 *   - R10: CACHE 800→900 KB after johnny1 needed 852 KB
 *   - R11: TRANSIENT held at 256 KB; bumping to 288 broke johnny1's
 *          libc fallback path (CACHE-overflow needs 114 KB contig)
 *   - R12: sectorBuffer + grBlitTempBuf moved to CACHE
 *   - R13: tested CACHE 1000 + TRANSIENT 156 with clean-rect→CACHE;
 *          activity10/activity1 STILL overflowed (clean-rect 300 KB +
 *          bg-tile 600 KB + LRU residency > 1000 KB CACHE). Reverted
 *          to R12 budgets and kept clean-rect in TRANSIENT. The
 *          fundamental constraint is PS1's 1.5 MB practical RAM
 *          vs ~1.2 MB simultaneous live memory across heavy
 *          scene types; no single static partition satisfies all.
 *
 * Total = 1188 KB, under the 1228 KB linker-map ceiling. */
/* Round 16 Option M: bump CACHE to 1024 KB.
 * Reasoning:
 * - Linker map: _end ~ 447 KB, stack ~ 64 KB → 1537 KB usable RAM for region+libc
 * - Region @ 1312 KB → libc has 225 KB total, 77 KB free after walk_clean (148 KB)
 * - johnny1 (CACHE peak 852 KB) fits 1024 KB natively — no libc fallback needed,
 *   so libc-headroom shrinkage doesn't break its fallback path
 * - visitor3 needs CACHE peak ~973 KB at BSOD point — fits 1024 KB with 51 KB margin
 * - Other scenes' peaks all < 800 KB — no impact
 * - Bumped _Static_assert ceiling accordingly */
#define MEM_BOOT_BUDGET      (  32u * 1024u)
#define MEM_CACHE_BUDGET     (1024u * 1024u)
#define MEM_TRANSIENT_BUDGET ( 256u * 1024u)
#define MEM_REGION_TOTAL     (MEM_BOOT_BUDGET + MEM_CACHE_BUDGET + MEM_TRANSIENT_BUDGET)

/* Hard ceiling against the linker map (build-ps1/jcreborn.map):
 *   _end = 0x800ad4fc → exe+BSS = 629 KB
 *   usable RAM = 1.92 MB - 629 KB exe/BSS - 64 KB stack = ~1.26 MB
 * Keep the total comfortably under 1.2 MB so other dynamic state
 * (PsyQ padmgr, memcard pool, etc.) has headroom. */
/* All-libc-backed mode (MEM_REGION_TOTAL = 0): no static buffer
 * constraint applies. _Static_assert kept as documentation of the
 * historical 1.2 MB ceiling, trivially satisfied at 0. */
#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 201112L
_Static_assert(MEM_REGION_TOTAL <= (1340u * 1024u),
               "MEM_REGION_TOTAL exceeds 1340 KB ceiling — leaves <77 KB libc headroom");
#endif
#endif

/* ---------------------------------------------------------------------
 * Boot lifecycle
 * ------------------------------------------------------------------- */

/* Initialize the allocator. Must be called as one of the first things
 * in main(), AFTER ps1DebugInit (the pre-graphics halt path needs the
 * debug overlay) but BEFORE any allocation. Subsequent memAlloc calls
 * are safe; calls before memInit halt. */
void memInit(void);

/* Freeze the BOOT region. Called once all boot-time BOOT allocations
 * are complete (after audioInit, resourceCatalogParse, fontInit,
 * surfacePoolInit, walkPilotInit, pauseMenuInit). Subsequent BOOT
 * allocations halt. */
void memFreezeBoot(void);

/* Returns 1 once memInit() has completed; 0 before. Used by code that
 * may run both pre- and post-init (e.g., ps1_fopen called during
 * early TITLE.RAW load before memInit, and later for OCEAN.SCR after
 * memInit) — early callers stay on libc, later ones use regions. */
int memIsReady(void);

/* ---------------------------------------------------------------------
 * Allocation API
 * ------------------------------------------------------------------- */

/* Allocate from the given region. Returns a valid pointer or halts.
 * Never returns NULL.
 *
 * `tag` is for telemetry only — NOT stored per-allocation (no per-
 * allocation header). Consumed by the call site that provides it.
 * May be NULL.
 *
 * Alignment: returned pointer is MEM_REGION_ALIGN-byte aligned. */
void *memAlloc(MemRegion region, size_t size, const char *tag);

/* Free a previously-allocated pointer.
 *   BOOT:      no-op (allocations are permanent); halts post-freeze.
 *   CACHE:     real release; used by LRU evictor.
 *   TRANSIENT: decrements sceneAllocBalance; bytes are not reclaimed
 *              until the next memSceneReset.
 *
 * Passing NULL is safe (no-op). */
void memFree(MemRegion region, void *ptr);

/* Reset the TRANSIENT region. Wipes the bump pointer back to the top
 * of TRANSIENT. In debug builds with MEM_POISON_TRANSIENT defined,
 * also fills the region with 0xCD to catch use-after-reset bugs.
 *
 * Asserts sceneAllocBalance == 0 before wipe (debug only; release
 * does not check). */
void memSceneReset(const char *sceneName);

/* ---------------------------------------------------------------------
 * Halt primitive
 * ------------------------------------------------------------------- */

/* Unified halt — every allocator failure path calls this. Internally
 * dispatches:
 *   - JC_BSOD(scene, reason) if graphics is initialized
 *   - ps1DebugError + while(1) otherwise
 *
 * Never returns. */
__attribute__((noreturn))
void memHalt(const char *scene, const char *reason);

/* MEM_REQUIRE: invariant check that ships in release builds. Hand-
 * rolled because C's assert() compiles out under NDEBUG. ~10 cycles
 * per check on PSX (mostly the COP0 read in callers that use
 * ps1IsMainContext). */
#define MEM_REQUIRE(cond)                                       \
    do {                                                        \
        if (!(cond)) memHalt("(mem_require)",                   \
                             "invariant failed: " #cond);       \
    } while (0)

/* ---------------------------------------------------------------------
 * Context predicate
 * ------------------------------------------------------------------- */

/* True if we're running in main (non-ISR) context. Used by
 * MEM_REQUIRE inside memAlloc/memFree to enforce the allocator's
 * main-thread-only contract. Implemented in mem_region.c using
 * COP0 SR/CAUSE reads on PSX; always returns 1 on PC. */
int ps1IsMainContext(void);

/* ---------------------------------------------------------------------
 * Boot proof
 * ------------------------------------------------------------------- */

/* Boot-time verification that every scene's worst-case footprint fits
 * within its region's budget. Called from main() after memInit but
 * before any other init. memHalt on any violation.
 *
 * The arguments to each verify function come from
 * src/generated/pack_header_metrics.h (offline-generated). */
void memVerifyBootBudget(void);
void memVerifyAllScenesFitTransient(void);
void memVerifyAllScenesPinnedFitCache(void);

#ifdef JC_VERIFY_PACK_HASHES
/* Verify each pack's header CRC-32 against the offline-recorded hash.
 * Dev/QA builds only — adds ~9 sec of CD work at boot. Release builds
 * skip this and trust the offline metrics. */
void memVerifyPackHashes(void);
#endif

/* ---------------------------------------------------------------------
 * Diagnostics
 * ------------------------------------------------------------------- */

/* memRegionUsed/Peak/memSafeRead and sceneAllocBalanceGet are declared
 * in mem_region_extern.h (single source of truth). */

/* Emit one JCMEM telemetry line summarizing current region state.
 * Gated behind FG_HEAP_PROBE_LOGS in callers — this function itself
 * just emits when called. */
void memLogTelemetry(void);

/* CACHE pre-emptive eviction. Called from the main scene-loop after
 * fgLoopApplyVariant has resolved the effective scene name, before
 * fgLoopWalkToScene. Frees unpinned CACHE entries until the next
 * scene's worst-case pinned set will fit. Runs during the already-
 * paused walk-to-scene window; ~3-5 ms in the worst case. */
void memCachePreEvictForNextScene(const char *effectiveSceneName);

#endif /* MEM_REGION_H */
