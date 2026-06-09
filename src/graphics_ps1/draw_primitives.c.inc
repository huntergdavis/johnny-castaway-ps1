void grSetClipZone(PS1Surface *sfc, sint16 x1, sint16 y1, sint16 x2, sint16 y2)
{
    /* Set clip rectangle in draw environment */
    draw[db].clip.x = x1;
    draw[db].clip.y = y1;
    draw[db].clip.w = x2 - x1;
    draw[db].clip.h = y2 - y1;
    PutDrawEnv(&draw[db]);
}

/*
 * Draw pixel
 */
void grDrawPixel(PS1Surface *sfc, sint16 x, sint16 y, uint8 color)
{
    /* Pixel opcodes are not used by the PS1 scene path. Keep the API present
     * for upstream compatibility without queuing unreachable GPU primitives. */
    (void)sfc;
    (void)x;
    (void)y;
    (void)color;
}

/*
 * Draw line — software composite to background tiles.
 * (GPU primitives are reserved for the sprite OT rendered by DrawOTag.)
 */
void grDrawLine(PS1Surface *sfc, sint16 x1, sint16 y1, sint16 x2, sint16 y2, uint8 color)
{
    /* TTM line draws are rare and cosmetic (e.g. fishing line). Previously
     * these GPU primitives were silently accumulated but never rendered
     * because this path does not submit a DrawOTag. */
    (void)sfc; (void)x1; (void)y1; (void)x2; (void)y2; (void)color;
}

/*
 * Draw filled rectangle — software composite to background tiles.
 * Used by TTM DRAW_RECT opcode for screen clears and overlays.
 */
static void grDrawRectColor15(sint16 x, sint16 y, uint16 width, uint16 height, uint16 bgColor)
{
    /* Software fill directly into bgTile buffers (matching composite approach).
     * This replaces the GPU POLY_F3 path that was never rendered before. */
    sint16 x2 = x + (sint16)width;
    sint16 y2 = y + (sint16)height;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x2 > 640) x2 = 640;
    if (y2 > 480) y2 = 480;
    if (x >= x2 || y >= y2) return;

    /* Mark dirty region */
    grMarkRectDirty(x, y, x2, y2);

    /* Pack two pixels into uint32 for word-fill */
    uint32 bgColor32 = (uint32)bgColor | ((uint32)bgColor << 16);

    for (sint16 py = y; py < y2; py++) {
        int tileLocalY = (py < 240) ? py : py - 240;
        PS1Surface *tileLeft = (py < 240) ? bgTile0 : bgTile3;
        PS1Surface *tileRight = (py < 240) ? bgTile1 : bgTile4;

        if (x < 320 && tileLeft && tileLeft->pixels) {
            sint16 fillStart = x;
            sint16 fillEnd = (x2 < 320) ? x2 : 320;
            uint16 *dst = tileLeft->pixels + (tileLocalY * (int)tileLeft->width) + fillStart;
            int fillCount = fillEnd - fillStart;
            /* Word-align: handle odd start pixel */
            if (fillCount > 0 && ((uintptr_t)dst & 2)) {
                *dst++ = bgColor;
                fillCount--;
            }
            /* Fill 2 pixels per uint32 store */
            uint32 *dst32 = (uint32 *)dst;
            while (fillCount >= 2) {
                *dst32++ = bgColor32;
                fillCount -= 2;
            }
            /* Handle trailing pixel */
            if (fillCount) {
                *(uint16 *)dst32 = bgColor;
            }
        }

        if (x2 > 320 && tileRight && tileRight->pixels) {
            sint16 fillStart = (x > 320) ? x : 320;
            sint16 fillEnd = x2;
            uint16 *dst = tileRight->pixels + (tileLocalY * (int)tileRight->width) + (fillStart - 320);
            int fillCount = fillEnd - fillStart;
            if (fillCount > 0 && ((uintptr_t)dst & 2)) {
                *dst++ = bgColor;
                fillCount--;
            }
            uint32 *dst32 = (uint32 *)dst;
            while (fillCount >= 2) {
                *dst32++ = bgColor32;
                fillCount -= 2;
            }
            if (fillCount) {
                *(uint16 *)dst32 = bgColor;
            }
        }
    }
}

