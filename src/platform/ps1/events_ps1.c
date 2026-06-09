/*
 *  This file is part of 'Johnny Reborn' - PS1 Port
 *
 *  PlayStation 1 input/event handling using PSn00bSDK
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

#include <psxpad.h>
#include <psxapi.h>
#include <psxgpu.h>
#include <stdio.h>
#include <string.h>

#include "mytypes.h"
#include "events_ps1.h"
#include "pause_menu.h"
#include "config.h"
#include "spi.h"
#include "ps1_pad_input.h"

#ifndef PS1_VERBOSE_DIAGNOSTICS
#define PS1_VERBOSE_DIAGNOSTICS 0
#endif

/* Global variables */
int evHotKeysEnabled = 0;
int evPadDiagnosticsEnabled = 0;

/* Controller data buffers — non-static so pause_menu.c can read the
 * same shared pad state via `extern uint8 pad_buff[2][34];`. */
uint8 pad_buff[2][34];

#if PS1_VERBOSE_DIAGNOSTICS
/* JCSPI persistent btn tracker — watches every SPI poll (250 Hz)
 * and remembers extreme values + change count across the whole run.
 * If btn_lo ever drops below 0xFF the user pressed something; if it
 * stays 0xFF for thousands of polls the bytes never carry button
 * state regardless of human input timing. */
volatile uint8  spi_btn_min_lo = 0xFF;
volatile uint8  spi_btn_min_hi = 0xFF;
volatile uint8  spi_btn_max_lo = 0;
volatile uint8  spi_btn_max_hi = 0;
volatile uint32 spi_btn_change_count = 0;
volatile uint8  spi_btn_last_lo = 0xFF;
volatile uint8  spi_btn_last_hi = 0xFF;
volatile uint32 spi_btn_p0_polls = 0;
volatile uint32 spi_btn_p0_nonff = 0;
#endif

static uint16 delayResidue = 0;
static uint32 lastFrameTick = 0;

#if PS1_VERBOSE_DIAGNOSTICS
/* JCSPI T14: read MIPS COP0 STATUS register (SR). Bit 0 = IEc (current
 * interrupt enable). If 0, CPU-level IRQs are gated and nothing fires. */
static uint32 cop0_read_sr(void)
{
    uint32 v;
    __asm__ volatile("mfc0 %0, $12" : "=r"(v));
    return v;
}

/* JCSPI T22: read COP0 CAUSE register. Bits 6:2 = ExcCode, bits 15:8
 * = pending IRQ bits. Non-zero ExcCode means an exception happened. */
static uint32 cop0_read_cause(void)
{
    uint32 v;
    __asm__ volatile("mfc0 %0, $13" : "=r"(v));
    return v;
}

/* JCSPI T22: read COP0 EPC register. PC at the time of the last
 * exception. Useful for diagnosing where a fault landed. */
static uint32 cop0_read_epc(void)
{
    uint32 v;
    __asm__ volatile("mfc0 %0, $14" : "=r"(v));
    return v;
}

/* JCSPI T15: VBlank baseline counter. ps1_perf-style hook is too
 * heavy; we just count VBlank ticks via VSync(-1) reads in the main
 * loop. If VBlank rate >> SPI poll rate, IRQ subsystem favors VBlank
 * over timer 2 (could be priority issue). */
static uint32 vbl_seen_at_last_snapshot = 0;

/* JCSPI T21: baseline main-loop $gp captured ONCE in eventsInit, before
 * any IRQs hit. Later we compare against gp captured at IRQ entry — if
 * they differ, the BIOS dispatch isn't restoring gp and our handler
 * can't access globals. */
static uint32 main_gp_baseline = 0;
static uint32 main_sp_baseline = 0;
#endif

void eventsSetPadDiagnostics(int enabled)
{
#if PS1_VERBOSE_DIAGNOSTICS
    evPadDiagnosticsEnabled = enabled ? 1 : 0;
#else
    (void)enabled;
    evPadDiagnosticsEnabled = 0;
#endif
}

#if PS1_VERBOSE_DIAGNOSTICS
/*
 * Synchronous SIO0 hardware probe — IRQ-free.
 *
 * Bypasses our SPI driver entirely and manually drives SIO0 to confirm
 * the bus + emulated controller respond at all. If even THIS times out,
 * the issue is downstream of any IRQ wiring (DuckStation port config,
 * SIO emulation, missing controller), and every IRQ-based theory is
 * moot. If this returns valid bytes, hardware is alive and the IRQ
 * path is the suspect.
 *
 * Sequence:
 *   1. Reset SIO0, configure MODE/BAUD as SPI driver does.
 *   2. /CS low for port 0 (no /ACK IRQ enable).
 *   3. Drain any pending RX bytes.
 *   4. Write 0x01 (address byte).
 *   5. Spin-poll SIO_STAT bit 1 (RX FIFO not empty) up to ~5 ms.
 *   6. Read SIO_DATA — for a connected digital pad this should be 0xFF
 *      (open-bus initial response).
 *   7. /CS high, reset.
 *
 * Run BEFORE SPI_Init so we don't fight the timer-2 IRQ mid-probe.
 */
