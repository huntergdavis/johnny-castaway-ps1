/*
 * ps1_mem_model.h — bit-level memory model of the PS1 build, extracted
 * from the real source, for the host memory simulator/fuzzer
 * (tests/mem_region_fuzz.c, tests/mem_sim.c).
 *
 * Every constant here is sourced from a specific file:line in the PS1
 * tree (cited inline). The simulator links the REAL allocator
 * (src/mem_region.c) and drives it with these profiles so a simulated
 * run exercises the genuine free-list/coalescing/relief logic — letting
 * us run millions of full-day-soak-equivalents in seconds and match
 * them against the real soak telemetry.
 *
 * FIDELITY STATUS (2026-06-14): memory-side constants are exact;
 * per-scene clean-rect SIZES are computed from the real estimate
 * geometry (PS1_CLEANRECT_*) using per-scene foreground bounds that are
 * still being tabulated from pack headers (see SCENE table notes).
 * Picker sequence is currently sampled (not the exact in-game RNG) —
 * see docs/ps1/mem-fuzz-sim.md for the fidelity roadmap.
 */
#ifndef PS1_MEM_MODEL_H
#define PS1_MEM_MODEL_H

/* ---- Region budgets (src/mem_region/mem_region.h:154-156) ---- */
#define PS1_BOOT_BUDGET       (0u)
#define PS1_CACHE_BUDGET      (672u*1024u)   /* 688128 */
#define PS1_TRANSIENT_BUDGET  (768u*1024u)   /* 786432 */

/* ---- Retained "stable shape" CACHE band, reserved at boot
 * (foregroundPilotReserveStableShape, foreground_pilot.c:2436+).
 * Sizes are block-user bytes; the allocator adds a 4B header + 4B
 * align rounding. ---- */
#define PS1_STREAM_WINDOW_BYTES   (96u*1024u)  /* FG_NEXT_STAGE_SIDE_BYTES, 98304 */
#define PS1_FLOOR_SLAB_BYTES      98304u       /* GR_CLEAN_SLAB_FLOOR_BYTES, x2 */
#define PS1_WALK_PSB_BYTES        49152u       /* walkPilotReservePsbSlab(49152) */
#define PS1_FRAME_BYTES           16384u       /* gFgFrameBuffer init (grows to 112K) */
#define PS1_PREFETCH_BYTES        16384u       /* gFgPrefetchFrameBuffer init */
#define PS1_SCRATCH_BYTES         16384u       /* gFgStreamScratch init */
#define PS1_FRAME_MAX_BYTES       (112u*1024u) /* grow-only cap */

/* ---- Island backdrop sheets + SCR cache (background_screen.c.inc) ---- */
#define PS1_BACKGRND_PSB_BYTES    94208u       /* slot 0, island constant */
#define PS1_HOLIDAY_PSB_BYTES     26624u       /* slot 2, holiday windows only */
#define PS1_SCR_CACHE_BYTES       153600u      /* 480 * SCR_STREAM_ROW_BYTES(320) */

/* ---- Clean-rect engine (clean_rects.c.inc) ---- */
#define PS1_CLEANRECT_SLOTS       16u          /* GR_MAX_CLEAN_RECTS (was 8) */
#define PS1_CLEANRECT_CAP_DEFAULT (96u*1024u)  /* gFgCleanRectMaxBytes default */
#define PS1_SLAB_POOL_CAP_BYTES   (2u*98308u + 32768u)

/* Clean-rect dirty-region geometry (backdrop_clean.c.inc:190-359).
 * Wave band unioned with the foreground bounding box; split at y=190
 * into an optional upper rect + a lower rect, clamped to 640x480, each
 * sub-divided into <=cap strips. */
#define PS1_CR_WAVE_MIN_X   129
#define PS1_CR_WAVE_MIN_Y   303
#define PS1_CR_WAVE_END_X   608
#define PS1_CR_WAVE_END_Y   356
#define PS1_CR_UPPER_SPLIT_Y 190
#define PS1_CR_SCREEN_W     640
#define PS1_CR_SCREEN_H     480