void grDrawRect(PS1Surface *sfc, sint16 x, sint16 y, uint16 width, uint16 height, uint8 color)
{
    (void)sfc;
    grDrawRectColor15(x, y, width, height, ttmPalette[color & 0xF]);
}

/*
 * Draw circle (hollow)
 */
void grDrawCircle(PS1Surface *sfc, sint16 x1, sint16 y1, uint16 width, uint16 height,
                  uint8 fgColor, uint8 bgColor)
{
    /* Circle opcodes are retained as a compatibility stub; the PS1 content
     * path composes sprites and rect fills directly into background tiles. */
    (void)sfc;
    (void)x1;
    (void)y1;
    (void)width;
    (void)height;
    (void)fgColor;
    (void)bgColor;
}

/*
 * Draw sprite from BMP slot
 */
static void grRecordReplaySprite(struct TTtmThread *thread,
                                 PS1Surface *sprite, sint16 x, sint16 y,
                                 uint16 spriteNo, uint16 imageNo, uint8 flip)
{
    if (!thread || !sprite || !sprite->indexedPixels) return;

    /* Deduplicate exact same draw within the frame.
     * Fast-path: check last 8 entries first (draws are sequential). */
    int scanStart = (thread->numDrawnSprites > 8) ? thread->numDrawnSprites - 8 : 0;
    for (int pass = 0; pass < 2; pass++) {
        int lo = (pass == 0) ? scanStart : 0;
        int hi = (pass == 0) ? thread->numDrawnSprites : scanStart;
        for (int i = lo; i < hi; i++) {
            struct TDrawnSprite *ds = &thread->drawnSprites[i];
            if (ds->imageNo == imageNo &&
                ds->spriteNo == spriteNo &&
                ds->flip == flip &&
                ds->x == x &&
                ds->y == y &&
                ds->sceneEpoch == thread->sceneEpoch) {
                ds->indexedPixels = sprite->indexedPixels;
                ds->width = sprite->width;
                ds->height = sprite->height;
                ds->psbNibbles = sprite->psbNibbles;
                ds->bmpName = thread->ttmSlot ? thread->ttmSlot->loadedBmpNames[imageNo] : NULL;
                return;
            }
        }
    }

    uint16 recIdx;
    if (thread->numDrawnSprites >= MAX_DRAWN_SPRITES) {
        /* Keep most recent draws when scene density exceeds replay capacity.
         * Dropping new records causes actor vanish (Johnny lost behind props). */
        recIdx = thread->replayWriteCursor;
        thread->replayWriteCursor++;
        if (thread->replayWriteCursor >= MAX_DRAWN_SPRITES)
            thread->replayWriteCursor = 0;
    } else {
        recIdx = thread->numDrawnSprites++;
    }

    struct TDrawnSprite *ds = &thread->drawnSprites[recIdx];
    ds->indexedPixels = sprite->indexedPixels;
    ds->width = sprite->width;
    ds->height = sprite->height;
    ds->x = x;
    ds->y = y;
    ds->spriteNo = spriteNo;
    ds->imageNo = imageNo;
    ds->sceneEpoch = thread->sceneEpoch;
    ds->flip = flip;
    ds->psbNibbles = sprite->psbNibbles;
    ds->bmpName = thread->ttmSlot ? thread->ttmSlot->loadedBmpNames[imageNo] : NULL;
}

