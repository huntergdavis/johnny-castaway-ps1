static int fpLoadSlot(uint8 slot, const char *bmp)
{
    if (bmp == NULL || bmp[0] == '\0')
        return 0;
    if (slot >= MAX_BMP_SLOTS)
        return 0;
    if (gFpSlot.numSprites[slot] > 0 &&
        gFpSlot.loadedBmpNames[slot] != NULL &&
        strcmp(gFpSlot.loadedBmpNames[slot], bmp) == 0) {
        return 1;
    }
    if (gFpSlot.numSprites[slot] > 0)
        grReleaseBmp(&gFpSlot, slot);
    grLoadBmp(&gFpSlot, slot, (char *)bmp);
    if (gFpSlot.numSprites[slot] == 0) {
        gFp.assetLoadFailures++;
        FP_LOG(2, ("JCFREE asset-fail slot=%u bmp=%s failures=%lu\n",
                   (unsigned)slot, bmp, (unsigned long)gFp.assetLoadFailures));
        return 0;
    }
    FP_LOG(3, ("JCFREE asset slot=%u bmp=%s frames=%u\n",
               (unsigned)slot, bmp, (unsigned)gFpSlot.numSprites[slot]));
    return 1;
}

static void fpCancelAsset(struct TFpAsset *asset)
{
    if (asset == NULL)
        return;
    if (!asset->active) {
        memset(asset, 0, sizeof(*asset));
        return;
    }
    if (asset->slot < MAX_BMP_SLOTS && gFpSlot.numSprites[asset->slot] > 0)
        grReleaseBmp(&gFpSlot, asset->slot);
    memset(asset, 0, sizeof(*asset));
}

static void fpReleaseSlots(void)
{
    for (uint8 i = 0; i < MAX_BMP_SLOTS; i++) {
        if (gFpSlot.numSprites[i] > 0)
            grReleaseBmp(&gFpSlot, i);
    }
}

static void fpSetIslandDrawOffset(void)
{
    grDx = islandState.xPos;
    grDy = islandState.yPos;
}

static void fpSetScreenDrawOffset(void)
{
    grDx = 0;
    grDy = 0;
}

static int fpBehindTree(sint16 x, sint16 y)
{
    return (x >= 430 && x <= 478 && y >= 184 && y <= 238) ? 1 : 0;
}

static uint16 fpIdleFrame(void)
{
    switch (gFp.face) {
    case FP_FACE_W: return FP_JOHN_IDLE_W;
    case FP_FACE_N: return FP_JOHN_IDLE_N;
    case FP_FACE_S: return FP_JOHN_IDLE_S;
    case FP_FACE_E:
    default: return FP_JOHN_IDLE_E;
    }
}

static uint16 fpWalkFrame(void)
{
    uint16 idx;
    switch (gFp.face) {
    case FP_FACE_W:
        idx = (uint16)((gFp.walkFrame / 4) %
                       (sizeof(kFpWalkWestFrames) / sizeof(kFpWalkWestFrames[0])));
        return kFpWalkWestFrames[idx];
    case FP_FACE_N:
        idx = (uint16)((gFp.walkFrame / 5) %
                       (sizeof(kFpWalkNorthFrames) / sizeof(kFpWalkNorthFrames[0])));
        return kFpWalkNorthFrames[idx];
    case FP_FACE_S:
        idx = (uint16)((gFp.walkFrame / 5) %
                       (sizeof(kFpWalkSouthFrames) / sizeof(kFpWalkSouthFrames[0])));
        return kFpWalkSouthFrames[idx];
    case FP_FACE_E:
    default:
        idx = (uint16)((gFp.walkFrame / 4) %
                       (sizeof(kFpWalkEastFrames) / sizeof(kFpWalkEastFrames[0])));
        return kFpWalkEastFrames[idx];
    }
}

static void fpRestampTreeIfNeeded(void)
{
    struct TTtmSlot *bgSlot;
    if (!fpBehindTree(gFp.x, gFp.y))
        return;
    bgSlot = fgBackdropGetSlot();
    if (bgSlot == NULL)
        return;
    fpSetIslandDrawOffset();
    grDrawSprite(grBackgroundSfc, bgSlot, 442, 148, 13, 0);
    grDrawSprite(grBackgroundSfc, bgSlot, 365, 122, 12, 0);
}

