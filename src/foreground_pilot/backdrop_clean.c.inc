static void fgBackdropRelease(int keepBackgrnd)
{
    int slot;
    /* Under the loading-waves proof, the BACKGRND sprite sheet (slot 0,
     * ~93 KB PSB) is a scene-independent constant: every island scene
     * needs it back. Keep it loaded even when a custom-backdrop scene
     * drops the island (the wave thread and wave cache below still
     * stop — there is no island on screen), so the return-to-island
     * scene skips the CD re-read (walkstuf2 after suzy1:
     * backdrop_vb 42). */
    int keepSlot0 = keepBackgrnd || gFgLoadingWaveProofEnabled;
    int firstSlot = keepSlot0 ? 1 : 0;

    /* When the backdrop is kept across the boundary, keep the wave
     * thread running too: islandAnimate touches only slot 0 (kept
     * below) and grBackgroundSfc, which now survives the boundary
     * (deferred TRANSIENT wipe). This is what makes the ocean keep
     * moving through the inter-scene walk — with the thread stopped,
     * walk_pilot's fgBackdropTickWavesPublic was a no-op and the waves
     * froze for the whole transition. Scenes that drop the backdrop
     * (black / scene-specific) still stop the thread. */
    if (!keepBackgrnd) {
        gFgBackdropThread.isRunning = 0;
        islandClearWaveCache();
    }

    for (slot = firstSlot; slot < MAX_BMP_SLOTS; slot++) {
        /* Slot 2 (holiday sheet) is retained with slot 0 under the
         * proof — both are island constants for their lifetime. */
        if (keepSlot0 && slot == 2)
            continue;
        if (gFgBackdropSlot.numSprites[slot])
            grReleaseBmp(&gFgBackdropSlot, (uint8)slot);
    }

    if (!keepSlot0)
        memset(&gFgBackdropSlot, 0, sizeof(gFgBackdropSlot));
}

static void fgBackdropPreloadHolidaySheet(void)
{
    /* Mirror of the lazy slot-2 load in fgBackdropEnableWaveBackdrop:
     * used at boot (and after a scheduled cache rebuild) so the
     * holiday sheet lands adjacent to the stable band instead of as a
     * mid-region wedge. Holiday state is date-derived and stable for
     * a session; a holiday window opening mid-session still loads
     * lazily on the next backdrop enable. */
    if (holidayById(islandState.holiday)) {
        if (gFgBackdropSlot.numSprites[2] == 0)
            grLoadBmp(&gFgBackdropSlot, 2, "HOLIDAY.BMP");
    }
}

static void fgBackdropPreloadBackgrndBmp(void)
{
    if (gFgBackdropSlot.numSprites[0] > 0 &&
        gFgBackdropSlot.loadedBmpNames[0] != NULL &&
        strcmp(gFgBackdropSlot.loadedBmpNames[0], "BACKGRND.BMP") == 0) {
        fgBackdropRelease(1);
        return;
    }

    fgBackdropRelease(0);
    grLoadBmp(&gFgBackdropSlot, 0, "BACKGRND.BMP");
}

static void fgOverlayKeyFill(struct TFgCleanOverlayKey *key,
                             const char *sceneName)
{
    size_t i;

    memset(key, 0, sizeof(*key));
    if (sceneName == NULL)
        return;

    key->valid = 1;
    key->lowTide = (uint8)(islandState.lowTide ? 1 : 0);
    key->night = (uint8)(islandState.night ? 1 : 0);
    key->raft = islandState.raft;
    key->holiday = islandState.holiday;
    key->islandX = islandState.xPos;
    key->islandY = islandState.yPos;
    key->drawX = gFgSceneDrawOffsetX;
    key->drawY = gFgSceneDrawOffsetY;
    for (i = 0; i + 1 < sizeof(key->sceneName) && sceneName[i] != '\0'; i++)
        key->sceneName[i] = sceneName[i];
    key->sceneName[i] = '\0';
}

