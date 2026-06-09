static void adsAddScene(uint16 ttmSlotNo, uint16 ttmTag, uint16 arg3)
{
    if (!adsIsValidTtmSlot(ttmSlotNo))
        return;

    if (adsConsumePendingStop(ttmSlotNo, ttmTag)) {
        debugMsg("(%d,%d) consumed pending STOP_SCENE - didn't add scene\n", ttmSlotNo, ttmTag);
        return;
    }

    /* On PS1, STOP_SCENE may mark a thread terminated earlier in the same ADS
     * chunk. Do not reap here or ADD_SCENE can immediately recycle that slot
     * before the current handoff finishes. */
#ifndef PS1_BUILD
    adsReapTerminatedThreads();
#endif

    for (int i=0; i < MAX_TTM_THREADS; i++) {

        struct TTtmThread *ttmThread = &ttmThreads[i];

        if (ttmThread->isRunning == 1) {

            if (ttmThread->sceneSlot == ttmSlotNo && ttmThread->sceneTag == ttmTag) {
                debugMsg("(%d,%d) thread is already running - didn't add extra one\n", ttmSlotNo, ttmTag);
                return;
            }
        }
    }

    int i=0;

    while (i < MAX_TTM_THREADS && ttmThreads[i].isRunning)
        i++;

    /* Guard against thread-pool overflow: avoid writing past ttmThreads[]. */
    if (i >= MAX_TTM_THREADS) {
        fatalError("ADS thread overflow: add scene (%d,%d) with MAX_TTM_THREADS=%d",
                   ttmSlotNo, ttmTag, MAX_TTM_THREADS);
#ifdef PS1_BUILD
        grPs1StatThreadDrop();
#endif
    }

    struct TTtmThread newThread;
    struct TTtmThread *ttmThread;

    memset(&newThread, 0, sizeof(newThread));
    newThread.ttmSlot         = &ttmSlots[ttmSlotNo];
    newThread.isRunning       = 1;
    newThread.sceneSlot       = ttmSlotNo;
    newThread.sceneTag        = ttmTag;
    newThread.delay           = 4;
    newThread.fgColor         = 0x0f;
    newThread.bgColor         = 0x0f;
#ifdef PS1_BUILD
    if (adsStringEquals(ps1AdsCurrentName, "FISHING.ADS"))
        newThread.delay = 2;
    newThread.sceneEpoch = ttmThreads[i].sceneEpoch + 1;
#endif
    ttmThreads[i] = newThread;
    ttmThread = &ttmThreads[i];
    adsDiagNoteAdd(i, ttmSlotNo, ttmTag);

    if (ttmSlotNo) {
        struct TTtmSlot *slot = &ttmSlots[ttmSlotNo];
        if (slot->numTags > 0 && slot->tags && slot->tags[0].id == ttmTag)
            ttmThread->ip = 0;
        else {
            ttmThread->ip = ttmFindTag(slot, ttmTag);
            if (ttmTag && ttmThread->ip == 0) {
                ttmThread->isRunning = 0;
                return;
            }
        }
    } else
        ttmThread->ip = 0;

    adsNoteSceneStarted(ttmSlotNo, ttmTag);

    if (((short)arg3) < 0) {
        ttmThread->sceneTimer = -((short)arg3);
    }
    else if (((short)arg3) > 0) {
        ttmThread->sceneIterations = arg3 - 1;
    }

#ifdef PS1_BUILD
    /* PS1 RAM-sprite path composites directly into the background tiles, so
     * scene restore ops must target that same surface instead of an empty
     * side layer that never reaches the framebuffer. */
    ttmThread->ttmLayer = grBackgroundSfc;
    adsSeedFromHandoffReplay(ttmThread);
#else
    ttmThread->ttmLayer = grNewLayer();
#endif

    if (numThreads < MAX_TTM_THREADS)
        numThreads++;
}


static void adsStopScene(int sceneNo)
{
#ifdef PS1_BUILD
    adsDiagNoteReap(sceneNo, ttmThreads[sceneNo].sceneSlot, ttmThreads[sceneNo].sceneTag);
#endif
#ifdef PS1_BUILD
    adsCaptureHandoffReplay(&ttmThreads[sceneNo]);
#endif
#ifdef PS1_BUILD
    if (ttmThreads[sceneNo].ttmLayer != grBackgroundSfc)
        grFreeLayer(ttmThreads[sceneNo].ttmLayer);
#else
    grFreeLayer(ttmThreads[sceneNo].ttmLayer);
#endif
    memset(&ttmThreads[sceneNo], 0, sizeof(ttmThreads[sceneNo]));
#ifdef PS1_BUILD
#endif
    if (numThreads > 0)
        numThreads--;
}


