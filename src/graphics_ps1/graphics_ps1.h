/*
 *  This file is part of 'Johnny Reborn' - PS1 Port
 *
 *  PlayStation 1 graphics implementation using PSn00bSDK
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef GRAPHICS_PS1_H
#define GRAPHICS_PS1_H

#include <psxgpu.h>
#include <psxgte.h>

/* Forward declaration for LRU cache */
struct TTtmResource;
struct TBmpResource;

#define SCREEN_WIDTH        640
#define SCREEN_HEIGHT       480

#define MAX_BMP_SLOTS       6
#define MAX_SPRITES_PER_BMP 255
#define MAX_TTM_SLOTS       10
#define MAX_TTM_THREADS     20

struct TAdsScene {
    uint16 slot;
    uint16 tag;
    uint16 numPlays;
};

/* PS1 Sprite structure - replaces SDL_Surface */
/* Supports multi-tile sprites for dimensions > 64 pixels */
typedef struct PS1Surface {
    uint16 *pixels;     /* 15-bit direct color pixel data (NULL if using indexedPixels) */
    uint8  *indexedPixels; /* 4-bit packed indexed pixel data (NULL if using pixels) */
    uint8  indexedOwned; /* 1 if indexedPixels must be freed with this surface */
    uint8  psbNibbles;  /* 1 if indexedPixels are PS1 nibble order (pre-swapped PSB) */
    uint16 width;       /* This tile's width (max 64) */
    uint16 height;      /* This tile's height (max 64) */
    uint16 x, y;        /* Position in VRAM */
    uint16 clutX, clutY; /* CLUT position in VRAM */
    /* Multi-tile support */
    uint16 fullWidth;   /* Original sprite full width */
    uint16 fullHeight;  /* Original sprite full height */
    uint16 tileOffsetX; /* This tile's X offset in original sprite */
    uint16 tileOffsetY; /* This tile's Y offset in original sprite */
    struct PS1Surface *nextTile; /* Next tile in chain (NULL if last/only) */
} PS1Surface;

/* Compatibility alias for code that uses SDL_Surface */
typedef PS1Surface SDL_Surface;

struct TTtmSlot {
    uint8       *data;
    uint32      dataSize;
    struct      TTtmTag *tags;
    int         numTags;
    int         numSprites[MAX_BMP_SLOTS];
    uint16      spriteGen[MAX_BMP_SLOTS];
    struct TBmpResource *loadedBmp[MAX_BMP_SLOTS];
    const char  *loadedBmpNames[MAX_BMP_SLOTS];
    uint8       *psbData[MAX_BMP_SLOTS]; /* PSB buffer per-slot (sprites point into it) */
    uint8       psbDataCallerOwned[MAX_BMP_SLOTS]; /* 1: release must NOT free psbData */
    PS1Surface  *sprites[MAX_BMP_SLOTS][MAX_SPRITES_PER_BMP];
    struct TTtmResource *ttmResource;  /* For LRU cache unpinning */
};

struct TTtmTag {
    uint16 id;
    uint32 offset;
};

#define MAX_DRAWN_SPRITES 255

struct TDrawnSprite {
    uint8  *indexedPixels;  /* Snapshot — always valid (BMP data never freed) */
    uint16 width, height;  /* Sprite dimensions at record time */
    sint16 x, y;           /* Screen position */
    uint16 spriteNo;       /* Dedup key */
    uint16 imageNo;        /* Dedup key */
    uint16 sceneEpoch;     /* Dedup key (iteration boundary) */
    uint8  flip;
    uint8  psbNibbles;  /* 1 if indexedPixels are PS1 nibble order (pre-swapped PSB) */
    const char *bmpName; /* Source BMP name for capture overlay/replay diagnostics */
};

