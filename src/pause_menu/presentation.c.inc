/* ---------------------------------------------------------------------------
 *  Sub-screen: Controls
 * ------------------------------------------------------------------------- */
static void drawControls(void)
{
    pmPrintf("       CONTROLS\n");
    drawSeparator();
    pmPrintf(" D-PAD/L-STICK  Walk\n");
    pmPrintf(" L2 / R2        Slow / Fast\n");
    pmPrintf(" CIRCLE         Fish\n");
    pmPrintf(" R1+UP          Day/Night\n");
    pmPrintf(" R1+DOWN        Tide\n");
    pmPrintf(" R1+LEFT        Raft\n");
    pmPrintf(" R1+RIGHT       Holiday\n");
    pmPrintf(" START          Pause\n");
    drawSeparator();
    pmPrintf(" O/START = back\n");
}

static void drawFreeplayOptions(void)
{
    pmPrintf("    FREEPLAY OPTIONS\n");
    drawSeparator();
    pmPrintf(" %s Gags...\n",
             freeplayOptionsCursor == FPO_GAGS ? ">" : " ");
    pmPrintf(" %s Visitors...\n",
             freeplayOptionsCursor == FPO_VISITORS ? ">" : " ");
    pmPrintf(" %s Controls\n",
             freeplayOptionsCursor == FPO_CONTROLS ? ">" : " ");
    pmPrintf(" %s Clear Screen\n",
             freeplayOptionsCursor == FPO_CLEAR ? ">" : " ");
    pmPrintf(" %s Back\n",
             freeplayOptionsCursor == FPO_BACK ? ">" : " ");
    drawSeparator();
    if (!menuFreeplayActive)
        pmPrintf(" Start freeplay to spawn.\n");
    pmPrintf(" X = select   O/START = back\n");
}

/* ---------------------------------------------------------------------------
 *  Scene Set Options sub-screen — pool selector + picker policy in one
 *  page. Up/Down navigates rows, Left/Right cycles values inline,
 *  Cross commits any pending Scene Set change and backs out, Circle
 *  backs out without committing.
 * ------------------------------------------------------------------------- */
