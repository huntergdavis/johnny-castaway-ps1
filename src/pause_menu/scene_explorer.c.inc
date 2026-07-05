/* ---------------------------------------------------------------------------
 *  Scene Explorer — chapter-select screen. Thumbnails are 320x240 raw
 *  RGB555 SCR files streamed directly into the centered framebuffer rect
 *  whenever the cursor changes; text/chrome are drawn over the loaded
 *  pixels in the normal pause-menu OT.
 * ------------------------------------------------------------------------- */

static int sceneExplorerFamilyForCursor(int cursor)
{
    int i;
    int family = 0;
    for (i = 0; i < gSceneExplorerFamilyCount; i++) {
        if (gSceneExplorerFamilyStart[i] <= cursor) {
            family = i;
        }
    }
    return family;
}

static uint8 *sceneExplorerStringData = NULL;
static uint32 sceneExplorerStringDataSize = 0;
static uint32 sceneExplorerStringBase = 0;
static int sceneExplorerStringDataValid = 0;
static char sceneExplorerLaunchName[64] = "";

static uint16 sceneExplorerReadLe16(const uint8 *p)
{
    return (uint16)((uint16)p[0] | ((uint16)p[1] << 8));
}

const char *sceneExplorerFamilyName(int familyIndex)
{
    switch (familyIndex) {
    case 0: return pmText(PMT_FAMILY_FISHING);
    case 1: return pmText(PMT_FAMILY_JOHNNY);
    case 2: return pmText(PMT_FAMILY_MARY);
    case 3: return pmText(PMT_FAMILY_VISITORS);
    case 4: return pmText(PMT_FAMILY_ACTIVITIES);
    case 5: return pmText(PMT_FAMILY_MISC_SUZY);
    case 6: return pmText(PMT_FAMILY_STANDING);
    case 7: return pmText(PMT_FAMILY_WALKING);
    case 8: return pmText(PMT_FAMILY_BUILDING);
    default:
        return pmText(PMT_UNKNOWN);
    }
}

static void sceneExplorerCopyText(char *dst, int dstSize, const char *src)
{
    int i;

    if (dst == NULL || dstSize <= 0)
        return;
    if (src == NULL)
        src = "";
    for (i = 0; i + 1 < dstSize && src[i] != '\0'; i++)
        dst[i] = src[i];
    dst[i] = '\0';
}

static void sceneExplorerReleaseStringData(void)
{
    if (sceneExplorerStringData != NULL) {
        free(sceneExplorerStringData);
        sceneExplorerStringData = NULL;
    }
    sceneExplorerStringDataSize = 0;
    sceneExplorerStringBase = 0;
    sceneExplorerStringDataValid = 0;
}

static int sceneExplorerValidateStringData(const uint8 *data, uint32 size)
{
    uint32 stringBase;
    uint32 stringBytes;
    uint16 count;
    int i;

    if (data == NULL || size < 8)
        return 0;
    if (data[0] != 'J' || data[1] != 'C' ||
        data[2] != 'S' || data[3] != 'X')
        return 0;
    if (sceneExplorerReadLe16(&data[4]) != 1)
        return 0;
    count = sceneExplorerReadLe16(&data[6]);
    if ((int)count != gSceneExplorerCount)
        return 0;

    stringBase = 8u;
    stringBytes = size - stringBase;

    for (i = 0; i < (int)count; i++) {
        uint32 offset = (uint32)gSceneExplorer[i].display_name_offset;
        uint32 pos = offset;
        if (offset >= stringBytes)
            return 0;
        while (pos < stringBytes && data[stringBase + pos] != '\0')
            pos++;
        if (pos >= stringBytes)
            return 0;
    }

    return 1;
}

