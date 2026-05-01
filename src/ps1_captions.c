/*
 *  Closed-caption display system for PS1.
 *
 *  Provides subtitle text for each scene, intended for
 *  accessibility.  The caption text and the scene-to-ADS lookup
 *  table both live here (moved out of ps1_captions.h on 2026-04-26
 *  so multiple translation units don't get private copies).
 *
 *  Usage:
 *    captionsSetEnabled(1);                 // turn on
 *    captionsOnSceneStart("scene05");       // by scene ID
 *    captionsOnAdsStart("ACTIVITY", 5);     // by ADS name+tag
 *    const char *txt = captionsGetCurrent(); // each frame
 */

#include <stddef.h>
#include <string.h>

#include "ps1_captions.h"

#ifdef PS1_BUILD
#include <psxgpu.h>
#include "pause_menu.h"
#include "ps1_gpu_ot.h"
#endif

/* ------------------------------------------------------------------ */
/*  Caption text  — moved here from the header so the data lives in   */
/*  one translation unit. Special captions (intro, christmas, etc.)   */
/*  are triggered from runtime hooks; numbered captions sceneNN are   */
/*  reached through captionSceneMap[] below.                           */
/* ------------------------------------------------------------------ */

static const struct TCaption captions[] = {

    /* --- Special / environmental captions --- */

    {"intro",
        "This is Johnny Castaway.\n"
        "Stranded on a tiny island\n"
        "with one palm tree.\n"
        "He wears white shorts and a hat."},

    {"christmas",
        "It is Christmas.\n"
        "A small tree with red bulbs\n"
        "and a golden star."},

    {"halloween",
        "It is Halloween.\n"
        "A carved jack-o-lantern\n"
        "sits on the island."},

    {"newyears",
        "It is New Years.\n"
        "A banner reads Happy New Year\n"
        "on the palm tree."},

    {"stpatrick",
        "It is St Patrick's Day.\n"
        "Four-leaf clovers grow\n"
        "on the island."},

    {"night",
        "It is night.\n"
        "The island is bathed\n"
        "in moonlight."},

    {"day",
        "It is day.\n"
        "The sun shines brightly."},

    {"regularday",
        "It is a regular day."},

    {"hightide",
        "It is high tide.\n"
        "Waves lap at the island."},

    {"lowtide",
        "It is low tide.\n"
        "Waves lap at the island."},

    {"fadeout",
        "The scene fades to black."},

    {"walking",
        "Johnny walks around\n"
        "the island."},

    /* --- Numbered scenes (match storyScenes[] indices) --- */

    {"scene00",
        "Johnny dives off the palm tree.\n"
        "A perfect flip into the ocean.\n"
        "Crabs and seagull hold up\n"
        "low scorecards."},

    {"scene01",
        "Johnny dives off the palm tree.\n"
        "It turns into a belly-flop.\n"
        "Crabs and seagull hold up\n"
        "low scorecards."},

    {"scene02",
        "Johnny reads under the tree.\n"
        "A seagull lands on his head.\n"
        "He swings a club but misses\n"
        "and hits himself."},

    {"scene03",
        "Johnny bathes in the ocean.\n"
        "A seagull steals his clothes\n"
        "for its nest.\n"
        "Johnny shivers angrily."},

    {"scene04",
        "Johnny reads under the tree.\n"
        "A seagull swoops down\n"
        "and steals his book."},

    {"scene05",
        "Johnny climbs the palm tree.\n"
        "He looks around, then dives.\n"
        "He walks back and looks around."},

    {"scene06",
        "Johnny fans himself in heat.\n"
        "He does a rain dance in a mask.\n"
        "A cloud appears but no rain.\n"
        "Lightning strikes him to ash."},

    {"scene07",
        "Johnny reads under the tree.\n"
        "He falls asleep.\n"
        "A coconut bonks his head.\n"
        "He wakes and keeps reading."},

    {"scene08",
        "Johnny reads under the tree.\n"
        "He scratches his head confused.\n"
        "The book was upside down.\n"
        "He flips it and reads on."},

    {"scene09",
        "Johnny bathes in the ocean.\n"
        "He scrubs, smells the brush\n"
        "in disgust, grabs his clothes\n"
        "and walks behind the tree."},

    {"scene10",
        "Johnny wears a mask and skirt.\n"
        "A yacht couple takes photos.\n"
        "His grass skirt falls open.\n"
        "The yacht sails away."},

    {"scene11",
        "Johnny builds a sand castle.\n"
        "It crumbles.\n"
        "He stomps it in frustration."},

    {"scene12",
        "Johnny sleeps under the tree.\n"
        "Lilliputians row ashore\n"
        "and tie him down.\n"
        "A seagull nests on him."},

    {"scene13",
        "Johnny sleeps under the tree.\n"
        "Zs float as he snores.\n"
        "He walks to the island edge."},

    {"scene14",
        "Johnny builds a sand castle.\n"
        "Lilliputians claim it as\n"
        "their fortress.\n"
        "Tiny planes attack Johnny."},

    {"scene15",
        "Johnny tries to build a fire.\n"
        "He rubs sticks together.\n"
        "It finally lights!\n"
        "He warms his hands, it dies."},

    {"scene16",
        "Johnny relaxes by a fire.\n"
        "He roasts an old boot.\n"
        "He eats the boot whole."},

    {"scene17",
        "Johnny sleeps under the tree.\n"
        "Lilliputians row ashore\n"
        "and tie him down.\n"
        "He goes back to sleep."},

    {"scene18",
        "Johnny goes fishing.\n"
        "He catches a starfish.\n"
        "He throws it back."},

    {"scene19",
        "Johnny goes fishing.\n"
        "He catches a boot.\n"
        "He keeps the boot."},

    {"scene20",
        "Johnny goes fishing.\n"
        "He catches five green fish.\n"
        "Then an angry octopus.\n"
        "The octopus chokes him."},

    {"scene21",
        "Johnny goes fishing.\n"
        "He catches a shark.\n"
        "The shark drags him around\n"
        "the ocean like a jet-ski."},

    {"scene22",
        "Johnny goes fishing.\n"
        "A shark eats him.\n"
        "The shark spits him back out."},

    {"scene23",
        "Johnny goes fishing.\n"
        "He catches a big green fish.\n"
        "It spits water in his face."},

    {"scene24",
        "Johnny goes fishing.\n"
        "He catches a crab.\n"
        "It snaps his nose."},

    {"scene25",
        "Johnny goes fishing.\n"
        "He catches a boot.\n"
        "He keeps the boot."},

    {"scene26",
        "A clock spins wildly.\n"
        "Sunset silhouette. A plane.\n"
        "Johnny parachutes down.\n"
        "The End."},

    {"scene27",
        "A bottle washes ashore.\n"
        "Johnny writes an S.O.S.\n"
        "He corks the bottle\n"
        "and throws it out to sea."},

    {"scene28",
        "Johnny writes a message.\n"
        "He imagines a clock at 3pm.\n"
        "He throws the bottle out\n"
        "to prepare for his date."},

    {"scene29",
        "A bottle washes ashore.\n"
        "Johnny picks it up excitedly.\n"
        "Sadly, it is his own S.O.S.\n"
        "He throws it back out."},

    {"scene30",
        "Johnny writes an S.O.S.\n"
        "He corks the bottle\n"
        "and throws it out to sea."},

    {"scene31",
        "A clock spins wildly.\n"
        "Johnny types at an office PC.\n"
        "He dreams of the island\n"
        "and the mermaid. He looks sad."},

    {"scene32",
        "Johnny sets up a fancy dinner.\n"
        "A mermaid appears.\n"
        "They eat, toast champagne,\n"
        "and dance. She swims away."},

    {"scene33",
        "A mermaid swims up.\n"
        "She gives Johnny a necklace.\n"
        "He gives her a life preserver.\n"
        "He proposes a date."},

    {"scene34",
        "Johnny fishes at the edge.\n"
        "A mermaid swims up behind him.\n"
        "He thinks it is a fish."},

    {"scene35",
        "Johnny fixes his raft.\n"
        "The mermaid asks what he does.\n"
        "He says he is leaving.\n"
        "She is heartbroken."},

    {"scene36",
        "Johnny packs his bags.\n"
        "The mermaid and shark say bye.\n"
        "The shark shakes his hand.\n"
        "Johnny paddles away."},

    {"scene37",
        "Johnny fans himself in heat.\n"
        "He fans harder and harder.\n"
        "He melts into a puddle."},

    {"scene38",
        "Johnny goes to swim.\n"
        "He dips a toe in the ocean.\n"
        "A shark snaps at him.\n"
        "He scrambles back to shore."},

    {"scene39",
        "Johnny stands at the edge.\n"
        "He taps his foot nervously."},

    {"scene40",
        "Johnny adjusts his pants."},

    {"scene41",
        "Johnny looks over the ocean.\n"
        "He adjusts his hat and pants."},

    {"scene42",
        "Johnny taps his foot."},

    {"scene43",
        "Johnny lifts his hat\n"
        "and looks around."},

    {"scene44",
        "Johnny taps his foot.\n"
        "He lifts his hat\n"
        "and looks around."},

    {"scene45",
        "Johnny taps his foot.\n"
        "He looks back into\n"
        "the distance."},

    {"scene46",
        "Johnny lifts his hat."},

    {"scene47",
        "Johnny taps his foot.\n"
        "He looks at the palm tree."},

    {"scene48",
        "Johnny looks at his raft."},

    {"scene49",
        "Johnny looks over the ocean."},

    {"scene50",
        "Johnny looks around\n"
        "under the palm tree shade."},

    {"scene51",
        "Johnny pulls out a spyglass\n"
        "and scans the horizon."},

    {"scene52",
        "Johnny pulls out a spyglass\n"
        "and scans the horizon."},

    {"scene53",
        "A frog clock spins wildly.\n"
        "A redhead finds the bottle.\n"
        "She imagines a volcano island\n"
        "and a handsome man."},

    {"scene54",
        "A frog clock spins wildly.\n"
        "Johnny's raft reaches her.\n"
        "She kisses him passionately,\n"
        "then scolds him."},

    {"scene55",
        "Johnny scans with a spyglass.\n"
        "A plane flies overhead.\n"
        "He looks the wrong way\n"
        "and misses it entirely."},

    {"scene56",
        "A red boat spots Johnny.\n"
        "He waves excitedly.\n"
        "The boat is enormous,\n"
        "it fills the whole screen."},

    {"scene57",
        "Johnny shakes the palm tree.\n"
        "A coconut bonks his head\n"
        "and flies into the ocean."},

    {"scene58",
        "Johnny shakes the palm tree.\n"
        "A coconut falls down.\n"
        "He chases and catches it.\n"
        "He cracks and eats it."},

    {"scene59",
        "Johnny shakes the palm tree.\n"
        "A coconut falls down.\n"
        "He cracks it on the tree\n"
        "and eats it."},

    {"scene60",
        ""},  /* empty in source data */

    {"scene61",
        "A boat with partygoers sails up.\n"
        "Johnny swims to the boat.\n"
        "He returns very drunk,\n"
        "wearing a party hat."},

    {"scene62",
        "Johnny builds up his raft."},

    {"scene63",
        "Johnny jogs around the island\n"
        "in a grey jogging outfit.\n"
        "He changes back to normal."},

    /* --- New captions for orphaned ADS slots (filled in caption-data
     * audit, 2026-04-26) --- */

    {"buildingdone",
        "Johnny finishes building.\n"
        "He stands at the island edge\n"
        "and admires his work."},

    {"visitorboat",
        "A boat reaches the island.\n"
        "Johnny climbs aboard\n"
        "and sails away."},

    {"fishingraft",
        "Johnny goes fishing.\n"
        "He catches a life raft.\n"
        "He drags it ashore."},

    {NULL, NULL}  /* sentinel */
};


