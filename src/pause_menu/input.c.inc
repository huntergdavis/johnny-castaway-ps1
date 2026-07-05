static int handleSceneExplorerInput(uint16 pressed)
{
    if (pressed & PAD_LEFT) {
        pauseMenuExplorerCursor--;
        if (pauseMenuExplorerCursor < 0)
            pauseMenuExplorerCursor = gSceneExplorerCount - 1;
    }
    if (pressed & PAD_RIGHT) {
        pauseMenuExplorerCursor++;
        if (pauseMenuExplorerCursor >= gSceneExplorerCount)
            pauseMenuExplorerCursor = 0;
    }
    if (pressed & PAD_L1) {
        int fam = sceneExplorerFamilyForCursor(pauseMenuExplorerCursor);
        /* Jump to the start of the previous family. If we're already at
         * a family boundary, go back one further. */
        if (pauseMenuExplorerCursor == gSceneExplorerFamilyStart[fam] && fam > 0)
            fam--;
        else if (pauseMenuExplorerCursor == gSceneExplorerFamilyStart[fam])
            fam = gSceneExplorerFamilyCount - 1;
        else
            ; /* fam already points at the current family's start */
        pauseMenuExplorerCursor = gSceneExplorerFamilyStart[fam];
    }
    if (pressed & PAD_R1) {
        int fam = sceneExplorerFamilyForCursor(pauseMenuExplorerCursor);
        fam = (fam + 1) % gSceneExplorerFamilyCount;
        pauseMenuExplorerCursor = gSceneExplorerFamilyStart[fam];
    }
    if (pressed & PAD_CROSS) {
        sceneExplorerRememberLaunchName(pauseMenuExplorerCursor);
        pauseMenuRequestPlayScene = pauseMenuExplorerCursor;
        pauseMenuRequestLoopScene = -1;
        sceneExplorerLoadedCursor = -1;
        grFreeSceneExplorerThumbnailBuffer();
        sceneExplorerReleaseStringData();
        return 0;       /* close menu — scene plays on next iteration */
    }
    if (pressed & PAD_TRIANGLE) {
        sceneExplorerRememberLaunchName(pauseMenuExplorerCursor);
        pauseMenuRequestLoopScene = pauseMenuExplorerCursor;
        pauseMenuRequestPlayScene = -1;
        sceneExplorerLoadedCursor = -1;
        grFreeSceneExplorerThumbnailBuffer();
        sceneExplorerReleaseStringData();
        return 0;
    }
    /* 1.0: the SQUARE test-pattern binding (RAM pattern through the
     * thumbnail upload path — the diagnostic that split CD-data vs
     * GPU-upload corruption in one press) is retired from user input.
     * grLoadSceneExplorerTestPattern stays available for debug builds. */
    if (pressed & (PAD_CIRCLE | PAD_START)) {
        menuState = PAUSE_MENU_MAIN;
        menuCursor = MENU_SCENE_EXPLORER;
        prevButtons = 0xFFFF;
        sceneExplorerLoadedCursor = -1;
        grFreeSceneExplorerThumbnailBuffer();
        sceneExplorerReleaseStringData();
        /* Force the main menu to re-draw its dim+panel over whatever
         * thumbnail pixels are sitting in the framebuffer. */
        menuFramebufferPrimed = 0;
    }
    return 1;
}

/* ---------------------------------------------------------------------------
 *  Input handling per sub-screen
 * ------------------------------------------------------------------------- */