static int sceneExplorerEnsureStringData(void)
{
    uint32 size = 0;
    uint8 *data;

    if (sceneExplorerStringDataValid)
        return 1;
    if (sceneExplorerStringData != NULL)
        return 0;

    {
        /* Browse-time read: same cosmetic policy as thumbnails. */
        extern void ps1CdSetCosmeticReads(int);
        ps1CdSetCosmeticReads(1);
        data = (uint8*)ps1_loadRawFile("\\SCEXPL.DAT;1", &size);
        ps1CdSetCosmeticReads(0);
    }
    if (data == NULL)
        return 0;
    if (!sceneExplorerValidateStringData(data, size)) {
        free(data);
        return 0;
    }

    sceneExplorerStringData = data;
    sceneExplorerStringDataSize = size;
    sceneExplorerStringBase = 8u;
    sceneExplorerStringDataValid = 1;
    return 1;
}

const char *sceneExplorerDisplayName(int index)
{
    uint32 offset;

    if (index < 0 || index >= gSceneExplorerCount)
        return "?";
    if (!sceneExplorerEnsureStringData())
        return gSceneExplorer[index].slug;

    offset = (uint32)gSceneExplorer[index].display_name_offset;
    if (sceneExplorerStringBase + offset >= sceneExplorerStringDataSize)
        return gSceneExplorer[index].slug;
    return (const char *)&sceneExplorerStringData[sceneExplorerStringBase + offset];
}

/* Catalog slug at a given explorer-order index. Lets the scene picker walk
 * scenes in the same order the pause menu numbers them ("Scene N/M"), so
 * Sequential advances 1,2,3,4 in displayed order. */
const char *sceneExplorerSlugAt(int index)
{
    if (index < 0 || index >= gSceneExplorerCount)
        return NULL;
    return gSceneExplorer[index].slug;
}

static void sceneExplorerRememberLaunchName(int index)
{
    sceneExplorerCopyText(sceneExplorerLaunchName,
                          sizeof(sceneExplorerLaunchName),
                          sceneExplorerDisplayName(index));
}

const char *sceneExplorerSelectedDisplayName(void)
{
    return sceneExplorerLaunchName[0] ? sceneExplorerLaunchName : "?";
}

static void sceneExplorerBuildPackLabel(int index, char *dst, int dstSize)
{
    const char *slug;
    int pos = 0;

    if (dst == NULL || dstSize <= 0)
        return;

    dst[0] = '\0';
    if (index < 0 || index >= gSceneExplorerCount)
        return;

    slug = gSceneExplorer[index].slug;
    if (pos + 1 < dstSize) dst[pos++] = 'F';
    if (pos + 1 < dstSize) dst[pos++] = 'G';
    if (pos + 1 < dstSize) dst[pos++] = '/';
    while (*slug != '\0' && pos + 1 < dstSize) {
        char c = *slug++;
        if (c >= 'a' && c <= 'z')
            c = (char)(c - 'a' + 'A');
        dst[pos++] = c;
    }
    if (pos + 1 < dstSize) dst[pos++] = '.';
    if (pos + 1 < dstSize) dst[pos++] = 'F';
    if (pos + 1 < dstSize) dst[pos++] = 'G';
    if (pos + 1 < dstSize) dst[pos++] = '2';
    dst[pos] = '\0';
}

/* Track which thumbnail is currently in VRAM. -1 forces a load on the
 * next sceneExplorerEnsureThumbnail call. Reset to -1 on entry to the
 * explorer state so the first cursor lands fresh; then advances on
 * each cursor change. */
static int sceneExplorerLoadedCursor = -1;
static int sceneExplorerFailedCursor = -1;
static int sceneExplorerSettleFrames = 0;
/* Circuit breaker: once the drive shows a persistent failure streak,
 * stop attempting thumbnail loads for the rest of this menu visit.
 * Each attempt on a struggling drive burns seconds of bounded retries
 * inside the draw path — the menu looks crashed and inputs land
 * between stalls (console report: START re-enters the explorer, O
 * appears dead, next scene load then fails: a read-failure cascade).
 * Metadata text keeps working; previews just stop updating. */
static int sceneExplorerThumbsOffline = 0;

void sceneExplorerReleaseStringDataPublic(void)
{
    sceneExplorerReleaseStringData();
}

static int sceneExplorerLastSeenCursor = -1;

