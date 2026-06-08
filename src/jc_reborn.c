/*
 *  This file is part of 'Johnny Reborn'
 *
 *  An open-source engine for the classic
 *  'Johnny Castaway' screensaver by Sierra.
 *
 *  Copyright (C) 2019 Jeremie GUILLAUME
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

/* PS1 Build - needs special header handling */
#ifdef PS1_BUILD
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>  /* Provides exit(), atoi(), malloc(), etc. */
#include <string.h>
#ifndef _FILE_DEFINED
#define _FILE_DEFINED
typedef struct _FILE FILE;
#endif
#define stderr ((FILE*)2)  /* PSn00bSDK doesn't define stderr */
#define fprintf(stream, ...) printf(__VA_ARGS__)  /* Redirect to printf */
#ifndef JC_BOOT_DIAG_LOGS
#define JC_BOOT_DIAG_LOGS 0
#endif
#ifndef JC_PRINTF_PROBE_LOGS
#define JC_PRINTF_PROBE_LOGS 0
#endif
#ifndef JC_PAUSE_REQUEST_DIAG_LOGS
#define JC_PAUSE_REQUEST_DIAG_LOGS 0
#endif
#ifndef PS1_VERBOSE_DIAGNOSTICS
#define PS1_VERBOSE_DIAGNOSTICS 0
#endif
/* Declare functions implemented in ps1_stubs.c */
void exit(int status);
int atoi(const char *str);
FILE *fopen(const char *pathname, const char *mode);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
int fclose(FILE *stream);
#else
/* Standard SDL build */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#endif

#include "mytypes.h"
#include "utils.h"
#include "resource.h"

/* Platform-specific headers */
#ifdef PS1_BUILD
#include <psxgpu.h>
#include <psxgte.h>
#include <psxcd.h>
#include <psxapi.h>
#include <psxpad.h>
#include "graphics_ps1.h"
#include "events_ps1.h"
#include "sound_ps1.h"
#include "ps1_spu_cache.h"
#include "memcard.h"
#include "mem_region.h"
#include "cdrom_ps1.h"
#include "ps1_debug.h"
#include "pause_menu.h"
#include "ps1_captions.h"
#include "ps1_pad_script.h"
#include "scene_explorer_data.h"
#include "config/ps1/bootmode_embedded.h"

/* Shared with events_ps1.c — populated by InitPAD(). Padtest path
 * reads it directly. */
extern uint8 pad_buff[2][34];
#else
#include "graphics.h"
#include "events.h"
#include "sound.h"
#endif

#include "island.h"
#include "holidays.h"
#include "foreground_pilot.h"
#include "ps1_perf.h"

#ifdef PS1_BUILD
static void ps1ShowFreeplayLoadingFrame(const char *phase, int tick)
{
    (void)phase;
    grShowMeanwhileLoadingFrame((uint16)tick);
}

static void ps1SpuCacheProofHalt(int ok)
{
    DRAWENV draw;

    ResetGraph(0);
    SetVideoMode(MODE_NTSC);
    SetDefDrawEnv(&draw, 0, 0, 640, 480);
    if (ok)
        setRGB0(&draw, 0, 96, 0);
    else
        setRGB0(&draw, 96, 0, 0);
    draw.isbg = 1;
    PutDrawEnv(&draw);
    SetDispMask(1);

    while (1) {
        VSync(0);
        PutDrawEnv(&draw);
    }
}

/* story_data.h is platform-independent — it's just a const struct
 * array. Pulling it into the PS1 build gives the screensaver-loop
 * picker access to per-scene start/end spot/heading metadata so it
 * can call fgWalkRender between scenes. The story.c / ads.c / ttm.c
 * runtime engines stay out of the PS1 build. */
#include "story_data.h"
#include "walk_render.h"
#include "walk_pilot.h"
#include "scene_freeplay.h"

static void ps1PrepareSceneExplorerLaunch(void)
{
    /* Scene Explorer uses a full frog wipe and deliberately skips the
     * inter-scene walk, so none of the current thumbnail/walk/FG caches are
     * needed while the selected scene allocates its clean snapshot. Keeping
     * normal story-loop buffers resident is still intentional; this is only
     * for the manual direct-launch path. */
    grFreeSceneExplorerThumbnailBuffer();
    fgWalkRenderTeardown();
    walkPilotReleaseCleanWalkArea();
    foregroundPilotTeardownForFreeplay();
    walkRenderResetCache();
}
#endif

#ifndef PS1_BUILD
#include "dump.h"
#include "ttm.h"
#include "ads.h"
#include "story.h"
#endif

/* Root counters are exposed by PSn00bSDK on PS1 builds.
 *
 * Last seed actually fed to srand() — exposed so the pause-menu Set
 * RNG Seed sub-screen can show what's currently in play. Value is the
 * unsigned int that was passed; -1 if no seed has been applied yet. */
unsigned int ps1LastSeedApplied = 0;
int          ps1LastSeedKnown   = 0;

#ifdef PS1_BUILD
#define PS1_RCNT_CNT0_SPEC ((int)0xf2000000u)
#define PS1_RCNT_CNT1_SPEC ((int)0xf2000001u)
#define PS1_RCNT_CNT2_SPEC ((int)0xf2000002u)

void ps1SeedRandom(void)
{
    uint32 seed = 0x9e3779b9u;

    for (int i = 0; i < 32; i++) {
        uint32 t0 = (uint32)GetRCnt(PS1_RCNT_CNT0_SPEC);
        uint32 t1 = (uint32)GetRCnt(PS1_RCNT_CNT1_SPEC);
        uint32 t2 = (uint32)GetRCnt(PS1_RCNT_CNT2_SPEC);
        seed ^= (t0 << (i & 7)) ^ (t1 << ((i + 3) & 7)) ^ (t2 << ((i + 5) & 7));
        seed = (seed << 5) | (seed >> 27);
        seed += 0x7f4a7c15u + (uint32)i;
    }

    if (seed == 0)
        seed = 1;
    srand(seed);
    ps1LastSeedApplied = seed;
    ps1LastSeedKnown   = 1;
}

void ps1SetSeed(unsigned int seed)
{
    srand(seed);
    ps1LastSeedApplied = seed;
    ps1LastSeedKnown   = 1;
}
#else
/* Host build: provide stub linkage for pause_menu compilation parity. */
void ps1SeedRandom(void) {}
void ps1SetSeed(unsigned int seed) { srand(seed); ps1LastSeedApplied = seed; ps1LastSeedKnown = 1; }
#endif


#ifndef PS1_BUILD
static int  argDump     = 0;
static int  argBench    = 0;
static int  argTtm      = 0;
static int  argAds      = 0;
static int  argPlayAll  = 0;
static int  argIsland   = 0;
#endif
static int  argForegroundPilot = 0;

static char *args[3];
static int  numArgs  = 0;

#ifndef PS1_BUILD
static int hostForcedSeed = -1;
static int hostForcedStoryDay = -1;
static int hostBootDirectSceneIndex = -1;
#endif

/* Non-static so the pause menu can flip these. hostForcedIslandPosValid
 * is the gate (0 = AUTO/varpos-random, 1 = use the X/Y values).
 * Set via BOOTMODE tokens (legacy) or pause-menu Set Island Pos. */
int hostForcedIslandPosValid = 0;
int hostForcedIslandX = 0;
int hostForcedIslandY = 0;
/* -1 = auto (random per scene). Set via BOOTMODE tokens (legacy) or
 * pause-menu Options cycling. */
int hostForcedLowTide = -1;
int hostForcedRaftStage = -1;
int hostForcedNight = -1;
int hostHolidayMode = HOLIDAY_MODE_AUTO_ORIGINAL4;
int hostForcedHoliday = 0;
static int hostBootForcedNightValid = 0;
static int hostBootForcedHolidayValid = 0;

/* Screensaver loop: fgpilot mode replays the scene forever with randomized
 * variant params per iteration, unless the `noloop` boot token is set.
 * Variant fields that were explicitly forced via BOOTMODE (night, lowtide,
 * raft-stage, holiday) stay forced across iterations. */
static int screensaverLoopDisabled = 0;

/* Scenes that have reached the "fully validated" bar in
 * docs/ps1/scene-status.md, plus a small set used for walk testing.
 *
 * fishing1/fishing2 are visually-validated (FISHING-bar) and both
 * have spotStart/End at SPOT_D — walks between them are turn-in-place.
 * To exercise the walk subsystem against actual spot-to-spot
 * traversal, the picker also includes building1 (SPOT_F→SPOT_A) and
 * walkstuf3 (SPOT_D→SPOT_E). Their FG2 packs render but their scene
 * content isn't visually-signed-off; they're here for walk-system
 * verification on the walk-implementation-20260429 branch only.
 * Trim the list back to the validated pair before merging to main. */
/* The "All Scenes" pool — every scene that has an FG2 pack on disc.
 * 63 entries, mirroring storyScenes[]. There used to be a hand-curated
 * "proven/validated" subset here; that gate was retired once the full
 * set ran cleanly, so the picker now draws from everything. New scenes
 * land here as soon as their FG2 pack ships on the CD layout. */
static const char *kAllScenes[] = {
    /* Activities (10 — ACTIVITY 2/3 were unused Sierra placeholders) */
    "activity1",  "activity4",  "activity5",  "activity6",  "activity7",
    "activity8",  "activity9",  "activity10", "activity11", "activity12",
    /* Building (7) */
    "building1",  "building2",  "building3",  "building4",  "building5",
    "building6",  "building7",
    /* Fishing (8) */
    "fishing1",   "fishing2",   "fishing3",   "fishing4",   "fishing5",
    "fishing6",   "fishing7",   "fishing8",
    /* Johnny (6) */
    "johnny1",    "johnny2",    "johnny3",    "johnny4",    "johnny5",
    "johnny6",
    /* Mary (5) */
    "mary1",      "mary2",      "mary3",      "mary4",      "mary5",
    /* Misc gags (2) */
    "miscgag1",   "miscgag2",
    /* Stand (14 — gaps in Sierra's tag numbering: STAND 13/14 unused) */
    "stand1",     "stand2",     "stand3",     "stand4",     "stand5",
    "stand6",     "stand7",     "stand8",     "stand9",     "stand10",
    "stand11",    "stand12",    "stand15",    "stand16",
    /* Suzy (2) */
    "suzy1",      "suzy2",
    /* Visitor (6 — VISITOR 2 was an unused Sierra placeholder) */
    "visitor1",   "visitor3",   "visitor4",   "visitor5",   "visitor6",
    "visitor7",
    /* Walkstuf (3) */
    "walkstuf1",  "walkstuf2",  "walkstuf3",
};
#define NUM_ALL_SCENES ((int)(sizeof(kAllScenes) / sizeof(kAllScenes[0])))

/* Scene-set framework — the pause-menu "Scene Set" item cycles
 * through these pools. Each set is a family-curated slice of
 * scenes; the screensaver loop draws from the current set's pool
 * instead of the default kAllScenes. Order MUST match the
 * kSceneSetNames array in pause_menu.c.
 *
 * Adding a set: declare a kSet<Name>Scenes[] array, append it to
 * gSceneSetPools, append the human-readable name to kSceneSetNames
 * in pause_menu.c. If a set is empty (size 0), the picker falls
 * back to kAllScenes — same effect as picking "All Scenes" from
 * the menu. */
static const char *kSetFishingScenes[] = {
    "fishing1", "fishing2", "fishing3", "fishing4",
    "fishing5", "fishing6", "fishing7", "fishing8",
};
/* Johnny stories — johnny1..4 are visually signed off as of v0.6.7-ps1;
 * johnny5/6 ride along (their FG2 packs ship on disc) and will look
 * right as visual validation lands without any code change here. */
static const char *kSetJohnnyScenes[] = {
    "johnny1", "johnny2", "johnny3", "johnny4", "johnny5", "johnny6",
};
/* Mary visits — none visually signed off yet, but FG2 packs are on disc
 * (MARY1..MARY5 + LOW variants), so the runtime can play them. As each
 * scene gets a ✅ in scene-status.md the pool just lights up. */
static const char *kSetMaryScenes[] = {
    "mary1", "mary2", "mary3", "mary4", "mary5",
};
/* Visitors (gulls, biplane, canoe, etc.) — VISITOR2 has no FG2 on disc,
 * so the pool skips it instead of risking a pack-start failure. */