struct TTtmThread {
    struct TTtmSlot   *ttmSlot;
    int    isRunning;
    char   currentStringArg[20];
    uint16 sceneSlot;
    uint16 sceneTag;
    short  sceneTimer;
    uint16 sceneIterations;
    uint32 ip;
    uint16 delay;
    uint16 timer;
    uint32 nextGotoOffset;
    uint8  selectedBmpSlot;
    uint8  fgColor;
    uint8  bgColor;
    uint16 currentRegionId;
    PS1Surface *ttmLayer;
    uint16 sceneEpoch;
    /* Track composited sprites for frame replay */
    struct TDrawnSprite drawnSprites[MAX_DRAWN_SPRITES];
    uint8  numDrawnSprites;
    uint8  replayWriteCursor;
    /* Per-thread actor continuity (avoid cross-scene/global contamination). */
    struct TDrawnSprite lastActorReplay;
    uint8  lastActorReplayValid;
};

extern PS1Surface *grBackgroundSfc;

extern int grDx;
extern int grDy;
extern int grWindowed;
extern int grUpdateDelay;

/* Frame capture for visual regression testing */
extern int grCaptureFrameNumber;
extern int grCaptureForegroundOnly;
extern char *grCaptureFilename;
extern char *grCaptureDir;
extern char *grCaptureMetaDir;
extern int grCaptureInterval;
extern int grCaptureStartFrame;
extern int grCaptureEndFrame;
extern int grCaptureOverlay;
extern int grCaptureOverlayMaskOnly;
extern int grCaptureForegroundIncludeStaticBase;
extern int grCaptureForegroundSkipVisibilityMask;
extern char *grCaptureSoundEventsPath;
extern int grPs1TelemetryEnabled;
void grCaptureSetSceneLabel(const char *sceneLabel);
void grCaptureSoundEvent(int sampleNo);

/* Flag to track if GPU was already initialized (e.g., by loadTitleScreenEarly)
 * Set this to 1 BEFORE calling graphicsInit() if GPU is already set up */
extern int grGpuAlreadyInitialized;

void graphicsInit();
void graphicsEnd();

/* Returns 1 once graphicsInit() has completed, 0 otherwise. The
 * memory-region allocator's memHalt primitive uses this to decide
 * between the full JC_BSOD UI (ps1Bsod, requires graphics) and the
 * minimal pre-graphics text panel (ps1DebugError). See plan v9
 * "Failure UX". */
int graphicsIsInitialized(void);

/* Sets the gGraphicsReady flag. graphicsInit() calls this with 1 at
 * its tail; tests or shutdown paths may call with 0. */
void memSetGraphicsReady(int ready);

void grRefreshDisplay();
void grToggleFullScreen();
/* Force every bgTile row to be re-uploaded on the next grDrawBackground.
 * Sets currDirty only — caller is responsible for prevDirty if they
 * need the upload to survive grRestoreBgTiles. */
void grMarkAllTilesDirty(void);
void grMarkScreenRectDirty(int x0, int y0, int x1, int y1);

/* Force the next grDrawBackground call to upload all 4 tiles in full.
 * Sets BOTH currDirty AND prevDirty so the upload survives
 * grRestoreBgTiles' currDirty reset at frame start. Used by
 * pause_menu.c on resume to fully repaint VRAM. */
void grForceFullRedrawNextFrame(void);
void grUpdateDisplay(struct TTtmThread *ttmBackgroundThread,
                     struct TTtmThread *ttmThreads,
                     struct TTtmThread *ttmHolidayThreads);

PS1Surface *grNewEmptyBackground();
PS1Surface *grNewLayer();
void grFreeLayer(PS1Surface *sfc);

void grLoadBmp(struct TTtmSlot *ttmSlot, uint16 slotNo, char *strArg);
void grLoadBmpRAM(struct TTtmSlot *ttmSlot, uint16 slotNo, char *strArg);
int grLoadPsbBuffer(struct TTtmSlot *ttmSlot, uint16 slotNo,
                    char *strArg, uint8 *psbBuf, uint32 psbSize);
/* Like grLoadPsbBuffer, but the PSB buffer stays caller-owned:
 * grReleaseBmp clears the slot reference without freeing the bytes.
 * Used by walk_pilot's persistent JOHNWALK load buffer so the 48 KB
 * block is not churned through CACHE on every inter-scene walk. */
