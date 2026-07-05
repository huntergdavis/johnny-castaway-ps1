void pauseMenuEnsureFontUploaded(void)
{
    if (!pmFontUploaded)
        pmUploadFont();
}

void pauseMenuUploadDebugFont(void)
{
    pmFontUploaded = 0;
    pmUploadFont();
}

void pauseMenuInit(void)
{
    if (fontID < 0)
        fontID = 0;
#if PAUSE_MENU_DIAG_LOGS
    printf("JCPAUSE pauseMenuInit fontID=%d\n", fontID);
#endif

    menuVisible = 0;
    menuCursor  = 0;
    menuState   = PAUSE_MENU_MAIN;
    menuFramebufferPrimed = 0;
}

void pauseMenuSetFreeplayActive(int active)
{
    menuFreeplayActive = active ? 1 : 0;
}

void pauseMenuShow(void)
{
    pmMenuTextEnsure();

    /* Re-upload the font + CLUT on EVERY open. The one-shot boot upload
     * left the glyph texture at VRAM (640,256) exposed to any scene
     * sprite sheet that lands in the same texture region — on hardware
     * with the full random scene mix this showed as menu panels with no
     * text. The upload is ~12 KB (sub-millisecond); re-stamping per
     * open makes the menu immune to whoever stomped the area. */
    pmFontUploaded = 0;
    pmUploadFont();

    {
        /* New menu visit: give thumbnails another chance (the drive may
         * have recovered since the breaker tripped). */
        extern void sceneExplorerResetThumbBreaker(void);
        sceneExplorerResetThumbBreaker();
    }
    menuVisible           = 1;
    menuCursor            = 0;
    optionsCursor         = 0;
    holidayCursor         = 0;
    accessCursor          = 0;
    systemCursor          = 0;
    pendingSceneSet       = pauseMenuSceneSet;  /* fresh preview each open */
    menuState             = PAUSE_MENU_MAIN;
    menuFramebufferPrimed = 0;
    prevButtons           = 0xFFFF;  /* Treat all buttons as "held" so the
                                        initial press that opened the menu
                                        is not re-acted. */

    /* Sound stays in whatever state the user left it — defaults ON.
     * The Sound menu item lets the user mute/unmute manually. (When
     * memory-card support lands, we'll persist the user's choice.) */
    pauseMutedSound = 0;

#if PAUSE_MENU_DIAG_LOGS
    /* One-shot JCPAUSE snapshot for log-mining. */
    printf("JCPAUSE show frame=%lu scene=%s mode=%s perfLevel=%u soundMuted=%d\n",
           (unsigned long)ps1FrameCount,
           foregroundPilotRuntimeSceneName(),
           foregroundPilotRuntimeModeName(),
           (unsigned)ps1PerfLevel,
           soundMuted);
#endif
#if PS1_VERBOSE_DIAGNOSTICS
    if (ps1PadScriptVerboseLogEnabled()) {
        printf("JCPAUSE show state=%d cursor=%d\n",
               (int)menuState, menuCursor);
    }
#endif

    /* Refresh the heap-free cache once per pause-show. fgProbeLargestAlloc
     * does ~9 malloc/free cycles which fragments the heap if called every
     * pause-loop iteration; caching avoids that. */
    pmCachedHeapKB = fgProbeLargestAlloc() / 1024UL;
}

void pauseMenuHide(void)
{
#if PS1_VERBOSE_DIAGNOSTICS
    if (ps1PadScriptVerboseLogEnabled()) {
        printf("JCPAUSE hide state=%d cursor=%d\n",
               (int)menuState, menuCursor);
    }
#endif
    menuVisible = 0;
    menuFramebufferPrimed = 0;
    /* Sound stays in user's chosen state — no auto-restore. */
    pmMenuTextRelease();
}

int pauseMenuIsVisible(void)
{
    return menuVisible;
}

enum PauseMenuState pauseMenuGetState(void)
{
    return menuState;
}

void pauseMenuSetState(enum PauseMenuState state)
{
    menuState = state;
    if (state == PAUSE_MENU_MAIN)
        menuCursor = 0;
    prevButtons = 0xFFFF;  /* suppress stale press */
}