/* Handle main menu input.  Returns 0 if user chose Resume. */
static int handleMainInput(uint16 pressed)
{
    if (pressed & PAD_UP) {
        menuCursor--;
        if (menuCursor < 0) menuCursor = MENU_COUNT - 1;
    }
    if (pressed & PAD_DOWN) {
        menuCursor++;
        if (menuCursor >= MENU_COUNT) menuCursor = 0;
    }

    /* Scene Set inline cycling moved into the Scene Set Options
     * sub-screen alongside the Scene Picker policy. Keep pending in
     * sync with the committed value so the sub-screen starts on the
     * current set, not a stale preview. */
    pendingSceneSet = pauseMenuSceneSet;

    /* X = select current item */
    if (pressed & PAD_CROSS) {
        switch (menuCursor) {
        case MENU_RESUME:
            return 0;  /* close menu */

        case MENU_FREEPLAY:
            if (menuFreeplayActive) {
                pauseMenuRequestExitFreeplay = 1;
            } else {
                pauseMenuRequestFreeplay = 1;
            }
            return 0;

        case MENU_FREEPLAY_OPTIONS:
            menuState = PAUSE_MENU_FREEPLAY_OPTIONS;
            prevButtons = 0xFFFF;
            break;

        case MENU_SCENE_SET:
            /* Open the Scene Set Options sub-screen. Cursor lands on
             * the Scene Set row (most-used) by default. */
            menuState             = PAUSE_MENU_SCENE_SET_OPTIONS;
            sceneSetOptionsCursor = SSO_SCENE_SET;
            prevButtons           = 0xFFFF;
            break;

        case MENU_SCENE_EXPLORER:
            menuState = PAUSE_MENU_SCENE_EXPLORER;
            sceneExplorerLoadedCursor = -1;  /* force load on first draw */
            /* Force the settle debounce: the state transition's reprime
             * queues ~600 KB of bgTile LoadImages THIS frame; streaming
             * a thumbnail in the same frame raced that queue. */
            sceneExplorerLastSeenCursor = -1;
            prevButtons = 0xFFFF;
            break;

        case MENU_WORLD:
            menuState = PAUSE_MENU_OPTIONS;
            prevButtons = 0xFFFF;
            break;

        case MENU_ACCESSIBILITY:
            menuState = PAUSE_MENU_ACCESSIBILITY;
            prevButtons = 0xFFFF;
            break;

        case MENU_SYSTEM:
            menuState = PAUSE_MENU_SYSTEM;
            prevButtons = 0xFFFF;
            break;

        default:
            break;
        }
    }

    /* START / Circle close the menu — the sub-screen owns its own
     * commit semantics now (X commits + leaves, Circle backs out). */
    if (pressed & PAD_START)
        return 0;
    if (pressed & PAD_CIRCLE)
        return 0;

    return 1;  /* keep menu open */
}

static int handleFreeplayOptionsInput(uint16 pressed)
{
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_MAIN;
        menuCursor = MENU_FREEPLAY_OPTIONS;
        prevButtons = 0xFFFF;
        return 1;
    }
    if (pressed & PAD_UP) {
        freeplayOptionsCursor--;
        if (freeplayOptionsCursor < 0) freeplayOptionsCursor = FPO_COUNT - 1;
    }
    if (pressed & PAD_DOWN) {
        freeplayOptionsCursor++;
        if (freeplayOptionsCursor >= FPO_COUNT) freeplayOptionsCursor = 0;
    }
    if (pressed & PAD_CROSS) {
        switch (freeplayOptionsCursor) {
        case FPO_GAGS:
            menuState = PAUSE_MENU_FREEPLAY_GAGS;
            prevButtons = 0xFFFF;
            break;
        case FPO_VISITORS:
            menuState = PAUSE_MENU_FREEPLAY_VISITORS;
            prevButtons = 0xFFFF;
            break;
        case FPO_CONTROLS:
            menuState = PAUSE_MENU_CONTROLS;
            prevButtons = 0xFFFF;
            break;
        case FPO_CLEAR:
            if (menuFreeplayActive) {
                pauseMenuRequestFreeplayClear = 1;
                return 0;
            }
            break;
        case FPO_BACK:
            menuState = PAUSE_MENU_MAIN;
            menuCursor = MENU_FREEPLAY_OPTIONS;
            prevButtons = 0xFFFF;
            break;
        }
    }
    return 1;
}

