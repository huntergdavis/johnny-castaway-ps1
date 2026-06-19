void grSetPresentDuringScreenLoad(int enabled)
{
    grPresentDuringScreenLoad = enabled ? 1 : 0;
}

static void grClearDirtyRows(sint16 rowsMinX[4][BG_TILE_HEIGHT],
                             sint16 rowsMaxX[4][BG_TILE_HEIGHT])
{
    for (int i = 0; i < 4; i++) {
        for (int y = 0; y < BG_TILE_HEIGHT; y++) {
            rowsMinX[i][y] = -1;
            rowsMaxX[i][y] = -1;
        }
    }
}

static void grEnsureDirtyRowState(void)
{
    if (dirtyRowStateInitialized)
        return;

    grClearDirtyRows(currDirtyRowMinX, currDirtyRowMaxX);
    grClearDirtyRows(prevDirtyRowMinX, prevDirtyRowMaxX);
    dirtyRowStateInitialized = 1;
}

static void grClearDirtyRowRange(sint16 rowsMinX[4][BG_TILE_HEIGHT],
                                 sint16 rowsMaxX[4][BG_TILE_HEIGHT],
                                 int idx,
                                 int minY,
                                 int maxY)
{
    if (idx < 0 || idx >= 4)
        return;
    if (minY < 0)
        minY = 0;
    if (maxY >= BG_TILE_HEIGHT)
        maxY = BG_TILE_HEIGHT - 1;
    if (minY > maxY)
        return;

    for (int y = minY; y <= maxY; y++) {
        rowsMinX[idx][y] = -1;
        rowsMaxX[idx][y] = -1;
    }
}

static void grClearCurrDirtyState(void)
{
    grEnsureDirtyRowState();
    for (int i = 0; i < 4; i++) {
        grClearDirtyRowRange(currDirtyRowMinX, currDirtyRowMaxX,
                             i, currDirtyMinY[i], currDirtyMaxY[i]);
        currDirtyMinX[i] = -1;
        currDirtyMaxX[i] = -1;
        currDirtyMinY[i] = -1;
        currDirtyMaxY[i] = -1;
    }
}

static void grPromoteCurrDirtyToPrev(void)
{
    grEnsureDirtyRowState();
    for (int i = 0; i < 4; i++) {
        int minY;
        int maxY;

        grClearDirtyRowRange(prevDirtyRowMinX, prevDirtyRowMaxX,
                             i, prevDirtyMinY[i], prevDirtyMaxY[i]);

        prevDirtyMinX[i] = currDirtyMinX[i];
        prevDirtyMaxX[i] = currDirtyMaxX[i];
        prevDirtyMinY[i] = currDirtyMinY[i];
        prevDirtyMaxY[i] = currDirtyMaxY[i];

        minY = currDirtyMinY[i];
        maxY = currDirtyMaxY[i];
        if (minY < 0)
            continue;
        if (maxY >= BG_TILE_HEIGHT)
            maxY = BG_TILE_HEIGHT - 1;
        for (int y = minY; y <= maxY; y++) {
            prevDirtyRowMinX[i][y] = currDirtyRowMinX[i][y];
            prevDirtyRowMaxX[i][y] = currDirtyRowMaxX[i][y];
        }
    }
}

static void grClearPrevDirtyState(void)
{
    grEnsureDirtyRowState();
    for (int i = 0; i < 4; i++) {
        grClearDirtyRowRange(prevDirtyRowMinX, prevDirtyRowMaxX,
                             i, prevDirtyMinY[i], prevDirtyMaxY[i]);
        prevDirtyMinX[i] = -1;
        prevDirtyMaxX[i] = -1;
        prevDirtyMinY[i] = -1;
        prevDirtyMaxY[i] = -1;
    }
}

static void grMarkDirtyRows(sint16 rowsMinX[4][BG_TILE_HEIGHT],
                            sint16 rowsMaxX[4][BG_TILE_HEIGHT],
                            int idx,
                            int minX,
                            int maxX,
                            int minY,
                            int maxY)
{
    if (idx < 0 || idx >= 4)
        return;
    if (minX < 0) minX = 0;
    if (maxX > 319) maxX = 319;
    if (minY < 0) minY = 0;
    if (maxY > 239) maxY = 239;
    if (minX > maxX || minY > maxY)
        return;

    grEnsureDirtyRowState();
    for (int y = minY; y <= maxY; y++) {
        if (rowsMinX[idx][y] < 0) {
            rowsMinX[idx][y] = (sint16)minX;
            rowsMaxX[idx][y] = (sint16)maxX;
        } else {
            if (minX < rowsMinX[idx][y]) rowsMinX[idx][y] = (sint16)minX;
            if (maxX > rowsMaxX[idx][y]) rowsMaxX[idx][y] = (sint16)maxX;
        }
    }
}

