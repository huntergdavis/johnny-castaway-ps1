#ifdef PS1_BUILD

/* Minimal wave-backdrop enable for the fgpilot path. Loads BACKGRND.BMP into
 * the background slot, sets up the background thread, draws the four initial
 * shore waves, and re-saves the clean bg tiles so the waves persist across
 * the grRestoreBgTiles that runs every frame. Must be called after the
 * static scene base (ocean + island) is in place. */
void adsPilotReleaseBackdrop(int keepBackgrnd)
{
    int slot;
    int firstSlot = keepBackgrnd ? 1 : 0;

    ttmBackgroundThread.isRunning = 0;

    for (slot = firstSlot; slot < MAX_BMP_SLOTS; slot++) {
        if (ttmBackgroundSlot.numSprites[slot])
            grReleaseBmp(&ttmBackgroundSlot, (uint8)slot);
    }

    if (!keepBackgrnd)
        ttmResetSlot(&ttmBackgroundSlot);
}

/* Pre-load BACKGRND.BMP while the heap is freshest — call BEFORE
 * fgInitVisiblePipeline so the ~93 KB PSB stream has room.  Safe to call
 * early because ttmBackgroundSlot is a file-scope static and already
 * zero-initialized. */
void adsPilotPreloadBackgrndBmp(void)
{
    /* Fast path: BACKGRND.BMP is already loaded in slot 0 from a previous
     * screensaver-loop iteration. Skip the release+reinit+reload cycle
     * entirely — the PSB buffer is ~93 KB and re-allocating it each
     * scene churns the heap and contributes to fragmentation failures
     * after a few iterations. Slots 1 (MRAFT) and 2 (HOLIDAY) still need
     * cleanup since they're variant-dependent. */
    if (ttmBackgroundSlot.numSprites[0] > 0 &&
        ttmBackgroundSlot.loadedBmpNames[0] != NULL &&
        strcmp(ttmBackgroundSlot.loadedBmpNames[0], "BACKGRND.BMP") == 0) {
        adsPilotReleaseBackdrop(1);
        return;
    }

    /* Cold path (first iteration, or slot 0 held some other BMP):
     * release everything, re-init, and load BACKGRND fresh. Releasing
     * all slots BEFORE ttmInitSlot is load-bearing — ttmInitSlot zeros
     * the metadata, and grLoadBmp's internal release-check reads
     * numSprites[slot] == 0 after that, so the real release would be
     * skipped and the previous sprite array would leak. */
    adsPilotReleaseBackdrop(0);
    grLoadBmp(&ttmBackgroundSlot, 0, "BACKGRND.BMP");
}

