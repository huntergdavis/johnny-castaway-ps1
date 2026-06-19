/* ---------------------------------------------------------------------------
 *  Main menu drawing
 * ------------------------------------------------------------------------- */
static const char *perfLevelLabel(void)
{
    switch (ps1PerfLevel) {
    case 0: return pmText(PMT_OFF);
    case 1: return pmText(PMT_SUMMARY);
    case 2: return pmText(PMT_DETAIL);
    case 3: return pmText(PMT_DEBUG);
    default: return pmText(PMT_UNKNOWN);
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
    case -1: return pmText(PMT_AUTO);
    case 0:  return pmText(PMT_DAY);
    case 1:  return pmText(PMT_NIGHT);
    default: return pmText(PMT_UNKNOWN);
    }
}

static const char *holidayLabel(void)
{
    if (hostHolidayMode == HOLIDAY_MODE_AUTO_ORIGINAL4 ||
        hostHolidayMode == HOLIDAY_MODE_AUTO_ALL)
        return pmText(PMT_AUTO);
    if (hostHolidayMode == HOLIDAY_MODE_NONE) return pmText(PMT_NONE);
    if (hostForcedHoliday == 0) return pmText(PMT_NONE);
    return pmHolidayShortName(hostForcedHoliday);
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
    case HOLIDAY_MODE_AUTO_ORIGINAL4: return pmText(PMT_AUTO_DATE_ORIG4);
    case HOLIDAY_MODE_AUTO_ALL:       return pmText(PMT_AUTO_DATE);
    case HOLIDAY_MODE_NONE:           return pmText(PMT_NONE);
    case HOLIDAY_MODE_MANUAL_ORIG4:   return pmText(PMT_ORIGINAL_4);
    case HOLIDAY_MODE_MANUAL_EXPANDED:return pmText(PMT_EXPANDED);
    default: return pmText(PMT_UNKNOWN);
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
    case -1: return pmText(PMT_AUTO);
    case  0: return pmText(PMT_HIGH);
    case  1: return pmText(PMT_LOW);
    default: return pmText(PMT_UNKNOWN);
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
    case -1: return pmText(PMT_AUTO);
    case  0: return pmText(PMT_NONE);
    case  1: return "1";
    case  2: return "2";
    case  3: return "3";
    case  4: return "4";
    case  5: return "5";
    default: return pmText(PMT_UNKNOWN);
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
    return captionsGetEnabled() ? pmText(PMT_ON) : pmText(PMT_OFF);
}

static void cycleCaptions(int dir)
{
    /* Two states; either direction toggles. */
    (void)dir;
    captionsSetEnabled(!captionsGetEnabled());
}

