/*
 *  Closed captions for PS1 accessibility.
 *
 *  Caption text extracted from the closed_captions branch.
 *  Each scene has descriptive text explaining what is
 *  happening on screen, displayed as subtitles.
 *
 *  Text is condensed to fit PS1 screen constraints:
 *  max ~35 characters per line for readability at 320px.
 */

#ifndef PS1_CAPTIONS_H
#define PS1_CAPTIONS_H

#include "mytypes.h"

/* ------------------------------------------------------------------ */
/*  Caption data structures                                           */
/* ------------------------------------------------------------------ */

struct TCaption {
    const char *scene_id;   /* e.g. "scene01", "intro" */
    const char *text;       /* Multi-line caption text  */
};

/* Scene-to-ADS mapping for caption lookup by ADS name + tag. */
struct TCaptionSceneMap {
    const char *caption_id; /* "scene00" etc.       */
    const char *ads_name;   /* "ACTIVITY" etc.      */
    uint16      ads_tag;    /* ADS tag number       */
};


/* ------------------------------------------------------------------ */
/*  Caption text + lookup table live in ps1_captions.c                */
/* ------------------------------------------------------------------ */

/* (Both `captions[]` and `captionSceneMap[]` were `static const` in
 * this header until 2026-04-26. They moved into ps1_captions.c so
 * every translation unit doesn't get a private copy of ~10 KB of
 * string data. Header only declares the structs + API now.) */


/* ------------------------------------------------------------------ */
/*  Caption system API                                                 */
/* ------------------------------------------------------------------ */

void captionsSetEnabled(int enabled);
int  captionsGetEnabled(void);
int  captionsIsVisible(void);
void captionsClear(void);
extern int ps1CaptionsEnabled;

/* Call when a scene starts — looks up caption by scene ID. */
void captionsOnSceneStart(const char *sceneId);

/* Call when an ADS scene starts — looks up caption by ADS name + tag. */
void captionsOnAdsStart(const char *adsName, uint16 adsTag);

/* Direct caption injection for runtime-only modes such as freeplay. */
void captionsShowText(const char *text, int frames);

/* Call each frame. Returns current caption text, or NULL if none. */
const char *captionsGetCurrent(void);

/* Draw the current caption (if any) into a dark band near the bottom
 * of the 640x480 frame. Called by grUpdateDisplay after the scene
 * composite + LoadImage and before VSync. No-op when captions are
 * disabled or there's no current text. */
void captionsRender(void);

#endif /* PS1_CAPTIONS_H */
