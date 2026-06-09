static uint16 fgRuntimeWindowMinSlackVBlanks(void)
{
    if (gFgRuntimeSceneId == FG_SCENE_MARY3)
        return FG_MARY3_WINDOW_MIN_SLACK_VBLANKS;
    if (islandState.lowTide && gFgRuntimeSceneId == FG_SCENE_BUILDING2)
        return FG_BUILDING2_LOW_WINDOW_MIN_SLACK_VBLANKS;
    if (gFgRuntimeSceneId == FG_SCENE_BUILDING6)
        return FG_BUILDING6_WINDOW_MIN_SLACK_VBLANKS;
    if (islandState.lowTide && gFgRuntimeSceneId == FG_SCENE_WALKSTUF1)
        return FG_PREFETCH_WINDOW_MIN_SLACK_VBLANKS + 1;
    if (!islandState.lowTide && gFgRuntimeSceneId == FG_SCENE_VISITOR3)
        return FG_VISITOR3_HIGH_WINDOW_MIN_SLACK_VBLANKS;
    if (islandState.lowTide && gFgRuntimeSceneId == FG_SCENE_VISITOR3)
        return FG_VISITOR3_LOW_DUAL_SEGMENT_MIN_SLACK_VBLANKS;
    return FG_PREFETCH_WINDOW_MIN_SLACK_VBLANKS;
}

static int fgRuntimeWindowSlackEligible(uint16 slackVBlanks)
{
    return slackVBlanks >= fgRuntimeWindowMinSlackVBlanks();
}

static int fgRuntimeShouldDirectStageEntry(uint16 frameIndex,
                                           const struct TFgPilotEntry *entry,
                                           uint16 slackVBlanks)
{
    uint16 minSlack;

    if (entry == NULL ||
        entry->dataSize > FG_PREFETCH_DIRECT_STAGE_MAX_BYTES)
        return 0;

    minSlack = fgRuntimeWindowMinSlackVBlanks();
    if (slackVBlanks == minSlack)
        return 1;

    /* VISITOR3 high frame132 is tiny after D4 compression, but a 3-VBlank
     * direct stage can take 4 VBlanks. Pull it into the earlier slack slot. */
    if (!islandState.lowTide &&
        frameIndex == 132 &&
        gFgRuntimeSceneId == FG_SCENE_VISITOR3 &&
        slackVBlanks > minSlack)
        return 1;

    if (!islandState.lowTide &&
        gFgRuntimeSceneId == FG_SCENE_WALKSTUF1 &&
        frameIndex >= 185 &&
        frameIndex <= 191 &&
        slackVBlanks > minSlack)
        return 1;

    return 0;
}

static int fgSceneUsesBlackBackdrop(const char *sceneName)
{
    return fgSceneEquals(sceneName, "johnny1") ||
           fgSceneEquals(sceneName, "johnny6");
}

static const char *fgSceneBackdropScreen(const char *sceneName)
{
    if (fgSceneEquals(sceneName, "suzy1") ||
        fgSceneEquals(sceneName, "suzy2")) {
        return "SUZBEACH.SCR";
    }

    return NULL;
}

static uint32 fgHeaderCleanSnapshotEstimate(const struct TFgPilotHeader *header)
{
    if (header == NULL)
        return 0;
    return (uint32)header->unionWidth *
           (uint32)header->unionHeight *
           (uint32)sizeof(uint16);
}

static int __attribute__((noinline, optimize("Os")))
fgScenePreservesPrefetchUnderCleanPressure(const char *sceneName)
{
    return fgSceneEquals(sceneName, "building2") ||
           fgSceneEquals(sceneName, "building4") ||
           fgSceneEquals(sceneName, "building6") ||
           fgSceneEquals(sceneName, "walkstuf1") ||
           fgSceneEquals(sceneName, "visitor3") ||
           fgSceneEquals(sceneName, "visitor5") ||
           fgSceneEquals(sceneName, "activity10") ||
           fgSceneEquals(sceneName, "johnny3") ||
           fgSceneEquals(sceneName, "johnny1") ||
           fgSceneEquals(sceneName, "johnny2") ||
           fgSceneEquals(sceneName, "johnny6") ||
           fgSceneEquals(sceneName, "activity9") ||
           fgSceneEquals(sceneName, "mary1") ||
           fgSceneEquals(sceneName, "mary2") ||
           fgSceneEquals(sceneName, "activity11") ||
           fgSceneEquals(sceneName, "activity12") ||
           fgSceneEquals(sceneName, "activity4") ||
           fgSceneEquals(sceneName, "mary3") ||
           fgSceneEquals(sceneName, "fishing4");
}

static int fgSceneKeepsStage1UnderCleanMemoryRelief(const char *sceneName)
{
    return fgSceneEquals(sceneName, "visitor3");
}

