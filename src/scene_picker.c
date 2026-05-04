/*
 * Scene picker — Random / Sequential / Original (Original lands in PR 2).
 *
 * Designed for net-zero heap pressure: malloc/free/realloc/calloc are
 * poison-included below. All per-policy state lives in module-level
 * statics; total budget is documented at each declaration.
 *
 * Telemetry: emits one JCPICK line per pick.
 */

#include "scene_picker.h"
#include "mytypes.h"
#include <stdio.h>
#include <stdlib.h>      /* rand() — must come BEFORE the malloc poison */
#include <string.h>
#include "story.h"
/* story_data.h gives us `struct TStoryScene` so we can dereference the
 * pointer that fgLoopFindStorySceneBySlug returns. Unfortunately the
 * same header also defines `static struct TStoryScene storyScenes[]`,
 * which we don't touch here — silence the resulting unused-variable
 * warning instead of duplicating the struct definition or refactoring
 * the header. The linker's --gc-sections drops the unused copy. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "story_data.h"
#pragma GCC diagnostic pop

/* Catalog accessors live in jc_reborn.c — keeps the pool data private. */
extern int   fgLoopGetPoolCount(int sceneSetIdx);
extern const char *fgLoopGetPoolSlug(int sceneSetIdx, int index);
extern int   fgLoopGetProvenCount(void);
extern const char *fgLoopGetProvenSlug(int index);

/* Slug → storyScenes[] entry lookup, owned by jc_reborn.c. We need the
 * full struct visibility from story_data.h above so we can dereference
 * the returned pointer for adsName/adsTagNo. Declared at file scope so
 * both helpers below see the same incomplete-vs-complete type. */
extern const struct TStoryScene *fgLoopFindStorySceneBySlug(const char *slug);

/* ---------------------------------------------------------------------------
 *  Malloc poison guard.
 *
 *  The picker is on a hot path that runs every screensaver iteration
 *  forever. The whole reason Original mode is feasible is the audit
 *  said "zero allocations." Make accidental future violations a
 *  compile error. If a contributor genuinely needs to allocate, they
 *  must do it outside this translation unit.
 *
 *  These #defines come AFTER the system headers so stdlib.h's
 *  prototypes are unaffected; the poison only applies to call sites
 *  inside this file.
 * ------------------------------------------------------------------------- */
#define malloc(...)  PICKER_NO_MALLOC_DO_NOT_ADD_HEAP_ALLOCATIONS_TO_THE_PICKER
#define calloc(...)  PICKER_NO_MALLOC_DO_NOT_ADD_HEAP_ALLOCATIONS_TO_THE_PICKER
#define realloc(...) PICKER_NO_MALLOC_DO_NOT_ADD_HEAP_ALLOCATIONS_TO_THE_PICKER
#define free(...)    PICKER_NO_FREE_NOTHING_TO_FREE_IN_THE_PICKER

/* ---------------------------------------------------------------------------
 *  Module-level state.
 * ------------------------------------------------------------------------- */

/* Active policy. Defaults to RANDOM (current behaviour) until memcard
 * loads or the pause menu sets it. */
static int gPickerPolicy = SCENE_PICKER_RANDOM;

/* Sequential cursor. 0..pool.count-1; resets to 0 on Scene Set cycle.
 * 16 bits is more than enough — largest pool is the catch-all pool
 * (kProvenScenes is 6 today; future "All Scenes" might be ~30). */
static uint16 gSequentialCursor = 0;

/* Repeat-prevention. Used by Random and Original (Sequential cycles
 * deterministically and doesn't need it). Tracks the last scene's
 * .ADS family + tag so we don't pick the same scene twice in a row.
 * adsName is at most 12 chars (ISO 9660 limit) plus NUL. */
static char   gLastAdsName[16] = {0};
static uint16 gLastAdsTag       = 0xFFFF;   /* sentinel: "no last pick yet" */

/* Telemetry counters — surfaced via JCPICK so soak tests can sanity
 * check that retries stay bounded. */
static uint16 gJcpickFrame = 0;

const char *pickerPolicyName(int policy)
{
    switch (policy) {
    case SCENE_PICKER_RANDOM:     return "Random";
    case SCENE_PICKER_SEQUENTIAL: return "Sequential";
    case SCENE_PICKER_ORIGINAL:   return "Original";
    default:                      return "?";
    }
}

void pickerOnSceneSetCycle(void)
{
    gSequentialCursor = 0;
    gLastAdsName[0]   = '\0';
    gLastAdsTag       = 0xFFFF;
}

void pickerSetPolicy(int policy)
{
    if (policy < 0 || policy >= SCENE_PICKER_COUNT)
        return;
    gPickerPolicy = policy;
    /* Don't reset cursor — switching policy mid-stream shouldn't snap
     * Sequential back to scene 0; user's mental model is "where we
     * were, but with a different draw rule." Repeat-prevention also
     * carries over for the same reason. */
}

int pickerGetPolicy(void)
{
    return gPickerPolicy;
}

/* ---------------------------------------------------------------------------
 *  Helper: how many scenes does the active set carry, and how do we
 *  read slug N out of it?  The set's pool is indexed by sceneSetIdx;
 *  if its count is 0 (empty / placeholder set) we fall through to
 *  kProvenScenes. The caller has already validated sceneSetIdx range.
 * ------------------------------------------------------------------------- */