static const char *kSetVisitorScenes[] = {
    "visitor1", "visitor3", "visitor4", "visitor5", "visitor6", "visitor7",
};
/* Activities — ACTIVITY2/3 have no FG2 on disc; the pool follows the
 * scene-status.md table. */
static const char *kSetActivityScenes[] = {
    "activity1", "activity4", "activity5", "activity6", "activity7",
    "activity8", "activity9", "activity10", "activity11", "activity12",
};
/* Misc & Suzy — combined because each family alone is just two scenes;
 * together they're a usable 4-scene rotation. */
static const char *kSetMiscSuzyScenes[] = {
    "suzy1", "suzy2", "miscgag1", "miscgag2",
};

struct SceneSetPool {
    const char *const *scenes;
    int               count;
};

#define SCENE_SET_POOL(arr) { arr, (int)(sizeof(arr) / sizeof((arr)[0])) }

static const struct SceneSetPool gSceneSetPools[] = {
    { NULL, 0 },                              /* All Scenes — uses kAllScenes */
    SCENE_SET_POOL(kSetFishingScenes),        /* Fishing Only */
    SCENE_SET_POOL(kSetJohnnyScenes),         /* Johnny Stories */
    SCENE_SET_POOL(kSetMaryScenes),           /* Mary Visits */
    SCENE_SET_POOL(kSetVisitorScenes),        /* Visitors */
    SCENE_SET_POOL(kSetActivityScenes),       /* Activities */
    SCENE_SET_POOL(kSetMiscSuzyScenes),       /* Misc & Suzy */
};
#define NUM_SCENE_SET_POOLS \
    ((int)(sizeof(gSceneSetPools) / sizeof(gSceneSetPools[0])))

#ifndef PS1_BUILD
static int hostForcedSceneOffsetValid = 0;
static int hostForcedSceneOffsetX = 0;
static int hostForcedSceneOffsetY = 0;
static int hostCapturePreludeFrame = 0;
#endif

/* Catalog accessors used by src/scene_picker.c so it doesn't need to
 * know the layout of gSceneSetPools[] / kAllScenes. The picker reads
 * pool counts + slugs through these and chooses an entry per its
 * active policy. Returning a count of 0 from fgLoopGetPoolCount
 * signals an empty / placeholder set; callers fall back to
 * kAllScenes via fgLoopGetAllCount/Slug. */
int fgLoopGetPoolCount(int sceneSetIdx)
{
    if (sceneSetIdx < 0 || sceneSetIdx >= NUM_SCENE_SET_POOLS)
        return 0;
    return gSceneSetPools[sceneSetIdx].count;
}

const char *fgLoopGetPoolSlug(int sceneSetIdx, int index)
{
    if (sceneSetIdx < 0 || sceneSetIdx >= NUM_SCENE_SET_POOLS)
        return NULL;
    const struct SceneSetPool *p = &gSceneSetPools[sceneSetIdx];
    if (index < 0 || index >= p->count || p->scenes == NULL)
        return NULL;
    return p->scenes[index];
}

int fgLoopGetAllCount(void)
{
    return NUM_ALL_SCENES;
}

const char *fgLoopGetAllSlug(int index)
{
    if (index < 0 || index >= NUM_ALL_SCENES)
        return NULL;
    return kAllScenes[index];
}

/* Pick the scene to play on this screensaver-loop iteration. Pinning
 * (Scene Explorer Cross/Triangle, fgpilot CLI) wins, otherwise we
 * delegate to the picker module which honours the user-selected
 * policy (Random / Sequential / Original). The picker also emits the
 * JCPICK telemetry line per pick. */
/* Three-state machine for diagnostic continuity through the scene-
 * pick window — see docs/ps1/memory-region-allocator-plan.md (A22,
 * A27). memHalt call sites use fgLoopGetLastScene() to report the
 * effective scene name; during pre-evict / scene setup / playback
 * the "target" is meaningful even though no scene has fully played
 * yet. */
static int         gFgLoopPickInProgress = 0;
static const char *gFgLoopLastTarget     = NULL;
static const char *gFgLoopLastPlayed     = NULL;

/* Public accessor — called from fgRuntimeReset (which feeds the
 * scene name to memSceneReset for its JCMEM line) and from
 * memHalt-emitting code paths. */
const char *fgLoopGetLastScene(void) {
    return gFgLoopPickInProgress ? gFgLoopLastTarget : gFgLoopLastPlayed;
}

/* Called from the main loop after foregroundPilotPlay returns
 * successfully. */
void fgLoopMarkScenePlayed(void) {
    gFgLoopLastPlayed     = gFgLoopLastTarget;
    gFgLoopPickInProgress = 0;
}

static const char *fgLoopNextScene(const char *explicitScene,
                                   int sceneSetIdx)
{
    const char *chosen;
#ifdef PS1_BUILD
    extern const char *pickerNextScene(const char *explicitScene,
                                       int sceneSetIdx);
    chosen = pickerNextScene(explicitScene, sceneSetIdx);
#else
    (void)sceneSetIdx;
    chosen = explicitScene;
#endif

    /* Update the diagnostic continuity state. lastTarget records the
     * scene we're about to attempt; pickInProgress flips on so
     * fgLoopGetLastScene returns the target rather than the last-
     * successfully-played one. fgLoopMarkScenePlayed flips it off
     * after a successful play. */
    if (chosen != NULL) {
        gFgLoopLastTarget     = chosen;
        gFgLoopPickInProgress = 1;
    }
    return chosen;
}

/* Set by fgLoopApplyVariant when the story-sequence counter expires
 * and a new island position is randomized. The PS1 screensaver loop
 * consumes this to suppress the walk-between-scenes for the iteration
 * where position changed. Host fgpilot shares the variant picker, so
 * keep the flag in shared scope even though only PS1 reads it. */
static int fgLoopSequenceJustReset = 1;   /* iter 0: treat as new sequence */

/* When set, the current explicitScene was pinned by Scene Explorer's
 * Cross press and should clear after one play, so the loop returns to
 * the active Scene Set's pool. Triangle (loop) leaves this 0, matching
 * the legacy CLI fgpilot path. */
static int sceneExplorerOneShot = 0;

#ifdef PS1_BUILD
/* Walk subsystem state — Johnny's last known spot/heading. -1 means
 * "no defined position" (LEFT_ISLAND scene set the sentinel, or this
 * is the first iteration and no scene has run yet). The screensaver
 * loop walks from this state to the next scene's start before each
 * scene plays. */
static int storyCurrentSpot = -1;
static int storyCurrentHdg  = -1;

/* The walk renderer draws on top of the framebuffer left by the previous
 * scene. A walk is only safe when the next scene uses exactly the same island
 * backdrop state. Scene sequences can mix VARPOS_OK and fixed/left-island
 * scenes without a sequence reset, so compare the actual backdrop key instead
 * of trusting fgLoopSequenceJustReset alone. */
static int storyWalkBackdropValid = 0;
static int storyWalkLowTide = 0;
static int storyWalkRaft = 0;
static int storyWalkNight = 0;
static int storyWalkHoliday = 0;
static int storyWalkXPos = 0;
static int storyWalkYPos = 0;

static void fgLoopForgetWalkContext(void)
{
    storyCurrentSpot = -1;
    storyCurrentHdg  = -1;
    storyWalkBackdropValid = 0;
}

static int fgLoopWalkBackdropMatchesCurrent(void)
{
    return storyWalkBackdropValid &&
           storyWalkLowTide == islandState.lowTide &&
           storyWalkRaft == islandState.raft &&
           storyWalkNight == islandState.night &&
           storyWalkHoliday == islandState.holiday &&
           storyWalkXPos == islandState.xPos &&
           storyWalkYPos == islandState.yPos;
}

static void fgLoopRememberWalkBackdrop(const struct TStoryScene *scene)
{
    if (scene == NULL || !(scene->flags & ISLAND)) {
        storyWalkBackdropValid = 0;
        return;
    }
    storyWalkLowTide = islandState.lowTide;
    storyWalkRaft = islandState.raft;
    storyWalkNight = islandState.night;
    storyWalkHoliday = islandState.holiday;
    storyWalkXPos = islandState.xPos;
    storyWalkYPos = islandState.yPos;
    storyWalkBackdropValid = 1;
}

/* 11-day story-calendar progression (Phase 8). Loaded from memcard
 * on boot, advances when ps1Soft date rolls over (mirrors the host
 * engine's day-of-year tracker). The picker filters scenes whose
 * dayNo is non-zero and != storyCurrentDay; matches src/story.c's
 * gating in storyApplySceneDay/storyPickScene. */
int storyCurrentDay      = 1;       /* extern; memcard load writes it */
int storyLastSeenDayOfYr = 0;       /* initialised from memcard */

extern int getDayOfYear(void);

/* Advance storyCurrentDay if the ps1Soft date has rolled over since
 * the last call. Wraps 1..11. Called at the top of each screensaver
 * loop iteration. */
static void fgLoopAdvanceStoryDayIfNeeded(void)
{
    int today = getDayOfYear();
    if (today <= 0) return;     /* defensive — shouldn't happen */
    if (storyLastSeenDayOfYr <= 0) {
        storyLastSeenDayOfYr = today;
        return;                  /* first call — calibrate, don't advance */
    }
    if (today != storyLastSeenDayOfYr) {
        storyCurrentDay++;
        if (storyCurrentDay > 11) storyCurrentDay = 1;
        storyLastSeenDayOfYr = today;
    }
}

/* Match the original story.c's spot/heading validation. */
static int fgLoopIsValidSpot(int spot)
{
    return (spot >= SPOT_A && spot <= SPOT_F);
}

static int fgLoopIsValidHdg(int hdg)
{
    return (hdg >= HDG_S && hdg <= HDG_SE);
}

/* Non-static: scene_picker.c uses these via extern decls so the
 * Original-mode walk-aware retry can skip scenes without valid
 * start/end coordinates (matches Sierra's storyPlay() filter). */
int fgLoopSceneHasValidStart(const struct TStoryScene *s)
{
    return s != NULL
        && fgLoopIsValidSpot(s->spotStart)
        && fgLoopIsValidHdg(s->hdgStart);
}

int fgLoopSceneHasValidEnd(const struct TStoryScene *s)
{
    return s != NULL
        && fgLoopIsValidSpot(s->spotEnd)
        && fgLoopIsValidHdg(s->hdgEnd);
}

/* Resolve a slug like "fishing1" to its storyScenes[] entry by parsing
 * the slug into (family, tag) and linear-searching the array. 63
 * entries; an O(N) scan is ~1µs and only runs once per scene transition.
 *
 * Non-static: src/scene_picker.c uses this via an extern declaration
 * to map slugs back to (adsName, adsTagNo) for repeat-prevention. */
const struct TStoryScene *fgLoopFindStorySceneBySlug(const char *slug)
{
    if (slug == NULL || slug[0] == '\0') return NULL;

    /* Split into letters + digits. */
    int letterLen = 0;
    while (slug[letterLen] && (slug[letterLen] < '0' || slug[letterLen] > '9'))
        letterLen++;
    if (letterLen == 0 || slug[letterLen] == '\0')
        return NULL;

    /* Tag from trailing digits. */
    int tag = 0;
    for (int i = letterLen; slug[i]; i++) {
        if (slug[i] < '0' || slug[i] > '9') return NULL;
        tag = tag * 10 + (slug[i] - '0');
    }

    /* Build the ADS name "FISHING.ADS" from "fishing". */
    char adsName[13];
    int j = 0;
    for (int i = 0; i < letterLen && j < 8; i++) {
        char c = slug[i];
        if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
        adsName[j++] = c;
    }
    /* Append ".ADS" — TStoryScene.adsName uses the original Sierra
     * filename suffix even though the on-disc PS1 packs are .FG2. */
    if (j + 4 >= (int)sizeof(adsName)) return NULL;
    adsName[j++] = '.';
    adsName[j++] = 'A';
    adsName[j++] = 'D';
    adsName[j++] = 'S';
    adsName[j]   = '\0';

    for (int i = 0; i < NUM_SCENES; i++) {
        const struct TStoryScene *s = &storyScenes[i];
        if (s->adsTagNo != tag) continue;
        /* strcmp against fixed-length adsName field. */
        int eq = 1;
        for (int k = 0; k < 13; k++) {
            if (s->adsName[k] != adsName[k]) { eq = 0; break; }
            if (adsName[k] == '\0') break;
        }
        if (eq) return s;
    }
    return NULL;
}

