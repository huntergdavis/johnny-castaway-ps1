/* ---------------------------------------------------------------------------
 *  Drawing helpers -- direct BIOS-font SPRT renderer
 * ------------------------------------------------------------------------- */

/* MANUAL SPRT TEXT RENDERER.
 *
 * Builds SPRT primitives directly, pointing at the BIOS font texture uploaded
 * to VRAM. The BIOS font is 16 chars wide by 16 chars tall, 8x8 each
 * (256 chars total), stored 4-bit packed.
 *
 * ASCII char N → texture coords (u, v):
 *   col = N % 16, row = N / 16
 *   u = col * 8, v = row * 8
 *
 * TPage covers a 256-texel-wide × 256-line region. The BIOS font at
 * (960, 0) sits at tpageX = 960/64 = 15, tpageY = 0/256 = 0.
 * BIOS font CLUT is conventionally at (x, y+128) → (960, 128).
 */
static int pmTextX = 60;
static int pmTextY = 100;
static int pmPrintfX = 80;

static void pmTextStart(int x, int y)
{
    pmTextX = x;
    pmTextY = y;
}

static void pmTextDrawChar(uint8 **nextp, uint32 *otSlot, char c)
{
    if (c == '\n') {
        pmTextX = pmPrintfX;
        pmTextY += PM_LINE_STEP;
        return;
    }
    unsigned char uc = (unsigned char)c;
    if (uc < PM_FONT_FIRST || uc >= PM_FONT_FIRST + PM_FONT_COUNT) {
        pmTextX += PM_GLYPH_W;
        return;
    }
    int idx = uc - PM_FONT_FIRST;
    int col = idx % 16;
    int row = idx / 16;

    SPRT *sprt = (SPRT*)(*nextp);
    *nextp += sizeof(SPRT);
    setSprt(sprt);
    setXY0(sprt, pmTextX, pmTextY);
    /* SPRT samples 1:1; on-screen size == texel size. Both 16x16 since
     * we pre-doubled the font at upload time. */
    setWH(sprt, PM_DRAW_W, PM_DRAW_H);
    setUV0(sprt, col * PM_DRAW_W, row * PM_DRAW_H);
    setClut(sprt, PM_CLUT_VRAM_X, PM_CLUT_VRAM_Y);
    setRGB0(sprt, 128, 128, 128);
    ps1GpuOtAddPrim(otSlot, sprt);
    pmTextX += PM_DRAW_W;
}

static void pmTextDrawStrN(uint8 **nextp, uint32 *otSlot,
                           const char *s, int maxChars)
{
    int count = 0;

    if (s == NULL)
        s = "(null)";
    while (*s && (maxChars < 0 || count < maxChars)) {
        if (*s == '\n' || *s == '\r') {
            s++;
            continue;
        }
        pmTextDrawChar(nextp, otSlot, *s);
        s++;
        count++;
    }
}

static int pmTextLenN(const char *s, int maxChars)
{
    int len = 0;

    if (s == NULL)
        s = "(null)";
    while (*s && (maxChars < 0 || len < maxChars)) {
        if (*s != '\n' && *s != '\r')
            len++;
        s++;
    }
    return len;
}

static void pmTextDrawPad(int count, char c)
{
    while (count-- > 0)
        pmTextDrawChar(&pmFramePrimNext, pmFrameOtSlot, c);
}

static int pmUnsignedDigits(unsigned long value, char *digits)
{
    char tmp[16];
    int count = 0;
    int out = 0;

    do {
        tmp[count++] = (char)('0' + (value % 10UL));
        value /= 10UL;
    } while (value != 0 && count < (int)sizeof(tmp));

    while (count > 0)
        digits[out++] = tmp[--count];
    digits[out] = '\0';
    return out;
}

static void pmTextDrawUnsigned(unsigned long value, char signChar,
                               int width, int leftJustify, int zeroPad)
{
    char digits[16];
    int digitCount = pmUnsignedDigits(value, digits);
    int total = digitCount + (signChar ? 1 : 0);
    int pad = (width > total) ? (width - total) : 0;
    int i;

    if (!leftJustify && !zeroPad)
        pmTextDrawPad(pad, ' ');
    if (signChar)
        pmTextDrawChar(&pmFramePrimNext, pmFrameOtSlot, signChar);
    if (!leftJustify && zeroPad)
        pmTextDrawPad(pad, '0');
    for (i = 0; i < digitCount; i++)
        pmTextDrawChar(&pmFramePrimNext, pmFrameOtSlot, digits[i]);
    if (leftJustify)
        pmTextDrawPad(pad, ' ');
}

