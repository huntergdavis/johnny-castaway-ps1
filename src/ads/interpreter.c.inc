void adsInit()    // Initialize slots and threads for TTM scripts.
{
    /* Allocate TTM slots/threads dynamically to reduce BSS size */
    if (ttmSlots == NULL) {
        ttmSlots = (struct TTtmSlot*)malloc(MAX_TTM_SLOTS * sizeof(struct TTtmSlot));
        if (!ttmSlots) {
            if (debugMode) {
                printf("ERROR: Failed to allocate TTM slots\n");
            }
            return;
        }
    }

    if (ttmThreads == NULL) {
        ttmThreads = (struct TTtmThread*)malloc(MAX_TTM_THREADS * sizeof(struct TTtmThread));
        if (!ttmThreads) {
            if (debugMode) {
                printf("ERROR: Failed to allocate TTM threads\n");
            }
            return;
        }
    }

    for (int i=0; i < MAX_TTM_SLOTS; i++)
        ttmInitSlot(&ttmSlots[i]);

    for (int i=0; i < MAX_TTM_THREADS; i++) {
        memset(&ttmThreads[i], 0, sizeof(ttmThreads[i]));
    }
#ifdef PS1_BUILD
    ps1AdsDbgActiveThreads = 0;
    ps1AdsDbgRunningThreads = 0;
    ps1AdsDbgTerminatedThreads = 0;
    ps1AdsDbgReplayCount = 0;
    ps1AdsDbgReplayTryFrame = 0;
    ps1AdsDbgReplayDrawFrame = 0;
    ps1AdsDbgMergeCarryFrame = 0;
    ps1AdsDbgNoDrawThreadsFrame = 0;
    ps1AdsDbgPlayedThreadsFrame = 0;
    ps1AdsDbgRecordedSpritesFrame = 0;
    gHandoffReplayCount = 0;
    gHandoffReplayValid = 0;
#endif

    grUpdateDelay = 0;
    ttmBackgroundThread.isRunning = 0;
    ttmHolidayThread.isRunning    = 0;
    numThreads = 0;
    adsStopRequested = 0;
    numAdsPendingStops = 0;
}


void adsPlaySingleTtm(char *ttmName, uint16 startTag)  // Play one TTM scene from `startTag`.
{
    adsInit();
    ttmLoadTtm(ttmSlots, ttmName);
#ifdef PS1_BUILD
    if (ttmSlots[0].data == NULL) return;
#endif
    adsAddScene(0,0,0);
    ttmThreads[0].ip = (startTag ? ttmFindTag(ttmSlots, startTag) : 0);

    while (ttmThreads[0].ip < ttmSlots[0].dataSize) {
#ifdef PS1_BUILD
        /* Reset GPU frame state and restore clean background */
        grBeginFrame();
        grRestoreBgTiles();
        grCurrentThread = &ttmThreads[0];
#endif
        ttmPlay(ttmThreads);
#ifdef PS1_BUILD
        grCurrentThread = NULL;
#endif
        ttmThreads[0].isRunning = 1;
        grUpdateDisplay(NULL, ttmThreads, NULL);
        if (grCaptureSequenceComplete())
            break;
        grUpdateDelay = ttmThreads[0].delay;
    }

    adsStopScene(0);
    ttmResetSlot(&ttmSlots[0]);
}