/* ------------------------------------------------------------------ */
/*  Scene-to-ADS mapping                                               */
/*  Maps caption scene IDs to ADS file names and tag numbers.          */
/*  Follows the order of storyScenes[] in src/story_data.h.            */
/*  Re-derived 2026-04-26 from a content-driven audit (see             */
/*  scratch/caption-audit/corrected-map.yaml).                          */
/* ------------------------------------------------------------------ */

static const struct TCaptionSceneMap captionSceneMap[] = {
    /* ACTIVITY.ADS scenes */
    {"scene00", "ACTIVITY",  1},
    {"scene01", "ACTIVITY", 12},
    {"scene06", "ACTIVITY", 11},
    {"scene04", "ACTIVITY", 10},
    {"scene02", "ACTIVITY",  4},
    {"scene05", "ACTIVITY",  5},
    {"scene07", "ACTIVITY",  6},
    {"scene03", "ACTIVITY",  7},
    {"scene08", "ACTIVITY",  8},
    {"scene09", "ACTIVITY",  9},

    /* BUILDING.ADS scenes */
    {"scene11", "BUILDING",  1},
    {"scene14", "BUILDING",  4},
    {"scene15", "BUILDING",  3},
    {"scene16", "BUILDING",  2},
    {"scene35", "BUILDING",  5},
    {"scene62", "BUILDING",  7},
    {"buildingdone", "BUILDING",  6},

    /* FISHING.ADS scenes */
    {"scene18",      "FISHING",  1},  /* confirmed: starfish */
    {"fishingraft",  "FISHING",  2},  /* confirmed: life raft */
    {"scene20",      "FISHING",  3},  /* confirmed: octopus */
    {"scene21",      "FISHING",  4},  /* guess: shark drag */
    {"scene22",      "FISHING",  5},  /* guess: shark eats */
    {"scene23",      "FISHING",  6},  /* guess: big green fish */
    {"scene24",      "FISHING",  7},  /* guess: crab */
    {"scene19",      "FISHING",  8},  /* guess: boot kept */

    /* JOHNNY.ADS scenes */
    {"scene26", "JOHNNY",  1},
    {"scene27", "JOHNNY",  2},
    {"scene29", "JOHNNY",  3},
    {"scene30", "JOHNNY",  4},
    {"scene28", "JOHNNY",  5},
    {"scene31", "JOHNNY",  6},

    /* MARY.ADS scenes */
    {"scene32", "MARY",  1},
    {"scene34", "MARY",  3},
    {"scene33", "MARY",  2},
    {"scene35", "MARY",  4},
    {"scene36", "MARY",  5},

    /* MISCGAG.ADS scenes */
    {"scene37", "MISCGAG",  1},
    {"scene38", "MISCGAG",  2},

    /* STAND.ADS scenes */
    {"scene39", "STAND",  1},
    {"scene40", "STAND",  2},
    {"scene46", "STAND",  3},
    {"scene42", "STAND",  4},
    {"scene41", "STAND",  5},
    {"scene47", "STAND",  6},
    {"scene43", "STAND",  7},
    {"scene44", "STAND",  8},
    {"scene45", "STAND",  9},
    {"scene48", "STAND", 10},
    {"scene50", "STAND", 11},
    {"scene49", "STAND", 12},
    {"scene51", "STAND", 15},
    {"scene52", "STAND", 16},

    /* SUZY.ADS scenes */
    {"scene53", "SUZY",  1},
    {"scene54", "SUZY",  2},

    /* VISITOR.ADS scenes */
    {"scene12", "VISITOR",  1},
    {"scene10", "VISITOR",  3},
    {"scene56", "VISITOR",  4},
    {"scene61", "VISITOR",  6},
    {"scene55", "VISITOR",  7},
    {"visitorboat", "VISITOR",  5},

    /* WALKSTUF.ADS scenes */
    {"walking", "WALKSTUF",  1},
    {"walking", "WALKSTUF",  2},
    {"walking", "WALKSTUF",  3},

    {NULL, NULL, 0}  /* sentinel */
};


