/* ---------------------------------------------------------------------------
 *  Sub-screen: Controls
 * ------------------------------------------------------------------------- */
static void drawControls(void)
{
    pmPrintf(pmText(PMT_CONTROLS_TITLE));
    drawSeparator();
    pmPrintf(pmText(PMT_CONTROLS_WALK));
    pmPrintf(pmText(PMT_CONTROLS_SPEED));
    pmPrintf(pmText(PMT_CONTROLS_FISH));
    pmPrintf(pmText(PMT_CONTROLS_DAYNIGHT));
    pmPrintf(pmText(PMT_CONTROLS_TIDE));
    pmPrintf(pmText(PMT_CONTROLS_RAFT));
    pmPrintf(pmText(PMT_CONTROLS_HOLIDAY));
    pmPrintf(pmText(PMT_CONTROLS_PAUSE));
    drawSeparator();
    pmPrintf(pmText(PMT_O_START_BACK));
}

static void drawFreeplayOptions(void)
{
    pmPrintf(pmText(PMT_FREEPLAY_OPTIONS_TITLE));
    drawSeparator();
    pmPrintf(pmText(PMT_FPO_GAGS),
             freeplayOptionsCursor == FPO_GAGS ? ">" : " ");
    pmPrintf(pmText(PMT_FPO_CLEAR),
             freeplayOptionsCursor == FPO_CLEAR ? ">" : " ");
    pmPrintf(pmText(PMT_MENU_BACK),
             freeplayOptionsCursor == FPO_BACK ? ">" : " ");
    drawSeparator();
    if (!menuFreeplayActive)
        pmPrintf(pmText(PMT_START_FREEPLAY_TO_SPAWN));
    pmPrintf(pmText(PMT_HINT_SELECT_BACK));
}

/* ---------------------------------------------------------------------------
 *  Scene Set Options sub-screen — pool selector + picker policy in one
 *  page. Up/Down navigates rows, Left/Right cycles values inline,
 *  Cross commits any pending Scene Set change and backs out, Circle
 *  backs out without committing.
 * ------------------------------------------------------------------------- */
static void drawSceneSetOptions(void)
{
    pmPrintf(pmText(PMT_SCENE_SET_TITLE));
    drawSeparator();

    /* Scene Set row: shows pending value with arrows when focused. */
    {
        int showIdx = (sceneSetOptionsCursor == SSO_SCENE_SET)
                          ? pendingSceneSet : pauseMenuSceneSet;
        int dirty   = (pendingSceneSet != pauseMenuSceneSet);
        if (showIdx < 0 || showIdx >= NUM_SCENE_SETS) showIdx = 0;
        pmPrintf(pmText(PMT_SCENE_SET),
                 sceneSetOptionsCursor == SSO_SCENE_SET ? ">" : " ",
                 sceneSetOptionsCursor == SSO_SCENE_SET ? "<" : " ",
                 kSceneSetNames[showIdx],
                 sceneSetOptionsCursor == SSO_SCENE_SET ? ">" : " ",
                 dirty ? "*" : "");
    }

    /* Picker policy row: cycles immediately on Left/Right (no commit
     * step — the policy switch is cheap and reversible). */
    {
        int policy = pickerGetPolicy();
        pmPrintf(pmText(PMT_SCENE_PICKER),
                 sceneSetOptionsCursor == SSO_PICKER ? ">" : " ",
                 sceneSetOptionsCursor == SSO_PICKER ? "<" : " ",
                 pickerPolicyName(policy),
                 sceneSetOptionsCursor == SSO_PICKER ? ">" : " ");
    }

    pmPrintf(pmText(PMT_MENU_BACK),
             sceneSetOptionsCursor == SSO_BACK ? ">" : " ");

    drawSeparator();
    pmPrintf(pmText(PMT_L_R_CYCLE_COMMIT));
    pmPrintf(pmText(PMT_O_START_BACK));
}

static void drawFreeplayGagCatalog(void)
{
    int count = freeplayGagCount();
    if (count <= 0)
        count = 1;
    if (freeplayGagCursor >= count)
        freeplayGagCursor = 0;
    if (freeplayGagCursor < 0)
        freeplayGagCursor = count - 1;

    pmPrintf(pmText(PMT_FREEPLAY_GAG_TITLE));
    drawSeparator();
    pmPrintf(pmText(PMT_FREEPLAY_ENTRY),
             freeplayGagCursor + 1, count,
             pmFreeplayGagTitle(freeplayGagCursor));
    pmPrintf(pmText(PMT_BMP_LINE), pmFreeplayGagBmp(freeplayGagCursor));
    pmPrintf(pmText(PMT_FRAMES_RAM),
             freeplayGagFrames(freeplayGagCursor),
             freeplayGagMemoryKB(freeplayGagCursor));
    pmPrintf(pmText(PMT_DESC_LINE), pmFreeplayGagDescription(freeplayGagCursor));
    drawSeparator();
    pmPrintf(pmText(PMT_UP_DOWN_CHOOSE));
    pmPrintf(menuFreeplayActive ? pmText(PMT_X_SPAWN_NOW) : pmText(PMT_START_FREEPLAY_FIRST));
    pmPrintf(pmText(PMT_O_START_BACK));
}