/* ---- Relief tier order (fgCachePressureRelief, foreground_pilot.c:2169+).
 * Each tier fires only if its yield >= request; else fall through. ---- */
typedef enum {
    PS1_RELIEF_SUBFLOOR_SLABS = 0, /* grLargestPooledCleanRectSlabBytes(0) */
    PS1_RELIEF_WALK_SLAB,          /* walkPilotPsbSlabIdleBytes (49152) */
    PS1_RELIEF_FLOOR_SLABS,        /* if req<=98304: grLargestPooled...(1) */
    PS1_RELIEF_SCR_CACHE,          /* grScrCacheResidentBytes (153600) */
    PS1_RELIEF_LAST_RESORT         /* free everything cheapest-first */
} Ps1ReliefTier;

/* ---- Scheduled rebuild triggers (fgMaybeScheduledCacheRebuild). ---- */
#define PS1_REBUILD_SCENE_CAP        40   /* FG_REBUILD_SCENE_CAP */
#define PS1_REBUILD_COOLDOWN         20
#define PS1_REBUILD_RELIEF_MIN        2   /* gFgReliefSinceRebuild >= 2 ... */
#define PS1_REBUILD_RELIEF_LARGEST   98304u /* ...&& largest < 98304 */
#define PS1_REBUILD_SCR_STREAK        3
#define PS1_REBUILD_SCR_LARGEST   (160u*1024u)

/* ---- Scene catalog (scene_catalog.c.inc:66-86): 63 FG2 scenes in 10
 * families. Per-scene clean-rect cap overrides (stream_runtime.c.inc):
 * default 96K except the rows below. islandScene=1 => uses the wave
 * band + SCR cache; 0 => building/interior. ---- */
typedef struct {
    const char *name;
    unsigned    capLow;   /* gFgCleanRectMaxBytes, low tide */
    unsigned    capHigh;  /* high tide */
    int         island;   /* contributes wave band + SCR refill */
} Ps1SceneProfile;

#define KB(n) ((n)*1024u)
static const Ps1SceneProfile PS1_SCENES[] = {
    /* name        capLow     capHigh    island */
    { "building2", KB(80),    KB(96),    0 },   /* stream_runtime.c.inc:1584 */
    { "visitor3",  KB(96),    KB(64),    1 },   /* stream_runtime.c.inc:1514 */
    { "walkstuf1", KB(96),    KB(64),    1 },   /* :1639/:1654 (low fixed to 96 here) */
    /* All other 60 scenes use the 96K default at both tides. The full
     * family list (activity1,4-12; building1-7; fishing1-8; johnny1-6;
     * mary1-5; miscgag1-2; stand1-12,15-16; suzy1-2; visitor1,4-7;
     * walkstuf2-3) is enumerated in scene_catalog.c.inc:66-86 and added
     * to this table with per-scene foreground bounds as they are
     * tabulated from the FG2 pack headers. */
};
#define PS1_SCENE_COUNT ((int)(sizeof(PS1_SCENES)/sizeof(PS1_SCENES[0])))

/* Island position ranges when VARPOS_OK (host/story.c:403-415). The
 * clean-rect size varies with position via sprite clipping. */
#define PS1_POS_X_MIN  (-222)
#define PS1_POS_X_MAX  (19)
#define PS1_POS_Y_MIN  (-73)
#define PS1_POS_Y_MAX  (84)

/* ---- Holidays: 36 records (holidays_table.c). One shared 26624-byte
 * HOLIDAY sheet; per-holiday variation is sprite index/position only,
 * NOT sheet size — so holiday adds a constant 26624 to the band. ---- */
#define PS1_HOLIDAY_COUNT  36

/* ---- Non-scene features (competing CACHE only; static BSS excluded). ---- */
#define PS1_FREEPLAY_CLEANRECT_MAX  (96u*1024u)  /* fpOverlay clean rects */
#define PS1_FREEPLAY_ISLAND_MAX     (150u*1024u) /* island backdrop assets */
/* Pause menu / scene explorer / frog-clock: static BSS or libc one-shot,
 * do NOT compete with the scene-playback CACHE working set. */

#endif /* PS1_MEM_MODEL_H */