static int fgSceneKeepsWindowUnderCleanMemoryRelief(const char *sceneName)
{
    /* VISITOR3-high still benefits from a small clean-relief window. VISITOR3-low
     * is the long-soak cliff: its 48 KB grouped window competes with the low
     * setup tail and five split clean-rect strips, leaving the final small
     * clean strip dependent on CACHE/libc fragmentation luck. */
    return fgSceneEquals(sceneName, "visitor3") && !islandState.lowTide;
}

static uint32 fgSceneCleanReliefWindowBytes(const char *sceneName)
{
    if (fgSceneEquals(sceneName, "visitor3") && !islandState.lowTide)
        return 80UL * 1024UL;
    return 0;
}

/* Round 33 allocator-era note: VISITOR3 still needs cleanMemoryRelief=1
 * because its split clean rects can consume ~360 KB and TRANSIENT is already
 * mostly occupied by bg tiles. Unlike the older broad relief path, keep the
 * tiny stage1 prefetch frame buffer live. High tide also keeps a reduced
 * stream window; low tide drops that window so its final clean-rect strip
 * does not depend on a long-run CACHE/libc fallback hole. */
static int fgSceneForcesCleanMemoryRelief(const char *sceneName)
{
    return fgSceneEquals(sceneName, "visitor3");
}

static int fgSceneNeedsCleanMemoryRelief(const char *sceneName,
                                         uint32 cleanBytes,
                                         uint32 maxFrameBytes)
{
    if (fgScenePreservesPrefetchUnderCleanPressure(sceneName))
        return 0;

    if (cleanBytes >= FG_CLEAN_SNAPSHOT_PRESSURE_BYTES)
        return 1;

    if (cleanBytes >= FG_LARGE_CLEAN_SNAPSHOT_BYTES &&
        maxFrameBytes >= FG_LARGE_FRAME_PAYLOAD_BYTES)
        return 1;

    return 0;
}

static void fgDropPressureCachesForCleanSnapshot(const char *sceneName,
                                                 uint32 cleanBytes)
{
    (void)cleanBytes;
    if (sceneName != NULL)
        fgNextSceneStageInvalidate();
    fgDropOptionalPrefetchBuffersForCleanSnapshot();
}

static void fgTelemetryUpdate(void)
{
}

static void fgInitVisiblePipeline(void)
{
    grUpdateDelay = 0;
}

#if FG_ENABLE_LEGACY_DIAGNOSTIC_SCENES
static void fgInitBlackBackground(void)
{
    grInitEmptyBackground();
    grSaveCleanBgTiles();
}

static void fgBlit16ToBackgroundRect(int dstX, int dstY,
                                     uint16 width, uint16 height,
                                     const uint16 *srcPixels)
{
    if (srcPixels == NULL || width == 0 || height == 0)
        return;
    grCompositeDirect16ToBackground(srcPixels, width, height, (sint16)dstX, (sint16)dstY);
}

static void fgPresentCurrentBackground(uint16 holdFrames)
{
    uint16 i;

    for (i = 0; i < holdFrames; i++) {
        grUpdateDisplay(NULL, NULL, NULL);
    }
}

static uint8 *fgLoadRawFileDirect(const char *cdPath, uint32 *outSize)
{
    CdlFILE fileInfo;
    uint32 totalBytes;
    uint8 *buffer;
    int totalSectors;

    if (cdPath == NULL || outSize == NULL)
        return NULL;

    if (ps1_cdSearchFileQuiesced(&fileInfo, cdPath) == NULL) {
        printf("FG search %s\n", cdPath);
        return NULL;
    }

    totalBytes = (uint32)fileInfo.size;
    totalSectors = (int)((totalBytes + 2047u) / 2048u);
    buffer = (uint8 *)malloc((size_t)totalSectors * 2048u);
    if (buffer == NULL)
        return NULL;

    ps1_streamAsyncReadDrain();
    CdControl(CdlSetloc, (uint8 *)&fileInfo.pos, 0);
    CdRead(totalSectors, (uint32 *)buffer, CdlModeSpeed);
    if (CdReadSync(0, 0) < 0) {
        printf("FG read %s\n", cdPath);
        free(buffer);
        return NULL;
    }

    *outSize = totalBytes;
    cdromResetState();
    return buffer;
}

static void fgShowRawFrame(const char *cdPath, uint16 holdFrames)
{
    uint32 rawSize = 0;
    uint8 *screenBuffer;

    if (cdPath == NULL)
        return;

    screenBuffer = fgLoadRawFileDirect(cdPath, &rawSize);
    if (screenBuffer == NULL)
        return;
    if (rawSize < (uint32)(640 * 480 * 2)) {
        printf("FG raw %s %u\n", cdPath, (unsigned int)rawSize);
        free(screenBuffer);
        return;
    }

    fgInitVisiblePipeline();
    fgInitBlackBackground();
    grBeginFrame();
    grRestoreBgTiles();
    fgBlit16ToBackgroundRect(0, 0, 640, 480, (const uint16 *)screenBuffer);
    free(screenBuffer);
    fgPresentCurrentBackground(holdFrames);
}