void grDrawSprite(PS1Surface *sfc, struct TTtmSlot *ttmSlot, sint16 x, sint16 y,
                  uint16 spriteNo, uint16 imageNo)
{
    x += grDx;
    y += grDy;

    /* Validate imageNo bounds */
    if (imageNo >= MAX_BMP_SLOTS || ttmSlot->numSprites[imageNo] == 0) {
        return;
    }

    /* Wrap sprite index to available frames (handles frame cap) */
    uint16 actualSpriteNo = spriteNo % ttmSlot->numSprites[imageNo];

    PS1Surface *sprite = ttmSlot->sprites[imageNo][actualSpriteNo];
    if (sprite == NULL) {
        return;
    }

    /* RAM-based sprites (loaded via grLoadBmpRAM) have x=0, y=0 with valid pixel data. */
    if (sprite->x == 0 && sprite->y == 0 && (sprite->pixels != NULL || sprite->indexedPixels != NULL)) {
        grCompositeToBackground(sprite, x, y);
        grRecordReplaySprite(grCurrentThread, sprite, x, y, spriteNo, imageNo, 0);
        return;
    }

    /* Draw all tiles in this sprite's linked list */
    PS1Surface *tile = sprite;
    while (tile != NULL) {
        /* Allocate DR_TPAGE + SPRT primitives from buffer */
        if (primitiveIndex[db] + sizeof(DR_TPAGE) + sizeof(SPRT) > PRIMITIVE_BUFFER_SIZE) {
            GR_DIAG_PRINTF("Warning: Primitive buffer full!\n");
            return;
        }

        /* Calculate screen position for this tile */
        sint16 tileX = x + tile->tileOffsetX;
        sint16 tileY = y + tile->tileOffsetY;

        /* Add texture page primitive first - tells GPU where texture data is */
        DR_TPAGE *tpage = (DR_TPAGE*)nextPrimitive[db];
        nextPrimitive[db] += sizeof(DR_TPAGE);
        primitiveIndex[db] += sizeof(DR_TPAGE);

        /* Calculate texture page from tile VRAM position
         * tpage X: in 64-pixel units (tile->x / 64)
         * tpage Y: in 256-pixel units (tile->y / 256)
         * Color mode: 0 = 4-bit CLUT (16 colors) */
        uint16 tpageX = tile->x / 64;
        uint16 tpageY = tile->y / 256;
        setDrawTPage(tpage, 0, 0, getTPage(0, 0, tpageX * 64, tpageY * 256));
        ps1GpuOtAddPrim(&ot[db][0], tpage);

        SPRT *sprt = (SPRT*)nextPrimitive[db];
        nextPrimitive[db] += sizeof(SPRT);
        primitiveIndex[db] += sizeof(SPRT);

        /* Initialize sprite primitive */
        setSprt(sprt);
        setXY0(sprt, tileX, tileY);
        setWH(sprt, tile->width, tile->height);
        /* UV coords are relative to texture page (0-255 range)
         * For 4-bit textures: U = ((vram_x % 64) * 4) & 0xFF
         * This is because each texture page is 64 VRAM pixels = 256 texture pixels */
        setUV0(sprt, ((tile->x % 64) * 4) & 0xFF, (tile->y % 256) & 0xFF);
        setClut(sprt, tile->clutX, tile->clutY);
        setRGB0(sprt, 128, 128, 128);  /* Normal brightness */

        /* Add to ordering table */
        ps1GpuOtAddPrim(&ot[db][0], sprt);

        GR_DIAG_PRINTF("Draw tile: pos=(%d,%d) size=%dx%d VRAM=(%d,%d)\n",
                       tileX, tileY, tile->width, tile->height, tile->x, tile->y);

        tile = tile->nextTile;
    }
}

/*
 * Composite sprite to background tiles with horizontal flip
 */
