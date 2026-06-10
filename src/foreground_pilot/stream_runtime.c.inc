static int fgRuntimeCanStageNextFrame(void)
{
    return gFgPrefetchStage1Enabled &&
           gFgRuntime.active &&
           gFgRuntime.mode == FG_RUNTIME_SCENE_PACK &&
           gFgRuntime.packCdFileValid &&
           gFgRuntime.streamScratch != NULL &&
           gFgRuntime.prefetchFrameBuffer != NULL;
}

static void fgRuntimeSyncFrameBufferGlobals(void)
{
    gFgFrameBuffer = gFgRuntime.frameBuffer;
    gFgFrameBufferSize = gFgRuntime.frameBufferSize;
    gFgPrefetchFrameBuffer = gFgRuntime.prefetchFrameBuffer;
    gFgPrefetchFrameBufferSize = gFgRuntime.prefetchFrameBufferSize;
}

static inline void fgRuntimeClearVolatileSetupSegment(void)
{
    if (!gFgRuntime.setupSegmentReusable) {
        gFgRuntime.setupSegmentPrimed = 0;
        gFgRuntime.setupSegment2Primed = 0;
        gFgRuntime.setupSegment3Primed = 0;
        gFgRuntime.setupSegment4Primed = 0;
    }
}

static int fgRuntimeFindSetupSegmentForEntry(const struct TFgPilotEntry *entry,
                                             uint8 **outBuffer,
                                             uint32 *outStart)
{
    uint32 entryEnd = entry->dataOffset + entry->dataSize;

    if (gFgRuntime.setupSegmentPrimed &&
        entry->dataOffset >= gFgRuntime.setupSegmentStart &&
        entryEnd <= gFgRuntime.setupSegmentStart + gFgRuntime.setupSegmentBytes) {
        if (outBuffer != NULL)
            *outBuffer = gFgRuntime.setupSegmentBuffer;
        if (outStart != NULL)
            *outStart = gFgRuntime.setupSegmentStart;
        return 1;
    }

    if (gFgRuntime.setupSegment2Primed &&
        entry->dataOffset >= gFgRuntime.setupSegment2Start &&
        entryEnd <= gFgRuntime.setupSegment2Start + gFgRuntime.setupSegment2Bytes) {
        if (outBuffer != NULL)
            *outBuffer = gFgRuntime.setupSegment2Buffer;
        if (outStart != NULL)
            *outStart = gFgRuntime.setupSegment2Start;
        return 1;
    }

    if (gFgRuntime.setupSegment3Primed &&
        entry->dataOffset >= gFgRuntime.setupSegment3Start &&
        entryEnd <= gFgRuntime.setupSegment3Start + gFgRuntime.setupSegment3Bytes) {
        if (outBuffer != NULL)
            *outBuffer = gFgRuntime.setupSegment3Buffer;
        if (outStart != NULL)
            *outStart = gFgRuntime.setupSegment3Start;
        return 1;
    }

    if (gFgRuntime.setupSegment4Primed &&
        entry->dataOffset >= gFgRuntime.setupSegment4Start &&
        entryEnd <= gFgRuntime.setupSegment4Start + gFgRuntime.setupSegment4Bytes) {
        if (outBuffer != NULL)
            *outBuffer = gFgRuntime.setupSegment4Buffer;
        if (outStart != NULL)
            *outStart = gFgRuntime.setupSegment4Start;
        return 1;
    }

    return 0;
}

static uint32 fgRuntimeGroupedAppendTargetEnd(uint32 appendStart,
                                              uint32 windowStart,
                                              uint32 targetEnd,
                                              uint16 slackVBlanks)
{
    uint16 startSector;
    uint8 i;

    if (gFgRuntime.streamReadGroupCount == 0 ||
        (appendStart & 2047UL) != 0)
        return targetEnd;

    startSector = (uint16)(appendStart >> 11);
    for (i = 0; i < gFgRuntime.streamReadGroupCount; i++) {
        if (gFgRuntime.streamReadGroups[i].startSector == startSector) {
            uint32 candidateEnd = ((uint32)gFgRuntime.streamReadGroups[i].endSector) << 11;
            if (gFgRuntime.streamReadGroups[i].minSlackVBlanks > 0 &&
                slackVBlanks < gFgRuntime.streamReadGroups[i].minSlackVBlanks)
                return targetEnd;
            if (candidateEnd > targetEnd &&
                candidateEnd - windowStart <= gFgRuntime.streamWindowSize)
                return candidateEnd;
            return targetEnd;
        }
    }

    return targetEnd;
}

static int fgRuntimeWalkstuf1LowFreshOwner(uint32 *ioWindowStart,
                                           uint32 *ioReadEnd,
                                           uint16 slackVBlanks,
                                           uint16 ownerFrameIndex)
{
    uint16 entryStartSector;
    uint32 candidateStart;
    uint32 candidateEnd;

    if (ioWindowStart == NULL ||
        ioReadEnd == NULL ||
        (*ioWindowStart & 2047UL) != 0)
        return 0;

    /* The W1-low 160..176 cluster has a long validation gap; broader
     * neighboring clusters saved reads but stole visible/refill cadence. */
    if (!islandState.lowTide ||
        gFgRuntimeSceneId != FG_SCENE_WALKSTUF1 ||
        ownerFrameIndex < 101 ||
        ownerFrameIndex > 111)
        return 0;
    if (slackVBlanks == 0)
        return 0;

    entryStartSector = (uint16)(*ioWindowStart >> 11);
    if (entryStartSector < 160 || entryStartSector >= 176)
        return 0;

    candidateStart = 160UL * FG_CD_SECTOR_SIZE;
    candidateEnd = 176UL * FG_CD_SECTOR_SIZE;
    if (candidateEnd > *ioReadEnd &&
        candidateEnd - candidateStart <= gFgRuntime.streamWindowSize) {
        *ioWindowStart = candidateStart;
        *ioReadEnd = candidateEnd;
        return 1;
    }

    return 0;
}

static int fgRuntimeWalkstuf1HighFreshOwner(uint32 *ioWindowStart,
                                            uint32 *ioReadEnd,
                                            uint16 slackVBlanks,
                                            uint16 ownerFrameIndex)
{
    uint16 entryStartSector;
    uint32 candidateStart;
    uint32 candidateEnd;

    if (ioWindowStart == NULL ||
        ioReadEnd == NULL ||
        (*ioWindowStart & 2047UL) != 0)
        return 0;

    if (islandState.lowTide ||
        gFgRuntimeSceneId != FG_SCENE_WALKSTUF1 ||
        slackVBlanks == 0)
        return 0;

    entryStartSector = (uint16)(*ioWindowStart >> 11);
    if (ownerFrameIndex >= 183 &&
        ownerFrameIndex <= 187 &&
        entryStartSector >= 372 &&
        entryStartSector < 384) {
        candidateStart = 372UL * FG_CD_SECTOR_SIZE;
        candidateEnd = 384UL * FG_CD_SECTOR_SIZE;
        if (candidateEnd > *ioReadEnd &&
            candidateEnd - candidateStart <= gFgRuntime.streamWindowSize) {
            *ioWindowStart = candidateStart;
            *ioReadEnd = candidateEnd;
            return 1;
        }
    }

    return 0;
}

static int fgRuntimeEntryFitsWindow(const struct TFgPilotEntry *entry)
{
    uint32 windowStart;
    uint32 offsetInWindow;
    uint32 readBytes;

    if (gFgPrefetchWindowBytes == 0 ||
        !gFgRuntime.active ||
        gFgRuntime.mode != FG_RUNTIME_SCENE_PACK ||
        !gFgRuntime.packCdFileValid ||
        gFgRuntime.streamWindowBuffer == NULL ||
        gFgRuntime.streamWindowSize == 0 ||
        !fgEntryHasPayload(entry))
        return 0;

    windowStart = fgSectorAlignDown(entry->dataOffset);
    offsetInWindow = entry->dataOffset - windowStart;
    readBytes = fgRuntimeWindowReadSize();
    return (offsetInWindow + entry->dataSize <= readBytes) ? 1 : 0;
}

static int fgRuntimeWindowContainsEntry(const struct TFgPilotEntry *entry)
{
    uint32 entryEnd;
    uint32 windowEnd;

    entryEnd = entry->dataOffset + entry->dataSize;

    if (gFgRuntime.streamWindowValid) {
        windowEnd = gFgRuntime.streamWindowStart + gFgRuntime.streamWindowBytes;
        if (entry->dataOffset >= gFgRuntime.streamWindowStart &&
            entryEnd <= windowEnd)
            return 1;
    }

    return fgRuntimeFindSetupSegmentForEntry(entry, NULL, NULL);
}

static uint32 fgRuntimeTrimReadEndBeforeResidentSegment(uint32 windowStart,
                                                        uint32 readEnd,
                                                        uint32 entryEnd)
{
    uint16 i;
    uint32 trimEnd = readEnd;
    uint32 payloadEnd = entryEnd;

    if (gFgRuntimeSceneId != FG_SCENE_VISITOR3 || islandState.lowTide)
        return readEnd;

    if (gFgRuntime.setupSegmentPrimed &&
        gFgRuntime.setupSegmentStart > windowStart &&
        gFgRuntime.setupSegmentStart < trimEnd &&
        entryEnd <= gFgRuntime.setupSegmentStart)
        trimEnd = gFgRuntime.setupSegmentStart;

    if (gFgRuntime.setupSegment2Primed &&
        gFgRuntime.setupSegment2Start > windowStart &&
        gFgRuntime.setupSegment2Start < trimEnd &&
        entryEnd <= gFgRuntime.setupSegment2Start)
        trimEnd = gFgRuntime.setupSegment2Start;

    if (trimEnd != readEnd) {
        for (i = 0; i < gFgRuntime.entryTable.count; i++) {
            const struct TFgPilotEntry *candidate = &gFgRuntime.entryTable.entries[i];
            uint32 candidateEnd;

            if (!fgEntryHasPayload(candidate) ||
                candidate->dataOffset < windowStart)
                continue;

            candidateEnd = candidate->dataOffset + candidate->dataSize;
            if (candidateEnd <= trimEnd && candidateEnd > payloadEnd)
                payloadEnd = candidateEnd;
        }
        payloadEnd = fgSectorAlignUp(payloadEnd);
        if (payloadEnd > windowStart && payloadEnd < trimEnd)
            trimEnd = payloadEnd;
    }

    return trimEnd;
}

static int fgRuntimeCopyEntryFromWindow(const struct TFgPilotEntry *entry,
                                        uint8 *dst,
                                        uint8 countsAsDueHit)
{
    uint32 entryEnd;
    uint32 offsetInWindow;

    entryEnd = entry->dataOffset + entry->dataSize;

    {
        uint8 *segmentBuffer;
        uint32 segmentStart;
        if (fgRuntimeFindSetupSegmentForEntry(entry, &segmentBuffer, &segmentStart)) {
            offsetInWindow = entry->dataOffset - segmentStart;
            memcpy(dst, segmentBuffer + offsetInWindow, entry->dataSize);
            fgRuntimeClearVolatileSetupSegment();
        } else {
            uint32 windowEnd;
            if (!gFgRuntime.streamWindowValid)
                return 0;
            windowEnd = gFgRuntime.streamWindowStart + gFgRuntime.streamWindowBytes;
            if (entry->dataOffset < gFgRuntime.streamWindowStart ||
                entryEnd > windowEnd)
                return 0;
            offsetInWindow = entry->dataOffset - gFgRuntime.streamWindowStart;
            memcpy(dst, gFgRuntime.streamWindowBuffer + offsetInWindow, entry->dataSize);
        }
    }
    if (ps1PerfEnabled)
        ps1PerfMarkPrefetchWindowHit(countsAsDueHit);
    return 1;
}

static int fgRuntimeDecodeFramePayload(uint8 *data,
                                       uint32 dataCapacity,
                                       struct TFgPilotEntry *entry,
                                       const uint8 *baseFrameData,
                                       uint32 baseFrameSize)
{
    if (!fgDecodeFrameDelta(data,
                            dataCapacity,
                            entry,
                            baseFrameData,
                            baseFrameSize))
        return 0;
    if (!fgDecodeLocalLzPayload(data, dataCapacity, entry))
        return 0;
    return 1;
}

static int fgRuntimeDecodeAndSetStagedFrame(uint16 frameIndex,
                                            const struct TFgPilotEntry *entry)
{
    struct TFgPilotEntry stagedEntry;

    if (entry == NULL ||
        gFgRuntime.prefetchFrameBuffer == NULL)
        return 0;

    stagedEntry = *entry;
    if (!fgRuntimeDecodeFramePayload(gFgRuntime.prefetchFrameBuffer,
                                     gFgRuntime.prefetchFrameBufferSize,
                                     &stagedEntry,
                                     gFgRuntime.currentFrameData,
                                     gFgRuntime.currentEntry.dataSize))
        return 0;

    fgRuntimeSetStagedFrame(frameIndex, &stagedEntry);
    return 1;
}