static void grMarkPrevTileDirtyRect(int idx, int minX, int maxX, int minY, int maxY)
{
    if (idx < 0 || idx >= 4)
        return;
    if (minX < 0) minX = 0;
    if (maxX > 319) maxX = 319;
    if (minY < 0) minY = 0;
    if (maxY > 239) maxY = 239;
    if (minX > maxX || minY > maxY)
        return;

    prevDirtyMinX[idx] = minX;
    prevDirtyMaxX[idx] = maxX;
    prevDirtyMinY[idx] = minY;
    prevDirtyMaxY[idx] = maxY;
    grMarkDirtyRows(prevDirtyRowMinX, prevDirtyRowMaxX, idx, minX, maxX, minY, maxY);
}

static void grMarkPrevAllTilesDirty(void)
{
    grEnsureDirtyRowState();
    grClearDirtyRows(prevDirtyRowMinX, prevDirtyRowMaxX);
    for (int i = 0; i < 4; i++)
        grMarkPrevTileDirtyRect(i, 0, 319, 0, 239);
}

static inline void markTileDirtyRect(int idx, int minX, int maxX, int minY, int maxY)
{
    if (idx < 0 || idx >= 4)
        return;
    if (minX < 0) minX = 0;
    if (maxX > 319) maxX = 319;
    if (minY < 0) minY = 0;
    if (maxY > 239) maxY = 239;
    if (minX > maxX || minY > maxY)
        return;

    if (currDirtyMinY[idx] < 0) {
        currDirtyMinX[idx] = minX;
        currDirtyMaxX[idx] = maxX;
        currDirtyMinY[idx] = minY;
        currDirtyMaxY[idx] = maxY;
    } else {
        if (minX < currDirtyMinX[idx]) currDirtyMinX[idx] = minX;
        if (maxX > currDirtyMaxX[idx]) currDirtyMaxX[idx] = maxX;
        if (minY < currDirtyMinY[idx]) currDirtyMinY[idx] = minY;
        if (maxY > currDirtyMaxY[idx]) currDirtyMaxY[idx] = maxY;
    }
    grMarkDirtyRows(currDirtyRowMinX, currDirtyRowMaxX, idx, minX, maxX, minY, maxY);
}

static inline void markTileDirty(int idx, int minY, int maxY)
{
    markTileDirtyRect(idx, 0, 319, minY, maxY);
}

static inline void grMarkSingleColumnDirty(int tileBaseX,
                                           int x,
                                           int width,
                                           int y0,
                                           int y1Exclusive)
{
    int idxBase = (tileBaseX == 0) ? 0 : 1;
    int minX = x - tileBaseX;
    int maxX = minX + width - 1;

    if (width <= 0)
        return;

    if (y0 < 240) {
        int minY = y0;
        int maxY = (y1Exclusive < 240) ? (y1Exclusive - 1) : 239;
        markTileDirtyRect(idxBase, minX, maxX, minY, maxY);
    }

    if (y1Exclusive > 240) {
        int minY = (y0 > 240) ? (y0 - 240) : 0;
        int maxY = y1Exclusive - 241;
        if (maxY > 239)
            maxY = 239;
        markTileDirtyRect(idxBase + 2, minX, maxX, minY, maxY);
    }
}

void grMarkAllTilesDirty(void)
{
    grEnsureDirtyRowState();
    grClearDirtyRows(currDirtyRowMinX, currDirtyRowMaxX);
    for (int i = 0; i < 4; i++) {
        currDirtyMinX[i] = 0;
        currDirtyMaxX[i] = 319;
        currDirtyMinY[i] = 0;
        currDirtyMaxY[i] = 239;
        grMarkDirtyRows(currDirtyRowMinX, currDirtyRowMaxX, i, 0, 319, 0, 239);
    }
}