static void drawFreeplayVisitorCatalog(void)
{
    int count = freeplayVisitorCount();
    if (count <= 0)
        count = 1;
    if (freeplayVisitorCursor >= count)
        freeplayVisitorCursor = 0;
    if (freeplayVisitorCursor < 0)
        freeplayVisitorCursor = count - 1;

    pmPrintf(pmText(PMT_FREEPLAY_VISITOR_TITLE));
    drawSeparator();
    pmPrintf(pmText(PMT_FREEPLAY_ENTRY),
             freeplayVisitorCursor + 1, count,
             pmFreeplayVisitorTitle(freeplayVisitorCursor));
    pmPrintf(pmText(PMT_BMP_LINE), pmFreeplayVisitorBmp(freeplayVisitorCursor));
    pmPrintf(pmText(PMT_FRAMES_RAM),
             freeplayVisitorFrames(freeplayVisitorCursor),
             freeplayVisitorMemoryKB(freeplayVisitorCursor));
    pmPrintf(pmText(PMT_DESC_LINE), pmFreeplayVisitorDescription(freeplayVisitorCursor));
    drawSeparator();
    pmPrintf(pmText(PMT_UP_DOWN_CHOOSE));
    pmPrintf(menuFreeplayActive ? pmText(PMT_X_SPAWN_NOW) : pmText(PMT_START_FREEPLAY_FIRST));
    pmPrintf(pmText(PMT_O_START_BACK));
}

/* ---------------------------------------------------------------------------
 *  Sub-screen: Set Time/Date
 * ------------------------------------------------------------------------- */
static void drawSetTime(void)
{
    pmPrintf("\n");
    drawSeparator();
    pmPrintf(pmText(PMT_SET_TIME_TITLE));
    drawSeparator();
    pmPrintf("\n");

    /* Month */
    pmPrintf(pmText(PMT_SET_TIME_MONTH),
             editField == 0 ? ">" : " ",
             editField == 0 ? "[" : " ",
             editMonth,
             editField == 0 ? "]" : " ");

    /* Day */
    pmPrintf(pmText(PMT_SET_TIME_DAY),
             editField == 1 ? ">" : " ",
             editField == 1 ? "[" : " ",
             editDay,
             editField == 1 ? "]" : " ");

    /* Year */
    pmPrintf(pmText(PMT_SET_TIME_YEAR),
             editField == 2 ? ">" : " ",
             editField == 2 ? "[" : " ",
             editYear,
             editField == 2 ? "]" : " ");

    /* Hour */
    pmPrintf(pmText(PMT_SET_TIME_HOUR),
             editField == 3 ? ">" : " ",
             editField == 3 ? "[" : " ",
             editHour,
             editField == 3 ? "]" : " ");

    /* Minute */
    pmPrintf(pmText(PMT_SET_TIME_MIN),
             editField == 4 ? ">" : " ",
             editField == 4 ? "[" : " ",
             editMinute,
             editField == 4 ? "]" : " ");

    {
        int dateHoliday = holidayForDate(editYear, editMonth, editDay);
        pmPrintf(pmText(PMT_SET_TIME_HOLIDAY),
                 dateHoliday ? pmHolidayShortName(dateHoliday) : pmText(PMT_NONE));
        if (dateHoliday)
            pmPrintf(pmText(PMT_HOLIDAY_TITLE_LINE), pmHolidayTitle(dateHoliday));
    }

    pmPrintf("\n");
    pmPrintf(pmText(PMT_UP_DOWN_SELECT_FIELD));
    pmPrintf(pmText(PMT_LEFT_RIGHT_ADJUST_VALUE));
    pmPrintf(pmText(PMT_X_CONFIRM_BACK));
}

/* ---------------------------------------------------------------------------
 *  Sub-screen: Set Island Pos
 * ---------------------------------------------------------------------------
 *  Three fields: X offset, Y offset, and a Mode toggle (AUTO vs MANUAL).
 *  AUTO = let the runtime pick (random varpos for fishing scenes,
 *         (0, 0) otherwise).
 *  MANUAL = use the X/Y values from this screen on every scene.
 *  Range guards: X clamped to -250..+250, Y to -100..+200. Real
 *  observed varpos from fgLoopRandomVarPos lives well inside that.
 *  X confirms back to main; Circle/START goes back without applying.
 * ------------------------------------------------------------------------- */