/* ------------------------------------------------------------------ */
/*  State                                                              */
/* ------------------------------------------------------------------ */

int ps1CaptionsEnabled          = 0;
static const char *currentCaption = NULL;
static int captionDisplayTimer  = 0;

/* ~5 seconds at 60 fps */
#define CAPTION_DURATION_FRAMES  300

/* ------------------------------------------------------------------ */
/*  Enable / disable                                                  */
/* ------------------------------------------------------------------ */

void captionsSetEnabled(int enabled)
{
    ps1CaptionsEnabled = enabled;
    if (!enabled) {
        currentCaption = NULL;
        captionDisplayTimer = 0;
    }
}

int captionsGetEnabled(void)
{
    return ps1CaptionsEnabled;
}

int captionsIsVisible(void)
{
    return ps1CaptionsEnabled && captionDisplayTimer > 0 &&
           currentCaption != NULL;
}

void captionsClear(void)
{
    currentCaption = NULL;
    captionDisplayTimer = 0;
}


/* ------------------------------------------------------------------ */
/*  Scene start — lookup by scene ID string (e.g. "scene05")          */
/* ------------------------------------------------------------------ */

void captionsOnSceneStart(const char *sceneId)
{
    if (!ps1CaptionsEnabled || sceneId == NULL) return;

    currentCaption = NULL;
    captionDisplayTimer = 0;

    for (int i = 0; captions[i].scene_id != NULL; i++) {
        if (strcmp(captions[i].scene_id, sceneId) == 0) {
            /* Skip empty captions (e.g. scene60) */
            if (captions[i].text[0] == '\0') return;
            currentCaption = captions[i].text;
            captionDisplayTimer = CAPTION_DURATION_FRAMES;
            return;
        }
    }
}