void sceneExplorerResetThumbBreaker(void)
{
    sceneExplorerThumbsOffline = 0;
    /* Also force the settle debounce on (re-)entry: with the cursor
     * unchanged from the previous visit, the first explorer frame used
     * to start streaming IMMEDIATELY — in the SAME frame as the state-
     * transition reprime that just queued ~600 KB of bgTile LoadImages.
     * The settle gives the queue 12 real frames to drain first. */
    sceneExplorerLastSeenCursor = -1;
}

static int sceneExplorerRetryLeft = 0;
static int sceneExplorerRetryWait = 0;
static int sceneExplorerSettleTarget = 12;

static void sceneExplorerEnsureThumbnail(void)
{
    int cur = pauseMenuExplorerCursor;
    if (cur < 0 || cur >= gSceneExplorerCount) return;

    if (cur != sceneExplorerLastSeenCursor) {
        sceneExplorerLastSeenCursor = cur;
        sceneExplorerSettleFrames = 0;
        sceneExplorerFailedCursor = -1;
        sceneExplorerRetryLeft = 2;
        sceneExplorerRetryWait = 0;
        return;
    }
    if (sceneExplorerThumbsOffline)
        return;
    {
        extern int ps1CdSceneAbortNeeded(void);
        if (ps1CdSceneAbortNeeded()) {
            sceneExplorerThumbsOffline = 1;
            return;
        }
    }
    if (cur == sceneExplorerLoadedCursor) return;

    /* A failed load (missing thumbnail on disc, transient read error)
     * must NOT retry every frame: per-frame CdSearchFile + read streams
     * hammer a real drive until it latches an error and the recovery
     * reset stalls the menu for seconds. But a hard latch left one
     * transient blip (console D1) as a permanently part-black preview.
     * Allow two SPACED retries (~0.75 s apart) per cursor, then latch
     * for good until the cursor moves. */
    if (cur == sceneExplorerFailedCursor) {
        if (sceneExplorerRetryLeft <= 0) return;
        if (++sceneExplorerRetryWait < 45) return;
        sceneExplorerRetryWait = 0;
        sceneExplorerRetryLeft--;
        sceneExplorerFailedCursor = -1;   /* one more attempt below */
    }

    /* Debounce rapid paging: each thumbnail is a full synchronous CD
     * stream (~15 Setloc/Read cycles), so hammering left/right on a
     * real drive queues seconds of seeking. Stream only once the
     * cursor has been still for a few frames — the chrome text still
     * updates instantly, emulator behavior is unchanged apart from a
     * ~200 ms settle. */
    /* ADAPTIVE settle. Each preview is 15 chunked reads, each with its
     * own seek + stop/pause cycle — mechanically the hardest workload
     * in the product for a worn drive, while emulators do it in a few
     * vblanks. So let the measured load time pick the debounce:
     * fast loads (emulator / healthy drive) keep the snappy 12-frame
     * settle; a slow drive earns 75 (~1.25 s — fast paging then touches
     * the drive ZERO times); any read failure escalates to 120. */
    if (sceneExplorerSettleFrames < sceneExplorerSettleTarget) {
        sceneExplorerSettleFrames++;
        return;
    }

    /* grLoadSceneExplorerThumbnail LoadImages the new thumbnail to the
     * centered framebuffer rect immediately. We do NOT touch
     * menuFramebufferPrimed here — re-priming would have grDrawBackground
     * repaint the paused scene's bgTile* over our thumbnail. The chrome
     * strip's OT primitives draw on top of the LoadImage'd pixels each
     * frame. */
    {
        /* Cosmetic mode: browse failures are latched locally and must
         * not feed the scene-abort streak (post-menu kick-out) or the
         * freeze beacon (red bar over the live menu). */
        extern void ps1CdSetCosmeticReads(int);
        extern int ps1CdReadFailureCount(void);
        int ok;
        int startTick = VSync(-1);
        ps1CdSetCosmeticReads(1);
        ok = grLoadSceneExplorerThumbnail(gSceneExplorer[cur].slug);
        ps1CdSetCosmeticReads(0);
        /* Adapt the settle to what the hardware just showed us. */
        if (ps1CdReadFailureCount() > 0)
            sceneExplorerSettleTarget = 120;
        else if (VSync(-1) - startTick > 30 && sceneExplorerSettleTarget < 75)
            sceneExplorerSettleTarget = 75;
        if (ok) {
            sceneExplorerLoadedCursor = cur;
        } else {
            /* Leave fb alone (chrome strip + text still communicate the
             * metadata; the previous thumbnail shows above) and latch so
             * this slug isn't re-streamed every frame. */
            sceneExplorerFailedCursor = cur;
        }
    }
}

