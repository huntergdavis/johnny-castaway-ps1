/* ---------------------------------------------------------------------------
 *  pauseMenuUpdate -- one frame of the overlay
 *
 *  Called in a tight VSync loop while paused.  Reads pad, updates state,
 *  draws text, uploads background.  Returns 1 to stay paused, 0 to resume.
 * ------------------------------------------------------------------------- */
int pauseMenuUpdate(void)
{
    int drawDim;

    if (!menuVisible) return 0;

    /* Match the scene renderer: wait first, then touch the single
     * framebuffer. Waiting at the end caused the next pause iteration to
     * erase the completed menu immediately after VBlank, making the display
     * alternate between raw scene and menu. */
    VSync(0);

    drawDim = !menuFramebufferPrimed;

    if (!menuFramebufferPrimed) {
        /* Restore the raw scene once, then draw the semi-transparent dim
         * once. Re-uploading the raw scene every pause frame fights the
         * single framebuffer and produces visible scene/menu flicker. */
        grForceFullRedrawNextFrame();
        grDrawBackground();
        DrawSync(0);
    }

    /* Upload our font on first frame. */
    if (!pmFontUploaded)
        pmUploadFont();

    /* Build OT, slots high-to-low = drawn first-to-last:
     *   N-1: TPAGE (font tpage with abr=0 for 50% blend)
     *   N-2: dim quad (semi-trans full-screen black)
     *   N-3: panel quads (3, with corner cutouts for rounded look)
     *   N-4: text SPRTs
     */
    ClearOTagR(pauseOt, PAUSE_OT_LEN);
    uint8 *next = pausePrimBuf;

    DR_TPAGE *tp = (DR_TPAGE*)next;
    next += sizeof(DR_TPAGE);
    setDrawTPage(tp, 0, 1, getTPage(0, 0, PM_FONT_VRAM_X, PM_FONT_VRAM_Y));
    ps1GpuOtAddPrim(&pauseOt[PAUSE_OT_LEN - 1], tp);

    /* Panel-quad build is intentionally deferred until AFTER the input
     * handler runs (below) — it depends on the menu state the input
     * handler may transition to. Otherwise the first-frame transition
     * into Scene Explorer (or any sub-screen) would still build panels
     * for the OUTGOING state and cover the new screen's content. */

    /* Read pad through the game's shared pad buffer (events_ps1.c owns
     * InitPAD, so we just peek at its buffer via the extern). */
    extern uint8 pad_buff[2][34];
    uint16 cur = 0;
    for (int i = 0; i < 2; i++) {
        PADTYPE *pad = (PADTYPE *)pad_buff[i];
        /* Don't gate on pad->stat — DualShock analog reports non-zero
         * stat values in some modes and pad->btn is still valid. */
        cur |= ps1PadButtonsWithAnalog(pad);
    }
    uint16 pressed = pmNewPress(cur);
    prevButtons = cur;

    /* Dispatch to current sub-screen input handler. */
    enum PauseMenuState oldState = menuState;
    int oldMenuCursor = menuCursor;
    int oldOptionsCursor = optionsCursor;
    int oldFreeplayOptionsCursor = freeplayOptionsCursor;
    int oldAccessCursor = accessCursor;
    int oldSystemCursor = systemCursor;
    int keepOpen = 1;
    switch (menuState) {
    case PAUSE_MENU_MAIN:
        keepOpen = handleMainInput(pressed);
        break;
    case PAUSE_MENU_FREEPLAY_OPTIONS:
        keepOpen = handleFreeplayOptionsInput(pressed);
        break;
    case PAUSE_MENU_FREEPLAY_GAGS:
        keepOpen = handleFreeplayGagInput(pressed);
        break;
    case PAUSE_MENU_FREEPLAY_VISITORS:
        keepOpen = handleFreeplayVisitorInput(pressed);
        break;
    case PAUSE_MENU_OPTIONS:
        keepOpen = handleOptionsInput(pressed);
        break;
    case PAUSE_MENU_HOLIDAYS:
        keepOpen = handleHolidayInput(pressed);
        break;
    case PAUSE_MENU_ACCESSIBILITY:
        keepOpen = handleAccessibilityInput(pressed);
        break;
    case PAUSE_MENU_SOUND_TEST:
        keepOpen = handleSoundTestInput(pressed);
        break;
    case PAUSE_MENU_SYSTEM:
        keepOpen = handleSystemInput(pressed);
        break;
    case PAUSE_MENU_SET_TIME:
        keepOpen = handleSetTimeInput(pressed);
        break;
    case PAUSE_MENU_ISLAND_POS:
        keepOpen = handleIslandPosInput(pressed);
        break;
    case PAUSE_MENU_SET_SEED:
        keepOpen = handleSetSeedInput(pressed);
        break;
    case PAUSE_MENU_SCENE_INFO:
    case PAUSE_MENU_CONTROLS:
    case PAUSE_MENU_CREDITS:
        keepOpen = handleSubInput(pressed);
        break;
    case PAUSE_MENU_SCENE_EXPLORER:
        keepOpen = handleSceneExplorerInput(pressed);
        break;
    case PAUSE_MENU_SCENE_SET_OPTIONS:
        keepOpen = handleSceneSetOptionsInput(pressed);
        break;
    }

    /* If the state transitioned this frame (e.g. out of Scene Explorer
     * back to main menu), the framebuffer is full of the OUTGOING
     * screen's pixels. Re-prime now: re-upload the paused scene's bg
     * tiles, then let the panel/text draws below paint chrome on top.
     * Without this the main menu would draw over thumbnail+bands and
     * leave bits of them peeking out around the panel edges. */
    if (oldState != menuState && keepOpen) {
        grForceFullRedrawNextFrame();
        grDrawBackground();
        DrawSync(0);
        drawDim = 1;          /* re-dim on transition */
        menuFramebufferPrimed = 0;
    }

    if (ps1PadScriptVerboseLogEnabled() &&
        (pressed ||
         oldState != menuState ||
         oldMenuCursor != menuCursor ||
         oldOptionsCursor != optionsCursor ||
         oldFreeplayOptionsCursor != freeplayOptionsCursor ||
         oldAccessCursor != accessCursor ||
         oldSystemCursor != systemCursor ||
         !keepOpen)) {
        printf("JCPAUSE input state=%d->%d main=%d->%d opt=%d->%d fp=%d->%d acc=%d->%d sys=%d->%d cur=%04x pressed=%04x keep=%d\n",
               (int)oldState, (int)menuState,
               oldMenuCursor, menuCursor,
               oldOptionsCursor, optionsCursor,
               oldFreeplayOptionsCursor, freeplayOptionsCursor,
               oldAccessCursor, accessCursor,
               oldSystemCursor, systemCursor,
               (unsigned)cur,
               (unsigned)pressed,
               keepOpen);
    }

    if (!keepOpen) {
        pauseMenuHide();
        /* Force a full bgTile re-upload on the first scene frame after
         * resume — our dim/panel quads painted into VRAM and we need
         * the scene to fully repaint over them. prevDirty survives
         * grRestoreBgTiles' currDirty reset; setting both is the
         * standard pattern (see grFadeOut/grFreeCleanBgTiles). */
        grForceFullRedrawNextFrame();
        return 0;
    }

    /* Build panel quads NOW (post-input-handler) so the menu state we
     * transition into this frame controls the panel layout, not the
     * outgoing state. */
    if (menuState == PAUSE_MENU_SCENE_EXPLORER) {
        /* Scene Explorer chrome — two opaque dark-purple bands sitting
         * ABOVE and BELOW the centered 320x240 thumbnail (which lives at
         * fb (160, 120, 320, 240) per grLoadSceneExplorerThumbnail).
         * The thumbnail rect is left untouched.
         *
         *      +------+------+------+
         *      |    top band         |   y=0..120 logical, 5 lines
         *      +------+------+------+
         *      | left | thumb| rite |   y=120..360, image
         *      +------+------+------+
         *      |   bottom band       |   y=360..480, 5 lines
         *      +------+------+------+
         */
        POLY_F4 *pTop = (POLY_F4*)next;
        next += sizeof(POLY_F4);
        setPolyF4(pTop);
        setRGB0(pTop, 0x10, 0x08, 0x20);
        setXY4(pTop,   0,   0, 640,   0,
                       0, 120, 640, 120);
        ps1GpuOtAddPrim(&pauseOt[PAUSE_OT_LEN - 3], pTop);

        POLY_F4 *pBot = (POLY_F4*)next;
        next += sizeof(POLY_F4);
        setPolyF4(pBot);
        setRGB0(pBot, 0x10, 0x08, 0x20);
        setXY4(pBot,   0, 360, 640, 360,
                       0, 480, 640, 480);
        ps1GpuOtAddPrim(&pauseOt[PAUSE_OT_LEN - 3], pBot);

        pmFramePrimNext = next;
        pmFrameOtSlot   = &pauseOt[PAUSE_OT_LEN - 4];
        pmPrintfX       = 8;
        pmTextStart(pmPrintfX, 8);   /* first line lands in top band */
    } else {
        pmBuildPanelQuads(&next,
                          drawDim ? &pauseOt[PAUSE_OT_LEN - 2] : NULL,
                          &pauseOt[PAUSE_OT_LEN - 3]);

        pmFramePrimNext = next;
        pmFrameOtSlot   = &pauseOt[PAUSE_OT_LEN - 4];
        pmPrintfX       = PM_PANEL_X0 + 24;
        pmTextStart(pmPrintfX, PM_PANEL_Y0 + 24);
    }

    /* Draw the appropriate screen — pmPrintf adds SPRT primitives to
     * pauseOt via the pmFrame* globals. */
    switch (menuState) {
    case PAUSE_MENU_MAIN:       drawMainMenu();  break;
    case PAUSE_MENU_FREEPLAY_OPTIONS: drawFreeplayOptions(); break;
    case PAUSE_MENU_FREEPLAY_GAGS: drawFreeplayGagCatalog(); break;
    case PAUSE_MENU_FREEPLAY_VISITORS: drawFreeplayVisitorCatalog(); break;
    case PAUSE_MENU_OPTIONS:    drawOptions();   break;
    case PAUSE_MENU_HOLIDAYS:   drawHolidayMenu(); break;
    case PAUSE_MENU_ACCESSIBILITY: drawAccessibilityMenu(); break;
    case PAUSE_MENU_SOUND_TEST: drawSoundTestMenu(); break;
    case PAUSE_MENU_SYSTEM:     drawSystemMenu(); break;
    case PAUSE_MENU_SCENE_INFO: drawSceneInfo(); break;
    case PAUSE_MENU_CONTROLS:   drawControls();  break;
    case PAUSE_MENU_SET_TIME:   drawSetTime();   break;
    case PAUSE_MENU_ISLAND_POS: drawIslandPos(); break;
    case PAUSE_MENU_SET_SEED:   drawSetSeed();   break;
    case PAUSE_MENU_CREDITS:    drawCredits();   break;
    case PAUSE_MENU_SCENE_EXPLORER: drawSceneExplorer(); break;
    case PAUSE_MENU_SCENE_SET_OPTIONS: drawSceneSetOptions(); break;
    }

    /* Submit the OT to GPU. */
    DrawOTag(&pauseOt[PAUSE_OT_LEN - 1]);
    DrawSync(0);
    menuFramebufferPrimed = 1;

#if PAUSE_MENU_DIAG_LOGS
    /* JCPAUSE per-frame diag. */
    {
        static uint32 pauseFrameDbg = 0;
        if ((pauseFrameDbg++ % 60) == 0) {
            printf("JCPAUSE flush #%lu fontID=%d state=%d cursor=%d\n",
                   (unsigned long)pauseFrameDbg, fontID, (int)menuState, menuCursor);
        }
    }
#endif

    return 1;
}