static int handleSceneSetOptionsInput(uint16 pressed)
{
    /* Circle / Start: back out. If a Scene Set preview was pending,
     * silently drop it — Circle = "I changed my mind." */
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        pendingSceneSet      = pauseMenuSceneSet;
        menuState            = PAUSE_MENU_MAIN;
        menuCursor           = MENU_SCENE_SET;
        prevButtons          = 0xFFFF;
        return 1;
    }

    if (pressed & PAD_UP) {
        sceneSetOptionsCursor--;
        if (sceneSetOptionsCursor < 0) sceneSetOptionsCursor = SSO_COUNT - 1;
    }
    if (pressed & PAD_DOWN) {
        sceneSetOptionsCursor++;
        if (sceneSetOptionsCursor >= SSO_COUNT) sceneSetOptionsCursor = 0;
    }

    /* Left/Right cycles the focused row's value. Scene Set uses the
     * same pending-preview pattern as before; Picker commits inline
     * because the policy switch is cheap and reversible. */
    if (sceneSetOptionsCursor == SSO_SCENE_SET) {
        if (pressed & PAD_LEFT) {
            pendingSceneSet--;
            if (pendingSceneSet < 0) pendingSceneSet = NUM_SCENE_SETS - 1;
        }
        if (pressed & PAD_RIGHT) {
            pendingSceneSet++;
            if (pendingSceneSet >= NUM_SCENE_SETS) pendingSceneSet = 0;
        }
    } else if (sceneSetOptionsCursor == SSO_PICKER) {
        int policy = pickerGetPolicy();
        if (pressed & PAD_LEFT) {
            policy = (policy + SCENE_PICKER_COUNT - 1) % SCENE_PICKER_COUNT;
            pickerSetPolicy(policy);
            pauseMenuRequestPickerReset = 1;
        }
        if (pressed & PAD_RIGHT) {
            policy = (policy + 1) % SCENE_PICKER_COUNT;
            pickerSetPolicy(policy);
            pauseMenuRequestPickerReset = 1;
        }
    }

    /* X commits a pending Scene Set and returns to main; on Picker it
     * just returns (policy is already applied); on Back it returns. */
    if (pressed & PAD_CROSS) {
        if (pendingSceneSet != pauseMenuSceneSet) {
            pauseMenuSceneSet             = pendingSceneSet;
            pauseMenuRequestSceneSetCycle = 1;
        }
        if (sceneSetOptionsCursor == SSO_BACK) {
            menuState   = PAUSE_MENU_MAIN;
            menuCursor  = MENU_SCENE_SET;
            prevButtons = 0xFFFF;
            return 1;
        }
        /* SSO_SCENE_SET commit: close the menu so the cycle banner
         * shows immediately. SSO_PICKER X: also close — user picked,
         * we're done. Mirrors the old "X commits Scene Set + closes"
         * UX from the legacy main-menu inline cycler. */
        return 0;
    }

    return 1;
}

static int handleFreeplayGagInput(uint16 pressed)
{
    int count = freeplayGagCount();
    if (count <= 0)
        count = 1;
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_FREEPLAY_OPTIONS;
        freeplayOptionsCursor = FPO_GAGS;
        prevButtons = 0xFFFF;
        return 1;
    }
    if (pressed & (PAD_UP | PAD_LEFT)) {
        freeplayGagCursor--;
        if (freeplayGagCursor < 0) freeplayGagCursor = count - 1;
    }
    if (pressed & (PAD_DOWN | PAD_RIGHT)) {
        freeplayGagCursor++;
        if (freeplayGagCursor >= count) freeplayGagCursor = 0;
    }
    if ((pressed & PAD_CROSS) && menuFreeplayActive) {
        pauseMenuRequestFreeplayGag = freeplayGagCursor;
        return 0;
    }
    return 1;
}

static int handleFreeplayVisitorInput(uint16 pressed)
{
    int count = freeplayVisitorCount();
    if (count <= 0)
        count = 1;
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_FREEPLAY_OPTIONS;
        freeplayOptionsCursor = FPO_GAGS;
        prevButtons = 0xFFFF;
        return 1;
    }
    if (pressed & (PAD_UP | PAD_LEFT)) {
        freeplayVisitorCursor--;
        if (freeplayVisitorCursor < 0) freeplayVisitorCursor = count - 1;
    }
    if (pressed & (PAD_DOWN | PAD_RIGHT)) {
        freeplayVisitorCursor++;
        if (freeplayVisitorCursor >= count) freeplayVisitorCursor = 0;
    }
    if ((pressed & PAD_CROSS) && menuFreeplayActive) {
        pauseMenuRequestFreeplayVisitor = freeplayVisitorCursor;
        return 0;
    }
    return 1;
}