/* Force the next grDrawBackground call to upload all 4 tiles in full.
 * Sets BOTH currDirty and prevDirty because grRestoreBgTiles resets
 * currDirty at the start of every frame. prevDirty survives that
 * reset and feeds into grDrawBackground's union(prev, curr) upload
 * range. Mirrors the pattern in grFadeOut / grFreeCleanBgTiles. */
void grForceFullRedrawNextFrame(void)
{
    /* Latch a pending full upload that survives any dirty-state clear before
     * the next grDrawBackground (the dirty arrays below can be wiped by a
     * residual-clean present path; the flag cannot). */
    grForcedFullRedrawPending = 1;
    grEnsureDirtyRowState();
    grClearDirtyRows(currDirtyRowMinX, currDirtyRowMaxX);
    grClearDirtyRows(prevDirtyRowMinX, prevDirtyRowMaxX);
    for (int i = 0; i < 4; i++) {
        currDirtyMinX[i] = 0;
        currDirtyMaxX[i] = 319;
        currDirtyMinY[i] = 0;
        currDirtyMaxY[i] = 239;
        prevDirtyMinX[i] = 0;
        prevDirtyMaxX[i] = 319;
        prevDirtyMinY[i] = 0;
        prevDirtyMaxY[i] = 239;
        grMarkDirtyRows(currDirtyRowMinX, currDirtyRowMaxX, i, 0, 319, 0, 239);
        grMarkDirtyRows(prevDirtyRowMinX, prevDirtyRowMaxX, i, 0, 319, 0, 239);
    }
}

/* Mark dirty region from a screen-space rectangle (x0,y0)-(x1,y1) exclusive */
static void grMarkRectDirty(int x0, int y0, int x1, int y1)
{
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > 640) x1 = 640;
    if (y1 > 480) y1 = 480;
    if (x0 >= x1 || y0 >= y1) return;

    /* Top row tiles (screen y 0-239) */
    if (y0 < 240) {
        int ty0 = y0;
        int ty1 = (y1 < 240) ? y1 - 1 : 239;
        if (x0 < 320) {
            int tx1 = (x1 < 320) ? x1 - 1 : 319;
            markTileDirtyRect(0, x0, tx1, ty0, ty1);
        }
        if (x1 > 320) {
            int tx0 = (x0 > 320) ? x0 - 320 : 0;
            int tx1 = x1 - 321;
            markTileDirtyRect(1, tx0, tx1, ty0, ty1);
        }
    }
    /* Bottom row tiles (screen y 240-479) */
    if (y1 > 240) {
        int ty0 = (y0 > 240) ? y0 - 240 : 0;
        int ty1 = y1 - 240 - 1;
        if (ty1 > 239) ty1 = 239;
        if (x0 < 320) {
            int tx1 = (x1 < 320) ? x1 - 1 : 319;
            markTileDirtyRect(2, x0, tx1, ty0, ty1);
        }
        if (x1 > 320) {
            int tx0 = (x0 > 320) ? x0 - 320 : 0;
            int tx1 = x1 - 321;
            markTileDirtyRect(3, tx0, tx1, ty0, ty1);
        }
    }
}

void grMarkScreenRectDirty(int x0, int y0, int x1, int y1)
{
    grMarkRectDirty(x0, y0, x1, y1);
}

static void grRebuildPaletteLuts(void)
{
    for (int i = 0; i < 256; i++) {
        /* Sierra: high nibble = even pixel, low nibble = odd pixel */
        uint16 pe = ttmPalette[(i >> 4) & 0x0F];
        uint16 po = ttmPalette[i & 0x0F];
        palLutSierra[i] = (uint32)pe | ((uint32)po << 16);
        /* PSB: low nibble = even pixel, high nibble = odd pixel */
        pe = ttmPalette[i & 0x0F];
        po = ttmPalette[(i >> 4) & 0x0F];
        palLutPsb[i] = (uint32)pe | ((uint32)po << 16);
    }
}

struct TPs1SavedZone {
    uint16 x;
    uint16 y;
    uint16 width;
    uint16 height;
    uint8 valid;
};

static struct TPs1SavedZone grPs1SavedZone = {0, 0, 0, 0, 0};

/* Global variables matching original implementation */
int grDx = 0;
int grDy = 0;
int grWindowed = 0;  /* PS1 is always fullscreen, but keep for compatibility */
int grUpdateDelay = 0;

