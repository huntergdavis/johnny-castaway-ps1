static const char *adsStabilizeName(const char *adsName, char *buffer, size_t bufferSize)
{
    size_t i;

    if (buffer == NULL || bufferSize == 0)
        return adsName;

    if (adsName == NULL) {
        buffer[0] = '\0';
        return buffer;
    }

    for (i = 0; i + 1 < bufferSize && adsName[i] != '\0'; i++)
        buffer[i] = adsName[i];
    buffer[i] = '\0';
    return buffer;
}

static int adsIsValidTtmSlot(uint16 slot)
{
    return slot < MAX_TTM_SLOTS;
}

static void adsStopScene(int sceneNo);

#define ADS_THREAD_RUNNING 1
#define ADS_THREAD_TERMINATED 2

#ifdef PS1_BUILD
static const struct TPs1RestorePilot *cachedPilot = NULL;
static char cachedPilotAdsName[16] = {0};
static uint16 cachedPilotAdsTag = 0xFFFF;
extern uint16 ps1AdsDbgActiveThreads;
extern uint16 ps1AdsDbgMini;
extern uint16 ps1AdsDbgRunningThreads;
extern uint16 ps1AdsDbgTerminatedThreads;
extern uint16 ps1AdsDbgSceneSlot;
extern uint16 ps1AdsDbgSceneTag;
extern uint16 ps1AdsDbgReplayCount;
extern uint16 ps1AdsDbgReplayTryFrame;
extern uint16 ps1AdsDbgReplayDrawFrame;
extern uint16 ps1AdsDbgMergeCarryFrame;
extern uint16 ps1AdsDbgNoDrawThreadsFrame;
extern uint16 ps1AdsDbgPlayedThreadsFrame;
extern uint16 ps1AdsDbgRecordedSpritesFrame;
extern uint16 ps1AdsDbgLastStopThread;
extern uint16 ps1AdsDbgLastStopSceneSig;
extern uint16 ps1AdsDbgLastReapThread;
extern uint16 ps1AdsDbgLastReapSceneSig;
extern uint16 ps1AdsDbgLastAddThread;
extern uint16 ps1AdsDbgLastAddSceneSig;
#endif

static ADS_HOST_UNUSED void adsSetCurrentScene(char *adsName, uint16 adsTag)
{
    int i;

    for (i = 0; i < (int)(sizeof(ps1AdsCurrentName) - 1) && adsName != NULL && adsName[i] != '\0'; i++)
        ps1AdsCurrentName[i] = adsName[i];
    ps1AdsCurrentName[i] = '\0';
    ps1AdsCurrentTag = adsTag;

#ifdef PS1_BUILD
    /* Invalidate pilot cache when scene changes */
    cachedPilotAdsName[0] = '\0';
#endif
}

static ADS_HOST_UNUSED int adsStringEquals(const char *a, const char *b)
{
    if (!a || !b) return 0;
    return strcmp(a, b) == 0;
}

#ifdef PS1_BUILD
static int adsPilotContainsAdsTag(const struct TPs1RestorePilot *pilot, uint16 adsTag)
{
    int i;
    if (pilot == NULL)
        return 0;
    for (i = 0; i < pilot->adsTagCount; i++) {
        if (pilot->adsTags[i] == adsTag)
            return 1;
    }
    return 0;
}

static const struct TPs1RestorePilot *adsFindActiveRestorePilot(void)
{
    int i;

    /* Return cached result if name+tag haven't changed */
    if (cachedPilotAdsName[0] != '\0' &&
        cachedPilotAdsTag == ps1AdsCurrentTag &&
        adsStringEquals(cachedPilotAdsName, ps1AdsCurrentName))
        return cachedPilot;

    for (i = 0; i < PS1_RESTORE_PILOT_COUNT; i++) {
        const struct TPs1RestorePilot *pilot = &gPs1RestorePilots[i];
        if (!adsStringEquals(ps1AdsCurrentName, pilot->adsName))
            continue;
        if (adsPilotContainsAdsTag(pilot, ps1AdsCurrentTag)) {
            /* Cache the result */
            cachedPilot = pilot;
            cachedPilotAdsTag = ps1AdsCurrentTag;
            for (i = 0; i < 15 && ps1AdsCurrentName[i]; i++)
                cachedPilotAdsName[i] = ps1AdsCurrentName[i];
            cachedPilotAdsName[i] = '\0';
            return pilot;
        }
    }

    /* Cache the negative result too */
    cachedPilot = NULL;
    cachedPilotAdsTag = ps1AdsCurrentTag;
    for (i = 0; i < 15 && ps1AdsCurrentName[i]; i++)
        cachedPilotAdsName[i] = ps1AdsCurrentName[i];
    cachedPilotAdsName[i] = '\0';
    return NULL;
}