static void fgPlayTestCard(void)
{
    static uint16 *colors[4] = { NULL, NULL, NULL, NULL };
    static const uint16 colorValues[4] = { 0x001f, 0x03e0, 0x03ff, 0x7c1f };
    const uint16 rectW = 120;
    const uint16 rectH = 80;
    uint16 i;

    fgInitVisiblePipeline();
    fgInitBlackBackground();

    for (int c = 0; c < 4; c++) {
        if (colors[c] == NULL) {
            colors[c] = (uint16 *)malloc((size_t)rectW * (size_t)rectH * sizeof(uint16));
            if (colors[c] == NULL)
                return;
            for (uint32 j = 0; j < (uint32)rectW * (uint32)rectH; j++)
                colors[c][j] = colorValues[c];
        }
    }

    for (i = 0; i < 120; i++) {
        grBeginFrame();
        grRestoreBgTiles();
        fgBlit16ToBackgroundRect(24, 24, rectW, rectH, colors[0]);
        fgBlit16ToBackgroundRect(176, 24, rectW, rectH, colors[1]);
        fgBlit16ToBackgroundRect(24, 136, rectW, rectH, colors[2]);
        fgBlit16ToBackgroundRect(176, 136, rectW, rectH, colors[3]);
        fgPresentCurrentBackground(1);
    }
}
#endif

/* Scene-local streaming buffers: allocated once per scene and reused for every
 * frame in that scene. They are intentionally released at scene boundaries so
 * the next backdrop load starts with maximum contiguous heap. */
static uint8 *gFgFrameBuffer = NULL;
static uint32 gFgFrameBufferSize = 0;
static uint8 *gFgPrefetchFrameBuffer = NULL;
static uint32 gFgPrefetchFrameBufferSize = 0;
static uint8 *gFgStreamWindowBuffer = NULL;
static uint32 gFgStreamWindowBufferSize = 0;
static uint8 *gFgStreamScratch = NULL;
static uint32 gFgStreamScratchSize = 0;
static uint8 *gFgSetupSegmentBuffer = NULL;
static uint32 gFgSetupSegmentBufferSize = 0;
static MemRegion gFgSetupSegmentBufferRegion = MEM_REGION_TRANSIENT;
static uint8 *gFgSetupSegment2OwnedBuffer = NULL;
static uint32 gFgSetupSegment2OwnedBufferSize = 0;
static MemRegion gFgSetupSegment2OwnedBufferRegion = MEM_REGION_TRANSIENT;
static uint8 *gFgSetupSegment4OwnedBuffer = NULL;
static uint32 gFgSetupSegment4OwnedBufferSize = 0;
static MemRegion gFgSetupSegment4OwnedBufferRegion = MEM_REGION_TRANSIENT;

static int fgNextSceneStageCanKeepStreamWindow(void)
{
    return gFgNextSceneStage.valid &&
           gFgNextSceneStage.usesStreamWindow &&
           !gFgNextSceneStage.active &&
           !gFgNextSceneStage.readInFlight &&
           !gFgNextSceneStage.parkedInSpu &&
           gFgNextSceneStage.loadedBytes > 0 &&
           gFgNextSceneStage.loadedBytes <= gFgNextSceneStage.windowBytes &&
           gFgStreamWindowBuffer != NULL &&
           gFgStreamWindowBufferSize >= gFgNextSceneStage.loadedBytes;
}

static void fgNextSceneStageInvalidateIfBorrowingStreamWindow(void)
{
    if (gFgNextSceneStage.usesStreamWindow ||
        gFgNextSceneStage.readInFlight)
        fgNextSceneStageInvalidate();
}

static void fgQuiesceCdBeforeCacheBufferMutation(void)
{
    if (gFgNextSceneStage.active ||
        gFgNextSceneStage.readInFlight)
        (void)fgNextSceneStageFinishPending();
    (void)ps1_streamAsyncReadDrain();
}