static void jcspiSyncProbe(void)
{
    /* Bring includes for hwregs in via spi.h's transitive include. */
    #define _SIO_CTRL ((volatile uint16_t *) 0xBF80104A)
    #define _SIO_MODE ((volatile uint16_t *) 0xBF801048)
    #define _SIO_BAUD ((volatile uint16_t *) 0xBF80104E)
    #define _SIO_STAT ((volatile uint32_t *) 0xBF801044)
    #define _SIO_DATA ((volatile uint32_t *) 0xBF801040)

    /* Mask all IRQs while we drive the bus manually. */
    EnterCriticalSection();

    /* Reset and configure. */
    *_SIO_CTRL = 0x0040;
    for (volatile int i = 0; i < 200; i++) __asm__ volatile("");
    *_SIO_MODE = 0x000d;
    *_SIO_BAUD = 0x0088;

    /* /CS low for port 0, TX+RX enabled, NO /ACK IRQ enable
     * (we don't want SIO0 IRQ firing in masked context). */
    *_SIO_CTRL = 0x0003;
    for (volatile int i = 0; i < 1000; i++) __asm__ volatile("");

    uint32_t stat_pre  = *_SIO_STAT;

    /* Drain any open-bus byte from RX FIFO. */
    if (stat_pre & 0x0002) (void)*_SIO_DATA;

    /* Send the address byte 0x01 (controller addr). */
    *_SIO_DATA = 0x01;
    uint32_t stat_after_w = *_SIO_STAT;

    /* Spin until RX FIFO has a byte, with timeout. */
    int rx_wait = 0;
    while (rx_wait < 50000 && !(*_SIO_STAT & 0x0002)) {
        rx_wait++;
    }
    uint32_t stat_with_rx = *_SIO_STAT;
    uint8_t byte0 = 0xEE;  /* sentinel for "never received" */
    if (*_SIO_STAT & 0x0002)
        byte0 = (uint8_t)*_SIO_DATA;

    /* Try a second byte (0x42 = pad read command) so we can also
     * see if the bus clocks a second transaction. */
    *_SIO_DATA = 0x42;
    int rx_wait2 = 0;
    while (rx_wait2 < 50000 && !(*_SIO_STAT & 0x0002)) {
        rx_wait2++;
    }
    uint8_t byte1 = 0xEE;
    if (*_SIO_STAT & 0x0002)
        byte1 = (uint8_t)*_SIO_DATA;

    /* Reset SIO0 to a known state. SPI_Init will reconfigure. */
    *_SIO_CTRL = 0x0040;

    ExitCriticalSection();

    printf("JCSPI SYNC stat_pre=%04lx stat_after_w=%04lx stat_with_rx=%04lx "
           "byte0=%02x byte1=%02x rx_wait=%d rx_wait2=%d %s\n",
           (unsigned long)stat_pre, (unsigned long)stat_after_w,
           (unsigned long)stat_with_rx,
           byte0, byte1, rx_wait, rx_wait2,
           (rx_wait < 50000) ? "HW_OK" : "HW_TIMEOUT");

    #undef _SIO_CTRL
    #undef _SIO_MODE
    #undef _SIO_BAUD
    #undef _SIO_STAT
    #undef _SIO_DATA
}
#endif

static uint16 eventsDelayTicksToTargetVBlanks(uint16 delay)
{
    uint32 scaled;
    uint16 targetVBlanks;

    if (delay == 0)
        return 0;

    /* PC ADS/TTM delay ticks are 20 ms. Convert to NTSC 60 Hz VBlanks with
     * persistent carry so repeated timing blocks do not drift. */
    scaled = ((uint32)delay * 6u) + (uint32)delayResidue;
    targetVBlanks = (uint16)(scaled / 5u);
    delayResidue = (uint16)(scaled % 5u);
    return targetVBlanks > 0 ? targetVBlanks : 1;
}

/*
 * SPI poll callback — translates a raw SPI controller response into the
 * PADTYPE-format byte layout that consumers (pause_menu.c, padtest path)
 * expect at pad_buff[port].
 *
 * The SPI ack handler discards the open-bus byte that comes back in
 * response to the 0x01 address byte (`if (!rx_len) SIO_DATA(0);`), so
 * the rx_buff layout for a digital pad is:
 *
 *   buff[0] = 0x41   (digital ID: type=4, len=1)
 *   buff[1] = 0x5A   (always 0x5A marker, ignored)
 *   buff[2] = btn low byte (active low)
 *   buff[3] = btn high byte (active low)
 *
 * For DualShock / analog response, additional bytes follow at buff[4..].
 *
 * PADTYPE layout in our pad_buff[port]:
 *   pad_buff[port][0] = stat    (0x00 connected, 0xFF disconnected)
 *   pad_buff[port][1] = type:len byte (matches buff[0])
 *   pad_buff[port][2] = btn lo
 *   pad_buff[port][3] = btn hi
 *   pad_buff[port][4..]         (analog axes for DualShock; zero for digital)
 *
 * Gate: a complete digital pad response fills exactly 4 rx bytes (after
 * the addr-response discard). Anything less means the port has no
 * responding device — write the disconnected pattern.
 */
