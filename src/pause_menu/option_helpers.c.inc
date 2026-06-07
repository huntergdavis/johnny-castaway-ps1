/* ---------------------------------------------------------------------------
 *  Main menu drawing
 * ------------------------------------------------------------------------- */
static const char *perfLevelLabel(void)
{
    switch (ps1PerfLevel) {
    case 0: return "OFF";
    case 1: return "SUMMARY";
    case 2: return "DETAIL";
    case 3: return "DEBUG";
    default: return "?";
    }
}

/* Externs from jc_reborn.c: -1 = auto/random, else forced. */
extern int hostForcedNight;
extern int hostHolidayMode;
extern int hostForcedHoliday;
extern int hostForcedLowTide;
extern int hostForcedRaftStage;
extern int hostForcedIslandPosValid;
extern int hostForcedIslandX;
extern int hostForcedIslandY;
extern int ps1SoftHour;
extern int ps1SoftMinute;
extern int ps1SoftMonth;
extern int ps1SoftDay;
extern int ps1SoftYear;
extern int ps1SoftTimeEnabled;

static const char *daynightLabel(void)
{
    switch (hostForcedNight) {
    case -1: return "AUTO";
    case 0:  return "DAY";
    case 1:  return "NIGHT";
    default: return "?";
    }
}

static const char *holidayLabel(void)
{
    if (hostHolidayMode == HOLIDAY_MODE_AUTO_ORIGINAL4 ||
        hostHolidayMode == HOLIDAY_MODE_AUTO_ALL)
        return "AUTO";
    if (hostHolidayMode == HOLIDAY_MODE_NONE) return "NONE";
    if (hostForcedHoliday == 0) return "NONE";
    return holidayShortName(hostForcedHoliday);
}

static int holidaySetMode(void)
{
    if (hostHolidayMode < 0 || hostHolidayMode >= HOLIDAY_MODE_COUNT)
        return HOLIDAY_MODE_AUTO_ORIGINAL4;
    return hostHolidayMode;
}

static const char *holidaySetLabel(void)
{
    switch (holidaySetMode()) {
    case HOLIDAY_MODE_AUTO_ORIGINAL4: return "AUTO DATE:ORIG4";
    case HOLIDAY_MODE_AUTO_ALL:       return "AUTO DATE";
    case HOLIDAY_MODE_NONE:           return "NONE";
    case HOLIDAY_MODE_MANUAL_ORIG4:   return "ORIGINAL 4";
    case HOLIDAY_MODE_MANUAL_EXPANDED:return "EXPANDED";
    default: return "?";
    }
}

static int firstExpandedHoliday(void)
{
    return holidayFirstExpandedId();
}

static int nextHolidayInRange(int current, int minId, int maxId, int dir)
{
    int best = 0;
    int found = 0;
    if (dir >= 0) {
        for (int i = 0; i < gHolidayCount; i++) {
            int id = gHolidays[i].id;
            if (id < minId || id > maxId)
                continue;
            if (!best)
                best = id;
            if (found)
                return id;
            if (id == current)
                found = 1;
        }
        return best;
    }

    for (int i = gHolidayCount - 1; i >= 0; i--) {
        int id = gHolidays[i].id;
        if (id < minId || id > maxId)
            continue;
        if (!best)
            best = id;
        if (found)
            return id;
        if (id == current)
            found = 1;
    }
    return best;
}

static void cycleHolidaySet(int dir)
{
    int mode = holidaySetMode();
    mode += (dir >= 0) ? 1 : -1;
    if (mode >= HOLIDAY_MODE_COUNT) mode = 0;
    if (mode < 0) mode = HOLIDAY_MODE_COUNT - 1;

    hostHolidayMode = mode;
    switch (mode) {
    case HOLIDAY_MODE_AUTO_ORIGINAL4:
    case HOLIDAY_MODE_AUTO_ALL:
    case HOLIDAY_MODE_NONE:
        hostForcedHoliday = 0;
        break;
    case HOLIDAY_MODE_MANUAL_ORIG4:
        if (hostForcedHoliday < 1 || hostForcedHoliday > 4)
            hostForcedHoliday = 1;
        break;
    case HOLIDAY_MODE_MANUAL_EXPANDED:
        if (hostForcedHoliday <= 4)
            hostForcedHoliday = firstExpandedHoliday();
        break;
    }
    if (menuFreeplayActive)
        pauseMenuRequestFreeplayWorldRefresh = 1;
}