void __attribute__((optimize("Os")))
grCompositeToBackgroundFlip(PS1Surface *sprite, sint16 screenX, sint16 screenY)
{
    if (!sprite) return;
    if (!sprite->pixels && !sprite->indexedPixels) return;

    int sprW = sprite->width;
    int sprH = sprite->height;

    /* Sanity check - prevent hang from corrupt/freed sprite data */
    if (sprW == 0 || sprH == 0 || sprW > 640 || sprH > 480) return;

    /* Choose indexed or direct color path */
    int useIndexed = (sprite->indexedPixels != NULL);
    int usePsb = (useIndexed && sprite->psbNibbles);

    int startSy = 0;
    int endSy = sprH;
    if (screenY < 0) startSy = -screenY;
    if (screenY + endSy > 480) endSy = 480 - screenY;
    if (startSy >= endSy) return;

    int startDestX = screenX < 0 ? 0 : screenX;
    int endDestX = screenX + sprW;
    if (endDestX > 640) endDestX = 640;
    if (startDestX >= endDestX) return;

    /* Mark dirty region for this flipped sprite */
    grMarkRectDirty(startDestX, screenY + startSy, endDestX, screenY + endSy);

    const uint16 *pal = ttmPalette;

    /* Iterate over each visible pixel in the sprite (flipped horizontally) */
    for (int sy = startSy; sy < endSy; sy++) {
        int destY = screenY + sy;

        /* Determine tile row once per scanline */
        PS1Surface *tileLeft, *tileRight;
        int tileLocalY;
        if (destY < 240) {
            tileLocalY = destY;
            tileLeft = bgTile0;
            tileRight = bgTile1;
        } else {
            tileLocalY = destY - 240;
            tileLeft = bgTile3;
            tileRight = bgTile4;
        }

        uint16 *rowLeft = (tileLeft && tileLeft->pixels) ? (tileLeft->pixels + tileLocalY * (int)tileLeft->width) : NULL;
        uint16 *rowRight = (tileRight && tileRight->pixels) ? (tileRight->pixels + tileLocalY * (int)tileRight->width) : NULL;
        uint32 srcRowBase = (uint32)sy * (uint32)sprW;

        if (usePsb) {
            /* PSB (PS1 nibble order): pre-transcoded, no runtime swap */
            if (rowLeft && startDestX < 320) {
                int lx0 = startDestX;
                int lx1 = (endDestX < 320) ? endDestX : 320;
                int srcX = sprW - 1 - (lx0 - screenX);
                int span = lx1 - lx0;
                compositePsbSpanRev(rowLeft + lx0, sprite->indexedPixels,
                                    srcRowBase + (uint32)srcX, span, pal);
            }

            if (rowRight && endDestX > 320) {
                int rx0 = (startDestX > 320) ? startDestX : 320;
                int rx1 = endDestX;
                int srcX = sprW - 1 - (rx0 - screenX);
                int span = rx1 - rx0;
                compositePsbSpanRev(rowRight + (rx0 - 320), sprite->indexedPixels,
                                    srcRowBase + (uint32)srcX, span, pal);
            }
        } else if (useIndexed) {
            if (rowLeft && startDestX < 320) {
                int lx0 = startDestX;
                int lx1 = (endDestX < 320) ? endDestX : 320;
                int srcX = sprW - 1 - (lx0 - screenX);
                int span = lx1 - lx0;
                compositeIndexedSpanRev(rowLeft + lx0, sprite->indexedPixels,
                                        srcRowBase + (uint32)srcX, span, pal);
            }

            if (rowRight && endDestX > 320) {
                int rx0 = (startDestX > 320) ? startDestX : 320;
                int rx1 = endDestX;
                int srcX = sprW - 1 - (rx0 - screenX);
                int span = rx1 - rx0;
                compositeIndexedSpanRev(rowRight + (rx0 - 320), sprite->indexedPixels,
                                        srcRowBase + (uint32)srcX, span, pal);
            }
        } else {
            if (rowLeft && startDestX < 320) {
                int lx0 = startDestX;
                int lx1 = (endDestX < 320) ? endDestX : 320;
                int srcX = sprW - 1 - (lx0 - screenX);
                int dx = lx0;
                for (; dx + 1 < lx1; dx += 2, srcX -= 2) {
                    uint16 p0 = sprite->pixels[srcRowBase + (uint32)srcX];
                    uint16 p1 = sprite->pixels[srcRowBase + (uint32)srcX - 1];
                    if (p0 != 0x0000) rowLeft[dx] = p0;
                    if (p1 != 0x0000) rowLeft[dx + 1] = p1;
                }
                if (dx < lx1) {
                    uint16 p = sprite->pixels[srcRowBase + (uint32)srcX];
                    if (p != 0x0000) rowLeft[dx] = p;
                }
            }

            if (rowRight && endDestX > 320) {
                int rx0 = (startDestX > 320) ? startDestX : 320;
                int rx1 = endDestX;
                int srcX = sprW - 1 - (rx0 - screenX);
                int dx = rx0;
                for (; dx + 1 < rx1; dx += 2, srcX -= 2) {
                    uint16 p0 = sprite->pixels[srcRowBase + (uint32)srcX];
                    uint16 p1 = sprite->pixels[srcRowBase + (uint32)srcX - 1];
                    if (p0 != 0x0000) rowRight[dx - 320] = p0;
                    if (p1 != 0x0000) rowRight[dx - 319] = p1;
                }
                if (dx < rx1) {
                    uint16 p = sprite->pixels[srcRowBase + (uint32)srcX];
                    if (p != 0x0000) rowRight[dx - 320] = p;
                }
            }
        }
    }
}