static int fgOverlayKeyMatches(const struct TFgCleanOverlayKey *key,
                               const char *sceneName)
{
    return key != NULL &&
           key->valid &&
           sceneName != NULL &&
           strcmp(key->sceneName, sceneName) == 0 &&
           key->lowTide == (uint8)(islandState.lowTide ? 1 : 0) &&
           key->night == (uint8)(islandState.night ? 1 : 0) &&
           key->raft == islandState.raft &&
           key->holiday == islandState.holiday &&
           key->islandX == islandState.xPos &&
           key->islandY == islandState.yPos &&
           key->drawX == gFgSceneDrawOffsetX &&
           key->drawY == gFgSceneDrawOffsetY;
}

static void fgCleanOverlayInvalidate(void)
{
    memset(&gFgCleanOverlayKey, 0, sizeof(gFgCleanOverlayKey));
}

static int fgCleanOverlayMatches(const char *sceneName)
{
    return gFgLoadingWaveProofEnabled &&
           grCleanBgRectsCount() > 0 &&
           fgOverlayKeyMatches(&gFgCleanOverlayKey, sceneName);
}

static void fgCleanOverlayRemember(const char *sceneName)
{
    if (!gFgLoadingWaveProofEnabled ||
        sceneName == NULL ||
        grCleanBgRectsCount() <= 0) {
        fgCleanOverlayInvalidate();
        return;
    }

    fgOverlayKeyFill(&gFgCleanOverlayKey, sceneName);
}

/* Composite the island backdrop sprites — island body, raft (if afloat),
 * low-tide rocks — from the resident gFgBackdropSlot onto the current backdrop
 * surface at the live island offset. Factored out of fgBackdropEnableWaveBackdrop
 * so the decline-render fallback can re-draw the island every frame on top of
 * the ocean repaint: without this the declined ceiling scene rendered ocean
 * only (the "island missing, ship partial" frame). Sprite 0 is the island; the
 * raft load is no-halt (a failed deep-soak load just skips the raft, no BSOD). */
static void fgBackdropDrawIslandSprites(void)
{
    if (gFgBackdropSlot.numSprites[0] == 0)
        return;

    grDx = islandState.xPos;
    grDy = islandState.yPos;

    if (islandState.raft >= 1 && islandState.raft <= 5) {
        int xRaft = islandState.lowTide ? 529 : 512;
        int yRaft = islandState.lowTide ? 281 : 266;
        if (!walkPilotLoadMraftFromSpu(&gFgBackdropSlot, 1)) {
            memSetCacheAllocNoHalt(1);
            grLoadBmp(&gFgBackdropSlot, 1, "MRAFT.BMP");
            memSetCacheAllocNoHalt(0);
        }
        grDrawSprite(grBackgroundSfc, &gFgBackdropSlot,
                     xRaft, yRaft, (uint16)(islandState.raft - 1), 1);
        grReleaseBmp(&gFgBackdropSlot, 1);
    }

    grDrawSprite(grBackgroundSfc, &gFgBackdropSlot, 288, 279,  0, 0);
    grDrawSprite(grBackgroundSfc, &gFgBackdropSlot, 442, 148, 13, 0);
    grDrawSprite(grBackgroundSfc, &gFgBackdropSlot, 365, 122, 12, 0);
    grDrawSprite(grBackgroundSfc, &gFgBackdropSlot, 396, 279, 14, 0);
    if (islandState.lowTide) {
        grDrawSprite(grBackgroundSfc, &gFgBackdropSlot, 249, 303,  1, 0);
        grDrawSprite(grBackgroundSfc, &gFgBackdropSlot, 150, 328,  2, 0);
    }
}