/* ------------------------------------------------------------------ */
/*  Scene start — lookup by ADS name + tag number                     */
/*  Translates the ADS identity to a scene ID, then looks up the      */
/*  caption text.  The ADS name comparison ignores the ".ADS" suffix.  */
/* ------------------------------------------------------------------ */

void captionsOnAdsStart(const char *adsName, uint16 adsTag)
{
    if (!ps1CaptionsEnabled || adsName == NULL) return;

    /* Strip ".ADS" suffix if present — compare only the base name. */
    char baseName[16];
    int len = 0;
    while (adsName[len] != '\0' && adsName[len] != '.' && len < 15) {
        baseName[len] = adsName[len];
        len++;
    }
    baseName[len] = '\0';

    for (int i = 0; captionSceneMap[i].caption_id != NULL; i++) {
        if (strcmp(captionSceneMap[i].ads_name, baseName) == 0
            && captionSceneMap[i].ads_tag == adsTag) {
            captionsOnSceneStart(captionSceneMap[i].caption_id);
            return;
        }
    }

    /* No mapping found — clear any previous caption. */
    currentCaption = NULL;
    captionDisplayTimer = 0;
}

void captionsShowText(const char *text, int frames)
{
    if (!ps1CaptionsEnabled || text == NULL || text[0] == '\0') return;
    currentCaption = text;
    captionDisplayTimer = (frames > 0) ? frames : CAPTION_DURATION_FRAMES;
}