static void drawMainMenu(void)
{
    pmPrintf(pmText(PMT_MAIN_TITLE));
    drawSeparator();

    /* Below the header: three rows identifying what's on screen —
     *   row 1: catalog number + current animation frame (Scene N/M  Frame X/Y)
     *   row 2: scene name
     *   row 3: scene description
     * The display name is stored as "NAME - description"; we split it at the
     * " - " separator. When Johnny is walking between scenes the FG2 runtime
     * is idle (empty scene name), so show a walking indicator instead. */
    {
        const char *slug = foregroundPilotRuntimeSceneName();
        if (slug == NULL || slug[0] == '\0') {
            pmPrintf("Walking between scenes...");
            drawSeparator();
        } else {
            int sceneIdx = -1;
            for (int i = 0; i < gSceneExplorerCount; i++) {
                if (strcmp(gSceneExplorer[i].slug, slug) == 0) {
                    sceneIdx = i;
                    break;
                }
            }
            unsigned fIdx = (unsigned)foregroundPilotRuntimeFrameIndex();
            unsigned fCnt = (unsigned)foregroundPilotRuntimeFrameCount();
            if (sceneIdx >= 0) {
                const char *name = sceneExplorerDisplayName(sceneIdx);
                /* Find the " - " separator splitting name from description. */
                const char *desc = NULL;
                int nameLen = 0;
                {
                    int i = 0;
                    while (name[i] != '\0') {
                        if (name[i] == ' ' && name[i + 1] == '-' &&
                            name[i + 2] == ' ') {
                            desc = name + i + 3;
                            break;
                        }
                        i++;
                    }
                    nameLen = i;
                }
                /* Row 1: number + current frame, same line. */
                if (fCnt > 0)
                    pmPrintf("Scene %d/%d   Frame %u/%u", sceneIdx + 1,
                             gSceneExplorerCount, fIdx + 1u, fCnt);
                else
                    pmPrintf("Scene %d/%d", sceneIdx + 1, gSceneExplorerCount);
                /* Row 2: name (ellipsis-truncated to the panel width). */
                {
                    char nameBuf[31];
                    int n = nameLen;
                    if (n > (int)sizeof(nameBuf) - 1) n = sizeof(nameBuf) - 1;
                    for (int i = 0; i < n; i++) nameBuf[i] = name[i];
                    if (nameLen > (int)sizeof(nameBuf) - 1 && n >= 4) {
                        nameBuf[n - 3] = '.'; nameBuf[n - 2] = '.'; nameBuf[n - 1] = '.';
                    }
                    nameBuf[n] = '\0';
                    pmPrintf("%s", nameBuf);
                }
                /* Row 3: description (or family name if the title had none). */
                if (desc != NULL && desc[0] != '\0') {
                    char descBuf[31];
                    int n = 0;
                    while (desc[n] != '\0' && n < (int)sizeof(descBuf) - 1) {
                        descBuf[n] = desc[n];
                        n++;
                    }
                    if (desc[n] != '\0' && n >= 4) {
                        descBuf[n - 3] = '.'; descBuf[n - 2] = '.'; descBuf[n - 1] = '.';
                    }
                    descBuf[n] = '\0';
                    pmPrintf("%s", descBuf);
                } else {
                    pmPrintf("%s",
                             sceneExplorerFamilyName(gSceneExplorer[sceneIdx].family_index));
                }
                drawSeparator();
            } else {
                if (fCnt > 0)
                    pmPrintf("Scene: %s   Frame %u/%u", slug, fIdx + 1u, fCnt);
                else
                    pmPrintf("Scene: %s", slug);
                drawSeparator();
            }
        }
    }

    pmPrintf(pmText(PMT_MAIN_RESUME),
             menuCursor == MENU_RESUME ? ">" : " ");
    /* Scene Set Options is a sub-screen; the inline cycler moved into
     * it alongside the new Scene Picker policy selector. The active
     * set name lives inside the sub-screen — appending it here would
     * overflow the panel for the longer set names ("Misc & Suzy",
     * "Johnny Stories", etc.). */
    pmPrintf(pmText(PMT_MAIN_SCENE_SET),
             menuCursor == MENU_SCENE_SET ? ">" : " ");
    pmPrintf(pmText(PMT_MAIN_SCENE_EXPLORER),
             menuCursor == MENU_SCENE_EXPLORER ? ">" : " ");
    pmPrintf(pmText(PMT_MAIN_FREEPLAY),
             menuCursor == MENU_FREEPLAY ? ">" : " ",
             menuFreeplayActive ? pmText(PMT_ON) : pmText(PMT_OFF));
    pmPrintf(pmText(PMT_MAIN_FREEPLAY_OPTIONS),
             menuCursor == MENU_FREEPLAY_OPTIONS ? ">" : " ");
    pmPrintf(pmText(PMT_MAIN_WORLD_OPTIONS),
             menuCursor == MENU_WORLD ? ">" : " ");
    pmPrintf(pmText(PMT_MAIN_ACCESSIBILITY),
             menuCursor == MENU_ACCESSIBILITY ? ">" : " ");
    pmPrintf(pmText(PMT_MAIN_SYSTEM),
             menuCursor == MENU_SYSTEM ? ">" : " ");

    drawSeparator();
    pmPrintf(pmText(PMT_HINT_SELECT_RESUME));
}

