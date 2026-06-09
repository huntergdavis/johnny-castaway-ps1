static void fpClearScreen(void)
{
    fpCancelTransientAction();
    fpSetBanner("CLEARED", 90);
    captionsShowText(kFpCaptionClear, 120);
    grShowMeanwhileLoadingFrame((uint16)(gFp.frame & 7UL));
    if (!fpRebuildBackdrop())
        gFreeplayExitRequested = 1;
}

static void fpStartGag(enum TFpGag gag)
{
    gFp.actionCount++;
    gFp.lastGag = (uint8)gag;

    fpCancelAsset(&gFp.gag);
    fpCancelAsset(&gFp.summon);
    if (!fpLoadSlot(FP_SLOT_GAG, kFpGagBmps[gag])) {
        fpSetBanner("ACTION UNAVAILABLE", 60);
        return;
    }
    strncpy(gFp.gag.bmp, kFpGagBmps[gag], sizeof(gFp.gag.bmp) - 1);
    gFp.gag.slot = FP_SLOT_GAG;
    gFp.gag.active = 1;
    gFp.gag.frameDelay = kFpGagFrameDelays[gag];
    gFp.gag.ttl = kFpGagDurations[gag];
    gFp.gag.x = (sint16)(gFp.x + kFpGagXOffsets[gag]);
    gFp.gag.y = (sint16)(gFp.y + kFpGagYOffsets[gag]);
    gFp.gag.firstFrame = 0;
    gFp.gag.frameCount = kFpGagFrameCounts[gag];
    if (gFp.gag.frameCount > gFpSlot.numSprites[FP_SLOT_GAG])
        gFp.gag.frameCount = gFpSlot.numSprites[FP_SLOT_GAG];
    if (gFp.gag.frameCount == 0)
        gFp.gag.frameCount = 1;
    gFp.gag.flip = (kFpGagFlipWithFace[gag] && gFp.face == FP_FACE_W) ? 1 : 0;
    gFp.gag.loop = (gag == FP_GAG_STRUT || gag == FP_GAG_RUNAWAY) ? 1 : 0;
    gFp.gag.screenRelative = 0;
    gFp.mode = (gag == FP_GAG_STRUT) ? FP_MODE_STRUT :
               (gag == FP_GAG_RUNAWAY) ? FP_MODE_RUNAWAY :
               FP_MODE_ONESHOT;
    gFp.modeTimer = gFp.gag.ttl;
    fpSetBanner(kFpGagLabels[gag], 90);
    captionsShowText(kFpCaptionAction, 150);

    if (gag == FP_GAG_BONK) soundPlay(5);
    else if (gag == FP_GAG_EAT) soundPlay(8);
    else if (gag == FP_GAG_HOT) soundPlay(4);
}

static void fpStartSummon(enum TFpSummon kind)
{
    const char *bmp = "GJGULL1.BMP";
    sint16 x = 452, y = 154, vx = 0, vy = 0;
    uint16 ttl = 420;
    uint16 delay = 10;
    uint16 frameCount = 1;
    uint8 screenRelative = 0;
    int soundId = -1;

    switch (kind) {
    case FP_SUMMON_SEAGULL:  bmp = "GJGULL1.BMP"; x = 452; y = 154; ttl = 420; soundId = 4; break;
    case FP_SUMMON_LILIPUTS: bmp = "LILIPUTS.BMP"; x = 255; y = 292; ttl = 480; frameCount = 4; break;
    case FP_SUMMON_BIPLANE:  bmp = "GJBIPLAN.BMP"; x = 420; y = 128; ttl = 300; break;
    case FP_SUMMON_CANOE:    bmp = "GJNAT1.BMP"; x = 506; y = 292; ttl = 420; gFp.fridayCount++; break;
    case FP_SUMMON_BOAT:     bmp = "BOAT.BMP"; x = 505; y = 276; ttl = 420; soundId = 16; break;
    case FP_SUMMON_KINGKONG: bmp = "GJKINGKO.BMP"; x = 265; y = 120; ttl = 360; soundId = 5; break;
    case FP_SUMMON_MARY:     bmp = "MJBATH.BMP"; x = 255; y = 300; ttl = 420; frameCount = 6; break;
    case FP_SUMMON_PIRATE:   bmp = "FISHMAN.BMP"; x = 500; y = 235; ttl = 360; break;
    case FP_SUMMON_FLOCK:    bmp = "GJGULL3.BMP"; x = 444; y = 150; ttl = 360; frameCount = 3; break;
    case FP_SUMMON_MEANWHILE:bmp = "MEANWHIL.BMP"; x = 270; y = 120; ttl = 90; delay = 10; break;
    case FP_SUMMON_CLOUD:    bmp = "CLOUDS.BMP"; x = 420; y = 96; ttl = 480; break;
    default: break;
    }

    fpCancelAsset(&gFp.gag);
    fpCancelAsset(&gFp.summon);
    if (!fpLoadSlot(FP_SLOT_SUMMON, bmp)) {
        fpSetBanner("SUMMON SKIPPED", 60);
        return;
    }
    strncpy(gFp.summon.bmp, bmp, sizeof(gFp.summon.bmp) - 1);
    gFp.summon.slot = FP_SLOT_SUMMON;
    gFp.summon.active = 1;
    gFp.summon.x = x;
    gFp.summon.y = y;
    gFp.summon.vx = vx;
    gFp.summon.vy = vy;
    gFp.summon.ttl = ttl;
    gFp.summon.frameDelay = delay;
    gFp.summon.firstFrame = 0;
    gFp.summon.frameCount = frameCount;
    gFp.summon.loop = 1;
    gFp.summon.screenRelative = screenRelative;
    gFp.summonCount++;
    captionsShowText(kFpCaptionSummon, 180);
    if (soundId >= 0)
        soundPlay(soundId);

    if (kind == FP_SUMMON_MARY && gFp.fridayCount >= 3)
        fpSetBanner("FRIDAY REMEMBERS", 150);
    else if (kind == FP_SUMMON_KINGKONG)
        fpSetBanner("KING KONG!", 120);
}