/* ------------------------------------------------------------------ */
/*  Per-frame tick — returns current text or NULL                      */
/* ------------------------------------------------------------------ */

const char *captionsGetCurrent(void)
{
    if (!ps1CaptionsEnabled || captionDisplayTimer <= 0)
        return NULL;

    captionDisplayTimer--;
    return currentCaption;
}


/* ------------------------------------------------------------------ */
/*  PS1 on-screen caption renderer                                     */
/* ------------------------------------------------------------------ */
/*
 * captionsRender() draws the current caption inside a dark
 * semi-transparent band near the bottom of the 640x480 frame.
 * Reuses the pause-menu font glyph atlas (uploaded to VRAM at
 * (PAUSE_FONT_VRAM_X, PAUSE_FONT_VRAM_Y) — see pause_menu.c
 * pmUploadFont). Called once per scene frame from
 * grUpdateDisplay, after the scene composite + LoadImage and
 * before VSync.
 *
 * Bails immediately if captions are disabled or there's no
 * current caption text — zero cost when off.
 *
 * Builds its own OT + primitive scratch and submits via DrawOTag,
 * so it doesn't disturb whatever else is in flight.
 */

#ifdef PS1_BUILD

#define CAP_OT_LEN        4
#define CAP_PRIM_BUF_LEN  4096

#define CAP_BAND_X0   40
#define CAP_BAND_X1   600
#define CAP_BAND_Y0   400
#define CAP_BAND_Y1   470
#define CAP_LINE_STEP (PAUSE_GLYPH_DRAW_H + 2)

static uint32 capOt[CAP_OT_LEN];
static uint8  capPrimBuf[CAP_PRIM_BUF_LEN];

/* Count chars up to next '\n' or end. */
static int capLineLen(const char *s)
{
    int n = 0;
    while (s[n] != '\0' && s[n] != '\n') n++;
    return n;
}

/* Count number of '\n'-separated lines in s. */
static int capLineCount(const char *s)
{
    int n = 1;
    while (*s) {
        if (*s++ == '\n') n++;
    }
    return n;
}

/* Append one SPRT primitive for char `c` at (x, y), using the
 * pause-menu font glyph atlas. Returns advance in pixels (always
 * PAUSE_GLYPH_DRAW_W). */
