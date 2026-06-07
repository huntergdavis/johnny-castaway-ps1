static void fpTickAsset(struct TFpAsset *asset)
{
    if (!asset->active)
        return;
    if (asset->ttl > 0)
        asset->ttl--;
    if (asset->ttl == 0) {
        fpCancelAsset(asset);
        return;
    }
    asset->x = (sint16)(asset->x + asset->vx);
    asset->y = (sint16)(asset->y + asset->vy);
    asset->frameTimer++;
    if (asset->frameTimer >= asset->frameDelay) {
        asset->frameTimer = 0;
        if (asset->frameCount > 1) {
            if (asset->loop) {
                if (asset->frame + 1 < asset->frameCount)
                    asset->frame++;
                else
                    asset->frame = asset->loopStart;  /* skip intro on wrap */
            } else if (asset->frame + 1 < asset->frameCount) {
                asset->frame++;
            }
        }
    }
}

static void fpTick(void)
{
    fpTickAsset(&gFp.gag);
    fpTickAsset(&gFp.summon);

    if (gFp.modeTimer > 0) {
        gFp.modeTimer--;
        if (gFp.modeTimer == 0)
            gFp.mode = FP_MODE_IDLE;
    }
    if (gFp.bannerTimer > 0)
        gFp.bannerTimer--;

    if (gFp.mode == FP_MODE_IDLE)
        gFp.idleFrames++;

    if (gFp.fireActive) {
        gFp.fireTimer++;
        if (gFp.fireTimer >= 6) {
            gFp.fireTimer = 0;
            gFp.fireFrame++;
        }
    }

}

static void fpDrawAsset(const struct TFpAsset *asset)
{
    uint16 drawFrame;
    int slotFrames;

    if (!asset->active)
        return;
    if (asset->slot >= MAX_BMP_SLOTS)
        return;
    slotFrames = gFpSlot.numSprites[asset->slot];
    if (slotFrames <= 0)
        return;
    drawFrame = (uint16)(asset->firstFrame + asset->frame);
    if (drawFrame >= (uint16)slotFrames)
        drawFrame = (uint16)(slotFrames - 1);

    if (asset->screenRelative)
        fpSetScreenDrawOffset();
    else
        fpSetIslandDrawOffset();

    /* Resolve the sprite top-left x for this draw.
     *
     * Legacy mode (bodyAnchorX == 0): asset->x is the sprite top-left
     * directly — caller pre-computed any anchor offset. This is what
     * gags, summons, and the persistent fire all use.
     *
     * Width-aware mode (bodyAnchorX > 0): asset->x is the screen x
     * where Johnny's *body* should land, and bodyAnchorX is how many
     * pixels from sprite-left the body sits in the unflipped sprite.
     * Different frames have different fullWidths, so we recompute the
     * top-left per frame to pin the body in place. Used by fishing
     * because MJFISH1 frame widths span 64..152 px and a single fixed
     * fx makes the body drift visibly across the loop. */
    sint16 drawX = asset->x;
    if (asset->bodyAnchorX > 0) {
        PS1Surface *spr = gFpSlot.sprites[asset->slot][drawFrame];
        if (spr != NULL) {
            if (asset->flip)
                drawX = (sint16)(asset->x - (sint16)spr->fullWidth + asset->bodyAnchorX);
            else
                drawX = (sint16)(asset->x - asset->bodyAnchorX);
        }
    }

    if (asset->flip)
        grDrawSpriteFlip(grBackgroundSfc, &gFpSlot,
                         drawX, asset->y,
                         drawFrame, asset->slot);
    else
        grDrawSprite(grBackgroundSfc, &gFpSlot,
                     drawX, asset->y,
                     drawFrame, asset->slot);
}

static void fpDrawFire(void)
{
    char bmp[12];
    if (!gFp.fireActive)
        return;
    snprintf(bmp, sizeof(bmp), "FIRE%u.BMP", (unsigned)((gFp.fireFrame % 5) + 1));
    if (!fpLoadSlot(FP_SLOT_PERSIST, bmp))
        return;
    fpSetIslandDrawOffset();
    grDrawSprite(grBackgroundSfc, &gFpSlot,
                 gFp.fireX, gFp.fireY, 0, FP_SLOT_PERSIST);
}

static void fpDrawJohnny(void)
{
    if (gFp.gag.active) {
        fpDrawAsset(&gFp.gag);
        fpRestampTreeIfNeeded();
        return;
    }

    fpSetIslandDrawOffset();
    if (gFp.mode == FP_MODE_WALK || gFp.mode == FP_MODE_STRUT) {
        int flip = (gFp.face == FP_FACE_W) ? 1 : 0;
        walkRenderFrame(grBackgroundSfc, &gFpSlot, fgBackdropGetSlot(),
                        gFp.x, gFp.y, fpWalkFrame(), flip,
                        fpBehindTree(gFp.x, gFp.y));
    } else {
        int flip = (gFp.face == FP_FACE_W) ? 1 : 0;
        if (flip)
            grDrawSpriteFlip(grBackgroundSfc, &gFpSlot,
                             gFp.x, gFp.y, fpIdleFrame(), FP_SLOT_JOHN);
        else
            grDrawSprite(grBackgroundSfc, &gFpSlot,
                         gFp.x, gFp.y, fpIdleFrame(), FP_SLOT_JOHN);
        fpRestampTreeIfNeeded();
    }
}

static void fpTextStart(int x, int y)
{
    fpTextX = x;
    fpTextY = y;
}