static void adsPlayChunk(uint8 *data, uint32 dataSize, uint32 offset)
{
 #ifdef PS1_BUILD
    enum { ADS_DEFERRED_OP_CAP = 16 };
    struct TAdsDeferredOp {
        uint16 opcode;
        uint16 slot;
        uint16 tag;
        uint16 arg3;
    };
    struct TAdsDeferredOp deferredOps[ADS_DEFERRED_OP_CAP];
    int deferredCount = 0;
#endif
    uint16 opcode;
    uint16 args[10];
    int inRandBlock          = 0;
    int inOrBlock            = 0;
    int inSkipBlock          = 0;
    int inIfLastplayedLocal  = 0;
    int continueLoop         = 1;


    while (continueLoop && offset < dataSize) {

        opcode = peekUint16(data, &offset);

        switch (opcode) {

            case 0x1070:
                // Inside an IF_LASTPLAYED chunk, local IF_LASTPLAYED
                // which overrides the global IF_LASTPLAYEDs.
                if (!adsPeekWordArgs(data, dataSize, &offset, args, 2))
                    return;
                debugMsg("IF_LASTPLAYED_LOCAL");
                inIfLastplayedLocal = 1;
                adsStoreLocalChunk(args[0], args[1], offset);
                break;

            case 0x1330:
                // Always just before a call to ADD_SCENE with same (ttm,tag)
                // references tags which init commands : LOAD_IMAGE LOAD_SCREEN etc.
                //   - one exception: FISHING.ADS tag 3
                //   - seems to be a synonym of "IF_NOT_RUNNING"
                //   - if so, our implementation works fine anyway by ignoring this one...
                if (!adsPeekWordArgs(data, dataSize, &offset, args, 2))
                    return;
                debugMsg("IF_UNKNOWN_1 %d %d", args[0], args[1]);
                break;

            case 0x1350:
                if (!adsPeekWordArgs(data, dataSize, &offset, args, 2))
                    return;
                debugMsg("IF_LASTPLAYED %d %d", args[0], args[1]);

                if (!inOrBlock)
                    continueLoop = 0;

                inOrBlock = 0;

                break;

            case 0x1360:
                if (!adsPeekWordArgs(data, dataSize, &offset, args, 2))
                    return;
                debugMsg("IF_NOT_RUNNING %d %d", args[0], args[1]);
                if (isSceneRunning(args[0], args[1]))
                    inSkipBlock = 1;
                break;

            case 0x1370:
                if (!adsPeekWordArgs(data, dataSize, &offset, args, 2))
                    return;
                debugMsg("IF_IS_RUNNING %d %d", args[0], args[1]);
                inSkipBlock = !isSceneRunning(args[0], args[1]);
                break;

            case 0x1420:
                debugMsg("AND");
                break;

            case 0x1430:
                debugMsg("OR");
                inOrBlock = 1;
                break;

            case 0x1510:
                /* PLAY_SCENE acts as the closing delimiter for several ADS
                 * statement blocks. Keep the original control-flow model:
                 * close skipped blocks, otherwise end this interpreter pass. */
                debugMsg("PLAY_SCENE");
                if (inSkipBlock)
                    inSkipBlock = 0;
                else
                    continueLoop = 0;
                break;

            case 0x1520:
                // Only in ACTIVITY.ADS tag 7, after IF_LASTPLAYED_LOCAL
                if (!adsPeekWordArgs(data, dataSize, &offset, args, 5))
                    return;
                debugMsg("ADD_SCENE_LOCAL");

                if (inIfLastplayedLocal) {
                    // First pass : the scene was queued by IF_LASTPLAYED_LOCAL,
                    // nothing more to do for now
                    inIfLastplayedLocal = 0;
                }
                else {
                    // Second pass (we were called directly from the scheduler)
                    // --> we launch the execution of the scene
#ifdef PS1_BUILD
                    if (deferredCount < ADS_DEFERRED_OP_CAP) {
                        deferredOps[deferredCount].opcode = opcode;
                        deferredOps[deferredCount].slot = args[1];
                        deferredOps[deferredCount].tag = args[2];
                        deferredOps[deferredCount].arg3 = args[3];
                        deferredCount++;
                    } else
#endif
                    adsAddScene(args[1],args[2],args[3]);
                }

                break;

            case 0x2005:
                if (!adsPeekWordArgs(data, dataSize, &offset, args, 4))
                    return;
                debugMsg("ADD_SCENE %d %d %d %d", args[0], args[1], args[2], args[3]);

                if (!inSkipBlock) {
                    if (inRandBlock)
                        adsRandomAddScene(args[0],args[1],args[2], args[3]);
                    else {
#ifdef PS1_BUILD
                        if (deferredCount < ADS_DEFERRED_OP_CAP) {
                            deferredOps[deferredCount].opcode = opcode;
                            deferredOps[deferredCount].slot = args[0];
                            deferredOps[deferredCount].tag = args[1];
                            deferredOps[deferredCount].arg3 = args[2];
                            deferredCount++;
                        } else
#endif
                        adsAddScene(args[0],args[1],args[2]);
                    }
                }

                break;

            case 0x2010:
                if (!adsPeekWordArgs(data, dataSize, &offset, args, 3))
                    return;
                debugMsg("STOP_SCENE %d %d %d", args[0], args[1], args[2]);

                if (!inSkipBlock) {
                    if (inRandBlock)
                        adsRandomStopSceneByTtmTag(args[0], args[1], args[2]);
                    else {
#ifdef PS1_BUILD
                        if (deferredCount < ADS_DEFERRED_OP_CAP) {
                            deferredOps[deferredCount].opcode = opcode;
                            deferredOps[deferredCount].slot = args[0];
                            deferredOps[deferredCount].tag = args[1];
                            deferredOps[deferredCount].arg3 = args[2];
                            deferredCount++;
                        } else
#endif
                        adsStopSceneByTtmTag(args[0], args[1]);
                    }
                }

                break;

            case 0x3010:
                debugMsg("RANDOM_START");
                adsRandomStart();
                inRandBlock = 1;
                break;

            case 0x3020:
                if (!adsPeekWordArgs(data, dataSize, &offset, args, 1))
                    return;
                debugMsg("NOP");
                if (inRandBlock)
                    adsRandomNop(args[0]);
                break;

            case 0x30ff:
                debugMsg("RANDOM_END");
                adsRandomEnd();
                inRandBlock = 0;
                break;

            case 0x4000:
                if (!adsPeekWordArgs(data, dataSize, &offset, args, 3))
                    return;
                debugMsg("IF_PLAY_COUNT_LT %d %d %d", args[0], args[1], args[2]);
                if (adsGetScenePlayCount(args[0], args[1]) >= args[2])
                    inSkipBlock = 1;
                break;

            case 0xf010:
                debugMsg("FADE_OUT");
                break;

            case 0xf200:
                if (!adsPeekWordArgs(data, dataSize, &offset, args, 1))
                    return;
                debugMsg("GOSUB_TAG %d", args[0]);    // ex UNKNOWN_8
                // "quick and dirty" implementation, sufficient for
                // JCastaway : only encountered in STAND.ADS to tag 14
                // which only contains 1 scene
                adsPlayChunk(data, dataSize, adsFindTag(args[0]));
                break;

            case 0xffff:
                debugMsg("END");

                /* END closes a skipped block when the interpreter is walking
                 * a false branch; otherwise it terminates the current ADS. */
                if (inSkipBlock)
                    inSkipBlock = 0;
                else
                    adsStopRequested = 1;

                break;

            case 0xfff0:
                debugMsg("END_IF");
                break;

            default :
                debugMsg(":TAG %d\n", opcode);
                break;

        }
    }
#ifdef PS1_BUILD
    for (int i = 0; i < deferredCount; i++) {
        if (deferredOps[i].opcode == 0x2010)
            adsStopSceneByTtmTag(deferredOps[i].slot, deferredOps[i].tag);
        else
            adsAddScene(deferredOps[i].slot, deferredOps[i].tag, deferredOps[i].arg3);
    }
#endif
}


static void adsPlayTriggeredChunks(uint8 *data, uint32 dataSize, uint16 ttmSlotNo, uint16 ttmTag)
{
    // First we deal with the case where a local trigger was declared
    // (only one occurence of this, in ACTIVITY.ADS tag #7)

    if (numAdsChunksLocal) {
        for (int i=0; i < numAdsChunksLocal; i++)
            if (adsChunksLocal[i].scene.slot == ttmSlotNo && adsChunksLocal[i].scene.tag == ttmTag) {
                adsPlayChunk(data, dataSize, adsChunksLocal[i].offset);
                numAdsChunksLocal--;
            }
    }

    // Then, the general case
    else {
        // Note : in a few rare cases (eg BUILDING.ADS tag #2), the ADS script
        // contains several 'IF_LASTPLAYED' commands for one given scene.

        for (int i=0; i < numAdsChunks; i++)
            if (adsChunks[i].scene.slot == ttmSlotNo && adsChunks[i].scene.tag == ttmTag)
                adsPlayChunk(data, dataSize, adsChunks[i].offset);
    }
}