static int fgRuntimeTryExtendWindow(uint32 windowStart,
                                    uint32 readBytes,
                                    uint16 slackVBlanks,
                                    uint8 countAsPrefetch,
                                    uint16 *outElapsedVBlanks)
{
    uint32 currentStart;
    uint32 currentEnd;
    uint32 targetEnd;
    uint32 preserveOffset;
    uint32 preserveBytes;
    uint32 appendBytes;
    uint32 stageTick;
    uint16 elapsedVBlanks;
    int ok;

    if (!gFgRuntime.streamWindowValid)
        return 0;

    currentStart = gFgRuntime.streamWindowStart;
    currentEnd = currentStart + gFgRuntime.streamWindowBytes;
    targetEnd = windowStart + readBytes;
    targetEnd = fgRuntimeGroupedAppendTargetEnd(currentEnd,
                                                windowStart,
                                                targetEnd,
                                                slackVBlanks);
    readBytes = targetEnd - windowStart;

    if (windowStart < currentStart ||
        windowStart > currentEnd ||
        targetEnd <= currentEnd)
        return 0;

    /* Append reads write whole sectors, so only extend fully aligned windows. */
    if ((currentEnd & 2047UL) != 0 || (targetEnd & 2047UL) != 0)
        return 0;

    preserveOffset = windowStart - currentStart;
    preserveBytes = currentEnd - windowStart;
    appendBytes = targetEnd - currentEnd;
    if (preserveBytes + appendBytes > gFgRuntime.streamWindowSize)
        return 0;

    stageTick = fgReadTickCounter();
    if (countAsPrefetch && ps1PerfEnabled)
        ps1PerfBeginPrefetchRead(slackVBlanks);

    if (preserveOffset > 0 && preserveBytes > 0)
        memmove(gFgRuntime.streamWindowBuffer,
                gFgRuntime.streamWindowBuffer + preserveOffset,
                preserveBytes);

    ok = ps1_streamReadAlignedIntoFile(&gFgRuntime.packCdFile,
                                       currentEnd,
                                       appendBytes,
                                       gFgRuntime.streamWindowBuffer + preserveBytes);
    elapsedVBlanks = (uint16)ps1PerfElapsedVBlanks(stageTick);
    if (countAsPrefetch && ps1PerfEnabled)
        ps1PerfEndPrefetchRead(elapsedVBlanks, appendBytes, ok);
    if (outElapsedVBlanks != NULL)
        *outElapsedVBlanks = elapsedVBlanks;

    if (!ok) {
        gFgRuntime.streamWindowValid = 0;
        return -1;
    }

    gFgRuntime.streamWindowStart = windowStart;
    gFgRuntime.streamWindowBytes = readBytes;
    return 1;
}

static int fgRuntimeFillWindowForEntry(const struct TFgPilotEntry *entry,
                                       uint16 ownerFrameIndex,
                                       uint16 slackVBlanks,
                                       uint8 countAsPrefetch,
                                       uint16 *outElapsedVBlanks)
{
    uint32 windowStart;
    uint32 readBytes;
    uint32 readEnd;
    uint32 stageTick;
    uint16 elapsedVBlanks;
    int ok;

    if (!fgRuntimeEntryFitsWindow(entry))
        return 0;

    windowStart = fgSectorAlignDown(entry->dataOffset);
    readBytes = fgRuntimeWindowReadSize();
    if (windowStart >= (uint32)gFgRuntime.packCdFile.size)
        return 0;
    if (countAsPrefetch &&
        !islandState.lowTide &&
        gFgRuntimeSceneId == FG_SCENE_VISITOR3 &&
        slackVBlanks > 0 &&
        slackVBlanks <= FG_VISITOR3_HIGH_TIGHT_WINDOW_SLACK_VBLANKS &&
        readBytes > FG_VISITOR3_HIGH_TIGHT_WINDOW_BYTES) {
        uint32 tightReadEnd = windowStart + FG_VISITOR3_HIGH_TIGHT_WINDOW_BYTES;
        uint32 entryEnd = fgSectorAlignUp(entry->dataOffset + entry->dataSize);
        if (tightReadEnd < entryEnd)
            tightReadEnd = entryEnd;
        if (tightReadEnd - windowStart < readBytes)
            readBytes = tightReadEnd - windowStart;
    }
    readEnd = windowStart + readBytes;
    fgRuntimeWalkstuf1LowFreshOwner(&windowStart,
                                    &readEnd,
                                    slackVBlanks,
                                    ownerFrameIndex);
    fgRuntimeWalkstuf1HighFreshOwner(&windowStart,
                                     &readEnd,
                                     slackVBlanks,
                                     ownerFrameIndex);
    readBytes = readEnd - windowStart;
    if (readEnd > (uint32)gFgRuntime.packCdFile.size)
        readBytes = (uint32)gFgRuntime.packCdFile.size - windowStart;
    readEnd = fgRuntimeTrimReadEndBeforeResidentSegment(windowStart,
                                                        windowStart + readBytes,
                                                        entry->dataOffset + entry->dataSize);
    readBytes = readEnd - windowStart;

    ok = fgRuntimeTryExtendWindow(windowStart,
                                  readBytes,
                                  slackVBlanks,
                                  countAsPrefetch,
                                  outElapsedVBlanks);
    if (ok != 0)
        return (ok > 0) ? 1 : 0;

    stageTick = fgReadTickCounter();
    if (countAsPrefetch && ps1PerfEnabled)
        ps1PerfBeginPrefetchRead(slackVBlanks);
    ok = ps1_streamReadAlignedIntoFile(&gFgRuntime.packCdFile,
                                       windowStart,
                                       readBytes,
                                       gFgRuntime.streamWindowBuffer);
    elapsedVBlanks = (uint16)ps1PerfElapsedVBlanks(stageTick);
    if (countAsPrefetch && ps1PerfEnabled)
        ps1PerfEndPrefetchRead(elapsedVBlanks, readBytes, ok);
    if (outElapsedVBlanks != NULL)
        *outElapsedVBlanks = elapsedVBlanks;

    if (!ok) {
        gFgRuntime.streamWindowValid = 0;
        return 0;
    }

    gFgRuntime.streamWindowStart = windowStart;
    gFgRuntime.streamWindowBytes = readBytes;
    gFgRuntime.streamWindowValid = 1;
    return 1;
}

static const struct TFgPilotEntry *fgRuntimeNextPayloadEntry(uint16 *outFrameIndex)
{
    uint16 frameIndex;

    frameIndex = gFgRuntime.stagedFrameValid
        ? (uint16)(gFgRuntime.stagedFrameIndex + 1)
        : (uint16)(gFgRuntime.frameIndex + 1);

    while (frameIndex < gFgRuntime.header.frameCount) {
        const struct TFgPilotEntry *entry =
            fgGetEntryFromTable(&gFgRuntime.entryTable, frameIndex);
        if (fgEntryHasPayload(entry)) {
            if (outFrameIndex != NULL)
                *outFrameIndex = frameIndex;
            return entry;
        }
        frameIndex++;
    }

    return NULL;
}

static int __attribute__((noinline,optimize("Os")))
fgNextSceneStageCanRun(void)
{
    return gFgLoadingWaveProofEnabled &&
           gFgRuntime.active &&
           gFgRuntime.mode == FG_RUNTIME_SCENE_PACK &&
           gFgRuntime.packCdFileValid &&
           /* The stage borrows the global stream window, so the scene
            * must either share it (streaming scenes, and scenes that
            * adopted a staged window — both wait for payload drain
            * before the borrow) or run without one entirely
            * (setup-primed packs; the global window is free all
            * scene). The old streamWindowReadSize>=SECTOR arm excluded
            * adopted-window scenes with no refills (readSize=0, e.g.
            * johnny5), making their successors structural cold misses. */
           (gFgRuntime.streamWindowBuffer == NULL ||
            gFgRuntime.streamWindowBuffer == gFgStreamWindowBuffer);
}

/* Common stage-start body. The caller is responsible for the context
 * gate: in-scene starts must wait for the current scene's payload
 * reads to drain (the stage borrows the stream window); walk-time
 * starts need no such gate because the CD is idle and the retained
 * window has no other user during a walk. */
static int __attribute__((noinline,optimize("Os")))
fgNextSceneStageStartCommon(const char *sceneName)
{
    const char *path;
    uint32 metadataBytes;

    if (sceneName == NULL ||
        gFgStreamWindowBuffer == NULL ||
        gFgStreamWindowBufferSize < FG_CD_SECTOR_SIZE)
        return 0;

    path = fgCompactOverlayPackPathForScene(sceneName);
    if (path == NULL)
        return 0;

    fgNextSceneStageInvalidate();
    gFgNextSceneStage.active = 1;
    gFgNextSceneStage.usesStreamWindow = 1;
    gFgNextSceneStage.lowTide = (uint8)(islandState.lowTide ? 1 : 0);
    strncpy(gFgNextSceneStage.sceneName, sceneName,
            sizeof(gFgNextSceneStage.sceneName) - 1);
    gFgNextSceneStage.sceneName[sizeof(gFgNextSceneStage.sceneName) - 1] = '\0';
    if (!ps1_streamResolveFile(path, &gFgNextSceneStage.packCdFile)) {
        fgNextSceneStageInvalidate();
        return 0;
    }
    metadataBytes = FG_NEXT_STAGE_METADATA_BYTES;
    if (metadataBytes > gFgNextSceneStage.packCdFile.size)
        metadataBytes = fgSectorAlignDown(gFgNextSceneStage.packCdFile.size);
    if (metadataBytes < FG_CD_SECTOR_SIZE) {
        fgNextSceneStageInvalidate();
        return 0;
    }
    metadataBytes = fgSectorAlignDown(metadataBytes);
    gFgNextSceneStage.phase = FG_NEXT_STAGE_PHASE_HEADER;
    gFgNextSceneStage.metadataBytes = metadataBytes;
    gFgNextSceneStage.spuPayloadOffset = 0;
    gFgNextSceneStage.windowStart = 0;
    gFgNextSceneStage.windowBytes = 0;
    gFgNextSceneStage.loadedBytes = 0;
    gFgRuntime.streamWindowValid = 0;
    gFgRuntime.streamWindowStart = 0;
    gFgRuntime.streamWindowBytes = 0;

    if (!ps1_streamAsyncReadAlignedBegin(&gFgNextSceneStage.packCdFile,
                                         0,
                                         metadataBytes,
                                         gFgStreamWindowBuffer)) {
        fgNextSceneStageInvalidate();
        return 0;
    }

    gFgNextSceneStage.readInFlight = 1;
    gFgNextSceneStage.pendingBytes = metadataBytes;
    FG_STAGE_PRINTF("JCSTAGE H\n");
    return 1;
}

static int __attribute__((noinline,optimize("Os")))
fgNextSceneStageStart(const char *sceneName)
{
    if (!fgNextSceneStageCanRun())
        return 0;

    /* Setup-primed scene (no adopted stream window): the retained
     * global window has no other user; its payload entries are served
     * from setup segments, not CD, so there is no read to wait out. */
    if (gFgRuntime.streamWindowBuffer == NULL)
        return fgNextSceneStageStartCommon(sceneName);

    /* The first side-buffer proof staged early but malloc/free cycled an
     * extra 128KB CACHE block under retained ocean state. That was fast, but
     * it fragmented into a CACHE exhaustion BSOD. Borrow the existing stream
     * window only after current-scene payload reads are finished; the window
     * then survives fgRuntimeReset and can be adopted in place next scene. */
    if (fgRuntimeNextPayloadEntry(NULL) != NULL ||
        gFgRuntime.streamWindowBuffer != gFgStreamWindowBuffer)
        return 0;

    return fgNextSceneStageStartCommon(sceneName);
}

static int __attribute__((noinline,optimize("Os")))
fgNextSceneStageParkMetadataInSpu(void)
{
    if (!gFgSpuStageEnabled ||
        gFgNextSceneStage.metadataParkedInSpu ||
        gFgNextSceneStage.metadataBytes == 0 ||
        gFgStreamWindowBuffer == NULL)
        return 0;

    ps1SpuCacheInit();
    if (!ps1SpuCacheReady() ||
        ps1SpuCacheCapacity() < gFgNextSceneStage.metadataBytes)
        return 0;

    if (!ps1SpuCacheWrite(0, gFgStreamWindowBuffer,
                          gFgNextSceneStage.metadataBytes))
        return 0;

    gFgNextSceneStage.metadataParkedInSpu = 1;
    gFgNextSceneStage.spuPayloadOffset = FG_NEXT_STAGE_PAYLOAD_SPU_OFFSET;
    return 1;
}

static int __attribute__((noinline,optimize("Os")))
fgNextSceneStageParkInSpu(void)
{
    if (!gFgSpuStageEnabled ||
        !gFgNextSceneStage.usesStreamWindow ||
        gFgNextSceneStage.parkedInSpu ||
        gFgNextSceneStage.loadedBytes == 0 ||
        gFgNextSceneStage.loadedBytes > gFgNextSceneStage.windowBytes ||
        gFgStreamWindowBuffer == NULL)
        return 0;

    ps1SpuCacheInit();
    if (!ps1SpuCacheReady() ||
        ps1SpuCacheCapacity() < gFgNextSceneStage.spuPayloadOffset ||
        ps1SpuCacheCapacity() - gFgNextSceneStage.spuPayloadOffset <
            gFgNextSceneStage.loadedBytes)
        return 0;

    if (!ps1SpuCacheWrite(gFgNextSceneStage.spuPayloadOffset,
                          gFgStreamWindowBuffer,
                          gFgNextSceneStage.loadedBytes))
        return 0;

    gFgNextSceneStage.parkedInSpu = 1;
    gFgNextSceneStage.usesStreamWindow = 0;
    gFgRuntime.streamWindowValid = 0;
    return 1;
}

