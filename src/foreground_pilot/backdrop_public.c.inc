struct TTtmSlot *fgBackdropGetSlot(void)
{
    if (gFgBackdropSlot.numSprites[0] == 0)
        return NULL;
    return &gFgBackdropSlot;
}

/* Public wrapper around the file-static fgBackdropStampHoliday().
 * Used by walk_pilot.c during walk transitions so the holiday
 * emblem persists. */
void fgBackdropStampHolidayPublic(void)
{
    fgBackdropStampHoliday();
}

/* Public wrapper for the wave animation tick. walk_pilot calls this
 * each frame so the ocean keeps moving during walks. */
void fgBackdropTickWavesPublic(void)
{
    if (gFgBackdropThread.isRunning) {
        islandAnimate(&gFgBackdropThread);
    }
}

void fgBackdropPrepareIslandRuntimePublic(void)
{
    fgReleaseStreamBuffers();
    fgBackdropPreloadBackgrndBmp();

    fgInitVisiblePipeline();
    grSetPresentDuringScreenLoad(0);
    grSetSaveCleanOnScreenLoad(0);
    if (islandState.night) {
        grLoadScreen("NIGHT.SCR");
    } else {
        grLoadScreen("OCEAN00.SCR");
    }
    grSetSaveCleanOnScreenLoad(1);
    grFreeCleanBgTiles();
    fgBackdropEnableWaveBackdrop();
    grSetPresentDuringScreenLoad(1);
    grForceFullRedrawNextFrame();
}

void fgBackdropReleasePublic(int keepBackgrnd)
{
    fgBackdropRelease(keepBackgrnd);
}

/* No-op kept for ABI compatibility; walk_pilot now reuses the FG2 scene's
 * pre-playback rect snapshot rather than rebuilding bg at walk-time. The
 * earlier grLoadScreen approach worked but cost ~600KB malloc/free + a
 * CD reload per walk, fragmenting the heap so the *next* scene's bg load
 * stalled after a handful of transitions. */
void fgBackdropRebuildIslandBg(void)
{
}

/* Snapshot a walk-area bounding rect as clean. Spots A-F live in
 * approximately X∈[300..522], Y∈[213..255]. With Johnny's sprite
 * size (~40 wide × 60 tall) factored in, the walk's true draw area
 * is closer to X∈[240..580], Y∈[170..320]. Use a single generous
 * rect that covers all walk poses; the rect-clean machinery handles
 * the wave region separately via the same call shape used by
 * fgBackdropSaveCleanBgRectsForPack. */
int fgBackdropSaveCleanBgRectsForWalk(void)
{
    /* Generous walk bbox + the wave region merged. Same shape FG2
     * scenes use (see fgBackdropSaveCleanBgRectsForPack). */
    return fgBackdropSaveCleanBgRectsForPack(
        /* fgX  */ 240,
        /* fgY  */ 170,
        /* fgW  */ 340,
        /* fgH  */ 160);
}

void fgBackdropEndWalk(void)
{
    grDeactivateCleanBgRects();
}