/* ---------------------------------------------------------------------------
 *  Sub-screen: Options (sound, day/night, tide, raft, holiday, perf log)
 * ------------------------------------------------------------------------- */
static void drawOptions(void)
{
    pmPrintf(pmText(PMT_WORLD_TITLE));
    drawSeparator();

    pmPrintf(pmText(PMT_WORLD_DAYNIGHT),
             optionsCursor == WORLD_DAYNIGHT ? ">" : " ", daynightLabel());
    pmPrintf(pmText(PMT_WORLD_TIDE),
             optionsCursor == WORLD_TIDE ? ">" : " ", tideLabel());
    pmPrintf(pmText(PMT_WORLD_RAFT),
             optionsCursor == WORLD_RAFT ? ">" : " ", raftLabel());
    pmPrintf(pmText(PMT_WORLD_HOLIDAYS),
             optionsCursor == WORLD_HOLIDAYS ? ">" : " ");
    pmPrintf(pmText(PMT_WORLD_ISLAND_POS),
             optionsCursor == WORLD_ISLAND_POS ? ">" : " ");
    pmPrintf(pmText(PMT_MENU_BACK),
             optionsCursor == WORLD_BACK ? ">" : " ");

    drawSeparator();
    pmPrintf(pmText(PMT_HINT_ADJUST));
    pmPrintf(pmText(PMT_O_START_BACK));
}

static void drawHolidayMenu(void)
{
    pmPrintf(pmText(PMT_HOLIDAYS_TITLE));
    drawSeparator();
    pmPrintf(pmText(PMT_HOLIDAY_SET),
             holidayCursor == HOLIDAY_SET ? ">" : " ", holidaySetLabel());
    pmPrintf(pmText(PMT_HOLIDAY_SELECT),
             holidayCursor == HOLIDAY_SELECT ? ">" : " ", holidayLabel());
    pmPrintf(pmText(PMT_MENU_BACK),
             holidayCursor == HOLIDAY_BACK ? ">" : " ");
    drawSeparator();
    if (holidayModeIsManual(hostHolidayMode) &&
        hostForcedHoliday > 0 && holidayById(hostForcedHoliday)) {
        pmPrintf(pmText(PMT_HOLIDAY_DATE_LINE), pmHolidayDateLabel(hostForcedHoliday));
        pmPrintf(pmText(PMT_HOLIDAY_TITLE_LINE), pmHolidayTitle(hostForcedHoliday));
    } else if ((hostHolidayMode == HOLIDAY_MODE_AUTO_ORIGINAL4 ||
                hostHolidayMode == HOLIDAY_MODE_AUTO_ALL) &&
               ps1SoftTimeEnabled) {
        int dateHoliday = (hostHolidayMode == HOLIDAY_MODE_AUTO_ORIGINAL4)
                        ? holidayForDateOriginal4(ps1SoftYear, ps1SoftMonth, ps1SoftDay)
                        : holidayForDate(ps1SoftYear, ps1SoftMonth, ps1SoftDay);
        pmPrintf(pmText(PMT_DATE_PICKER),
                 dateHoliday ? pmHolidayShortName(dateHoliday) : pmText(PMT_NONE));
    } else if (hostHolidayMode == HOLIDAY_MODE_AUTO_ORIGINAL4) {
        pmPrintf(pmText(PMT_RANDOM_ORIG4_UNTIL));
        pmPrintf(pmText(PMT_SET_TIME_SAVED));
    } else if (hostHolidayMode == HOLIDAY_MODE_AUTO_ALL) {
        pmPrintf(pmText(PMT_RANDOM_EXPANDED_HOLIDAY));
        pmPrintf(pmText(PMT_UNTIL_SET_TIME));
    }
    pmPrintf(pmText(PMT_O_START_BACK));
}

