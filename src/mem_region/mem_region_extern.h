/*
 * mem_region_extern.h — cross-module forward declarations for the
 * memory-region allocator. Consumed by src/ps1_debug.c (which reads
 * region state in the BSOD detail block) without pulling in the full
 * mem_region.h public API.
 *
 * Uses `unsigned int` for the region parameter instead of the
 * `MemRegion` enum so callers don't need the enum body. The actual
 * function definitions in mem_region.c take `MemRegion`; enums and
 * int are link-compatible in C since enums are int-typed under the
 * hood. Originally tried an incomplete-enum forward decl but
 * -Wpedantic flagged it as a GCC extension; switching to `unsigned
 * int` keeps the build warning-free.
 *
 * Single source of truth: mem_region.h includes this header for its
 * own definitions, so the signatures stay in sync by linker check.
 */

#ifndef MEM_REGION_EXTERN_H
#define MEM_REGION_EXTERN_H

#include <stddef.h>

/* Region byte counters (current and peak). Used by ps1Bsod to dump
 * the JCBSOD memBootUsed/Peak (etc.) lines on a halt. Pass the
 * enum constant directly — int-compatible at the ABI. */
extern size_t memRegionUsed(unsigned int region);
extern size_t memRegionPeak(unsigned int region);

/* Clamped read of memRegionUsed — defensive against data corruption
 * in the BSOD path. Returns 0 if region is out of range, otherwise
 * the used count clamped to [0, region's budget]. */
extern size_t memSafeRead(unsigned int region);

/* Scene allocation balance — count of TRANSIENT-region allocations
 * outstanding since the last memSceneReset. Zero in steady state.
 * Used by ps1Bsod for diagnostic continuity. */
extern int sceneAllocBalanceGet(void);

#endif /* MEM_REGION_EXTERN_H */