static void fgBackdropEnableWaveBackdrop(void)
{
    gFgBackdropThread.ttmSlot   = &gFgBackdropSlot;
    gFgBackdropThread.ttmLayer  = grBackgroundSfc;
    gFgBackdropThread.isRunning = 3;
    /* Wave cadence: advance one wave frame every (delay+1) ticks. delay=2
     * (advance every ~3 frames) looked 2x too fast; delay=40 was far too
     * slow (foam barely populated/moved). delay=5 read as slightly too
     * slow; delay=3 -> advance every ~4 frames, between the old 2 (too
     * fast) and 5 (too slow). Tuned by eye against a real run. */
    gFgBackdropThread.delay     = 3;
    gFgBackdropThread.timer     = 0;

    if (gFgBackdropSlot.numSprites[0] == 0) {
        gFgBackdropThread.isRunning = 0;
        grSaveCleanBgTiles();
        return;
    }

    fgBackdropDrawIslandSprites();

    if (holidayById(islandState.holiday)) {
        /* The holiday sheet (25 KB PSB) is constant for the whole
         * holiday window: load once and retain (release path keeps
         * slot 2 under the proof). Reloading per setup cost a 13-
         * sector CD read on EVERY transition during holiday periods
         * and churned a 26 KB CACHE block. */
        if (gFgBackdropSlot.numSprites[2] == 0)
            grLoadBmp(&gFgBackdropSlot, 2, "HOLIDAY.BMP");
    } else if (gFgBackdropSlot.numSprites[2] != 0) {
        grReleaseBmp(&gFgBackdropSlot, 2);
    }

    if (gFgLoadingWaveProofEnabled) {
        islandClearWaveCache();
    } else {
        for (int i = 0; i < 4; i++)
            islandAnimate(&gFgBackdropThread);
    }
}