void adsPilotEnableWaveBackdrop(void)
{
    /* Called AFTER the static scene base is in place (grLoadScreen done).
     * By this point adsPilotPreloadBackgrndBmp should have already populated
     * ttmBackgroundSlot. We only need to (re)configure the thread and seed
     * the wave positions into the tiles before grSaveCleanBgTiles. */
    ttmBackgroundThread.ttmSlot   = &ttmBackgroundSlot;
    ttmBackgroundThread.ttmLayer  = grBackgroundSfc;
    ttmBackgroundThread.isRunning = 3;
    ttmBackgroundThread.delay     = 2;  /* new wave every 2 frames — each of 3 positions updates every 6 frames */
    ttmBackgroundThread.timer     = 0;

    /* Safety no-hang: if the BACKGRND load didn't populate the slot, skip the
     * per-position seed (islandAnimate would otherwise div-by-zero). */
    if (ttmBackgroundSlot.numSprites[0] == 0) {
        ttmBackgroundThread.isRunning = 0;
        grSaveCleanBgTiles();
        return;
    }

    grDx = islandState.xPos;
    grDy = islandState.yPos;

    /* Raft: MRAFT.BMP has 5 frames, one per raft-construction stage. Loaded
     * into BMP slot 1 so slot 0 (BACKGRND) stays intact for waves. Position
     * matches islandInit: high tide (512, 266), low tide (529, 281). */
    if (islandState.raft >= 1 && islandState.raft <= 5) {
        int xRaft = islandState.lowTide ? 529 : 512;
        int yRaft = islandState.lowTide ? 281 : 266;
        grLoadBmp(&ttmBackgroundSlot, 1, "MRAFT.BMP");
        grDrawSprite(grBackgroundSfc, &ttmBackgroundSlot,
                     xRaft, yRaft, (uint16)(islandState.raft - 1), 1);
        grReleaseBmp(&ttmBackgroundSlot, 1);
    }

    /* Island sprites. ISLETEMP.SCR only covers the day+high-tide case;
     * night loads NIGHT.SCR (no island baked in) and low-tide needs the
     * bigger shore. In either case we replay islandInit's sprite
     * sequence: 0 island, 13 trunk, 12 leafs, 14 palm shadow, then
     * low-tide extras (1 shore, 2 rock) if applicable. Sprite positions
     * match islandInit exactly. */
    if (islandState.night || islandState.lowTide) {
        grDrawSprite(grBackgroundSfc, &ttmBackgroundSlot, 288, 279,  0, 0);
        grDrawSprite(grBackgroundSfc, &ttmBackgroundSlot, 442, 148, 13, 0);
        grDrawSprite(grBackgroundSfc, &ttmBackgroundSlot, 365, 122, 12, 0);
        grDrawSprite(grBackgroundSfc, &ttmBackgroundSlot, 396, 279, 14, 0);
        if (islandState.lowTide) {
            grDrawSprite(grBackgroundSfc, &ttmBackgroundSlot, 249, 303,  1, 0);
            grDrawSprite(grBackgroundSfc, &ttmBackgroundSlot, 150, 328,  2, 0);
        }
    }

    /* Holiday overlay. We preload it into slot 2 so adsPilotStampHoliday can
     * restamp the sprite after Johnny and preserve the original overlay
     * z-order on this single-layer bg-tile path. */
    if (holidayById(islandState.holiday)) {
        grLoadBmp(&ttmBackgroundSlot, 2, "HOLIDAY.BMP");
    }

    /* Seed the clean baseline with the initial four shore wave positions,
     * mirroring what islandInit does in the normal ads path. All four
     * positions must be painted BEFORE the clean backup so that each
     * per-frame restore brings the waves back along with everything else. */
    for (int i = 0; i < 4; i++)
        islandAnimate(&ttmBackgroundThread);

    /* Clean-rect save is deferred to adsPilotSaveCleanBgRectsForPack — the
     * caller invokes it after the foreground pack header is loaded so the
     * rect can be sized to the pack's union bbox (varies per scene;
     * fishing3's catch arc rises to y=143, fishing1's cast arc only to
     * y=195, etc.). */
}

/* Union the foreground pack's bbox with the shoreline wave-sprite region
 * (x=129..608, y=303..356 for high/low tide) and save as the rect-mode
 * clean backup. Call this after adsPilotEnableWaveBackdrop has seeded the
 * initial wave positions into the tiles AND after foregroundPilotRuntimeStart
 * has loaded the pack header so fgX/fgY/fgW/fgH are known.
 *
 * Split the coverage into up to two rects when the pack bbox reaches
 * above y=190 (catch arc etc.): one narrow strip for the upper region,
 * one wider rect for the Johnny + wave area. This keeps the largest
 * single contiguous alloc under ~225 KB even for scenes whose union is
 * 268+ KB — the screensaver loop fragments the heap and failed the
 * one-shot 268 KB alloc by iteration 3. */
