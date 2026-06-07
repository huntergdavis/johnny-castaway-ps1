void grBlitToFramebuffer(PS1Surface *sprite, sint16 screenX, sint16 screenY)
{
    if (!sprite || !sprite->pixels) return;

    /* Clip to screen bounds */
    sint16 srcX = 0, srcY = 0;
    uint16 blitW = sprite->width;
    uint16 blitH = sprite->height;

    /* Left edge clipping */
    if (screenX < 0) {
        srcX = -screenX;
        blitW -= srcX;
        screenX = 0;
    }
    /* Top edge clipping */
    if (screenY < 0) {
        srcY = -screenY;
        blitH -= srcY;
        screenY = 0;
    }
    /* Right edge clipping */
    if (screenX + blitW > 640) {
        blitW = 640 - screenX;
    }
    /* Bottom edge clipping */
    if (screenY + blitH > 480) {
        blitH = 480 - screenY;
    }

    /* Nothing to draw if fully clipped */
    if (blitW <= 0 || blitH <= 0) return;

    /* If no clipping needed, direct LoadImage */
    if (srcX == 0 && srcY == 0 && blitW == sprite->width && blitH == sprite->height) {
        RECT dstRect;
        setRECT(&dstRect, screenX, screenY, sprite->width, sprite->height);
        LoadImage(&dstRect, (uint32*)sprite->pixels);
        /* No DrawSync here - let main loop handle sync */
    } else {
        /* MEM_REGION_RATIONALE: per-blit clipped-region temp buffer.
         * Lives ~one frame; freed inline. CACHE region: memFree(CACHE)
         * actually reclaims bytes via the coalescing free-list, so
         * per-blit allocs of any size can be reused. TRANSIENT was
         * the wrong fit because its memFree only decrements balance;
         * bytes persist until memSceneReset, and large blits (up to
         * 70 KB on activity-high variants) overflow the budget.
         * INIT_FULL_WRITE — populated by the memcpy loop below. */
        uint16 *tempBuf = (uint16*)memAlloc(MEM_REGION_CACHE,
                                            blitW * blitH * 2,
                                            "grBlitTempBuf");
        uint16 *src = sprite->pixels;
        uint16 *dst = tempBuf;

        for (uint16 y = 0; y < blitH; y++) {
            memcpy(dst, &src[(srcY + y) * sprite->width + srcX], blitW * 2);
            dst += blitW;
        }

        RECT dstRect;
        setRECT(&dstRect, screenX, screenY, blitW, blitH);
        LoadImage(&dstRect, (uint32*)tempBuf);
        DrawSync(0);  /* Must sync before "releasing" buffer - LoadImage is async! */

        memFree(MEM_REGION_CACHE, tempBuf);
    }
}

/* Indexed compositing helpers: simple per-byte decode.
 * Keep this path conservative; fishing regressions started after the later
 * LUT/unrolled fast path landed. */
static inline void compositeIndexedSpanFwd(uint16 *dst, const uint8 *src,
                                           uint32 pixelIdx, int count,
                                           const uint16 *pal)
{
    int di = 0;
    if (count <= 0) return;

    if (pixelIdx & 1) {
        uint8 packed = src[pixelIdx >> 1];
        uint16 p = pal[packed & 0x0F];
        if (p) dst[di] = p;
        di++;
        pixelIdx++;
        count--;
    }

    if (count >= 2) {
        while (count >= 2) {
            uint8 packed = src[pixelIdx >> 1];
            uint16 p0 = pal[(packed >> 4) & 0x0F];
            uint16 p1 = pal[packed & 0x0F];
            if (p0) dst[di] = p0;
            if (p1) dst[di + 1] = p1;
            di += 2;
            pixelIdx += 2;
            count -= 2;
        }
    }

    if (count) {
        uint8 packed = src[pixelIdx >> 1];
        uint16 p = pal[(packed >> 4) & 0x0F];
        if (p) dst[di] = p;
    }
}

static inline void compositeIndexedSpanRev(uint16 *dst, const uint8 *src,
                                           uint32 pixelIdx, int count,
                                           const uint16 *pal)
{
    int di = 0;
    if (count <= 0) return;

    if ((pixelIdx & 1) == 0) {
        uint8 packed = src[pixelIdx >> 1];
        uint16 p = pal[(packed >> 4) & 0x0F];
        if (p) dst[di] = p;
        di++;
        pixelIdx--;
        count--;
    }

    if (count >= 2) {
        while (count >= 2) {
            uint8 packed = src[pixelIdx >> 1];
            uint16 p0 = pal[packed & 0x0F];
            uint16 p1 = pal[(packed >> 4) & 0x0F];
            if (p0) dst[di] = p0;
            if (p1) dst[di + 1] = p1;
            di += 2;
            pixelIdx -= 2;
            count -= 2;
        }
    }

    if (count) {
        uint8 packed = src[pixelIdx >> 1];
        uint16 p = pal[packed & 0x0F];
        if (p) dst[di] = p;
    }
}