static void adsStopSceneByTtmTagEx(uint16 ttmSlotNo, uint16 ttmTag, int recordPending)
{
    struct TTtmThread *ttmThread;
    int found = 0;

    for (int i=0; i < MAX_TTM_THREADS; i++) {

        ttmThread = &ttmThreads[i];

        if (ttmThread->isRunning) {

            if (ttmThread->sceneSlot == ttmSlotNo && ttmThread->sceneTag == ttmTag) {
                found = 1;
#ifdef PS1_BUILD
                /* Defer cleanup to the main ADS loop so STOP_SCENE does not
                 * zero another live thread while we are still parsing the
                 * current ADS chunk. */
                adsDiagNoteStop(i, ttmThread->sceneSlot, ttmThread->sceneTag);
                ttmThread->isRunning = ADS_THREAD_TERMINATED;
                ttmThread->timer = 0;
                ttmThread->nextGotoOffset = 0;
                ttmThread->sceneTimer = 0;
                ttmThread->sceneIterations = 0;
#else
                adsStopScene(i);
#endif
            }
        }
    }

    if (!found && recordPending)
        adsRecordPendingStop(ttmSlotNo, ttmTag);
}


static void adsStopSceneByTtmTag(uint16 ttmSlotNo, uint16 ttmTag)
{
    adsStopSceneByTtmTagEx(ttmSlotNo, ttmTag, 1);
}


static int isSceneRunning(uint16 ttmSlotNo, uint16 ttmTag)
{
    for (int i=0; i < MAX_TTM_THREADS; i++) {

        struct TTtmThread thread = ttmThreads[i];

        if (    thread.isRunning == 1
             && thread.sceneSlot == ttmSlotNo
             && thread.sceneTag  == ttmTag    ) {
            return 1;
        }
    }

    return 0;
}


static struct TAdsRandOp *adsRandomPickOp()
{
    int totalWeight = 0;
    int partialWeight = 0;
    int res;


    // Pick in a list of weighted elements

    for (int i=0; i < adsNumRandOps; i++)
        totalWeight += adsRandOps[i].weight;

    int a = rand() % totalWeight;

    for (res=0; res < adsNumRandOps; res++) {
        partialWeight += adsRandOps[res].weight;
        if (a < partialWeight)
            break;
    }

    return &adsRandOps[res];
}


static void adsRandomStart()
{
    adsNumRandOps = 0;
}


static void adsRandomAddScene(uint16 ttmSlotNo, uint16 ttmTag, uint16 numPlays,
                              uint16 weight)
{
    adsRandOps[adsNumRandOps].type      = OP_ADD_SCENE;
    adsRandOps[adsNumRandOps].slot      = ttmSlotNo;
    adsRandOps[adsNumRandOps].tag       = ttmTag;
    adsRandOps[adsNumRandOps].numPlays  = numPlays;
    adsRandOps[adsNumRandOps].weight    = weight;
    adsNumRandOps++;
}


static void adsRandomStopSceneByTtmTag(uint16 ttmSlotNo, uint16 ttmTag,
                                       uint16 weight)
{
    adsRandOps[adsNumRandOps].type      = OP_STOP_SCENE;
    adsRandOps[adsNumRandOps].slot      = ttmSlotNo;
    adsRandOps[adsNumRandOps].tag       = ttmTag;
    adsRandOps[adsNumRandOps].numPlays  = 0;
    adsRandOps[adsNumRandOps].weight    = weight;
    adsNumRandOps++;
}


static void adsRandomNop(uint16 weight)
{
    adsRandOps[adsNumRandOps].type      = OP_NOP;
    adsRandOps[adsNumRandOps].slot      = 0;
    adsRandOps[adsNumRandOps].tag       = 0;
    adsRandOps[adsNumRandOps].numPlays  = 0;
    adsRandOps[adsNumRandOps].weight    = weight;
    adsNumRandOps++;
}


static void adsRandomEnd()
{
    if (adsNumRandOps) {

       struct TAdsRandOp *op = adsRandomPickOp();

       switch (op->type) {

           case OP_ADD_SCENE:
               debugMsg("RANDOM: chose ADD_SCENE %d %d", op->slot, op->tag);
               adsAddScene(op->slot, op->tag, op->numPlays);
               break;

           case OP_STOP_SCENE:
               debugMsg("RANDOM: chose STOP_SCENE %d %d", op->slot, op->tag);
               adsStopSceneByTtmTagEx(op->slot, op->tag, 0);
               break;

           default:
               debugMsg("RANDOM: chose NOP");
               break;
       }
    }
    else {
        debugMsg("RANDOM : no operation to choose from");
    }
}