int adsPilotSaveCleanBgRectsForPack(sint16 fgX, sint16 fgY, uint16 fgW, uint16 fgH)
{
    const sint16 kWaveMinX = 129;
    const sint16 kWaveMinY = 303;
    const sint16 kWaveEndX = 608;
    const sint16 kWaveEndY = 356;
    const sint16 kUpperSplitY = 190;

    sint16 fgEndX = (sint16)(fgX + fgW);
    sint16 fgEndY = (sint16)(fgY + fgH);

    if (fgW == 0 || fgH == 0) {
        /* Degenerate / unknown pack bbox — fall back to the wave region
         * alone, matching the legacy single-rect behavior. */
        sint16 xs[1]; sint16 ys[1]; uint16 ws[1]; uint16 hs[1];
        xs[0] = kWaveMinX;
        ys[0] = kWaveMinY;
        ws[0] = (uint16)(kWaveEndX - kWaveMinX);
        hs[0] = (uint16)(kWaveEndY - kWaveMinY);
        return grSaveCleanBgRects(xs, ys, ws, hs, 1) == 1;
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

        if (upperMinX < 0) upperMinX = 0;
        if (upperMinY < 0) upperMinY = 0;
        if (upperEndX > 640) upperEndX = 640;
        if (upperEndY > 480) upperEndY = 480;

        sint16 xs[2]; sint16 ys[2]; uint16 ws[2]; uint16 hs[2];
        if (upperEndX <= upperMinX || upperEndY <= upperMinY ||
            lowerEndX <= lowerMinX || lowerEndY <= lowerMinY)
            return 0;
        /* Keep the large lower rect in slot 0 so fishing1/fishing3 reuse the
         * same high-water clean buffer instead of alternating large slots. */
        xs[0] = lowerMinX;
        ys[0] = lowerMinY;
        ws[0] = (uint16)(lowerEndX - lowerMinX);
        hs[0] = (uint16)(lowerEndY - lowerMinY);
        xs[1] = upperMinX;
        ys[1] = upperMinY;
        ws[1] = (uint16)(upperEndX - upperMinX);
        hs[1] = (uint16)(upperEndY - upperMinY);
        return grSaveCleanBgRects(xs, ys, ws, hs, 2) == 2;
    } else {
        sint16 xs[1]; sint16 ys[1]; uint16 ws[1]; uint16 hs[1];
        if (lowerEndX <= lowerMinX || lowerEndY <= lowerMinY)
            return 0;
        xs[0] = lowerMinX;
        ys[0] = lowerMinY;
        ws[0] = (uint16)(lowerEndX - lowerMinX);
        hs[0] = (uint16)(lowerEndY - lowerMinY);
        return grSaveCleanBgRects(xs, ys, ws, hs, 1) == 1;
    }
}

/* Per-frame wave tick for the fgpilot main loop — mirrors the tick block
 * inside the normal adsPlay loop. Advances the wave frame on the delay
 * clock; between advances redraws the last wave so it remains visible
 * after each grRestoreBgTiles. */
void adsPilotTickBackgroundWaves(void)
{
    if (!ttmBackgroundThread.isRunning)
        return;

    if (ttmBackgroundThread.timer == 0) {
        ttmBackgroundThread.timer = ttmBackgroundThread.delay;
        islandAnimate(&ttmBackgroundThread);
    } else {
        islandRedrawWave(&ttmBackgroundThread);
        ttmBackgroundThread.timer--;
    }
}

/* Stamp the holiday sprite onto bg tiles. Called per-frame AFTER the
 * foreground pack has been composited, so the sprite appears on top of
 * Johnny (matching islandInitHoliday's separate-layer behaviour in the
 * normal ads path). No-op when islandState.holiday is 0 or when
 * HOLIDAY.BMP failed to preload. Sprite positions match
 * islandInitHoliday. */
void adsPilotStampHoliday(void)
{
    const struct Holiday *holiday = holidayById(islandState.holiday);

    if (!holiday)
        return;
    if (ttmBackgroundSlot.numSprites[2] == 0)
        return;

    grDx = islandState.xPos;
    grDy = islandState.yPos;
    grDrawSprite(grBackgroundSfc, &ttmBackgroundSlot,
                 holiday->island_x, holiday->island_y,
                 (uint16)holiday->sprite_index, 2);
}

#endif
