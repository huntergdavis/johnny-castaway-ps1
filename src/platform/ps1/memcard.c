/*
 * Memory card persistence via our own SPI driver (no BIOS card driver).
 *
 * The BIOS card driver (InitCARD/StartCARD/_bu_init/file API) installs
 * persistent VBlank handlers that fight our 250 Hz SPI controller poll
 * driver and tank scene framerate. To avoid that, we go around the
 * BIOS entirely: enqueue a MemCardRequest on our SPI driver's request
 * queue (spi.c was lifted with this support intact) and parse the raw
 * response from the card.
 *
 * The PS1 memcard protocol (per nocash):
 *   READ  — TX: 81 52 00 00 lba_h lba_l <dummy data 128B> 00 00 00
 *           RX: hi-Z 5A 5D 00 00 5C 5D lba_h lba_l <data 128B> chksum stat
 *   WRITE — TX: 81 57 00 00 lba_h lba_l <data 128B> chksum 00 00 00
 *           RX: hi-Z 5A 5D <dummy 131B> 5C 5D stat
 *
 * Card layout: 1024 sectors × 128 bytes = 128 KB.
 *   sector 0          : header (magic "MC")
 *   sectors 1-15      : directory entries for blocks 1-15
 *   sectors 64-127    : block 1 (8 KB user save)
 * We use block 1 with a proper directory entry at sector 1 so the
 * save shows up in the BIOS Memory Card Manager with the icon.
 */

#include <stdio.h>
#include <string.h>
#include <psxapi.h>
#include <psxgpu.h>

#include "mytypes.h"
#include "memcard.h"
#include "holidays.h"
#include "spi.h"
#include "scene_picker.h"

extern int soundMuted;
extern int oceanAmbientEnabled; /* sound_ps1.c — pause-menu Ocean toggle */
extern int storyCurrentDay;   /* jc_reborn.c — 11-day story calendar */
extern int hostForcedNight;
extern int hostHolidayMode;
extern int hostForcedHoliday;
extern int ps1SoftTimeEnabled;
extern int ps1SoftHour;
extern int ps1SoftMinute;
extern int ps1SoftMonth;
extern int ps1SoftDay;
extern int ps1SoftYear;
extern void eventsSpiPollCallback(uint32_t port, const volatile uint8_t *buff, size_t rx_len);
extern void oceanAmbientStart(void);
extern void oceanAmbientStop(void);
extern int  oceanAmbientLoaded(void);

#define MC_MAGIC       0x434D434A   /* 'JCMC' little-endian */
#define MC_VERSION     6  /* v6 adds holidayMode so auto policy and manual id
                           * are stored separately.
                           * v5 adds pickerPolicy (Random/Sequential/Original).
                           * v4 drops the unused footstepsEnabled toggle.
                           * v3 added oceanAmbientEnabled.
                           * v2 added footstepsEnabled + storyCurrentDay.
                           * v1 had no walk fields. v2..v4 saves still
                           * load (the old footsteps byte is ignored;
                           * v2 saves get oceanAmbientEnabled defaulted
                           * to ON; pre-v5 saves get pickerPolicy
                           * defaulted to Random). */
#define MC_BLOCK       1            /* memcard block we own (1..15) */
#define MC_FIRST_SECT  (MC_BLOCK * 64)
#define MC_FRAME_SIZE  8192
#define MC_SECTOR_SIZE 128
#define MC_NUM_SECT    (MC_FRAME_SIZE / MC_SECTOR_SIZE)  /* 64 */

#define CMD_READ       0x52
#define CMD_WRITE      0x57

#ifndef PS1_VERBOSE_DIAGNOSTICS
#define PS1_VERBOSE_DIAGNOSTICS 0
#endif

#if PS1_VERBOSE_DIAGNOSTICS
#define JCMC_DIAG_PRINTF(...) do { printf(__VA_ARGS__); } while (0)
#else
#define JCMC_DIAG_PRINTF(...) do { } while (0)
#endif

const char *memcardLastStatus = NULL;

/* Sync flag for SPI callback. SPI driver calls our callback from IRQ
 * context when the request completes. We wait for `mcardOpDone` to
 * flip in the main loop. */
static volatile int      mcardOpDone = 0;
static volatile int      mcardOpRxLen = 0;
static volatile uint8_t  mcardOpRxBuf[140];

