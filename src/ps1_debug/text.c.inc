/*
 * Initialize visual debugging system
 */
#define PS1_DEBUG_OT_LEN 8
/* 24 KB (was 32 KB): the libc heap headroom after the 1440 KB region
 * buffer is razor-thin, and exe growth pushes the 2x32 KB GPU primitive
 * buffer allocation over the cliff ("Failed to allocate primitive
 * buffers" boot hang) — first in verbose perf-schema builds (v0.9.3 +
 * ~4 KB of JCPERF2 strings), then in release builds on the
 * transition-zero branch. The debug renderer bounds-checks and drops
 * prims when full, so a denser panel degrades instead of corrupting;
 * 24 KB comfortably covers the BSOD text panel. */
#define PS1_DEBUG_PRIM_BYTES 24576
#define PS1_DEBUG_TEXT_BUF_BYTES 2048

static uint32_t ps1DebugOt[PS1_DEBUG_OT_LEN];
static uint8_t ps1DebugPrim[PS1_DEBUG_PRIM_BYTES];
static char ps1DebugTextBuf[PS1_DEBUG_TEXT_BUF_BYTES];

static void ps1DebugCopyLine(char *dst, int dstSize, const char *src)
{
    int i = 0;

    if (dst == NULL || dstSize <= 0)
        return;
    if (src == NULL)
        src = "(null)";

    while (i < dstSize - 1 && src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

static char *ps1DebugAppendText(char *dst, char *end, const char *src)
{
    if (src == NULL)
        src = "(null)";
    while (dst < end && *src != '\0')
        *dst++ = *src++;
    *dst = '\0';
    return dst;
}

static char *ps1DebugAppendUnsigned(char *dst, char *end, unsigned long value)
{
    char digits[16];
    int count = 0;

    do {
        digits[count++] = (char)('0' + (value % 10UL));
        value /= 10UL;
    } while (value != 0 && count < (int)sizeof(digits));

    while (count > 0 && dst < end)
        *dst++ = digits[--count];
    *dst = '\0';
    return dst;
}

static char *ps1DebugAppendSigned(char *dst, char *end, long value)
{
    if (value < 0) {
        if (dst < end)
            *dst++ = '-';
        *dst = '\0';
        return ps1DebugAppendUnsigned(dst, end, 0UL - (unsigned long)value);
    }
    return ps1DebugAppendUnsigned(dst, end, (unsigned long)value);
}

static void ps1DebugDrawTextBlock(int x, int y, int w, int h,
                                  const char *text,
                                  uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t *next = ps1DebugPrim;
    uint8_t *limit = ps1DebugPrim + PS1_DEBUG_PRIM_BYTES;
    uint32_t *textOt = &ps1DebugOt[PS1_DEBUG_OT_LEN - 3];
    int startX = x;
    int maxX = x + w;
    int maxY = y + h;

    if (text == NULL)
        text = "";

    pauseMenuEnsureFontUploaded();
    ClearOTagR(ps1DebugOt, PS1_DEBUG_OT_LEN);
    if (next + sizeof(DR_TPAGE) < limit) {
        DR_TPAGE *tp = (DR_TPAGE *)next;
        next += sizeof(DR_TPAGE);
        setDrawTPage(tp, 0, 1,
                     getTPage(0, 0, PAUSE_FONT_VRAM_X, PAUSE_FONT_VRAM_Y));
        ps1GpuOtAddPrim(&ps1DebugOt[PS1_DEBUG_OT_LEN - 1], tp);
    }

    while (*text != '\0' && y + PAUSE_GLYPH_DRAW_H <= maxY) {
        unsigned char c = (unsigned char)*text++;
        int col;
        int row;
        SPRT *sprt;

        if (c == '\r')
            continue;
        if (c == '\n' || x + PAUSE_GLYPH_DRAW_W > maxX) {
            x = startX;
            y += PAUSE_GLYPH_DRAW_H;
            if (c == '\n')
                continue;
        }
        if (c < PAUSE_GLYPH_FIRST ||
            c >= PAUSE_GLYPH_FIRST + PAUSE_GLYPH_COUNT) {
            x += PAUSE_GLYPH_DRAW_W;
            continue;
        }
        if (next + sizeof(SPRT) >= limit)
            break;

        {
            int idx = (int)c - PAUSE_GLYPH_FIRST;
            col = idx & 15;
            row = idx >> 4;
        }
        sprt = (SPRT *)next;
        next += sizeof(SPRT);
        setSprt(sprt);
        setXY0(sprt, x, y);
        setWH(sprt, PAUSE_GLYPH_DRAW_W, PAUSE_GLYPH_DRAW_H);
        setUV0(sprt, (uint8_t)(col * PAUSE_GLYPH_DRAW_W),
               (uint8_t)(row * PAUSE_GLYPH_DRAW_H));
        setClut(sprt, PAUSE_CLUT_VRAM_X, PAUSE_CLUT_VRAM_Y);
        setRGB0(sprt, r, g, b);
        ps1GpuOtAddPrim(textOt, sprt);
        x += PAUSE_GLYPH_DRAW_W;
    }

    DrawOTag(&ps1DebugOt[PS1_DEBUG_OT_LEN - 1]);
    DrawSync(0);
}

void ps1DebugInit(void)
{
    /* Reset GPU and set video mode (do this ONCE at startup) */
    /* Use 640x480 interlaced high res mode */
    ResetGraph(0);
    SetVideoMode(MODE_NTSC);

    {
        DISPENV disp;
        DRAWENV draw;
        SetDefDispEnv(&disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        disp.isinter = 1;
        SetDefDrawEnv(&draw, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        draw.isbg = 0;
        PutDispEnv(&disp);
        PutDrawEnv(&draw);
    }

    pauseMenuUploadDebugFont();
    fontID = 0;

    /* Enable display */
    SetDispMask(1);

    /* Clear debug buffer */
    ps1DebugClear();
}

/*
 * Clear the debug text buffer
 */
void ps1DebugClear(void)
{
    numDebugLines = 0;
    for (int i = 0; i < MAX_DEBUG_LINES; i++) {
        debugLines[i][0] = '\0';
    }
}

/*
 * Print a line of text to the debug display
 */
void ps1DebugPrint(const char *fmt, ...)
{
    if (numDebugLines >= MAX_DEBUG_LINES) {
        return;  /* Buffer full */
    }

    ps1DebugCopyLine(debugLines[numDebugLines], MAX_LINE_LENGTH, fmt);
    numDebugLines++;
}

/*
 * Update the screen with current debug text
 */
void ps1DebugFlush(void)
{
    /* Set up draw environment with background color */
    DRAWENV draw;
    SetDefDrawEnv(&draw, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    setRGB0(&draw, bgR, bgG, bgB);
    draw.isbg = 1;  /* Enable background clear */
    PutDrawEnv(&draw);

    /* Draw all accumulated debug lines through the direct SPRT text path. */
    char *p = ps1DebugTextBuf;
    char *end = ps1DebugTextBuf + sizeof(ps1DebugTextBuf) - 1;
    *p = '\0';
    for (int i = 0; i < numDebugLines; i++) {
        p = ps1DebugAppendText(p, end, debugLines[i]);
        p = ps1DebugAppendText(p, end, "\n");
    }
    ps1DebugDrawTextBlock(10, 10,
                          SCREEN_WIDTH - 20,
                          SCREEN_HEIGHT - 20,
                          ps1DebugTextBuf,
                          0, 0, 0);

    /* The direct text helper synchronizes its small debug primitive batch. */
}

/*
 * Wait for user input (SELECT button) before continuing
 */
void ps1DebugWait(void)
{
    /* Show "Press SELECT to continue" message */
    ps1DebugDrawTextBlock(10, SCREEN_HEIGHT - 28,
                          SCREEN_WIDTH - 20, 16,
                          "Press SELECT to continue...",
                          0, 0, 0);

    /* Initialize pad */
    uint8_t pad_buff[2][34];
    InitPAD(pad_buff[0], 34, pad_buff[1], 34);
    StartPAD();
    ChangeClearPAD(0);

    /* Wait for SELECT button press */
    int waiting = 1;
    while (waiting) {
        PADTYPE *pad = (PADTYPE*)pad_buff[0];

        if (pad->stat == 0) {
            /* Pad connected */
            if ((pad->type == 0x4) || (pad->type == 0x5) || (pad->type == 0x7)) {
                /* Digital pad, DualShock, or Analog */
                if (!(pad->btn & PAD_SELECT)) {
                    /* SELECT button pressed */
                    waiting = 0;
                }
            }
        }

        VSync(0);  /* Wait for next frame */
    }

    /* Wait for button release */
    while (1) {
        PADTYPE *pad = (PADTYPE*)pad_buff[0];

        if (pad->stat == 0) {
            if ((pad->type == 0x4) || (pad->type == 0x5) || (pad->type == 0x7)) {
                if (pad->btn & PAD_SELECT) {
                    /* Button released */
                    break;
                }
            }
        }

        VSync(0);
    }

    StopPAD();
}

/*
 * Show an error screen with text and wait
 */
void ps1DebugError(const char *fmt, ...)
{
    /* Re-initialize GPU and font - the game can overwrite draw state. */
    ResetGraph(0);
    SetVideoMode(MODE_NTSC);
    {
        DISPENV disp;
        DRAWENV draw;
        SetDefDispEnv(&disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        disp.isinter = 1;
        SetDefDrawEnv(&draw, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        draw.isbg = 1;
        PutDispEnv(&disp);
        PutDrawEnv(&draw);
    }
    pauseMenuUploadDebugFont();
    fontID = 0;
    SetDispMask(1);

    /* Set error background color (dark red) */
    bgR = 64;
    bgG = 0;
    bgB = 0;

    /* Clear and add error header */
    ps1DebugClear();
    ps1DebugPrint("=== ERROR ===");
    ps1DebugPrint("");

    /* Add error message */
    ps1DebugPrint(fmt ? fmt : "(unspecified)");

    /* Flush and wait */
    ps1DebugFlush();
    ps1DebugWait();

    /* Reset background color */
    bgR = 0;
    bgG = 0;
    bgB = 64;
}