/* Non-static so memcard.c can re-arm SPI_Init with this callback after
 * temporarily handing SIO0 to the BIOS card driver for a save/load. */
void eventsSpiPollCallback(uint32_t port, const volatile uint8_t *buff, size_t rx_len)
{
    if (port > 1)
        return;

    uint8 *dst = pad_buff[port];

    if (rx_len < 4) {
        /* Disconnected / partial response. Mark as no controller. */
        dst[0] = 0xFF;
        dst[1] = 0x00;
        dst[2] = 0xFF;
        dst[3] = 0xFF;
        return;
    }

    /* Connected. Translate raw SPI bytes → PADTYPE bytes. */
    dst[0] = 0x00;             /* stat: connected */
    dst[1] = buff[0];          /* type:len byte (e.g. 0x41 for digital) */
    dst[2] = buff[2];          /* btn low */
    dst[3] = buff[3];          /* btn high */

#if PS1_VERBOSE_DIAGNOSTICS
    /* JCSPI persistent btn tracker is diagnostic-only. Pause reads pad_buff
     * directly and does not need these volatile counters on every SPI IRQ. */
    if (evPadDiagnosticsEnabled && port == 0) {
        spi_btn_p0_polls++;
        if (buff[2] != 0xFF || buff[3] != 0xFF)
            spi_btn_p0_nonff++;
        if (buff[2] < spi_btn_min_lo) spi_btn_min_lo = buff[2];
        if (buff[2] > spi_btn_max_lo) spi_btn_max_lo = buff[2];
        if (buff[3] < spi_btn_min_hi) spi_btn_min_hi = buff[3];
        if (buff[3] > spi_btn_max_hi) spi_btn_max_hi = buff[3];
        if (buff[2] != spi_btn_last_lo || buff[3] != spi_btn_last_hi) {
            spi_btn_change_count++;
            spi_btn_last_lo = buff[2];
            spi_btn_last_hi = buff[3];
        }
    }
#endif

    /* Copy remaining bytes verbatim (analog axes etc., DualShock).
     * Cap at 30 bytes after the 4-byte header — pad_buff is 34 total. */
    if (rx_len > 4) {
        size_t extra = rx_len - 4;
        if (extra > 30) extra = 30;
        for (size_t i = 0; i < extra; i++)
            dst[4 + i] = buff[4 + i];
    }
}

/*
 * Initialize input system.
 *
 * Uses direct SPI controller polling (PSn00bSDK examples/io/pads/spi.c).
 * The BIOS pad system (InitPAD/StartPAD/ChangeClearPAD) was tested
 * empirically and never auto-polls in our PSn00bSDK 0.24 + DuckStation
 * environment — pad->btn freezes at 0xFFFF after the first poll. The SPI
 * driver bypasses the BIOS by hooking timer 2 + SIO IRQs directly and
 * polls at 250 Hz (125 Hz per port).
 */