static void mcardSpiCallback(uint32_t port, const volatile uint8_t *buff, size_t rx_len)
{
    (void)port;
    if (rx_len > sizeof(mcardOpRxBuf)) rx_len = sizeof(mcardOpRxBuf);
    for (size_t i = 0; i < rx_len; i++)
        mcardOpRxBuf[i] = buff[i];
    mcardOpRxLen = (int)rx_len;
    mcardOpDone = 1;
}

/* Wait up to ~1 second for the SPI driver to fire our callback. */
static int mcardWaitDone(void)
{
    for (int i = 0; i < 600 && !mcardOpDone; i++) {
        VSync(0);
    }
    return mcardOpDone;
}

/* Bracket each memcard op pair with poll-rate adjustment. PSn00bSDK's
 * spi.h explicitly says: "It is advisable to call spi_set_poll_rate()
 * to temporarily reduce poll rate while accessing memory cards." A
 * memcard transaction is 138 bytes ≈ 5.5 ms at 250 kbps, longer than
 * one 250 Hz timer tick (4 ms), so the next timer fires mid-transaction
 * and stomps it. Drop to 50 Hz (20 ms tick) for memcard ops. */
static void mcardSlowPoll(void)  { SPI_SetPollRate(50); }
static void mcardFastPoll(void)  { SPI_SetPollRate(250); }

/* PS1 memcard READ protocol (140 bytes total per nocash).
 * After spi.c discards the first response byte (open-bus from addr),
 * rx_buff layout is:
 *   rx[0]      = FLAG byte (0x08 = new card, 0x00 = ok)
 *   rx[1]      = 0x5A (ID1)
 *   rx[2]      = 0x5D (ID2)
 *   rx[3..4]   = (preceding bytes during MSB/LSB send)
 *   rx[5]      = 0x5C (ack1)
 *   rx[6]      = 0x5D (ack2)
 *   rx[7]      = MSB echo
 *   rx[8]      = LSB echo
 *   rx[9..136] = data[0..127]
 *   rx[137]    = checksum
 *   rx[138]    = 0x47 end byte
 */
static int mcardSpiReadSector(int sector, uint8_t *out128)
{
    EnterCriticalSection();
    SPI_Request *req = SPI_CreateRequest();
    if (!req) { ExitCriticalSection(); return 0; }
    /* Build TX bytes raw (PSn00bSDK's MemCardRequest struct doesn't
     * match the actual wire protocol). */
    uint8_t *tx = (uint8_t*)req->payload.data;
    memset(tx, 0, SPI_BUFF_LEN);
    tx[0] = 0x81;             /* addr — memcard slot 1 */
    tx[1] = CMD_READ;         /* 0x52 */
    /* tx[2..3] = 0 (already memset) */
    tx[4] = (uint8_t)((sector >> 8) & 0xFF);
    tx[5] = (uint8_t)(sector & 0xFF);
    /* tx[6..139] = 0 — rest of clocks just for receive */
    req->len = 140;
    req->port = 0;
    req->callback = (SPI_Callback)mcardSpiCallback;
    req->next = NULL;
    mcardOpDone = 0;
    ExitCriticalSection();

    if (!mcardWaitDone()) {
        memcardLastStatus = "read timeout";
        JCMC_DIAG_PRINTF("JCMC read sector %d: TIMEOUT\n", sector);
        return 0;
    }

    if (mcardOpRxLen < 139) {
        memcardLastStatus = "read short";
        JCMC_DIAG_PRINTF("JCMC read sector %d: short rx_len=%d\n", sector, mcardOpRxLen);
        return 0;
    }
    if (mcardOpRxBuf[1] != 0x5A || mcardOpRxBuf[2] != 0x5D) {
        memcardLastStatus = "bad ack";
        JCMC_DIAG_PRINTF("JCMC read sector %d: rx[0..3]=%02x %02x %02x %02x (expect ?? 5A 5D ..)\n",
                         sector, mcardOpRxBuf[0], mcardOpRxBuf[1],
                         mcardOpRxBuf[2], mcardOpRxBuf[3]);
        return 0;
    }
    if (mcardOpRxBuf[138] != 0x47) {
        JCMC_DIAG_PRINTF("JCMC read sector %d: end byte = %02x (expected 0x47)\n",
                         sector, mcardOpRxBuf[138]);
        /* Continue anyway — the data is likely intact. */
    }

    /* Data lives at rx[9..136]. */
    for (int i = 0; i < 128; i++) out128[i] = mcardOpRxBuf[9 + i];
    return 1;
}