static int __attribute__((noinline,optimize("Os")))
fgNextSceneStagePublishLoadedWindow(void)
{
    if (gFgNextSceneStage.loadedBytes == 0 ||
        gFgNextSceneStage.loadedBytes > gFgNextSceneStage.windowBytes)
        return 0;

    /* Windows that fit the SPU payload slot park there (frees the
     * stream window for the next scene's own use). Bigger windows —
     * the setup-prime-heavy scenes like walkstuf2 whose first reads
     * run to ~360 KB — publish IN PLACE in the retained stream window
     * instead of being thrown away: the window survives the boundary
     * under the proof, fgNextSceneStageCanKeepStreamWindow shields it
     * through the hard release, and AdoptWindow's in-place path
     * ('JCSTAGE A') hands it to the next scene without a copy. */
    if (gFgSpuStageEnabled &&
        gFgNextSceneStage.loadedBytes <= PS1_SPU_CACHE_FG_PAYLOAD_BYTES &&
        !fgNextSceneStageParkInSpu()) {
        fgNextSceneStageInvalidate();
        return 0;
    }

    gFgNextSceneStage.active = 0;
    gFgNextSceneStage.valid = 1;
    gFgNextSceneStage.phase = FG_NEXT_STAGE_PHASE_NONE;
    FG_STAGE_PRINTF("JCSTAGE R %lu/%lu\n",
                    (unsigned long)gFgNextSceneStage.loadedBytes,
                    (unsigned long)gFgNextSceneStage.windowBytes);
    return 1;
}

static int __attribute__((noinline,optimize("Os")))
fgNextSceneStageCompletePayloadRead(void)
{
    if (gFgNextSceneStage.loadedBytes < gFgNextSceneStage.windowBytes)
        return 1;

    if (!fgNextSceneStagePublishLoadedWindow()) {
        fgNextSceneStageInvalidate();
        return 0;
    }

    return 1;
}

static int __attribute__((noinline,optimize("Os")))
fgNextSceneStageCompleteHeaderRead(void)
{
    struct TFgPilotHeader header;
    uint32 fileBytes;
    uint32 windowStart;
    uint32 requestBytes;
    uint32 validBytes;
    uint32 windowBytes;

    if (gFgNextSceneStage.phase != FG_NEXT_STAGE_PHASE_HEADER ||
        gFgStreamWindowBuffer == NULL ||
        gFgStreamWindowBufferSize < gFgNextSceneStage.metadataBytes)
        return 0;

    fgParseHeader(gFgStreamWindowBuffer, &header);
    if ((!fgHeaderIsPal4Spans(&header) &&
         !fgHeaderIsIndexed8Spans(&header) &&
         !fgHeaderIsPal4TemporalResidual(&header) &&
         !fgHeaderIsIndexed8TemporalResidual(&header)) ||
        header.frameCount == 0) {
        fgNextSceneStageInvalidate();
        return 0;
    }

    windowStart = fgSectorAlignDown(header.dataOffset);
    fileBytes = gFgNextSceneStage.packCdFile.size;
    if (windowStart >= fileBytes) {
        fgNextSceneStageInvalidate();
        return 0;
    }

    /* Request up to the full side budget; whether the loaded window
     * later parks in SPU (<= payload slot) or publishes in place in
     * the retained stream window (> slot) is decided at publish time. */
    requestBytes = FG_NEXT_STAGE_SIDE_BYTES;
    if (requestBytes > gFgStreamWindowBufferSize)
        requestBytes = fgSectorAlignDown(gFgStreamWindowBufferSize);
    if (requestBytes < FG_CD_SECTOR_SIZE) {
        fgNextSceneStageInvalidate();
        return 0;
    }

    validBytes = fileBytes - windowStart;
    if (validBytes > requestBytes)
        validBytes = requestBytes;
    windowBytes = fgSectorAlignUp(validBytes);
    if (windowBytes > requestBytes)
        windowBytes = requestBytes;
    if (windowBytes == 0) {
        fgNextSceneStageInvalidate();
        return 0;
    }

    gFgNextSceneStage.phase = FG_NEXT_STAGE_PHASE_PAYLOAD;
    gFgNextSceneStage.windowStart = windowStart;
    gFgNextSceneStage.windowBytes = windowBytes;
    gFgNextSceneStage.loadedBytes = 0;
    gFgNextSceneStage.pendingBytes = 0;
    fgNextSceneStageParkMetadataInSpu();
    FG_STAGE_PRINTF("JCSTAGE S %lu\n", (unsigned long)windowBytes);
    return 1;
}

static int __attribute__((noinline,optimize("Os")))
fgNextSceneStageBeginPayloadRead(void)
{
    uint32 chunkBytes;

    if (gFgNextSceneStage.phase != FG_NEXT_STAGE_PHASE_PAYLOAD ||
        gFgNextSceneStage.readInFlight ||
        gFgNextSceneStage.loadedBytes >= gFgNextSceneStage.windowBytes)
        return fgNextSceneStageCompletePayloadRead();

    chunkBytes = gFgNextSceneStage.windowBytes - gFgNextSceneStage.loadedBytes;
    if (chunkBytes > FG_NEXT_STAGE_CHUNK_BYTES)
        chunkBytes = FG_NEXT_STAGE_CHUNK_BYTES;

    if (!ps1_streamAsyncReadAlignedBegin(
            &gFgNextSceneStage.packCdFile,
            gFgNextSceneStage.windowStart + gFgNextSceneStage.loadedBytes,
            chunkBytes,
            gFgStreamWindowBuffer + gFgNextSceneStage.loadedBytes)) {
        fgNextSceneStageInvalidate();
        return 0;
    }

    gFgNextSceneStage.readInFlight = 1;
    gFgNextSceneStage.pendingBytes = chunkBytes;
    return 1;
}

static int __attribute__((noinline,optimize("Os")))
fgNextSceneStageWorkStillValid(const char *sceneName)
{
    if (!fgNextSceneStageCanRun() ||
        !fgSceneEquals(sceneName, gFgNextSceneStage.sceneName) ||
        gFgNextSceneStage.lowTide != (uint8)(islandState.lowTide ? 1 : 0) ||
        !gFgNextSceneStage.usesStreamWindow ||
        /* Runtime either shares the global window (streaming scene) or
         * never adopted one (setup-primed scene staging into the free
         * global window) — anything else means the window changed
         * owners under the stage. */
        (gFgRuntime.streamWindowBuffer != gFgStreamWindowBuffer &&
         gFgRuntime.streamWindowBuffer != NULL) ||
        gFgStreamWindowBuffer == NULL ||
        gFgStreamWindowBufferSize < FG_CD_SECTOR_SIZE)
        return 0;

    if (gFgNextSceneStage.windowBytes > 0 &&
        gFgStreamWindowBufferSize < gFgNextSceneStage.windowBytes)
        return 0;

    return 1;
}

static int __attribute__((noinline,optimize("Os")))
fgNextSceneStageTryTick(const char *sceneName)
{
    int asyncResult;

    if (gFgNextSceneStage.valid)
        return 0;
    if (!gFgNextSceneStage.active &&
        !fgNextSceneStageStart(sceneName))
        return 0;
    if (!gFgNextSceneStage.active)
        return 0;
    if (!fgNextSceneStageWorkStillValid(sceneName)) {
        fgNextSceneStageInvalidate();
        return 0;
    }

    if (gFgNextSceneStage.readInFlight) {
        asyncResult = ps1_streamAsyncReadPoll();
        if (asyncResult == PS1_CD_ASYNC_PENDING)
            return 1;
        if (asyncResult != PS1_CD_ASYNC_DONE) {
            fgNextSceneStageInvalidate();
            return 0;
        }

        gFgNextSceneStage.readInFlight = 0;
        if (gFgNextSceneStage.phase == FG_NEXT_STAGE_PHASE_HEADER) {
            gFgNextSceneStage.pendingBytes = 0;
            if (!fgNextSceneStageCompleteHeaderRead())
                return 0;
            return fgNextSceneStageBeginPayloadRead();
        }

        if (gFgNextSceneStage.phase == FG_NEXT_STAGE_PHASE_PAYLOAD) {
            gFgNextSceneStage.loadedBytes += gFgNextSceneStage.pendingBytes;
            gFgNextSceneStage.pendingBytes = 0;
            return fgNextSceneStageCompletePayloadRead();
        }

        fgNextSceneStageInvalidate();
        return 0;
    }

    if (gFgNextSceneStage.phase == FG_NEXT_STAGE_PHASE_PAYLOAD)
        return fgNextSceneStageBeginPayloadRead();

    return 1;
}

static int fgRuntimeConsumeStagedFrame(uint16 frameIndex)
{
    uint8 *freeBuffer;
    uint32 freeBufferSize;

    if (!gFgRuntime.stagedFrameValid)
        return 0;

    if (gFgRuntime.stagedFrameIndex != frameIndex) {
        if (ps1PerfEnabled)
            ps1PerfMarkTripwire();
        return -1;
    }

    freeBuffer = gFgRuntime.frameBuffer;
    freeBufferSize = gFgRuntime.frameBufferSize;
    gFgRuntime.frameBuffer = gFgRuntime.prefetchFrameBuffer;
    gFgRuntime.frameBufferSize = gFgRuntime.prefetchFrameBufferSize;
    gFgRuntime.prefetchFrameBuffer = freeBuffer;
    gFgRuntime.prefetchFrameBufferSize = freeBufferSize;
    fgRuntimeSyncFrameBufferGlobals();

    gFgRuntime.currentEntry = gFgRuntime.stagedEntry;
    gFgRuntime.currentFrameData = gFgRuntime.frameBuffer;
    gFgRuntime.frameRendered = 0;
    gFgRuntime.stagedFrameValid = 0;

    gFgRuntime.displayVBlanks = fgEntryHoldVBlanks(&gFgRuntime.header,
                                                   &gFgRuntime.currentEntry,
                                                   gFgRuntime.presentedVBlanks);
    if (ps1PerfEnabled) {
        ps1PerfMarkPrefetchHit();
        ps1PerfMarkEntry(gFgRuntime.currentEntry.dataSize,
                         gFgRuntime.displayVBlanks,
                         0,
                         gFgRuntime.currentEntry.sourceFrame,
                         gFgRuntime.currentEntry.dataOffset);
    }
    fgFireSoundEventsUpTo(gFgRuntime.currentEntry.sourceFrame);
    fgTelemetryUpdate();
    return 1;
}

static int fgRuntimePrimeNextFrameForSetup(void)
{
    uint16 nextFrameIndex;
    const struct TFgPilotEntry *entry;

    if (!fgRuntimeCanStageNextFrame() || gFgRuntime.stagedFrameValid)
        return 0;

    if (gFgRuntime.frameIndex + 1 >= gFgRuntime.header.frameCount)
        return 0;
    nextFrameIndex = (uint16)(gFgRuntime.frameIndex + 1);
    entry = fgGetEntryFromTable(&gFgRuntime.entryTable, nextFrameIndex);
    if (entry == NULL)
        return -1;

    if (!fgEntryHasPayload(entry))
        return 0;

    if (entry->dataSize > gFgRuntime.prefetchFrameBufferSize)
        return -1;

    if (fgRuntimeEntryFitsWindow(entry)) {
        if (!fgRuntimeWindowContainsEntry(entry) &&
            !fgRuntimeFillWindowForEntry(entry, nextFrameIndex, 0, 0, NULL)) {
            return -1;
        }
        if (!fgRuntimeCopyEntryFromWindow(entry, gFgRuntime.prefetchFrameBuffer, 0))
            return -1;
        if (!fgRuntimeDecodeAndSetStagedFrame(nextFrameIndex, entry))
            return -1;
        return 1;
    }

    fgRuntimeClearVolatileSetupSegment();
    if (!ps1_streamReadIntoFileBuffered(&gFgRuntime.packCdFile,
                                        entry->dataOffset,
                                        entry->dataSize,
                                        gFgRuntime.prefetchFrameBuffer,
                                        gFgRuntime.streamScratch,
                                        gFgRuntime.streamScratchSize)) {
        return -1;
    }

    if (!fgRuntimeDecodeAndSetStagedFrame(nextFrameIndex, entry))
        return -1;
    return 1;
}

