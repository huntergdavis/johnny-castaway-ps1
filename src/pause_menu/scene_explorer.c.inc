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
    if (cur < 0) cur = 0;
    if (cur >= gSceneExplorerCount) cur = gSceneExplorerCount - 1;

    /* Stream the current cursor's thumbnail SCR into the framebuffer
     * if it isn't already there. The OT primitives below add the
     * top + bottom chrome bands and the text. */
    sceneExplorerEnsureThumbnail();

    const struct TSceneExplorerEntry *e = &gSceneExplorer[cur];

    /* Truncate display name so it fits in the 38-char top-band budget
     * (640 logical px / 16 logical per char ≈ 40 chars; we leave a
     * small right margin). */
    char title[40];
    int max = 38;
    int len = 0;
    while (e->display_name[len] != '\0' && len < max) {
        title[len] = e->display_name[len];
        len++;
    }
    if (e->display_name[len] != '\0' && len > 3) {
        title[len-3] = '.'; title[len-2] = '.'; title[len-1] = '.';
    }
    title[len] = '\0';

    /* Top band — title + position + display name + family/frames.
     * First line starts at y=24 logical so the title isn't clipped by
     * NTSC overscan at the very top of the framebuffer. */
    pmTextStart(pmPrintfX, 24);
    pmPrintf("       SCENE EXPLORER\n");
    drawSeparator();
    pmPrintf(" %d/%d   %s\n",
             cur + 1, gSceneExplorerCount,
             e->validated ? "* validated" : "? pending");
    pmPrintf(" %s\n", title);
    pmPrintf(" Family: %s   Frames: %u\n",
             e->family, (unsigned)e->frame_count);

    /* Bottom band — pack name, separator, nav hints (X/△/O on one
     * line so the band fits in 4 lines instead of 5 and we stay clear
     * of bottom NTSC overscan). */
    pmTextStart(pmPrintfX, 376);
    pmPrintf(" Pack: %s\n", e->pack);
    drawSeparator();
    pmPrintf(" <- -> scene     L1/R1 family\n");
    pmPrintf(" X play  /\\ loop  O back\n");
}