/* PS1 memcard WRITE protocol — 138 bytes, raw layout (not the
 * MemCardRequest struct, which is 2 bytes off). Response after
 * spi.c discards the FLAG byte: rx[0]=FLAG echo, rx[1]=5A, rx[2]=5D,
 * rx[133]=5C, rx[134]=5D, rx[135+]=end byte 0x47. */
static int mcardSpiWriteSector(int sector, const uint8_t *data128)
{
    EnterCriticalSection();
    SPI_Request *req = SPI_CreateRequest();
    if (!req) { ExitCriticalSection(); return 0; }
    uint8_t *tx = (uint8_t*)req->payload.data;
    memset(tx, 0, SPI_BUFF_LEN);
    tx[0] = 0x81;
    tx[1] = CMD_WRITE;
    tx[4] = (uint8_t)((sector >> 8) & 0xFF);
    tx[5] = (uint8_t)(sector & 0xFF);
    for (int i = 0; i < 128; i++) tx[6 + i] = data128[i];
    uint8_t chk = tx[4] ^ tx[5];
    for (int i = 0; i < 128; i++) chk ^= data128[i];
    tx[134] = chk;
    /* tx[135..137] = 0 — slots for ack1/ack2/end-byte responses */
    req->len = 138;
    req->port = 0;
    req->callback = (SPI_Callback)mcardSpiCallback;
    req->next = NULL;
    mcardOpDone = 0;
    ExitCriticalSection();
    if (!mcardWaitDone()) {
        memcardLastStatus = "write timeout";
        JCMC_DIAG_PRINTF("JCMC write sector %d: TIMEOUT\n", sector);
        return 0;
    }

    if (mcardOpRxLen < 137) {
        memcardLastStatus = "write short";
        JCMC_DIAG_PRINTF("JCMC write sector %d: short rx_len=%d\n", sector, mcardOpRxLen);
        return 0;
    }
    /* End byte 0x47=OK, 0x4E=BadChecksum, 0xFF=BadSector. */
    uint8_t stat = mcardOpRxBuf[mcardOpRxLen - 1];
    if (stat != 0x47) {
        JCMC_DIAG_PRINTF("JCMC write sector %d: stat=%02x rx[1..3]=%02x %02x %02x\n",
                         sector, stat, mcardOpRxBuf[1], mcardOpRxBuf[2],
                         mcardOpRxBuf[3]);
        memcardLastStatus = "write nack";
        return 0;
    }
    return 1;
}

/* Build a directory entry for our block. 128 bytes. */
static void mcardBuildDirEntry(uint8_t *de)
{
    memset(de, 0, MC_SECTOR_SIZE);
    /* Block state: 0x51 = available; 0x52 = used+more; 0x53 = used+last.
     * For a single-block file: 0x53 (used + last). */
    de[0] = 0x53;
    de[1] = 0x00; de[2] = 0x00; de[3] = 0x00;
    /* File size in bytes, little-endian. */
    de[4] = (uint8_t)(MC_FRAME_SIZE & 0xFF);
    de[5] = (uint8_t)((MC_FRAME_SIZE >> 8) & 0xFF);
    de[6] = 0; de[7] = 0;
    /* Pointer to next block, 0xFFFF = none. */
    de[8] = 0xFF; de[9] = 0xFF;
    /* Filename — max 21 chars including null. */
    static const char fname[] = "BASLUS-99999JCREB";
    memcpy(&de[10], fname, sizeof(fname));
    /* Bytes [10+len .. 126] stay zero. */
    /* Byte 127 = XOR checksum of bytes 0..126. */
    uint8_t chk = 0;
    for (int i = 0; i < 127; i++) chk ^= de[i];
    de[127] = chk;
}

/* The full 8KB save data — assembled in RAM, then sliced to 64 sectors. */
static uint8 mcardFrame[MC_FRAME_SIZE];

