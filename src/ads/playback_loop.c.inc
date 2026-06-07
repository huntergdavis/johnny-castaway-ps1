void adsPlay(char *adsName, uint16 adsTag)
{
    uint32 offset;
    uint8  *data;
    uint32 dataSize;
    char stableAdsName[16];
    const char *adsNameRef = adsStabilizeName(adsName, stableAdsName, sizeof(stableAdsName));

#ifdef PS1_BUILD
    /* Reset launch-state before any early-return path so story playback can
     * tell the difference between "scene failed to start" and "previous scene
     * launched successfully". */
    ps1AdsLastPlayLaunched = 0;
#endif

    struct TAdsResource *adsResource = findAdsResource((char *)adsNameRef);

#ifdef PS1_BUILD
    adsSetCurrentScene((char *)adsNameRef, adsTag);
#endif

#ifdef PS1_BUILD
    if (adsResource == NULL) {
        return;  /* Resource not found - skip scene silently */
    }
#endif

    debugMsg("\n\n========== Playing ADS: %s:%d ==========\n", adsResource->resName, adsTag);

    /* ADS lazy loading: Load ADS data on demand from extracted file if not already loaded */
    if (adsResource->uncompressedData == NULL) {
#ifdef PS1_BUILD
        /* PS1: Load from pre-extracted ADS file on CD */
        ps1_loadAdsData(adsResource);
        if (adsResource->uncompressedData == NULL) {
            return;  /* ADS data load failed - skip scene */
        }
#else
        char extractedPath[512];
        snprintf(extractedPath, sizeof(extractedPath), "extracted/ads/%s",
                 adsResource->resName);

        FILE *f = fopen(extractedPath, "rb");
        if (f) {
            adsResource->uncompressedData = safe_malloc(adsResource->uncompressedSize);
            if (fread(adsResource->uncompressedData, 1, adsResource->uncompressedSize, f) !=
                adsResource->uncompressedSize) {
                fatalError("Failed to load ADS data from extracted file");
            }
            fclose(f);
            if (debugMode) {
                printf("Loaded ADS data for %s from disk (%u bytes)\n",
                       adsResource->resName, adsResource->uncompressedSize);
            }
        } else {
            fatalError("ADS data not loaded and extracted file not found - cannot load %s", adsNameRef);
        }
#endif
    }

    /* Pin ADS resource to prevent eviction while in use */
    pinResource(adsResource, adsResource->uncompressedSize, "ADS");

    /* Check memory budget and potentially evict unused resources */
    checkMemoryBudget();

    data = adsResource->uncompressedData;
    dataSize = adsResource->uncompressedSize;

    for (int i=0; i < adsResource->numRes; i++) {
        uint16 slotId = adsResource->res[i].id;
        if (!adsIsValidTtmSlot(slotId)) {
            unpinResource(adsResource, "ADS");
            return;
        }
        ttmResetSlot(&ttmSlots[slotId]);
        ttmLoadTtm(&ttmSlots[slotId], adsResource->res[i].name);
    }

#ifdef PS1_BUILD
    /* If any TTM failed to load, skip this scene gracefully */
    {
        int loadOk = 1;
        for (int i=0; i < adsResource->numRes; i++) {
            if (!adsIsValidTtmSlot(adsResource->res[i].id)) {
                loadOk = 0;
                break;
            }
            if (ttmSlots[adsResource->res[i].id].data == NULL) {
                loadOk = 0;
                break;
            }
        }
        if (!loadOk) {
            for (int i=0; i < adsResource->numRes; i++) {
                if (adsIsValidTtmSlot(adsResource->res[i].id))
                    ttmResetSlot(&ttmSlots[adsResource->res[i].id]);
            }
            unpinResource(adsResource, "ADS");
            return;
        }
    }
#endif

    adsLoad(data, dataSize, adsResource->numTags, adsTag, &offset);

    adsStopRequested = 0;
    numAdsPlayCounts = 0;
    numAdsPendingStops = 0;
    grUpdateDelay = 0;

    // Play the first ADS chunk of the sequence
    adsPlayChunk(data, dataSize, offset);

#ifdef PS1_BUILD
    /* Recovery: some ADS control paths can parse without launching a scene
     * (no ADD_SCENE emitted), which leaves PS1 on a static background.
     * This happens when IF_LASTPLAYED conditions don't match because no
     * prior scene context exists (cold boot).  Try each bookmarked chunk
     * in random order until one actually spawns a thread. */
    if (numThreads == 0 && !adsStopRequested && numAdsChunks > 0) {
        int start = rand() % numAdsChunks;
        for (int attempt = 0; attempt < numAdsChunks && numThreads == 0; attempt++) {
            int pick = (start + attempt) % numAdsChunks;
            adsPlayChunk(data, dataSize, adsChunks[pick].offset);
        }
    }
#endif
    if (numThreads > 0) ps1AdsLastPlayLaunched = 1;

    // Main ADS loop
#ifdef PS1_BUILD
    grEnsureCleanBgTiles();
#endif
    while (numThreads) {
        /* Self-heal thread-count desync: trust actual running threads over numThreads counter. */
        {
            int activeRunning = 0;
            int firstIdx = -1;
            int runningCount = 0;
            int terminatedCount = 0;
            for (int i = 0; i < MAX_TTM_THREADS; i++) {
                if (ttmThreads[i].isRunning == ADS_THREAD_RUNNING ||
                    ttmThreads[i].isRunning == ADS_THREAD_TERMINATED) {
                    activeRunning++;
                    if (firstIdx < 0) firstIdx = i;
                    if (ttmThreads[i].isRunning == ADS_THREAD_RUNNING) runningCount++;
                    else terminatedCount++;
                }
            }
            if (activeRunning == 0) {
                numThreads = 0;
                break;
            }
            numThreads = activeRunning;
#ifdef PS1_BUILD
            if (grPs1TelemetryEnabled) {
                ps1AdsDbgActiveThreads = (uint16)activeRunning;
                ps1AdsDbgRunningThreads = (uint16)runningCount;
                ps1AdsDbgTerminatedThreads = (uint16)terminatedCount;
                if (firstIdx >= 0) {
                    ps1AdsDbgSceneSlot = ttmThreads[firstIdx].sceneSlot;
                    ps1AdsDbgSceneTag  = ttmThreads[firstIdx].sceneTag;
                    ps1AdsDbgReplayCount = ttmThreads[firstIdx].numDrawnSprites;
                } else {
                    ps1AdsDbgSceneSlot = 0;
                    ps1AdsDbgSceneTag  = 0;
                    ps1AdsDbgReplayCount = 0;
                }
                ps1AdsDbgReplayTryFrame = 0;
                ps1AdsDbgReplayDrawFrame = 0;
                ps1AdsDbgMergeCarryFrame = 0;
                ps1AdsDbgNoDrawThreadsFrame = 0;
                ps1AdsDbgPlayedThreadsFrame = 0;
                ps1AdsDbgRecordedSpritesFrame = 0;
            }
#endif
        }

#ifdef PS1_BUILD
        grBeginFrame();
        grRestoreBgTiles();
#endif

        if (ttmBackgroundThread.isRunning && ttmBackgroundThread.timer == 0) {
            debugMsg("    ------> Animate bg");
            ttmBackgroundThread.timer = ttmBackgroundThread.delay;
            islandAnimate(&ttmBackgroundThread);
        }
#ifdef PS1_BUILD
        else if (ttmBackgroundThread.isRunning) {
            islandRedrawWave(&ttmBackgroundThread);
        }
#endif

        for (int i=0; i < MAX_TTM_THREADS; i++) {

            // Call ttmPlay() for each thread which timer reaches 0
            if (ttmThreads[i].isRunning == ADS_THREAD_RUNNING && ttmThreads[i].timer == 0) {
                debugMsg("    ------> Thread #%d", i);
                ttmThreads[i].timer = ttmThreads[i].delay;
#ifdef PS1_BUILD
                uint8 prevCount = adsCaptureReplayRecords(&ttmThreads[i]);
                if (grPs1TelemetryEnabled)
                    adsDbgAddU16(&ps1AdsDbgReplayTryFrame, prevCount);
                ttmThreads[i].numDrawnSprites = 0;
                grDx = 0;
                grDy = 0;
                grCurrentThread = &ttmThreads[i];
                if (grPs1TelemetryEnabled)
                    adsDbgAddU16(&ps1AdsDbgPlayedThreadsFrame, 1);
                adsDiagTtmDispatch("before", i, &ttmThreads[i]);
#endif
                ttmPlay(&ttmThreads[i]);
#ifdef PS1_BUILD
                adsDiagTtmDispatch("after", i, &ttmThreads[i]);
                grCurrentThread = NULL;
                if (ttmThreads[i].isRunning == ADS_THREAD_TERMINATED) {
                    ttmThreads[i].timer = 0;
                } else {
                    if (!adsUseRestorePilotReplayPolicy()) {
                        uint8 carried = adsMergeReplayByProximity(&ttmThreads[i],
                                                                  gPrevReplayScratch,
                                                                  prevCount);
                        if (grPs1TelemetryEnabled)
                            adsDbgAddU16(&ps1AdsDbgMergeCarryFrame, carried);
                    }
                    if (!adsUseRestorePilotReplayPolicy()) {
                        adsRecoverMissingActor(&ttmThreads[i], gPrevReplayScratch, prevCount);
                    }
                }
                if (grPs1TelemetryEnabled) {
                    if (ttmThreads[i].numDrawnSprites == 0)
                        adsDbgAddU16(&ps1AdsDbgNoDrawThreadsFrame, 1);
                    adsDbgAddU16(&ps1AdsDbgRecordedSpritesFrame, ttmThreads[i].numDrawnSprites);
                }
#endif
            }
#ifdef PS1_BUILD
            else if (ttmThreads[i].isRunning == ADS_THREAD_RUNNING ||
                     ttmThreads[i].isRunning == ADS_THREAD_TERMINATED) {
                /* Replay via GPU path */
                for (int j = 0; j < ttmThreads[i].numDrawnSprites; j++) {
                    if (grPs1TelemetryEnabled) {
                        adsDbgAddU16(&ps1AdsDbgReplayTryFrame, 1);
                        adsDbgAddU16(&ps1AdsDbgReplayDrawFrame, 1);
                    }
                    grReplaySprite(&ttmThreads[i].drawnSprites[j]);
                }
            }
#endif
        }

#ifndef PS1_BUILD
        if (debugMode) {

            debugMsg("\n  +------ THREADS: %d -------", numThreads);

            if (ttmBackgroundThread.isRunning) {
                    debugMsg("  |");
                    debugMsg("  | #bg:");
                    debugMsg("  |   delay........... %d" , ttmBackgroundThread.delay    );
                    debugMsg("  |   timer........... %d" , ttmBackgroundThread.timer    );
            }

            for (int i=0; i < MAX_TTM_THREADS; i++) {
                if (ttmThreads[i].isRunning) {
                    debugMsg("  |");
                    debugMsg("  | #%d: (%d,%d)", i, ttmThreads[i].sceneSlot, ttmThreads[i].sceneTag);
                    debugMsg("  |   sceneTimer...... %d" , ttmThreads[i].sceneTimer     );
                    debugMsg("  |   isRunning....... %d" , ttmThreads[i].isRunning      );
                    debugMsg("  |   offset.......... %ld", ttmThreads[i].ip             );
                    debugMsg("  |   nextGotoOffset.. %ld", ttmThreads[i].nextGotoOffset );
                    debugMsg("  |   delay........... %d" , ttmThreads[i].delay          );
                    debugMsg("  |   timer........... %d" , ttmThreads[i].timer          );
                }
            }

            debugMsg("  +-------------------------\n");
        }
#endif

        // Refresh display
#ifdef PS1_BUILD
        grUpdateDisplay(&ttmBackgroundThread, ttmThreads, &ttmHolidayThread);
#else
        grUpdateDisplay(&ttmBackgroundThread, ttmThreads, &ttmHolidayThread);
#endif
#ifdef PS1_BUILD
        if (foregroundPilotRuntimeActive())
            foregroundPilotRuntimeAdvance();
#endif
        if (grCaptureSequenceComplete())
            break;

        // Determine min timer and collect active indices in a single pass
        {
            uint16 mini = 300;
            int activeIdx[MAX_TTM_THREADS];
            int numActive = 0;

            if (ttmBackgroundThread.isRunning)
                mini = ttmBackgroundThread.timer;

            for (int i = 0; i < MAX_TTM_THREADS; i++) {
                if (ttmThreads[i].isRunning == ADS_THREAD_RUNNING) {
                    activeIdx[numActive++] = i;
                    if (ttmThreads[i].delay < mini)
                        mini = ttmThreads[i].delay;
                    if (ttmThreads[i].timer < mini)
                        mini = ttmThreads[i].timer;
                }
            }

            /* Prevent zero-tick loops from starving progression and display updates. */
            if (mini == 0) mini = 1;

            // Decrease all timers by the shortest one, and wait that amount of time
#ifdef PS1_BUILD
            if (grPs1TelemetryEnabled)
                ps1AdsDbgMini = mini;
#endif
            if (ttmBackgroundThread.timer > mini)
                ttmBackgroundThread.timer -= mini;
            else
                ttmBackgroundThread.timer = 0;

            for (int j = 0; j < numActive; j++) {
                int idx = activeIdx[j];
                if (ttmThreads[idx].timer > mini)
                    ttmThreads[idx].timer -= mini;
                else
                    ttmThreads[idx].timer = 0;
            }

            debugMsg(" ******* WAIT: %d ticks *******\n", mini);
            grUpdateDelay = mini;
        }

        // Various threads processes
        for (int i=0; i < MAX_TTM_THREADS; i++) {

            if (ttmThreads[i].isRunning == ADS_THREAD_RUNNING && ttmThreads[i].timer == 0) {

                // Process jumps
                if (ttmThreads[i].nextGotoOffset) {
                    if (ttmThreads[i].nextGotoOffset < ttmThreads[i].ttmSlot->dataSize) {
                        ttmThreads[i].ip = ttmThreads[i].nextGotoOffset;
                        ttmThreads[i].nextGotoOffset = 0;
                    } else {
                        ttmThreads[i].isRunning = ADS_THREAD_TERMINATED;
                        ttmThreads[i].nextGotoOffset = 0;
                    }
                }

                // Managing the timer which was indicated in ADD_SCENE arg3 (neg. value)
                if (ttmThreads[i].sceneTimer > 0) {
                    ttmThreads[i].sceneTimer -= ttmThreads[i].delay;
                    if (ttmThreads[i].sceneTimer <= 0)
                        ttmThreads[i].isRunning = ADS_THREAD_TERMINATED;
                }
            }

            /* Free terminated threads immediately to avoid stale replay/timer side effects. */
            if (ttmThreads[i].isRunning == ADS_THREAD_TERMINATED) {

                // Managing the numPlays which was indicated in ADD_SCENE arg3 (postive value)
                if (ttmThreads[i].sceneIterations) {
                    ttmThreads[i].sceneIterations--;
                    ttmThreads[i].isRunning = ADS_THREAD_RUNNING;
                    ttmThreads[i].sceneEpoch++;
                    ttmThreads[i].numDrawnSprites = 0;
                    ttmThreads[i].lastActorReplayValid = 0;
                    ttmThreads[i].ip = ttmFindTag(&ttmSlots[ttmThreads[i].sceneSlot], ttmThreads[i].sceneTag);
                    if (ttmThreads[i].sceneTag != 0 && ttmThreads[i].ip == 0) {
                        adsStopScene(i);
                    } else {
                        ttmThreads[i].timer = 0;
                    }
                }

                // Is there one (or more) IF_LASTPLAYED matching the terminated thread ?
                else {
                    uint16 endedSlot = ttmThreads[i].sceneSlot;
                    uint16 endedTag = ttmThreads[i].sceneTag;
#ifdef PS1_BUILD
                    adsCaptureHandoffReplay(&ttmThreads[i]);
#endif
                    adsStopScene(i);
                    if (!adsStopRequested)
                        adsPlayTriggeredChunks(data, dataSize, endedSlot, endedTag);
#ifdef PS1_BUILD
                    /* If handoff wasn't consumed (no new thread started),
                     * inject actor into a surviving thread to prevent vanish. */
                    if (!adsUseRestorePilotReplayPolicy() &&
                        gHandoffReplayValid && gHandoffReplayCount > 0) {
                        for (int k = 0; k < MAX_TTM_THREADS; k++) {
                            if (ttmThreads[k].isRunning == ADS_THREAD_RUNNING &&
                                ttmThreads[k].numDrawnSprites < MAX_DRAWN_SPRITES) {
                                gHandoffReplay[0].sceneEpoch = ttmThreads[k].sceneEpoch;
                                ttmThreads[k].drawnSprites[ttmThreads[k].numDrawnSprites++] = gHandoffReplay[0];
                                ttmThreads[k].lastActorReplay = gHandoffReplay[0];
                                ttmThreads[k].lastActorReplayValid = 1;
                                gHandoffReplayValid = 0;
                                gHandoffReplayCount = 0;
                                break;
                            }
                        }
                    }
#endif
                }
            }
        }

    }

    for (int i=0; i < MAX_TTM_SLOTS; i++)
        ttmResetSlot(&ttmSlots[i]);

    grRestoreZone(NULL, 0, 0, 0, 0);

    adsReleaseAds();

#ifdef PS1_BUILD
    foregroundPilotRuntimeEnd();
#endif

    /* Unpin ADS resource to allow LRU eviction */
    unpinResource(adsResource, "ADS");
}


void adsPlayBench()
{
    int numsLayers[] = { 1, 4, 8 };

    uint32 startTicks, counter;

    adsInit();

    for (int i=0; i < 8; i++) {
        ttmThreads[i].ttmSlot         = &ttmSlots[0];;
        ttmThreads[i].isRunning       = 1;
        ttmThreads[i].selectedBmpSlot = 0;
        ttmThreads[i].ttmLayer        = grNewLayer();
    }

    benchInit(ttmSlots);
    grUpdateDelay = 0;

    for (int j=0; j < 3; j++) {

        int numLayers = numsLayers[j];

        for (int i=0; i < MAX_TTM_THREADS; i++)
            ttmThreads[i].isRunning = (i<numLayers ? 1 : 0);

        startTicks = SDL_GetTicks();
        counter = 0;

        while ((SDL_GetTicks() - startTicks) <= 3000) {

            for (int i=0; i < numLayers; i++)
                benchPlay(&ttmThreads[i], i);

            grUpdateDisplay(NULL, ttmThreads, NULL);

            counter++;
        }

        printf(" %d-layers test --> %d fps\n", numLayers, counter/3);
    }

    for (int i=0; i < 8; i++)
        adsStopScene(i);

    ttmResetSlot(&ttmSlots[0]);
}