static int fpNextHolidayId(int current)
{
    int maxId = holidayMaxId();
    int next = current;
    for (int i = 0; i <= maxId; i++) {
        next++;
        if (next > maxId)
            next = 0;
        if (next == 0 || holidayById(next) != NULL)
            return next;
    }
    return 0;
}

static int fpApplyMenuWorldOverrides(void)
{
    int changed = 0;
    int desiredNight = islandState.night;
    int desiredLowTide = islandState.lowTide;
    int desiredRaft = islandState.raft;
    int desiredHoliday = islandState.holiday;
    int desiredX = islandState.xPos;
    int desiredY = islandState.yPos;

    if (hostForcedNight >= 0) {
        desiredNight = hostForcedNight;
    } else if (ps1SoftTimeEnabled) {
        desiredNight = (ps1SoftHour < 6 || ps1SoftHour >= 20) ? 1 : 0;
    }

    if (hostForcedLowTide >= 0)
        desiredLowTide = hostForcedLowTide;
    if (hostForcedRaftStage >= 0)
        desiredRaft = hostForcedRaftStage;

    if (hostHolidayMode == HOLIDAY_MODE_MANUAL_ORIG4 ||
        hostHolidayMode == HOLIDAY_MODE_MANUAL_EXPANDED) {
        desiredHoliday = hostForcedHoliday;
    } else if (hostHolidayMode == HOLIDAY_MODE_NONE) {
        desiredHoliday = 0;
    } else if (hostHolidayMode == HOLIDAY_MODE_AUTO_ORIGINAL4 &&
               ps1SoftTimeEnabled) {
        desiredHoliday = holidayForDateOriginal4(ps1SoftYear,
                                                 ps1SoftMonth,
                                                 ps1SoftDay);
    } else if (ps1SoftTimeEnabled) {
        desiredHoliday = holidayForDate(ps1SoftYear, ps1SoftMonth, ps1SoftDay);
    }

    if (hostForcedIslandPosValid) {
        desiredX = hostForcedIslandX;
        desiredY = hostForcedIslandY;
    }

    if (islandState.night != desiredNight) {
        islandState.night = desiredNight;
        changed = 1;
    }
    if (islandState.lowTide != desiredLowTide) {
        islandState.lowTide = desiredLowTide;
        changed = 1;
    }
    if (islandState.raft != desiredRaft) {
        islandState.raft = desiredRaft;
        changed = 1;
    }
    if (islandState.holiday != desiredHoliday) {
        islandState.holiday = desiredHoliday;
        changed = 1;
    }
    if (islandState.xPos != desiredX || islandState.yPos != desiredY) {
        islandState.xPos = (sint16)desiredX;
        islandState.yPos = (sint16)desiredY;
        changed = 1;
    }

    if (changed) {
        fpCancelTransientAction();
        fpSetBanner("WORLD UPDATED", 90);
        grShowMeanwhileLoadingFrame((uint16)(gFp.frame & 7UL));
        if (!fpRebuildBackdrop())
            gFreeplayExitRequested = 1;
    }

    return changed;
}