typedef struct {
    uint32 magic;
    uint32 version;
    uint8  soundMuted;
    sint8  dayNightOverride;
    sint8  holidayOverride;
    uint8  softTimeEnabled;
    uint8  softHour;
    uint8  softMonth;
    uint8  softDay;
    uint8  softYearLo;
    uint8  softYearHi;
    uint8  softMinute;
    uint8  _obsoleteFootsteps; /* dropped in MC_VERSION 4. Position kept so
                                * v2/v3 saves binary-compat: their stored
                                * byte sits here but the runtime never
                                * reads or writes it. */
    uint8  storyCurrentDay;    /* added in MC_VERSION 2 — walk plan Phase 8 */
    uint8  oceanAmbientEnabled;/* added in MC_VERSION 3 — CC0 ocean ambience */
    uint8  pickerPolicy;       /* added in MC_VERSION 5 — Random/Sequential/Original.
                                * Reuses the v4 reserved[1] byte slot, so v4 saves
                                * load with whatever was sitting there — clamped to
                                * RANDOM by the version-gated load below. */
    uint8  holidayMode;        /* added in MC_VERSION 6 — enum HolidayMode.
                                * v2..v5 derive this from holidayOverride. */
} JCMCSettings;

#define DATA_OFFSET 0x180

/* 16x16 icon. 1 = transparent, 2 = palm green, 3 = sand, 4 = sun yellow, 5 = sky blue. */
static const uint8 mcardIconBitmap[16][16] = {
    {1,1,1,1,1,5,5,5,5,5,5,1,1,1,1,1},
    {1,1,1,5,5,5,4,4,4,4,5,5,5,1,1,1},
    {1,1,5,5,4,4,4,4,4,4,4,4,5,5,1,1},
    {1,5,5,4,4,4,4,2,4,4,4,4,4,5,5,1},
    {1,5,4,4,4,2,2,2,2,2,4,4,4,4,5,1},
    {1,5,4,4,2,2,2,2,2,2,2,4,4,4,5,1},
    {1,5,4,4,4,2,2,2,2,2,4,4,4,4,5,1},
    {1,5,5,4,4,4,2,2,2,4,4,4,4,5,5,1},
    {1,1,5,5,4,4,4,2,4,4,4,4,5,5,1,1},
    {1,1,1,5,5,5,4,2,4,5,5,5,1,1,1,1},
    {1,1,1,1,1,1,5,2,5,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
};

static void mcardWriteSCHeader(uint8 *frame)
{
    frame[0] = 'S';
    frame[1] = 'C';
    frame[2] = 0x11;   /* one static icon frame */
    frame[3] = 0x01;   /* one block */
    static const char title[] = "JOHNNY CASTAWAY";
    memcpy(&frame[4], title, sizeof(title) - 1);
}

static void mcardWriteIconClut(uint8 *frame)
{
    static const uint16 palette[16] = {
        0x0000, 0x0000,
        (uint16)(0x8000 | (4) | (24 << 5) | (4 << 10)),
        (uint16)(0x8000 | (28) | (24 << 5) | (8 << 10)),
        (uint16)(0x8000 | (31) | (28 << 5) | (8 << 10)),
        (uint16)(0x8000 | (16) | (24 << 5) | (28 << 10)),
        0,0,0,0,0,0,0,0,0,0
    };
    uint16 *clut = (uint16*)&frame[0x80];
    for (int i = 0; i < 16; i++) clut[i] = palette[i];
}

static void mcardWriteIconBitmap(uint8 *frame)
{
    uint8 *icon = &frame[0x100];
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x += 2) {
            uint8 lo = mcardIconBitmap[y][x]     & 0x0F;
            uint8 hi = mcardIconBitmap[y][x + 1] & 0x0F;
            icon[y * 8 + x / 2] = (uint8)(lo | (hi << 4));
        }
    }
}