static void fgReleaseStreamBuffers(void)
{
    /* Grow-only buffers — DO NOT free here. The free+malloc cycle is
     * what fragmented the heap and starved scene N+1 around minute 11
     * of an organic session. The size fields stay set, so
     * foregroundPilotRuntimeStart's "capacity sufficient?" check skips
     * the alloc on every subsequent scene unless a bigger pack comes
     * along. Buffers grow over the first few scenes, then stabilize.
     *
     *   gFgFrameBuffer           — grow-only (per-frame payload)
     *   gFgPrefetchFrameBuffer   — grow-only (prefetch payload)
     *   gFgStreamWindowBuffer    — grow-only (prefetch read window)
     *   gFgStreamScratch         — grow-only (alignment scratch)
     *
     * gFgSetupSegmentBuffer is scene-segment-specific and bounded; we
     * still cycle it because it's low-cost and makes the failure path
     * easier to read. */
    if (gFgSetupSegmentBuffer != NULL) {
        /* Region-specific release: VISITOR3/fishing3 use TRANSIENT, while
         * large building2/walkstuf1 setup windows use CACHE to avoid both
         * libc fragmentation and TRANSIENT exhaustion. */
        memFree(gFgSetupSegmentBufferRegion, gFgSetupSegmentBuffer);
        gFgSetupSegmentBuffer = NULL;
        gFgSetupSegmentBufferSize = 0;
        gFgSetupSegmentBufferRegion = MEM_REGION_TRANSIENT;
    }
    if (gFgSetupSegment2OwnedBuffer != NULL) {
        memFree(gFgSetupSegment2OwnedBufferRegion, gFgSetupSegment2OwnedBuffer);
        gFgSetupSegment2OwnedBuffer = NULL;
        gFgSetupSegment2OwnedBufferSize = 0;
        gFgSetupSegment2OwnedBufferRegion = MEM_REGION_TRANSIENT;
    }
    if (gFgSetupSegment4OwnedBuffer != NULL) {
        memFree(gFgSetupSegment4OwnedBufferRegion, gFgSetupSegment4OwnedBuffer);
        gFgSetupSegment4OwnedBuffer = NULL;
        gFgSetupSegment4OwnedBufferSize = 0;
        gFgSetupSegment4OwnedBufferRegion = MEM_REGION_TRANSIENT;
    }
}

static void fgReleaseStreamBuffersHard(void)
{
    int keepStagedWindow;

    if (gFgNextSceneStage.active ||
        gFgNextSceneStage.readInFlight)
        fgNextSceneStageFinishPending();
    fgQuiesceCdBeforeCacheBufferMutation();

    keepStagedWindow = fgNextSceneStageCanKeepStreamWindow();
    if (!keepStagedWindow &&
        (gFgNextSceneStage.active ||
         gFgNextSceneStage.readInFlight ||
         gFgNextSceneStage.usesStreamWindow ||
         (gFgNextSceneStage.valid && !gFgNextSceneStage.parkedInSpu))) {
        fgNextSceneStageInvalidate();
    }

    if (gFgFrameBuffer != NULL) {
        memFree(MEM_REGION_CACHE, gFgFrameBuffer);
        gFgFrameBuffer = NULL;
        gFgFrameBufferSize = 0;
    }
    if (gFgPrefetchFrameBuffer != NULL) {
        memFree(MEM_REGION_CACHE, gFgPrefetchFrameBuffer);
        gFgPrefetchFrameBuffer = NULL;
        gFgPrefetchFrameBufferSize = 0;
    }
    if (gFgStreamWindowBuffer != NULL && !keepStagedWindow) {
        memFree(MEM_REGION_CACHE, gFgStreamWindowBuffer);
        gFgStreamWindowBuffer = NULL;
        gFgStreamWindowBufferSize = 0;
    }
    if (gFgStreamScratch != NULL) {
        memFree(MEM_REGION_CACHE, gFgStreamScratch);
        gFgStreamScratch = NULL;
        gFgStreamScratchSize = 0;
        gFgRuntime.streamScratch = NULL;
        gFgRuntime.streamScratchSize = 0;
    }
    fgReleaseStreamBuffers();
}

