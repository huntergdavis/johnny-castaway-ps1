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
#include <stdlib.h>
#include <string.h>

#include "ps1_captions.h"

#ifdef PS1_BUILD
#include <psxgpu.h>
#include "cdrom_ps1.h"
#include "pause_menu.h"
#include "ps1_gpu_ot.h"
#else
#include <stdio.h>
#endif

/* ------------------------------------------------------------------ */
/*  Caption text metadata                                             */
/* ------------------------------------------------------------------ */

#define PS1_CAPTION_DATA_DEFINE
#include "generated/ps1/ps1_caption_data.h"
#undef PS1_CAPTION_DATA_DEFINE


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
static int currentCaptionFromDisc = 0;
static uint8 *captionTextData = NULL;
static uint32 captionTextDataSize = 0;
static uint32 captionTextBase = 0;
static int captionTextDataValid = 0;

/* ~5 seconds at 60 fps */
#define CAPTION_DURATION_FRAMES  300

static uint16 captionsReadLe16(const uint8 *p)
{
    return (uint16)((uint16)p[0] | ((uint16)p[1] << 8));
}

static void captionsReleaseTextData(void)
{
    if (captionTextData != NULL) {
        free(captionTextData);
        captionTextData = NULL;
    }
    captionTextDataSize = 0;
    captionTextBase = 0;
    captionTextDataValid = 0;
}

static void captionsDropCurrent(void)
{
    currentCaption = NULL;
    captionDisplayTimer = 0;
    currentCaptionFromDisc = 0;
    captionsReleaseTextData();
}

static uint8 *captionsLoadTextFile(uint32 *outSize)
{
#ifdef PS1_BUILD
    return (uint8 *)ps1_loadRawFile("\\CAPTION.DAT;1", outSize);
#else
    FILE *f;
    long size;
    uint8 *data;

    if (outSize == NULL)
        return NULL;
    f = fopen("generated/ps1/CAPTION.DAT", "rb");
    if (f == NULL)
        return NULL;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    size = ftell(f);
    if (size <= 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);
    data = (uint8 *)malloc((size_t)size);
    if (data == NULL) {
        fclose(f);
        return NULL;
    }
    if (fread(data, 1, (size_t)size, f) != (size_t)size) {
        free(data);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *outSize = (uint32)size;
    return data;
#endif
}

static int captionsValidateTextData(const uint8 *data, uint32 size)
{
    uint16 count;
    uint32 stringBase;
    uint32 stringBytes;
    int i;

    if (data == NULL || size < 8)
        return 0;
    if (data[0] != 'J' || data[1] != 'C' ||
        data[2] != 'C' || data[3] != 'P')
        return 0;
    if (captionsReadLe16(&data[4]) != 1)
        return 0;
    count = captionsReadLe16(&data[6]);
    if (count != gCaptionDataCount)
        return 0;

    stringBase = 8u;
    stringBytes = size - stringBase;
    for (i = 0; i < (int)count; i++) {
        uint32 offset = (uint32)gCaptionData[i].text_offset;
        uint32 pos = offset;
        if (offset >= stringBytes)
            return 0;
        while (pos < stringBytes && data[stringBase + pos] != '\0')
            pos++;
        if (pos >= stringBytes)
            return 0;
    }
    return 1;
}

static int captionsEnsureTextData(void)
{
    uint32 size = 0;
    uint8 *data;

    if (captionTextDataValid)
        return 1;
    if (captionTextData != NULL)
        return 0;

    data = captionsLoadTextFile(&size);
    if (data == NULL)
        return 0;
    if (!captionsValidateTextData(data, size)) {
        free(data);
        return 0;
    }

    captionTextData = data;
    captionTextDataSize = size;
    captionTextBase = 8u;
    captionTextDataValid = 1;
    return 1;
}

static const char *captionsTextByOffset(uint16 offset)
{
    uint32 pos;

    if (!captionsEnsureTextData())
        return NULL;
    pos = captionTextBase + (uint32)offset;
    if (pos >= captionTextDataSize)
        return NULL;
    return (const char *)&captionTextData[pos];
}

/* ------------------------------------------------------------------ */
/*  Enable / disable                                                  */
/* ------------------------------------------------------------------ */

void captionsSetEnabled(int enabled)
{
    ps1CaptionsEnabled = enabled;
    if (!enabled)
        captionsDropCurrent();
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
    captionsDropCurrent();
}


/* ------------------------------------------------------------------ */
/*  Scene start — lookup by scene ID string (e.g. "scene05")          */
/* ------------------------------------------------------------------ */

void captionsOnSceneStart(const char *sceneId)
{
    int i;

    if (!ps1CaptionsEnabled || sceneId == NULL) return;

    currentCaption = NULL;
    captionDisplayTimer = 0;
    currentCaptionFromDisc = 0;

    for (i = 0; gCaptionData[i].scene_id != NULL; i++) {
        if (strcmp(gCaptionData[i].scene_id, sceneId) == 0) {
            const char *text = captionsTextByOffset(gCaptionData[i].text_offset);
            /* Skip empty captions (e.g. scene60) */
            if (text == NULL || text[0] == '\0') {
                captionsDropCurrent();
                return;
            }
            currentCaption = text;
            currentCaptionFromDisc = 1;
            captionDisplayTimer = CAPTION_DURATION_FRAMES;
            return;
        }
    }
    captionsDropCurrent();
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
    captionsDropCurrent();
}

void captionsShowText(const char *text, int frames)
{
    if (!ps1CaptionsEnabled || text == NULL || text[0] == '\0') return;
    captionsReleaseTextData();
    currentCaption = text;
    currentCaptionFromDisc = 0;
    captionDisplayTimer = (frames > 0) ? frames : CAPTION_DURATION_FRAMES;
}


/* ------------------------------------------------------------------ */
/*  Per-frame tick — returns current text or NULL                      */
/* ------------------------------------------------------------------ */

const char *captionsGetCurrent(void)
{
    if (!ps1CaptionsEnabled || captionDisplayTimer <= 0) {
        if (currentCaptionFromDisc)
            captionsDropCurrent();
        return NULL;
    }

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
        currentCaption == NULL) {
        if (currentCaptionFromDisc)
            captionsDropCurrent();
        return;
    }

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
        captionsDropCurrent();
}

#else  /* host build — no PS1 GPU; render is a no-op */

void captionsRender(void)
{
}

#endif