void eventsInit()
{
    delayResidue = 0;
    lastFrameTick = (uint32)VSync(-1);

#if PS1_VERBOSE_DIAGNOSTICS
    if (evPadDiagnosticsEnabled) {
        /* JCSPI T21 baseline: capture main-loop $gp / $sp BEFORE anything
         * else. Compared later against $gp/$sp captured at IRQ entry. If
         * the BIOS dispatch isn't restoring gp, IRQ-time gp will differ
         * and globals (counters!) point to garbage. */
        __asm__ volatile("move %0, $gp" : "=r"(main_gp_baseline));
        __asm__ volatile("move %0, $sp" : "=r"(main_sp_baseline));
        printf("JCSPI T21 baseline main_gp=%08lx main_sp=%08lx\n",
               (unsigned long)main_gp_baseline, (unsigned long)main_sp_baseline);

        /* JCSPI T-NEW: PADTYPE layout probe. type/len are bit-fields in
         * PSn00bSDK 0.24, so we can't take their address. Instead, write
         * known sentinels into pad_buff[0][0..7] and read back through
         * the PADTYPE struct. This tells us exactly which bytes pad->stat
         * and pad->btn map to. If pad->btn != 0xDDCC (little-endian read
         * of bytes 2,3) we have an offset mismatch and our SPI callback
         * is writing btn into the wrong slot. */
        pad_buff[0][0] = 0xAA;
        pad_buff[0][1] = 0xBB;
        pad_buff[0][2] = 0xCC;
        pad_buff[0][3] = 0xDD;
        pad_buff[0][4] = 0xEE;
        pad_buff[0][5] = 0xFF;
        pad_buff[0][6] = 0x11;
        pad_buff[0][7] = 0x22;
        PADTYPE *p = (PADTYPE*)pad_buff[0];
        printf("JCSPI PADTYPE probe sizeof=%lu sentinels[0..7]=AA BB CC DD EE FF 11 22 → stat=%02x btn=%04x\n",
               (unsigned long)sizeof(PADTYPE),
               (unsigned)p->stat, (unsigned)p->btn);
        printf("JCSPI PADTYPE expected: stat=AA btn=DDCC (if btn at off 2 little-endian) — mismatch means LAYOUT BUG\n");
    }
#endif

    /* Initialize pad_buff to a safe disconnected-controller state so any
     * reads that happen before the first SPI poll fires give 0xFFFF (no
     * buttons) instead of stale memory. */
    for (int p = 0; p < 2; p++) {
        pad_buff[p][0] = 0xFF;     /* stat: disconnected */
        pad_buff[p][1] = 0x00;
        pad_buff[p][2] = 0xFF;     /* btn = 0xFFFF (no buttons) */
        pad_buff[p][3] = 0xFF;
        for (int i = 4; i < 34; i++) pad_buff[p][i] = 0x00;
    }

#if PS1_VERBOSE_DIAGNOSTICS
    if (evPadDiagnosticsEnabled) {
        printf("JCPAD eventsInit ENTER (SPI driver) pad_buff[0]=%p pad_buff[1]=%p\n",
               (void*)pad_buff[0], (void*)pad_buff[1]);
        printf("JCPAD bits: PAD_START=%04x PAD_CROSS=%04x PAD_SELECT=%04x PAD_TRIANGLE=%04x\n",
               (unsigned)PAD_START, (unsigned)PAD_CROSS, (unsigned)PAD_SELECT, (unsigned)PAD_TRIANGLE);

        /* JCSPI T14/T22: COP0 SR/CAUSE/EPC at boot. SR bit 0 = IEc must be 1
         * for IRQs to fire. CAUSE bits 6:2 = last exception code (should be 0). */
        printf("JCSPI T14 cop0 sr=%08lx cause=%08lx epc=%08lx (sr.IEc=%d sr.IM=%02lx cause.ExcCode=%lu)\n",
               (unsigned long)cop0_read_sr(), (unsigned long)cop0_read_cause(),
               (unsigned long)cop0_read_epc(),
               (int)(cop0_read_sr() & 1),
               (unsigned long)((cop0_read_sr() >> 8) & 0xFF),
               (unsigned long)((cop0_read_cause() >> 2) & 0x1F));
    }
#endif

    /* JCSPI: snapshot driver state IMMEDIATELY before SPI_Init for a
     * baseline. After SPI_Init, snapshot again so we can prove the
     * registers we want set are actually set. */
#if PS1_VERBOSE_DIAGNOSTICS
    if (evPadDiagnosticsEnabled) {
        SPI_DbgState pre, post;
        SPI_DbgSnapshot(&pre);
        printf("JCSPI PRE  irq_mask=%08lx irq_stat=%08lx sio_ctrl=%08lx sio_mode=%08lx sio_baud=%08lx "
               "tmr_ctrl=%08lx tmr_reload=%08lx default_cb=%08lx\n",
               (unsigned long)pre.irq_mask, (unsigned long)pre.irq_stat,
               (unsigned long)pre.sio_ctrl, (unsigned long)pre.sio_mode, (unsigned long)pre.sio_baud,
               (unsigned long)pre.timer_ctrl, (unsigned long)pre.timer_reload,
               (unsigned long)pre.default_cb);

        /* JCSPI sync HW probe — IRQ-free SIO0 round-trip. Tests whether
         * the SIO bus + DuckStation's emulated controller respond at all.
         * If this prints HW_OK, hardware is alive and the issue is in
         * IRQ wiring. If HW_TIMEOUT, the bus or port isn't reachable. */
        jcspiSyncProbe();

        /* SPI_Init installs the timer-2 + SIO0 ack IRQ handlers and starts
         * polling at the default 250 Hz. The callback is invoked each time
         * a poll completes (alternating ports each tick). */
        SPI_Init(eventsSpiPollCallback);

        SPI_DbgSnapshot(&post);
        printf("JCSPI POST irq_mask=%08lx irq_stat=%08lx sio_ctrl=%08lx sio_mode=%08lx sio_baud=%08lx "
               "tmr_ctrl=%08lx tmr_reload=%08lx tmr_value=%08lx default_cb=%08lx\n",
               (unsigned long)post.irq_mask, (unsigned long)post.irq_stat,
               (unsigned long)post.sio_ctrl, (unsigned long)post.sio_mode, (unsigned long)post.sio_baud,
               (unsigned long)post.timer_ctrl, (unsigned long)post.timer_reload,
               (unsigned long)post.timer_value, (unsigned long)post.default_cb);

        /* JCSPI T1: report whether IRQ_MASK has bits 6 (timer 2) and 7
         * (SIO0/controller) set. PSn00bSDK's InterruptCallback should
         * unmask these. If not set, the IRQ never fires. */
        printf("JCSPI T1 irq_mask bit6(tmr2)=%d bit7(sio0)=%d\n",
               (post.irq_mask & 0x40) ? 1 : 0,
               (post.irq_mask & 0x80) ? 1 : 0);

        /* JCSPI T9: report computed reload value sanity. F_CPU=33868800,
         * /8=4233600, /250=16934 (0x4226). If reload != 0x4226 the
         * write didn't land or F_CPU was wrong. */
        printf("JCSPI T9 timer_reload expected=0x4226 actual=0x%04lx %s\n",
               (unsigned long)(post.timer_reload & 0xFFFF),
               ((post.timer_reload & 0xFFFF) == 0x4226) ? "OK" : "MISMATCH");

        /* JCSPI T7: report whether the default callback pointer made it
         * into the driver context. If 0, SPI_Init didn't actually wire
         * our callback. */
        printf("JCSPI T7 default_cb=%08lx expected=%08lx %s\n",
               (unsigned long)post.default_cb,
               (unsigned long)(uint32_t)eventsSpiPollCallback,
               (post.default_cb == (uint32_t)eventsSpiPollCallback) ? "OK" : "MISMATCH");

        /* JCSPI T16: cached/uncached check on _context. PS1 KSEG0 is
         * 0x80000000-0x9FFFFFFF (cached), KSEG1 is 0xA0000000-0xBFFFFFFF
         * (uncached/io), KUSEG is 0x00000000-0x7FFFFFFF (cached, user).
         * Driver expects KSEG0 (cached) for normal RAM. */
        const char *seg = "KUSEG";
        if ((post.ctx_addr & 0xE0000000) == 0x80000000) seg = "KSEG0(cached)";
        else if ((post.ctx_addr & 0xE0000000) == 0xA0000000) seg = "KSEG1(uncached)";
        printf("JCSPI T16 ctx_addr=%08lx tx_buff=%08lx rx_buff=%08lx seg=%s\n",
               (unsigned long)post.ctx_addr, (unsigned long)post.tx_buff_addr,
               (unsigned long)post.rx_buff_addr, seg);

        /* JCSPI T19: handler addresses we wrote. */
        printf("JCSPI T19 poll_handler=%08lx ack_handler=%08lx\n",
               (unsigned long)post.poll_handler, (unsigned long)post.ack_handler);

        /* JCSPI T17: DMA control register state. DMA_DPCR at 0x1F8010F0
         * controls per-channel enable/priority. DMA_DICR at 0x1F8010F4
         * is the IRQ enable/status. SIO0 doesn't use DMA but bus
         * contention from active GPU/SPU DMA could stall SIO. */
        printf("JCSPI T17 DMA dpcr=%08lx dicr=%08lx\n",
               (unsigned long)*(volatile uint32_t*)0xBF8010F0,
               (unsigned long)*(volatile uint32_t*)0xBF8010F4);
    } else {
#endif
        SPI_Init(eventsSpiPollCallback);
#if PS1_VERBOSE_DIAGNOSTICS
    }
    if (evPadDiagnosticsEnabled)
        printf("JCPAD SPI_Init called — driver polling at 250 Hz (125 Hz per port)\n");
#endif
}

