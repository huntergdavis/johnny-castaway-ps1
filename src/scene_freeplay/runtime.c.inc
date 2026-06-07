static void fpInitState(void)
{
    memset(&gFp, 0, sizeof(gFp));
    gFp.x = 392;
    gFp.y = 238;
    gFp.face = FP_FACE_E;
    gFp.mode = FP_MODE_IDLE;
    gFp.prevButtons = 0;
    gFp.ambientNextFrame = 0xffffffffUL;
    gFp.lastGag = FP_GAG_BONK;
    fpSetBanner("FREEPLAY", 120);
}

void freeplayRun(void)
{
    uint16 cur;
    uint16 pressed;

    gFreeplayExitRequested = 0;
    pauseMenuRequestExitFreeplay = 0;
    pauseMenuRequestFreeplayGag = -1;
    pauseMenuRequestFreeplayVisitor = -1;
    pauseMenuRequestFreeplayClear = 0;
    pauseMenuRequestFreeplayWorldRefresh = 0;
    pauseMenuSetFreeplayActive(1);
    fpInitState();
    memset(&gFpSlot, 0, sizeof(gFpSlot));
    captionsShowText(kFpCaptionStart, 240);

    FP_LOG(1, ("JCFREE start heapKB=%lu night=%d low=%d raft=%d holiday=%d\n",
               fgProbeLargestAlloc() / 1024UL,
               islandState.night, islandState.lowTide,
               islandState.raft, islandState.holiday));

    FP_LOG(1, ("JCFREE boot backdrop-first heapKB=%lu\n",
               fgProbeLargestAlloc() / 1024UL));
    FP_LOG(1, ("JCFREE rebuild backdrop begin heapKB=%lu\n",
               fgProbeLargestAlloc() / 1024UL));
    if (!fpRebuildBackdrop()) {
        gFreeplayExitRequested = 1;
        fpReleaseSlots();
        grFreeCleanBgRects();
        fgBackdropReleasePublic(1);
        captionsClear();
        pauseMenuSetFreeplayActive(0);
        return;
    }
    FP_LOG(1, ("JCFREE backdrop ready heapKB=%lu rectKB=%lu\n",
               fgProbeLargestAlloc() / 1024UL,
               grCleanBgRectsBytes() / 1024UL));
    FP_LOG(1, ("JCFREE rebuild backdrop done heapKB=%lu rectKB=%lu\n",
               fgProbeLargestAlloc() / 1024UL,
               grCleanBgRectsBytes() / 1024UL));

    FP_LOG(1, ("JCFREE load johnwalk heapKB=%lu\n",
               fgProbeLargestAlloc() / 1024UL));
    if (!fpLoadSlot(FP_SLOT_JOHN, "JOHNWALK.BMP")) {
        FP_LOG(1, ("JCFREE fatal missing JOHNWALK\n"));
        gFreeplayExitRequested = 1;
        fpReleaseSlots();
        grFreeCleanBgRects();
        fgBackdropReleasePublic(1);
        captionsClear();
        pauseMenuSetFreeplayActive(0);
        return;
    }
    FP_LOG(1, ("JCFREE johnwalk loaded heapKB=%lu\n",
               fgProbeLargestAlloc() / 1024UL));

    grForceFullRedrawNextFrame();
    gFp.prevButtons = fpReadButtons();

    while (!gFreeplayExitRequested) {
        cur = fpReadButtons();
        pressed = (uint16)(cur & ~gFp.prevButtons);
        gFp.prevButtons = cur;

        fpApplyInput(cur, pressed);
        fpTick();
        fpFrame();

        gFp.frame++;
        if (gFreeplayTelemetryLevel >= 1 && (gFp.frame % 300U) == 0) {
            FP_LOG(1,
                   ("JCFREE summary frame=%lu mode=%s x=%d y=%d walk=%lu action=%lu summon=%lu clean=%lu cleanFail=%lu assetFail=%lu rectKB=%lu heapKB=%lu\n",
                    (unsigned long)gFp.frame,
                    fpModeName(gFp.mode),
                    (int)gFp.x, (int)gFp.y,
                    (unsigned long)gFp.walkFrames,
                    (unsigned long)gFp.actionCount,
                    (unsigned long)gFp.summonCount,
                    (unsigned long)gFp.cleanRebuilds,
                    (unsigned long)gFp.cleanFailures,
                    (unsigned long)gFp.assetLoadFailures,
                    grCleanBgRectsBytes() / 1024UL,
                    fgProbeLargestAlloc() / 1024UL));
        }
    }

    while (fpReadButtons() & PAD_START)
        VSync(0);

    FP_LOG(1,
           ("JCFREE end frame=%lu walk=%lu action=%lu summon=%lu clean=%lu cleanFail=%lu assetFail=%lu heapKB=%lu\n",
            (unsigned long)gFp.frame,
            (unsigned long)gFp.walkFrames,
            (unsigned long)gFp.actionCount,
            (unsigned long)gFp.summonCount,
            (unsigned long)gFp.cleanRebuilds,
            (unsigned long)gFp.cleanFailures,
            (unsigned long)gFp.assetLoadFailures,
            fgProbeLargestAlloc() / 1024UL));

    fpReleaseSlots();
    grFreeCleanBgRects();
    fgBackdropReleasePublic(1);
    walkRenderResetCache();
    captionsClear();
    grShowMeanwhileLoadingFrame((uint16)(gFp.frame & 7UL));
    pauseMenuSetFreeplayActive(0);
    grForceFullRedrawNextFrame();
}

int freeplayExitRequested(void)
{
    return gFreeplayExitRequested;
}

void freeplayClearExitRequest(void)
{
    gFreeplayExitRequested = 0;
}


void freeplaySetTelemetryLevel(int level)
{
    if (level < 0) level = 0;
    if (level > 3) level = 3;
    gFreeplayTelemetryLevel = level;
}

int freeplayGetTelemetryLevel(void)
{
    return gFreeplayTelemetryLevel;
}