/* Walk Johnny from his current spot/heading to the given scene's
 * start. No-op if any of the spot/hdg values are unset (-1). */
static void fgLoopWalkToScene(const struct TStoryScene *next)
{
    if (!fgLoopSceneHasValidStart(next))
        return;     /* scene doesn't care where Johnny was */
    if (!fgLoopIsValidSpot(storyCurrentSpot)
        || !fgLoopIsValidHdg(storyCurrentHdg))
        return;     /* Johnny has no defined position to walk from */

    fgWalkRender(storyCurrentSpot, storyCurrentHdg,
                 next->spotStart, next->hdgStart);
}

/* Update storyCurrentSpot/Hdg after a scene plays. */
static void fgLoopUpdatePosFromScene(const struct TStoryScene *s)
{
    if (fgLoopSceneHasValidEnd(s)) {
        storyCurrentSpot = s->spotEnd;
        storyCurrentHdg  = s->hdgEnd;
    } else {
        /* LEFT_ISLAND scene, or scene with no defined end. The next
         * picker iteration must skip the walk-to-scene step (caller
         * already handles this via fgLoopSceneHasValidStart's gate). */
        storyCurrentSpot = -1;
        storyCurrentHdg  = -1;
    }
}
#endif /* PS1_BUILD */

static int fgLoopSceneMatchesPrefixNumber(const char *sceneName,
                                          const char *prefix)
{
    if (sceneName == NULL)
        return 0;
    while (*prefix != '\0') {
        if (*sceneName != *prefix)
            return 0;
        sceneName++;
        prefix++;
    }
    if (*sceneName < '0' || *sceneName > '9')
        return 0;
    while (*sceneName >= '0' && *sceneName <= '9')
        sceneName++;
    return *sceneName == '\0';
}

static int fgLoopSceneUsesVarPos(const char *sceneName)
{
#ifdef PS1_BUILD
    const struct TStoryScene *scene = fgLoopFindStorySceneBySlug(sceneName);
    if (scene != NULL)
        return (scene->flags & VARPOS_OK) != 0;
#endif
    return fgLoopSceneMatchesPrefixNumber(sceneName, "activity") ||
           fgLoopSceneMatchesPrefixNumber(sceneName, "building") ||
           fgLoopSceneMatchesPrefixNumber(sceneName, "fishing") ||
           fgLoopSceneMatchesPrefixNumber(sceneName, "johnny") ||
           fgLoopSceneMatchesPrefixNumber(sceneName, "mary") ||
           fgLoopSceneMatchesPrefixNumber(sceneName, "miscgag") ||
           fgLoopSceneMatchesPrefixNumber(sceneName, "stand") ||
           fgLoopSceneMatchesPrefixNumber(sceneName, "suzy") ||
           fgLoopSceneMatchesPrefixNumber(sceneName, "visitor") ||
           fgLoopSceneMatchesPrefixNumber(sceneName, "walkstuf");
}

static int fgLoopSceneUsesLeftIsland(const char *sceneName)
{
#ifdef PS1_BUILD
    const struct TStoryScene *scene = fgLoopFindStorySceneBySlug(sceneName);
    return (scene != NULL && (scene->flags & LEFT_ISLAND) != 0) ? 1 : 0;
#else
    (void)sceneName;
    return 0;
#endif
}

static int fgLoopSceneUsesNoRaft(const char *sceneName)
{
#ifdef PS1_BUILD
    const struct TStoryScene *scene = fgLoopFindStorySceneBySlug(sceneName);
    if (scene != NULL)
        return (scene->flags & NORAFT) != 0;
#endif
    /* Host fgpilot does not link story_data.h. Keep the known no-raft scene
     * here so direct validation and PS1 story playback share the same policy. */
    return sceneName != NULL && strcmp(sceneName, "mary5") == 0;
}

static void fgLoopRandomVarPos(int *outX, int *outY)
{
    if (rand() % 2) {
        *outX = -222 + (rand() % 109);
        *outY = -44  + (rand() % 128);
    } else if (rand() % 2) {
        *outX = -114 + (rand() % 134);
        *outY = -14  + (rand() % 99);
    } else {
        *outX = -114 + (rand() % 119);
        *outY = -73  + (rand() % 60);
    }
}

static int fgLoopRandomHolidayAll(void)
{
    int roll = rand() % (gHolidayCount + 1);
    return (roll == 0) ? 0 : gHolidays[roll - 1].id;
}

static int fgLoopRandomHolidayOriginal4(void)
{
    int roll = rand() % 5; /* none + the four Sierra-era holiday ids */
    return (roll == 0 || !holidayIsOriginalId(roll)) ? 0 : roll;
}

/* Set islandState variant fields for one iteration. Fields explicitly
 * forced via BOOTMODE (hostForced* >= 0) stay forced; unforced fields
 * get a fresh random value each call. Position policy is scene-specific:
 * current validated fgpilot scenes are VARPOS_OK in story_data.h, so their
 * FG2 scene-relative overlays must follow the original random island offset. */
static void fgLoopApplyVariant(const char *sceneName)
{
    extern int ps1SoftTimeEnabled;
    extern int ps1SoftHour;
    extern int ps1SoftMonth;
    extern int ps1SoftDay;
    extern int ps1HolidayFromDate(int month, int day);
    extern int ps1HolidayFromDateOriginal4(int month, int day);

    /* Two pools of pinned state:
     *
     *   SESSION pool — night, holiday. Rolled ONCE at first call,
     *     never re-rolled. The "date" of the screensaver world stays
     *     consistent so users don't see the holiday emblem flip or
     *     day↔night flip mid-run. (Effectively: we pretend the soft
     *     date is fixed at a random day-of-year for the run.)
     *
     *   SEQUENCE pool — lowTide, raft, position. Pinned for 8-12
     *     consecutive scenes (mirrors the original engine's "story
     *     sequence" of 6-19 intermediates per src/story.c:576),
     *     re-rolled at sequence boundary. fgLoopSequenceJustReset
     *     is consumed by the screensaver loop to suppress the
     *     inter-scene walk on the iteration that re-randomized.
     *
     * Pause-menu overrides (hostForced*) and soft-time overrides
     * (ps1SoftTimeEnabled) still win per-field where set. */

    static int sessionValid = 0;
    static int sessionNight = 0;
    static int sessionHoliday = 0;
    static int sessionHolidayOriginal4 = 0;

    static int sequenceScenesRemaining = 0;
    static int seqLowTide = 0;
    static int seqRaft    = 0;
    static int seqXPos    = 0;
    static int seqYPos    = 0;
    static int seqValid   = 0;

    if (!sessionValid) {
        sessionNight = (rand() & 1);
        sessionHoliday = fgLoopRandomHolidayAll();
        sessionHolidayOriginal4 = fgLoopRandomHolidayOriginal4();
        sessionValid = 1;
    }

    int needNewSequence = (sequenceScenesRemaining <= 0) || !seqValid;
    if (needNewSequence) {
        seqLowTide = (rand() & 1);
        seqRaft    = (rand() % 6);
        if (fgLoopSceneUsesVarPos(sceneName)) {
            fgLoopRandomVarPos(&seqXPos, &seqYPos);
        } else if (fgLoopSceneUsesLeftIsland(sceneName)) {
            seqXPos = -272;
            seqYPos = 0;
        } else {
            seqXPos = 0;
            seqYPos = 0;
        }
        sequenceScenesRemaining = 8 + (rand() % 5);   /* 8..12 — long enough that
                                                       * the user feels Johnny is
                                                       * "spending time" at this
                                                       * island position before
                                                       * the next randomization */
        seqValid = 1;
        fgLoopSequenceJustReset = 1;
    } else {
        fgLoopSequenceJustReset = 0;
    }
    sequenceScenesRemaining--;

    islandState.lowTide = (hostForcedLowTide   >= 0) ? hostForcedLowTide   : seqLowTide;
    islandState.raft    = (hostForcedRaftStage >= 0) ? hostForcedRaftStage : seqRaft;
    if (fgLoopSceneUsesNoRaft(sceneName))
        islandState.raft = 0;

    if (hostForcedNight >= 0) {
        islandState.night = hostForcedNight;
    } else if (ps1SoftTimeEnabled) {
        islandState.night = (ps1SoftHour < 6 || ps1SoftHour >= 20) ? 1 : 0;
    } else {
        islandState.night = sessionNight;
    }

    if (hostHolidayMode == HOLIDAY_MODE_MANUAL_ORIG4 ||
        hostHolidayMode == HOLIDAY_MODE_MANUAL_EXPANDED) {
        islandState.holiday = hostForcedHoliday;
    } else if (hostHolidayMode == HOLIDAY_MODE_NONE) {
        islandState.holiday = 0;
    } else if (hostHolidayMode == HOLIDAY_MODE_AUTO_ORIGINAL4) {
        islandState.holiday = ps1SoftTimeEnabled
                            ? ps1HolidayFromDateOriginal4(ps1SoftMonth, ps1SoftDay)
                            : sessionHolidayOriginal4;
    } else if (ps1SoftTimeEnabled) {
        islandState.holiday = ps1HolidayFromDate(ps1SoftMonth, ps1SoftDay);
    } else {
        islandState.holiday = sessionHoliday;
    }

    if (hostForcedIslandPosValid) {
        islandState.xPos = hostForcedIslandX;
        islandState.yPos = hostForcedIslandY;
    } else if (fgLoopSceneUsesVarPos(sceneName) ||
               fgLoopSceneUsesLeftIsland(sceneName)) {
        islandState.xPos = seqXPos;
        islandState.yPos = seqYPos;
    } else {
        islandState.xPos = 0;
        islandState.yPos = 0;
    }
}

#ifdef PS1_BUILD
#define PS1_BOOT_OVERRIDE_FILE "BOOTMODE.TXT"

static int ps1BootForcedSeed = -1;  /* -1 = use hardware RNG */
static char ps1BootArgStorage[3][32];
static char ps1BootOverrideSource[16];
static char ps1BootOverrideText[128];
static char ps1BootForegroundOverlayScene[32];
static int ps1BootSpuCacheTest = 0;
static int ps1BootSpuCacheProof = 0;
#if PS1_VERBOSE_DIAGNOSTICS
static char ps1BootCaptureMetaDirStorage[32];
static char ps1BootCaptureSceneLabelStorage[64];
volatile uint16 ps1BootDbgCaptureMode = 0;
#endif
#if JC_PRINTF_PROBE_LOGS
static int ps1BootPrintfTest = 0;
#endif
#if PS1_VERBOSE_DIAGNOSTICS
/* When set, fire JC_BSOD synthetically right after the first scene
 * completes — used to verify the BSOD UI without a real failure. */
static int ps1BootBsodAfterFirstScene = 0;
#endif