static int __attribute__((noinline, optimize("Os")))
fgRuntimePresentAndConsumeFirstFrameForW1High(void)
{
    grBeginResidualCleanBgFirstFrame();
    grUpdateDisplay(NULL, NULL, NULL);
    fgRuntimeMarkFrameRendered();
    gFgRuntime.presentedVBlanks = (uint16)(gFgRuntime.presentedVBlanks +
                                           gFgRuntime.displayVBlanks);
    gFgRuntime.frameIndex = 1;
    gFgRuntime.frameVBlank = 0;
    if (!fgRuntimeConsumeStagedFrame(gFgRuntime.frameIndex))
        return 0;
    if (fgRuntimePrimeNextFrameForSetup() < 0)
        return 0;
    fgRuntimeWaitHeldVBlank();
    fgRuntimeWaitHeldVBlank();
    fgRuntimeWaitHeldVBlank();
    gFgRuntime.sceneClockTick = fgReadTickCounter();
    return 1;
}

static int __attribute__((noinline, optimize("Os")))
fgRuntimePresentFirstFrameNoWait(void)
{
    grBeginResidualCleanBgFirstFrame();
    grUpdateDisplay(NULL, NULL, NULL);
    fgRuntimeMarkFrameRendered();
    gFgRuntime.presentedVBlanks = (uint16)(gFgRuntime.presentedVBlanks +
                                           gFgRuntime.displayVBlanks);
    gFgRuntime.frameIndex = 1;
    gFgRuntime.frameVBlank = 0;
    if (!fgRuntimeConsumeStagedFrame(gFgRuntime.frameIndex))
        return 0;
    return fgRuntimePrimeNextFrameForSetup() >= 0;
}

static int __attribute__((noinline, optimize("Os")))
fgRuntimePresentNextStagedAndAdvance(void)
{
    grBeginResidualCleanBgFrame();
    grUpdateDisplay(NULL, NULL, NULL);
    gFgRuntime.presentedVBlanks = (uint16)(gFgRuntime.presentedVBlanks +
                                           gFgRuntime.displayVBlanks);
    gFgRuntime.frameIndex++;
    gFgRuntime.frameVBlank = 0;
    if (!fgRuntimeConsumeStagedFrame(gFgRuntime.frameIndex))
        return 0;
    return fgRuntimePrimeNextFrameForSetup() >= 0;
}

static uint32 fgRuntimePackPayloadEndBytes(void)
{
    uint16 i;
    uint32 payloadEnd = 0;

    for (i = 0; i < gFgRuntime.entryTable.count; i++) {
        const struct TFgPilotEntry *entry = &gFgRuntime.entryTable.entries[i];
        if (entry->dataSize > 0) {
            uint32 entryEnd = entry->dataOffset + entry->dataSize;
            if (entryEnd > payloadEnd)
                payloadEnd = entryEnd;
        }
    }

    if (gFgRuntime.header.soundEventsOffset != 0) {
        uint32 soundEnd = gFgRuntime.header.soundEventsOffset +
            ((uint32)gFgRuntime.header.soundEventCount * 4UL);
        if (soundEnd > payloadEnd)
            payloadEnd = soundEnd;
    }

    return payloadEnd;
}

static uint32 fgRuntimeStreamWindowBytes(const char *sceneName,
                                         uint32 requestedWindowBytes)
{
    uint8 i;

    if (requestedWindowBytes != FG_PREFETCH_DEFAULT_WINDOW_BYTES)
        return requestedWindowBytes;

    for (i = 0;
         i < sizeof(kFgRuntimeWindowPolicies) / sizeof(kFgRuntimeWindowPolicies[0]);
         i++) {
        if (kFgRuntimeWindowPolicies[i].requiredPackFormat != 0 &&
            kFgRuntimeWindowPolicies[i].requiredPackFormat != gFgRuntime.packFormat)
            continue;
        if (fgSceneEquals(sceneName, kFgRuntimeWindowPolicies[i].sceneName))
            return islandState.lowTide ?
                kFgRuntimeWindowPolicies[i].lowWindowBytes :
                kFgRuntimeWindowPolicies[i].highWindowBytes;
    }

    return requestedWindowBytes;
}

static int fgRuntimeSkipsAutoFgp3SetupPrime(const char *sceneName)
{
    return fgSceneEquals(sceneName, "building1") ||
           fgSceneEquals(sceneName, "visitor5");
}

static uint32 fgRuntimeSetupPrimeWindowBytes(const char *sceneName,
                                             uint32 normalWindowBytes)
{
    uint8 i;
    uint32 requested = 0;

    if (fgSceneEquals(sceneName, "walkstuf1")) {
        uint32 maxResidentBytes = islandState.lowTide ?
            FG_WALKSTUF1_LOW_SETUP_PRIME_MAX_RESIDENT_BYTES :
            FG_WALKSTUF1_HIGH_SETUP_PRIME_MAX_RESIDENT_BYTES;
        requested = (normalWindowBytes << 2) + FG_WALKSTUF1_SETUP_PRIME_BASE_BYTES -
            (islandState.lowTide ? 0 : FG_WALKSTUF1_HIGH_SETUP_PRIME_TRIM_BYTES);
        return requested > maxResidentBytes ? maxResidentBytes : requested;
    }

    if (normalWindowBytes != FG_PREFETCH_DEFAULT_WINDOW_BYTES)
        return 0;

    if (fgRuntimeUsesTemporalResidual() &&
        !fgRuntimeSkipsAutoFgp3SetupPrime(sceneName)) {
        uint32 payloadEnd = fgRuntimePackPayloadEndBytes();
        uint32 windowStart = fgSectorAlignDown(gFgRuntime.header.dataOffset);
        if (payloadEnd > windowStart) {
            uint32 windowBytes = fgSectorAlignUp(payloadEnd - windowStart);
            if (windowBytes <= FG_SETUP_PRIME_AUTO_PACK_BYTES) {
                requested = windowBytes;
                return requested > FG_SETUP_PRIME_MAX_RESIDENT_BYTES ?
                    FG_SETUP_PRIME_MAX_RESIDENT_BYTES : requested;
            }
        }
    }

    /* Scene-specific until generated prime budgets exist for all variants. */
    for (i = 0;
         i < sizeof(kFgRuntimeSetupPrimePolicies) / sizeof(kFgRuntimeSetupPrimePolicies[0]);
         i++) {
        if (fgSceneEquals(sceneName, kFgRuntimeSetupPrimePolicies[i].sceneName)) {
            uint32 maxResidentBytes = fgSceneEquals(sceneName, "visitor3") ?
                FG_VISITOR3_SETUP_PRIME_MAX_RESIDENT_BYTES :
                FG_SETUP_PRIME_MAX_RESIDENT_BYTES;
            requested = islandState.lowTide ?
                kFgRuntimeSetupPrimePolicies[i].lowWindowBytes :
                kFgRuntimeSetupPrimePolicies[i].highWindowBytes;
            return requested > maxResidentBytes ?
                maxResidentBytes : requested;
        }
    }
    return 0;
}

static int fgRuntimePrimeSetupWindow(void)
{
    uint32 windowStart;
    uint32 windowBytes;
    uint32 fileBytes;
    uint32 validBytes;

    if (!gFgRuntime.packCdFileValid ||
        gFgRuntime.setupPrimeWindowBytes == 0 ||
        gFgRuntime.streamWindowBuffer == NULL ||
        gFgRuntime.streamWindowSize < gFgRuntime.setupPrimeWindowBytes)
        return 1;

    windowStart = (gFgRuntime.header.dataOffset / FG_CD_SECTOR_SIZE) * FG_CD_SECTOR_SIZE;
    fileBytes = gFgRuntime.packCdFile.size;
    if (windowStart >= fileBytes)
        return 1;

    validBytes = fileBytes - windowStart;
    if (validBytes > gFgRuntime.setupPrimeWindowBytes)
        validBytes = gFgRuntime.setupPrimeWindowBytes;
    windowBytes = ((validBytes + FG_CD_SECTOR_SIZE - 1u) / FG_CD_SECTOR_SIZE) *
        FG_CD_SECTOR_SIZE;
    if (windowBytes > gFgRuntime.streamWindowSize)
        return 1;

    if (gFgRuntime.streamWindowValid &&
        gFgRuntime.streamWindowStart == windowStart &&
        gFgRuntime.streamWindowBytes >= windowBytes) {
        gFgRuntime.setupWindowPrimed = 1;
        return 1;
    }

    if (gFgRuntime.streamWindowValid &&
        gFgRuntime.streamWindowStart == windowStart &&
        gFgRuntime.streamWindowBytes > 0 &&
        gFgRuntime.streamWindowBytes < windowBytes) {
        uint32 residentBytes = gFgRuntime.streamWindowBytes;
        uint32 missingBytes = windowBytes - residentBytes;
        if (!ps1_streamReadAlignedIntoFile(&gFgRuntime.packCdFile,
                                           windowStart + residentBytes,
                                           missingBytes,
                                           gFgRuntime.streamWindowBuffer + residentBytes)) {
            gFgRuntime.streamWindowValid = 0;
            return 0;
        }
        gFgRuntime.streamWindowBytes = windowBytes;
        gFgRuntime.setupWindowPrimed = 1;
        return 1;
    }

    if (!ps1_streamReadAlignedIntoFile(&gFgRuntime.packCdFile,
                                       windowStart,
                                       windowBytes,
                                       gFgRuntime.streamWindowBuffer)) {
        gFgRuntime.streamWindowValid = 0;
        return 0;
    }

    gFgRuntime.streamWindowStart = windowStart;
    gFgRuntime.streamWindowBytes = windowBytes;
    gFgRuntime.streamWindowValid = 1;
    gFgRuntime.setupWindowPrimed = 1;
    return 1;
}

static void __attribute__((noinline,optimize("Os")))
fgNextSceneStageInvalidate(void)
{
    if (gFgNextSceneStage.readInFlight) {
        while (ps1_streamAsyncReadPoll() == PS1_CD_ASYNC_PENDING)
            VSync(0);
    }
    (void)ps1_streamAsyncReadDrain();
    gFgNextSceneStage.active = 0;
    gFgNextSceneStage.valid = 0;
    gFgNextSceneStage.sceneName[0] = '\0';
    gFgNextSceneStage.windowStart = 0;
    gFgNextSceneStage.windowBytes = 0;
    gFgNextSceneStage.loadedBytes = 0;
    gFgNextSceneStage.metadataBytes = 0;
    gFgNextSceneStage.spuPayloadOffset = 0;
    gFgNextSceneStage.usesStreamWindow = 0;
    gFgNextSceneStage.parkedInSpu = 0;
    gFgNextSceneStage.metadataParkedInSpu = 0;
    gFgNextSceneStage.readInFlight = 0;
    gFgNextSceneStage.phase = FG_NEXT_STAGE_PHASE_NONE;
    gFgNextSceneStage.pendingBytes = 0;
}

static int __attribute__((noinline,optimize("Os")))
fgNextSceneStageFinishPending(void)
{
    int asyncResult;

    if (!gFgNextSceneStage.active ||
        gFgNextSceneStage.valid)
        return 1;

    if (gFgNextSceneStage.readInFlight) {
        do {
            asyncResult = ps1_streamAsyncReadPoll();
            if (asyncResult == PS1_CD_ASYNC_PENDING)
                VSync(0);
        } while (asyncResult == PS1_CD_ASYNC_PENDING);
        if (asyncResult != PS1_CD_ASYNC_DONE) {
            fgNextSceneStageInvalidate();
            return 0;
        }

        gFgNextSceneStage.readInFlight = 0;
        if (gFgNextSceneStage.phase == FG_NEXT_STAGE_PHASE_HEADER) {
            gFgNextSceneStage.pendingBytes = 0;
            if (!fgNextSceneStageCompleteHeaderRead()) {
                fgNextSceneStageInvalidate();
                return 0;
            }
        } else if (gFgNextSceneStage.phase == FG_NEXT_STAGE_PHASE_PAYLOAD) {
            gFgNextSceneStage.loadedBytes += gFgNextSceneStage.pendingBytes;
            gFgNextSceneStage.pendingBytes = 0;
        } else {
            fgNextSceneStageInvalidate();
            return 0;
        }
    }

    if (gFgNextSceneStage.loadedBytes >= gFgNextSceneStage.windowBytes &&
        gFgNextSceneStage.windowBytes > 0)
        return fgNextSceneStageCompletePayloadRead();

    if (gFgSpuStageEnabled && gFgNextSceneStage.loadedBytes > 0)
        return fgNextSceneStagePublishLoadedWindow();

    /* Header done but zero payload bytes loaded. This is the common case
     * for streaming scenes: staging can only start once the current
     * scene's payload reads drain (the last held frames), so the payload
     * window rarely gets going before scene end. The metadata was parked
     * in SPU by fgNextSceneStageCompleteHeaderRead a moment ago —
     * publish a metadata-only stage instead of discarding it, so the
     * next scene still skips CdSearchFile and the cold metadata read.
     * AdoptWindow correctly reports no window (usesStreamWindow=0) and
     * invalidates the stage after the metadata has been consumed. */
    if (gFgNextSceneStage.metadataParkedInSpu) {
        gFgNextSceneStage.active = 0;
        gFgNextSceneStage.valid = 1;
        gFgNextSceneStage.phase = FG_NEXT_STAGE_PHASE_NONE;
        gFgNextSceneStage.usesStreamWindow = 0;
        /* windowStart/windowBytes survive (loadedBytes stays 0): the
         * inter-scene walk tick can resume the payload read into the
         * retained stream window (fgNextSceneStageWalkTick). AdoptWindow
         * ignores them for a metadata-only stage (usesStreamWindow=0). */
        gFgNextSceneStage.loadedBytes = 0;
        FG_STAGE_PRINTF("JCSTAGE M %lu\n",
                        (unsigned long)gFgNextSceneStage.metadataBytes);
        return 1;
    }

    fgNextSceneStageInvalidate();
    return 1;
}