/*
 * Draw horizontally flipped sprite
 */
void grDrawSpriteFlip(PS1Surface *sfc, struct TTtmSlot *ttmSlot, sint16 x, sint16 y,
                      uint16 spriteNo, uint16 imageNo)
{
    x += grDx;
    y += grDy;

    /* Validate imageNo bounds */
    if (imageNo >= MAX_BMP_SLOTS || ttmSlot->numSprites[imageNo] == 0) {
        return;
    }

    /* Wrap sprite index to available frames (handles frame cap) */
    uint16 actualSpriteNo = spriteNo % ttmSlot->numSprites[imageNo];

    PS1Surface *sprite = ttmSlot->sprites[imageNo][actualSpriteNo];
    if (sprite == NULL) {
        return;
    }

    /* RAM-based sprites (loaded via grLoadBmpRAM) have x=0, y=0 with valid pixel data. */
    if (sprite->x == 0 && sprite->y == 0 && (sprite->pixels != NULL || sprite->indexedPixels != NULL)) {
        grCompositeToBackgroundFlip(sprite, x, y);
        grRecordReplaySprite(grCurrentThread, sprite, x, y, spriteNo, imageNo, 1);
        return;
    }

    /* Draw all tiles in this sprite's linked list (flipped) */
    PS1Surface *tile = sprite;
    while (tile != NULL) {
        /* Allocate POLY_FT4 primitive from buffer */
        /* PS1 doesn't have hardware flip, so we use textured quad with reversed UVs */
        if (primitiveIndex[db] + sizeof(POLY_FT4) > PRIMITIVE_BUFFER_SIZE) {
            GR_DIAG_PRINTF("Warning: Primitive buffer full!\n");
            return;
        }

        /* Calculate flipped screen position for this tile
         * For horizontal flip: tile at offsetX goes to (fullWidth - offsetX - tileWidth) */
        sint16 tileX = x + (sprite->fullWidth - tile->tileOffsetX - tile->width);
        sint16 tileY = y + tile->tileOffsetY;

        POLY_FT4 *poly = (POLY_FT4*)nextPrimitive[db];
        nextPrimitive[db] += sizeof(POLY_FT4);
        primitiveIndex[db] += sizeof(POLY_FT4);

        /* Initialize textured quad */
        setPolyFT4(poly);

        /* Set screen coordinates (normal quad, flipping happens in UV) */
        setXY4(poly,
               tileX, tileY,                              /* Top-left */
               tileX + tile->width, tileY,                /* Top-right */
               tileX, tileY + tile->height,               /* Bottom-left */
               tileX + tile->width, tileY + tile->height); /* Bottom-right */

        /* Calculate texture page from tile VRAM position (4-bit mode) */
        uint16 tpageX = tile->x / 64;
        uint16 tpageY = tile->y / 256;
        poly->tpage = getTPage(0, 0, tpageX * 64, tpageY * 256);

        /* Set UV coordinates (flipped horizontally, relative to texture page)
         * For 4-bit textures: U = ((vram_x % 64) * 4) & 0xFF */
        uint8 baseU = ((tile->x % 64) * 4) & 0xFF;
        uint8 baseV = (tile->y % 256) & 0xFF;
        uint8 u0 = baseU + tile->width;  /* Right edge */
        uint8 u1 = baseU;                 /* Left edge */
        uint8 v0 = baseV;
        uint8 v1 = baseV + tile->height;

        setUV4(poly, u0, v0, u1, v0, u0, v1, u1, v1);  /* Flipped U coords */

        setClut(poly, tile->clutX, tile->clutY);
        setRGB0(poly, 128, 128, 128);  /* Normal brightness */

        /* Add to ordering table */
        ps1GpuOtAddPrim(&ot[db][0], poly);

        GR_DIAG_PRINTF("Draw flipped tile: pos=(%d,%d) size=%dx%d\n",
                       tileX, tileY, tile->width, tile->height);

        tile = tile->nextTile;
    }
}