int grLoadPsbBufferCallerOwned(struct TTtmSlot *ttmSlot, uint16 slotNo,
                               char *strArg, uint8 *psbBuf, uint32 psbSize);
void grReleaseBmp(struct TTtmSlot *ttmSlot, uint16 bmpSlotNo);
void grBlitToFramebuffer(PS1Surface *sprite, sint16 screenX, sint16 screenY);
void grCompositeToBackground(PS1Surface *sprite, sint16 screenX, sint16 screenY);
void grCompositeToBackgroundFlip(PS1Surface *sprite, sint16 screenX, sint16 screenY);
void grCompositeDirect16ToBackground(const uint16 *srcPixels, uint16 srcWidth, uint16 srcHeight,
                                     sint16 screenX, sint16 screenY);
void grCompositePacked4SpansToBackground(const uint8 *spanData, uint32 spanDataSize,
                                         const uint16 *palette,
                                         sint16 screenX, sint16 screenY);
void grBeginResidualCleanBgFrame(void);
void grBeginResidualCleanBgFirstFrame(void);
void grCompositePacked4TemporalResidualToBackground(const uint8 *spanData, uint32 spanDataSize,
                                                    const uint16 *palette,
                                                    sint16 screenX, sint16 screenY);
void grCompositePacked4CompactTemporalResidualToBackground(const uint8 *spanData,
                                                           uint32 spanDataSize,
                                                           const uint16 *palette,
                                                           sint16 screenX,
                                                           sint16 screenY);
void grCompositeIndexed8SpansToBackground(const uint8 *spanData, uint32 spanDataSize,
                                          const uint16 *palette,
                                          sint16 screenX, sint16 screenY);
void grCompositeIndexed8TemporalResidualToBackground(const uint8 *spanData, uint32 spanDataSize,
                                                     const uint16 *palette,
                                                     sint16 screenX, sint16 screenY);

void grSetClipZone(PS1Surface *sfc, sint16 x1, sint16 y1, sint16 x2, sint16 y2);
void grCopyZoneToBg(PS1Surface *sfc, uint16 arg0, uint16 arg1, uint16 arg2, uint16 arg3);
void grSaveImage1(PS1Surface *sfc, uint16 arg0, uint16 arg1, uint16 arg2, uint16 arg3);
void grSaveZone(PS1Surface *sfc, uint16 arg0, uint16 arg1, uint16 arg2, uint16 arg3);
void grRestoreZone(PS1Surface *sfc, uint16 arg0, uint16 arg1, uint16 arg2, uint16 arg3);
void grDrawPixel(PS1Surface *sfc, sint16 x, sint16 y, uint8 color);
void grDrawLine(PS1Surface *sfc, sint16 x1, sint16 y1, sint16 x2, sint16 y2, uint8 color);
void grDrawRect(PS1Surface *sfc, sint16 x, sint16 y, uint16 width, uint16 height, uint8 color);
void grDrawCircle(PS1Surface *sfc, sint16 x1, sint16 y1, uint16 width, uint16 height, uint8 fgColor, uint8 bgColor);
void grDrawSprite(PS1Surface *sfc, struct TTtmSlot *ttmSlot, sint16 x, sint16 y, uint16 spriteNo, uint16 imageNo);
void grDrawSpriteFlip(PS1Surface *sfc, struct TTtmSlot *ttmSlot, sint16 x, sint16 y, uint16 spriteNo, uint16 imageNo);

/* Extended sprite drawing - allows caller to provide their own OT and primitive buffer */
int grDrawSpriteExt(unsigned long *extOT, char **nextPri, PS1Surface *sprite, sint16 x, sint16 y);
void grInitEmptyBackground();
void grShowMeanwhileLoadingFrame(uint16 tick);
void grSaveCleanBgTiles(void);
void grSetSaveCleanOnScreenLoad(int enabled);
void grSetFullScreenScrCacheEnabled(int enabled);
void grFreeCleanBgTiles(void);
void grReleaseBackgroundTiles(void);
/* Round 33: called from fgRuntimeReset immediately after memSceneReset.
 * memSceneReset has already reclaimed every TRANSIENT byte — including
 * the bg-tile struct AND pixel buffers (Round 33 migration). The static
 * slot pointers (bgTile0/1/3/4 etc.) and grBackgroundSfc still hold the
 * post-wipe dangling addresses, so this helper NULLs them without
 * calling memFree (the bytes are gone; calling memFree would decrement
 * the TRANSIENT pin counter that memSceneReset already zeroed). */