static void fgDropOptionalPrefetchBuffersForCleanSnapshot(void)
{
    fgNextSceneStageInvalidate();
    fgQuiesceCdBeforeCacheBufferMutation();
    if (gFgPrefetchFrameBuffer != NULL) {
        memFree(MEM_REGION_CACHE, gFgPrefetchFrameBuffer);
        gFgPrefetchFrameBuffer = NULL;
        gFgPrefetchFrameBufferSize = 0;
    }
    if (gFgStreamWindowBuffer != NULL) {
        memFree(MEM_REGION_CACHE, gFgStreamWindowBuffer);
        gFgStreamWindowBuffer = NULL;
        gFgStreamWindowBufferSize = 0;
    }
    gFgRuntime.frameBuffer = gFgFrameBuffer;
    gFgRuntime.frameBufferSize = gFgFrameBufferSize;
    gFgRuntime.prefetchFrameBuffer = NULL;
    gFgRuntime.prefetchFrameBufferSize = 0;
    gFgRuntime.streamWindowBuffer = NULL;
    gFgRuntime.streamWindowSize = 0;
    gFgRuntime.streamWindowReadSize = 0;
    gFgRuntime.streamWindowStart = 0;
    gFgRuntime.streamWindowBytes = 0;
    gFgRuntime.setupPrimeWindowBytes = 0;
    gFgRuntime.streamScratch = gFgStreamScratch;
    gFgRuntime.streamScratchSize = gFgStreamScratchSize;
    gFgRuntime.setupSegmentBuffer = NULL;
    gFgRuntime.setupSegmentStart = 0;
    gFgRuntime.setupSegmentBytes = 0;
    gFgRuntime.setupSegment2Buffer = NULL;
    gFgRuntime.setupSegment2Start = 0;
    gFgRuntime.setupSegment2Bytes = 0;
    gFgRuntime.setupSegment3Buffer = NULL;
    gFgRuntime.setupSegment3Start = 0;
    gFgRuntime.setupSegment3Bytes = 0;
    gFgRuntime.setupSegment4Buffer = NULL;
    gFgRuntime.setupSegment4Start = 0;
    gFgRuntime.setupSegment4Bytes = 0;
    gFgRuntime.setupSegmentReusable = 0;
    gFgRuntime.streamReadGroups = NULL;
    gFgRuntime.streamReadGroupCount = 0;
    gFgRuntime.streamWindowValid = 0;
    gFgRuntime.setupWindowPrimed = 0;
    gFgRuntime.setupSegmentPrimed = 0;
    gFgRuntime.setupSegment2Primed = 0;
    gFgRuntime.setupSegment3Primed = 0;
    gFgRuntime.setupSegment4Primed = 0;
    gFgRuntime.stagedFrameValid = 0;
    gFgRuntime.preparedFrameValid = 0;

    if (ps1PerfEnabled) {
        ps1PerfSetPrefetchPolicy(PS1_PERF_PREFETCH_NONE, 0);
        ps1PerfMarkBufferSizes(gFgFrameBufferSize, gFgStreamScratchSize);
    }
}

static uint32 fgSetupResidencyCacheBytes(void)
{
    uint32 bytes = 0;

    if (gFgSetupSegmentBuffer != NULL &&
        gFgSetupSegmentBufferRegion == MEM_REGION_CACHE)
        bytes += gFgSetupSegmentBufferSize;
    if (gFgSetupSegment2OwnedBuffer != NULL &&
        gFgSetupSegment2OwnedBufferRegion == MEM_REGION_CACHE)
        bytes += gFgSetupSegment2OwnedBufferSize;
    if (gFgSetupSegment4OwnedBuffer != NULL &&
        gFgSetupSegment4OwnedBufferRegion == MEM_REGION_CACHE)
        bytes += gFgSetupSegment4OwnedBufferSize;

    return bytes;
}

static int fgPointerInByteRange(const void *ptr, const void *base, uint32 bytes)
{
    unsigned long p = (unsigned long)ptr;
    unsigned long b = (unsigned long)base;

    return ptr != NULL && base != NULL && p >= b && p < b + (unsigned long)bytes;
}

static void fgClearRuntimeSetupSegment1(void)
{
    gFgRuntime.setupSegmentBuffer = NULL;
    gFgRuntime.setupSegmentStart = 0;
    gFgRuntime.setupSegmentBytes = 0;
    gFgRuntime.setupSegmentPrimed = 0;
}

static void fgClearRuntimeSetupSegment2(void)
{
    gFgRuntime.setupSegment2Buffer = NULL;
    gFgRuntime.setupSegment2Start = 0;
    gFgRuntime.setupSegment2Bytes = 0;
    gFgRuntime.setupSegment2Primed = 0;
}

static void fgClearRuntimeSetupSegment3(void)
{
    gFgRuntime.setupSegment3Buffer = NULL;
    gFgRuntime.setupSegment3Start = 0;
    gFgRuntime.setupSegment3Bytes = 0;
    gFgRuntime.setupSegment3Primed = 0;
}

static void fgClearRuntimeSetupSegment4(void)
{
    gFgRuntime.setupSegment4Buffer = NULL;
    gFgRuntime.setupSegment4Start = 0;
    gFgRuntime.setupSegment4Bytes = 0;
    gFgRuntime.setupSegment4Primed = 0;
}

static void fgRefreshSetupSegmentReusable(void)
{
    if (!gFgRuntime.setupSegmentPrimed &&
        !gFgRuntime.setupSegment2Primed &&
        !gFgRuntime.setupSegment3Primed &&
        !gFgRuntime.setupSegment4Primed)
        gFgRuntime.setupSegmentReusable = 0;
}