/* Frame capture - stubbed for PS1 */
int grCaptureFrameNumber = -1;
int grCaptureForegroundOnly = 0;
char *grCaptureFilename = NULL;
char *grCaptureDir = NULL;
char *grCaptureMetaDir = NULL;
int grCaptureInterval = 0;
int grCaptureStartFrame = 0;
int grCaptureEndFrame = -1;
int grCaptureOverlay = 0;
int grCaptureOverlayMaskOnly = 0;
int grCaptureForegroundIncludeStaticBase = 0;
int grCaptureForegroundSkipVisibilityMask = 0;
char *grCaptureSoundEventsPath = NULL;

/* Flag to track if GPU was already initialized (e.g., by loadTitleScreenEarly) */
int grGpuAlreadyInitialized = 0;

/* Current thread being played - used to record sprite draws for replay */
struct TTtmThread *grCurrentThread = NULL;
int grPs1TelemetryEnabled = 0;

/* Persistent debug counters for sprite/frame clipping diagnostics. */
static uint32 gStatThreadDrops = 0;
static uint32 gStatBmpFrameCapHits = 0;
static uint32 gStatBmpShortLoads = 0;
static uint16 gStatBmpMaxRequested = 0;
static uint16 gStatBmpMinLoaded = 0xFFFF;
static uint16 gStatLastBmpSlot = 0;
static uint16 gStatLastBmpFrames = 0;
static uint16 gStatLastBmpStatus = 0;

/* Story transition diagnostics from story.c */
extern uint16 ps1StoryDbgPhase;
extern uint16 ps1StoryDbgSceneTag;
extern uint16 ps1StoryDbgAdsSig;
extern uint16 ps1StoryDbgPrevSpot;
extern uint16 ps1StoryDbgPrevHdg;
extern uint16 ps1StoryDbgNextSpot;
extern uint16 ps1StoryDbgNextHdg;
extern uint16 ps1StoryDbgSeq;
extern uint16 ps1AdsDbgActiveThreads;
extern uint16 ps1AdsDbgMini;
extern uint16 ps1AdsDbgRunningThreads;
extern uint16 ps1AdsDbgTerminatedThreads;
extern uint16 ps1AdsDbgSceneSlot;
extern uint16 ps1AdsDbgSceneTag;
extern uint16 ps1AdsDbgReplayCount;
extern uint16 ps1AdsDbgReplayTryFrame;
extern uint16 ps1AdsDbgReplayDrawFrame;
extern uint16 ps1AdsDbgMergeCarryFrame;
extern uint16 ps1AdsDbgNoDrawThreadsFrame;
extern uint16 ps1AdsDbgPlayedThreadsFrame;
extern uint16 ps1AdsDbgRecordedSpritesFrame;
extern uint16 ps1AdsDbgLastStopThread;
extern uint16 ps1AdsDbgLastStopSceneSig;
extern uint16 ps1AdsDbgLastReapThread;
extern uint16 ps1AdsDbgLastReapSceneSig;
extern uint16 ps1AdsDbgLastAddThread;
extern uint16 ps1AdsDbgLastAddSceneSig;
extern uint16 ps1PilotDbgActivePack;
extern uint16 ps1PilotDbgHits;
extern uint16 ps1PilotDbgFallbacks;
extern uint16 ps1PilotDbgLastHitEntry;
extern uint16 ps1PilotDbgLastFallbackEntry;
extern uint16 ps1PilotDbgFallbackWhilePackActive;

void grPs1StatThreadDrop(void)
{
    if (gStatThreadDrops < 0xFFFFFFFFU) gStatThreadDrops++;
}

void grPs1StatBmpFrameCap(uint16 requested, uint16 cap)
{
    (void)cap;
    if (gStatBmpFrameCapHits < 0xFFFFFFFFU) gStatBmpFrameCapHits++;
    if (requested > gStatBmpMaxRequested) gStatBmpMaxRequested = requested;
}

void grPs1StatBmpShortLoad(uint16 requested, uint16 loaded)
{
    if (gStatBmpShortLoads < 0xFFFFFFFFU) gStatBmpShortLoads++;
    if (requested > gStatBmpMaxRequested) gStatBmpMaxRequested = requested;
    if (loaded < gStatBmpMinLoaded) gStatBmpMinLoaded = loaded;
}