static int capDrawChar(uint8 **nextp, uint32 *otSlot,
                       int x, int y, char c)
{
    unsigned char uc = (unsigned char)c;
    if (uc < PAUSE_GLYPH_FIRST ||
        uc >= PAUSE_GLYPH_FIRST + PAUSE_GLYPH_COUNT)
        return PAUSE_GLYPH_DRAW_W;
    int idx = uc - PAUSE_GLYPH_FIRST;
    int col = idx % 16;
    int row = idx / 16;

    SPRT *sprt = (SPRT *)(*nextp);
    *nextp += sizeof(SPRT);
    setSprt(sprt);
    setXY0(sprt, x, y);
    setWH(sprt, PAUSE_GLYPH_DRAW_W, PAUSE_GLYPH_DRAW_H);
    setUV0(sprt, col * PAUSE_GLYPH_DRAW_W, row * PAUSE_GLYPH_DRAW_H);
    setClut(sprt, PAUSE_CLUT_VRAM_X, PAUSE_CLUT_VRAM_Y);
    setRGB0(sprt, 128, 128, 128);   /* white via mid-gray (PS1 doubles) */
    ps1GpuOtAddPrim(otSlot, sprt);
    return PAUSE_GLYPH_DRAW_W;
}

void captionsRender(void)
{
    if (!ps1CaptionsEnabled || captionDisplayTimer <= 0 ||
        currentCaption == NULL)
        return;

    /* Caller (grUpdateDisplay) might invoke us before the first pause
     * has been opened. Make sure the font is in VRAM. Idempotent. */
    pauseMenuEnsureFontUploaded();

    ClearOTagR(capOt, CAP_OT_LEN);
    uint8 *next = capPrimBuf;

    /* Slot N-1: TPAGE for the font region. abr=0 = source-blend
     * (50% ish). The font uses CLUT entry 1=white, 0=transparent. */
    DR_TPAGE *tp = (DR_TPAGE *)next;
    next += sizeof(DR_TPAGE);
    setDrawTPage(tp, 0, 1,
                 getTPage(0, 0, PAUSE_FONT_VRAM_X, PAUSE_FONT_VRAM_Y));
    ps1GpuOtAddPrim(&capOt[CAP_OT_LEN - 1], tp);

    /* Slot N-2: dark semi-transparent band behind the text. Sized to
     * fit up to 3 caption lines; collapses to the actual line count.
     * 50% black blend means the scene shows through. */
    int lineCount = capLineCount(currentCaption);
    if (lineCount > 4) lineCount = 4;       /* clamp */
    int bandH = lineCount * CAP_LINE_STEP + 8;
    int bandY0 = CAP_BAND_Y1 - bandH;
    if (bandY0 < CAP_BAND_Y0) bandY0 = CAP_BAND_Y0;

    POLY_F4 *band = (POLY_F4 *)next;
    next += sizeof(POLY_F4);
    setPolyF4(band);
    setSemiTrans(band, 1);
    setRGB0(band, 0, 0, 0);
    setXY4(band,
           CAP_BAND_X0,  bandY0,
           CAP_BAND_X1,  bandY0,
           CAP_BAND_X0,  CAP_BAND_Y1,
           CAP_BAND_X1,  CAP_BAND_Y1);
    ps1GpuOtAddPrim(&capOt[CAP_OT_LEN - 2], band);

    /* Slot N-3: text SPRTs, line by line. Center each line in the band. */
    const char *p = currentCaption;
    int lineY = bandY0 + 4;
    int line = 0;
    while (*p && line < 4) {
        int len = capLineLen(p);
        int width = len * PAUSE_GLYPH_DRAW_W;
        int x = (CAP_BAND_X0 + CAP_BAND_X1 - width) / 2;
        if (x < CAP_BAND_X0 + 4) x = CAP_BAND_X0 + 4;
        for (int i = 0; i < len; i++) {
            x += capDrawChar(&next, &capOt[CAP_OT_LEN - 3],
                             x, lineY, p[i]);
        }
        p += len;
        if (*p == '\n') p++;
        lineY += CAP_LINE_STEP;
        line++;
    }

    DrawOTag(&capOt[CAP_OT_LEN - 1]);

    if (captionDisplayTimer > 0)
        captionDisplayTimer--;
    if (captionDisplayTimer <= 0)
        currentCaption = NULL;
}

#else  /* host build — no PS1 GPU; render is a no-op */

void captionsRender(void)
{
}

#endif