static int ps1IsSpace(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static void ps1ResetBootArgs(void)
{
    argForegroundPilot = 0;
    numArgs = 0;

    for (int i = 0; i < 3; i++) {
        args[i] = NULL;
        ps1BootArgStorage[i][0] = '\0';
    }
    ps1BootOverrideSource[0] = '\0';
    ps1BootOverrideText[0] = '\0';
    ps1BootForegroundOverlayScene[0] = '\0';
    ps1BootSpuCacheTest = 0;
    ps1BootSpuCacheProof = 0;
#if PS1_VERBOSE_DIAGNOSTICS
    ps1BootCaptureMetaDirStorage[0] = '\0';
    ps1BootCaptureSceneLabelStorage[0] = '\0';

    grCaptureMetaDir = NULL;
    grCaptureOverlay = 0;
    grCaptureOverlayMaskOnly = 0;
    grCaptureSetSceneLabel("");
#endif
    foregroundPilotSetHeapProbe(0);
    foregroundPilotSetLoadingWaveProof(0);
    foregroundPilotResetPrefetchDefaults();
    ps1PerfSetEnabled(0);
    freeplaySetTelemetryLevel(0);
#if PS1_VERBOSE_DIAGNOSTICS
    ps1BootDbgCaptureMode = 0;
#endif
    ps1BootForcedSeed = -1;
#if JC_PRINTF_PROBE_LOGS
    ps1BootPrintfTest = 0;
#endif
#if PS1_VERBOSE_DIAGNOSTICS
    ps1BootBsodAfterFirstScene = 0;
#endif
    hostForcedIslandPosValid = 0;
    hostForcedIslandX = 0;
    hostForcedIslandY = 0;
    hostForcedLowTide = -1;
    hostForcedRaftStage = -1;
    hostForcedNight = -1;
    hostHolidayMode = HOLIDAY_MODE_AUTO_ORIGINAL4;
    hostForcedHoliday = 0;
    hostBootForcedNightValid = 0;
    hostBootForcedHolidayValid = 0;
}

static int ps1CopyBootArg(int index, const char *src)
{
    if (index < 0 || index >= 3 || !src) {
        return 0;
    }

    strncpy(ps1BootArgStorage[index], src, sizeof(ps1BootArgStorage[index]) - 1);
    ps1BootArgStorage[index][sizeof(ps1BootArgStorage[index]) - 1] = '\0';
    args[index] = ps1BootArgStorage[index];
    return 1;
}

static char *ps1CopyBootString(char *dst, size_t dstSize, const char *src)
{
    if (dst == NULL || dstSize == 0 || src == NULL) {
        return NULL;
    }

    strncpy(dst, src, dstSize - 1);
    dst[dstSize - 1] = '\0';
    return dst;
}

static int ps1BuildText3(char *dst, size_t dstSize,
                         const char *a, const char *b, const char *c)
{
    size_t pos = 0;
    const char *parts[3];
    int part;

    if (dst == NULL || dstSize == 0)
        return 0;

    parts[0] = a ? a : "";
    parts[1] = b ? b : "";
    parts[2] = c ? c : "";
    for (part = 0; part < 3; part++) {
        const char *s = parts[part];
        while (*s != '\0') {
            if (pos + 1 >= dstSize) {
                dst[0] = '\0';
                return 0;
            }
            dst[pos++] = *s++;
        }
    }
    dst[pos] = '\0';
    return 1;
}

static void ps1RememberBootOverride(const char *source, const char *buffer)
{
    ps1CopyBootString(ps1BootOverrideSource,
                      sizeof(ps1BootOverrideSource),
                      source ? source : "?");
    ps1CopyBootString(ps1BootOverrideText,
                      sizeof(ps1BootOverrideText),
                      buffer ? buffer : "");
    for (int i = 0; ps1BootOverrideText[i] != '\0'; i++) {
        if (ps1IsSpace(ps1BootOverrideText[i])) {
            ps1BootOverrideText[i] = ' ';
        }
    }
}

static int ps1BootStringHasToken(const char *text, const char *token)
{
    size_t tokenLen;
    const char *cursor;

    if (!text || !text[0] || !token)
        return 0;

    tokenLen = strlen(token);
    cursor = text;
    while (*cursor) {
        const char *start;
        size_t len;

        while (*cursor && ps1IsSpace(*cursor))
            cursor++;
        start = cursor;
        while (*cursor && !ps1IsSpace(*cursor))
            cursor++;

        len = (size_t)(cursor - start);
        if (len == tokenLen && !strncmp(start, token, tokenLen))
            return 1;
    }

    return 0;
}

static int ps1BootOverrideHasToken(const char *token)
{
    return ps1BootStringHasToken(ps1BootOverrideText, token);
}

static int ps1IsFgPilotOptionToken(const char *token)
{
    if (token == NULL)
        return 0;

#if PS1_VERBOSE_DIAGNOSTICS
    if (!strcmp(token, "capture-overlay-mask") ||
        !strcmp(token, "capture-overlay") ||
        !strcmp(token, "capture-meta-dir") ||
        !strcmp(token, "capture-range") ||
        !strcmp(token, "capture-interval") ||
        !strcmp(token, "capture-scene-label") ||
        !strcmp(token, "bsod-test") ||
        !strcmp(token, "bsod-ui-test-mem-boot") ||
        !strcmp(token, "bsod-ui-test-mem-cache") ||
        !strcmp(token, "bsod-ui-test-mem-transient") ||
        !strcmp(token, "heap-probe"))
        return 1;
#endif
#if JC_PRINTF_PROBE_LOGS
    if (!strcmp(token, "printf-test") || !strcmp(token, "logtest"))
        return 1;
#endif

    return !strcmp(token, "fgoverlay") ||
           !strcmp(token, "island-pos") ||
           !strcmp(token, "lowtide") ||
           !strcmp(token, "raft-stage") ||
           !strcmp(token, "night") ||
           !strcmp(token, "holiday") ||
           !strcmp(token, "noloop") ||
           !strcmp(token, "loading-waves") ||
           !strcmp(token, "load-waves") ||
           !strcmp(token, "async-load-waves") ||
           !strcmp(token, "loading-waves-off") ||
           !strcmp(token, "no-loading-waves") ||
           !strcmp(token, "prefetch-off") ||
           !strcmp(token, "no-prefetch") ||
           !strcmp(token, "prefetch-stage1") ||
           !strcmp(token, "stage1") ||
           !strcmp(token, "prefetch-stage1-off") ||
           !strcmp(token, "no-stage1") ||
           !strcmp(token, "prefetch-window32") ||
           !strcmp(token, "window32") ||
           !strcmp(token, "prefetch-window48") ||
           !strcmp(token, "window48") ||
           !strcmp(token, "prefetch-window64") ||
           !strcmp(token, "window64") ||
           !strcmp(token, "prefetch-window") ||
           !strcmp(token, "spu-cache-test") ||
           !strcmp(token, "spu-cache-proof") ||
           !strcmp(token, "spu-stage") ||
           !strcmp(token, "no-spu-stage") ||
           !strcmp(token, "perf-log") ||
           !strcmp(token, "perf") ||
           !strcmp(token, "perf-detail") ||
           !strcmp(token, "perf-debug") ||
           !strcmp(token, "freeplay-log") ||
           !strcmp(token, "freeplay-detail") ||
           !strcmp(token, "freeplay-debug") ||
#if PS1_VERBOSE_DIAGNOSTICS
           !strcmp(token, "pad-diag") ||
           !strcmp(token, "pad-debug") ||
#endif
           !strcmp(token, "pad-script") ||
           !strcmp(token, "pad-script-log") ||
           !strcmp(token, "seed");
}

static int ps1ApplyBootOverride(char *buffer, const char *source)
{
    char *tokens[32];
    int tokenCount = 0;
    char *cursor = buffer;
    int tokenBase = 0;

    ps1RememberBootOverride(source, buffer);
    if (ps1BootOverrideHasToken("spu-cache-test"))
        ps1BootSpuCacheTest = 1;
    if (ps1BootOverrideHasToken("spu-cache-proof"))
        ps1BootSpuCacheProof = 1;

#if JC_BOOT_DIAG_LOGS
    /* JCBOOT diag: print the entire buffer so we can confirm which
     * boot string the runtime actually received. */
    printf("JCBOOT applyBootOverride buffer=[%s] len_bytes_first=%d %d %d %d %d %d %d %d\n",
           buffer ? buffer : "(null)",
           buffer ? buffer[0] : -1, buffer ? buffer[1] : -1,
           buffer ? buffer[2] : -1, buffer ? buffer[3] : -1,
           buffer ? buffer[4] : -1, buffer ? buffer[5] : -1,
           buffer ? buffer[6] : -1, buffer ? buffer[7] : -1);
#endif

    while (*cursor && tokenCount < (int)(sizeof(tokens) / sizeof(tokens[0]))) {
        while (*cursor && ps1IsSpace(*cursor)) {
            cursor++;
        }

        if (*cursor == '\0' || *cursor == '#') {
            break;
        }

        tokens[tokenCount++] = cursor;

        while (*cursor && !ps1IsSpace(*cursor) && *cursor != '#') {
            cursor++;
        }

        if (*cursor == '#') {
            *cursor = '\0';
            break;
        }

        if (*cursor == '\0') {
            break;
        }

        *cursor = '\0';
        cursor++;
    }

    if (tokenCount == 0) {
        return 0;
    }

    /* Scan for trailing "seed N" parameter anywhere in the token list */
    for (int i = 0; i + 1 < tokenCount; i++) {
        if (!strcmp(tokens[i], "seed")) {
            ps1BootForcedSeed = atoi(tokens[i + 1]);
            break;
        }
    }

    for (int i = 0; i < tokenCount; i++) {
        if (0) {
            /* Anchor the optional diagnostic-only else-if clauses below. */
        }
#if PS1_VERBOSE_DIAGNOSTICS
        else if (!strcmp(tokens[i], "capture-overlay-mask")) {
            grCaptureOverlay = 1;
            grCaptureOverlayMaskOnly = 1;
            ps1BootDbgCaptureMode = 2;
        } else if (!strcmp(tokens[i], "capture-overlay")) {
            grCaptureOverlay = 1;
            if (ps1BootDbgCaptureMode == 0)
                ps1BootDbgCaptureMode = 1;
        }
#endif
        else if (!strcmp(tokens[i], "fgoverlay") && (i + 1) < tokenCount) {
            ps1CopyBootString(
                ps1BootForegroundOverlayScene,
                sizeof(ps1BootForegroundOverlayScene),
                tokens[i + 1]
            );
            i++;
#if PS1_VERBOSE_DIAGNOSTICS
        } else if (!strcmp(tokens[i], "capture-meta-dir") && (i + 1) < tokenCount) {
            grCaptureMetaDir = ps1CopyBootString(
                ps1BootCaptureMetaDirStorage,
                sizeof(ps1BootCaptureMetaDirStorage),
                tokens[i + 1]
            );
            i++;
        } else if (!strcmp(tokens[i], "capture-range") && (i + 2) < tokenCount) {
            grCaptureStartFrame = atoi(tokens[i + 1]);
            grCaptureEndFrame = atoi(tokens[i + 2]);
            i += 2;
        } else if (!strcmp(tokens[i], "capture-interval") && (i + 1) < tokenCount) {
            grCaptureInterval = atoi(tokens[i + 1]);
            i++;
        } else if (!strcmp(tokens[i], "capture-scene-label") && (i + 1) < tokenCount) {
            grCaptureSetSceneLabel(ps1CopyBootString(
                ps1BootCaptureSceneLabelStorage,
                sizeof(ps1BootCaptureSceneLabelStorage),
                tokens[i + 1]
            ));
            i++;
#endif
        } else if (!strcmp(tokens[i], "island-pos") && (i + 2) < tokenCount) {
            hostForcedIslandX = atoi(tokens[i + 1]);
            hostForcedIslandY = atoi(tokens[i + 2]);
            hostForcedIslandPosValid = 1;
            i += 2;
        } else if (!strcmp(tokens[i], "lowtide") && (i + 1) < tokenCount) {
            hostForcedLowTide = atoi(tokens[i + 1]) ? 1 : 0;
            i++;
        } else if (!strcmp(tokens[i], "raft-stage") && (i + 1) < tokenCount) {
            hostForcedRaftStage = atoi(tokens[i + 1]);
            i++;
        } else if (!strcmp(tokens[i], "night") && (i + 1) < tokenCount) {
            hostForcedNight = atoi(tokens[i + 1]) ? 1 : 0;
            hostBootForcedNightValid = 1;
            i++;
        } else if (!strcmp(tokens[i], "holiday") && (i + 1) < tokenCount) {
            hostForcedHoliday = atoi(tokens[i + 1]);
            if (hostForcedHoliday < 0) hostForcedHoliday = 0;
            if (hostForcedHoliday > holidayMaxId()) hostForcedHoliday = holidayMaxId();
            hostHolidayMode = holidayModeFromOverride(hostForcedHoliday);
            hostBootForcedHolidayValid = 1;
            i++;
        } else if (!strcmp(tokens[i], "noloop")) {
            screensaverLoopDisabled = 1;
#if PS1_VERBOSE_DIAGNOSTICS
        } else if (!strcmp(tokens[i], "bsod-test")) {
            /* Force a synthetic BSOD after the first scene plays so we
             * can sanity-check the fatal-error UI. */
            ps1BootBsodAfterFirstScene = 1;
        } else if (!strcmp(tokens[i], "bsod-ui-test-mem-boot")) {
            /* Plan v9 step 16: synthesize a BOOT-region memHalt to
             * QA-verify the boot-time (pre-graphics) ps1DebugError
             * panel. Fires immediately at boot. */
            extern __attribute__((noreturn))
                void memHalt(const char *, const char *);
            memHalt("(bsod-ui-test)", "synthetic BOOT region halt");
        } else if (!strcmp(tokens[i], "bsod-ui-test-mem-cache")) {
            /* Synthesize a CACHE-region halt mid-scene. Fires once
             * graphics is up, routing through JC_BSOD. */
            ps1BootBsodAfterFirstScene = 1;
            /* Reuse the same after-first-scene trigger; the JC_BSOD
             * message identifies the synthetic CACHE case. */
        } else if (!strcmp(tokens[i], "bsod-ui-test-mem-transient")) {
            /* Synthesize a TRANSIENT-region halt. Same trigger pattern. */
            ps1BootBsodAfterFirstScene = 1;
        } else if (!strcmp(tokens[i], "heap-probe")) {
            foregroundPilotSetHeapProbe(1);
#endif
        } else if (!strcmp(tokens[i], "loading-waves") ||
                   !strcmp(tokens[i], "load-waves") ||
                   !strcmp(tokens[i], "async-load-waves")) {
            foregroundPilotSetLoadingWaveProof(1);
        } else if (!strcmp(tokens[i], "loading-waves-off") ||
                   !strcmp(tokens[i], "no-loading-waves")) {
            foregroundPilotSetLoadingWaveProof(0);
        } else if (!strcmp(tokens[i], "prefetch-off") || !strcmp(tokens[i], "no-prefetch")) {
            foregroundPilotSetPrefetchStage1(0);
            foregroundPilotSetPrefetchWindow(0);
        } else if (!strcmp(tokens[i], "prefetch-stage1") || !strcmp(tokens[i], "stage1")) {
            foregroundPilotSetPrefetchStage1(1);
        } else if (!strcmp(tokens[i], "prefetch-stage1-off") || !strcmp(tokens[i], "no-stage1")) {
            foregroundPilotSetPrefetchStage1(0);
        } else if (!strcmp(tokens[i], "prefetch-window32") || !strcmp(tokens[i], "window32")) {
            foregroundPilotSetPrefetchWindow(32UL * 1024UL);
        } else if (!strcmp(tokens[i], "prefetch-window48") || !strcmp(tokens[i], "window48")) {
            foregroundPilotSetPrefetchWindow(48UL * 1024UL);
        } else if (!strcmp(tokens[i], "prefetch-window64") || !strcmp(tokens[i], "window64")) {
            foregroundPilotSetPrefetchWindow(64UL * 1024UL);
        } else if (!strcmp(tokens[i], "prefetch-window") && (i + 1) < tokenCount) {
            foregroundPilotSetPrefetchWindow((unsigned long)atoi(tokens[i + 1]));
            i++;
        } else if (!strcmp(tokens[i], "spu-cache-test")) {
            ps1BootSpuCacheTest = 1;
        } else if (!strcmp(tokens[i], "spu-cache-proof")) {
            ps1BootSpuCacheProof = 1;
        } else if (!strcmp(tokens[i], "spu-stage")) {
            foregroundPilotSetSpuStage(1);
        } else if (!strcmp(tokens[i], "no-spu-stage")) {
            foregroundPilotSetSpuStage(0);
        } else if (!strcmp(tokens[i], "perf-log") || !strcmp(tokens[i], "perf")) {
            ps1PerfSetLevel(PS1_PERF_LEVEL_SUMMARY);
        } else if (!strcmp(tokens[i], "perf-detail")) {
            ps1PerfSetLevel(PS1_PERF_LEVEL_DETAIL);
        } else if (!strcmp(tokens[i], "perf-debug")) {
            ps1PerfSetLevel(PS1_PERF_LEVEL_DEBUG);
        } else if (!strcmp(tokens[i], "freeplay-log")) {
            freeplaySetTelemetryLevel(1);
        } else if (!strcmp(tokens[i], "freeplay-detail")) {
            freeplaySetTelemetryLevel(2);
        } else if (!strcmp(tokens[i], "freeplay-debug")) {
            freeplaySetTelemetryLevel(3);
#if PS1_VERBOSE_DIAGNOSTICS
        } else if (!strcmp(tokens[i], "pad-diag") || !strcmp(tokens[i], "pad-debug")) {
            eventsSetPadDiagnostics(1);
#endif
        } else if (!strcmp(tokens[i], "pad-script")) {
            ps1PadScriptConfigureFromEmbedded(1, 0);
        } else if (!strcmp(tokens[i], "pad-script-log")) {
            ps1PadScriptConfigureFromEmbedded(1, 1);
#if JC_PRINTF_PROBE_LOGS
        } else if (!strcmp(tokens[i], "printf-test") || !strcmp(tokens[i], "logtest")) {
            ps1BootPrintfTest = 1;
#endif
        }
#if PS1_VERBOSE_DIAGNOSTICS
        else if (!strcmp(tokens[i], "padtest")) {
            /* Minimal pad-input sanity test. Runs an infinite loop that
             * paints the background red on Start, green on Cross, blue on
             * any other button, and black when nothing's pressed. Logs
             * every button-bit change to TTY. Bypasses scene runtime,
             * pause menu, ps1_perf, and everything else.
             *
             * This is the option-2 cheap test from the pad-debug planning:
             * if Start works HERE in a clean PSn00bSDK environment, the
             * issue is environment-specific to the scene runtime. If
             * Start STILL doesn't work, the BIOS pad system is broken
             * for our specific PSn00bSDK + DuckStation combination and
             * we need direct SPI polling. */
            DISPENV padDisp;
            DRAWENV padDraw;
            uint16 padPrevBtn = 0xFFFF;
            uint32 padFrame = 0;

            ResetGraph(0);
            SetVideoMode(MODE_NTSC);
            SetDefDispEnv(&padDisp, 0, 0, 320, 240);
            SetDefDrawEnv(&padDraw, 0, 0, 320, 240);
            padDraw.isbg = 1;
            setRGB0(&padDraw, 0, 0, 0);
            PutDispEnv(&padDisp);
            PutDrawEnv(&padDraw);
            SetDispMask(1);

            InitPAD(pad_buff[0], 34, pad_buff[1], 34);
            StartPAD();
            ChangeClearPAD(0);

            printf("PADTEST init done. Watching pad_buff[0] forever.\n");
            printf("PADTEST PAD_START=0x%04x PAD_CROSS=0x%04x\n",
                   PAD_START, PAD_CROSS);

            for (;;) {
                PADTYPE *pad = (PADTYPE*)pad_buff[0];
                uint16 cur = pad->btn;
                uint16 inv = (uint16)~cur;
                uint8 r = 0, g = 0, b = 0;

                if (inv & PAD_START)      { r = 255; }       /* red */
                else if (inv & PAD_CROSS) { g = 255; }       /* green */
                else if (inv != 0)        { b = 255; }       /* blue (any) */

                if (cur != padPrevBtn) {
                    printf("PADTEST frame=%lu btn %04x -> %04x inv=%04x stat=%02x type=%02x\n",
                           (unsigned long)padFrame, padPrevBtn, cur, inv,
                           pad->stat, pad->type);
                    padPrevBtn = cur;
                }

                /* Repaint background each frame so color reflects state. */
                setRGB0(&padDraw, r, g, b);
                PutDrawEnv(&padDraw);

                if ((padFrame % 60) == 0) {
                    uint8 *raw = pad_buff[0];
                    printf("PADTEST tick frame=%lu raw0=%02x %02x %02x %02x %02x %02x %02x %02x\n",
                           (unsigned long)padFrame,
                           raw[0], raw[1], raw[2], raw[3],
                           raw[4], raw[5], raw[6], raw[7]);
                }

                VSync(0);
                padFrame++;
            }
            /* unreachable */
        }
#endif
    }

    if (!strcmp(tokens[0], "island")) {
        tokenBase = 1;
    }

    if (tokenBase >= tokenCount) {
        return 1;
    }

    if (!strcmp(tokens[tokenBase], "fgpilot")) {
        if ((tokenBase + 1) < tokenCount &&
            !ps1IsFgPilotOptionToken(tokens[tokenBase + 1]) &&
            ps1CopyBootArg(0, tokens[tokenBase + 1]))
            numArgs = 1;
        argForegroundPilot = 1;
        return 1;
    }

    return 1;
}

static void ps1LoadBootOverride(void)
{
    PS1File *file;
    char buffer[512];
    size_t readCount = 0;
    uint32 rawSize = 0;
    uint8 *rawData;

    ps1ResetBootArgs();

    file = ps1_fopen(PS1_BOOT_OVERRIDE_FILE, "rb");
    if (file != NULL) {
        readCount = ps1_fread(buffer, 1, sizeof(buffer) - 1, file);
        ps1_fclose(file);
        if (readCount > 0) {
            buffer[readCount] = '\0';
            if (ps1ApplyBootOverride(buffer, "file")) {
                return;
            }
        }
    }

    rawData = ps1_loadRawFile("\\BOOTMODE.TXT;1", &rawSize);
    if (rawData != NULL) {
        readCount = (rawSize < (sizeof(buffer) - 1)) ? rawSize : (sizeof(buffer) - 1);
        memcpy(buffer, rawData, readCount);
        free(rawData);
        buffer[readCount] = '\0';
        if (ps1ApplyBootOverride(buffer, "raw")) {
            return;
        }
    }

    if (PS1_EMBEDDED_BOOT_OVERRIDE[0] != '\0') {
        strncpy(buffer, PS1_EMBEDDED_BOOT_OVERRIDE, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
        ps1ApplyBootOverride(buffer, "embedded");
        return;
    }
}

#if JC_BOOT_DIAG_LOGS
static void ps1LogBootSelection(const char *explicitScene)
{
    printf(
        "JCBOOT source=%s fgpilot=%d scene=%s args=%d seed=%d loop=%d lowtide=%d night=%d holiday=%d raft=%d pos=%d,%d text=%s\n",
        ps1BootOverrideSource[0] ? ps1BootOverrideSource : "none",
        argForegroundPilot,
        explicitScene ? explicitScene : "-",
        numArgs,
        ps1BootForcedSeed,
        screensaverLoopDisabled ? 0 : 1,
        hostForcedLowTide,
        hostForcedNight,
        hostForcedHoliday,
        hostForcedRaftStage,
        hostForcedIslandX,
        hostForcedIslandY,
        ps1BootOverrideText[0] ? ps1BootOverrideText : "-"
    );
}
#else
#define ps1LogBootSelection(explicitScene) ((void)0)
#endif

#if JC_PRINTF_PROBE_LOGS
static void ps1PrintfProbe(const char *phase, const char *sceneName)
{
    if (!ps1BootPrintfTest) {
        return;
    }

    printf(
        "JCLOG phase=%s scene=%s fgpilot=%d seed=%d lowtide=%d night=%d holiday=%d raft=%d pos=%d,%d loop=%d\n",
        phase ? phase : "?",
        sceneName ? sceneName : "?",
        argForegroundPilot,
        ps1BootForcedSeed,
        islandState.lowTide,
        islandState.night,
        islandState.holiday,
        islandState.raft,
        islandState.xPos,
        islandState.yPos,
        screensaverLoopDisabled
    );
}
#else
#define ps1PrintfProbe(phase, sceneName) ((void)0)
#endif

/* Load and display title screen from raw file on CD */
/* This runs BEFORE resource parsing for instant visual feedback */
static void initTitleDisplayEarly(void)
{
    /* Initialize graphics for 640x480 interlaced */
    ResetGraph(0);
    SetVideoMode(MODE_NTSC);
    grGpuAlreadyInitialized = 1;

    /* Set up display environment for 640x480 */
    DISPENV disp;
    DRAWENV draw;
    SetDefDispEnv(&disp, 0, 0, 640, 480);
    SetDefDrawEnv(&draw, 0, 0, 640, 480);
    disp.isinter = 1;  /* Interlaced mode */
    draw.isbg = 0;     /* Don't clear - we'll load image directly */
    PutDispEnv(&disp);
    PutDrawEnv(&draw);

    /* Enable display */
    SetDispMask(1);
}

static void loadTitleScreenEarly(void)
{
    initTitleDisplayEarly();

    /* Allocate buffer for full title screen (640x480 x 2 bytes = 614400) */
    int totalBytes = 640 * 480 * 2;  /* 614400 bytes */
    uint8 *screenBuffer = (uint8*)malloc(totalBytes);
    if (!screenBuffer) {
        return;  /* Can't show title, continue anyway */
    }

    /* Load TITLE.RAW using direct CD calls */
    CdlFILE fileInfo;
    if (!ps1_cdSearchFileQuiesced(&fileInfo, "\\TITLE.RAW;1")) {
        free(screenBuffer);
        return;  /* File not found, continue anyway */
    }

    /* Calculate sectors needed (2048 bytes per sector) */
    int totalSectors = (totalBytes + 2047) / 2048;

    /* Seek to file location */
    CdControl(CdlSetloc, (uint8*)&fileInfo.pos, 0);

    /* Read data */
    CdRead(totalSectors, (uint32*)screenBuffer, CdlModeSpeed);
    CdReadSync(0, 0);

    /* Upload to framebuffer in strips (GPU DMA works better with smaller chunks) */
    int stripHeight = 60;
    int numStrips = 480 / stripHeight;  /* 8 strips */

    for (int strip = 0; strip < numStrips; strip++) {
        int yOffset = strip * stripHeight;
        uint8 *stripData = screenBuffer + (yOffset * 640 * 2);

        RECT rect;
        setRECT(&rect, 0, yOffset, 640, stripHeight);
        LoadImage(&rect, (uint32*)stripData);
        DrawSync(0);
    }

    free(screenBuffer);

    /* Reset CD state for subsequent resource loading */
    /* This ensures ps1_fopen works correctly after direct CD calls */
    cdromResetState();

}


#endif

#ifndef PS1_BUILD
static void usage()
{
        printf("\n");
        printf(" Usage :\n");
        printf("         jc_reborn\n");
        printf("         jc_reborn help\n");
        printf("         jc_reborn version\n");
        printf("         jc_reborn dump\n");
        printf("         jc_reborn [<options>] bench\n");
        printf("         jc_reborn [<options>] ttm <TTM name>\n");
        printf("         jc_reborn [<options>] ads <ADS name> <ADS tag no>\n");
        printf("         jc_reborn [<options>] fgpilot <scene>\n");
        printf("\n");
        printf(" Available options are:\n");
        printf("         window          - play in windowed mode\n");
        printf("         nosound         - quiet mode\n");
        printf("         island          - display the island as background for ADS play\n");
        printf("         debug           - print some debug info on stdout\n");
        printf("         hotkeys         - enable hot keys\n");
        printf("         capture-frame N - capture frame N to file (for visual testing)\n");
        printf("         capture-output FILE - specify output file for captured frame\n");
        printf("         capture-dir DIR - capture a frame sequence into DIR/frame_XXXXX.bmp\n");
        printf("         capture-meta-dir DIR - emit per-frame sprite metadata JSON into DIR\n");
        printf("         capture-range START END - capture inclusive frame range; END=-1 means until exit\n");
        printf("         capture-interval N - capture every Nth frame in the active range\n");
        printf("         capture-overlay - embed a machine-readable debug overlay in captures\n");
        printf("         capture-overlay-mask - draw overlay background only for paired baseline captures\n");
        printf("         capture-foreground-only - capture composited non-background layers over magenta key\n");
        printf("         capture-foreground-include-static-base - include current base-BMP ledger draws in foreground-only captures\n");
        printf("         capture-foreground-skip-visibility-mask - replay current foreground ledger without final-frame masking\n");
        printf("         noloop          - disable the fgpilot screensaver loop (single-shot play)\n");
        printf("         loading-waves   - PS1-only proof: animate waves during FG2 setup CD waits\n");
        printf("         FG2 prefetch defaults to stage1 + 32KB stream window\n");
        printf("         prefetch-window32|48|64 or prefetch-window BYTES - override FG2 stream window size\n");
        printf("         no-prefetch      - disable FG2 prefetch for diagnostics\n");
        printf("         capture-sound-events FILE - append {frame,sample} JSONL for every PLAY_SAMPLE opcode\n");
        printf("         capture-scene-label TEXT - annotate metadata with the scene label\n");
        printf("         seed N          - force deterministic RNG seed for host runs\n");
        printf("         story-day N     - force story day 1..11 for host story runs\n");
        printf("         island-pos X Y  - force island position for host story/island runs\n");
        printf("         lowtide 0|1     - force low tide state for host story/island runs\n");
        printf("         raft-stage N    - force raft stage 0..5 for host story/island runs\n");
        printf("         scene-offset X Y - force thread-layer scene offset for host story runs\n");
        printf("         capture-prelude-frame - capture one establishing frame before forced non-final story scenes\n");
        printf("\n");
        printf(" While-playing hot-keys (if enabled):\n");
        printf("         Esc        - Terminate immediately\n");
        printf("         Alt+Return - Toggle full screen / windowed mode\n");
        printf("         Space      - Toggle pause / unpause\n");
        printf("         Return     - When paused, advance one frame\n");
        printf("         <M>        - toggle max / normal speed\n");
        printf("\n");
        exit(1);
}


static void version()
{
        printf("\n");
        printf("    Johnny Reborn, an open-source engine for\n");
        printf("    the classic Johnny Castaway screensaver by Sierra.\n");
        printf("    Development version Copyright (C) 2019 Jeremie GUILLAUME\n");
        printf("\n");
        exit(1);
}


static void parseArgs(int argc, char **argv)
{
    int numExpectedArgs = 0;

    for (int i=1; i < argc; i++) {

        if (numExpectedArgs) {
            args[numArgs++] = argv[i];
            numExpectedArgs--;
        }
        else {
            if (!strcmp(argv[i], "help")) {
                usage();
            }
            if (!strcmp(argv[i], "version")) {
                version();
            }
            else if (!strcmp(argv[i], "dump")) {
                argDump = 1;
            }
            else if (!strcmp(argv[i], "bench")) {
                argBench = 1;
            }
            else if (!strcmp(argv[i], "story")) {
                if (i + 2 < argc && !strcmp(argv[i + 1], "single")) {
                    storySetBootSingleSceneIndex(atoi(argv[i + 2]));
                    i += 2;
                }
                else if (i + 2 < argc && !strcmp(argv[i + 1], "direct")) {
                    hostBootDirectSceneIndex = atoi(argv[i + 2]);
                    i += 2;
                }
                else if (i + 2 < argc &&
                         (!strcmp(argv[i + 1], "scene") || !strcmp(argv[i + 1], "index"))) {
                    storySetBootSceneIndex(atoi(argv[i + 2]));
                    i += 2;
                }
                else if (i + 3 < argc && !strcmp(argv[i + 1], "ads")) {
                    storySetBootScene(argv[i + 2], (uint16)atoi(argv[i + 3]));
                    i += 3;
                }
                else {
                    fprintf(stderr, "Error: unsupported story boot form\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "ttm")) {
                argTtm = 1;
                numExpectedArgs = 1;
            }
            else if (!strcmp(argv[i], "ads")) {
                argAds = 1;
                numExpectedArgs = 2;
            }
            else if (!strcmp(argv[i], "fgpilot")) {
                argForegroundPilot = 1;
                argPlayAll = 0;
                numExpectedArgs = 1;
            }
            else if (!strcmp(argv[i], "prefetch-off") || !strcmp(argv[i], "no-prefetch")) {
                foregroundPilotSetPrefetchStage1(0);
                foregroundPilotSetPrefetchWindow(0);
            }
            else if (!strcmp(argv[i], "loading-waves") ||
                     !strcmp(argv[i], "load-waves") ||
                     !strcmp(argv[i], "async-load-waves")) {
                foregroundPilotSetLoadingWaveProof(1);
            }
            else if (!strcmp(argv[i], "loading-waves-off") ||
                     !strcmp(argv[i], "no-loading-waves")) {
                foregroundPilotSetLoadingWaveProof(0);
            }
            else if (!strcmp(argv[i], "prefetch-stage1") || !strcmp(argv[i], "stage1")) {
                foregroundPilotSetPrefetchStage1(1);
            }
            else if (!strcmp(argv[i], "prefetch-stage1-off") || !strcmp(argv[i], "no-stage1")) {
                foregroundPilotSetPrefetchStage1(0);
            }
            else if (!strcmp(argv[i], "prefetch-window32") || !strcmp(argv[i], "window32")) {
                foregroundPilotSetPrefetchWindow(32UL * 1024UL);
            }
            else if (!strcmp(argv[i], "prefetch-window48") || !strcmp(argv[i], "window48")) {
                foregroundPilotSetPrefetchWindow(48UL * 1024UL);
            }
            else if (!strcmp(argv[i], "prefetch-window64") || !strcmp(argv[i], "window64")) {
                foregroundPilotSetPrefetchWindow(64UL * 1024UL);
            }
            else if (!strcmp(argv[i], "prefetch-window")) {
                if (i + 1 < argc) {
                    foregroundPilotSetPrefetchWindow((unsigned long)atoi(argv[++i]));
                } else {
                    fprintf(stderr, "Error: prefetch-window requires a byte count\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "spu-stage")) {
                foregroundPilotSetSpuStage(1);
            }
            else if (!strcmp(argv[i], "no-spu-stage")) {
                foregroundPilotSetSpuStage(0);
            }
#ifdef PS1_BUILD
#if PS1_VERBOSE_DIAGNOSTICS
            else if (!strcmp(argv[i], "pad-diag") || !strcmp(argv[i], "pad-debug")) {
                eventsSetPadDiagnostics(1);
            }
#endif
            else if (!strcmp(argv[i], "pad-script")) {
                ps1PadScriptConfigureFromEmbedded(1, 0);
            }
            else if (!strcmp(argv[i], "pad-script-log")) {
                ps1PadScriptConfigureFromEmbedded(1, 1);
            }
#endif
            else if (!strcmp(argv[i], "window")) {
                grWindowed = 1;
            }
            else if (!strcmp(argv[i], "nosound")) {
                soundDisabled = 1;
            }
            else if (!strcmp(argv[i], "island")) {
                argIsland = 1;
            }
            else if (!strcmp(argv[i], "debug")) {
                debugMode = 1;
            }
            else if (!strcmp(argv[i], "hotkeys")) {
                evHotKeysEnabled = 1;
            }
            else if (!strcmp(argv[i], "capture-frame")) {
                if (i + 1 < argc) {
                    grCaptureFrameNumber = atoi(argv[++i]);
                    if (grCaptureFrameNumber < 0) {
                        fprintf(stderr, "Error: capture-frame must be >= 0\n");
                        usage();
                    }
                } else {
                    fprintf(stderr, "Error: capture-frame requires a frame number\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "capture-output")) {
                if (i + 1 < argc) {
                    grCaptureFilename = argv[++i];
                } else {
                    fprintf(stderr, "Error: capture-output requires a filename\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "capture-dir")) {
                if (i + 1 < argc) {
                    grCaptureDir = argv[++i];
                } else {
                    fprintf(stderr, "Error: capture-dir requires a directory\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "capture-meta-dir")) {
                if (i + 1 < argc) {
                    grCaptureMetaDir = argv[++i];
                } else {
                    fprintf(stderr, "Error: capture-meta-dir requires a directory\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "capture-range")) {
                if (i + 2 < argc) {
                    grCaptureStartFrame = atoi(argv[++i]);
                    grCaptureEndFrame = atoi(argv[++i]);
                } else {
                    fprintf(stderr, "Error: capture-range requires START and END\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "capture-interval")) {
                if (i + 1 < argc) {
                    grCaptureInterval = atoi(argv[++i]);
                    if (grCaptureInterval <= 0) {
                        fprintf(stderr, "Error: capture-interval must be > 0\n");
                        usage();
                    }
                } else {
                    fprintf(stderr, "Error: capture-interval requires N\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "capture-overlay")) {
                grCaptureOverlay = 1;
            }
            else if (!strcmp(argv[i], "capture-overlay-mask")) {
                grCaptureOverlay = 1;
                grCaptureOverlayMaskOnly = 1;
            }
            else if (!strcmp(argv[i], "capture-foreground-only")) {
                grCaptureForegroundOnly = 1;
            }
            else if (!strcmp(argv[i], "capture-foreground-include-static-base")) {
                grCaptureForegroundIncludeStaticBase = 1;
            }
            else if (!strcmp(argv[i], "capture-foreground-skip-visibility-mask")) {
                grCaptureForegroundSkipVisibilityMask = 1;
            }
            else if (!strcmp(argv[i], "noloop")) {
                screensaverLoopDisabled = 1;
            }
            else if (!strcmp(argv[i], "capture-sound-events")) {
                if (i + 1 < argc) {
                    grCaptureSoundEventsPath = argv[++i];
                } else {
                    fprintf(stderr, "Error: capture-sound-events requires a file path\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "capture-scene-label")) {
                if (i + 1 < argc) {
                    grCaptureSetSceneLabel(argv[++i]);
                } else {
                    fprintf(stderr, "Error: capture-scene-label requires text\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "seed")) {
                if (i + 1 < argc) {
                    hostForcedSeed = atoi(argv[++i]);
                } else {
                    fprintf(stderr, "Error: seed requires a value\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "story-day")) {
                if (i + 1 < argc) {
                    hostForcedStoryDay = atoi(argv[++i]);
                    if (hostForcedStoryDay < 1 || hostForcedStoryDay > 11) {
                        fprintf(stderr, "Error: story-day must be in range 1..11\n");
                        usage();
                    }
                } else {
                    fprintf(stderr, "Error: story-day requires a value\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "island-pos")) {
                if (i + 2 < argc) {
                    hostForcedIslandX = atoi(argv[++i]);
                    hostForcedIslandY = atoi(argv[++i]);
                    hostForcedIslandPosValid = 1;
                } else {
                    fprintf(stderr, "Error: island-pos requires X and Y\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "lowtide")) {
                if (i + 1 < argc) {
                    hostForcedLowTide = atoi(argv[++i]) ? 1 : 0;
                } else {
                    fprintf(stderr, "Error: lowtide requires 0 or 1\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "raft-stage")) {
                if (i + 1 < argc) {
                    hostForcedRaftStage = atoi(argv[++i]);
                    if (hostForcedRaftStage < 0 || hostForcedRaftStage > 5) {
                        fprintf(stderr, "Error: raft-stage must be in range 0..5\n");
                        usage();
                    }
                } else {
                    fprintf(stderr, "Error: raft-stage requires a value\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "night")) {
                if (i + 1 < argc) {
                    hostForcedNight = atoi(argv[++i]) ? 1 : 0;
                    hostBootForcedNightValid = 1;
                } else {
                    fprintf(stderr, "Error: night requires 0 or 1\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "holiday")) {
                if (i + 1 < argc) {
                    hostForcedHoliday = atoi(argv[++i]);
                    if (hostForcedHoliday < 0) hostForcedHoliday = 0;
                    if (hostForcedHoliday > holidayMaxId())
                        hostForcedHoliday = holidayMaxId();
                    hostHolidayMode = holidayModeFromOverride(hostForcedHoliday);
                    hostBootForcedHolidayValid = 1;
                } else {
                    fprintf(stderr, "Error: holiday requires a value 0..%d\n",
                            holidayMaxId());
                    usage();
                }
            }
            else if (!strcmp(argv[i], "scene-offset")) {
                if (i + 2 < argc) {
                    hostForcedSceneOffsetX = atoi(argv[++i]);
                    hostForcedSceneOffsetY = atoi(argv[++i]);
                    hostForcedSceneOffsetValid = 1;
                } else {
                    fprintf(stderr, "Error: scene-offset requires X and Y\n");
                    usage();
                }
            }
            else if (!strcmp(argv[i], "capture-prelude-frame")) {
                hostCapturePreludeFrame = 1;
            }
        }
    }

    if (numExpectedArgs)
        usage();

    if (argDump + argBench + argTtm + argAds + argForegroundPilot > 1)
        usage();

    if (argDump + argBench + argTtm + argAds + argForegroundPilot == 0)
        argPlayAll = 1;
}
#endif


int main(int argc, char **argv)
{
#ifdef PS1_BUILD
    /* Initialize debug system FIRST, before any CD operations */
    /* FntLoad must happen before CdInit or it causes hangs */
    ps1DebugInit();

    /* memInit is DEFERRED until after parseResourceFiles. The region
     * buffer is 1.2 MB of libc malloc, but parseResourceFiles loads
     * RESOURCE.001 (~1.1 MB) into a temporary libc buffer first.
     * Doing memInit() here would leave libc with insufficient heap
     * for the catalog. The region buffer is allocated AFTER the
     * catalog is parsed and its temp buffer returned to libc.
     *
     * Migrated call sites that run BEFORE memInit must use libc
     * malloc — currently that's just safe_malloc (libc-backed). */

    /* Initialize CD-ROM subsystem */
    if (cdromInit() < 0) {
        ps1DebugError("CD-ROM init failed!");
        while(1);
    }

    debugMode = 0;  /* Keep PS1 debug chatter opt-in; use BOOTMODE probes for logs. */

    /* Load boot override BEFORE seeding RNG so "seed N" can override. */
    ps1LoadBootOverride();
    if (!argForegroundPilot) {
        argForegroundPilot = 1;
    }
    ps1LogBootSelection(
        (ps1BootForegroundOverlayScene[0] != '\0')
            ? ps1BootForegroundOverlayScene
            : ((numArgs >= 1) ? args[0] : NULL)
    );
    ps1PrintfProbe("boot-override-loaded", NULL);

    loadTitleScreenEarly();
    ps1PrintfProbe("title-loaded", NULL);

    /* Parse resource files from CD - needed for background and sprites */
    parseResourceFiles("RESOURCE.MAP");
    ps1PrintfProbe("resources-loaded", NULL);

    /* Now that parseResourceFiles has closed RESOURCE.001 and returned
     * its temp buffer to libc, libc has the headroom for the
     * memory-region allocator's 1.2 MB buffer. Initialize it and run
     * the boot proofs. Per plan v9 boot sequence (revised).
     *
     * Boot-time memVerify* failures route through memHalt; ps1DebugInit
     * has already run so the pre-graphics path renders correctly. */
    memInit();
    memVerifyBootBudget();
    memVerifyAllScenesFitTransient();
    memVerifyAllScenesPinnedFitCache();
#ifdef JC_VERIFY_PACK_HASHES
    memVerifyPackHashes();
#endif
    ps1PrintfProbe("mem-region-ready", NULL);

    /* Seed RNG — use forced seed if specified in BOOTMODE, else hardware RNG. */
    if (ps1BootForcedSeed >= 0) {
        srand((unsigned int)ps1BootForcedSeed);
    } else {
        ps1SeedRandom();
    }
#else
    /* Non-PS1: normal flow */
    parseArgs(argc, argv);

    if (argDump)
        debugMode = 1;

    parseResourceFiles("RESOURCE.MAP");

    storySetForcedCurrentDay(hostForcedStoryDay);
    storySetIslandOverrides(
        hostForcedIslandPosValid,
        hostForcedIslandX,
        hostForcedIslandY,
        hostForcedLowTide >= 0,
        hostForcedLowTide,
        hostForcedRaftStage >= 0,
        hostForcedRaftStage
    );
    storySetSceneOffsetOverride(
        hostForcedSceneOffsetValid,
        hostForcedSceneOffsetX,
        hostForcedSceneOffsetY
    );
    storySetCapturePreludeFrame(hostCapturePreludeFrame);
    if (argForegroundPilot && numArgs >= 1)
        foregroundPilotSetScene(args[0]);
    else
        foregroundPilotSetScene(NULL);

    if (hostForcedSeed >= 0)
        srand((unsigned int)hostForcedSeed);
    else
        srand((unsigned int)time(NULL));
#endif

#ifdef PS1_BUILD
    /* Resource counts available via extern */
    extern int numPalResources;
    extern struct TPalResource *palResources[];
#endif

    /* Initialize LRU cache for memory management */
    initLRUCache();

#ifdef PS1_BUILD
    if (ps1BootForegroundOverlayScene[0] != '\0')
        foregroundPilotSetScene(ps1BootForegroundOverlayScene);
    else if (argForegroundPilot && numArgs >= 1)
        foregroundPilotSetScene(args[0]);
    else
        foregroundPilotSetScene(NULL);

    graphicsInit();
    ps1PrintfProbe("graphics-init", NULL);
    /* Keep FG2 stream buffers lazy. A boot-time 192 KB frame pin plus
     * 64 KB scratch pin starves large clean-rect scenes like WALKSTUF1 low
     * after the static CD sector pool is reserved. The runtime can still
     * grow and retain these buffers per scene, and can drop prefetch when
     * clean-background allocation needs the heap. */
    int bootNightValid = hostBootForcedNightValid;
    int bootHolidayValid = hostBootForcedHolidayValid;
    int bootNight = hostForcedNight;
    int bootHoliday = hostForcedHoliday;
    int bootHolidayMode = hostHolidayMode;
    int memcardSettingsLoaded = 0;
    int memcardRequestedMute = 0;

    pauseMenuInit();

    /* Restore settings before SPU init so saved mute/ocean state can prevent
     * boot-time ambience from keying on. Explicit BOOTMODE parameters are
     * launch-time intent and must win over saved defaults. */
    memcardSettingsLoaded = memcardLoadSettings();
    memcardRequestedMute = soundMuted;
    soundInit();
    if (memcardSettingsLoaded && memcardRequestedMute) {
        soundMuted = 0;
        soundMuteToggle();  /* apply saved mute to SPU registers */
    }
    if (ps1BootSpuCacheTest || ps1BootSpuCacheProof ||
        ps1BootOverrideHasToken("spu-cache-test") ||
        ps1BootOverrideHasToken("spu-cache-proof") ||
        ps1BootStringHasToken(PS1_EMBEDDED_BOOT_OVERRIDE, "spu-cache-test") ||
        ps1BootStringHasToken(PS1_EMBEDDED_BOOT_OVERRIDE, "spu-cache-proof")) {
        int cacheOk = ps1SpuCacheSelfTest();
        if (ps1BootSpuCacheProof ||
            ps1BootOverrideHasToken("spu-cache-proof") ||
            ps1BootStringHasToken(PS1_EMBEDDED_BOOT_OVERRIDE, "spu-cache-proof"))
            ps1SpuCacheProofHalt(cacheOk);
    }
    ps1PrintfProbe("sound-init", NULL);
    if (bootNightValid)
        hostForcedNight = bootNight;
    if (bootHolidayValid) {
        hostForcedHoliday = bootHoliday;
        hostHolidayMode = bootHolidayMode;
    }

    if (numPalResources > 0 && palResources[0]) {
        grLoadPalette(palResources[0]);
    }

    if (hostForcedLowTide >= 0)
        islandState.lowTide = hostForcedLowTide;
    if (hostForcedRaftStage >= 0)
        islandState.raft = hostForcedRaftStage;
    if (hostForcedNight >= 0)
        islandState.night = hostForcedNight;
    if (hostHolidayMode == HOLIDAY_MODE_NONE)
        islandState.holiday = 0;
    if (holidayModeIsManual(hostHolidayMode))
        islandState.holiday = hostForcedHoliday;
    if (hostForcedIslandPosValid) {
        islandState.xPos = hostForcedIslandX;
        islandState.yPos = hostForcedIslandY;
    }

    /* PS1 is now FG2-scene-playback only. Host ADS/TTM/story engines stay
     * available for capture tooling, but they are no longer linked into the
     * console executable. */
    const char *explicitScene = (ps1BootForegroundOverlayScene[0] != '\0')
                                ? ps1BootForegroundOverlayScene
                                : ((numArgs >= 1) ? args[0] : NULL);

    /* Force walk-subsystem to pre-allocate its 149 KB clean buffer
     * before memFreezeBoot. Without this, the lazy allocation inside
     * walkPilotCaptureCleanWalkAreaIfStale fires post-freeze and
     * halts. */
    {
        extern int walkPilotInit(void);
        (void)walkPilotInit();
    }

    /* Freeze the BOOT region. Activating the no-fail invariant: any
     * later memAlloc(MEM_REGION_BOOT, ...) halts via memHalt with a
     * clear "post-freeze" message. Per plan v9 step 13.
     *
     * BOOT call sites remaining at this point: primitiveBuffer[0/1]
     * (in graphicsInit, runs before this), walkCleanBuf (walkPilotInit
     * just ran), and any safe_malloc calls (still on libc after revert).
     * Graphics surface descriptors (grNewEmptyBackground, PSB/BMP
     * frames) stay on libc — they're scene-runtime, not BOOT lifetime. */
    memFreezeBoot();

    do {
        int skipWalkThisIteration = 0;

        /* Scene-set cycling — when the pause menu cycles to a new
         * set, drop any pinned scene, show the frog-clock transition,
         * and surface the new set name as a caption. The actual pool
         * is read from pauseMenuSceneSet by the picker each iteration.
         *
         * The frog-clock animation zeros bgTile* (it draws on a black
         * backdrop). That leaves the walk subsystem nothing to compose
         * against, so we treat scene-set cycling as a sequence reset:
         * forget Johnny's last spot/heading and let the next scene
         * load its bg fresh via foregroundPilotPlay. The walk is
         * skipped this iteration; future iterations resume normal
         * walk-between-scenes behaviour because the next scene will
         * update storyCurrentSpot/Hdg on completion. */
        if (pauseMenuRequestSceneSetCycle) {
            extern const char *pauseMenuSceneSetName(int idx);
            extern void pickerOnSceneSetCycle(void);
            pauseMenuRequestSceneSetCycle = 0;
            explicitScene = NULL;
            pickerOnSceneSetCycle();   /* reset Sequential cursor + repeat-prevention */
            ps1ShowFreeplayLoadingFrame("changing scene set", 0);
            {
                char banner[40];
                if (ps1BuildText3(banner, sizeof(banner), "Scene Set: ",
                                  pauseMenuSceneSetName(pauseMenuSceneSet), ""))
                    captionsShowText(banner, 300);
            }
            fgLoopForgetWalkContext();
            fgLoopSequenceJustReset = 1;
            skipWalkThisIteration = 1;
        }

        /* Scene Explorer pins — Cross plays once and reverts; Triangle
         * loops the scene every iteration until the user changes Scene
         * Set or picks a different scene from the explorer. Mirrors the
         * scene-set-cycle handler: clear Johnny's spot/heading, fire the
         * frog clock, and let foregroundPilotPlay reload the bg. The
         * sceneExplorerOneShot flag distinguishes one-shot from loop. */
        if (pauseMenuRequestPlayScene >= 0
            || pauseMenuRequestLoopScene >= 0) {
            int idx = (pauseMenuRequestPlayScene >= 0)
                          ? pauseMenuRequestPlayScene
                          : pauseMenuRequestLoopScene;
            int oneShot = (pauseMenuRequestPlayScene >= 0);
            pauseMenuRequestPlayScene = -1;
            pauseMenuRequestLoopScene = -1;
            if (idx >= 0 && idx < gSceneExplorerCount) {
                explicitScene = gSceneExplorer[idx].slug;
                sceneExplorerOneShot = oneShot;
                fgLoopForgetWalkContext();
                fgLoopSequenceJustReset = 1;
                skipWalkThisIteration = 1;
                ps1PrepareSceneExplorerLaunch();
                ps1ShowFreeplayLoadingFrame(
                    oneShot ? "now playing" : "looping", 0);
                {
                    char banner[64];
                    if (ps1BuildText3(banner, sizeof(banner),
                                      oneShot ? "Now playing: " : "Looping: ",
                                      sceneExplorerSelectedDisplayName(), ""))
                        captionsShowText(banner, 240);
                }
            }
        }

        /* Phase 8: tick the story-day calendar. Advances when the
         * ps1Soft date has rolled over since the last iteration. */
        fgLoopAdvanceStoryDayIfNeeded();

        const char *loopScene = fgLoopNextScene(explicitScene,
                                                pauseMenuSceneSet);
        fgLoopApplyVariant(loopScene);
        /* Pre-emptive CACHE eviction. Runs AFTER fgLoopApplyVariant so
         * the lookup keys on the effective scene name (variant slug, if
         * a variant fires). Runs BEFORE fgLoopWalkToScene so the
         * ~3-5 ms LRU scan hides inside the walk animation's pause.
         * See plan v9 "Pre-emptive CACHE eviction" section. */
        memCachePreEvictForNextScene(loopScene);

        /* Walk subsystem: walk Johnny from his last spot/heading to
         * this scene's start before the FG2 pack plays. fgLoopWalkToScene
         * is a no-op if either side has no defined position; the
         * scene's FG2 pack still owns the actual scene visuals. */
        const struct TStoryScene *storyScene =
            fgLoopFindStorySceneBySlug(loopScene);
        if (storyScene && (storyScene->flags & ISLAND)) {
            foregroundPilotSetSceneDrawOffset(
                islandState.xPos + ((storyScene->flags & LEFT_ISLAND) ? 272 : 0),
                islandState.yPos);
        } else {
            foregroundPilotSetSceneDrawOffset(0, 0);
        }
        int playedScene = 0;
        /* Story-day filter: scenes with non-zero dayNo only fire on
         * the matching day. None of the currently-validated scenes
         * have a dayNo > 0, so this is a no-op for the active set;
         * the gate is in place for future validations of JOHNNY 2/3,
         * MARY 1-5, SUZY 1-2 (all dayNo-gated in story_data.h). */
        if (storyScene && storyScene->dayNo > 0
            && storyScene->dayNo != storyCurrentDay) {
            /* Pick a different scene on the next iteration; for now
             * just play this one anyway since fishing1/2 are dayNo=0. */
        }

        /* Walk only when the story-sequence and the actual island backdrop
         * are both continuing. Sequence reset catches deliberate rerolls;
         * the backdrop key catches scene-specific policy changes inside the
         * same sequence (e.g. VARPOS_OK -> fixed/left-island, NORAFT, tide,
         * holiday). Without this, Johnny can draw at the new scene's walk
         * coordinates over the previous scene's island, leaving water trails. */
        if (!skipWalkThisIteration &&
            !fgLoopSequenceJustReset &&
            fgLoopWalkBackdropMatchesCurrent() &&
            !(storyScene && (storyScene->flags & FIRST))) {
            fgLoopWalkToScene(storyScene);
        } else {
            /* New sequence/backdrop or FIRST/full-wipe scene: don't walk, but
             * still clear cached walk pixels before the FG2 scene owns the
             * screen. */
            walkRenderResetCache();
        }

        if (!pauseMenuRequestNextScene &&
            !pauseMenuRequestFreeplay &&
            !pauseMenuRequestResetLoop) {
            fgWalkRenderTeardown();
            foregroundPilotSetScene(loopScene);
            ps1PerfBeginScene(loopScene);
            ps1PrintfProbe("scene-start", loopScene);
            foregroundPilotPlay();
            ps1PerfEndScene(loopScene);
            ps1PrintfProbe("scene-end", loopScene);
            playedScene = 1;
            /* Diagnostic continuity: scene-N has just finished playing
             * successfully. Flip the picker state machine so any
             * memHalt during scene-N+1's setup reports N+1, not N. */
            fgLoopMarkScenePlayed();
        }

        if (freeplayExitRequested()) {
            freeplayClearExitRequest();
            explicitScene = NULL;       /* return to random story rotation */
            fgLoopForgetWalkContext();
            fgLoopSequenceJustReset = 1;
#if JC_PAUSE_REQUEST_DIAG_LOGS
            printf("JCFREE consume freeplay-exit\n");
#endif
        }

#if PS1_VERBOSE_DIAGNOSTICS
        /* BOOTMODE bsod-test: synthesize a fatal-error after the first
         * scene completes so the BSOD UI can be verified visually. */
        if (ps1BootBsodAfterFirstScene) {
            JC_BSOD(loopScene, "bsod-test bootmode flag — synthetic fatal "
                              "after first scene to validate BSOD rendering");
        }
#endif

        /* After the scene played, update Johnny's spot/heading so the
         * next loop iteration knows where to walk from. */
        if (playedScene &&
            !pauseMenuRequestNextScene &&
            !pauseMenuRequestFreeplay &&
            !pauseMenuRequestResetLoop) {
            fgLoopUpdatePosFromScene(storyScene);
            fgLoopRememberWalkBackdrop(storyScene);
        }

        /* Scene Explorer one-shot pin: if Cross was pressed (oneShot=1),
         * clear explicitScene now so the next iteration falls back to
         * the active Scene Set's pool. Triangle (loop) leaves this 0
         * so the explicit scene keeps replaying until the user changes
         * Scene Set or re-enters the explorer. */
        if (sceneExplorerOneShot && playedScene) {
            sceneExplorerOneShot = 0;
            explicitScene = NULL;
        }

        /* Consume pause-menu requests. NextScene = let the next loop
         * iteration pick a fresh scene (already happens). ResetLoop =
         * force re-randomization by clearing any explicit scene state
         * and falling through to fgLoopNextScene's random branch. */
        if (pauseMenuRequestNextScene) {
            pauseMenuRequestNextScene = 0;
#if JC_PAUSE_REQUEST_DIAG_LOGS
            printf("JCPAUSE consume next-scene\n");
#endif
        }
        if (pauseMenuRequestFreeplay) {
            pauseMenuRequestFreeplay = 0;
            fgLoopForgetWalkContext();
            fgLoopSequenceJustReset = 1;
#if JC_PAUSE_REQUEST_DIAG_LOGS
            printf("JCPAUSE consume freeplay direct\n");
#endif
            fgWalkRenderTeardown();
            foregroundPilotTeardownForFreeplay();
            walkRenderResetCache();
            ps1ShowFreeplayLoadingFrame("building freeplay island", 0);
#if JC_PAUSE_REQUEST_DIAG_LOGS
            printf("JCPAUSE freeplay teardown done\n");
#endif
            foregroundPilotSetScene("freeplay");
            ps1PerfBeginScene("freeplay");
            freeplayRun();
            ps1PerfEndScene("freeplay");
            freeplayClearExitRequest();
            explicitScene = NULL;
        }
        if (pauseMenuRequestResetLoop) {
            pauseMenuRequestResetLoop = 0;
            explicitScene = NULL;  /* drop pinned scene → next iter random */
#if JC_PAUSE_REQUEST_DIAG_LOGS
            printf("JCPAUSE consume reset-loop\n");
#endif
        }
    } while (!screensaverLoopDisabled);

    soundEnd();
    graphicsEnd();
    return 0;
#endif

#ifndef PS1_BUILD
    if (hostBootDirectSceneIndex >= 0) {
        printf("Initializing graphics...\n");
        graphicsInit();
        printf("Graphics initialized\n");

        printf("Initializing sound...\n");
        soundInit();
        printf("Sound initialized\n");

        printf("Starting direct story scene %d...\n", hostBootDirectSceneIndex);
        storyPlayBootSceneDirect(hostBootDirectSceneIndex);

        printf("Shutting down sound...\n");
        soundEnd();
        printf("Shutting down graphics...\n");
        graphicsEnd();
        printf("Shutdown complete\n");
    }

    else if (argPlayAll) {
        printf("Initializing graphics...\n");
        graphicsInit();
        printf("Graphics initialized\n");

        printf("Initializing sound...\n");
        soundInit();
        printf("Sound initialized\n");

        printf("Starting story mode...\n");
        storyPlay();

        printf("Shutting down sound...\n");
        soundEnd();
        printf("Shutting down graphics...\n");
        graphicsEnd();
        printf("Shutdown complete\n");
    }

    else if (argDump) {
        dumpAllResources();
    }

    else if (argBench) {
        graphicsInit();
        adsPlayBench();
        graphicsEnd();
    }

    else if (argTtm) {
        graphicsInit();

#ifdef PS1_BUILD
        /* PS1: Simple render test - bypass TTM logic for now */
        printf("PS1: Starting simple render test (300 frames)...\n");

        int frame_count = 0;
        while (frame_count < 300) {  /* Run for 5 seconds at 60fps */
            grRefreshDisplay();

            frame_count++;
            if ((frame_count % 60) == 0) {
                printf("Frame %d\n", frame_count);
            }
        }
        printf("PS1: Render test complete\n");
#else
        soundInit();
        adsPlaySingleTtm(args[0], (numArgs >= 2) ? (uint16)atoi(args[1]) : 0);
        soundEnd();
#endif

        graphicsEnd();
    }

    else if (argAds) {

        graphicsInit();
        soundInit();
        adsInit();

        if (hostForcedLowTide >= 0)
            islandState.lowTide = hostForcedLowTide;
        if (hostForcedRaftStage >= 0)
            islandState.raft = hostForcedRaftStage;
        if (hostForcedIslandPosValid) {
            islandState.xPos = hostForcedIslandX;
            islandState.yPos = hostForcedIslandY;
        }

        if (argIsland)
            adsInitIsland();
        else
            adsNoIsland();

        adsPlay(args[0], atoi(args[1]));

        soundEnd();
        graphicsEnd();
    }

    else if (argForegroundPilot) {
        printf("Initializing graphics...\n");
        graphicsInit();
        printf("Graphics initialized\n");
        /* Host fgpilot path: same screensaver loop as PS1. See comment
         * on the PS1 branch above. */
        const char *explicitScene = (numArgs >= 1) ? args[0] : NULL;
        do {
            const char *loopScene = fgLoopNextScene(explicitScene, 0);
            fgLoopApplyVariant(loopScene);
            foregroundPilotSetScene(loopScene);
            foregroundPilotPlay();
        } while (!screensaverLoopDisabled);
        graphicsEnd();
        printf("Shutdown complete\n");
    }
#endif

    return 0;
}