static void cycleHolidaySelection(int dir)
{
    int oldHoliday = hostForcedHoliday;
    if (hostHolidayMode == HOLIDAY_MODE_MANUAL_ORIG4) {
        hostForcedHoliday = nextHolidayInRange(hostForcedHoliday, 1, 4, dir);
    } else if (hostHolidayMode == HOLIDAY_MODE_MANUAL_EXPANDED) {
        hostForcedHoliday = nextHolidayInRange(hostForcedHoliday, 5,
                                               holidayMaxId(), dir);
    }
    if (menuFreeplayActive && hostForcedHoliday != oldHoliday)
        pauseMenuRequestFreeplayWorldRefresh = 1;
}

static void cycleDaynight(int dir)
{
    /* AUTO(-1) -> DAY(0) -> NIGHT(1) -> AUTO */
    hostForcedNight += dir;
    if (hostForcedNight > 1)  hostForcedNight = -1;
    if (hostForcedNight < -1) hostForcedNight = 1;
    if (menuFreeplayActive)
        pauseMenuRequestFreeplayWorldRefresh = 1;
}

static void cycleHoliday(int dir)
{
    cycleHolidaySelection(dir);
}

static const char *tideLabel(void)
{
    switch (hostForcedLowTide) {
    case -1: return "AUTO";
    case  0: return "HIGH";
    case  1: return "LOW";
    default: return "?";
    }
}

static void cycleTide(int dir)
{
    /* AUTO(-1) -> HIGH(0) -> LOW(1) -> AUTO */
    hostForcedLowTide += dir;
    if (hostForcedLowTide > 1)  hostForcedLowTide = -1;
    if (hostForcedLowTide < -1) hostForcedLowTide = 1;
    if (menuFreeplayActive)
        pauseMenuRequestFreeplayWorldRefresh = 1;
}

static const char *raftLabel(void)
{
    switch (hostForcedRaftStage) {
    case -1: return "AUTO";
    case  0: return "NONE";
    case  1: return "1";
    case  2: return "2";
    case  3: return "3";
    case  4: return "4";
    case  5: return "5";
    default: return "?";
    }
}

static void cycleRaft(int dir)
{
    /* AUTO(-1) -> NONE(0) -> 1..5 -> AUTO */
    hostForcedRaftStage += dir;
    if (hostForcedRaftStage > 5)  hostForcedRaftStage = -1;
    if (hostForcedRaftStage < -1) hostForcedRaftStage = 5;
    if (menuFreeplayActive)
        pauseMenuRequestFreeplayWorldRefresh = 1;
}

static void cyclePerf(int dir)
{
    /* OFF(0) <-> SUMMARY(1) <-> DETAIL(2) <-> DEBUG(3) wrapping */
    int lvl = ((int)ps1PerfLevel + dir) & 0x3;
    ps1PerfSetLevel((uint32)lvl);
}

static const char *captionsLabel(void)
{
    return captionsGetEnabled() ? "ON" : "OFF";
}

static void cycleCaptions(int dir)
{
    /* Two states; either direction toggles. */
    (void)dir;
    captionsSetEnabled(!captionsGetEnabled());
}

static void drawMainMenu(void)
{
    pmPrintf(" JOHNNY CASTAWAY  - PAUSED -\n");
    drawSeparator();

    pmPrintf(" %s Resume\n",
             menuCursor == MENU_RESUME ? ">" : " ");
    /* Scene Set Options is a sub-screen; the inline cycler moved into
     * it alongside the new Scene Picker policy selector. The active
     * set name lives inside the sub-screen — appending it here would
     * overflow the panel for the longer set names ("Misc & Suzy",
     * "Johnny Stories", etc.). */
    pmPrintf(" %s Scene Set Options...\n",
             menuCursor == MENU_SCENE_SET ? ">" : " ");
    pmPrintf(" %s Scene Explorer\n",
             menuCursor == MENU_SCENE_EXPLORER ? ">" : " ");
    pmPrintf(" %s Freeplay: %s\n",
             menuCursor == MENU_FREEPLAY ? ">" : " ",
             menuFreeplayActive ? "ON" : "OFF");
    pmPrintf(" %s Freeplay Options\n",
             menuCursor == MENU_FREEPLAY_OPTIONS ? ">" : " ");
    pmPrintf(" %s World Options\n",
             menuCursor == MENU_WORLD ? ">" : " ");
    pmPrintf(" %s Accessibility\n",
             menuCursor == MENU_ACCESSIBILITY ? ">" : " ");
    pmPrintf(" %s System\n",
             menuCursor == MENU_SYSTEM ? ">" : " ");

    drawSeparator();
    pmPrintf("  X = select   O/START = resume\n");
}

