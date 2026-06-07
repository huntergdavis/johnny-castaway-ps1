/*
 *  Scripted PlayStation pad input for headless tests.
 *
 *  The script is embedded at build time from config/ps1/PADSCRIPT.TXT.
 *  BOOTMODE token "pad-script" enables it; "pad-script-log" also prints
 *  parsed events and screenshot markers. Script syntax:
 *
 *      wait 30s
 *      tap START
 *      tap DOWN
 *      hold R1+RIGHT 12
 *      shot pause-main 30
 *
 *  Durations are frames by default; a trailing "s" means seconds at 60 Hz.
 */

#include <psxgpu.h>
#include <psxpad.h>
#include <stdio.h>
#include <string.h>

#include "mytypes.h"
#include "ps1_pad_script.h"
#include "config/ps1/padscript_embedded.h"

#ifndef PS1_VERBOSE_DIAGNOSTICS
#define PS1_VERBOSE_DIAGNOSTICS 0
#endif

#define PAD_SCRIPT_MAX_EVENTS 160
#define PAD_SCRIPT_LABEL_LEN  32
#define PAD_SCRIPT_LINE_LEN   96
#define PAD_SCRIPT_TAP_FRAMES 12UL
#define PAD_SCRIPT_GAP_FRAMES 8UL

struct TPadScriptEvent {
    uint32 startFrame;
    uint32 endFrame;
    uint16 buttons;
    uint8 isShot;
    char label[PAD_SCRIPT_LABEL_LEN];
};

static struct TPadScriptEvent gPadScriptEvents[PAD_SCRIPT_MAX_EVENTS];
static int gPadScriptEnabled = 0;
static int gPadScriptVerbose = 0;
static int gPadScriptEventCount = 0;
static int gPadScriptParsed = 0;
static uint32 gPadScriptFrame = 0;
static uint32 gPadScriptLastTick = 0;
static uint16 gPadScriptButtons = 0;
static uint16 gPadScriptLastLoggedButtons = 0;

static char padUpper(char c)
{
    if (c >= 'a' && c <= 'z')
        return (char)(c - ('a' - 'A'));
    return c;
}