static void fpHandleEnvironment(uint16 pressed)
{
    if (pressed & PAD_UP) {
        islandState.night = !islandState.night;
        fpSetBanner(islandState.night ? "NIGHT" : "DAY", 90);
        fpRebuildBackdrop();
    } else if (pressed & PAD_DOWN) {
        islandState.lowTide = !islandState.lowTide;
        fpSetBanner(islandState.lowTide ? "LOW TIDE" : "HIGH TIDE", 90);
        fpRebuildBackdrop();
    } else if (pressed & PAD_RIGHT) {
        islandState.holiday = fpNextHolidayId(islandState.holiday);
        fpSetBanner(islandState.holiday ? "HOLIDAY" : "NO HOLIDAY", 90);
        fpRebuildBackdrop();
    } else if (pressed & PAD_LEFT) {
        islandState.raft++;
        if (islandState.raft > 5)
            islandState.raft = 0;
        fpSetBanner("RAFT STAGE", 90);
        fpRebuildBackdrop();
    }
}

static void fpOpenPauseMenu(void)
{
    captionsShowText(kFpCaptionPause, 120);
    pauseMenuShow();
    while (pauseMenuIsVisible()) {
        pauseMenuUpdate();
    }
    if (pauseMenuRequestExitFreeplay ||
        pauseMenuRequestNextScene ||
        pauseMenuRequestResetLoop ||
        pauseMenuRequestSceneSetCycle) {
        pauseMenuRequestExitFreeplay = 0;
        pauseMenuRequestFreeplayWorldRefresh = 0;
        gFreeplayExitRequested = 1;
    } else {
        int gagRequest = pauseMenuRequestFreeplayGag;
        int visitorRequest = pauseMenuRequestFreeplayVisitor;
        int clearRequest = pauseMenuRequestFreeplayClear;
        int worldRequest = pauseMenuRequestFreeplayWorldRefresh;

        pauseMenuRequestFreeplayGag = -1;
        pauseMenuRequestFreeplayVisitor = -1;
        pauseMenuRequestFreeplayClear = 0;
        pauseMenuRequestFreeplayWorldRefresh = 0;

        if (worldRequest) {
            fpApplyMenuWorldOverrides();
        } else if (clearRequest) {
            fpClearScreen();
        } else if (gagRequest >= 0 && gagRequest < FP_GAG_COUNT) {
            fpStartGag((enum TFpGag)gagRequest);
        } else if (visitorRequest >= 0 && visitorRequest < FP_SUMMON_COUNT) {
            fpStartSummon((enum TFpSummon)visitorRequest);
        }
    }
    gFp.prevButtons = 0xffff;
    grForceFullRedrawNextFrame();
}

static void fpApplyInput(uint16 cur, uint16 pressed)
{
    int dx = 0;
    int dy = 0;

    if (cur)
        gFp.inputFrames++;

    if (pressed & PAD_START) {
        fpOpenPauseMenu();
        return;
    }

    if (cur & PAD_R1) {
        fpHandleEnvironment(pressed);
        return;
    }

    if (cur & PAD_LEFT)      { dx = -1; gFp.face = FP_FACE_W; }
    else if (cur & PAD_RIGHT){ dx =  1; gFp.face = FP_FACE_E; }
    else if (cur & PAD_UP)   { dy = -1; gFp.face = FP_FACE_N; }
    else if (cur & PAD_DOWN) { dy =  1; gFp.face = FP_FACE_S; }

    if (dx || dy) {
        int speed = 1;
        if (cur & PAD_R2)
            speed = 2;
        else if (cur & PAD_L2)
            speed = (gFp.frame & 1) ? 1 : 0;
        if (gFp.gag.active || gFp.summon.active || gFp.mode != FP_MODE_WALK)
            fpCancelTransientAction();
        gFp.x = (sint16)(gFp.x + dx * speed);
        gFp.y = (sint16)(gFp.y + dy * speed);
        fpClampJohnny();
        gFp.walkFrame++;
        gFp.walkFrames++;
        gFp.idleFrames = 0;
        gFp.mode = FP_MODE_WALK;
        return;
    } else if (gFp.mode == FP_MODE_WALK) {
        gFp.mode = FP_MODE_IDLE;
    }

    if (pressed & PAD_SELECT) {
        fpClearScreen();
        return;
    }

    /* CIRCLE-button fishing was retired in favor of a top-level
     * "fishing mode" (toggle from the pause menu). Freeplay no longer
     * spawns a fishing animation directly. */
    (void)pressed;
}