int memcardLoadSettings(void)
{
    int ok = 1;
    JCMC_DIAG_PRINTF("JCMC load: starting (slow poll, MemCardRequest path)\n");
    mcardSlowPoll();

    /* Read directory entry first to verify the file is present. */
    uint8 dirEntry[MC_SECTOR_SIZE];
    if (!mcardSpiReadSector(MC_BLOCK, dirEntry)) {
        memcardLastStatus = "no card";
        mcardFastPoll();
        return 0;
    }
    if (dirEntry[0] != 0x53 || memcmp(&dirEntry[10], "BASLUS-99999JCREB", 17) != 0) {
        memcardLastStatus = "no save";
        mcardFastPoll();
        return 0;
    }

    /* Read all 64 sectors of block 1 into mcardFrame. */
    for (int s = 0; s < MC_NUM_SECT; s++) {
        if (!mcardSpiReadSector(MC_FIRST_SECT + s, &mcardFrame[s * MC_SECTOR_SIZE])) {
            JCMC_DIAG_PRINTF("JCMC load: read sector %d failed\n", MC_FIRST_SECT + s);
            ok = 0;
            break;
        }
    }
    mcardFastPoll();
    if (!ok) return 0;

    JCMCSettings *s = (JCMCSettings *)&mcardFrame[DATA_OFFSET];
    if (s->magic != MC_MAGIC) {
        memcardLastStatus = "bad magic";
        JCMC_DIAG_PRINTF("JCMC load: bad magic %08lx\n", (unsigned long)s->magic);
        return 0;
    }
    /* Accept v2..v6. v6 adds holidayMode. v5 adds pickerPolicy
     * (re-uses v4's reserved byte).
     * v4 drops the unused footstepsEnabled toggle but keeps its byte
     * position for binary compatibility — the field is ignored on
     * load and zeroed on save. v3 added oceanAmbientEnabled; v2 saves
     * get that defaulted to ON. Reject v < 2 — those were transient
     * and never released. */
    if (s->version != MC_VERSION
        && s->version != 2
        && s->version != 3
        && s->version != 4
        && s->version != 5) {
        memcardLastStatus = "version mismatch";
        return 0;
    }
    int loadedVersion = s->version;

    soundMuted        = s->soundMuted ? 1 : 0;
    /* oceanAmbientEnabled is field-of-record in v3+ only; older versions
     * default to ON (so users opting in via the new toggle don't have
     * to migrate their save manually). */
    oceanAmbientEnabled = (loadedVersion >= 3)
                          ? (s->oceanAmbientEnabled ? 1 : 0)
                          : 1;
    storyCurrentDay   = (s->storyCurrentDay >= 1 && s->storyCurrentDay <= 11)
                          ? s->storyCurrentDay : 1;
    hostForcedNight   = s->dayNightOverride;
    hostForcedHoliday = s->holidayOverride;
    if (hostForcedHoliday < 0 ||
        (hostForcedHoliday > 0 && !holidayById(hostForcedHoliday)))
        hostForcedHoliday = 0;
    if (loadedVersion >= 6) {
        hostHolidayMode = s->holidayMode;
    } else if (s->holidayOverride < 0) {
        /* 0.7.1 changes the first-run/no-card auto policy to the original
         * Sierra four holidays. Migrate old "auto all" saves to the new
         * default unless the user explicitly saved a manual holiday/none. */
        hostHolidayMode = HOLIDAY_MODE_AUTO_ORIGINAL4;
    } else {
        hostHolidayMode = holidayModeFromOverride(s->holidayOverride);
    }
    if (hostHolidayMode < 0 || hostHolidayMode >= HOLIDAY_MODE_COUNT)
        hostHolidayMode = HOLIDAY_MODE_AUTO_ORIGINAL4;
    if (hostHolidayMode == HOLIDAY_MODE_MANUAL_ORIG4 &&
        !holidayIsOriginalId(hostForcedHoliday))
        hostForcedHoliday = 1;
    if (hostHolidayMode == HOLIDAY_MODE_MANUAL_EXPANDED &&
        (hostForcedHoliday <= 4 || !holidayById(hostForcedHoliday)))
        hostForcedHoliday = holidayFirstExpandedId();
    ps1SoftTimeEnabled = s->softTimeEnabled ? 1 : 0;
    ps1SoftHour       = s->softHour;
    ps1SoftMinute     = (s->softMinute <= 59) ? s->softMinute : 0;
    ps1SoftMonth      = s->softMonth;
    ps1SoftDay        = s->softDay;
    ps1SoftYear       = (int)s->softYearLo | ((int)s->softYearHi << 8);
    if (ps1SoftHour < 0 || ps1SoftHour > 23)
        ps1SoftHour = 12;
    if (ps1SoftMonth < 1 || ps1SoftMonth > 12)
        ps1SoftMonth = 6;
    if (ps1SoftDay < 1 || ps1SoftDay > 31)
        ps1SoftDay = 30;
    if (ps1SoftYear < 1583)
        ps1SoftYear = 2026;

    /* pickerPolicy is field-of-record in v5+ only. v < 5 saves had a
     * reserved byte sitting at this offset; force RANDOM so we don't
     * accidentally restore garbage as a "real" policy. */
    {
        int policy = (loadedVersion >= 5) ? (int)s->pickerPolicy
                                          : SCENE_PICKER_RANDOM;
        if (policy < 0 || policy >= SCENE_PICKER_COUNT)
            policy = SCENE_PICKER_RANDOM;
        pickerSetPolicy(policy);
    }

    /* Sync the ocean ambience SPU voice to the loaded toggle value when
     * settings are loaded after SPU init. Normal boot loads memcard first,
     * so this block is a no-op until soundInit has uploaded OCEAN.VAG. */
    if (oceanAmbientLoaded()) {
        if (oceanAmbientEnabled) oceanAmbientStart();
        else                     oceanAmbientStop();
    }

    memcardLastStatus = "loaded";
    JCMC_DIAG_PRINTF("JCMC loaded: muted=%d dn=%d holimode=%d holi=%d soft=%d %02d:%02d %02d/%02d/%04d ocean=%d picker=%d\n",
                     soundMuted, hostForcedNight, hostHolidayMode,
                     hostForcedHoliday, ps1SoftTimeEnabled, ps1SoftHour,
                     ps1SoftMinute, ps1SoftMonth, ps1SoftDay, ps1SoftYear,
                     oceanAmbientEnabled, pickerGetPolicy());
    return 1;
}