#if PS1_VERBOSE_DIAGNOSTICS
/* JCPAD diagnostic state — checked + printed by eventsWaitTick. */
static uint32 padDiagCalls = 0;
static uint16 padDiagLastBtn0 = 0xFFFF;
static uint16 padDiagLastBtn1 = 0xFFFF;
static uint8  padDiagLastStat0 = 0xFF;
static uint16 padDiagMinBtn0 = 0xFFFF;
static uint16 padDiagMaxBtn0 = 0;
static int    padDiagBtnEverChanged0 = 0;
static int    padDiagBtnEverChanged1 = 0;
static int    padDiagStatEverChanged0 = 0;
static int    padDiagStartEverSeen = 0;
static int    padDiagAnyPressedEverSeen = 0;
#endif

/*
 * Wait for specified number of ticks (frame timing).
 *
 * Also the universal pause-trigger point: every scene runtime calls
 * grUpdateDisplay → eventsWaitTick on every rendered frame, so reading
 * Start here covers all scenes uniformly. While the menu is open we
 * own the foreground render via pauseMenuUpdate(); on resume the runtime
 * resumes its normal pacing.
 */
void eventsWaitTick(uint16 delay)
{
#if PS1_VERBOSE_DIAGNOSTICS
    /* JCPAD diagnostics are intentionally opt-in. Normal playback still
     * polls Start below, but does not log or sample debug registers. */
    if (evPadDiagnosticsEnabled) {
        PADTYPE *pad0 = (PADTYPE*)pad_buff[0];
        PADTYPE *pad1 = (PADTYPE*)pad_buff[1];
        uint8 *raw0 = pad_buff[0];
        uint8 *raw1 = pad_buff[1];
        uint16 cur0 = pad0->btn;
        uint16 cur1 = pad1->btn;
        uint16 inv0 = (uint16)~cur0;

        padDiagCalls++;

        /* Track EVERY change (port 0) — diagnostic phase. If user presses
         * buttons and the pad poll is alive, we'll see flapping here. */
        if (cur0 != padDiagLastBtn0) {
            padDiagBtnEverChanged0 = 1;
            if (cur0 < padDiagMinBtn0) padDiagMinBtn0 = cur0;
            if (cur0 > padDiagMaxBtn0) padDiagMaxBtn0 = cur0;
            printf("JCPAD CHANGE p0 btn %04x → %04x inv=%04x (call #%lu)\n",
                   padDiagLastBtn0, cur0, (uint16)~cur0,
                   (unsigned long)padDiagCalls);
            padDiagLastBtn0 = cur0;
        }
        if (cur1 != padDiagLastBtn1) {
            padDiagBtnEverChanged1 = 1;
            printf("JCPAD CHANGE p1 btn %04x → %04x (call #%lu)\n",
                   padDiagLastBtn1, cur1, (unsigned long)padDiagCalls);
            padDiagLastBtn1 = cur1;
        }
        if (pad0->stat != padDiagLastStat0) {
            padDiagStatEverChanged0 = 1;
            printf("JCPAD STAT CHANGE p0 stat %02x → %02x (call #%lu)\n",
                   padDiagLastStat0, pad0->stat, (unsigned long)padDiagCalls);
            padDiagLastStat0 = pad0->stat;
        }
        if (inv0 != 0 && !padDiagAnyPressedEverSeen) {
            padDiagAnyPressedEverSeen = 1;
            printf("JCPAD FIRST PRESS p0 inv=%04x (any button at all) call #%lu\n",
                   inv0, (unsigned long)padDiagCalls);
        }
        if ((inv0 & PAD_START) && !padDiagStartEverSeen) {
            padDiagStartEverSeen = 1;
            printf("JCPAD FIRST START SEEN call #%lu inv=%04x\n",
                   (unsigned long)padDiagCalls, inv0);
        }

        /* Periodic snapshot — once per ~30 scene frames so SPI counters
         * (which tick at 250 Hz independent of scene rate) advance enough
         * between prints to be informative. */
        if ((padDiagCalls % 30) == 0) {
            printf("JCPAD #%lu p0[stat=%02x type=%02x btn=%04x inv=%04x] p1[stat=%02x btn=%04x] "
                   "raw0=%02x %02x %02x %02x %02x %02x %02x %02x  "
                   "changed0=%d changed1=%d statchg0=%d anypress=%d startseen=%d "
                   "min=%04x max=%04x\n",
                   (unsigned long)padDiagCalls,
                   pad0->stat, pad0->type, cur0, inv0,
                   pad1->stat, cur1,
                   raw0[0], raw0[1], raw0[2], raw0[3], raw0[4], raw0[5], raw0[6], raw0[7],
                   padDiagBtnEverChanged0, padDiagBtnEverChanged1, padDiagStatEverChanged0,
                   padDiagAnyPressedEverSeen, padDiagStartEverSeen,
                   padDiagMinBtn0, padDiagMaxBtn0);
            (void)raw1;

            /* JCSPI: live driver + register snapshot. Disambiguates
             * the "callback never fired" cases. */
            SPI_DbgState s;
            SPI_DbgSnapshot(&s);
            uint32 cur_vbl = (uint32)VSync(-1);
            uint32 vbl_delta = cur_vbl - vbl_seen_at_last_snapshot;
            vbl_seen_at_last_snapshot = cur_vbl;

            printf("JCSPI #%lu cnt[poll=%lu ack=%lu cb=%lu rxlen=%lu] "
                   "vbl_delta=%lu vbl_total=%lu "
                   "irq[mask=%08lx stat=%08lx] "
                   "sio[ctrl=%04lx mode=%04lx baud=%04lx stat=%04lx] "
                   "tmr[ctrl=%04lx reload=%04lx value=%04lx] "
                   "ctx[port=%lu txlen=%lu rxlen=%lu cb=%08lx] "
                   "tx=%02x %02x %02x %02x rx=%02x %02x %02x %02x %02x\n",
                   (unsigned long)padDiagCalls,
                   (unsigned long)s.poll_count, (unsigned long)s.ack_count,
                   (unsigned long)s.cb_count, (unsigned long)s.last_rxlen,
                   (unsigned long)vbl_delta, (unsigned long)cur_vbl,
                   (unsigned long)s.irq_mask, (unsigned long)s.irq_stat,
                   (unsigned long)s.sio_ctrl, (unsigned long)s.sio_mode,
                   (unsigned long)s.sio_baud, (unsigned long)s.sio_stat,
                   (unsigned long)s.timer_ctrl, (unsigned long)s.timer_reload,
                   (unsigned long)s.timer_value,
                   (unsigned long)s.ctx_port, (unsigned long)s.ctx_tx_len,
                   (unsigned long)s.ctx_rx_len, (unsigned long)s.default_cb,
                   s.tx0, s.tx1, s.tx2, s.tx3,
                   s.rx0, s.rx1, s.rx2, s.rx3, s.rx4);

            /* JCSPI T18/T21: snapshot taken AT IRQ ENTRY. */
            printf("JCSPI #%lu @irq[sio_ctrl=%04lx sio_stat=%04lx irq_stat=%08lx gp=%08lx sp=%08lx]\n",
                   (unsigned long)padDiagCalls,
                   (unsigned long)s.at_irq_sio_ctrl, (unsigned long)s.at_irq_sio_stat,
                   (unsigned long)s.at_irq_irq_stat,
                   (unsigned long)s.at_irq_gp, (unsigned long)s.at_irq_sp);

            /* JCSPI T14/T22: live COP0 readback. SR.IEc must be 1, no
             * pending exception. */
            printf("JCSPI #%lu cop0[sr=%08lx cause=%08lx epc=%08lx IEc=%d ExcCode=%lu]\n",
                   (unsigned long)padDiagCalls,
                   (unsigned long)cop0_read_sr(),
                   (unsigned long)cop0_read_cause(),
                   (unsigned long)cop0_read_epc(),
                   (int)(cop0_read_sr() & 1),
                   (unsigned long)((cop0_read_cause() >> 2) & 0x1F));

            /* JCSPI: rx history — last 4 snapshots of raw response. */
            printf("JCSPI #%lu rx_hist (count=%lu):\n",
                   (unsigned long)padDiagCalls, (unsigned long)s.rx_hist_count);
            for (int slot = 0; slot < SPI_DBG_RX_HIST_SLOTS; slot++) {
                printf("  slot[%d] port=%lu rxlen=%lu  %02x %02x %02x %02x %02x %02x %02x %02x\n",
                       slot, (unsigned long)s.rx_hist_port[slot],
                       (unsigned long)s.rx_hist_rxlen[slot],
                       s.rx_hist[slot][0], s.rx_hist[slot][1], s.rx_hist[slot][2], s.rx_hist[slot][3],
                       s.rx_hist[slot][4], s.rx_hist[slot][5], s.rx_hist[slot][6], s.rx_hist[slot][7]);
            }

            /* JCSPI: pad_buff[0] full 34-byte hex dump (memory frame). */
            {
                uint8 *p = pad_buff[0];
                printf("JCSPI #%lu pad_buff[0]:\n  ", (unsigned long)padDiagCalls);
                for (int i = 0; i < 34; i++) {
                    printf("%02x ", p[i]);
                    if ((i & 0x0F) == 0x0F) printf("\n  ");
                }
                printf("\n");
            }

            /* JCSPI: pad_buff[1] full 34-byte hex dump. If port-flip
             * works in the SPI driver (T11 variant) we'd see DIFFERENT
             * content here vs pad_buff[0]. */
            {
                uint8 *p = pad_buff[1];
                printf("JCSPI #%lu pad_buff[1]:\n  ", (unsigned long)padDiagCalls);
                for (int i = 0; i < 34; i++) {
                    printf("%02x ", p[i]);
                    if ((i & 0x0F) == 0x0F) printf("\n  ");
                }
                printf("\n");
            }

            /* JCSPI T21: compare IRQ-time gp/sp against main-loop
             * baseline. If gp differs, BIOS dispatch isn't restoring
             * it and our IRQ globals are reading garbage. */
            if (s.at_irq_gp != 0 || s.at_irq_sp != 0) {
                printf("JCSPI T21 gp_match=%d sp_close=%d (main_gp=%08lx irq_gp=%08lx, "
                       "main_sp=%08lx irq_sp=%08lx)\n",
                       (s.at_irq_gp == main_gp_baseline) ? 1 : 0,
                       /* IRQ stack typically differs by a few hundred bytes — flag if it's the same word as main */
                       (s.at_irq_sp == main_sp_baseline) ? 1 : 0,
                       (unsigned long)main_gp_baseline, (unsigned long)s.at_irq_gp,
                       (unsigned long)main_sp_baseline, (unsigned long)s.at_irq_sp);
            }

            /* JCSPI persistent btn tracker — full-rate (250 Hz) view of
             * btn bytes from port 0 polls across the WHOLE run. If
             * spi_btn_p0_nonff stays 0 across many thousands of polls,
             * the controller really never sent non-FF btn bytes. If
             * spi_btn_p0_nonff > 0, button data IS reaching the SPI
             * driver — investigate downstream of the callback. */
            printf("JCSPI BTNTRK p0_polls=%lu p0_nonff=%lu changes=%lu min[lo=%02x hi=%02x] max[lo=%02x hi=%02x] last[lo=%02x hi=%02x]\n",
                   (unsigned long)spi_btn_p0_polls,
                   (unsigned long)spi_btn_p0_nonff,
                   (unsigned long)spi_btn_change_count,
                   spi_btn_min_lo, spi_btn_min_hi,
                   spi_btn_max_lo, spi_btn_max_hi,
                   spi_btn_last_lo, spi_btn_last_hi);

            /* JCSPI verdict: classify which theory the data points to. */
            const char *verdict = "?";
            if (s.poll_count == 0 && s.ack_count == 0 && s.cb_count == 0) {
                verdict = "T1/T8/T14/T20: timer-2 IRQ never fires (mask/InterruptCallback/COP0/init-order)";
            } else if (s.poll_count > 0 && s.ack_count == 0) {
                verdict = "T2/T3/T17: timer fires, SIO0 /ACK never asserts (port disconnected, SIO bug, or DMA stall)";
            } else if (s.poll_count > 0 && s.ack_count > 0 && s.cb_count == 0) {
                verdict = "T7/T19/T21: IRQs fire but callback not invoked — handler wiring/GP issue";
            } else if (s.cb_count > 0 && s.last_rxlen < 4) {
                verdict = "T10/T11/T18: callback fires but rx_len < 4 — SPI sequence aborts early";
            } else if (s.cb_count > 0 && s.last_rxlen >= 4) {
                verdict = "T-WORK: SPI working — rx data flowing, but no btn change → check pad_buff translation";
            }
            printf("JCSPI VERDICT: %s\n", verdict);
        }
    }
#endif

    /* Read controller state.
     *
     * Pause is core UX, intentionally NOT gated behind evHotKeysEnabled —
     * that flag defaults to 0 and was only flipped on by the legacy
     * `hotkeys` CLI arg. The original busy-loop pause never fired in
     * normal play because of this; the new menu shouldn't inherit that.
     *
     * We also no longer gate on `pad->stat == 0`. For DualShock analog
     * controllers, the stat byte sometimes reports non-zero values that
     * vary by mode; pad->btn is active-low so a disconnected/uninitialized
     * pad reads as 0xFFFF which inverts to no-buttons-pressed — safe. */
    {
        PADTYPE *pad = (PADTYPE*)pad_buff[0];
        uint16 buttons = ps1PadButtonsWithAnalog(pad);

        if (buttons & PAD_START) {
#if PS1_VERBOSE_DIAGNOSTICS
            if (evPadDiagnosticsEnabled)
                printf("JCPAD START PATH ENTERED call #%lu\n", (unsigned long)padDiagCalls);
#endif
            pauseMenuShow();
            /* pauseMenuUpdate() VSyncs internally — don't double-pace. */
            while (pauseMenuUpdate()) { }
            /* Debounce: hold the loop until Start is released so we
             * don't immediately reopen the menu the next time
             * eventsWaitTick fires. */
            while (ps1PadButtonsWithAnalog((PADTYPE*)pad_buff[0]) & PAD_START) {
                VSync(0);
            }
            /* Reset the frame-pacing anchor so the missed VBlanks
             * during pause aren't counted against the next frame's
             * timing budget. */
            lastFrameTick = (uint32)VSync(-1);
            if (pauseMenuRequestFreeplay ||
                pauseMenuRequestNextScene ||
                pauseMenuRequestResetLoop) {
                return;
            }
#if PS1_VERBOSE_DIAGNOSTICS
            if (evPadDiagnosticsEnabled)
                printf("JCPAD START PATH EXITED\n");
#endif
        }
    }

    {
        uint32 nowTick = (uint32)VSync(-1);
        uint16 targetVBlanks = eventsDelayTicksToTargetVBlanks(delay);
        uint16 elapsedVBlanks = 0;
        uint16 extraVBlanks;

        if (nowTick >= lastFrameTick)
            elapsedVBlanks = (uint16)(nowTick - lastFrameTick);
        extraVBlanks = (elapsedVBlanks < targetVBlanks)
            ? (uint16)(targetVBlanks - elapsedVBlanks)
            : 0;
        while (extraVBlanks-- > 0) {
            VSync(0);
        }
        lastFrameTick = (uint32)VSync(-1);
    }
}