/*
 * Extended sprite drawing - allows caller to provide their own OT and primitive buffer
 * Walks linked list of tiles for multi-tile sprites (sprites > 64 pixels)
 * Returns 0 on success, -1 on failure
 */
int grDrawSpriteExt(unsigned long *extOT, char **nextPri, PS1Surface *sprite, sint16 x, sint16 y)
{
    if (sprite == NULL || extOT == NULL || nextPri == NULL) {
        return -1;
    }

    x += grDx;
    y += grDy;

    /* RAM-based sprites (loaded via grLoadBmpRAM) have x=0, y=0 with valid pixel data.
     * Composite to background tiles with transparency (0x0000 = transparent).
     * grDrawBackground() will upload the composited tiles later this frame. */
    if (sprite->x == 0 && sprite->y == 0 && (sprite->pixels != NULL || sprite->indexedPixels != NULL)) {
        grCompositeToBackground(sprite, x, y);
        return 0;
    }

    /* Calculate texture page ONCE from first tile - all tiles share same tpage */
    uint16 tpageX = sprite->x / 64;
    uint16 tpageY = sprite->y / 256;

    /* Walk linked list of tiles */
    PS1Surface *tile = sprite;
    while (tile != NULL) {
        /* Screen position for this tile */
        sint16 tileX = x + tile->tileOffsetX;
        sint16 tileY = y + tile->tileOffsetY;

        /* Allocate DR_TPAGE primitive */
        DR_TPAGE *tpage = (DR_TPAGE*)(*nextPri);
        *nextPri += sizeof(DR_TPAGE);

        /* Use SAME texture page for all tiles (calculated from first tile) */
        setDrawTPage(tpage, 0, 0, getTPage(0, 0, tpageX * 64, tpageY * 256));

        /* Allocate SPRT primitive */
        SPRT *sprt = (SPRT*)(*nextPri);
        *nextPri += sizeof(SPRT);

        /* Initialize sprite primitive */
        setSprt(sprt);
        setXY0(sprt, tileX, tileY);
        setWH(sprt, tile->width, tile->height);
        /* UV coords are relative to texture page (0-255 range)
         * For 4-bit textures: U = ((vram_x % 64) * 4) & 0xFF */
        setUV0(sprt, ((tile->x % 64) * 4) & 0xFF, (tile->y % 256) & 0xFF);
        setClut(sprt, tile->clutX, tile->clutY);
        setRGB0(sprt, 128, 128, 128);  /* Normal brightness */

        /* Add to ordering table - sprt FIRST so tpage renders BEFORE it
         * (addPrim adds to HEAD, so last added = first rendered) */
        ps1GpuOtAddPrim(extOT, sprt);
        ps1GpuOtAddPrim(extOT, tpage);

        tile = tile->nextTile;
    }

    return 0;
}

/*
 * Create an empty background tile (black, for RAM compositing)
 */
