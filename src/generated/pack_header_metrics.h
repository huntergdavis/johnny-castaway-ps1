/*
 * src/generated/pack_header_metrics.h
 *
 * **STUB** — replaced by generated content from
 * scripts/generate-pack-metrics.py once the generator lands in
 * Phase 1 step 5. The stub provides empty data so the allocator
 * code compiles; memVerify* checks against the empty list are
 * trivially satisfied (no scenes to check). This is acceptable for
 * the infrastructure commit; the next commit wires up real metrics.
 *
 * DO NOT edit by hand. The generator script is the source of truth.
 */

#ifndef MEM_REGION_PACK_HEADER_METRICS_H
#define MEM_REGION_PACK_HEADER_METRICS_H

#include <stddef.h>
#include <stdint.h>

struct PackHeaderMetric {
    const char *packName;
    uint32_t    maxFrameBytes;
    uint32_t    maxPrefetchBytes;
    uint32_t    maxStreamWindowBytes;
    uint32_t    maxStreamScratchBytes;
    uint32_t    transientWorstCase;
    uint32_t    cachePinnedWorstCase;
    uint32_t    headerCrc;
};

/* Stub: zero scenes. memVerify* loops are no-ops until the generator
 * fills this in. */
extern const struct PackHeaderMetric kPackHeaderMetrics[];
extern const size_t kPackHeaderMetricsCount;

#endif