/* Apply a +1 / -1 cycle to the field at `optionsCursor`. */
static void optionsCycle(int dir)
{
    switch (optionsCursor) {
    case WORLD_DAYNIGHT: cycleDaynight(dir); break;
    case WORLD_TIDE:     cycleTide(dir);     break;
    case WORLD_RAFT:     cycleRaft(dir);     break;
    default: break;
    }
}

/* Options sub-screen input. UP/DOWN move cursor; on cycle rows
 * LEFT/RIGHT or X cycle the value; on launcher rows X opens the
 * editor sub-screen. Circle/START goes back to the main menu. */
static int handleOptionsInput(uint16 pressed)
{
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_MAIN;
        menuCursor = MENU_WORLD;   /* return cursor to where we came from */
        prevButtons = 0xFFFF;
        return 1;
    }

    if (pressed & PAD_UP) {
        optionsCursor--;
        if (optionsCursor < 0) optionsCursor = WORLD_COUNT - 1;
    }
    if (pressed & PAD_DOWN) {
        optionsCursor++;
        if (optionsCursor >= WORLD_COUNT) optionsCursor = 0;
    }

    if (pressed & (PAD_RIGHT | PAD_CROSS)) {
        switch (optionsCursor) {
        case WORLD_DAYNIGHT:
        case WORLD_TIDE:
        case WORLD_RAFT:
            optionsCycle(+1);
            if (menuFreeplayActive)
                return 0;
            break;
        case WORLD_HOLIDAYS:
            menuState = PAUSE_MENU_HOLIDAYS;
            prevButtons = 0xFFFF;
            break;
        case WORLD_ISLAND_POS:
            editIslandX     = hostForcedIslandX;
            editIslandY     = hostForcedIslandY;
            editIslandValid = hostForcedIslandPosValid;
            editIslandField = 0;
            menuState   = PAUSE_MENU_ISLAND_POS;
            prevButtons = 0xFFFF;
            break;
        case WORLD_BACK:
            menuState = PAUSE_MENU_MAIN;
            menuCursor = MENU_WORLD;
            prevButtons = 0xFFFF;
            break;
        }
    } else if (pressed & PAD_LEFT) {
        switch (optionsCursor) {
        case WORLD_DAYNIGHT:
        case WORLD_TIDE:
        case WORLD_RAFT:
            optionsCycle(-1);
            if (menuFreeplayActive)
                return 0;
            break;
        default:
            break;
        }
    }

    return 1;
}

static int handleHolidayInput(uint16 pressed)
{
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_OPTIONS;
        optionsCursor = WORLD_HOLIDAYS;
        prevButtons = 0xFFFF;
        return 1;
    }
    if (pressed & PAD_UP) {
        holidayCursor--;
        if (holidayCursor < 0) holidayCursor = HOLIDAY_COUNT - 1;
    }
    if (pressed & PAD_DOWN) {
        holidayCursor++;
        if (holidayCursor >= HOLIDAY_COUNT) holidayCursor = 0;
    }
    if (pressed & (PAD_RIGHT | PAD_CROSS)) {
        if (holidayCursor == HOLIDAY_SET) {
            cycleHolidaySet(+1);
            if (menuFreeplayActive)
                return 0;
        } else if (holidayCursor == HOLIDAY_SELECT) {
            cycleHoliday(+1);
            if (menuFreeplayActive && pauseMenuRequestFreeplayWorldRefresh)
                return 0;
        } else if (holidayCursor == HOLIDAY_BACK) {
            menuState = PAUSE_MENU_OPTIONS;
            optionsCursor = WORLD_HOLIDAYS;
            prevButtons = 0xFFFF;
        }
    } else if (pressed & PAD_LEFT) {
        if (holidayCursor == HOLIDAY_SET) {
            cycleHolidaySet(-1);
            if (menuFreeplayActive)
                return 0;
        } else if (holidayCursor == HOLIDAY_SELECT) {
            cycleHoliday(-1);
            if (menuFreeplayActive && pauseMenuRequestFreeplayWorldRefresh)
                return 0;
        }
    }
    return 1;
}