/* PS1-nibble-order compositing helpers for PSB sprites.
 * Keep these simple as well while chasing fishing sprite loss. */
static inline void compositePsbSpanFwd(uint16 *dst, const uint8 *src,
                                       uint32 pixelIdx, int count,
                                       const uint16 *pal)
{
    int di = 0;
    if (count <= 0) return;

    if (pixelIdx & 1) {
        uint8 packed = src[pixelIdx >> 1];
        uint16 p = pal[(packed >> 4) & 0x0F];
        if (p) dst[di] = p;
        di++;
        pixelIdx++;
        count--;
    }

    if (count >= 2) {
        while (count >= 2) {
            uint8 packed = src[pixelIdx >> 1];
            uint16 p0 = pal[packed & 0x0F];
            uint16 p1 = pal[(packed >> 4) & 0x0F];
            if (p0) dst[di] = p0;
            if (p1) dst[di + 1] = p1;
            di += 2;
            pixelIdx += 2;
            count -= 2;
        }
    }

    if (count) {
        uint8 packed = src[pixelIdx >> 1];
        uint16 p = pal[packed & 0x0F];
        if (p) dst[di] = p;
    }
}

static inline void compositePsbSpanRev(uint16 *dst, const uint8 *src,
                                       uint32 pixelIdx, int count,
                                       const uint16 *pal)
{
    int di = 0;
    if (count <= 0) return;

    if ((pixelIdx & 1) == 0) {
        uint8 packed = src[pixelIdx >> 1];
        uint16 p = pal[packed & 0x0F];
        if (p) dst[di] = p;
        di++;
        pixelIdx--;
        count--;
    }

    if (count >= 2) {
        while (count >= 2) {
            uint8 packed = src[pixelIdx >> 1];
            uint16 p0 = pal[(packed >> 4) & 0x0F];
            uint16 p1 = pal[packed & 0x0F];
            if (p0) dst[di] = p0;
            if (p1) dst[di + 1] = p1;
            di += 2;
            pixelIdx -= 2;
            count -= 2;
        }
    }

    if (count) {
        uint8 packed = src[pixelIdx >> 1];
        uint16 p = pal[(packed >> 4) & 0x0F];
        if (p) dst[di] = p;
    }
}

static inline void compositeDirectOpaqueRuns(uint16 *dst, const uint16 *src, int count)
{
    int runStart = -1;

    for (int i = 0; i < count; i++) {
        if (src[i] != 0x0000) {
            if (runStart < 0)
                runStart = i;
        } else if (runStart >= 0) {
            memcpy(dst + runStart, src + runStart, (size_t)(i - runStart) * sizeof(uint16));
            runStart = -1;
        }
    }

    if (runStart >= 0)
        memcpy(dst + runStart, src + runStart, (size_t)(count - runStart) * sizeof(uint16));
}

static void grCompositeDirectOpaqueSingleColumn(const uint16 *srcPixels,
                                                uint16 srcWidth,
                                                int screenX,
                                                int screenY,
                                                int height,
                                                int tileBaseX,
                                                PS1Surface *topTile,
                                                PS1Surface *bottomTile)
{
    int tileLocalX;
    int topRows;
    int bottomRows;

    if (srcPixels == NULL || topTile == NULL || bottomTile == NULL ||
        topTile->pixels == NULL || bottomTile->pixels == NULL) {
        return;
    }

    tileLocalX = screenX - tileBaseX;
    topRows = 240 - screenY;
    if (topRows < 0)
        topRows = 0;
    if (topRows > height)
        topRows = height;
    bottomRows = height - topRows;

    for (int row = 0; row < topRows; row++) {
        uint16 *dst = topTile->pixels + ((screenY + row) * (int)topTile->width) + tileLocalX;
        const uint16 *src = srcPixels + ((uint32)row * (uint32)srcWidth);
        compositeDirectOpaqueRuns(dst, src, srcWidth);
    }

    for (int row = 0; row < bottomRows; row++) {
        uint16 *dst = bottomTile->pixels + (row * (int)bottomTile->width) + tileLocalX;
        const uint16 *src = srcPixels + ((uint32)(topRows + row) * (uint32)srcWidth);
        compositeDirectOpaqueRuns(dst, src, srcWidth);
    }
}

/*
 * Composite a RAM-stored sprite into the background tile buffers WITH TRANSPARENCY
 * Skips pixels with value 0x0000 (transparent)
 * This modifies the bgTile RAM buffers so grDrawBackground() renders with transparency
 *
 * Background tile layout:
 * - bgTile0: x=0-319, y=0-239
 * - bgTile1: x=320-639, y=0-239
 * - bgTile3: x=0-319, y=240-479
 * - bgTile4: x=320-639, y=240-479
 */
