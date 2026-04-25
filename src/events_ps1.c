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

/* Global variables */
int evHotKeysEnabled = 0;

/* Controller data buffers — non-static so pause_menu.c can read the
 * same shared pad state via `extern uint8 pad_buff[2][34];`. */
uint8 pad_buff[2][34];

static uint16 delayResidue = 0;
static uint32 lastFrameTick = 0;

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
 * Raw SPI response for a digital pad poll (TX = 01 42 00 00):
 *   buff[0] = 0xFF              (response to 0x01 address byte; ignored)
 *   buff[1] = 0x41              (digital ID: type=4, len=1)
 *   buff[2] = 0x5A              (always 0x5A, ignored)
 *   buff[3] = btn low byte
 *   buff[4] = btn high byte
 *
 * PADTYPE layout in our pad_buff[port]:
 *   pad_buff[port][0] = stat    (0x00 connected, 0xFF disconnected)
 *   pad_buff[port][1] = byte 1  (low nibble = len, high nibble = type)
 *   pad_buff[port][2] = btn lo
 *   pad_buff[port][3] = btn hi
 *   pad_buff[port][4..]         (analog axes for DualShock; zero for digital)
 *
 * We treat any rx_len < 5 as "no controller" → write stat=0xFF, btn=0xFFFF.
 */
static void eventsSpiPollCallback(uint32_t port, const volatile uint8_t *buff, size_t rx_len)
{
    if (port > 1)
        return;

    uint8 *dst = pad_buff[port];

    if (rx_len < 5) {
        /* Disconnected / partial response. Mark as no controller. */
        dst[0] = 0xFF;
        dst[1] = 0x00;
        dst[2] = 0xFF;
        dst[3] = 0xFF;
        return;
    }

    /* Connected. Translate raw SPI bytes → PADTYPE bytes. */
    dst[0] = 0x00;             /* stat: connected */
    dst[1] = buff[1];          /* type:len byte (e.g. 0x41 for digital) */
    dst[2] = buff[3];          /* btn low */
    dst[3] = buff[4];          /* btn high */

    /* Copy remaining bytes verbatim (analog axes etc., DualShock).
     * Cap at 30 bytes after the 4-byte header — pad_buff is 34 total. */
    if (rx_len > 5) {
        size_t extra = rx_len - 5;
        if (extra > 30) extra = 30;
        for (size_t i = 0; i < extra; i++)
            dst[4 + i] = buff[5 + i];
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

    printf("JCPAD eventsInit ENTER (SPI driver) pad_buff[0]=%p pad_buff[1]=%p\n",
           (void*)pad_buff[0], (void*)pad_buff[1]);
    printf("JCPAD bits: PAD_START=%04x PAD_CROSS=%04x PAD_SELECT=%04x PAD_TRIANGLE=%04x\n",
           (unsigned)PAD_START, (unsigned)PAD_CROSS, (unsigned)PAD_SELECT, (unsigned)PAD_TRIANGLE);

    /* SPI_Init installs the timer-2 + SIO0 ack IRQ handlers and starts
     * polling at the default 250 Hz. The callback is invoked each time
     * a poll completes (alternating ports each tick). */
    SPI_Init(eventsSpiPollCallback);
    printf("JCPAD SPI_Init called — driver polling at 250 Hz (125 Hz per port)\n");
}

/* JCPAD diagnostic state — checked + printed by eventsWaitTick. */
static uint32 padDiagCalls = 0;
static uint16 padDiagLastBtn0 = 0xFFFF;
static uint16 padDiagLastBtn1 = 0xFFFF;
static uint8  padDiagLastStat0 = 0xFF;
static uint8  padDiagLastStat1 = 0xFF;
static uint16 padDiagMinBtn0 = 0xFFFF;
static uint16 padDiagMaxBtn0 = 0;
static int    padDiagBtnEverChanged0 = 0;
static int    padDiagBtnEverChanged1 = 0;
static int    padDiagStatEverChanged0 = 0;
static int    padDiagStartEverSeen = 0;
static int    padDiagAnyPressedEverSeen = 0;

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
    /* JCPAD diagnostic — runs per-frame, prints once a second + on
     * notable transitions. Goal: figure out why the Start handler
     * never fires. */
    {
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

        /* Periodic snapshot — once per second (60 calls). */
        if ((padDiagCalls % 60) == 0) {
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
            (void)raw1;  /* keep raw1 var alive — helpful if we want to log it later */
        }
    }

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
        uint16 buttons = ~(pad->btn);  /* active low — invert */

        if (buttons & PAD_START) {
            printf("JCPAD START PATH ENTERED call #%lu\n", (unsigned long)padDiagCalls);
            pauseMenuShow();
            while (pauseMenuUpdate()) {
                VSync(0);
            }
            /* Debounce: hold the loop until Start is released so we
             * don't immediately reopen the menu the next time
             * eventsWaitTick fires. */
            while ((~((PADTYPE*)pad_buff[0])->btn) & PAD_START) {
                VSync(0);
            }
            /* Reset the frame-pacing anchor so the missed VBlanks
             * during pause aren't counted against the next frame's
             * timing budget. */
            lastFrameTick = (uint32)VSync(-1);
            printf("JCPAD START PATH EXITED\n");
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