static ADS_HOST_UNUSED int adsUseRestorePilotReplayPolicy(void)
{
    return adsFindActiveRestorePilot() != NULL;
}

static ADS_HOST_UNUSED int adsUseReplayRecovery(void)
{
    return 0;
}
#else
static ADS_HOST_UNUSED int adsUseRestorePilotReplayPolicy(void)
{
    return 0;
}

static ADS_HOST_UNUSED int adsUseReplayRecovery(void)
{
    return 0;
}
#endif

#ifdef PS1_BUILD
static int adsDiagTrackCurrentTtmPlay(void)
{
    if (adsStringEquals(ps1AdsCurrentName, "BUILDING.ADS") &&
        ps1AdsCurrentTag == 1)
        return 1;

    return 0;
}

static uint16 adsDiagSceneSigFor(uint16 sceneSlot, uint16 sceneTag)
{
    return (uint16)((((sceneSlot & 0x7) << 3) | (sceneTag & 0x7)) & 0x3F);
}

static ADS_HOST_UNUSED void adsDiagNoteStop(int threadIndex, uint16 sceneSlot, uint16 sceneTag)
{
    if (!adsDiagTrackCurrentTtmPlay())
        return;
    ps1AdsDbgLastStopThread = (uint16)(threadIndex & 0x3F);
    ps1AdsDbgLastStopSceneSig = adsDiagSceneSigFor(sceneSlot, sceneTag);
}

static ADS_HOST_UNUSED void adsDiagNoteReap(int threadIndex, uint16 sceneSlot, uint16 sceneTag)
{
    if (!adsDiagTrackCurrentTtmPlay())
        return;
    ps1AdsDbgLastReapThread = (uint16)(threadIndex & 0x3F);
    ps1AdsDbgLastReapSceneSig = adsDiagSceneSigFor(sceneSlot, sceneTag);
}

static void adsDiagNoteAdd(int threadIndex, uint16 sceneSlot, uint16 sceneTag)
{
    if (!adsDiagTrackCurrentTtmPlay())
        return;
    ps1AdsDbgLastAddThread = (uint16)(threadIndex & 0x3F);
    ps1AdsDbgLastAddSceneSig = adsDiagSceneSigFor(sceneSlot, sceneTag);
}

static void adsDiagStoreDispatch(int phase, int threadIndex, struct TTtmThread *ttmThread)
{
    uint32 ip = 0;
    uint32 next = 0;
    uint16 sceneSig = 0;

    if (!adsDiagTrackCurrentTtmPlay() || ttmThread == NULL)
        return;

    ip = ttmThread->ip & 0x3FFFFFFFul;
    next = ttmThread->nextGotoOffset & 0x3FFFFu;
    sceneSig = (uint16)((((ttmThread->sceneSlot & 0x7) << 3) |
                         (ttmThread->sceneTag & 0x7)) & 0x3F);

    /* Reuse ADS telemetry rows as a persistent last-dispatch dump.
     * active_threads is a marker:
     *   61 => before ttmPlay()
     *   62 => after ttmPlay() */
    ps1AdsDbgActiveThreads = (uint16)(phase ? 62 : 61);
    ps1AdsDbgMini = (uint16)(threadIndex & 0x3F);
    ps1AdsDbgSceneSlot = ttmThread->sceneSlot;
    ps1AdsDbgSceneTag = ttmThread->sceneTag;
    ps1AdsDbgReplayCount = (uint16)(ttmThread->selectedBmpSlot & 0x3F);
    ps1AdsDbgRunningThreads = (uint16)(ttmThread->currentRegionId & 0x3F);
    ps1AdsDbgReplayTryFrame = (uint16)(ttmThread->timer & 0x3F);
    ps1AdsDbgReplayDrawFrame = (uint16)(ip & 0x3F);
    ps1AdsDbgMergeCarryFrame = (uint16)((ip >> 6) & 0x3F);
    ps1AdsDbgNoDrawThreadsFrame = (uint16)((ip >> 12) & 0x3F);
    ps1AdsDbgPlayedThreadsFrame = (uint16)((ip >> 18) & 0x3F);
    ps1AdsDbgRecordedSpritesFrame = (uint16)((ip >> 24) & 0x3F);
    ps1AdsDbgTerminatedThreads = (uint16)(next & 0x3F);
    grPs1SetLastBmpTelemetry((uint16)((next >> 6) & 0x3F),
                             (uint16)((next >> 12) & 0x3F),
                             (uint16)(((ttmThread->isRunning & 0x0F) |
                                       ((phase & 0x01) << 4) |
                                       ((sceneSig & 0x03) << 5)) & 0x3F));
}