static int padIsSpace(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static void copyUpperToken(char *dst, int dstLen, const char *src)
{
    int i = 0;
    if (dstLen <= 0)
        return;
    while (src[i] && !padIsSpace(src[i]) && i < dstLen - 1) {
        dst[i] = padUpper(src[i]);
        i++;
    }
    dst[i] = '\0';
}

static const char *skipSpaces(const char *p)
{
    while (*p && padIsSpace(*p))
        p++;
    return p;
}

static const char *readToken(const char *p, char *out, int outLen)
{
    int i = 0;
    p = skipSpaces(p);
    while (*p && !padIsSpace(*p)) {
        if (i < outLen - 1)
            out[i++] = *p;
        p++;
    }
    out[i] = '\0';
    return p;
}

static uint32 parseDurationFrames(const char *token, uint32 fallback)
{
    int value = 0;
    int i = 0;
    if (token == NULL || token[0] == '\0')
        return fallback;
    while (token[i] >= '0' && token[i] <= '9') {
        value = (value * 10) + (token[i] - '0');
        i++;
    }
    if (value <= 0)
        return fallback;
    if (token[i] == 's' || token[i] == 'S')
        return (uint32)value * 60UL;
    return (uint32)value;
}

static uint16 buttonForName(const char *name)
{
    char t[24];
    copyUpperToken(t, (int)sizeof(t), name);

    if (!strcmp(t, "START")) return PAD_START;
    if (!strcmp(t, "SELECT")) return PAD_SELECT;
    if (!strcmp(t, "UP")) return PAD_UP;
    if (!strcmp(t, "DOWN")) return PAD_DOWN;
    if (!strcmp(t, "LEFT")) return PAD_LEFT;
    if (!strcmp(t, "RIGHT")) return PAD_RIGHT;
    if (!strcmp(t, "CROSS") || !strcmp(t, "X")) return PAD_CROSS;
    if (!strcmp(t, "CIRCLE") || !strcmp(t, "O")) return PAD_CIRCLE;
    if (!strcmp(t, "TRIANGLE")) return PAD_TRIANGLE;
    if (!strcmp(t, "SQUARE")) return PAD_SQUARE;
    if (!strcmp(t, "L1")) return PAD_L1;
    if (!strcmp(t, "R1")) return PAD_R1;
    if (!strcmp(t, "L2")) return PAD_L2;
    if (!strcmp(t, "R2")) return PAD_R2;
    return 0;
}

static uint16 parseButtonMask(const char *token)
{
    uint16 mask = 0;
    char one[24];
    int oi = 0;
    const char *p = token;

    while (1) {
        char c = *p;
        if (c == '+' || c == ',' || c == '\0') {
            one[oi] = '\0';
            if (oi > 0)
                mask |= buttonForName(one);
            oi = 0;
            if (c == '\0')
                break;
        } else if (oi < (int)sizeof(one) - 1) {
            one[oi++] = c;
        }
        p++;
    }
    return mask;
}

static int addPadEvent(uint32 startFrame, uint32 endFrame, uint16 buttons,
                       const char *label, int isShot)
{
    struct TPadScriptEvent *ev;
    int i;

    if (gPadScriptEventCount >= PAD_SCRIPT_MAX_EVENTS)
        return 0;

    ev = &gPadScriptEvents[gPadScriptEventCount++];
    ev->startFrame = startFrame;
    ev->endFrame = endFrame;
    ev->buttons = buttons;
    ev->isShot = isShot ? 1 : 0;
    ev->label[0] = '\0';
    if (label != NULL) {
        for (i = 0; label[i] && i < PAD_SCRIPT_LABEL_LEN - 1; i++)
            ev->label[i] = label[i];
        ev->label[i] = '\0';
    }
    return 1;
}

static void parsePadScriptLine(const char *line, uint32 *cursorFrame)
{
    char command[24];
    char arg1[48];
    char arg2[24];
    char upperCommand[24];
    const char *p;
    uint32 duration;
    uint16 buttons;

    p = skipSpaces(line);
    if (*p == '\0' || *p == '#')
        return;

    p = readToken(p, command, (int)sizeof(command));
    p = readToken(p, arg1, (int)sizeof(arg1));
    p = readToken(p, arg2, (int)sizeof(arg2));
    copyUpperToken(upperCommand, (int)sizeof(upperCommand), command);

    if (!strcmp(upperCommand, "WAIT")) {
        *cursorFrame += parseDurationFrames(arg1, 1);
        return;
    }

    if (!strcmp(upperCommand, "SHOT") ||
        !strcmp(upperCommand, "SCREENSHOT") ||
        !strcmp(upperCommand, "MARK")) {
        if (arg1[0] != '\0') {
            uint32 shotFrame = *cursorFrame + parseDurationFrames(arg2, 0);
            addPadEvent(shotFrame, shotFrame, 0, arg1, 1);
            /* A marker should describe the screen that is already open.
             * Keep the next scripted button press off the marker frame so
             * screenshot capture does not race the transition away from it.
             * `shot label 30` means "mark it 30 frames from now." */
            *cursorFrame = shotFrame + PAD_SCRIPT_GAP_FRAMES;
        }
        return;
    }

    if (!strcmp(upperCommand, "TAP") || !strcmp(upperCommand, "PRESS")) {
        buttons = parseButtonMask(arg1);
        duration = parseDurationFrames(arg2, PAD_SCRIPT_TAP_FRAMES);
        if (buttons != 0) {
            addPadEvent(*cursorFrame, *cursorFrame + duration, buttons, arg1, 0);
            *cursorFrame += duration + PAD_SCRIPT_GAP_FRAMES;
        }
        return;
    }

    if (!strcmp(upperCommand, "HOLD")) {
        buttons = parseButtonMask(arg1);
        duration = parseDurationFrames(arg2, PAD_SCRIPT_TAP_FRAMES);
        if (buttons != 0) {
            addPadEvent(*cursorFrame, *cursorFrame + duration, buttons, arg1, 0);
            *cursorFrame += duration;
        }
    }
}

static void parsePadScript(const char *script)
{
    char line[PAD_SCRIPT_LINE_LEN];
    int li = 0;
    const char *p = script;
    uint32 cursorFrame = 0;

    gPadScriptEventCount = 0;
    if (script == NULL)
        return;

    while (*p) {
        char c = *p++;
        if (c == '\n' || c == '\r') {
            line[li] = '\0';
            parsePadScriptLine(line, &cursorFrame);
            li = 0;
            if (c == '\r' && *p == '\n')
                p++;
            continue;
        }
        if (li < PAD_SCRIPT_LINE_LEN - 1)
            line[li++] = c;
    }
    if (li > 0) {
        line[li] = '\0';
        parsePadScriptLine(line, &cursorFrame);
    }
}

static void updatePadScriptState(void)
{
    uint16 buttons = 0;
    int i;

    for (i = 0; i < gPadScriptEventCount; i++) {
        struct TPadScriptEvent *ev = &gPadScriptEvents[i];
        if (ev->isShot && ev->startFrame == gPadScriptFrame) {
            printf("JCPADSHOT label=%s frame=%lu tick=%lu\n",
                   ev->label,
                   (unsigned long)gPadScriptFrame,
                   (unsigned long)VSync(-1));
        }
        if (!ev->isShot &&
            gPadScriptFrame >= ev->startFrame &&
            gPadScriptFrame < ev->endFrame) {
            buttons |= ev->buttons;
        }
    }

    gPadScriptButtons = buttons;
}

static void syncPadScriptToVSync(void)
{
    uint32 now;
    uint32 delta;

    if (!gPadScriptEnabled)
        return;

    now = (uint32)VSync(-1);
    if (gPadScriptLastTick == 0) {
        gPadScriptLastTick = now;
        updatePadScriptState();
        return;
    }

    if (now < gPadScriptLastTick) {
        gPadScriptLastTick = now;
        return;
    }

    delta = now - gPadScriptLastTick;
    while (delta-- > 0) {
        gPadScriptFrame++;
        updatePadScriptState();
    }
    gPadScriptLastTick = now;
}

void ps1PadScriptConfigureFromEmbedded(int enabled, int verbose)
{
    gPadScriptEnabled = 0;
    gPadScriptVerbose = verbose ? 1 : 0;
    gPadScriptParsed = 0;
    gPadScriptFrame = 0;
    gPadScriptLastTick = (uint32)VSync(-1);
    gPadScriptButtons = 0;
    gPadScriptLastLoggedButtons = 0;

    if (!enabled || PS1_EMBEDDED_PAD_SCRIPT[0] == '\0')
        return;

    parsePadScript(PS1_EMBEDDED_PAD_SCRIPT);
    gPadScriptParsed = 1;
    gPadScriptEnabled = (gPadScriptEventCount > 0) ? 1 : 0;
    updatePadScriptState();

#if PS1_VERBOSE_DIAGNOSTICS
    if (gPadScriptVerbose) {
        int i;
        printf("JCPADSCRIPT %s events=%d verbose=%d\n",
               gPadScriptEnabled ? "enabled" : "empty",
               gPadScriptEventCount,
               gPadScriptVerbose);
        for (i = 0; i < gPadScriptEventCount; i++) {
            const struct TPadScriptEvent *ev = &gPadScriptEvents[i];
            printf("JCPADSCRIPT event=%d frame=%lu..%lu mask=%04x shot=%d label=%s\n",
                   i,
                   (unsigned long)ev->startFrame,
                   (unsigned long)ev->endFrame,
                   (unsigned)ev->buttons,
                   ev->isShot,
                   ev->label);
        }
    }
#endif
}

uint16 ps1PadScriptMergeButtons(uint16 physicalButtons)
{
    uint16 merged;
    if (!gPadScriptEnabled)
        return physicalButtons;
    syncPadScriptToVSync();
    merged = (uint16)(physicalButtons | gPadScriptButtons);
#if PS1_VERBOSE_DIAGNOSTICS
    if (gPadScriptVerbose && merged != gPadScriptLastLoggedButtons) {
        printf("JCPADSCRIPT buttons frame=%lu tick=%lu physical=%04x script=%04x merged=%04x\n",
               (unsigned long)gPadScriptFrame,
               (unsigned long)VSync(-1),
               (unsigned)physicalButtons,
               (unsigned)gPadScriptButtons,
               (unsigned)merged);
        gPadScriptLastLoggedButtons = merged;
    }
#endif
    return merged;
}

int ps1PadScriptIsEnabled(void)
{
    return gPadScriptEnabled && gPadScriptParsed;
}

int ps1PadScriptVerboseLogEnabled(void)
{
#if PS1_VERBOSE_DIAGNOSTICS
    return gPadScriptEnabled && gPadScriptVerbose;
#else
    return 0;
#endif
}