static const char *pmParseFormat(const char *p, int *leftJustify,
                                 int *plusSign, int *zeroPad,
                                 int *width, int *precision,
                                 int *longFlag, char *spec)
{
    *leftJustify = 0;
    *plusSign = 0;
    *zeroPad = 0;
    *width = 0;
    *precision = -1;
    *longFlag = 0;

    for (;;) {
        if (*p == '-') {
            *leftJustify = 1;
            p++;
        } else if (*p == '+') {
            *plusSign = 1;
            p++;
        } else if (*p == '0') {
            *zeroPad = 1;
            p++;
        } else {
            break;
        }
    }

    while (*p >= '0' && *p <= '9') {
        *width = (*width * 10) + (*p - '0');
        p++;
    }

    if (*p == '.') {
        p++;
        *precision = 0;
        while (*p >= '0' && *p <= '9') {
            *precision = (*precision * 10) + (*p - '0');
            p++;
        }
    }

    if (*p == 'l') {
        *longFlag = 1;
        p++;
    }

    *spec = *p;
    if (*p != '\0')
        p++;
    return p;
}

/* Small menu-only printf subset. It supports the formats used by the
 * pause UI without pulling in the general formatter on PS1. */
static void pmPrintf(const char *fmt, ...)
{
    va_list ap;
    const char *p;

    if (fmt == NULL)
        fmt = "";

    pmTextX = pmPrintfX;
    va_start(ap, fmt);
    p = fmt;
    while (*p) {
        int leftJustify;
        int plusSign;
        int zeroPad;
        int width;
        int precision;
        int longFlag;
        char spec;

        if (*p == '\n' || *p == '\r') {
            p++;
            continue;
        }
        if (*p != '%') {
            pmTextDrawChar(&pmFramePrimNext, pmFrameOtSlot, *p++);
            continue;
        }

        p++;
        if (*p == '%') {
            pmTextDrawChar(&pmFramePrimNext, pmFrameOtSlot, '%');
            p++;
            continue;
        }

        p = pmParseFormat(p, &leftJustify, &plusSign, &zeroPad,
                          &width, &precision, &longFlag, &spec);
        if (spec == 's') {
            const char *s = va_arg(ap, const char *);
            int len = pmTextLenN(s, precision);
            int pad = (width > len) ? (width - len) : 0;
            if (!leftJustify)
                pmTextDrawPad(pad, ' ');
            pmTextDrawStrN(&pmFramePrimNext, pmFrameOtSlot, s, precision);
            if (leftJustify)
                pmTextDrawPad(pad, ' ');
        } else if (spec == 'd') {
            long value = longFlag ? va_arg(ap, long) : (long)va_arg(ap, int);
            unsigned long mag;
            char signChar = 0;
            if (value < 0) {
                signChar = '-';
                mag = 0UL - (unsigned long)value;
            } else {
                mag = (unsigned long)value;
                if (plusSign)
                    signChar = '+';
            }
            pmTextDrawUnsigned(mag, signChar, width, leftJustify, zeroPad);
        } else if (spec == 'u') {
            unsigned long value = longFlag
                ? va_arg(ap, unsigned long)
                : (unsigned long)va_arg(ap, unsigned int);
            pmTextDrawUnsigned(value, 0, width, leftJustify, zeroPad);
        } else if (spec != '\0') {
            pmTextDrawChar(&pmFramePrimNext, pmFrameOtSlot, '%');
            pmTextDrawChar(&pmFramePrimNext, pmFrameOtSlot, spec);
        }
    }
    va_end(ap);
    pmTextY += PM_LINE_STEP;
}

/* Panel geometry — rounded-rect look via 3 overlapping rectangles
 * with 8x8 corner cutouts that show the dim layer through. */
#define PM_PANEL_X0 60
#define PM_PANEL_Y0 60
#define PM_PANEL_X1 580
#define PM_PANEL_Y1 420
#define PM_CORNER 12

/* Build dim + 3 panel quads. Caller threads the buffer pointer +
 * separate OT slots so the chain order matches GPU draw order:
 *   slot N-2: dim   (drawn after TPAGE at N-1)
 *   slot N-3: panel quads
 *   slot N-4: text
 */