static void drawSceneExplorer(void)
{
    int cur = pauseMenuExplorerCursor;
    const char *displayName;
    char packLabel[24];
    if (cur < 0) cur = 0;
    if (cur >= gSceneExplorerCount) cur = gSceneExplorerCount - 1;

    /* Stream the current cursor's thumbnail SCR into the framebuffer
     * if it isn't already there. The OT primitives below add the
     * top + bottom chrome bands and the text. */
    sceneExplorerEnsureThumbnail();

    const struct TSceneExplorerEntry *e = &gSceneExplorer[cur];
    displayName = sceneExplorerDisplayName(cur);
    sceneExplorerBuildPackLabel(cur, packLabel, sizeof(packLabel));

    /* Wrap the display name across two 38-char top-band lines (640
     * logical px / 16 logical per char ≈ 40 chars; we leave a small
     * right margin). Break at the last space that fits; a remainder
     * longer than the second line still ends in "...". */
    char titleLine1[40];
    char titleLine2[40];
    int max = 38;
    int len = 0;
    int split = 0;
    while (displayName[len] != '\0' && len < max) {
        titleLine1[len] = displayName[len];
        len++;
    }
    split = len;
    if (displayName[len] != '\0') {
        int back = len;
        while (back > 0 && displayName[back] != ' ')
            back--;
        if (back > 0)
            split = back;
        len = split;
    }
    titleLine1[len] = '\0';
    {
        const char *rest = displayName + split;
        int len2 = 0;
        while (*rest == ' ')
            rest++;
        while (rest[len2] != '\0' && len2 < max) {
            titleLine2[len2] = rest[len2];
            len2++;
        }
        if (rest[len2] != '\0' && len2 > 3) {
            titleLine2[len2-3] = '.';
            titleLine2[len2-2] = '.';
            titleLine2[len2-1] = '.';
        }
        titleLine2[len2] = '\0';
    }

    /* Top band — title, separator, two description lines, family/frames.
     * The scene index moved down to the pack line and the validated
     * marker was retired to free the second description line. First
     * line starts at y=24 logical so the title isn't clipped by NTSC
     * overscan at the very top of the framebuffer. */
    pmTextStart(pmPrintfX, 24);
    pmPrintf(pmText(PMT_SCENE_EXPLORER_TITLE));
    drawSeparator();
    pmPrintf(pmText(PMT_LINE_STRING), titleLine1);
    pmPrintf(pmText(PMT_LINE_STRING), titleLine2);
    pmPrintf(pmText(PMT_SCENE_FAMILY_FRAMES),
             sceneExplorerFamilyName(e->family_index), (unsigned)e->frame_count);

    /* Bottom band — pack name + scene index, separator, nav hints
     * (X/△/O on one line so the band fits in 4 lines instead of 5 and
     * we stay clear of bottom NTSC overscan). */
    pmTextStart(pmPrintfX, 376);
    pmPrintf(pmText(PMT_SCENE_PACK), packLabel, cur + 1, gSceneExplorerCount);
    /* 1.0: the P/Q/R/W/c/D/S diag line that lived in this slot through
     * the 0.9.13/0.9.14 hardware campaign is retired from the display;
     * the counters remain live (grSxDiag*, ps1Cd*Count,
     * pauseMenuGpuUnwedgeCount) for the next investigation. */
    drawSeparator();
    pmPrintf(pmText(PMT_SCENE_NAV));
    pmPrintf(pmText(PMT_SCENE_PLAY_LOOP));
}