static int fgBackdropSaveCleanBgRectsForPack(sint16 fgX, sint16 fgY, uint16 fgW, uint16 fgH)
{
    const uint32 kMaxCleanRectBytes = gFgCleanRectMaxBytes;
    const sint16 kWaveMinX = 129;
    const sint16 kWaveMinY = 303;
    const sint16 kWaveEndX = 608;
    const sint16 kWaveEndY = 356;
    const sint16 kUpperSplitY = 190;

    sint16 fgEndX = (sint16)(fgX + fgW);
    sint16 fgEndY = (sint16)(fgY + fgH);

    if (fgW == 0 || fgH == 0) {
        sint16 xs[1]; sint16 ys[1]; uint16 ws[1]; uint16 hs[1];
        xs[0] = kWaveMinX;
        ys[0] = kWaveMinY;
        ws[0] = (uint16)(kWaveEndX - kWaveMinX);
        hs[0] = (uint16)(kWaveEndY - kWaveMinY);
        return grSaveCleanBgRectsSplit(xs, ys, ws, hs, 1,
                                       kMaxCleanRectBytes) > 0;
    }

    sint16 lowerMinX = fgX;
    sint16 lowerMinY = fgY >= kUpperSplitY ? fgY : kUpperSplitY;
    sint16 lowerEndX = fgEndX;
    sint16 lowerEndY = fgEndY;

    if (kWaveMinX < lowerMinX) lowerMinX = kWaveMinX;
    if (kWaveMinY < lowerMinY) lowerMinY = kWaveMinY;
    if (kWaveEndX > lowerEndX) lowerEndX = kWaveEndX;
    if (kWaveEndY > lowerEndY) lowerEndY = kWaveEndY;

    if (lowerMinX < 0) lowerMinX = 0;
    if (lowerMinY < 0) lowerMinY = 0;
    if (lowerEndX > 640) lowerEndX = 640;
    if (lowerEndY > 480) lowerEndY = 480;

    if (fgY < kUpperSplitY) {
        sint16 upperMinX = fgX;
        sint16 upperMinY = fgY;
        sint16 upperEndX = fgEndX;
        sint16 upperEndY = kUpperSplitY;
        sint16 xs[2]; sint16 ys[2]; uint16 ws[2]; uint16 hs[2];

        if (upperMinX < 0) upperMinX = 0;
        if (upperMinY < 0) upperMinY = 0;
        if (upperEndX > 640) upperEndX = 640;
        if (upperEndY > 480) upperEndY = 480;

        if (upperEndX <= upperMinX || upperEndY <= upperMinY ||
            lowerEndX <= lowerMinX || lowerEndY <= lowerMinY)
            return 0;

        xs[0] = lowerMinX;
        ys[0] = lowerMinY;
        ws[0] = (uint16)(lowerEndX - lowerMinX);
        hs[0] = (uint16)(lowerEndY - lowerMinY);
        xs[1] = upperMinX;
        ys[1] = upperMinY;
        ws[1] = (uint16)(upperEndX - upperMinX);
        hs[1] = (uint16)(upperEndY - upperMinY);
#if PS1_VERBOSE_DIAGNOSTICS
#if FG_HEAP_PROBE_LOGS
        if (gFgHeapProbeEnabled) {
            printf("JCRECT 2-rect lower=(%d,%d,%u,%u) upper=(%d,%d,%u,%u) heapKB=%lu\n",
                   (int)xs[0], (int)ys[0], (unsigned)ws[0], (unsigned)hs[0],
                   (int)xs[1], (int)ys[1], (unsigned)ws[1], (unsigned)hs[1],
                   fgProbeLargestAlloc() / 1024UL);
        } else
#endif
        {
            printf("JCRECT 2-rect lower=(%d,%d,%u,%u) upper=(%d,%d,%u,%u)\n",
                   (int)xs[0], (int)ys[0], (unsigned)ws[0], (unsigned)hs[0],
                   (int)xs[1], (int)ys[1], (unsigned)ws[1], (unsigned)hs[1]);
        }
#endif
        int rc = grSaveCleanBgRectsSplit(xs, ys, ws, hs, 2,
                                         kMaxCleanRectBytes);
#if PS1_VERBOSE_DIAGNOSTICS
        printf("JCRECT 2-rect split grSaveCleanBgRects=%d\n", rc);
#endif
        return rc > 0;
    } else {
        sint16 xs[1]; sint16 ys[1]; uint16 ws[1]; uint16 hs[1];
        if (lowerEndX <= lowerMinX || lowerEndY <= lowerMinY)
            return 0;
        xs[0] = lowerMinX;
        ys[0] = lowerMinY;
        ws[0] = (uint16)(lowerEndX - lowerMinX);
        hs[0] = (uint16)(lowerEndY - lowerMinY);
#if PS1_VERBOSE_DIAGNOSTICS
#if FG_HEAP_PROBE_LOGS
        if (gFgHeapProbeEnabled) {
            printf("JCRECT 1-rect lower=(%d,%d,%u,%u) heapKB=%lu\n",
                   (int)xs[0], (int)ys[0], (unsigned)ws[0], (unsigned)hs[0],
                   fgProbeLargestAlloc() / 1024UL);
        } else
#endif
        {
            printf("JCRECT 1-rect lower=(%d,%d,%u,%u)\n",
                   (int)xs[0], (int)ys[0], (unsigned)ws[0], (unsigned)hs[0]);
        }
#endif
        int rc = grSaveCleanBgRectsSplit(xs, ys, ws, hs, 1,
                                         kMaxCleanRectBytes);
#if PS1_VERBOSE_DIAGNOSTICS
        printf("JCRECT 1-rect split grSaveCleanBgRects=%d\n", rc);
#endif
        return rc > 0;
    }
}