static void drawAccessibilityMenu(void)
{
    extern int oceanAmbientEnabled;
    const char *soundLabel = soundMuted ? pmText(PMT_MUTED) : pmText(PMT_ON);
    const char *oceanLabel = oceanAmbientEnabled ? pmText(PMT_ON) : pmText(PMT_OFF);

    pmPrintf(pmText(PMT_ACCESS_TITLE));
    drawSeparator();
    pmPrintf(pmText(PMT_ACCESS_CAPTIONS),
             accessCursor == ACCESS_CAPTIONS ? ">" : " ", captionsLabel());
    pmPrintf(pmText(PMT_ACCESS_SOUND),
             accessCursor == ACCESS_SOUND ? ">" : " ", soundLabel);
    pmPrintf(pmText(PMT_ACCESS_OCEAN),
             accessCursor == ACCESS_OCEAN ? ">" : " ", oceanLabel);
    pmPrintf(pmText(PMT_ACCESS_SOUND_TEST),
             accessCursor == ACCESS_SOUND_TEST ? ">" : " ");
    pmPrintf(pmText(PMT_MENU_BACK),
             accessCursor == ACCESS_BACK ? ">" : " ");
    drawSeparator();
    pmPrintf(pmText(PMT_HINT_ADJUST));
    pmPrintf(pmText(PMT_O_START_BACK));
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

    pmPrintf(pmText(PMT_SOUND_TEST_TITLE));
    drawSeparator();
    pmPrintf(pmText(PMT_SOUND_TEST_ENTRY),
             soundTestCursor + 1, count, soundTestCursor);
    pmPrintf(pmText(PMT_STATUS_LINE),
             soundEffectLoaded(soundTestCursor) ? pmText(PMT_READY) : pmText(PMT_MISSING));
    pmPrintf(pmText(PMT_SIZE_LINE),
             (soundEffectSizeBytes(soundTestCursor) + 1023UL) / 1024UL);
    pmPrintf(pmText(PMT_RATE_LINE),
             soundEffectSampleRate(soundTestCursor));
    pmPrintf(pmText(PMT_OUTPUT_LINE), soundMuted ? pmText(PMT_MUTED) : pmText(PMT_ON));
    drawSeparator();
    pmPrintf(pmText(PMT_UP_DOWN_CHOOSE));
    pmPrintf(pmText(PMT_X_PLAY_BACK));
}

static void drawSystemMenu(void)
{
    pmPrintf(pmText(PMT_SYSTEM_TITLE));
    drawSeparator();
    pmPrintf(pmText(PMT_SYSTEM_SAVE),
             systemCursor == SYSTEM_SAVE ? ">" : " ");
    pmPrintf(pmText(PMT_SYSTEM_SET_TIME),
             systemCursor == SYSTEM_SET_TIME ? ">" : " ");
    pmPrintf(pmText(PMT_SYSTEM_SET_SEED),
             systemCursor == SYSTEM_SET_SEED ? ">" : " ");
    pmPrintf(pmText(PMT_SYSTEM_PERF),
             systemCursor == SYSTEM_PERF ? ">" : " ", perfLevelLabel());
    {
        int day = storyCurrentDay;
        if (day < 1 || day > 11) day = 1;
        pmPrintf(pmText(PMT_SYSTEM_STORY_DAY),
                 systemCursor == SYSTEM_STORY_DAY ? ">" : " ",
                 systemCursor == SYSTEM_STORY_DAY ? "<" : " ",
                 day,
                 systemCursor == SYSTEM_STORY_DAY ? ">" : " ");
    }
    pmPrintf(pmText(PMT_SYSTEM_RESET_SCENE),
             systemCursor == SYSTEM_RESET_LOOP ? ">" : " ");
    pmPrintf(pmText(PMT_SYSTEM_NEXT_SCENE),
             systemCursor == SYSTEM_NEXT_SCENE ? ">" : " ");
    pmPrintf("%s Scene Info",
             systemCursor == SYSTEM_SCENE_INFO ? ">" : " ");
    pmPrintf("%s Credits",
             systemCursor == SYSTEM_CREDITS ? ">" : " ");
    drawSeparator();
    pmPrintf(pmText(PMT_O_START_BACK));
}