/* Inter-scene walk tick: the walk between scene N and N+1 runs for
 * hundreds of VBlanks with the CD idle, while the published
 * metadata-only stage still knows the payload window it never got to
 * read (streaming scenes drain their own payload reads right up to
 * scene end, so the in-scene stage rarely progresses past the header).
 * Resume the payload read into the retained stream window here, one
 * async chunk per walk frame poll. Every ending is already handled:
 * full window -> SPU park + publish; walk ends mid-chunk ->
 * fgNextSceneStageFinishPending's bounded wait at the next scene's
 * fgRuntimeReset publishes the partial window. Proof-mode only: the
 * resume depends on the stream window surviving the boundary, which
 * only the loading-waves shape guarantees. */
static int __attribute__((noinline,optimize("Os")))
fgNextSceneStageWalkTick(void)
{
    int asyncResult;

    if (!gFgLoadingWaveProofEnabled || !gFgSpuStageEnabled)
        return 0;

    if (gFgNextSceneStage.readInFlight) {
        asyncResult = ps1_streamAsyncReadPoll();
        if (asyncResult == PS1_CD_ASYNC_PENDING)
            return 1;
        gFgNextSceneStage.readInFlight = 0;
        if (asyncResult != PS1_CD_ASYNC_DONE) {
            fgNextSceneStageInvalidate();
            return 0;
        }
        if (gFgNextSceneStage.phase != FG_NEXT_STAGE_PHASE_PAYLOAD) {
            fgNextSceneStageInvalidate();
            return 0;
        }
        gFgNextSceneStage.loadedBytes += gFgNextSceneStage.pendingBytes;
        gFgNextSceneStage.pendingBytes = 0;
        if (gFgNextSceneStage.loadedBytes >= gFgNextSceneStage.windowBytes)
            return fgNextSceneStageCompletePayloadRead();
        return fgNextSceneStageBeginPayloadRead();
    }

    if (gFgNextSceneStage.active &&
        gFgNextSceneStage.phase == FG_NEXT_STAGE_PHASE_PAYLOAD &&
        gFgNextSceneStage.loadedBytes < gFgNextSceneStage.windowBytes)
        return fgNextSceneStageBeginPayloadRead();

    /* Resume a published metadata-only stage whose payload window was
     * computed but never read. */
    if (gFgNextSceneStage.valid &&
        gFgNextSceneStage.metadataParkedInSpu &&
        !gFgNextSceneStage.parkedInSpu &&
        !gFgNextSceneStage.usesStreamWindow &&
        gFgNextSceneStage.windowBytes > 0 &&
        gFgNextSceneStage.loadedBytes < gFgNextSceneStage.windowBytes &&
        gFgStreamWindowBuffer != NULL &&
        gFgStreamWindowBufferSize >= gFgNextSceneStage.windowBytes) {
        gFgNextSceneStage.valid = 0;
        gFgNextSceneStage.active = 1;
        gFgNextSceneStage.usesStreamWindow = 1;
        gFgNextSceneStage.phase = FG_NEXT_STAGE_PHASE_PAYLOAD;
        FG_STAGE_PRINTF("JCSTAGE W %lu\n",
                        (unsigned long)gFgNextSceneStage.windowBytes);
        return fgNextSceneStageBeginPayloadRead();
    }

    /* Nothing staged at all: start from scratch for the scene the walk
     * is leading into (gFgStageSceneName was primed before the previous
     * scene played and still names the upcoming one). This covers
     * scenes whose predecessor never ran a stream window (tiny
     * setup-primed packs like johnny5), where the in-scene stage gate
     * (fgNextSceneStageCanRun) is false for the whole scene — their
     * successors were structural cold misses. The walk has no
     * competing CD work, so the in-scene payload-drain gate does not
     * apply. */
    if (!gFgNextSceneStage.active &&
        !gFgNextSceneStage.valid &&
        gFgStageSceneName[0] != '\0')
        return fgNextSceneStageStartCommon(gFgStageSceneName);

    return 0;
}

static int __attribute__((noinline,optimize("Os")))
fgNextSceneStageMatches(const char *sceneName)
{
    return sceneName != NULL &&
           fgSceneEquals(sceneName, gFgNextSceneStage.sceneName) &&
           gFgNextSceneStage.lowTide == (uint8)(islandState.lowTide ? 1 : 0);
}

static int __attribute__((noinline,optimize("Os")))
fgNextSceneStageAdoptFile(const char *sceneName, CdlFILE *outFile)
{
    if (outFile == NULL ||
        !fgNextSceneStageMatches(sceneName))
        return 0;

    *outFile = gFgNextSceneStage.packCdFile;
    return 1;
}

static int __attribute__((noinline,optimize("Os")))
fgNextSceneStageReadMetadataPrefix(const char *sceneName,
                                   uint8 *dst,
                                   uint32 bytes)
{
    if (dst == NULL ||
        bytes == 0 ||
        !fgNextSceneStageMatches(sceneName) ||
        !gFgNextSceneStage.metadataParkedInSpu ||
        bytes > gFgNextSceneStage.metadataBytes ||
        ps1SpuCacheCapacity() < bytes)
        return 0;

    return ps1SpuCacheRead(0, dst, bytes);
}

static int __attribute__((noinline,optimize("Os")))
fgNextSceneStageAdoptWindow(const char *sceneName)
{
    if (!gFgNextSceneStage.valid &&
        (!gFgNextSceneStage.active || gFgNextSceneStage.loadedBytes == 0))
        return 0;
    if (gFgNextSceneStage.parkedInSpu) {
        if (!fgNextSceneStageMatches(sceneName) ||
            gFgNextSceneStage.readInFlight ||
            gFgRuntime.streamWindowBuffer != gFgStreamWindowBuffer ||
            gFgRuntime.streamWindowSize < gFgNextSceneStage.loadedBytes ||
            gFgStreamWindowBuffer == NULL ||
            gFgStreamWindowBufferSize < gFgNextSceneStage.loadedBytes ||
            ps1SpuCacheCapacity() < gFgNextSceneStage.spuPayloadOffset ||
            ps1SpuCacheCapacity() - gFgNextSceneStage.spuPayloadOffset <
                gFgNextSceneStage.loadedBytes ||
            !ps1SpuCacheRead(gFgNextSceneStage.spuPayloadOffset,
                             gFgStreamWindowBuffer,
                             gFgNextSceneStage.loadedBytes)) {
            fgNextSceneStageInvalidate();
            return 0;
        }

        gFgRuntime.streamWindowStart = gFgNextSceneStage.windowStart;
        gFgRuntime.streamWindowBytes = gFgNextSceneStage.loadedBytes;
        gFgRuntime.streamWindowValid = 1;
        FG_STAGE_PRINTF("JCSTAGE P %lu/%lu\n",
                        (unsigned long)gFgNextSceneStage.loadedBytes,
                        (unsigned long)gFgNextSceneStage.windowBytes);
        fgNextSceneStageInvalidate();
        ps1PerfMarkStageAdopt(PS1_PERF_STAGE_ADOPT_WINDOW);
        return 1;
    }

    if (!fgNextSceneStageMatches(sceneName) ||
        !gFgNextSceneStage.usesStreamWindow ||
        gFgNextSceneStage.readInFlight ||
        gFgRuntime.streamWindowBuffer != gFgStreamWindowBuffer ||
        gFgRuntime.streamWindowSize < gFgNextSceneStage.loadedBytes ||
        gFgStreamWindowBuffer == NULL ||
        gFgStreamWindowBufferSize < gFgNextSceneStage.loadedBytes) {
        fgNextSceneStageInvalidate();
        return 0;
    }

    gFgRuntime.streamWindowStart = gFgNextSceneStage.windowStart;
    gFgRuntime.streamWindowBytes = gFgNextSceneStage.loadedBytes;
    gFgRuntime.streamWindowValid = 1;
    FG_STAGE_PRINTF("JCSTAGE A %lu/%lu\n",
                    (unsigned long)gFgNextSceneStage.loadedBytes,
                    (unsigned long)gFgNextSceneStage.windowBytes);
    fgNextSceneStageInvalidate();
    ps1PerfMarkStageAdopt(PS1_PERF_STAGE_ADOPT_WINDOW);
    return 1;
}