static int handleAccessibilityInput(uint16 pressed)
{
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_MAIN;
        menuCursor = MENU_ACCESSIBILITY;
        prevButtons = 0xFFFF;
        return 1;
    }
    if (pressed & PAD_UP) {
        accessCursor--;
        if (accessCursor < 0) accessCursor = ACCESS_COUNT - 1;
    }
    if (pressed & PAD_DOWN) {
        accessCursor++;
        if (accessCursor >= ACCESS_COUNT) accessCursor = 0;
    }
    if (pressed & (PAD_LEFT | PAD_RIGHT | PAD_CROSS)) {
        switch (accessCursor) {
        case ACCESS_CAPTIONS:
            cycleCaptions(+1);
            break;
        case ACCESS_SOUND:
            soundMuteToggle();
            pauseMutedSound = 0;
            break;
        case ACCESS_OCEAN: {
            extern int oceanAmbientEnabled;
            extern void oceanAmbientStart(void);
            extern void oceanAmbientStop(void);
            oceanAmbientEnabled = !oceanAmbientEnabled;
            if (oceanAmbientEnabled) oceanAmbientStart();
            else                     oceanAmbientStop();
            break;
        }
        case ACCESS_SOUND_TEST:
            menuState = PAUSE_MENU_SOUND_TEST;
            prevButtons = 0xFFFF;
            break;
        case ACCESS_BACK:
            menuState = PAUSE_MENU_MAIN;
            menuCursor = MENU_ACCESSIBILITY;
            prevButtons = 0xFFFF;
            break;
        }
    }
    return 1;
}

static int handleSoundTestInput(uint16 pressed)
{
    int count = soundEffectCount();
    if (count <= 0)
        count = 1;
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_ACCESSIBILITY;
        accessCursor = ACCESS_SOUND_TEST;
        prevButtons = 0xFFFF;
        return 1;
    }
    if (pressed & (PAD_UP | PAD_LEFT)) {
        soundTestCursor--;
        if (soundTestCursor < 0) soundTestCursor = count - 1;
    }
    if (pressed & (PAD_DOWN | PAD_RIGHT)) {
        soundTestCursor++;
        if (soundTestCursor >= count) soundTestCursor = 0;
    }
    if (pressed & PAD_CROSS)
        soundPlay(soundTestCursor);
    return 1;
}

static int handleSystemInput(uint16 pressed)
{
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_MAIN;
        menuCursor = MENU_SYSTEM;
        prevButtons = 0xFFFF;
        return 1;
    }
    if (pressed & PAD_UP) {
        systemCursor--;
        if (systemCursor < 0) systemCursor = SYSTEM_COUNT - 1;
    }
    if (pressed & PAD_DOWN) {
        systemCursor++;
        if (systemCursor >= SYSTEM_COUNT) systemCursor = 0;
    }
    if (pressed & PAD_RIGHT) {
        if (systemCursor == SYSTEM_PERF)
            cyclePerf(+1);
        else if (systemCursor == SYSTEM_STORY_DAY) {
            storyCurrentDay++;
            if (storyCurrentDay > 11) storyCurrentDay = 1;
            if (storyCurrentDay < 1)  storyCurrentDay = 1;
        }
    } else if (pressed & PAD_LEFT) {
        if (systemCursor == SYSTEM_PERF)
            cyclePerf(-1);
        else if (systemCursor == SYSTEM_STORY_DAY) {
            storyCurrentDay--;
            if (storyCurrentDay < 1) storyCurrentDay = 11;
        }
    } else if (pressed & PAD_CROSS) {
        switch (systemCursor) {
        case SYSTEM_SAVE:
            memcardSaveSettings();
            break;
        case SYSTEM_SET_TIME:
            editMonth  = ps1SoftMonth;
            editDay    = ps1SoftDay;
            editYear   = ps1SoftYear;
            editHour   = ps1SoftHour;
            editMinute = ps1SoftMinute;
            editField  = 0;
            menuState   = PAUSE_MENU_SET_TIME;
            prevButtons = 0xFFFF;
            break;
        case SYSTEM_SET_SEED:
            editSeedValue = ps1LastSeedKnown ? ps1LastSeedApplied : 1u;
            editSeedFixed = 0;
            editSeedField = 0;
            menuState   = PAUSE_MENU_SET_SEED;
            prevButtons = 0xFFFF;
            break;
        case SYSTEM_PERF:
            cyclePerf(+1);
            break;
        case SYSTEM_RESET_LOOP:
            pauseMenuRequestResetLoop = 1;
            return 0;
        case SYSTEM_NEXT_SCENE:
            pauseMenuRequestNextScene = 1;
            return 0;
        case SYSTEM_SCENE_INFO:
            menuState   = PAUSE_MENU_SCENE_INFO;
            prevButtons = 0xFFFF;
            break;
        case SYSTEM_CREDITS:
            menuState   = PAUSE_MENU_CREDITS;
            prevButtons = 0xFFFF;
            break;
        }
    }
    return 1;
}