static void pmBuildPanelQuads(uint8 **nextp, uint32 *otDim, uint32 *otPanel)
{
    if (otDim != NULL) {
        /* Dim — full screen, semi-trans 50% black. Halves what's behind. */
        POLY_F4 *dim = (POLY_F4*)*nextp;
        *nextp += sizeof(POLY_F4);
        setPolyF4(dim);
        setSemiTrans(dim, 1);
        setRGB0(dim, 0, 0, 0);
        setXY4(dim,   0,   0,
                    640,   0,
                      0, 480,
                    640, 480);
        ps1GpuOtAddPrim(otDim, dim);
    }

    /* Panel: 3 rectangles. Middle full width, top/bottom narrower so
     * the corners stay as dim background — fakes rounded corners. */
    int x0 = PM_PANEL_X0, y0 = PM_PANEL_Y0;
    int x1 = PM_PANEL_X1, y1 = PM_PANEL_Y1;
    int c  = PM_CORNER;
    uint8 r = 0x40, g = 0x10, b = 0x60;  /* dark purple */

    /* Middle (full width, no corners). */
    POLY_F4 *p1 = (POLY_F4*)*nextp;
    *nextp += sizeof(POLY_F4);
    setPolyF4(p1);
    setRGB0(p1, r, g, b);
    setXY4(p1, x0, y0 + c, x1, y0 + c, x0, y1 - c, x1, y1 - c);
    ps1GpuOtAddPrim(otPanel, p1);

    /* Top edge (narrower). */
    POLY_F4 *p2 = (POLY_F4*)*nextp;
    *nextp += sizeof(POLY_F4);
    setPolyF4(p2);
    setRGB0(p2, r, g, b);
    setXY4(p2, x0 + c, y0, x1 - c, y0, x0 + c, y0 + c, x1 - c, y0 + c);
    ps1GpuOtAddPrim(otPanel, p2);

    /* Bottom edge (narrower). */
    POLY_F4 *p3 = (POLY_F4*)*nextp;
    *nextp += sizeof(POLY_F4);
    setPolyF4(p3);
    setRGB0(p3, r, g, b);
    setXY4(p3, x0 + c, y1 - c, x1 - c, y1 - c, x0 + c, y1, x1 - c, y1);
    ps1GpuOtAddPrim(otPanel, p3);
}

static void drawSeparator(void)
{
    pmPrintf(pmText(PMT_SEPARATOR));
}

/* ---------------------------------------------------------------------------
 *  Sub-screen: Debug Info
 * ------------------------------------------------------------------------- */
static void drawSceneInfo(void)
{
    const char *scene = foregroundPilotRuntimeSceneName();
    const char *mode  = foregroundPilotRuntimeModeName();
    uint16 fIdx = foregroundPilotRuntimeFrameIndex();
    uint16 fCnt = foregroundPilotRuntimeFrameCount();
    uint32 uptimeSec = ps1FrameCount / 60;

    pmPrintf(pmText(PMT_DEBUG_INFO_TITLE));
    drawSeparator();
    pmPrintf(pmText(PMT_DEBUG_SCENE), scene && *scene ? scene : pmText(PMT_NONE_PAREN));
    pmPrintf(pmText(PMT_DEBUG_MODE), mode ? mode : pmText(PMT_UNKNOWN));
    pmPrintf(pmText(PMT_DEBUG_FRAME), (unsigned)fIdx, (unsigned)fCnt);
    pmPrintf(pmText(PMT_DEBUG_HEAP), pmCachedHeapKB);
    pmPrintf(pmText(PMT_DEBUG_UPTIME),
             (unsigned long)(uptimeSec / 60), (unsigned long)(uptimeSec % 60));
    pmPrintf(pmText(PMT_DEBUG_BUILD), PS1_EMBEDDED_BUILD_DATE);
    pmPrintf(pmText(PMT_DEBUG_PERF), perfLevelLabel());
    pmPrintf(pmText(PMT_DEBUG_SOUND), soundMuted ? pmText(PMT_MUTED) : pmText(PMT_ON));
    pmPrintf(pmText(PMT_DEBUG_SAVE), memcardLastStatus ? memcardLastStatus : pmText(PMT_NONE_PAREN));
    drawSeparator();
    pmPrintf(pmText(PMT_O_START_BACK_CAPS));
}