static void fpTextChar(char c)
{
    int idx;
    int col;
    int row;
    SPRT *sprt;

    if (c == '\n') {
        fpTextX = 24;
        fpTextY += PAUSE_GLYPH_DRAW_H + 2;
        return;
    }
    if (c < PAUSE_GLYPH_FIRST ||
        c >= (PAUSE_GLYPH_FIRST + PAUSE_GLYPH_COUNT)) {
        c = '?';
    }
    if (fpOverlayNext + sizeof(SPRT) >= fpOverlayPrim + FP_OVERLAY_PRIM_BYTES)
        return;

    idx = c - PAUSE_GLYPH_FIRST;
    col = idx & 15;
    row = idx >> 4;
    sprt = (SPRT *)fpOverlayNext;
    fpOverlayNext += sizeof(SPRT);
    setSprt(sprt);
    setXY0(sprt, fpTextX, fpTextY);
    setWH(sprt, PAUSE_GLYPH_DRAW_W, PAUSE_GLYPH_DRAW_H);
    setUV0(sprt, (uint8)(col * PAUSE_GLYPH_DRAW_W),
           (uint8)(row * PAUSE_GLYPH_DRAW_H));
    setClut(sprt, PAUSE_CLUT_VRAM_X, PAUSE_CLUT_VRAM_Y);
    setRGB0(sprt, 128, 128, 128);
    ps1GpuOtAddPrim(fpOverlayTextOt, sprt);
    fpTextX += PAUSE_GLYPH_DRAW_W;
}

static void fpText(const char *text)
{
    while (text && *text)
        fpTextChar(*text++);
}

static void fpBuildOverlay(void)
{
    DR_TPAGE *tp;
    POLY_F4 *panel;

    ClearOTagR(fpOverlayOt, FP_OVERLAY_OT_LEN);
    fpOverlayNext = fpOverlayPrim;
    fpOverlayTextOt = &fpOverlayOt[FP_OVERLAY_OT_LEN - 4];

    tp = (DR_TPAGE *)fpOverlayNext;
    fpOverlayNext += sizeof(DR_TPAGE);
    setDrawTPage(tp, 0, 1, getTPage(0, 0, PAUSE_FONT_VRAM_X, PAUSE_FONT_VRAM_Y));
    ps1GpuOtAddPrim(&fpOverlayOt[FP_OVERLAY_OT_LEN - 1], tp);

    if (gFp.bannerTimer > 0) {
        panel = (POLY_F4 *)fpOverlayNext;
        fpOverlayNext += sizeof(POLY_F4);
        setPolyF4(panel);
        setSemiTrans(panel, 1);
        setRGB0(panel, 0, 0, 0);
        setXY4(panel,
               FP_BANNER_X0, FP_BANNER_Y0,
               FP_BANNER_X1, FP_BANNER_Y0,
               FP_BANNER_X0, FP_BANNER_Y1,
               FP_BANNER_X1, FP_BANNER_Y1);
        ps1GpuOtAddPrim(&fpOverlayOt[FP_OVERLAY_OT_LEN - 3], panel);
        fpTextStart(210, 38);
        fpText(gFp.banner);
    }

    if (gFreeplayTelemetryLevel >= 3) {
        char line[64];
        snprintf(line, sizeof(line), "FP %s X%d Y%d R%lu A%lu",
                 fpModeName(gFp.mode), (int)gFp.x, (int)gFp.y,
                 (unsigned long)gFp.cleanRebuilds,
                 (unsigned long)gFp.assetLoadFailures);
        fpTextStart(12, 204);
        fpText(line);
    }
}

static int fpOverlayVisible(void)
{
    return (gFp.bannerTimer > 0 ||
            gFreeplayTelemetryLevel >= 3 ||
            captionsIsVisible()) ? 1 : 0;
}

static void fpMarkOverlayDirty(void)
{
    int visible = fpOverlayVisible();
    if (visible || gFp.overlayWasVisible) {
        grMarkScreenRectDirty(FP_BANNER_X0 - 4, FP_BANNER_Y0 - 4,
                              FP_BANNER_X1 + 4, FP_BANNER_Y1 + 4);
        grMarkScreenRectDirty(FP_CAPTION_X0, FP_CAPTION_Y0,
                              FP_CAPTION_X1, FP_CAPTION_Y1);
    }
    gFp.overlayWasVisible = (uint8)visible;
}

static void fpPresent(void)
{
    ps1FrameCount++;
    VSync(0);
    grDrawBackground();
    if (gFp.bannerTimer > 0 || gFreeplayTelemetryLevel >= 3) {
        pauseMenuEnsureFontUploaded();
        fpBuildOverlay();
        DrawOTag(&fpOverlayOt[FP_OVERLAY_OT_LEN - 1]);
    }
    if (ps1CaptionsEnabled)
        captionsRender();
    DrawSync(0);
}

static void fpFrame(void)
{
    grBeginFrame();
    grRestoreBgFromRects();
    /* Wave tick rate matches the FG2 backdrop thread (delay=8 in
     * islandInit), i.e. once every 8 frames. The previous & 3UL
     * (every 4 frames) ran the ocean at 2x speed in freeplay. */
    if ((gFp.frame & 7UL) == 0)
        fgBackdropTickWavesPublic();
    fpDrawFire();
    fpDrawAsset(&gFp.summon);
    fpDrawJohnny();
    fgBackdropStampHolidayPublic();
    fpMarkOverlayDirty();
    fpPresent();
}
