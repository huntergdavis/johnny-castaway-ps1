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

    data = (uint8*)ps1_loadRawFile("\\SCEXPL.DAT;1", &size);
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

static void sceneExplorerEnsureThumbnail(void)
{
    int cur = pauseMenuExplorerCursor;
    if (cur < 0 || cur >= gSceneExplorerCount) return;
    if (cur == sceneExplorerLoadedCursor) return;

    /* grLoadSceneExplorerThumbnail LoadImages the new thumbnail to the
     * centered framebuffer rect immediately. We do NOT touch
     * menuFramebufferPrimed here — re-priming would have grDrawBackground
     * repaint the paused scene's bgTile* over our thumbnail. The chrome
     * strip's OT primitives draw on top of the LoadImage'd pixels each
     * frame. */
    if (grLoadSceneExplorerThumbnail(gSceneExplorer[cur].slug)) {
        sceneExplorerLoadedCursor = cur;
    }
    /* If the load failed (missing thumbnail file on disc — e.g., a
     * not-yet-captured scene), leave fb alone. The chrome strip + text
     * still communicate the metadata; the previous thumbnail (or main
     * menu's bg) shows above. */
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

    /* Truncate display name so it fits in the 38-char top-band budget
     * (640 logical px / 16 logical per char ≈ 40 chars; we leave a
     * small right margin). */
    char title[40];
    int max = 38;
    int len = 0;
    while (displayName[len] != '\0' && len < max) {
        title[len] = displayName[len];
        len++;
    }
    if (displayName[len] != '\0' && len > 3) {
        title[len-3] = '.'; title[len-2] = '.'; title[len-1] = '.';
    }
    title[len] = '\0';

    /* Top band — title + position + display name + family/frames.
     * First line starts at y=24 logical so the title isn't clipped by
     * NTSC overscan at the very top of the framebuffer. */
    pmTextStart(pmPrintfX, 24);
    pmPrintf(pmText(PMT_SCENE_EXPLORER_TITLE));
    drawSeparator();
    pmPrintf(pmText(PMT_SCENE_EXPLORER_COUNT),
             cur + 1, gSceneExplorerCount,
             e->validated ? pmText(PMT_VALIDATED) : pmText(PMT_PENDING));
    pmPrintf(pmText(PMT_LINE_STRING), title);
    pmPrintf(pmText(PMT_SCENE_FAMILY_FRAMES),
             sceneExplorerFamilyName(e->family_index), (unsigned)e->frame_count);

    /* Bottom band — pack name, separator, nav hints (X/△/O on one
     * line so the band fits in 4 lines instead of 5 and we stay clear
     * of bottom NTSC overscan). */
    pmTextStart(pmPrintfX, 376);
    pmPrintf(pmText(PMT_SCENE_PACK), packLabel);
    drawSeparator();
    pmPrintf(pmText(PMT_SCENE_NAV));
    pmPrintf(pmText(PMT_SCENE_PLAY_LOOP));
}