/* ---------------------------------------------------------------------------
 *  Sub-screen: Options (sound, day/night, tide, raft, holiday, perf log)
 * ------------------------------------------------------------------------- */
static void drawOptions(void)
{
    pmPrintf("     WORLD OPTIONS\n");
    drawSeparator();

    pmPrintf(" %s Day/Night: %s\n",
             optionsCursor == WORLD_DAYNIGHT ? ">" : " ", daynightLabel());
    pmPrintf(" %s Tide:      %s\n",
             optionsCursor == WORLD_TIDE ? ">" : " ", tideLabel());
    pmPrintf(" %s Raft:      %s\n",
             optionsCursor == WORLD_RAFT ? ">" : " ", raftLabel());
    pmPrintf(" %s Holidays...\n",
             optionsCursor == WORLD_HOLIDAYS ? ">" : " ");
    pmPrintf(" %s Set Island Pos...\n",
             optionsCursor == WORLD_ISLAND_POS ? ">" : " ");
    pmPrintf(" %s Back\n",
             optionsCursor == WORLD_BACK ? ">" : " ");

    drawSeparator();
    pmPrintf(" LEFT/RIGHT or X adjust\n");
    pmPrintf(" O/START = back\n");
}

static void drawHolidayMenu(void)
{
    pmPrintf("        HOLIDAYS\n");
    drawSeparator();
    pmPrintf(" %s Set:     %s\n",
             holidayCursor == HOLIDAY_SET ? ">" : " ", holidaySetLabel());
    pmPrintf(" %s Holiday: %s\n",
             holidayCursor == HOLIDAY_SELECT ? ">" : " ", holidayLabel());
    pmPrintf(" %s Back\n",
             holidayCursor == HOLIDAY_BACK ? ">" : " ");
    drawSeparator();
    if (holidayModeIsManual(hostHolidayMode) &&
        hostForcedHoliday > 0 && holidayById(hostForcedHoliday)) {
        pmPrintf("   %s\n", holidayDateLabel(hostForcedHoliday));
        pmPrintf("   %.28s\n", holidayTitle(hostForcedHoliday));
    } else if ((hostHolidayMode == HOLIDAY_MODE_AUTO_ORIGINAL4 ||
                hostHolidayMode == HOLIDAY_MODE_AUTO_ALL) &&
               ps1SoftTimeEnabled) {
        int dateHoliday = (hostHolidayMode == HOLIDAY_MODE_AUTO_ORIGINAL4)
                        ? holidayForDateOriginal4(ps1SoftYear, ps1SoftMonth, ps1SoftDay)
                        : holidayForDate(ps1SoftYear, ps1SoftMonth, ps1SoftDay);
        pmPrintf("   Date picker: %s\n",
                 dateHoliday ? holidayShortName(dateHoliday) : "NONE");
    } else if (hostHolidayMode == HOLIDAY_MODE_AUTO_ORIGINAL4) {
        pmPrintf("   Random original 4 until\n");
        pmPrintf("   Set Time/Date is saved.\n");
    } else if (hostHolidayMode == HOLIDAY_MODE_AUTO_ALL) {
        pmPrintf("   Random expanded holiday\n");
        pmPrintf("   until Set Time/Date.\n");
    }
    pmPrintf(" O/START = back\n");
}