static int fpSaveCleanRects(void)
{
    const uint32 kMaxCleanRectBytes = 96UL * 1024UL;
    sint16 islandX = (sint16)islandState.xPos;
    sint16 islandY = (sint16)islandState.yPos;
    sint16 xs[FP_CLEAN_RECT_COUNT];
    sint16 ys[FP_CLEAN_RECT_COUNT];
    uint16 ws[FP_CLEAN_RECT_COUNT];
    uint16 hs[FP_CLEAN_RECT_COUNT];
    int rc;

    xs[0] = (sint16)(islandX + 129);
    ys[0] = (sint16)(islandY + 180);
    ws[0] = 479;
    hs[0] = 220; /* island, Johnny, surf */

    xs[1] = (sint16)(islandX + 240);
    ys[1] = (sint16)(islandY + 80);
    ws[1] = 400;
    hs[1] = 128; /* sky/summons */

    xs[2] = (sint16)(islandX + 0);
    ys[2] = (sint16)(islandY + 120);
    ws[2] = 260;
    hs[2] = 96; /* upper-left Johnny smear */

    rc = grSaveCleanBgRectsSplit(xs, ys, ws, hs, FP_CLEAN_RECT_COUNT,
                                 kMaxCleanRectBytes);
    if (rc <= 0) {
        gFp.cleanFailures++;
        FP_LOG(1, ("JCFREE clean-fail rc=%d source=%d failures=%lu heapKB=%lu\n",
                   rc, FP_CLEAN_RECT_COUNT,
                   (unsigned long)gFp.cleanFailures,
                   fgProbeLargestAlloc() / 1024UL));
        return 0;
    }
    gFp.cleanRebuilds++;
    FP_LOG(2, ("JCFREE clean-ok rebuild=%lu bytes=%lu heapKB=%lu\n",
               (unsigned long)gFp.cleanRebuilds,
               grCleanBgRectsBytes(),
               fgProbeLargestAlloc() / 1024UL));
    return 1;
}

static void fpStampPersistents(void)
{
    int wantCastle = 0;
    int wantCoconut = 0;

    for (int i = 0; i < (int)(sizeof(gFp.persist) / sizeof(gFp.persist[0])); i++) {
        if (!gFp.persist[i].active)
            continue;
        if (gFp.persist[i].kind == FP_PERSIST_CASTLE)
            wantCastle = 1;
        else if (gFp.persist[i].kind == FP_PERSIST_COCONUT)
            wantCoconut = 1;
    }

    fpSetIslandDrawOffset();

    if (gFp.fireActive) {
        /* Fire is animated, not baked into the baseline. */
    }

    if (wantCastle && fpLoadSlot(FP_SLOT_PERSIST, "GJCASTLE.BMP")) {
        for (int i = 0; i < (int)(sizeof(gFp.persist) / sizeof(gFp.persist[0])); i++) {
            if (!gFp.persist[i].active || gFp.persist[i].kind != FP_PERSIST_CASTLE)
                continue;
            grDrawSprite(grBackgroundSfc, &gFpSlot,
                         gFp.persist[i].x, gFp.persist[i].y,
                         gFp.persist[i].frame, FP_SLOT_PERSIST);
        }
    }

    if (wantCoconut && fpLoadSlot(FP_SLOT_PERSIST, "COCONUTS.BMP")) {
        for (int i = 0; i < (int)(sizeof(gFp.persist) / sizeof(gFp.persist[0])); i++) {
            if (!gFp.persist[i].active || gFp.persist[i].kind != FP_PERSIST_COCONUT)
                continue;
            grDrawSprite(grBackgroundSfc, &gFpSlot,
                         gFp.persist[i].x, gFp.persist[i].y,
                         gFp.persist[i].frame, FP_SLOT_PERSIST);
        }
    }

    if (gFpSlot.numSprites[FP_SLOT_PERSIST] > 0)
        grReleaseBmp(&gFpSlot, FP_SLOT_PERSIST);
}

static int fpRebuildBackdrop(void)
{
    grDeactivateCleanBgRects();
    fgBackdropPrepareIslandRuntimePublic();
    fpStampPersistents();
    fgBackdropStampHolidayPublic();
    if (!fpSaveCleanRects())
        return 0;
    grForceFullRedrawNextFrame();
    return 1;
}

static uint16 fpReadButtons(void)
{
    uint16 buttons = 0;
    for (int i = 0; i < 2; i++) {
        PADTYPE *pad = (PADTYPE *)pad_buff[i];
        buttons |= ps1PadButtonsWithAnalog(pad);
    }
    return buttons;
}

static int fpClampJohnny(void)
{
    sint16 oldX = gFp.x;
    sint16 oldY = gFp.y;
    if (gFp.x < 245) gFp.x = 245;
    if (gFp.x > 535) gFp.x = 535;
    if (gFp.y < 205) gFp.y = 205;
    if (gFp.y > 322) gFp.y = 322;
    if (gFp.y < 235 && gFp.x < 300) gFp.x = 300;
    if (gFp.y < 220 && gFp.x > 500) gFp.x = 500;
    if (oldX != gFp.x || oldY != gFp.y) {
        gFp.clampCount++;
        return 1;
    }
    return 0;
}

static void fpSetBanner(const char *text, uint16 frames)
{
    strncpy(gFp.banner, text, sizeof(gFp.banner) - 1);
    gFp.banner[sizeof(gFp.banner) - 1] = '\0';
    gFp.bannerTimer = frames;
}

static void fpCancelTransientAction(void)
{
    fpCancelAsset(&gFp.gag);
    fpCancelAsset(&gFp.summon);
    gFp.mode = FP_MODE_IDLE;
    gFp.modeTimer = 0;
    gFp.idleFrames = 0;
    grForceFullRedrawNextFrame();
}