/* Returns 0 if user goes back to main. */
static int handleSubInput(uint16 pressed)
{
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        enum PauseMenuState fromState = menuState;
        if (fromState == PAUSE_MENU_CONTROLS) {
            menuState = PAUSE_MENU_FREEPLAY_OPTIONS;
            freeplayOptionsCursor = FPO_GAGS;
            prevButtons = 0xFFFF;
            return 1;
        }
        /* Scene Info and Credits are reached from the System menu — return
         * there with the cursor on the row that opened them. */
        if (fromState == PAUSE_MENU_SCENE_INFO || fromState == PAUSE_MENU_CREDITS) {
            menuState = PAUSE_MENU_SYSTEM;
            systemCursor = (fromState == PAUSE_MENU_CREDITS) ? SYSTEM_CREDITS
                                                            : SYSTEM_SCENE_INFO;
            prevButtons = 0xFFFF;
            return 1;
        }
        menuState = PAUSE_MENU_MAIN;
        menuCursor = MENU_RESUME;
        prevButtons = 0xFFFF;
        return 1;
    }
    return 1;
}

/* Set Time input -- LEFT/RIGHT adjust value, UP/DOWN change field. */
static int handleSetTimeInput(uint16 pressed)
{
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_SYSTEM;
        systemCursor = SYSTEM_SET_TIME;
        prevButtons = 0xFFFF;
        return 1;
    }

    if (pressed & PAD_UP) {
        editField--;
        if (editField < 0) editField = 4;
    }
    if (pressed & PAD_DOWN) {
        editField++;
        if (editField > 4) editField = 0;
    }

    int delta = 0;
    if (pressed & PAD_RIGHT) delta = 1;
    if (pressed & PAD_LEFT)  delta = -1;

    if (delta) {
        switch (editField) {
        case 0: editMonth  = clampInt(editMonth + delta, 1, 12); break;
        case 1: editDay    = clampInt(editDay + delta, 1, daysInMonth(editMonth, editYear)); break;
        case 2: editYear   = clampInt(editYear + delta, 1990, 2099); break;
        case 3: editHour   = clampInt(editHour + delta, 0, 23); break;
        case 4: editMinute = clampInt(editMinute + delta, 0, 59); break;
        }
    }

    /* X = confirm -- write values into the PS1 time stubs.
     * The game reads time via getHour()/getMonthAndDay()/getDayOfYear() in
     * utils.c.  We update those stubs through extern globals exposed
     * specifically for the pause menu. */
    if (pressed & PAD_CROSS) {
        extern int ps1SoftHour;
        extern int ps1SoftMinute;
        extern int ps1SoftMonth;
        extern int ps1SoftDay;
        extern int ps1SoftYear;
        extern int ps1SoftTimeEnabled;
        ps1SoftTimeEnabled = 1;  /* future scene picks honor user-set date */

        ps1SoftHour  = editHour;
        ps1SoftMinute = editMinute;
        ps1SoftMonth = editMonth;
        ps1SoftDay   = editDay;
        ps1SoftYear  = editYear;
        if (hostHolidayMode != HOLIDAY_MODE_AUTO_ORIGINAL4 &&
            hostHolidayMode != HOLIDAY_MODE_AUTO_ALL)
            hostHolidayMode = HOLIDAY_MODE_AUTO_ORIGINAL4;
        hostForcedHoliday = 0;  /* date picker drives holiday while in AUTO */
        /* getDayOfYear() in utils.c computes from ps1SoftMonth/ps1SoftDay */

        /* Return to System after confirming. */
        menuState = PAUSE_MENU_SYSTEM;
        systemCursor = SYSTEM_SET_TIME;
        prevButtons = 0xFFFF;
        if (menuFreeplayActive) {
            pauseMenuRequestFreeplayWorldRefresh = 1;
            return 0;
        }
    }

    return 1;
}