void __attribute__((optimize("Os")))
grCompositeToBackground(PS1Surface *sprite, sint16 screenX, sint16 screenY)
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

    int startSx = 0;
    int endSx = sprW;
    if (screenX < 0) startSx = -screenX;
    if (screenX + endSx > 640) endSx = 640 - screenX;
    if (startSx >= endSx) return;

    /* Mark dirty region for this sprite */
    grMarkRectDirty(screenX + startSx, screenY + startSy,
                    screenX + endSx, screenY + endSy);

    const uint16 *pal = ttmPalette;

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
        int destStartX = screenX + startSx;
        int destEndX = screenX + endSx;

        if (usePsb) {
            /* PSB (PS1 nibble order): skip nibble swap — data is pre-transcoded */
            if (rowLeft && destStartX < 320) {
                int lx0 = destStartX;
                int lx1 = (destEndX < 320) ? destEndX : 320;
                int srcX = startSx + (lx0 - destStartX);
                int span = lx1 - lx0;
                compositePsbSpanFwd(rowLeft + lx0, sprite->indexedPixels,
                                    srcRowBase + (uint32)srcX, span, pal);
            }

            if (rowRight && destEndX > 320) {
                int rx0 = (destStartX > 320) ? destStartX : 320;
                int rx1 = destEndX;
                int srcX = startSx + (rx0 - destStartX);
                int span = rx1 - rx0;
                compositePsbSpanFwd(rowRight + (rx0 - 320), sprite->indexedPixels,
                                    srcRowBase + (uint32)srcX, span, pal);
            }
        } else if (useIndexed) {
            if (rowLeft && destStartX < 320) {
                int lx0 = destStartX;
                int lx1 = (destEndX < 320) ? destEndX : 320;
                int srcX = startSx + (lx0 - destStartX);
                int span = lx1 - lx0;
                compositeIndexedSpanFwd(rowLeft + lx0, sprite->indexedPixels,
                                        srcRowBase + (uint32)srcX, span, pal);
            }

            if (rowRight && destEndX > 320) {
                int rx0 = (destStartX > 320) ? destStartX : 320;
                int rx1 = destEndX;
                int srcX = startSx + (rx0 - destStartX);
                int span = rx1 - rx0;
                compositeIndexedSpanFwd(rowRight + (rx0 - 320), sprite->indexedPixels,
                                        srcRowBase + (uint32)srcX, span, pal);
            }
        } else {
            if (rowLeft && destStartX < 320) {
                int lx0 = destStartX;
                int lx1 = (destEndX < 320) ? destEndX : 320;
                int srcX = startSx + (lx0 - destStartX);
                int dx = lx0;
                for (; dx + 1 < lx1; dx += 2, srcX += 2) {
                    uint16 p0 = sprite->pixels[srcRowBase + (uint32)srcX];
                    uint16 p1 = sprite->pixels[srcRowBase + (uint32)srcX + 1];
                    if (p0 != 0x0000) rowLeft[dx] = p0;
                    if (p1 != 0x0000) rowLeft[dx + 1] = p1;
                }
                if (dx < lx1) {
                    uint16 p = sprite->pixels[srcRowBase + (uint32)srcX];
                    if (p != 0x0000) rowLeft[dx] = p;
                }
            }

            if (rowRight && destEndX > 320) {
                int rx0 = (destStartX > 320) ? destStartX : 320;
                int rx1 = destEndX;
                int srcX = startSx + (rx0 - destStartX);
                int dx = rx0;
                for (; dx + 1 < rx1; dx += 2, srcX += 2) {
                    uint16 p0 = sprite->pixels[srcRowBase + (uint32)srcX];
                    uint16 p1 = sprite->pixels[srcRowBase + (uint32)srcX + 1];
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

void grCompositeDirect16ToBackground(const uint16 *srcPixels, uint16 srcWidth, uint16 srcHeight,
                                     sint16 screenX, sint16 screenY)
{
    int rectEndX;
    int rectEndY;
    int startSy;
    int endSy;
    int startSx;
    int endSx;

    if (srcPixels == NULL || srcWidth == 0 || srcHeight == 0)
        return;
    if (srcWidth > 640 || srcHeight > 480)
        return;

    rectEndX = screenX + (int)srcWidth;
    rectEndY = screenY + (int)srcHeight;

    if (screenX >= 0 && screenY >= 0 &&
        rectEndX <= 640 && rectEndY <= 480) {
        int tileBaseX = (screenX >= 320) ? 320 : 0;
        if (rectEndX <= tileBaseX + 320) {
            if (screenY < 240 && rectEndY > 240) {
                grMarkSingleColumnDirty(tileBaseX, screenX, srcWidth, screenY, rectEndY);
                if (tileBaseX == 0) {
                    grCompositeDirectOpaqueSingleColumn(srcPixels, srcWidth,
                                                        screenX, screenY, srcHeight,
                                                        tileBaseX, bgTile0, bgTile3);
                } else {
                    grCompositeDirectOpaqueSingleColumn(srcPixels, srcWidth,
                                                        screenX, screenY, srcHeight,
                                                        tileBaseX, bgTile1, bgTile4);
                }
                return;
            }

            int tileLocalX = screenX - tileBaseX;

            grMarkSingleColumnDirty(tileBaseX, screenX, srcWidth, screenY, rectEndY);

            {
                PS1Surface *tile;
                int startLocalY;

                if (screenY < 240) {
                    tile = (tileBaseX == 0) ? bgTile0 : bgTile1;
                    startLocalY = screenY;
                } else {
                    tile = (tileBaseX == 0) ? bgTile3 : bgTile4;
                    startLocalY = screenY - 240;
                }

                if (tile == NULL || tile->pixels == NULL)
                    return;

                for (int sy = 0; sy < (int)srcHeight; sy++) {
                    uint16 *row = tile->pixels + ((startLocalY + sy) * (int)tile->width);
                    const uint16 *srcRow = srcPixels + ((uint32)sy * (uint32)srcWidth);
                    compositeDirectOpaqueRuns(row + tileLocalX, srcRow, srcWidth);
                }
            }
            return;
        }
    }

    startSy = 0;
    endSy = srcHeight;
    if (screenY < 0)
        startSy = -screenY;
    if (screenY + endSy > 480)
        endSy = 480 - screenY;
    if (startSy >= endSy)
        return;

    startSx = 0;
    endSx = srcWidth;
    if (screenX < 0)
        startSx = -screenX;
    if (screenX + endSx > 640)
        endSx = 640 - screenX;
    if (startSx >= endSx)
        return;

    grMarkRectDirty(screenX + startSx, screenY + startSy,
                    screenX + endSx, screenY + endSy);

    for (int sy = startSy; sy < endSy; sy++) {
        int destY = screenY + sy;
        PS1Surface *tileLeft;
        PS1Surface *tileRight;
        int tileLocalY;
        uint16 *rowLeft;
        uint16 *rowRight;
        const uint16 *srcRow = srcPixels + ((uint32)sy * (uint32)srcWidth);
        int destStartX = screenX + startSx;
        int destEndX = screenX + endSx;

        if (destY < 240) {
            tileLocalY = destY;
            tileLeft = bgTile0;
            tileRight = bgTile1;
        } else {
            tileLocalY = destY - 240;
            tileLeft = bgTile3;
            tileRight = bgTile4;
        }

        rowLeft = (tileLeft && tileLeft->pixels) ? (tileLeft->pixels + tileLocalY * (int)tileLeft->width) : NULL;
        rowRight = (tileRight && tileRight->pixels) ? (tileRight->pixels + tileLocalY * (int)tileRight->width) : NULL;

        if (rowLeft && destStartX < 320) {
            int lx0 = destStartX;
            int lx1 = (destEndX < 320) ? destEndX : 320;
            int srcX = startSx + (lx0 - destStartX);
            compositeDirectOpaqueRuns(rowLeft + lx0, srcRow + srcX, lx1 - lx0);
        }

        if (rowRight && destEndX > 320) {
            int rx0 = (destStartX > 320) ? destStartX : 320;
            int rx1 = destEndX;
            int srcX = startSx + (rx0 - destStartX);
            compositeDirectOpaqueRuns(rowRight + (rx0 - 320), srcRow + srcX, rx1 - rx0);
        }
    }
}

static uint16 grReadPackedSpanU16(const uint8 *p)
{
    return (uint16)((uint16)p[0] | ((uint16)p[1] << 8));
}

static uint8 grPacked4SpanMetadataCompact = 0;

static inline int
grReadCompactSpanU16(const uint8 *data,
                     uint32 dataSize,
                     uint32 *offset,
                     uint16 *outValue)
{
    uint8 value;

    if (*offset >= dataSize)
        return 0;

    value = data[*offset];
    *offset = *offset + 1u;
    if (value != 0xffu) {
        *outValue = value;
        return 1;
    }

    if (*offset + 2u > dataSize)
        return 0;
    *outValue = grReadPackedSpanU16(data + *offset);
    *offset = *offset + 2u;
    return 1;
}

static inline void grCompositePacked4OpaqueRun(uint16 *dst,
                                               const uint8 *packedPixels,
                                               int srcPixel,
                                               int count,
                                               const uint16 *palette)
{
    int out = 0;

    if (count <= 0)
        return;

    /* FG2 PAL4 spans contain only visible pixels; index 0 never appears
     * inside a span. The packer splits around no-diff pixels up front. */
    if (srcPixel & 1) {
        uint8 packed = packedPixels[srcPixel >> 1];
        dst[out++] = palette[packed & 0x0F];
        srcPixel++;
        count--;
    }

    while (count >= 2) {
        uint8 packed = packedPixels[srcPixel >> 1];
        dst[out] = palette[packed >> 4];
        dst[out + 1] = palette[packed & 0x0F];
        out += 2;
        srcPixel += 2;
        count -= 2;
    }

    if (count) {
        uint8 packed = packedPixels[srcPixel >> 1];
        dst[out] = palette[packed >> 4];
    }
}

static void grCompositePacked4SpanToBackground(const uint8 *packedPixels,
                                               uint16 pixelCount,
                                               const uint16 *palette,
                                               int destX,
                                               int destY)
{
    int start = 0;
    int end = (int)pixelCount;
    int destStartX;
    int destEndX;
    int tileLocalY;
    PS1Surface *tileLeft;
    PS1Surface *tileRight;

    if (packedPixels == NULL || palette == NULL || pixelCount == 0)
        return;
    if (destY < 0 || destY >= 480)
        return;
    if (destX < 0)
        start = -destX;
    if (destX + end > 640)
        end = 640 - destX;
    if (start >= end)
        return;

    destStartX = destX + start;
    destEndX = destX + end;
    if (destY < 240) {
        tileLocalY = destY;
        tileLeft = bgTile0;
        tileRight = bgTile1;
    } else {
        tileLocalY = destY - 240;
        tileLeft = bgTile3;
        tileRight = bgTile4;
    }

    if (tileLeft != NULL && tileLeft->pixels != NULL && destStartX < 320) {
        int lx0 = destStartX;
        int lx1 = (destEndX < 320) ? destEndX : 320;
        int srcStart = start + (lx0 - destStartX);
        int count = lx1 - lx0;
        uint16 *dst = tileLeft->pixels + (tileLocalY * (int)tileLeft->width) + lx0;

        grCompositePacked4OpaqueRun(dst, packedPixels, srcStart, count, palette);
    }

    if (tileRight != NULL && tileRight->pixels != NULL && destEndX > 320) {
        int rx0 = (destStartX > 320) ? destStartX : 320;
        int rx1 = destEndX;
        int srcStart = start + (rx0 - destStartX);
        int count = rx1 - rx0;
        uint16 *dst = tileRight->pixels + (tileLocalY * (int)tileRight->width) + (rx0 - 320);

        grCompositePacked4OpaqueRun(dst, packedPixels, srcStart, count, palette);
    }
}

void grCompositePacked4SpansToBackground(const uint8 *spanData, uint32 spanDataSize,
                                         const uint16 *palette,
                                         sint16 screenX, sint16 screenY)
{
    uint32 offset = 0;
    uint16 rowCount;
    uint16 perfSpans = 0;
    uint32 perfPixels = 0;
    int perfTrack = ps1PerfEnabled;
    int compactMetadata = grPacked4SpanMetadataCompact;

    if (spanData == NULL || palette == NULL || spanDataSize < 2)
        return;

    rowCount = grReadPackedSpanU16(spanData);
    offset = 2;

    for (uint16 row = 0; row < rowCount; row++) {
        uint16 relY;
        uint16 spanCount;
        int rowScreenY;
        uint16 *rowLeftPixels = NULL;
        uint16 *rowRightPixels = NULL;
        /* Dirty rows are tracked per 320px tile; do not merge across the tile boundary. */
        int rowLeftDirtyStart = 320;
        int rowLeftDirtyEnd = -1;
        int rowRightDirtyStart = 640;
        int rowRightDirtyEnd = 319;

        if (compactMetadata) {
            if (!grReadCompactSpanU16(spanData, spanDataSize, &offset, &relY) ||
                !grReadCompactSpanU16(spanData, spanDataSize, &offset, &spanCount))
                return;
        } else {
            if (offset + 4u > spanDataSize)
                return;
            relY = grReadPackedSpanU16(spanData + offset);
            spanCount = grReadPackedSpanU16(spanData + offset + 2u);
            offset += 4u;
        }
        rowScreenY = (int)screenY + (int)relY;
        if (rowScreenY >= 0 && rowScreenY < 480) {
            int tileLocalY;
            PS1Surface *tileLeft;
            PS1Surface *tileRight;

            if (rowScreenY < 240) {
                tileLocalY = rowScreenY;
                tileLeft = bgTile0;
                tileRight = bgTile1;
            } else {
                tileLocalY = rowScreenY - 240;
                tileLeft = bgTile3;
                tileRight = bgTile4;
            }
            if (tileLeft != NULL && tileLeft->pixels != NULL)
                rowLeftPixels = tileLeft->pixels + (tileLocalY * (int)tileLeft->width);
            if (tileRight != NULL && tileRight->pixels != NULL)
                rowRightPixels = tileRight->pixels + (tileLocalY * (int)tileRight->width);
        }

        for (uint16 span = 0; span < spanCount; span++) {
            uint16 relX;
            uint16 pixelCount;
            uint32 packedBytes;
            int spanX;
            int spanEndX;
            int dirtyStart;
            int dirtyEnd;

            if (compactMetadata) {
                if (!grReadCompactSpanU16(spanData, spanDataSize, &offset, &relX) ||
                    !grReadCompactSpanU16(spanData, spanDataSize, &offset, &pixelCount))
                    return;
            } else {
                if (offset + 4u > spanDataSize)
                    return;
                relX = grReadPackedSpanU16(spanData + offset);
                pixelCount = grReadPackedSpanU16(spanData + offset + 2u);
                offset += 4u;
            }
            if (perfTrack) {
                perfSpans++;
                perfPixels += pixelCount;
            }

            packedBytes = ((uint32)pixelCount + 1u) >> 1;
            if (offset + packedBytes > spanDataSize)
                return;

            spanX = (int)screenX + (int)relX;
            spanEndX = spanX + (int)pixelCount;
            dirtyStart = spanX;
            dirtyEnd = spanEndX;
            if (rowScreenY >= 0 && rowScreenY < 480) {
                if (dirtyStart < 320 && dirtyEnd > 0) {
                    int leftStart = dirtyStart;
                    int leftEnd = dirtyEnd;
                    if (leftStart < 0)
                        leftStart = 0;
                    if (leftEnd > 320)
                        leftEnd = 320;
                    if (leftStart < leftEnd) {
                        if (leftStart < rowLeftDirtyStart)
                            rowLeftDirtyStart = leftStart;
                        if (leftEnd > rowLeftDirtyEnd)
                            rowLeftDirtyEnd = leftEnd;
                    }
                }
                if (dirtyEnd > 320 && dirtyStart < 640) {
                    int rightStart = dirtyStart;
                    int rightEnd = dirtyEnd;
                    if (rightStart < 320)
                        rightStart = 320;
                    if (rightEnd > 640)
                        rightEnd = 640;
                    if (rightStart < rightEnd) {
                        if (rightStart < rowRightDirtyStart)
                            rowRightDirtyStart = rightStart;
                        if (rightEnd > rowRightDirtyEnd)
                            rowRightDirtyEnd = rightEnd;
                    }
                }
            }

            if (spanX >= 0 && spanEndX <= 320 && rowLeftPixels != NULL) {
                grCompositePacked4OpaqueRun(rowLeftPixels + spanX,
                                            spanData + offset,
                                            0,
                                            pixelCount,
                                            palette);
            } else if (spanX >= 320 && spanEndX <= 640 && rowRightPixels != NULL) {
                grCompositePacked4OpaqueRun(rowRightPixels + (spanX - 320),
                                            spanData + offset,
                                            0,
                                            pixelCount,
                                            palette);
            } else {
                grCompositePacked4SpanToBackground(spanData + offset,
                                                   pixelCount,
                                                   palette,
                                                   spanX,
                                                   rowScreenY);
            }
            offset += packedBytes;
        }
        if (rowLeftDirtyStart < rowLeftDirtyEnd)
            grMarkRectDirty(rowLeftDirtyStart, rowScreenY, rowLeftDirtyEnd, rowScreenY + 1);
        if (rowRightDirtyStart < rowRightDirtyEnd)
            grMarkRectDirty(rowRightDirtyStart, rowScreenY, rowRightDirtyEnd, rowScreenY + 1);
    }
    if (perfTrack)
        ps1PerfMarkCompose(rowCount, perfSpans, perfPixels, spanDataSize);
}

static void grCompositePacked4CompactSpansToBackground(const uint8 *spanData,
                                                       uint32 spanDataSize,
                                                       const uint16 *palette,
                                                       sint16 screenX,
                                                       sint16 screenY)
{
    grPacked4SpanMetadataCompact = 1;
    grCompositePacked4SpansToBackground(spanData, spanDataSize, palette, screenX, screenY);
    grPacked4SpanMetadataCompact = 0;
}

void grBeginResidualCleanBgFrame(void)
{
    grClearCurrDirtyState();
    grClearPrevDirtyState();
}

void grBeginResidualCleanBgFirstFrame(void)
{
    grClearCurrDirtyState();
}

void grCompositePacked4TemporalResidualToBackground(const uint8 *spanData, uint32 spanDataSize,
                                                    const uint16 *palette,
                                                    sint16 screenX, sint16 screenY)
{
    uint32 offset = 0;
    uint32 restoredBytes = 0;
    uint16 cleanupRows;

    if (spanData == NULL || palette == NULL || spanDataSize < 2)
        return;

    cleanupRows = grReadPackedSpanU16(spanData);
    offset = 2;
    for (uint16 row = 0; row < cleanupRows; row++) {
        uint16 relY;
        uint16 spanCount;
        int rowScreenY;

        if (offset + 4u > spanDataSize)
            return;
        relY = grReadPackedSpanU16(spanData + offset);
        spanCount = grReadPackedSpanU16(spanData + offset + 2u);
        offset += 4u;
        rowScreenY = (int)screenY + (int)relY;

        for (uint16 span = 0; span < spanCount; span++) {
            uint16 relX;
            uint16 pixelCount;

            if (offset + 4u > spanDataSize)
                return;
            relX = grReadPackedSpanU16(spanData + offset);
            pixelCount = grReadPackedSpanU16(spanData + offset + 2u);
            offset += 4u;
            restoredBytes += grRestoreCleanBgSpanFromRects((int)screenX + (int)relX,
                                                           rowScreenY,
                                                           (int)pixelCount);
        }
    }

    if (ps1PerfEnabled)
        ps1PerfMarkRestore(restoredBytes);
    if (offset < spanDataSize)
        grCompositePacked4SpansToBackground(spanData + offset,
                                            spanDataSize - offset,
                                            palette,
                                            screenX,
                                            screenY);
}

void __attribute__((optimize("Os")))
grCompositePacked4CompactTemporalResidualToBackground(const uint8 *spanData,
                                                      uint32 spanDataSize,
                                                      const uint16 *palette,
                                                      sint16 screenX,
                                                      sint16 screenY)
{
    uint32 offset = 0;
    uint32 restoredBytes = 0;
    uint16 cleanupRows;

    if (spanData == NULL || palette == NULL || spanDataSize < 2)
        return;

    cleanupRows = grReadPackedSpanU16(spanData);
    offset = 2;
    for (uint16 row = 0; row < cleanupRows; row++) {
        uint16 relY;
        uint16 spanCount;
        int rowScreenY;

        if (!grReadCompactSpanU16(spanData, spanDataSize, &offset, &relY) ||
            !grReadCompactSpanU16(spanData, spanDataSize, &offset, &spanCount))
            return;
        rowScreenY = (int)screenY + (int)relY;

        for (uint16 span = 0; span < spanCount; span++) {
            uint16 relX;
            uint16 pixelCount;

            if (!grReadCompactSpanU16(spanData, spanDataSize, &offset, &relX) ||
                !grReadCompactSpanU16(spanData, spanDataSize, &offset, &pixelCount))
                return;
            restoredBytes += grRestoreCleanBgSpanFromRects((int)screenX + (int)relX,
                                                           rowScreenY,
                                                           (int)pixelCount);
        }
    }

    if (ps1PerfEnabled)
        ps1PerfMarkRestore(restoredBytes);
    if (offset < spanDataSize)
        grCompositePacked4CompactSpansToBackground(spanData + offset,
                                                   spanDataSize - offset,
                                                   palette,
                                                   screenX,
                                                   screenY);
}

static inline void grCompositeIndexed8OpaqueRun(uint16 *dst,
                                                const uint8 *indexedPixels,
                                                int count,
                                                const uint16 *palette)
{
    for (int i = 0; i < count; i++)
        dst[i] = palette[indexedPixels[i]];
}

static void grCompositeIndexed8SpanToBackground(const uint8 *indexedPixels,
                                                uint16 pixelCount,
                                                const uint16 *palette,
                                                int destX,
                                                int destY)
{
    int start = 0;
    int end = (int)pixelCount;
    int destStartX;
    int destEndX;
    int tileLocalY;
    PS1Surface *tileLeft;
    PS1Surface *tileRight;

    if (indexedPixels == NULL || palette == NULL || pixelCount == 0)
        return;
    if (destY < 0 || destY >= 480)
        return;
    if (destX < 0)
        start = -destX;
    if (destX + end > 640)
        end = 640 - destX;
    if (start >= end)
        return;

    destStartX = destX + start;
    destEndX = destX + end;
    grMarkRectDirty(destStartX, destY, destEndX, destY + 1);

    if (destY < 240) {
        tileLocalY = destY;
        tileLeft = bgTile0;
        tileRight = bgTile1;
    } else {
        tileLocalY = destY - 240;
        tileLeft = bgTile3;
        tileRight = bgTile4;
    }

    if (tileLeft != NULL && tileLeft->pixels != NULL && destStartX < 320) {
        int lx0 = destStartX;
        int lx1 = (destEndX < 320) ? destEndX : 320;
        int srcStart = start + (lx0 - destStartX);
        uint16 *dst = tileLeft->pixels + (tileLocalY * (int)tileLeft->width) + lx0;
        grCompositeIndexed8OpaqueRun(dst, indexedPixels + srcStart, lx1 - lx0, palette);
    }

    if (tileRight != NULL && tileRight->pixels != NULL && destEndX > 320) {
        int rx0 = (destStartX > 320) ? destStartX : 320;
        int rx1 = destEndX;
        int srcStart = start + (rx0 - destStartX);
        uint16 *dst = tileRight->pixels + (tileLocalY * (int)tileRight->width) + (rx0 - 320);
        grCompositeIndexed8OpaqueRun(dst, indexedPixels + srcStart, rx1 - rx0, palette);
    }
}

void grCompositeIndexed8SpansToBackground(const uint8 *spanData, uint32 spanDataSize,
                                          const uint16 *palette,
                                          sint16 screenX, sint16 screenY)
{
    uint32 offset = 0;
    uint16 rowCount;
    uint16 perfSpans = 0;
    uint32 perfPixels = 0;
    int perfTrack = ps1PerfEnabled;

    if (spanData == NULL || palette == NULL || spanDataSize < 2)
        return;

    rowCount = grReadPackedSpanU16(spanData);
    offset = 2;

    for (uint16 row = 0; row < rowCount; row++) {
        uint16 relY;
        uint16 spanCount;
        int rowScreenY;
        uint16 *rowLeftPixels = NULL;
        uint16 *rowRightPixels = NULL;
        /* Dirty rows are tracked per 320px tile; keep indexed8 aligned with PAL4. */
        int rowLeftDirtyStart = 320;
        int rowLeftDirtyEnd = -1;
        int rowRightDirtyStart = 640;
        int rowRightDirtyEnd = 319;

        if (offset + 4u > spanDataSize)
            return;
        relY = grReadPackedSpanU16(spanData + offset);
        spanCount = grReadPackedSpanU16(spanData + offset + 2u);
        offset += 4u;
        rowScreenY = (int)screenY + (int)relY;
        if (rowScreenY >= 0 && rowScreenY < 480) {
            int tileLocalY;
            PS1Surface *tileLeft;
            PS1Surface *tileRight;

            if (rowScreenY < 240) {
                tileLocalY = rowScreenY;
                tileLeft = bgTile0;
                tileRight = bgTile1;
            } else {
                tileLocalY = rowScreenY - 240;
                tileLeft = bgTile3;
                tileRight = bgTile4;
            }
            if (tileLeft != NULL && tileLeft->pixels != NULL)
                rowLeftPixels = tileLeft->pixels + (tileLocalY * (int)tileLeft->width);
            if (tileRight != NULL && tileRight->pixels != NULL)
                rowRightPixels = tileRight->pixels + (tileLocalY * (int)tileRight->width);
        }

        for (uint16 span = 0; span < spanCount; span++) {
            uint16 relX;
            uint16 pixelCount;
            int spanX;
            int spanEndX;

            if (offset + 4u > spanDataSize)
                return;
            relX = grReadPackedSpanU16(spanData + offset);
            pixelCount = grReadPackedSpanU16(spanData + offset + 2u);
            offset += 4u;
            if (perfTrack) {
                perfSpans++;
                perfPixels += pixelCount;
            }

            if (offset + (uint32)pixelCount > spanDataSize)
                return;

            spanX = (int)screenX + (int)relX;
            spanEndX = spanX + (int)pixelCount;
            if (spanX >= 0 && spanEndX <= 320 && rowLeftPixels != NULL) {
                if (spanX < rowLeftDirtyStart)
                    rowLeftDirtyStart = spanX;
                if (spanEndX > rowLeftDirtyEnd)
                    rowLeftDirtyEnd = spanEndX;
                grCompositeIndexed8OpaqueRun(rowLeftPixels + spanX,
                                             spanData + offset,
                                             pixelCount,
                                             palette);
            } else if (spanX >= 320 && spanEndX <= 640 && rowRightPixels != NULL) {
                if (spanX < rowRightDirtyStart)
                    rowRightDirtyStart = spanX;
                if (spanEndX > rowRightDirtyEnd)
                    rowRightDirtyEnd = spanEndX;
                grCompositeIndexed8OpaqueRun(rowRightPixels + (spanX - 320),
                                             spanData + offset,
                                             pixelCount,
                                             palette);
            } else {
                grCompositeIndexed8SpanToBackground(spanData + offset,
                                                    pixelCount,
                                                    palette,
                                                    spanX,
                                                    rowScreenY);
            }
            offset += (uint32)pixelCount;
        }
        if (rowLeftDirtyStart < rowLeftDirtyEnd)
            grMarkRectDirty(rowLeftDirtyStart, rowScreenY, rowLeftDirtyEnd, rowScreenY + 1);
        if (rowRightDirtyStart < rowRightDirtyEnd)
            grMarkRectDirty(rowRightDirtyStart, rowScreenY, rowRightDirtyEnd, rowScreenY + 1);
    }
    if (perfTrack)
        ps1PerfMarkCompose(rowCount, perfSpans, perfPixels, spanDataSize);
}

void grCompositeIndexed8TemporalResidualToBackground(const uint8 *spanData, uint32 spanDataSize,
                                                     const uint16 *palette,
                                                     sint16 screenX, sint16 screenY)
{
    uint32 offset = 0;
    uint32 restoredBytes = 0;
    uint16 cleanupRows;

    if (spanData == NULL || palette == NULL || spanDataSize < 2)
        return;

    cleanupRows = grReadPackedSpanU16(spanData);
    offset = 2;
    for (uint16 row = 0; row < cleanupRows; row++) {
        uint16 relY;
        uint16 spanCount;
        int rowScreenY;

        if (offset + 4u > spanDataSize)
            return;
        relY = grReadPackedSpanU16(spanData + offset);
        spanCount = grReadPackedSpanU16(spanData + offset + 2u);
        offset += 4u;
        rowScreenY = (int)screenY + (int)relY;

        for (uint16 span = 0; span < spanCount; span++) {
            uint16 relX;
            uint16 pixelCount;

            if (offset + 4u > spanDataSize)
                return;
            relX = grReadPackedSpanU16(spanData + offset);
            pixelCount = grReadPackedSpanU16(spanData + offset + 2u);
            offset += 4u;
            restoredBytes += grRestoreCleanBgSpanFromRects((int)screenX + (int)relX,
                                                           rowScreenY,
                                                           (int)pixelCount);
        }
    }

    if (ps1PerfEnabled)
        ps1PerfMarkRestore(restoredBytes);
    if (offset < spanDataSize)
        grCompositeIndexed8SpansToBackground(spanData + offset,
                                             spanDataSize - offset,
                                             palette,
                                             screenX,
                                             screenY);
}

/*
 * Set clipping rectangle
 */