static int fgRuntimePrimeSetupSegment(const char *sceneName)
{
    uint32 segmentStart;
    uint32 segmentBytes;
    uint32 segment2Start = 0;
    uint32 segment2Bytes = 0;
    uint32 segment3Start = 0;
    uint32 segment3Bytes = 0;
    uint32 segment4Start = 0;
    uint32 segment4Bytes = 0;
    uint32 allocationBytes;
    uint8 *segmentBuffer;
    uint8 *segment2Buffer = NULL;
    uint8 *segment3Buffer = NULL;
    uint8 *segment4Buffer = NULL;

    gFgCleanRectMaxBytes = 96UL * 1024UL;
    gFgRuntime.setupSegmentReusable = 0;
    gFgRuntime.setupSegment2Buffer = NULL;
    gFgRuntime.setupSegment2Start = 0;
    gFgRuntime.setupSegment2Bytes = 0;
    gFgRuntime.setupSegment2Primed = 0;
    gFgRuntime.setupSegment3Buffer = NULL;
    gFgRuntime.setupSegment3Start = 0;
    gFgRuntime.setupSegment3Bytes = 0;
    gFgRuntime.setupSegment3Primed = 0;
    gFgRuntime.setupSegment4Buffer = NULL;
    gFgRuntime.setupSegment4Start = 0;
    gFgRuntime.setupSegment4Bytes = 0;
    gFgRuntime.setupSegment4Primed = 0;
    if (fgSceneEquals(sceneName, "visitor3")) {
        if (islandState.lowTide) {
            segmentStart = FG_VISITOR3_LOW_SETUP_SEGMENT_START;
            segmentBytes = FG_VISITOR3_LOW_SETUP_SEGMENT_BYTES;
            segment2Start = FG_VISITOR3_LOW_SETUP_SEGMENT2_START;
            segment2Bytes = FG_VISITOR3_LOW_SETUP_SEGMENT2_BYTES;
            segment3Start = FG_VISITOR3_LOW_SETUP_SEGMENT3_START;
            segment3Bytes = FG_VISITOR3_LOW_SETUP_SEGMENT3_BYTES;
            segment4Start = FG_VISITOR3_LOW_SETUP_SEGMENT4_START;
            segment4Bytes = FG_VISITOR3_LOW_SETUP_SEGMENT4_BYTES;
        } else {
            segmentStart = FG_VISITOR3_HIGH_SETUP_SEGMENT_START;
            segmentBytes = FG_VISITOR3_HIGH_SETUP_SEGMENT_BYTES;
            segment2Start = FG_VISITOR3_HIGH_SETUP_SEGMENT2_START;
            segment2Bytes = FG_VISITOR3_HIGH_SETUP_SEGMENT2_BYTES;
            segment3Start = FG_VISITOR3_HIGH_SETUP_SEGMENT3_START;
            segment3Bytes = FG_VISITOR3_HIGH_SETUP_SEGMENT3_BYTES;
            gFgCleanRectMaxBytes = 64UL * 1024UL;
        }
        allocationBytes = segmentBytes + segment2Bytes + segment3Bytes;
        /* MEM_REGION_RATIONALE: per-scene segment-decode scratch.
         * Reclaimed wholesale by memSceneReset. The old "if big
         * enough, reuse" logic is unnecessary under TRANSIENT
         * semantics — every scene gets a fresh buffer. */
        gFgSetupSegmentBuffer = (uint8 *)memAlloc(MEM_REGION_TRANSIENT,
                                                  allocationBytes,
                                                  "fgSetupSegmentBuffer");
        gFgSetupSegmentBufferSize = allocationBytes;
        gFgSetupSegmentBufferRegion = MEM_REGION_TRANSIENT;
        segmentBuffer = gFgSetupSegmentBuffer;
        if (segmentBuffer == NULL) {
            gFgSetupSegmentBufferSize = 0;
            if (ps1PerfEnabled)
                ps1PerfMarkAllocFail(allocationBytes);
            return 0;
        }
        if (segment2Bytes > 0)
            segment2Buffer = segmentBuffer + segmentBytes;
        if (segment3Bytes > 0)
            segment3Buffer = segmentBuffer + segmentBytes + segment2Bytes;
        if (segment4Bytes > 0) {
            /* MEM_REGION_RATIONALE: VISITOR3-low fourth setup slice.
             * CACHE keeps the paid low-tail residency alive past setup. */
            gFgSetupSegment4OwnedBuffer = (uint8 *)memAlloc(MEM_REGION_CACHE,
                                                            segment4Bytes,
                                                            "fgSetupSegment4Buffer");
            gFgSetupSegment4OwnedBufferSize = segment4Bytes;
            gFgSetupSegment4OwnedBufferRegion = MEM_REGION_CACHE;
            segment4Buffer = gFgSetupSegment4OwnedBuffer;
            if (segment4Buffer == NULL) {
                gFgSetupSegment4OwnedBufferSize = 0;
                memFree(gFgSetupSegmentBufferRegion, gFgSetupSegmentBuffer);
                gFgSetupSegmentBuffer = NULL;
                gFgSetupSegmentBufferSize = 0;
                gFgSetupSegmentBufferRegion = MEM_REGION_TRANSIENT;
                if (ps1PerfEnabled)
                    ps1PerfMarkAllocFail(segment4Bytes);
                return 0;
            }
        }
        gFgRuntime.setupSegmentReusable = 1;
    } else if (!islandState.lowTide && fgSceneEquals(sceneName, "building2")) {
        segmentStart = FG_BUILDING2_HIGH_SETUP_SEGMENT_START;
        segmentBytes = FG_BUILDING2_HIGH_SETUP_SEGMENT_BYTES;
        segment2Start = FG_BUILDING2_HIGH_SETUP_SEGMENT2_START;
        segment2Bytes = FG_BUILDING2_HIGH_SETUP_SEGMENT2_BYTES;
        allocationBytes = segmentBytes + segment2Bytes;
        /* MEM_REGION_RATIONALE: targeted reusable read-cluster cache.
         * CACHE avoids libc fragmentation and the reduced footprint preserves
         * clean-rect + stream-window headroom under the region allocator. */
        gFgSetupSegmentBuffer = (uint8 *)memAlloc(MEM_REGION_CACHE,
                                                  allocationBytes,
                                                  "fgSetupSegmentBuffer");
        gFgSetupSegmentBufferSize = allocationBytes;
        gFgSetupSegmentBufferRegion = MEM_REGION_CACHE;
        segmentBuffer = gFgSetupSegmentBuffer;
        if (segmentBuffer == NULL) {
            gFgSetupSegmentBufferSize = 0;
            if (ps1PerfEnabled)
                ps1PerfMarkAllocFail(allocationBytes);
            return 0;
        }
        segment2Buffer = segmentBuffer + segmentBytes;
        gFgRuntime.setupSegmentReusable = 1;
    } else if (islandState.lowTide && fgSceneEquals(sceneName, "building2")) {
        segmentStart = FG_BUILDING2_LOW_SETUP_SEGMENT_START;
        segmentBytes = FG_BUILDING2_LOW_SETUP_SEGMENT_BYTES;
        segment2Start = FG_BUILDING2_LOW_SETUP_SEGMENT2_START;
        segment2Bytes = FG_BUILDING2_LOW_SETUP_SEGMENT2_BYTES;
        gFgCleanRectMaxBytes = 80UL * 1024UL;
        allocationBytes = segmentBytes + segment2Bytes;
        /* MEM_REGION_RATIONALE: targeted low-tide BUILDING2 cluster cache.
         * The setup-prime window covers the early pack; this pays the next
         * dense active-loop read cluster without growing the main window. */
        gFgSetupSegmentBuffer = (uint8 *)memAlloc(MEM_REGION_CACHE,
                                                  allocationBytes,
                                                  "fgSetupSegmentBuffer");
        gFgSetupSegmentBufferSize = allocationBytes;
        gFgSetupSegmentBufferRegion = MEM_REGION_CACHE;
        segmentBuffer = gFgSetupSegmentBuffer;
        if (segmentBuffer == NULL) {
            gFgSetupSegmentBufferSize = 0;
            if (ps1PerfEnabled)
                ps1PerfMarkAllocFail(allocationBytes);
            return 0;
        }
        segment2Buffer = segmentBuffer + segmentBytes;
        gFgRuntime.setupSegmentReusable = 1;
    } else if (!islandState.lowTide && fgSceneEquals(sceneName, "building4")) {
        segmentStart = FG_BUILDING4_HIGH_SETUP_SEGMENT_START;
        segmentBytes = FG_BUILDING4_HIGH_SETUP_SEGMENT_BYTES;
        allocationBytes = segmentBytes;
        /* MEM_REGION_RATIONALE: targeted reusable read-cluster cache.
         * Small CACHE allocation; BUILDING4 rows are CD-bound near 99%. */
        gFgSetupSegmentBuffer = (uint8 *)memAlloc(MEM_REGION_CACHE,
                                                  allocationBytes,
                                                  "fgSetupSegmentBuffer");
        gFgSetupSegmentBufferSize = allocationBytes;
        gFgSetupSegmentBufferRegion = MEM_REGION_CACHE;
        segmentBuffer = gFgSetupSegmentBuffer;
        if (segmentBuffer == NULL) {
            gFgSetupSegmentBufferSize = 0;
            if (ps1PerfEnabled)
                ps1PerfMarkAllocFail(allocationBytes);
            return 0;
        }
        gFgRuntime.setupSegmentReusable = 1;
    } else if (fgSceneEquals(sceneName, "walkstuf1")) {
        if (islandState.lowTide) {
            segmentStart = FG_WALKSTUF1_LOW_SETUP_SEGMENT_START;
            segmentBytes = FG_WALKSTUF1_LOW_SETUP_SEGMENT_BYTES;
            segment2Start = FG_WALKSTUF1_LOW_SETUP_SEGMENT2_START;
            segment2Bytes = FG_WALKSTUF1_LOW_SETUP_SEGMENT2_BYTES;
            gFgCleanRectMaxBytes = 48UL * 1024UL;
        } else {
            segmentStart = FG_WALKSTUF1_HIGH_SETUP_SEGMENT_START;
            segmentBytes = FG_WALKSTUF1_HIGH_SETUP_SEGMENT_BYTES;
            segment2Start = FG_WALKSTUF1_HIGH_SETUP_SEGMENT2_START;
            segment2Bytes = FG_WALKSTUF1_HIGH_SETUP_SEGMENT2_BYTES;
            segment4Start = FG_WALKSTUF1_HIGH_SETUP_SEGMENT4_START;
            segment4Bytes = FG_WALKSTUF1_HIGH_SETUP_SEGMENT4_BYTES;
            /* Segment 4 is optional read-ahead. Keep its TRANSIENT bytes free
             * for the W1-high clean snapshot; the normal stream path fetches
             * the same FG2 payloads on demand. */
#if PS1_VERBOSE_DIAGNOSTICS
            printf("JCMEM setup-seg4-disabled scene=walkstuf1 bytes=%lu reason=clean-headroom\n",
                   (unsigned long)FG_WALKSTUF1_HIGH_SETUP_SEGMENT4_DISABLED_BYTES);
#endif
            gFgCleanRectMaxBytes = 64UL * 1024UL;
        }
        allocationBytes = segmentBytes + (islandState.lowTide ? 0UL : segment2Bytes);
        /* MEM_REGION_RATIONALE: targeted reusable read-cluster cache.
         * CACHE avoids the small libc heap for the main W1 residency. W1-low's
         * small trailing edge is split into TRANSIENT to avoid a large CACHE
         * cliff while still covering the active-loop boundary. */
        gFgSetupSegmentBuffer = (uint8 *)memAlloc(MEM_REGION_CACHE,
                                                  allocationBytes,
                                                  "fgSetupSegmentBuffer");
        gFgSetupSegmentBufferSize = allocationBytes;
        gFgSetupSegmentBufferRegion = MEM_REGION_CACHE;
        segmentBuffer = gFgSetupSegmentBuffer;
        if (segmentBuffer == NULL) {
            gFgSetupSegmentBufferSize = 0;
            if (ps1PerfEnabled)
                ps1PerfMarkAllocFail(allocationBytes);
            return 0;
        }
        if (segment2Bytes > 0) {
            if (islandState.lowTide) {
                /* MEM_REGION_RATIONALE: W1-low trailing setup edge.
                 * TRANSIENT avoids growing the already large CACHE segment. */
                gFgSetupSegment2OwnedBuffer = (uint8 *)memAlloc(MEM_REGION_TRANSIENT,
                                                                segment2Bytes,
                                                                "fgSetupSegment2Buffer");
                gFgSetupSegment2OwnedBufferSize = segment2Bytes;
                gFgSetupSegment2OwnedBufferRegion = MEM_REGION_TRANSIENT;
                segment2Buffer = gFgSetupSegment2OwnedBuffer;
                if (segment2Buffer == NULL) {
                    gFgSetupSegment2OwnedBufferSize = 0;
                    memFree(gFgSetupSegmentBufferRegion, gFgSetupSegmentBuffer);
                    gFgSetupSegmentBuffer = NULL;
                    gFgSetupSegmentBufferSize = 0;
                    gFgSetupSegmentBufferRegion = MEM_REGION_TRANSIENT;
                    if (ps1PerfEnabled)
                        ps1PerfMarkAllocFail(segment2Bytes);
                    return 0;
                }
            } else {
                segment2Buffer = segmentBuffer + segmentBytes;
            }
        }
        if (!islandState.lowTide && segment4Bytes > 0) {
            /* MEM_REGION_RATIONALE: W1-high late setup slice.
             * TRANSIENT pays the final cluster without exhausting CACHE. */
            gFgSetupSegment4OwnedBuffer = (uint8 *)memAlloc(MEM_REGION_TRANSIENT,
                                                            segment4Bytes,
                                                            "fgSetupSegment4Buffer");
            gFgSetupSegment4OwnedBufferSize = segment4Bytes;
            gFgSetupSegment4OwnedBufferRegion = MEM_REGION_TRANSIENT;
            segment4Buffer = gFgSetupSegment4OwnedBuffer;
            if (segment4Buffer == NULL) {
                gFgSetupSegment4OwnedBufferSize = 0;
                memFree(gFgSetupSegmentBufferRegion, gFgSetupSegmentBuffer);
                gFgSetupSegmentBuffer = NULL;
                gFgSetupSegmentBufferSize = 0;
                gFgSetupSegmentBufferRegion = MEM_REGION_TRANSIENT;
                if (ps1PerfEnabled)
                    ps1PerfMarkAllocFail(segment4Bytes);
                return 0;
            }
        }
        gFgRuntime.setupSegmentReusable = 1;
    } else if (!fgSceneEquals(sceneName, "fishing3")) {
        return 1;
    } else if (islandState.lowTide) {
        segmentStart = FG_FISHING3_LOW_SETUP_SEGMENT_START;
        segmentBytes = FG_FISHING3_LOW_SETUP_SEGMENT_BYTES;
        /* MEM_REGION_RATIONALE: per-scene segment-decode scratch (fishing3 low). */
        gFgSetupSegmentBuffer = (uint8 *)memAlloc(MEM_REGION_TRANSIENT,
                                                  FG_FISHING3_LOW_SETUP_SEGMENT_BYTES,
                                                  "fgSetupSegmentBuffer");
        gFgSetupSegmentBufferSize = segmentBytes;
        gFgSetupSegmentBufferRegion = MEM_REGION_TRANSIENT;
        segmentBuffer = gFgSetupSegmentBuffer;
        if (segmentBuffer == NULL) {
            gFgSetupSegmentBufferSize = 0;
            if (ps1PerfEnabled)
                ps1PerfMarkAllocFail(segmentBytes);
            return 0;
        }
    } else {
        segmentStart = FG_FISHING3_HIGH_SETUP_SEGMENT_START;
        segmentBytes = FG_FISHING3_HIGH_SETUP_SEGMENT_BYTES;
        if (gFgRuntime.streamScratch == NULL ||
            gFgRuntime.streamScratchSize < segmentBytes)
            return 1;
        segmentBuffer = gFgRuntime.streamScratch;
    }

    if (segment2Bytes > 0 &&
        !ps1_streamReadAlignedIntoFile(&gFgRuntime.packCdFile,
                                       segment2Start,
                                       segment2Bytes,
                                       segment2Buffer)) {
        gFgRuntime.setupSegmentPrimed = 0;
        gFgRuntime.setupSegment2Primed = 0;
        gFgRuntime.setupSegment3Primed = 0;
        gFgRuntime.setupSegment4Primed = 0;
        return 0;
    }

    if (segment3Bytes > 0 &&
        !ps1_streamReadAlignedIntoFile(&gFgRuntime.packCdFile,
                                       segment3Start,
                                       segment3Bytes,
                                       segment3Buffer)) {
        gFgRuntime.setupSegmentPrimed = 0;
        gFgRuntime.setupSegment2Primed = 0;
        gFgRuntime.setupSegment3Primed = 0;
        gFgRuntime.setupSegment4Primed = 0;
        return 0;
    }

    if (segment4Bytes > 0 &&
        !ps1_streamReadAlignedIntoFile(&gFgRuntime.packCdFile,
                                       segment4Start,
                                       segment4Bytes,
                                       segment4Buffer)) {
        gFgRuntime.setupSegmentPrimed = 0;
        gFgRuntime.setupSegment2Primed = 0;
        gFgRuntime.setupSegment3Primed = 0;
        gFgRuntime.setupSegment4Primed = 0;
        return 0;
    }

    if (!ps1_streamReadAlignedIntoFile(&gFgRuntime.packCdFile,
                                       segmentStart,
                                       segmentBytes,
                                       segmentBuffer)) {
        gFgRuntime.setupSegmentPrimed = 0;
        gFgRuntime.setupSegment2Primed = 0;
        gFgRuntime.setupSegment3Primed = 0;
        gFgRuntime.setupSegment4Primed = 0;
        return 0;
    }

    gFgRuntime.setupSegmentBuffer = segmentBuffer;
    gFgRuntime.setupSegmentStart = segmentStart;
    gFgRuntime.setupSegmentBytes = segmentBytes;
    gFgRuntime.setupSegmentPrimed = 1;
    if (segment2Bytes > 0) {
        gFgRuntime.setupSegment2Buffer = segment2Buffer;
        gFgRuntime.setupSegment2Start = segment2Start;
        gFgRuntime.setupSegment2Bytes = segment2Bytes;
        gFgRuntime.setupSegment2Primed = 1;
    }
    if (segment3Bytes > 0) {
        gFgRuntime.setupSegment3Buffer = segment3Buffer;
        gFgRuntime.setupSegment3Start = segment3Start;
        gFgRuntime.setupSegment3Bytes = segment3Bytes;
        gFgRuntime.setupSegment3Primed = 1;
    }
    if (segment4Bytes > 0) {
        gFgRuntime.setupSegment4Buffer = segment4Buffer;
        gFgRuntime.setupSegment4Start = segment4Start;
        gFgRuntime.setupSegment4Bytes = segment4Bytes;
        gFgRuntime.setupSegment4Primed = 1;
    }
    return 1;
}

