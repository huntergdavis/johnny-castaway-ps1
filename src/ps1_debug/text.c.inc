/*
 * Initialize visual debugging system
 */
#define PS1_DEBUG_FONT_X 960
#define PS1_DEBUG_FONT_Y 0
#define PS1_DEBUG_CLUT_X 960
#define PS1_DEBUG_CLUT_Y 128
#define PS1_DEBUG_GLYPH_W 8
#define PS1_DEBUG_GLYPH_H 8
#define PS1_DEBUG_OT_LEN 8
#define PS1_DEBUG_PRIM_BYTES 32768
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

    ClearOTagR(ps1DebugOt, PS1_DEBUG_OT_LEN);
    if (next + sizeof(DR_TPAGE) < limit) {
        DR_TPAGE *tp = (DR_TPAGE *)next;
        next += sizeof(DR_TPAGE);
        setDrawTPage(tp, 0, 0, getTPage(0, 0, PS1_DEBUG_FONT_X, PS1_DEBUG_FONT_Y));
        ps1GpuOtAddPrim(&ps1DebugOt[PS1_DEBUG_OT_LEN - 1], tp);
    }

    while (*text != '\0' && y + PS1_DEBUG_GLYPH_H <= maxY) {
        unsigned char c = (unsigned char)*text++;
        int col;
        int row;
        SPRT *sprt;

        if (c == '\r')
            continue;
        if (c == '\n' || x + PS1_DEBUG_GLYPH_W > maxX) {
            x = startX;
            y += PS1_DEBUG_GLYPH_H;
            if (c == '\n')
                continue;
        }
        if (c < 32 || c > 127) {
            x += PS1_DEBUG_GLYPH_W;
            continue;
        }
        if (next + sizeof(SPRT) >= limit)
            break;

        col = c & 15;
        row = c >> 4;
        sprt = (SPRT *)next;
        next += sizeof(SPRT);
        setSprt(sprt);
        setXY0(sprt, x, y);
        setWH(sprt, PS1_DEBUG_GLYPH_W, PS1_DEBUG_GLYPH_H);
        setUV0(sprt, (uint8_t)(col * PS1_DEBUG_GLYPH_W),
               (uint8_t)(row * PS1_DEBUG_GLYPH_H));
        setClut(sprt, PS1_DEBUG_CLUT_X, PS1_DEBUG_CLUT_Y);
        setRGB0(sprt, r, g, b);
        ps1GpuOtAddPrim(textOt, sprt);
        x += PS1_DEBUG_GLYPH_W;
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

    /* Load built-in PSX BIOS font (8x8 characters) */
    FntLoad(960, 0);
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
    /* Re-initialize GPU and font - the game overwrites font VRAM area */
    ResetGraph(0);
    SetVideoMode(MODE_NTSC);
    FntLoad(960, 0);
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