void grBackgroundTilesAssumeWiped(void);
void grEnsureCleanBgTiles(void);
/* Rect-based clean backup (option B): scenes declare small dynamic regions. */
int  grSaveCleanBgRects(const sint16 *x, const sint16 *y,
                        const uint16 *w, const uint16 *h, int n);
int  grSaveCleanBgRectsSplit(const sint16 *x, const sint16 *y,
                             const uint16 *w, const uint16 *h, int n,
                             uint32 maxBytesPerRect);
void grRestoreBgFromRects(void);
void grFreeCleanBgRects(void);
void grDeactivateCleanBgRects(void);
void grSetCleanBgBlackMode(int enabled);
/* Legacy no-op. Clean-rect pixels are scene-owned under mem_region and
 * deactivation releases their storage. */
void grPreallocCleanBgRects(const uint32 *capBytes, int n);
int  grCleanBgRectsCount(void);
unsigned long grCleanBgRectsBytes(void);
void grSetCleanBgRectsForceCache(int enabled);
void grSetCleanBgRectsSlabRetain(int enabled);
int grFlushCleanBgRectSlabs(void);
void grRestoreBgRectsFull(void);
/* Capture/restore rectangles into a caller-owned buffer (dst/src sized
 * w*h*sizeof(uint16)). Same per-tile splitting + dirty-rect gating as
 * the grSaveCleanBgRects snapshots — but independent of that machinery,
 * so multiple concurrent buffers can coexist (e.g. FG2 scene clean +
 * walk_pilot's persistent walk-area buffer). */
void grCaptureBgRect(uint16 *dst, sint16 x, sint16 y, uint16 w, uint16 h);
void grRestoreBgRect(const uint16 *src, sint16 x, sint16 y, uint16 w, uint16 h);
int  grCaptureBgRectToSpu(uint32 spuOffset, sint16 x, sint16 y,
                           uint16 w, uint16 h, uint32 rowStrideBytes);
void grRestoreBgRectFromSpu(uint32 spuOffset, sint16 x, sint16 y,
                             uint16 w, uint16 h, uint32 rowStrideBytes);
void grRestoreBgTiles(void);
void grRestoreBackgroundRectForFrame(int x, int y, int width, int height);
void grRestoreAndCompositeDirect16BackgroundRectForFrame(int x, int y, int width, int height,
                                                         const uint16 *srcPixels);
extern struct TTtmThread *grCurrentThread;
void grClearScreen(PS1Surface *sfc);

/* GPU sprite rendering — per-frame lifecycle */
void grBeginFrame(void);
void grDebugOverlayBox(int x, int y, int w, int h, uint16 color);
void grReplaySprite(struct TDrawnSprite *ds);

/* Background tiles - exported for dirty rectangle wiping */
extern PS1Surface *bgTile0;
extern PS1Surface *bgTile1;
extern PS1Surface *bgTile3;
extern PS1Surface *bgTile4;
void grDrawBackground(void);
void grFadeOut();
void grPs1StatThreadDrop(void);
void grPs1StatBmpFrameCap(uint16 requested, uint16 cap);
void grPs1StatBmpShortLoad(uint16 requested, uint16 loaded);
void grPs1SetLastBmpTelemetry(uint16 slot, uint16 frames, uint16 status);

void grLoadPalette();
void grLoadScreen(char *strArg);
int  grLoadSceneExplorerThumbnail(const char *slug);
void grFreeSceneExplorerThumbnailBuffer(void);
void grSetPresentDuringScreenLoad(int enabled);
int grGetCurrentFrame(void);

/* Frame capture for visual regression testing */
int grCaptureFrame(const char *filename);
int grCaptureSequenceComplete(void);

#endif /* GRAPHICS_PS1_H */