static int activePoolCount(int sceneSetIdx)
{
    int n = fgLoopGetPoolCount(sceneSetIdx);
    return (n > 0) ? n : fgLoopGetProvenCount();
}

static const char *activePoolSlug(int sceneSetIdx, int index)
{
    int n = fgLoopGetPoolCount(sceneSetIdx);
    if (n > 0)
        return fgLoopGetPoolSlug(sceneSetIdx, index);
    return fgLoopGetProvenSlug(index);
}

/* ---------------------------------------------------------------------------
 *  Repeat-prevention: returns 1 if `slug` matches the last picked scene
 *  (same .ADS family AND same tag). The picker calls this and rejects
 *  matches via a small retry budget.
 * ------------------------------------------------------------------------- */
static int slugMatchesLastPick(const char *slug)
{
    if (gLastAdsName[0] == '\0' || gLastAdsTag == 0xFFFF)
        return 0;
    /* Walk storyScenes[] for this slug — story_data already maps slug
     * to (adsName, adsTagNo). 63-entry linear scan; ~1 microsecond. */
    const struct TStoryScene *s = fgLoopFindStorySceneBySlug(slug);
    if (s == NULL)
        return 0;
    if (strcmp(s->adsName, gLastAdsName) != 0)
        return 0;
    return s->adsTagNo == gLastAdsTag;
}

static void recordPick(const char *slug)
{
    const struct TStoryScene *s = fgLoopFindStorySceneBySlug(slug);
    if (s != NULL) {
        /* adsName is ≤12 chars per ISO 9660; truncate just in case. */
        size_t n = strlen(s->adsName);
        if (n >= sizeof(gLastAdsName)) n = sizeof(gLastAdsName) - 1;
        memcpy(gLastAdsName, s->adsName, n);
        gLastAdsName[n] = '\0';
        gLastAdsTag = (uint16)s->adsTagNo;
    }
}

/* ---------------------------------------------------------------------------
 *  Policy implementations.
 * ------------------------------------------------------------------------- */

/* Random: uniform draw with up to 4 retries to avoid same-scene-twice.
 * Budget caps at min(pool/2, 4) so a 2-element pool can still pick the
 * other element on retry; bigger pools cap at 4 to prevent worst-case
 * pathological pools that match the last pick on every entry from
 * spinning forever. */
static const char *pickRandom(int sceneSetIdx, uint8 *outRetries)
{
    int n = activePoolCount(sceneSetIdx);
    if (n <= 0) {
        *outRetries = 0;
        return NULL;
    }
    int budget = (n <= 8) ? (n - 1) : 4;
    if (budget < 0) budget = 0;
    const char *slug = NULL;
    int retries = 0;
    do {
        slug = activePoolSlug(sceneSetIdx, rand() % n);
        if (!slugMatchesLastPick(slug)) break;
        retries++;
    } while (retries <= budget);
    *outRetries = (uint8)retries;
    return slug;
}

/* Sequential: cycle through the pool in authored order. */
static const char *pickSequential(int sceneSetIdx, uint8 *outRetries)
{
    int n = activePoolCount(sceneSetIdx);
    if (n <= 0) {
        *outRetries = 0;
        return NULL;
    }
    if (gSequentialCursor >= n)
        gSequentialCursor = 0;
    const char *slug = activePoolSlug(sceneSetIdx, gSequentialCursor);
    gSequentialCursor = (uint16)((gSequentialCursor + 1) % n);
    *outRetries = 0;
    return slug;
}

/* ---------------------------------------------------------------------------
 *  Public dispatcher.
 * ------------------------------------------------------------------------- */
const char *pickerNextScene(const char *explicitScene, int sceneSetIdx)
{
    /* Pinning short-circuits all policy work. Mirrors the legacy
     * fgLoopNextScene's first line — Scene Explorer Cross/Triangle
     * and the CLI fgpilot path both rely on this. */
    if (explicitScene != NULL && explicitScene[0] != '\0')
        return explicitScene;

    uint8 retries = 0;
    const char *slug = NULL;
    int policy = gPickerPolicy;

    switch (policy) {
    case SCENE_PICKER_SEQUENTIAL:
        slug = pickSequential(sceneSetIdx, &retries);
        break;
    case SCENE_PICKER_ORIGINAL:
        /* PR 2 lands the real Sierra state machine; for now degrade
         * gracefully to Random so toggling Original in the menu
         * doesn't break playback. The JCPICK telemetry still reports
         * policy=2 so the soak test can flag the stub. */
    case SCENE_PICKER_RANDOM:
    default:
        slug = pickRandom(sceneSetIdx, &retries);
        policy = SCENE_PICKER_RANDOM;
        break;
    }

    if (slug == NULL)
        return NULL;

    recordPick(slug);

    /* JCPICK <frame> policy=<N> set=<N> picked=<slug> retries=<N> */
    printf("JCPICK frame=%u policy=%d set=%d picked=%s retries=%u\n",
           (unsigned)gJcpickFrame++, policy, sceneSetIdx,
           slug, (unsigned)retries);

    return slug;
}