static int fgRuntimeTryStageNextFrame(uint16 *outElapsedVBlanks)
{
    uint16 nextFrameIndex;
    uint16 slackVBlanks;
    const struct TFgPilotEntry *entry;
    int ok;
    uint32 stageTick = 0;
    uint16 elapsedVBlanks = 0;

    if (!fgRuntimeCanStageNextFrame())
        return 0;

    if (gFgRuntime.stagedFrameValid) {
        if (ps1PerfEnabled)
            ps1PerfMarkPrefetchDuplicate();
        return 0;
    }

    if (gFgRuntime.frameIndex + 1 >= gFgRuntime.header.frameCount)
        return 0;
    nextFrameIndex = (uint16)(gFgRuntime.frameIndex + 1);
    entry = fgGetEntryFromTable(&gFgRuntime.entryTable, nextFrameIndex);
    if (entry == NULL) {
        if (ps1PerfEnabled)
            ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
        return 1;
    }

    if (!fgEntryHasPayload(entry))
        return 0;

    slackVBlanks = fgRuntimeHeldSlackBeforeWait();
    if (slackVBlanks == 0) {
        if (ps1PerfEnabled)
            ps1PerfMarkPrefetchAttempt(slackVBlanks, slackVBlanks, 0);
        if (ps1PerfEnabled)
            ps1PerfMarkPrefetchSkipNoSlack();
        return 0;
    }

    if (entry->dataSize > gFgRuntime.prefetchFrameBufferSize) {
        if (ps1PerfEnabled)
            ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
        return 1;
    }

    if (fgRuntimeEntryFitsWindow(entry)) {
        if (!fgRuntimeWindowContainsEntry(entry)) {
            int eligible = fgRuntimeWindowSlackEligible(slackVBlanks);
            if (ps1PerfEnabled)
                ps1PerfMarkPrefetchAttempt(slackVBlanks, slackVBlanks, eligible);
            if (!eligible) {
                if (ps1PerfEnabled)
                    ps1PerfMarkPrefetchSkipNoSlack();
                return 0;
            }
            if (fgRuntimeShouldDirectStageEntry(nextFrameIndex,
                                                entry,
                                                slackVBlanks)) {
                stageTick = fgReadTickCounter();
                if (ps1PerfEnabled)
                    ps1PerfBeginPrefetchRead(slackVBlanks);
                fgRuntimeClearVolatileSetupSegment();
                ok = ps1_streamReadIntoFileBuffered(&gFgRuntime.packCdFile,
                                                    entry->dataOffset,
                                                    entry->dataSize,
                                                    gFgRuntime.prefetchFrameBuffer,
                                                    gFgRuntime.streamScratch,
                                                    gFgRuntime.streamScratchSize);
                elapsedVBlanks = (uint16)ps1PerfElapsedVBlanks(stageTick);
                if (ps1PerfEnabled)
                    ps1PerfEndPrefetchRead(elapsedVBlanks, entry->dataSize, ok);
                *outElapsedVBlanks = elapsedVBlanks;

                if (!ok) {
                    if (ps1PerfEnabled)
                        ps1PerfMarkTripwire();
                    gFgRuntime.active = 0;
                    return 1;
                }

                if (!fgRuntimeDecodeAndSetStagedFrame(nextFrameIndex, entry)) {
                    if (ps1PerfEnabled)
                        ps1PerfMarkTripwire();
                    gFgRuntime.active = 0;
                    return 1;
                }
                FG_CACHE_CHECK("fg-stage-direct");
                {
                    uint32 windowStart = fgSectorAlignDown(entry->dataOffset);
                    uint32 windowBytes =
                        fgSectorAlignUp(entry->dataOffset + entry->dataSize) -
                        windowStart;
                    memcpy(gFgRuntime.streamWindowBuffer,
                           gFgRuntime.streamScratch,
                           windowBytes);
                    gFgRuntime.streamWindowStart = windowStart;
                    gFgRuntime.streamWindowBytes = windowBytes;
                    gFgRuntime.streamWindowValid = 1;
                }
                FG_CACHE_CHECK("fg-stage-direct-window");
                return 1;
            }
            if (!fgRuntimeFillWindowForEntry(entry, nextFrameIndex, slackVBlanks, 1, &elapsedVBlanks)) {
                if (ps1PerfEnabled)
                    ps1PerfMarkTripwire();
                gFgRuntime.active = 0;
                return 1;
            }
            *outElapsedVBlanks = elapsedVBlanks;
        } else if (ps1PerfEnabled) {
            ps1PerfMarkPrefetchAttempt(slackVBlanks, slackVBlanks, 1);
        }
        if (!fgRuntimeCopyEntryFromWindow(entry, gFgRuntime.prefetchFrameBuffer, 0)) {
            if (ps1PerfEnabled)
                ps1PerfMarkTripwire();
            gFgRuntime.active = 0;
            return 1;
        }
        if (!fgRuntimeDecodeAndSetStagedFrame(nextFrameIndex, entry)) {
            if (ps1PerfEnabled)
                ps1PerfMarkTripwire();
            gFgRuntime.active = 0;
            return 1;
        }
        FG_CACHE_CHECK("fg-stage-window");
        return 1;
    }

    if (gFgRuntimeSceneId == FG_SCENE_VISITOR3 &&
        slackVBlanks < FG_PREFETCH_FALLTHROUGH_MIN_SLACK_VBLANKS) {
        if (ps1PerfEnabled) {
            ps1PerfMarkPrefetchAttempt(slackVBlanks, slackVBlanks, 0);
            ps1PerfMarkPrefetchSkipNoSlack();
        }
        return 0;
    }

    if (ps1PerfEnabled)
        ps1PerfMarkPrefetchAttempt(slackVBlanks, slackVBlanks, 1);
    stageTick = fgReadTickCounter();
    if (ps1PerfEnabled) {
        ps1PerfBeginPrefetchRead(slackVBlanks);
    }
    fgRuntimeClearVolatileSetupSegment();
    ok = ps1_streamReadIntoFileBuffered(&gFgRuntime.packCdFile,
                                        entry->dataOffset,
                                        entry->dataSize,
                                        gFgRuntime.prefetchFrameBuffer,
                                        gFgRuntime.streamScratch,
                                        gFgRuntime.streamScratchSize);
    elapsedVBlanks = (uint16)ps1PerfElapsedVBlanks(stageTick);
    if (ps1PerfEnabled) {
        ps1PerfEndPrefetchRead(elapsedVBlanks, entry->dataSize, ok);
    }
    *outElapsedVBlanks = elapsedVBlanks;

    if (!ok) {
        if (ps1PerfEnabled)
            ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
        return 1;
    }

    if (!fgRuntimeDecodeAndSetStagedFrame(nextFrameIndex, entry)) {
        if (ps1PerfEnabled)
            ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
        return 1;
    }
    FG_CACHE_CHECK("fg-stage-read");
    return 1;
}

static int fgRuntimeTryPrefetchWindow(uint16 *outElapsedVBlanks)
{
    uint16 slackVBlanks;
    uint16 entryFrameIndex = 0;
    const struct TFgPilotEntry *entry;

    entry = fgRuntimeNextPayloadEntry(&entryFrameIndex);
    if (!fgRuntimeEntryFitsWindow(entry))
        return 0;

    if (fgRuntimeWindowContainsEntry(entry)) {
        if (ps1PerfEnabled)
            ps1PerfMarkPrefetchDuplicate();
        return 0;
    }

    slackVBlanks = fgRuntimeHeldSlackBeforeWait();
    if (slackVBlanks == 0) {
        if (ps1PerfEnabled) {
            ps1PerfMarkPrefetchAttempt(slackVBlanks, slackVBlanks, 0);
            ps1PerfMarkPrefetchSkipNoSlack();
        }
        return 0;
    }

    if (gFgRuntimeSceneId == FG_SCENE_WALKSTUF1 &&
        slackVBlanks <= (uint16)(islandState.lowTide ?
            FG_PREFETCH_WINDOW_MIN_SLACK_VBLANKS :
            FG_PREFETCH_WINDOW_MIN_SLACK_VBLANKS + 1)) {
        if (ps1PerfEnabled) {
            ps1PerfMarkPrefetchAttempt(slackVBlanks, slackVBlanks, 0);
            ps1PerfMarkPrefetchSkipNoSlack();
        }
        return 0;
    }

    if (!fgRuntimeWindowSlackEligible(slackVBlanks)) {
        if (ps1PerfEnabled) {
            ps1PerfMarkPrefetchAttempt(slackVBlanks, slackVBlanks, 0);
            ps1PerfMarkPrefetchSkipNoSlack();
        }
        return 0;
    }

    if (ps1PerfEnabled)
        ps1PerfMarkPrefetchAttempt(slackVBlanks, slackVBlanks, 1);

    if (!fgRuntimeFillWindowForEntry(entry, entryFrameIndex, slackVBlanks, 1, outElapsedVBlanks)) {
        if (ps1PerfEnabled)
            ps1PerfMarkTripwire();
        gFgRuntime.active = 0;
        return 1;
    }
    return 1;
}

static int fgRuntimeWindowPrefetchWouldRead(void)
{
    const struct TFgPilotEntry *entry;

    entry = fgRuntimeNextPayloadEntry(NULL);
    if (!fgRuntimeEntryFitsWindow(entry))
        return 0;

    return fgRuntimeWindowContainsEntry(entry) ? 0 : 1;
}