static void fgDropCacheSetupResidency(void)
{
    fgQuiesceCdBeforeCacheBufferMutation();

    if (gFgSetupSegmentBuffer != NULL &&
        gFgSetupSegmentBufferRegion == MEM_REGION_CACHE) {
        uint8 *segmentBuffer = gFgSetupSegmentBuffer;
        uint32 segmentBufferSize = gFgSetupSegmentBufferSize;

        memFree(MEM_REGION_CACHE, gFgSetupSegmentBuffer);
        gFgSetupSegmentBuffer = NULL;
        gFgSetupSegmentBufferSize = 0;
        gFgSetupSegmentBufferRegion = MEM_REGION_TRANSIENT;

        if (fgPointerInByteRange(gFgRuntime.setupSegmentBuffer,
                                 segmentBuffer,
                                 segmentBufferSize))
            fgClearRuntimeSetupSegment1();
        if (fgPointerInByteRange(gFgRuntime.setupSegment2Buffer,
                                 segmentBuffer,
                                 segmentBufferSize))
            fgClearRuntimeSetupSegment2();
        if (fgPointerInByteRange(gFgRuntime.setupSegment3Buffer,
                                 segmentBuffer,
                                 segmentBufferSize))
            fgClearRuntimeSetupSegment3();
        if (fgPointerInByteRange(gFgRuntime.setupSegment4Buffer,
                                 segmentBuffer,
                                 segmentBufferSize))
            fgClearRuntimeSetupSegment4();
    }

    if (gFgSetupSegment2OwnedBuffer != NULL &&
        gFgSetupSegment2OwnedBufferRegion == MEM_REGION_CACHE) {
        if (gFgRuntime.setupSegment2Buffer == gFgSetupSegment2OwnedBuffer)
            fgClearRuntimeSetupSegment2();
        memFree(MEM_REGION_CACHE, gFgSetupSegment2OwnedBuffer);
        gFgSetupSegment2OwnedBuffer = NULL;
        gFgSetupSegment2OwnedBufferSize = 0;
        gFgSetupSegment2OwnedBufferRegion = MEM_REGION_TRANSIENT;
    }

    if (gFgSetupSegment4OwnedBuffer != NULL &&
        gFgSetupSegment4OwnedBufferRegion == MEM_REGION_CACHE) {
        if (gFgRuntime.setupSegment4Buffer == gFgSetupSegment4OwnedBuffer)
            fgClearRuntimeSetupSegment4();
        memFree(MEM_REGION_CACHE, gFgSetupSegment4OwnedBuffer);
        gFgSetupSegment4OwnedBuffer = NULL;
        gFgSetupSegment4OwnedBufferSize = 0;
        gFgSetupSegment4OwnedBufferRegion = MEM_REGION_TRANSIENT;
    }

    fgRefreshSetupSegmentReusable();
}

static void fgDropSetupResidencyForCleanSnapshot(const char *sceneName,
                                                 uint32 cleanBytes)
{
    uint32 transientUsed;
    uint32 transientFree;
    uint32 setupCacheBytes = fgSetupResidencyCacheBytes();

    if (setupCacheBytes == 0)
        return;

    transientUsed = (uint32)memRegionUsed((unsigned int)MEM_REGION_TRANSIENT);
    transientFree = (transientUsed < MEM_TRANSIENT_BUDGET) ?
        (uint32)(MEM_TRANSIENT_BUDGET - transientUsed) : 0;
    if (cleanBytes <= transientFree)
        return;

    if (gFgRuntime.setupSegmentPrimed ||
        gFgRuntime.setupSegment2Primed ||
        gFgRuntime.setupSegment3Primed ||
        gFgRuntime.setupSegment4Primed) {
        printf("JCMEM clean-drop-setup scene=%s clean=%lu transFree=%lu "
               "cacheSetup=%lu seg=%lu seg2=%lu seg3=%lu seg4=%lu\n",
               sceneName ? sceneName : "?",
               (unsigned long)cleanBytes,
               (unsigned long)transientFree,
               (unsigned long)setupCacheBytes,
               (unsigned long)gFgRuntime.setupSegmentBytes,
               (unsigned long)gFgRuntime.setupSegment2Bytes,
               (unsigned long)gFgRuntime.setupSegment3Bytes,
               (unsigned long)gFgRuntime.setupSegment4Bytes);
    }

    fgDropCacheSetupResidency();
}

unsigned long fgGetFrameBufferBytes(void)
{
    return (unsigned long)gFgFrameBufferSize;
}

unsigned long fgGetPrefetchFrameBufferBytes(void)
{
    return (unsigned long)gFgPrefetchFrameBufferSize;
}

/* fgPrePrimeStreamBuffers was a v0.8.10-era pre-allocation helper
 * that the v0.8.11 rollback orphaned (no remaining callers).
 * Plan v9 Phase 2 manifest item #9: delete entirely. The new
 * memory-region allocator makes boot-time pre-priming the
 * stream-buffer maxes redundant (the worst-case grow pattern is
 * captured by the pack-header scan once frame-buffer maxes are
 * added to pack_header_metrics).
 *
 * Declaration in foreground_pilot.h is kept for API stability
 * until a downstream cleanup commit can remove it; the body here
 * is gone. */

unsigned long fgProbeLargestAlloc(void)
{
    unsigned long lo = 0;
    unsigned long hi = 512ul * 1024ul;

    while ((hi - lo) > 1024ul) {
        unsigned long mid = ((lo + hi + 1023ul) / 2048ul) * 1024ul;
        void *p;
        if (mid <= lo)
            mid = lo + 1024ul;
        p = malloc((size_t)mid);
        if (p != NULL) {
            free(p);
            lo = mid;
        } else {
            hi = mid - 1024ul;
        }
    }

    return lo;
}