int memcardSaveSettings(void)
{
    /* Build the 8KB save frame: SC header, icon, settings struct. */
    memset(mcardFrame, 0, MC_FRAME_SIZE);
    mcardWriteSCHeader(mcardFrame);
    mcardWriteIconClut(mcardFrame);
    mcardWriteIconBitmap(mcardFrame);

    JCMCSettings *s = (JCMCSettings *)&mcardFrame[DATA_OFFSET];
    s->magic              = MC_MAGIC;
    s->version            = MC_VERSION;
    s->soundMuted         = (uint8)(soundMuted ? 1 : 0);
    s->_obsoleteFootsteps = 0;   /* dropped in v4; byte zeroed on save */
    s->oceanAmbientEnabled = (uint8)(oceanAmbientEnabled ? 1 : 0);
    s->storyCurrentDay    = (uint8)((storyCurrentDay >= 1
                                    && storyCurrentDay <= 11)
                                   ? storyCurrentDay : 1);
    s->dayNightOverride = (sint8)hostForcedNight;
    s->holidayOverride  = (sint8)hostForcedHoliday;
    s->holidayMode      = (uint8)((hostHolidayMode >= 0 &&
                                   hostHolidayMode < HOLIDAY_MODE_COUNT)
                                  ? hostHolidayMode
                                  : HOLIDAY_MODE_AUTO_ORIGINAL4);
    s->softTimeEnabled  = (uint8)(ps1SoftTimeEnabled ? 1 : 0);
    s->softHour         = (uint8)ps1SoftHour;
    s->softMinute       = (uint8)ps1SoftMinute;
    s->softMonth        = (uint8)ps1SoftMonth;
    s->softDay          = (uint8)ps1SoftDay;
    s->softYearLo       = (uint8)(ps1SoftYear & 0xFF);
    s->softYearHi       = (uint8)((ps1SoftYear >> 8) & 0xFF);
    s->pickerPolicy     = (uint8)pickerGetPolicy();

    mcardSlowPoll();

    /* Write directory entry at sector MC_BLOCK (1). */
    uint8 dirEntry[MC_SECTOR_SIZE];
    mcardBuildDirEntry(dirEntry);
    if (!mcardSpiWriteSector(MC_BLOCK, dirEntry)) {
        JCMC_DIAG_PRINTF("JCMC save: dir-entry write failed\n");
        mcardFastPoll();
        return 0;
    }

    /* Write 64 data sectors starting at MC_FIRST_SECT (64). */
    for (int sect = 0; sect < MC_NUM_SECT; sect++) {
        if (!mcardSpiWriteSector(MC_FIRST_SECT + sect,
                                  &mcardFrame[sect * MC_SECTOR_SIZE])) {
            JCMC_DIAG_PRINTF("JCMC save: sector %d write failed\n", MC_FIRST_SECT + sect);
            mcardFastPoll();
            return 0;
        }
    }

    mcardFastPoll();
    memcardLastStatus = "saved";
    JCMC_DIAG_PRINTF("JCMC saved %d bytes (block %d)\n", MC_FRAME_SIZE, MC_BLOCK);
    return 1;
}