static int fgRuntimeLoadSceneFrame(uint16 frameIndex)
{
    const struct TFgPilotEntry *entry = fgGetEntryFromTable(&gFgRuntime.entryTable, frameIndex);
    const uint8 *baseFrameData;
    uint32 baseFrameSize;
    uint8 *loadBuffer;
    uint32 loadBufferCapacity;
    uint8 useAlternateLoadBuffer = 0;
    struct TFgPilotEntry loadedEntry;
    int entryIsEmpty;
    int stagedResult;

    if (entry == NULL)
        return 0;

    if (ps1PerfEnabled)
        ps1PerfSetCurrentFrame(frameIndex, entry->sourceFrame, entry->dataOffset);

    stagedResult = fgRuntimeConsumeStagedFrame(frameIndex);
    if (stagedResult != 0) {
        FG_CACHE_CHECK("fg-load-consumed-staged");
        return stagedResult > 0 ? 1 : 0;
    }

    /* An empty entry (w=0, h=0, no payload) is a capture artifact from a
     * host frame where the ledger was blank (mid-refresh). Hold the previous
     * frame's visible state rather than wiping, so the viewer doesn't see a
     * one-frame flash. Advance the sound-event cursor + hold timing, but
     * keep currentEntry + currentFrameData pointing at the prior frame. */
    entryIsEmpty = (entry->dataSize == 0 && entry->width == 0 && entry->height == 0);

    if (entryIsEmpty && gFgRuntime.currentFrameData != NULL) {
        gFgRuntime.frameRendered = 1;
        gFgRuntime.displayVBlanks = fgEntryHoldVBlanks(&gFgRuntime.header,
                                                       entry,
                                                       gFgRuntime.presentedVBlanks);
        if (ps1PerfEnabled)
            ps1PerfMarkEntry(0, gFgRuntime.displayVBlanks, 1,
                             entry->sourceFrame, entry->dataOffset);
        FG_CACHE_CHECK("fg-load-empty-hold");
        fgFireSoundEventsUpTo(entry->sourceFrame);
        fgTelemetryUpdate();
        return 1;
    }

    baseFrameData = gFgRuntime.currentFrameData;
    baseFrameSize = gFgRuntime.currentEntry.dataSize;
    loadedEntry = *entry;
    gFgRuntime.currentEntry = loadedEntry;
    gFgRuntime.currentFrameData = NULL;
    gFgRuntime.frameRendered = 0;

    /* Stream the frame payload into the pre-allocated frameBuffer. Avoids
     * the per-frame malloc+free churn of ps1_streamRead — important for
     * screensaver-loop scenes with big per-frame payloads (fishing3's
     * 89 KB squid-emerge frame was failing its second-iteration contiguous
     * alloc after the heap fragmented). frameBuffer and streamScratch are
     * allocated once at foregroundPilotRuntimeStart. */
    if (loadedEntry.dataSize > 0 &&
        loadedEntry.width > 0 &&
        loadedEntry.height > 0) {
        if (gFgRuntime.frameBuffer == NULL ||
            loadedEntry.dataSize > gFgRuntime.frameBufferSize ||
            !gFgRuntime.packCdFileValid ||
            gFgRuntime.streamScratch == NULL) {
            if (ps1PerfEnabled)
                ps1PerfMarkTripwire();
            return 0;
        }

        loadBuffer = gFgRuntime.frameBuffer;
        loadBufferCapacity = gFgRuntime.frameBufferSize;
        FG_CACHE_CHECK("fg-load-before-read");
        if (baseFrameData != NULL &&
            fgRuntimeUsesPreviousFrameDelta(frameIndex) &&
            gFgRuntime.prefetchFrameBuffer != NULL &&
            loadedEntry.dataSize <= gFgRuntime.prefetchFrameBufferSize) {
            loadBuffer = gFgRuntime.prefetchFrameBuffer;
            loadBufferCapacity = gFgRuntime.prefetchFrameBufferSize;
            useAlternateLoadBuffer = 1;
        }

        if (fgRuntimeCopyEntryFromWindow(&loadedEntry,
                                         loadBuffer,
                                         1)) {
            /* Loaded from the retained setup/window cache. */
            FG_CACHE_CHECK("fg-load-after-window-copy");
        } else if (fgRuntimeEntryFitsWindow(&loadedEntry)) {
            if (!fgRuntimeFillWindowForEntry(&loadedEntry, frameIndex, 0, 0, NULL) ||
                !fgRuntimeCopyEntryFromWindow(&loadedEntry,
                                              loadBuffer,
                                              0)) {
                if (ps1PerfEnabled)
                    ps1PerfMarkTripwire();
                return 0;
            }
            FG_CACHE_CHECK("fg-load-after-window-fill");
        } else {
            fgRuntimeClearVolatileSetupSegment();
            if (!ps1_streamReadIntoFileBuffered(&gFgRuntime.packCdFile,
                                                loadedEntry.dataOffset,
                                                loadedEntry.dataSize,
                                                loadBuffer,
                                                gFgRuntime.streamScratch,
                                                gFgRuntime.streamScratchSize)) {
                if (ps1PerfEnabled)
                    ps1PerfMarkTripwire();
                return 0;
            }
            FG_CACHE_CHECK("fg-load-after-read");
        }

        if (!fgRuntimeDecodeFramePayload(loadBuffer,
                                         loadBufferCapacity,
                                         &loadedEntry,
                                         baseFrameData,
                                         baseFrameSize)) {
            if (ps1PerfEnabled)
                ps1PerfMarkTripwire();
            return 0;
        }
        FG_CACHE_CHECK("fg-load-after-decode");

        if (useAlternateLoadBuffer) {
            uint8 *oldFrameBuffer = gFgRuntime.frameBuffer;
            uint32 oldFrameBufferSize = gFgRuntime.frameBufferSize;
            gFgRuntime.frameBuffer = gFgRuntime.prefetchFrameBuffer;
            gFgRuntime.frameBufferSize = gFgRuntime.prefetchFrameBufferSize;
            gFgRuntime.prefetchFrameBuffer = oldFrameBuffer;
            gFgRuntime.prefetchFrameBufferSize = oldFrameBufferSize;
            fgRuntimeSyncFrameBufferGlobals();
            FG_CACHE_CHECK("fg-load-after-swap");
        }
        gFgRuntime.currentEntry = loadedEntry;
        gFgRuntime.currentFrameData = gFgRuntime.frameBuffer;
    }

    FG_CACHE_CHECK("fg-load-frame");
    gFgRuntime.displayVBlanks = fgEntryHoldVBlanks(&gFgRuntime.header,
                                                   &gFgRuntime.currentEntry,
                                                   gFgRuntime.presentedVBlanks);
    if (ps1PerfEnabled)
        ps1PerfMarkEntry(gFgRuntime.currentEntry.dataSize,
                         gFgRuntime.displayVBlanks,
                         entryIsEmpty ? 1 : 0,
                         gFgRuntime.currentEntry.sourceFrame,
                         gFgRuntime.currentEntry.dataOffset);
    fgFireSoundEventsUpTo(gFgRuntime.currentEntry.sourceFrame);
    fgTelemetryUpdate();
    return 1;
}

static void fgRuntimeComposeEntryToBackground(const struct TFgPilotEntry *entry,
                                              uint8 *frameData)
{
    if (gFgRuntime.packFormat == kFgPilotPackFormatPal4Spans) {
        grCompositePacked4SpansToBackground(frameData,
                                            entry->dataSize,
                                            gFgRuntime.palette,
                                            entry->x,
                                            entry->y);
    } else if (gFgRuntime.packFormat == kFgPilotPackFormatPal4TemporalResidual) {
        grCompositePacked4TemporalResidualToBackground(frameData,
                                                       entry->dataSize,
                                                       gFgRuntime.palette,
                                                       entry->x,
                                                       entry->y);
    } else if (gFgRuntime.packFormat == kFgPilotPackFormatPal4CompactTemporalResidual) {
        grCompositePacked4CompactTemporalResidualToBackground(frameData,
                                                              entry->dataSize,
                                                              gFgRuntime.palette,
                                                              entry->x,
                                                              entry->y);
    } else if (gFgRuntime.packFormat == kFgPilotPackFormatIndexed8TemporalResidual) {
        grCompositeIndexed8TemporalResidualToBackground(frameData,
                                                        entry->dataSize,
                                                        gFgRuntime.palette,
                                                        entry->x,
                                                        entry->y);
    } else if (gFgRuntime.packFormat == kFgPilotPackFormatIndexed8Spans) {
        grCompositeIndexed8SpansToBackground(frameData,
                                             entry->dataSize,
                                             gFgRuntime.palette,
                                             entry->x,
                                             entry->y);
    }

    /* Stamp holiday overlay on top of the pack so Johnny walks behind
     * the holiday decoration, matching islandInitHoliday's z-order. */
    fgBackdropStampHoliday();
}

static int fgRuntimePrepareStagedFrameForPresent(uint16 *outElapsedVBlanks,
                                                 int perfDetail)
{
    uint32 prepTick;
    uint32 perfTick = 0;
    uint16 elapsedVBlanks;

    prepTick = fgReadTickCounter();

    if (perfDetail)
        ps1PerfBeginPipeline(PS1_PERF_PIPE_PREPARE);

    FG_CACHE_CHECK("fg-prepare-before");
    if (perfDetail)
        perfTick = ps1PerfTick();
    if (fgRuntimeUsesTemporalResidual())
        grBeginResidualCleanBgFrame();
    else
        grRestoreBgFromRects();
    FG_CACHE_CHECK("fg-prepare-after-restore");
    if (perfDetail)
        ps1PerfMarkRenderPhase(PS1_PERF_RENDER_RESTORE,
                               ps1PerfElapsedVBlanks(perfTick));

    /* Tick ocean wave animation each frame so scenes whose pack carries
     * no captured water frames (the STAND no-stitch foreground-only
     * fast path being the most common case) still get moving waves.
     * Mirrors adsPilotTickBackgroundWaves' timer pattern: advance one
     * wave frame every gFgBackdropThread.delay vblanks, and on
     * intermediate frames redraw the last wave so grRestoreBgFromRects
     * doesn't leave a gap. The foreground compose below draws on top,
     * so packs that carry their own water frames still mask this. */
    if (gFgBackdropThread.isRunning) {
        if (gFgBackdropThread.timer == 0) {
            gFgBackdropThread.timer = gFgBackdropThread.delay;
            islandAnimate(&gFgBackdropThread);
        } else {
            islandRedrawWave(&gFgBackdropThread);
            gFgBackdropThread.timer--;
        }
    }
    FG_CACHE_CHECK("fg-prepare-after-waves");

    if (perfDetail)
        perfTick = ps1PerfTick();
    fgRuntimeComposeEntryToBackground(&gFgRuntime.stagedEntry,
                                      gFgRuntime.prefetchFrameBuffer);
    FG_CACHE_CHECK("fg-prepare-after-compose");
    if (perfDetail)
        ps1PerfMarkRenderPhase(PS1_PERF_RENDER_COMPOSE,
                               ps1PerfElapsedVBlanks(perfTick));

    elapsedVBlanks = (uint16)ps1PerfElapsedVBlanks(prepTick);
    *outElapsedVBlanks = elapsedVBlanks;
    if (perfDetail)
        ps1PerfEndPipeline(PS1_PERF_PIPE_PREPARE, elapsedVBlanks);

    gFgRuntime.preparedFrameValid = 1;
    gFgRuntime.preparedFrameIndex = gFgRuntime.stagedFrameIndex;
    if (ps1PerfEnabled)
        ps1PerfMarkScheduler(PS1_PERF_SCHED_PREPARED_READY, 0);
    return 1;
}

static int fgRuntimePresentPreparedFrame(int perfDetail)
{
    uint32 perfRenderTick = 0;
    uint32 perfTick = 0;
    uint16 preparedFrameIndex;

    preparedFrameIndex = gFgRuntime.preparedFrameIndex;
    if (ps1PerfEnabled)
        ps1PerfMarkRenderedLoop();
    if (perfDetail)
        ps1PerfBeginPipeline(PS1_PERF_PIPE_PREPARED_PRESENT);
    if (perfDetail)
        perfRenderTick = ps1PerfTick();

    if (perfDetail)
        perfTick = ps1PerfTick();
    VSync(0);
    if (perfDetail)
        ps1PerfMarkRenderPhase(PS1_PERF_RENDER_PRESENT_WAIT,
                               ps1PerfElapsedVBlanks(perfTick));

    foregroundPilotRuntimeAdvance();
    FG_CACHE_CHECK("fg-present-after-advance");
    if (!gFgRuntime.active || gFgRuntime.frameIndex != preparedFrameIndex) {
        if (ps1PerfEnabled)
            ps1PerfMarkTripwire();
        gFgRuntime.preparedFrameValid = 0;
        if (perfDetail)
            ps1PerfEndPipeline(PS1_PERF_PIPE_PREPARED_PRESENT,
                               ps1PerfElapsedVBlanks(perfRenderTick));
        return 1;
    }

    if (perfDetail)
        perfTick = ps1PerfTick();
    grDrawBackground();
    if (perfDetail)
        ps1PerfMarkRenderPhase(PS1_PERF_RENDER_UPLOAD,
                               ps1PerfElapsedVBlanks(perfTick));

    if (perfDetail)
        perfTick = ps1PerfTick();
    eventsWaitTick(0);
    if (perfDetail)
        ps1PerfMarkRenderPhase(PS1_PERF_RENDER_EVENT_WAIT,
                               ps1PerfElapsedVBlanks(perfTick));

    if (perfDetail) {
        uint16 renderElapsed = ps1PerfElapsedVBlanks(perfRenderTick);
        ps1PerfMarkRenderTotal(renderElapsed);
        ps1PerfEndPipeline(PS1_PERF_PIPE_PREPARED_PRESENT, renderElapsed);
    }
    gFgRuntime.preparedFrameValid = 0;
    if (ps1PerfEnabled)
        ps1PerfMarkScheduler(PS1_PERF_SCHED_PREPARED_USED, 0);
    fgRuntimeMarkFrameRendered();
    return 1;
}

static int fgRuntimeComputeDrawBounds(sint16 *outX, sint16 *outY,
                                      uint16 *outW, uint16 *outH)
{
    int haveBounds = 0;
    int minX = 0;
    int minY = 0;
    int maxX = 0;
    int maxY = 0;
    uint16 i;

    if (outX == NULL || outY == NULL || outW == NULL || outH == NULL)
        return 0;

    for (i = 0; i < gFgRuntime.entryTable.count; i++) {
        const struct TFgPilotEntry *entry = &gFgRuntime.entryTable.entries[i];
        int x;
        int y;
        int endX;
        int endY;

        if (entry->width == 0 || entry->height == 0 || entry->dataSize == 0)
            continue;

        x = entry->x;
        y = entry->y;
        endX = x + (int)entry->width;
        endY = y + (int)entry->height;

        if (!haveBounds) {
            minX = x;
            minY = y;
            maxX = endX;
            maxY = endY;
            haveBounds = 1;
        } else {
            if (x < minX) minX = x;
            if (y < minY) minY = y;
            if (endX > maxX) maxX = endX;
            if (endY > maxY) maxY = endY;
        }
    }

    if (!haveBounds || maxX <= minX || maxY <= minY)
        return 0;

    *outX = (sint16)minX;
    *outY = (sint16)minY;
    *outW = (uint16)(maxX - minX);
    *outH = (uint16)(maxY - minY);
    return 1;
}