#if FG_HEAP_PROBE_LOGS
static void fgHeapProbe(const char *phase, const char *sceneName)
{
    unsigned long largest;

    if (!gFgHeapProbeEnabled)
        return;

    largest = fgProbeLargestAlloc();
    printf("FH p=%s s=%s l=%lu f=%lu p=%lu w=%lu s=%lu r=%d b=%lu\n",
           phase != NULL ? phase : "?",
           sceneName != NULL ? sceneName : "?",
           largest,
           (unsigned long)gFgFrameBufferSize,
           (unsigned long)gFgPrefetchFrameBufferSize,
           (unsigned long)gFgStreamWindowBufferSize,
           (unsigned long)gFgStreamScratchSize,
           grCleanBgRectsCount(),
           grCleanBgRectsBytes());
}
#else
#define fgHeapProbe(phase, sceneName) ((void)0)
#endif

static void fgRuntimeReset(void)
{
    fgQuiesceCdBeforeCacheBufferMutation();

    /* Wipe the TRANSIENT region — all per-scene allocations made via
     * memAlloc(MEM_REGION_TRANSIENT, ...) become unreachable. The bump
     * pointer rewinds in one word write. Cheap in release; ~15 ms in
     * MEM_POISON_TRANSIENT debug builds (250 KB 0xCD fill).
     * See docs/ps1/memory-region-allocator-plan.md "Boot lifecycle". */
    extern const char *fgLoopGetLastScene(void);
    memSceneReset(fgLoopGetLastScene());

    /* Round 33: bg-tile pixels + struct live in TRANSIENT (graphics_ps1.c
     * createEmptyBgTileRAM / ensureBgTileRAM). memSceneReset just
     * reclaimed those bytes, so the static bgTile0/1/3/4 slots (and
     * grBackgroundSfc) now point at dangling addresses. NULL them
     * before any subsequent code can dereference them. The next
     * grLoadScreen / grInitEmptyBackground call sees the cleared
     * slots and re-allocates fresh tiles in the new TRANSIENT frame. */
    grBackgroundTilesAssumeWiped();

    /* Note: the previous attempt to call fgReleaseStreamBuffersHard +
     * lruEvictAllUnpinned + memCacheRewindIfEmpty here fired on every
     * fgRuntimeReset invocation — including the many error-recovery
     * call sites inside foregroundPilotRuntimeStart. Those calls run
     * mid-scene-setup with resources actively loading; premature
     * release+evict freed buffers the in-progress setup was about
     * to use. Moved to fgPlayOceanRuntimeScene::sceneStartCacheReset
     * which fires exactly once per scene boundary. */
    fgReleaseStreamBuffersHard();

    /* Clear file-static TRANSIENT pointers so the next scene sees a
     * clean slate. memSceneReset reclaimed the underlying bytes, but
     * these globals still hold the (now-dangling) pointers from the
     * previous scene's allocation. Setting them to NULL ensures the
     * "is this allocated yet?" checks in setup paths take the alloc
     * branch on the new scene. */
    gFgSetupSegmentBuffer     = NULL;
    gFgSetupSegmentBufferSize = 0;
    gFgSetupSegmentBufferRegion = MEM_REGION_TRANSIENT;
    gFgSetupSegment2OwnedBuffer = NULL;
    gFgSetupSegment2OwnedBufferSize = 0;
    gFgSetupSegment2OwnedBufferRegion = MEM_REGION_TRANSIENT;
    gFgSetupSegment4OwnedBuffer = NULL;
    gFgSetupSegment4OwnedBufferSize = 0;
    gFgSetupSegment4OwnedBufferRegion = MEM_REGION_TRANSIENT;

    /* currentFrameData now points inside gFgFrameBuffer — don't free it
     * separately. The persistent buffers (gFgFrameBuffer / gFgStreamScratch)
     * survive the reset on purpose; only per-scene state is cleared. */
    gFgRuntime.currentFrameData = NULL;
    gFgRuntime.frameBuffer = NULL;
    gFgRuntime.frameBufferSize = 0;
    gFgRuntime.prefetchFrameBuffer = NULL;
    gFgRuntime.prefetchFrameBufferSize = 0;
    gFgRuntime.streamWindowBuffer = NULL;
    gFgRuntime.streamWindowSize = 0;
    gFgRuntime.streamWindowReadSize = 0;
    gFgRuntime.streamWindowStart = 0;
    gFgRuntime.streamWindowBytes = 0;
    gFgRuntime.setupPrimeWindowBytes = 0;
    gFgRuntime.setupSegmentBuffer = NULL;
    gFgRuntime.setupSegmentStart = 0;
    gFgRuntime.setupSegmentBytes = 0;
    gFgRuntime.setupSegment2Buffer = NULL;
    gFgRuntime.setupSegment2Start = 0;
    gFgRuntime.setupSegment2Bytes = 0;
    gFgRuntime.setupSegment3Buffer = NULL;
    gFgRuntime.setupSegment3Start = 0;
    gFgRuntime.setupSegment3Bytes = 0;
    gFgRuntime.setupSegment4Buffer = NULL;
    gFgRuntime.setupSegment4Start = 0;
    gFgRuntime.setupSegment4Bytes = 0;
    gFgRuntime.setupSegmentReusable = 0;
    gFgRuntime.streamReadGroups = NULL;
    gFgRuntime.streamReadGroupCount = 0;
    gFgRuntime.streamWindowValid = 0;
    gFgRuntime.setupSegment3Primed = 0;
    gFgRuntime.setupSegment4Primed = 0;
    gFgRuntime.streamScratch = NULL;
    gFgRuntime.streamScratchSize = 0;
    gFgRuntime.packCdFileValid = 0;
    fgFreeEntryTable(&gFgRuntime.entryTable);
    if (gFgRuntime.soundEvents != NULL) {
        /* TRANSIENT region — memFree decrements the balance counter; the
         * actual bytes get reclaimed by the memSceneReset call above. The
         * dangling pointer in gFgRuntime.soundEvents is cleared just below
         * by the memset(0). */
        memFree(MEM_REGION_TRANSIENT, gFgRuntime.soundEvents);
        gFgRuntime.soundEvents = NULL;
    }
    memset(&gFgRuntime, 0, sizeof(gFgRuntime));
    gFgRuntimeSceneId = FG_SCENE_UNKNOWN;
    fgTelemetryUpdate();
}