/* Set Island Pos input. UP/DOWN selects field; LEFT/RIGHT adjusts;
 * X confirms (write through to host overrides) and goes back; Circle/START
 * goes back without applying changes. */
static int handleIslandPosInput(uint16 pressed)
{
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_OPTIONS;
        optionsCursor = WORLD_ISLAND_POS;
        prevButtons = 0xFFFF;
        return 1;
    }

    if (pressed & PAD_UP) {
        editIslandField--;
        if (editIslandField < 0) editIslandField = 2;
    }
    if (pressed & PAD_DOWN) {
        editIslandField++;
        if (editIslandField > 2) editIslandField = 0;
    }

    int delta = 0;
    if (pressed & PAD_RIGHT) delta = +1;
    if (pressed & PAD_LEFT)  delta = -1;
    if (delta) {
        switch (editIslandField) {
        case 0:
            editIslandX = clampInt(editIslandX + delta * 4, -250, 250);
            /* Editing X implies the user wants MANUAL mode. */
            editIslandValid = 1;
            break;
        case 1:
            editIslandY = clampInt(editIslandY + delta * 4, -100, 200);
            editIslandValid = 1;
            break;
        case 2:
            editIslandValid = !editIslandValid;
            break;
        }
    }

    if (pressed & PAD_CROSS) {
        hostForcedIslandPosValid = editIslandValid;
        hostForcedIslandX        = editIslandX;
        hostForcedIslandY        = editIslandY;
        menuState  = PAUSE_MENU_OPTIONS;
        optionsCursor = WORLD_ISLAND_POS;
        prevButtons = 0xFFFF;
        if (menuFreeplayActive) {
            pauseMenuRequestFreeplayWorldRefresh = 1;
            return 0;
        }
    }

    return 1;
}

/* Set RNG Seed input. */
static int handleSetSeedInput(uint16 pressed)
{
    if (pressed & (PAD_START | PAD_CIRCLE)) {
        menuState = PAUSE_MENU_SYSTEM;
        systemCursor = SYSTEM_SET_SEED;
        prevButtons = 0xFFFF;
        return 1;
    }

    if (pressed & PAD_UP) {
        editSeedField--;
        if (editSeedField < 0) editSeedField = 1;
    }
    if (pressed & PAD_DOWN) {
        editSeedField++;
        if (editSeedField > 1) editSeedField = 0;
    }

    /* Determine step: +/-1, +/-100, or +/-10000 depending on
     * which shoulder button is held alongside LEFT/RIGHT.
     * (LEFT/RIGHT alone = 1; L1/R1 alone = 100; L2/R2 alone = 10000.) */
    int step = 0;
    int dir  = 0;
    if (pressed & PAD_RIGHT)   { dir = +1; step = 1; }
    if (pressed & PAD_LEFT)    { dir = -1; step = 1; }
    if (pressed & PAD_R1)      { dir = +1; step = 100; }
    if (pressed & PAD_L1)      { dir = -1; step = 100; }
    if (pressed & PAD_R2)      { dir = +1; step = 10000; }
    if (pressed & PAD_L2)      { dir = -1; step = 10000; }

    if (dir != 0) {
        if (editSeedField == 0) {
            editSeedFixed = !editSeedFixed;
        } else {
            unsigned int delta = (unsigned int)step;
            if (dir > 0) editSeedValue += delta;
            else         editSeedValue -= delta;
        }
    }

    if (pressed & PAD_CROSS) {
        if (editSeedFixed) {
            ps1SetSeed(editSeedValue);
        } else {
            ps1SeedRandom();   /* re-seed via root counters */
        }
        menuState   = PAUSE_MENU_SYSTEM;
        systemCursor = SYSTEM_SET_SEED;
        prevButtons = 0xFFFF;
    }

    return 1;
}