static ADS_HOST_UNUSED void adsDiagTtmDispatch(const char *phase, int threadIndex, struct TTtmThread *ttmThread)
{
    static int diagCount = 0;
    const char *ttmName = NULL;

    if (!adsDiagTrackCurrentTtmPlay())
        return;
    adsDiagStoreDispatch((phase && phase[0] == 'a') ? 1 : 0, threadIndex, ttmThread);
    if (diagCount >= 128)
        return;
    diagCount++;

    if (ttmThread != NULL && ttmThread->ttmSlot != NULL &&
        ttmThread->ttmSlot->ttmResource != NULL) {
        ttmName = ttmThread->ttmSlot->ttmResource->resName;
    }

    printf("ADS_DIAG %s idx=%d ads=%s:%u sceneSlot=%u sceneTag=%u ttm=%s ip=0x%08lX next=0x%08lX delay=%u timer=%u running=%d sceneTimer=%d sceneIterations=%d currentRegion=%u bmpSlot=%u drawn=%u\n",
           phase ? phase : "?",
           threadIndex,
           ps1AdsCurrentName,
           (unsigned int)ps1AdsCurrentTag,
           (unsigned int)(ttmThread ? ttmThread->sceneSlot : 0),
           (unsigned int)(ttmThread ? ttmThread->sceneTag : 0),
           ttmName ? ttmName : "?",
           (unsigned long)(ttmThread ? ttmThread->ip : 0),
           (unsigned long)(ttmThread ? ttmThread->nextGotoOffset : 0),
           (unsigned int)(ttmThread ? ttmThread->delay : 0),
           (unsigned int)(ttmThread ? ttmThread->timer : 0),
           ttmThread ? ttmThread->isRunning : -1,
           ttmThread ? ttmThread->sceneTimer : 0,
           ttmThread ? ttmThread->sceneIterations : 0,
           (unsigned int)(ttmThread ? ttmThread->currentRegionId : 0),
           (unsigned int)(ttmThread ? ttmThread->selectedBmpSlot : 0),
           (unsigned int)(ttmThread ? ttmThread->numDrawnSprites : 0));
}
#else
static ADS_HOST_UNUSED void adsDiagNoteStop(int threadIndex, uint16 sceneSlot, uint16 sceneTag)
{
    (void)threadIndex;
    (void)sceneSlot;
    (void)sceneTag;
}

static ADS_HOST_UNUSED void adsDiagNoteReap(int threadIndex, uint16 sceneSlot, uint16 sceneTag)
{
    (void)threadIndex;
    (void)sceneSlot;
    (void)sceneTag;
}

static ADS_HOST_UNUSED void adsDiagNoteAdd(int threadIndex, uint16 sceneSlot, uint16 sceneTag)
{
    (void)threadIndex;
    (void)sceneSlot;
    (void)sceneTag;
}

static ADS_HOST_UNUSED void adsDiagTtmDispatch(const char *phase, int threadIndex, struct TTtmThread *ttmThread)
{
    (void)phase;
    (void)threadIndex;
    (void)ttmThread;
}
#endif

#ifdef PS1_BUILD
static void adsPrimeRestorePilotResources(const struct TPs1RestorePilot *pilot)
{
    uint16 i;

    if (pilot == NULL)
        return;

    for (i = 0; i < pilot->scrCount; i++) {
        struct TScrResource *scrResource = findScrResource((char *)pilot->scrs[i]);
        if (scrResource != NULL && scrResource->uncompressedData == NULL)
            ps1_loadScrData(scrResource);
    }

    for (i = 0; i < pilot->sceneTtmCount; i++) {
        struct TTtmResource *ttmResource = findTtmResource((char *)pilot->sceneTtms[i]);
        if (ttmResource != NULL && ttmResource->uncompressedData == NULL)
            ps1_loadTtmData(ttmResource);
    }

    /* Warm Johnny's sprite sheet first when present. Several island scenes,
     * including FISHING 1, depend on JOHNWALK being available for the first
     * in-scene composed frames. */
    for (i = 0; i < pilot->bmpCount; i++) {
        if (!adsStringEquals(pilot->bmps[i], "JOHNWALK.BMP"))
            continue;

        {
            struct TBmpResource *bmpResource = findBmpResource((char *)pilot->bmps[i]);
            if (bmpResource != NULL && bmpResource->uncompressedData == NULL)
                ps1_loadBmpData(bmpResource);
        }
        break;
    }

    for (i = 0; i < pilot->bmpCount; i++) {
        if (adsStringEquals(pilot->bmps[i], "JOHNWALK.BMP")) {
            continue;
        }

        {
            struct TBmpResource *bmpResource = findBmpResource((char *)pilot->bmps[i]);
            if (bmpResource != NULL && bmpResource->uncompressedData == NULL)
                ps1_loadBmpData(bmpResource);
        }
    }
}
#endif