static int fgLoadSoundEvents(const CdlFILE *cdfile, const struct TFgPilotHeader *header,
                             struct TFgPilotSoundEvent **outEvents, uint16 *outCount)
{
    uint8 *data;
    uint32 byteCount;
    uint16 i;

    if (!outEvents || !outCount)
        return 0;

    *outEvents = NULL;
    *outCount = 0;

    if (!cdfile || !header)
        return 1;
    if (header->soundEventCount == 0 || header->soundEventsOffset == 0)
        return 1;

    byteCount = (uint32)header->soundEventCount * 4u;
    data = (uint8 *)memAlloc(MEM_REGION_CACHE, byteCount,
                             "fg-sound-events-raw");
    if (!data)
        return 0;
    if (!ps1_streamReadIntoFile(cdfile, header->soundEventsOffset,
                                byteCount, data)) {
        memFree(MEM_REGION_CACHE, data);
        return 0;
    }

    /* MEM_REGION_RATIONALE: per-scene sound-event table. Freed wholesale
     * by memSceneReset at the next fgRuntimeReset. INIT_FULL_WRITE — the
     * loop below populates every entry. */
    *outEvents = (struct TFgPilotSoundEvent *)memAlloc(
        MEM_REGION_TRANSIENT,
        (size_t)header->soundEventCount * sizeof(struct TFgPilotSoundEvent),
        "fgPilotSoundEvents");
    /* memAlloc never returns NULL — halts on exhaustion. No NULL check needed. */

    for (i = 0; i < header->soundEventCount; i++) {
        (*outEvents)[i].sourceFrame = fgReadU16(data + ((uint32)i * 4u));
        (*outEvents)[i].sampleId    = fgReadU16(data + ((uint32)i * 4u) + 2u);
    }
    *outCount = header->soundEventCount;
    memFree(MEM_REGION_CACHE, data);
    return 1;
}

/* Sound events fire at frame-load time, but the loaded frame is not
 * composited and flipped to the display until the next vblank or two. To
 * keep the sample's key-on aligned with the frame the user actually sees,
 * delay event firing by this many source-frames. Tuned by ear on
 * fishing1 — audio landed ~2 frames ahead of the visible trigger. */
#define FG_SOUND_EVENT_DELAY_FRAMES 3

static void fgFireSoundEventsUpTo(uint16 sourceFrame)
{
    uint16 threshold;

    if (sourceFrame < FG_SOUND_EVENT_DELAY_FRAMES)
        return;
    threshold = (uint16)(sourceFrame - FG_SOUND_EVENT_DELAY_FRAMES);

    while (gFgRuntime.soundEventCursor < gFgRuntime.soundEventCount) {
        const struct TFgPilotSoundEvent *ev =
            &gFgRuntime.soundEvents[gFgRuntime.soundEventCursor];
        if (ev->sourceFrame > threshold)
            break;
        soundPlay((int)ev->sampleId);
        if (ps1PerfEnabled)
            ps1PerfMarkSoundEvent();
        gFgRuntime.soundEventCursor++;
    }
}