static void drawAccessibilityMenu(void)
{
    extern int oceanAmbientEnabled;
    const char *soundLabel = soundMuted ? "MUTED" : "ON";
    const char *oceanLabel = oceanAmbientEnabled ? "ON" : "OFF";

    pmPrintf("     ACCESSIBILITY\n");
    drawSeparator();
    pmPrintf(" %s Captions:  %s\n",
             accessCursor == ACCESS_CAPTIONS ? ">" : " ", captionsLabel());
    pmPrintf(" %s Sound:     %s\n",
             accessCursor == ACCESS_SOUND ? ">" : " ", soundLabel);
    pmPrintf(" %s Ocean:     %s\n",
             accessCursor == ACCESS_OCEAN ? ">" : " ", oceanLabel);
    pmPrintf(" %s Sound Test...\n",
             accessCursor == ACCESS_SOUND_TEST ? ">" : " ");
    pmPrintf(" %s Back\n",
             accessCursor == ACCESS_BACK ? ">" : " ");
    drawSeparator();
    pmPrintf(" LEFT/RIGHT or X adjust\n");
    pmPrintf(" O/START = back\n");
}

static const char *soundTestName(int index)
{
    static const char *const names[] = {
        "SOUND00", "SOUND01", "SOUND02", "SOUND03", "SOUND04",
        "SOUND05", "SOUND06", "SOUND07", "SOUND08", "SOUND09",
        "SOUND10", "SOUND11", "SOUND12", "SOUND13", "SOUND14",
        "SOUND15", "SOUND16", "SOUND17", "SOUND18", "SOUND19",
        "SOUND20", "SOUND21", "SOUND22", "SOUND23", "SOUND24"
    };
    if (index < 0 || index >= (int)(sizeof(names) / sizeof(names[0])))
        return "SOUND??";
    return names[index];
}

static void drawSoundTestMenu(void)
{
    int count = soundEffectCount();
    if (count <= 0)
        count = 1;
    if (soundTestCursor < 0)
        soundTestCursor = count - 1;
    if (soundTestCursor >= count)
        soundTestCursor = 0;

    pmPrintf("       SOUND TEST\n");
    drawSeparator();
    pmPrintf(" %02d/%02d  %s\n",
             soundTestCursor + 1, count, soundTestName(soundTestCursor));
    pmPrintf(" Status: %s\n",
             soundEffectLoaded(soundTestCursor) ? "READY" : "MISSING");
    pmPrintf(" Size:   %lu KB\n",
             (soundEffectSizeBytes(soundTestCursor) + 1023UL) / 1024UL);
    pmPrintf(" Rate:   %d Hz\n",
             soundEffectSampleRate(soundTestCursor));
    pmPrintf(" Output: %s\n", soundMuted ? "MUTED" : "ON");
    drawSeparator();
    pmPrintf(" UP/DOWN choose\n");
    pmPrintf(" X play   O/START back\n");
}

static void drawSystemMenu(void)
{
    pmPrintf("        SYSTEM\n");
    drawSeparator();
    pmPrintf(" %s Save Settings\n",
             systemCursor == SYSTEM_SAVE ? ">" : " ");
    pmPrintf(" %s Set Time/Date...\n",
             systemCursor == SYSTEM_SET_TIME ? ">" : " ");
    pmPrintf(" %s Set RNG Seed...\n",
             systemCursor == SYSTEM_SET_SEED ? ">" : " ");
    pmPrintf(" %s Perf Log: %s\n",
             systemCursor == SYSTEM_PERF ? ">" : " ", perfLevelLabel());
    {
        int day = storyCurrentDay;
        if (day < 1 || day > 11) day = 1;
        pmPrintf(" %s Story Day: %s%2d/11%s\n",
                 systemCursor == SYSTEM_STORY_DAY ? ">" : " ",
                 systemCursor == SYSTEM_STORY_DAY ? "<" : " ",
                 day,
                 systemCursor == SYSTEM_STORY_DAY ? ">" : " ");
    }
    pmPrintf(" %s Reset Scene\n",
             systemCursor == SYSTEM_RESET_LOOP ? ">" : " ");
    pmPrintf(" %s Next Scene\n",
             systemCursor == SYSTEM_NEXT_SCENE ? ">" : " ");
    drawSeparator();
    pmPrintf(" O/START = back\n");
}