static uint32 fgBackdropCleanRectEstimateForPack(sint16 fgX, sint16 fgY,
                                                 uint16 fgW, uint16 fgH)
{
    const sint16 kWaveMinX = 129;
    const sint16 kWaveMinY = 303;
    const sint16 kWaveEndX = 608;
    const sint16 kWaveEndY = 356;
    const sint16 kUpperSplitY = 190;

    sint16 fgEndX = (sint16)(fgX + fgW);
    sint16 fgEndY = (sint16)(fgY + fgH);
    uint32 estimate = 0;

    if (fgW == 0 || fgH == 0)
        return (uint32)(kWaveEndX - kWaveMinX) *
               (uint32)(kWaveEndY - kWaveMinY) *
               (uint32)sizeof(uint16);

    {
        sint16 lowerMinX = fgX;
        sint16 lowerMinY = fgY >= kUpperSplitY ? fgY : kUpperSplitY;
        sint16 lowerEndX = fgEndX;
        sint16 lowerEndY = fgEndY;

        if (kWaveMinX < lowerMinX) lowerMinX = kWaveMinX;
        if (kWaveMinY < lowerMinY) lowerMinY = kWaveMinY;
        if (kWaveEndX > lowerEndX) lowerEndX = kWaveEndX;
        if (kWaveEndY > lowerEndY) lowerEndY = kWaveEndY;

        if (lowerMinX < 0) lowerMinX = 0;
        if (lowerMinY < 0) lowerMinY = 0;
        if (lowerEndX > 640) lowerEndX = 640;
        if (lowerEndY > 480) lowerEndY = 480;

        if (lowerEndX > lowerMinX && lowerEndY > lowerMinY) {
            estimate += (uint32)(lowerEndX - lowerMinX) *
                        (uint32)(lowerEndY - lowerMinY) *
                        (uint32)sizeof(uint16);
        }
    }

    if (fgY < kUpperSplitY) {
        sint16 upperMinX = fgX;
        sint16 upperMinY = fgY;
        sint16 upperEndX = fgEndX;
        sint16 upperEndY = kUpperSplitY;

        if (upperMinX < 0) upperMinX = 0;
        if (upperMinY < 0) upperMinY = 0;
        if (upperEndX > 640) upperEndX = 640;
        if (upperEndY > 480) upperEndY = 480;

        if (upperEndX > upperMinX && upperEndY > upperMinY) {
            estimate += (uint32)(upperEndX - upperMinX) *
                        (uint32)(upperEndY - upperMinY) *
                        (uint32)sizeof(uint16);
        }
    }

    return estimate;
}

static int fgBackdropSaveCleanBgRectsWithPressureFallback(const char *sceneName,
                                                          sint16 fgX,
                                                          sint16 fgY,
                                                          uint16 fgW,
                                                          uint16 fgH,
                                                          uint32 cleanBytes,
                                                          int *deferWalkCleanRecapture)
{
    /* Plan v9 Phase 2 manifest item #7: pressure-fallback chain is
     * no longer needed. Clean-rect snapshots now allocate from
     * MEM_REGION_TRANSIENT, which has its own 250 KB dedicated to
     * per-scene scratch and cannot fragment. memAlloc(TRANSIENT) halts
     * loudly on exhaustion; there is no "fall back and retry" semantic
     * to recover with.
     *
     * Kept as a thin pass-through so existing callers don't need
     * touching — they still call the same wrapper name; the wrapper
     * just delegates directly to the underlying save function and
     * returns its result. */
    (void)sceneName;
    (void)cleanBytes;
    (void)deferWalkCleanRecapture;
    if (gFgRuntimeSceneId == FG_SCENE_VISITOR3 && !islandState.lowTide)
        return fgBackdropSaveVisitor3HighCleanBgRects();
    return fgBackdropSaveCleanBgRectsForPack(fgX, fgY, fgW, fgH);
}

static void fgBackdropStampHoliday(void)
{
    static int cachedHolidayId = -2;
    static const struct Holiday *cachedHoliday = NULL;
    int holidayId = islandState.holiday;
    const struct Holiday *holiday;

    if (holidayId <= 0)
        return;

    if (cachedHolidayId != holidayId) {
        cachedHoliday = holidayById(holidayId);
        cachedHolidayId = holidayId;
    }
    holiday = cachedHoliday;
    if (holiday == NULL)
        return;
    if (gFgBackdropSlot.numSprites[2] == 0)
        return;

    grDx = islandState.xPos;
    grDy = islandState.yPos;
    grDrawSprite(grBackgroundSfc, &gFgBackdropSlot,
                 holiday->island_x, holiday->island_y,
                 (uint16)holiday->sprite_index, 2);
}