static void drawIslandPos(void)
{
    pmPrintf("\n");
    drawSeparator();
    pmPrintf(pmText(PMT_ISLAND_TITLE));
    drawSeparator();
    pmPrintf("\n");

    pmPrintf(pmText(PMT_ISLAND_X),
             editIslandField == 0 ? ">" : " ",
             editIslandField == 0 ? "[" : " ",
             editIslandX,
             editIslandField == 0 ? "]" : " ");
    pmPrintf(pmText(PMT_ISLAND_Y),
             editIslandField == 1 ? ">" : " ",
             editIslandField == 1 ? "[" : " ",
             editIslandY,
             editIslandField == 1 ? "]" : " ");
    pmPrintf(pmText(PMT_ISLAND_MODE),
             editIslandField == 2 ? ">" : " ",
             editIslandField == 2 ? "[" : " ",
             editIslandValid ? pmText(PMT_MANUAL) : pmText(PMT_AUTO),
             editIslandField == 2 ? "]" : " ");

    pmPrintf("\n");
    pmPrintf(pmText(PMT_UP_DOWN_SELECT_FIELD));
    pmPrintf(pmText(PMT_LEFT_RIGHT_ADJUST_XY));
    pmPrintf(pmText(PMT_TOGGLE_MODE));
    pmPrintf(pmText(PMT_X_CONFIRM_BACK_EQ));

    if (!editIslandValid && (editIslandX != 0 || editIslandY != 0)) {
        pmPrintf("\n");
        pmPrintf(pmText(PMT_XY_IGNORED));
    }
}

/* ---------------------------------------------------------------------------
 *  Sub-screen: Set RNG Seed
 * ---------------------------------------------------------------------------
 *  Two fields: Mode (AUTO / FIXED) and Value (unsigned 32-bit).
 *  AUTO  = on confirm, re-seed via ps1SeedRandom() (root counters).
 *  FIXED = on confirm, srand(value).
 *  LEFT/RIGHT on the value field nudges by 1; L1/R1 by 100; L2/R2 by 10000.
 *  X = confirm, Circle/START = back without applying.
 * ------------------------------------------------------------------------- */
static void drawSetSeed(void)
{
    pmPrintf("\n");
    drawSeparator();
    pmPrintf(pmText(PMT_SEED_TITLE));
    drawSeparator();
    pmPrintf("\n");

    pmPrintf(pmText(PMT_SEED_MODE),
             editSeedField == 0 ? ">" : " ",
             editSeedField == 0 ? "[" : " ",
             editSeedFixed ? pmText(PMT_FIXED) : pmText(PMT_AUTO),
             editSeedField == 0 ? "]" : " ");

    pmPrintf(pmText(PMT_SEED_VALUE),
             editSeedField == 1 ? ">" : " ",
             editSeedField == 1 ? "[" : " ",
             editSeedValue,
             editSeedField == 1 ? "]" : " ");

    pmPrintf("\n");
    if (ps1LastSeedKnown) {
        pmPrintf(pmText(PMT_CURRENT_SEED), ps1LastSeedApplied);
    } else {
        pmPrintf(pmText(PMT_CURRENT_SEED_UNKNOWN));
    }

    pmPrintf("\n");
    pmPrintf(pmText(PMT_UP_DOWN_SELECT_FIELD));
    pmPrintf(pmText(PMT_LEFT_RIGHT_PLUS1));
    pmPrintf(pmText(PMT_L1_R1_PLUS100));
    pmPrintf(pmText(PMT_L2_R2_PLUS10000));
    pmPrintf(pmText(PMT_X_CONFIRM_BACK_EQ));
}

/* ---------------------------------------------------------------------------
 *  Sub-screen: Credits
 * ---------------------------------------------------------------------------
 *  A labor-of-love attribution + open-source notice + the "if you paid
 *  you were cheated" disclaimer + the upstream URL. Circle/START = back.
 *  Lines are tuned to ~30 chars max so they fit the panel width.
 * ------------------------------------------------------------------------- */
static void drawCredits(void)
{
    pmPrintf(pmText(PMT_CREDITS_TITLE));
    drawSeparator();
    pmPrintf(pmText(PMT_CREDITS_LOVE));
    pmPrintf(pmText(PMT_CREDITS_HUNTER));
    pmPrintf("\n");
    pmPrintf(pmText(PMT_CREDITS_NO_OWN_1));
    pmPrintf(pmText(PMT_CREDITS_NO_OWN_2));
    pmPrintf(pmText(PMT_CREDITS_NO_OWN_3));
    pmPrintf(pmText(PMT_CREDITS_CREATOR));
    pmPrintf(pmText(PMT_CREDITS_FAN_PORTS));
    pmPrintf("\n");
    pmPrintf(pmText(PMT_CREDITS_PAID_1));
    pmPrintf(pmText(PMT_CREDITS_PAID_2));
    pmPrintf(pmText(PMT_CREDITS_FREE));
    pmPrintf(pmText(PMT_CREDITS_URL_1));
    pmPrintf(pmText(PMT_CREDITS_URL_2));
}