void grPs1SetLastBmpTelemetry(uint16 slot, uint16 frames, uint16 status)
{
    gStatLastBmpSlot = slot;
    gStatLastBmpFrames = frames;
    gStatLastBmpStatus = status;
}

/* VRAM allocation tracking
 * VRAM Layout for 640x480 interlaced:
 * (0,0)-(639,479): Framebuffer (single buffer for 640x480)
 * (640,0)-(656,1): CLUT (16 colors)
 * (640,2)-(895,2): CLUT 256 (grayscale)
 * (640,4) onwards: Textures
 */
static uint16 nextVRAMX = 640;  /* Start to the right of framebuffer */
static uint16 nextVRAMY = 4;    /* Below CLUTs */

static void grResetVramCursor(void)
{
    nextVRAMX = 640;
    nextVRAMY = 4;
}


/*
 * Initialize PS1 graphics subsystem
 */
void graphicsInit()
{
    /* Skip GPU reset if already initialized (e.g., by loadTitleScreenEarly)
     * CRITICAL: Calling ResetGraph(0) after GPU is already initialized
     * conflicts with the existing GPU state and causes hangs */
    if (!grGpuAlreadyInitialized) {
        GR_DIAG_PRINTF("GPU: Resetting GPU...\n");

        /* Reset GPU and set video mode */
        ResetGraph(0);
        SetVideoMode(MODE_NTSC);

        GR_DIAG_PRINTF("GPU: Initializing GTE...\n");

        /* Initialize geometry transformation engine */
        InitGeom();

        GR_DIAG_PRINTF("GPU: Setting up display buffers (%dx%d)...\n", SCREEN_WIDTH, SCREEN_HEIGHT);

        /* Setup display environments for 640x480 interlaced mode
         * Single buffer mode since 2x640x480 won't fit in VRAM
         * Both buffers point to same location - no flipping needed */
        SetDefDispEnv(&disp[0], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        SetDefDispEnv(&disp[1], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        /* Enable interlaced mode for 640x480 */
        disp[0].isinter = 1;
        disp[1].isinter = 1;

        /* Setup drawing environments - both draw to same buffer */
        SetDefDrawEnv(&draw[0], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        SetDefDrawEnv(&draw[1], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        /* Set background clear color - but DON'T enable clear!
         * We use grDrawBackground() LoadImage to paint the background each frame,
         * so isbg=0 preserves it for OT primitives to draw on top */
        setRGB0(&draw[0], 0, 0, 0);
        setRGB0(&draw[1], 0, 0, 0);
        draw[0].isbg = 0;  /* Don't clear - grDrawBackground handles it */
        draw[1].isbg = 0;

        GR_DIAG_PRINTF("GPU: Enabling display...\n");

        /* Enable display */
        SetDispMask(1);

        /* Apply first buffer */
        PutDispEnv(&disp[db]);
        PutDrawEnv(&draw[db]);
    } else {
        GR_DIAG_PRINTF("GPU: Skipping ResetGraph - already initialized\n");

        /* Still need to setup display/draw environments for rendering
         * GPU is already on, but we need proper environment structs */
        SetDefDispEnv(&disp[0], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        SetDefDispEnv(&disp[1], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        disp[0].isinter = 1;  /* Interlaced mode for 640x480 */
        disp[1].isinter = 1;

        SetDefDrawEnv(&draw[0], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        SetDefDrawEnv(&draw[1], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        setRGB0(&draw[0], 0, 0, 0);
        setRGB0(&draw[1], 0, 0, 0);
        draw[0].isbg = 0;  /* Don't clear - grDrawBackground handles it */
        draw[1].isbg = 0;

        /* Apply environments immediately (like the non-ELSE branch) */
        PutDispEnv(&disp[db]);
        PutDrawEnv(&draw[db]);
    }

    GR_DIAG_PRINTF("GPU: Initializing ordering tables...\n");

    /* Clear ordering tables */
    ClearOTagR(ot[0], OT_LENGTH);
    ClearOTagR(ot[1], OT_LENGTH);

    /* Reverted to libc — 2x32 KB exceeds the reduced BOOT budget. */
    primitiveBuffer[0] = (uint8*)malloc(PRIMITIVE_BUFFER_SIZE);
    primitiveBuffer[1] = (uint8*)malloc(PRIMITIVE_BUFFER_SIZE);
    if (!primitiveBuffer[0] || !primitiveBuffer[1]) {
        printf("ERROR: Failed to allocate primitive buffers\n");
        while(1);
    }

    /* Initialize primitive buffers */
    nextPrimitive[0] = primitiveBuffer[0];
    nextPrimitive[1] = primitiveBuffer[1];
    primitiveIndex[0] = 0;
    primitiveIndex[1] = 0;

    GR_DIAG_PRINTF("GPU: Loading default palette...\n");

    /* Load default palette with distinct, bright colors for testing
     * PS1 uses BGR555 format: (B << 10) | (G << 5) | R
     * Each channel is 5 bits (0-31) */
    ttmPalette[0]  = (0 << 10)  | (0 << 5)  | 0;   /* 0: Black */
    ttmPalette[1]  = (0 << 10)  | (0 << 5)  | 31;  /* 1: Red */
    ttmPalette[2]  = (0 << 10)  | (31 << 5) | 0;   /* 2: Green */
    ttmPalette[3]  = (31 << 10) | (0 << 5)  | 0;   /* 3: Blue */
    ttmPalette[4]  = (0 << 10)  | (31 << 5) | 31;  /* 4: Yellow */
    ttmPalette[5]  = (31 << 10) | (0 << 5)  | 31;  /* 5: Magenta */
    ttmPalette[6]  = (31 << 10) | (31 << 5) | 0;   /* 6: Cyan */
    ttmPalette[7]  = (31 << 10) | (31 << 5) | 31;  /* 7: White */
    ttmPalette[8]  = (16 << 10) | (16 << 5) | 16;  /* 8: Gray */
    ttmPalette[9]  = (0 << 10)  | (0 << 5)  | 20;  /* 9: Dark Red */
    ttmPalette[10] = (0 << 10)  | (20 << 5) | 0;   /* 10: Dark Green */
    ttmPalette[11] = (20 << 10) | (0 << 5)  | 0;   /* 11: Dark Blue */
    ttmPalette[12] = (0 << 10)  | (20 << 5) | 20;  /* 12: Orange */
    ttmPalette[13] = (20 << 10) | (0 << 5)  | 20;  /* 13: Purple */
    ttmPalette[14] = (20 << 10) | (20 << 5) | 0;   /* 14: Teal */
    ttmPalette[15] = (20 << 10) | (20 << 5) | 20;  /* 15: Light Gray */

    /* Upload 16-color CLUT for primitives at (640, 0) - right of framebuffer */
    RECT clutRect16;
    setRECT(&clutRect16, 640, 0, 16, 1);  /* 16 colors, 1 row */
    LoadImage(&clutRect16, (uint32*)ttmPalette);

    /* Create and upload 256-color grayscale CLUT for SCR textures at (640, 2) */
    static uint16 clut256[256];
    for (int i = 0; i < 256; i++) {
        /* Convert 8-bit grayscale to BGR555 */
        uint8 val = (i >> 3) & 0x1F;  /* Scale 0-255 to 0-31 */
        clut256[i] = (val << 10) | (val << 5) | val;  /* Grayscale */
    }
    RECT clutRect256;
    setRECT(&clutRect256, 640, 2, 256, 1);  /* 256 colors, 1 row */
    LoadImage(&clutRect256, (uint32*)clut256);

    GR_DIAG_PRINTF("GPU: Initializing event system...\n");

    /* Initialize event system */
    eventsInit();
    GR_DIAG_PRINTF("GPU: Graphics initialization complete!\n");

    /* Mark graphics ready — used by memHalt to decide between the
     * full BSOD panel (ps1Bsod) and the minimal pre-graphics text
     * panel (ps1DebugError). See plan v9 "Failure UX". */
    extern void memSetGraphicsReady(int ready);
    memSetGraphicsReady(1);
}

/* Mem-region allocator queries this to choose its halt path. Defined
 * here so the flag lives next to graphicsInit; the allocator uses
 * `extern int graphicsIsInitialized(void)` via mem_region.c. */
static int gGraphicsReady = 0;

int graphicsIsInitialized(void) {
    return gGraphicsReady;
}

void memSetGraphicsReady(int ready) {
    gGraphicsReady = ready ? 1 : 0;
}

/*
 * Shutdown graphics subsystem
 */
void graphicsEnd()
{
    /* Free allocated surfaces */
    if (grBackgroundSfc != NULL) {
        grFreeLayer(grBackgroundSfc);
        grBackgroundSfc = NULL;
    }

    if (grSavedZonesLayer != NULL) {
        grFreeLayer(grSavedZonesLayer);
        grSavedZonesLayer = NULL;
    }

    SetDispMask(0);
}

/*
 * Load palette from resource
 */
void grLoadPalette(struct TPalResource *palResource)
{
    if (palResource == NULL) {
        fatalError("NULL palette\n");
    }

    /* Convert VGA 6-bit RGB to PS1 15-bit RGB (5-5-5)
     * Magenta (0xa8, 0, 0xa8) = VGA 6-bit (42, 0, 42) is the transparent color.
     * Match SDL version: only EXACT magenta (168, 0, 168 in 8-bit) is transparent. */
    for (int i = 0; i < 16; i++) {
        uint8 vgaR = palResource->colors[i].r;  /* 6-bit VGA values (0-63) */
        uint8 vgaG = palResource->colors[i].g;
        uint8 vgaB = palResource->colors[i].b;

        /* Convert to 8-bit like SDL version */
        uint8 r8 = vgaR << 2;
        uint8 g8 = vgaG << 2;
        uint8 b8 = vgaB << 2;

        /* Check for EXACT magenta (0xa8=168, 0, 0xa8=168) only - matches SDL behavior */
        int isMagenta = (r8 == 0xa8) && (g8 == 0) && (b8 == 0xa8);

        if (isMagenta) {
            /* Magenta = 0x0000 = fully transparent on PS1 */
            ttmPalette[i] = 0x0000;
        } else {
            uint8 r = r8 >> 3;  /* 8-bit to 5-bit */
            uint8 g = g8 >> 3;
            uint8 b = b8 >> 3;
            uint16 color = (b << 10) | (g << 5) | r;

            /* IMPORTANT: If color is 0x0000 (black), use 0x0001 instead
             * because 0x0000 is reserved for transparency on PS1 */
            if (color == 0x0000) {
                color = 0x0001;  /* Very dark blue, nearly black */
            }
            ttmPalette[i] = color;
        }
    }

    /* Upload CLUT (Color Lookup Table) to VRAM */
    RECT clutRect;
    setRECT(&clutRect, 640, 0, 16, 1);  /* 16 colors, 1 row, at (640, 0) */
    LoadImage(&clutRect, (uint32*)ttmPalette);

    /* Rebuild byte-pair LUTs for compositing span functions */
    grRebuildPaletteLuts();
}

/*
 * Swap display buffers
 */
void grRefreshDisplay()
{
    /* Wait for GPU DMA to finish (LoadImage operations) */
    DrawSync(0);

    /* VSync is called in grUpdateDisplay before LoadImage to prevent tearing.
     * This function just ensures DMA is complete for standalone use cases. */
}

/*
 * Toggle fullscreen (no-op on PS1, always fullscreen)
 */
void grToggleFullScreen()
{
    /* PS1 is always fullscreen */
    grWindowed = !grWindowed;  /* Keep variable for compatibility */
}

/*
 * Per-frame initialisation for the remaining primitive-buffer path.
 */
void grBeginFrame(void)
{
    /* Reset OT and primitive buffer each frame.
     * Required because VRAM-based sprites (from grLoadBmp) still emit
     * GPU primitives into the OT — without reset the buffer overflows. */
    ClearOTagR(ot[0], OT_LENGTH);
    nextPrimitive[0] = primitiveBuffer[0];
    primitiveIndex[0] = 0;
}

/*
 * Replay a previously-drawn sprite via GPU upload.
 * Falls back to software composite if GPU path fails.
 */
void grReplaySprite(struct TDrawnSprite *ds)
{
    if (!ds || !ds->indexedPixels) return;

    PS1Surface tmpSfc = {0};
    tmpSfc.indexedPixels = ds->indexedPixels;
    tmpSfc.width = ds->width;
    tmpSfc.height = ds->height;
    tmpSfc.psbNibbles = ds->psbNibbles;
    if (ds->flip)
        grCompositeToBackgroundFlip(&tmpSfc, ds->x, ds->y);
    else
        grCompositeToBackground(&tmpSfc, ds->x, ds->y);
}

/*
 * Update display with all layers
 */
__attribute__((optimize("Os")))
void grUpdateDisplay(struct TTtmThread *ttmBackgroundThread,
                     struct TTtmThread *ttmThreads,
                     struct TTtmThread *ttmHolidayThread)
{
    int perfDetail = ps1PerfEnabled ? ps1PerfDetailEnabled() : 0;
    uint32 perfTick = 0;

    /* PS1 uses RAM-based compositing - sprites are drawn to bgTile buffers
     * via grCompositeToBackground(). By this point:
     * - grRestoreBgTiles was called before ttmPlay (at frame start)
     * - ttmPlay drew sprites to bgTile via grCompositeToBackground
     * Now we just need to upload and display.
     */

    ps1FrameCount++;

    /* Wait for VSync BEFORE uploading to framebuffer.
     * This ensures we write during vertical blank when display isn't scanning. */
    if (perfDetail)
        perfTick = ps1PerfTick();
    VSync(0);
    if (perfDetail)
        ps1PerfMarkRenderPhase(PS1_PERF_RENDER_PRESENT_WAIT,
                               ps1PerfElapsedVBlanks(perfTick));

    if (perfDetail)
        perfTick = ps1PerfTick();
    if (foregroundPilotRuntimeActive())
        foregroundPilotRuntimeCompose();
    if (perfDetail)
        ps1PerfMarkRenderPhase(PS1_PERF_RENDER_COMPOSE,
                               ps1PerfElapsedVBlanks(perfTick));

    /* Upload background tiles (with sprites composited in software) to framebuffer */
    if (perfDetail)
        perfTick = ps1PerfTick();
    grDrawBackground();
    if (perfDetail)
        ps1PerfMarkRenderPhase(PS1_PERF_RENDER_UPLOAD,
                               ps1PerfElapsedVBlanks(perfTick));

    /* Closed captions overlay — drawn AFTER the scene LoadImage so the
     * dark band + text land on top of the frame the user sees this
     * VSync. No-op when captions are off or no text is queued. */
    if (ps1CaptionsEnabled)
        captionsRender();

    /* Handle frame timing */
    if (perfDetail)
        perfTick = ps1PerfTick();
    eventsWaitTick(grUpdateDelay);
    if (perfDetail)
        ps1PerfMarkRenderPhase(PS1_PERF_RENDER_EVENT_WAIT,
                               ps1PerfElapsedVBlanks(perfTick));

}

/*
 * Create a new empty background surface
 */
PS1Surface *grNewEmptyBackground()
{
    /* Kept on libc malloc — this is called from island.c at scene
     * runtime (via grNewLayer for TTM threads), not strictly at boot.
     * Migrating to BOOT would block memFreezeBoot enablement.
     * Future: route through MEM_REGION_CACHE with corresponding
     * memFree in the TTM-thread shutdown path. */
    PS1Surface *sfc = (PS1Surface*)malloc(sizeof(PS1Surface));
    if (sfc == NULL) {
        fatalError("Failed to allocate PS1Surface");
    }

    sfc->width = SCREEN_WIDTH;
    sfc->height = SCREEN_HEIGHT;
    sfc->x = nextVRAMX;
    sfc->y = nextVRAMY;
    sfc->pixels = NULL;  /* Will be allocated in VRAM */
    sfc->indexedPixels = NULL;
    sfc->indexedOwned = 0;
    sfc->psbNibbles = 0;
    sfc->nextTile = NULL;

    /* Update VRAM allocation tracking */
    nextVRAMY += SCREEN_HEIGHT;
    if (nextVRAMY >= 512) {  /* VRAM height limit */
        nextVRAMX += SCREEN_WIDTH;
        nextVRAMY = 0;
    }

    return sfc;
}

/*
 * Create a new layer surface for TTM animations
 */
PS1Surface *grNewLayer()
{
    return grNewEmptyBackground();
}

/*
 * Free a layer surface
 */
void grFreeLayer(PS1Surface *sfc)
{
    while (sfc != NULL) {
        PS1Surface *next = sfc->nextTile;
        if (sfc->indexedPixels && sfc->indexedOwned) free(sfc->indexedPixels);
        if (sfc->pixels) free(sfc->pixels);
        free(sfc);
        sfc = next;
    }
}

/*
 * Load BMP sprite sheet into slot
 */
