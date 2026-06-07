/* ---------------------------------------------------------------------------
 *  Drawing helpers -- all use FntPrint into the existing fontID stream
 * ------------------------------------------------------------------------- */

/* MANUAL SPRT TEXT RENDERER.
 *
 * Bypasses PSn00bSDK's broken FntFlush by building SPRT primitives
 * ourselves, pointing at the BIOS font texture that FntLoad(960, 0)
 * uploaded to VRAM. The BIOS font is 16 chars wide × 16 chars tall,
 * 8x8 each (256 chars total), stored 4-bit packed.
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

static void pmTextDrawStr(uint8 **nextp, uint32 *otSlot, const char *s)
{
    while (*s) {
        pmTextDrawChar(nextp, otSlot, *s);
        s++;
    }
}

/* printf-style helper that uses the per-frame globals. Newline at end
 * advances pmTextY by PM_LINE_STEP and resets pmTextX to the column
 * the line started in (set by the caller before this call). */
static void pmPrintf(const char *fmt, ...)
{
    char buf[80];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    /* Strip trailing newlines — pmPrintf advances Y itself. */
    int len = (int)strlen(buf);
    while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r'))
        buf[--len] = '\0';
    /* Reset X to line start. */
    pmTextX = pmPrintfX;
    pmTextDrawStr(&pmFramePrimNext, pmFrameOtSlot, buf);
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
    pmPrintf("----------------------------\n");
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

    pmPrintf("    DEBUG INFO");
    drawSeparator();
    pmPrintf(" Scene:  %s",   scene && *scene ? scene : "(none)");
    pmPrintf(" Mode:   %s",   mode  ? mode  : "?");
    pmPrintf(" Frame:  %u/%u", (unsigned)fIdx, (unsigned)fCnt);
    pmPrintf(" Heap:   %lu KB free", pmCachedHeapKB);
    pmPrintf(" Uptime: %lu:%02lu",
             (unsigned long)(uptimeSec / 60), (unsigned long)(uptimeSec % 60));
    pmPrintf(" Build:  %s", PS1_EMBEDDED_BUILD_DATE);
    pmPrintf(" Perf:   %s", perfLevelLabel());
    pmPrintf(" Sound:  %s", soundMuted ? "MUTED" : "ON");
    pmPrintf(" Save:   %s", memcardLastStatus ? memcardLastStatus : "(none)");
    drawSeparator();
    pmPrintf(" O/START = BACK");
}