static void drawSceneSetOptions(void)
{
    pmPrintf("   SCENE SET OPTIONS\n");
    drawSeparator();

    /* Scene Set row: shows pending value with arrows when focused. */
    {
        int showIdx = (sceneSetOptionsCursor == SSO_SCENE_SET)
                          ? pendingSceneSet : pauseMenuSceneSet;
        int dirty   = (pendingSceneSet != pauseMenuSceneSet);
        if (showIdx < 0 || showIdx >= NUM_SCENE_SETS) showIdx = 0;
        pmPrintf(" %s Set:    %s%s%s%s\n",
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
        pmPrintf(" %s Picker: %s%s%s\n",
                 sceneSetOptionsCursor == SSO_PICKER ? ">" : " ",
                 sceneSetOptionsCursor == SSO_PICKER ? "<" : " ",
                 pickerPolicyName(policy),
                 sceneSetOptionsCursor == SSO_PICKER ? ">" : " ");
    }

    pmPrintf(" %s Back\n",
             sceneSetOptionsCursor == SSO_BACK ? ">" : " ");

    drawSeparator();
    pmPrintf(" L/R cycle   X commit\n");
    pmPrintf(" O/START = back\n");
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

    pmPrintf("      FREEPLAY GAG\n");
    drawSeparator();
    pmPrintf(" %02d/%02d  %.22s\n",
             freeplayGagCursor + 1, count,
             freeplayGagTitle(freeplayGagCursor));
    pmPrintf(" BMP: %.18s\n", freeplayGagBmp(freeplayGagCursor));
    pmPrintf(" Frames: %-3d  RAM: ~%d KB\n",
             freeplayGagFrames(freeplayGagCursor),
             freeplayGagMemoryKB(freeplayGagCursor));
    pmPrintf(" %.28s\n", freeplayGagDescription(freeplayGagCursor));
    drawSeparator();
    pmPrintf(" UP/DOWN choose\n");
    pmPrintf(menuFreeplayActive ? " X spawn now\n" : " Start freeplay first\n");
    pmPrintf(" O/START = back\n");
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

    pmPrintf("    FREEPLAY VISITOR\n");
    drawSeparator();
    pmPrintf(" %02d/%02d  %.22s\n",
             freeplayVisitorCursor + 1, count,
             freeplayVisitorTitle(freeplayVisitorCursor));
    pmPrintf(" BMP: %.18s\n", freeplayVisitorBmp(freeplayVisitorCursor));
    pmPrintf(" Frames: %-3d  RAM: ~%d KB\n",
             freeplayVisitorFrames(freeplayVisitorCursor),
             freeplayVisitorMemoryKB(freeplayVisitorCursor));
    pmPrintf(" %.28s\n", freeplayVisitorDescription(freeplayVisitorCursor));
    drawSeparator();
    pmPrintf(" UP/DOWN choose\n");
    pmPrintf(menuFreeplayActive ? " X spawn now\n" : " Start freeplay first\n");
    pmPrintf(" O/START = back\n");
}

/* ---------------------------------------------------------------------------
 *  Sub-screen: Set Time/Date
 * ------------------------------------------------------------------------- */
static void drawSetTime(void)
{
    const char *fieldNames[] = {"Month","Day","Year","Hour","Min"};

    pmPrintf("\n");
    drawSeparator();
    pmPrintf("   SET TIME AND DATE\n");
    drawSeparator();
    pmPrintf("\n");

    /* Month */
    pmPrintf(" %s Month: %s%02d%s\n",
             editField == 0 ? ">" : " ",
             editField == 0 ? "[" : " ",
             editMonth,
             editField == 0 ? "]" : " ");

    /* Day */
    pmPrintf(" %s Day:   %s%02d%s\n",
             editField == 1 ? ">" : " ",
             editField == 1 ? "[" : " ",
             editDay,
             editField == 1 ? "]" : " ");

    /* Year */
    pmPrintf(" %s Year:  %s%04d%s\n",
             editField == 2 ? ">" : " ",
             editField == 2 ? "[" : " ",
             editYear,
             editField == 2 ? "]" : " ");

    /* Hour */
    pmPrintf(" %s Hour:  %s%02d%s\n",
             editField == 3 ? ">" : " ",
             editField == 3 ? "[" : " ",
             editHour,
             editField == 3 ? "]" : " ");

    /* Minute */
    pmPrintf(" %s Min:   %s%02d%s\n",
             editField == 4 ? ">" : " ",
             editField == 4 ? "[" : " ",
             editMinute,
             editField == 4 ? "]" : " ");

    {
        int dateHoliday = holidayForDate(editYear, editMonth, editDay);
        pmPrintf("   Holiday: %s\n",
                 dateHoliday ? holidayShortName(dateHoliday) : "NONE");
        if (dateHoliday)
            pmPrintf("   %.28s\n", holidayTitle(dateHoliday));
    }

    pmPrintf("\n");
    pmPrintf(" UP/DOWN select field\n");
    pmPrintf(" LEFT/RIGHT adjust value\n");
    pmPrintf(" X to confirm, O/START back\n");

    (void)fieldNames;
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
    pmPrintf("    SET ISLAND POSITION\n");
    drawSeparator();
    pmPrintf("\n");

    pmPrintf(" %s X offset:  %s%+4d%s\n",
             editIslandField == 0 ? ">" : " ",
             editIslandField == 0 ? "[" : " ",
             editIslandX,
             editIslandField == 0 ? "]" : " ");
    pmPrintf(" %s Y offset:  %s%+4d%s\n",
             editIslandField == 1 ? ">" : " ",
             editIslandField == 1 ? "[" : " ",
             editIslandY,
             editIslandField == 1 ? "]" : " ");
    pmPrintf(" %s Mode:      %s%s%s\n",
             editIslandField == 2 ? ">" : " ",
             editIslandField == 2 ? "[" : " ",
             editIslandValid ? "MANUAL" : "AUTO",
             editIslandField == 2 ? "]" : " ");

    pmPrintf("\n");
    pmPrintf(" UP/DOWN  select field\n");
    pmPrintf(" LEFT/RIGHT adjust X/Y by 4\n");
    pmPrintf("           toggle mode\n");
    pmPrintf(" X = confirm   O/START = back\n");

    if (!editIslandValid && (editIslandX != 0 || editIslandY != 0)) {
        pmPrintf("\n");
        pmPrintf(" (X/Y kept; AUTO ignores them)\n");
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
    pmPrintf("    SET RNG SEED\n");
    drawSeparator();
    pmPrintf("\n");

    pmPrintf(" %s Mode:   %s%s%s\n",
             editSeedField == 0 ? ">" : " ",
             editSeedField == 0 ? "[" : " ",
             editSeedFixed ? "FIXED" : "AUTO",
             editSeedField == 0 ? "]" : " ");

    pmPrintf(" %s Value:  %s%10u%s\n",
             editSeedField == 1 ? ">" : " ",
             editSeedField == 1 ? "[" : " ",
             editSeedValue,
             editSeedField == 1 ? "]" : " ");

    pmPrintf("\n");
    if (ps1LastSeedKnown) {
        pmPrintf(" Current seed: %u\n", ps1LastSeedApplied);
    } else {
        pmPrintf(" Current seed: (unknown)\n");
    }

    pmPrintf("\n");
    pmPrintf(" UP/DOWN  select field\n");
    pmPrintf(" LEFT/RIGHT  +/-1\n");
    pmPrintf(" L1/R1       +/-100\n");
    pmPrintf(" L2/R2       +/-10000\n");
    pmPrintf(" X = confirm   O/START = back\n");
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
    pmPrintf(" CREDITS  (O/START to return)\n");
    drawSeparator();
    pmPrintf(" A labor of love by\n");
    pmPrintf(" Hunter Davis.\n");
    pmPrintf("\n");
    pmPrintf(" Hunter does not own or have\n");
    pmPrintf(" a license to the Johnny\n");
    pmPrintf(" Castaway character. The\n");
    pmPrintf(" original creator generously\n");
    pmPrintf(" allows fan ports.\n");
    pmPrintf("\n");
    pmPrintf(" If you paid for this, you\n");
    pmPrintf(" were cheated.\n");
    pmPrintf(" Open source and free:\n");
    pmPrintf(" github.com/huntergdavis/\n");
    pmPrintf(" Johnny-Castaway-PS1\n");
}
