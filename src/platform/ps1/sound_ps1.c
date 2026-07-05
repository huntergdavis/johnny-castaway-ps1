/*
 *  This file is part of 'Johnny Reborn' - PS1 Port
 *
 *  PlayStation 1 audio implementation using PSn00bSDK SPU
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

#include <psxspu.h>
#include <psxapi.h>
#include <hwregs_c.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "mytypes.h"     /* Defines uint8, uint16, uint32, etc. */
#include "sound_ps1.h"
#include "config.h"
#include "utils.h"
#include "cdrom_ps1.h"
#include "ps1_spu_cache.h"
#include "ps1_boot_progress.h"
#include "mem_region.h"

#ifndef SOUND_PS1_DIAG_LOGS
#define SOUND_PS1_DIAG_LOGS 0
#endif

#if SOUND_PS1_DIAG_LOGS
#define SOUND_DIAG_PRINTF(...) do { printf(__VA_ARGS__); } while (0)
#else
#define SOUND_DIAG_PRINTF(...) do { } while (0)
#endif

/* Global variables */
int soundDisabled = 0;
int soundMuted = 0;
/* Default ON; memcard load may overwrite at boot. The default lives in
 * memory as a uint8 0/1 mirror of int 0/1; see oceanAmbientEnabled
 * doc in sound_ps1.h. */
int oceanAmbientEnabled = 1;

/* SPU configuration */
#define MAX_SOUND_EFFECTS 25
#define SPU_DATA_START 0x1010  /* After SPU capture buffers + dummy block */
#define SPU_RAM_SIZE_BYTES (512u * 1024u)
#define SPU_DMA_ALIGN_BYTES 64u
#define VAG_HEADER_SIZE 48    /* Standard Sony VAG header size */
#define NUM_CHANNELS 8        /* Use 8 channels for round-robin */
/* Leave mixer headroom for scenes that intentionally overlap the same SFX.
 * SUZY 2 fires SOUND18 three times while earlier voices are still ringing; at
 * full scale those voices can clip when summed by the SPU/emulator mixer. */
#define SFX_VOLUME 0x3000
/* Dedicated SPU voice for the ambience loop. Voice 23 is the highest
 * available; SFX uses voices 0..7 round-robin so 23 is well clear. */
#define OCEAN_AMBIENT_VOICE 23
/* Volume sits slightly under SFX so SFX cuts through the ambience.
 * 0x2400 ≈ 56% of full scale; SFX uses 0x3FFF (full). */
#define OCEAN_AMBIENT_VOLUME 0x2400

/* Sound effect data loaded into SPU RAM */
static uint32_t soundAddresses[MAX_SOUND_EFFECTS];
static uint32_t soundSizes[MAX_SOUND_EFFECTS];
static uint16_t soundSampleRates[MAX_SOUND_EFFECTS];
static uint16_t soundPitches[MAX_SOUND_EFFECTS];  /* Pre-computed pitch values */
static int soundsLoaded = 0;
static int nextChannel = 0;

/* Ocean ambience — single loop in SPU RAM after the SFX block. */
static uint32_t oceanSpuAddr  = 0;
static uint32_t oceanAdpcmSize = 0;
static uint16_t oceanPitch    = 0;
static int      oceanLoaded   = 0;
static int      oceanPlaying  = 0;
static uint32_t soundSpuDataEndAddr = SPU_DATA_START;
static uint32_t soundSpuCacheBaseAddr = 0;
static uint32_t soundSpuCacheByteCount = 0;

static uint32_t soundAlignSpuDma(uint32_t value)
{
    return (value + (SPU_DMA_ALIGN_BYTES - 1u)) & ~(SPU_DMA_ALIGN_BYTES - 1u);
}

static void soundUpdateSpuCacheWindow(uint32_t usedEnd)
{
    uint32_t base = soundAlignSpuDma(usedEnd);
    soundSpuDataEndAddr = usedEnd;
    if (base >= SPU_RAM_SIZE_BYTES) {
        soundSpuCacheBaseAddr = SPU_RAM_SIZE_BYTES;
        soundSpuCacheByteCount = 0;
        return;
    }
    soundSpuCacheBaseAddr = base;
    soundSpuCacheByteCount =
        (SPU_RAM_SIZE_BYTES - base) & ~(SPU_DMA_ALIGN_BYTES - 1u);
}

static void soundStopAllVoices(void)
{
    volatile uint16_t *keyOff1 = (volatile uint16_t *)0xBF801D8C;
    volatile uint16_t *keyOff2 = (volatile uint16_t *)0xBF801D8E;
    *keyOff1 = 0xFFFFu;
    *keyOff2 = 0x00FFu;
    oceanPlaying = 0;
}

/* --- Hardware-truth helpers -------------------------------------------
 * Console rounds 3-5 showed SPU voice/key register writes being lost on
 * real silicon in ways emulators never reproduce (emulators apply every
 * register write unconditionally). Rather than keep modelling the exact
 * drop window, every write that matters is verified — voice registers by
 * reading them back, key-on/off through SPU_CH_ADSR_VOL (ENVX), the
 * SPU's live envelope, which is ground truth for "is this voice keyed".
 * Every loop is bounded; counters feed the sound-test screen so the
 * console itself reports what failed. */

static uint16_t gSndDiagProgFail   = 0;
static uint16_t gSndDiagKeyFail    = 0;
static uint16_t gSndDiagUploadRetry = 0;
static uint16_t gSndDiagUploadBad  = 0;

static int spuVoiceProgramVerified(int ch, uint16_t pitch, uint32_t addrBytes,
                                   uint16_t volL, uint16_t volR,
                                   uint16_t adsr1, uint16_t adsr2)
{
    uint16_t wantAddr = (uint16_t)getSPUAddr(addrBytes);
    int attempt;

    if (!ps1SpuRegReadsOk()) {
        /* Register reads are bus garbage on this unit: readback
         * verification is meaningless. Plain writes (writes work —
         * the boot ambience proves it). */
        SPU_CH_FREQ(ch)  = pitch;
        SPU_CH_ADDR(ch)  = wantAddr;
        SPU_CH_VOL_L(ch) = volL;
        SPU_CH_VOL_R(ch) = volR;
        SPU_CH_ADSR1(ch) = adsr1;
        SPU_CH_ADSR2(ch) = adsr2;
        return 1;
    }

    for (attempt = 0; attempt < 4; attempt++) {
        SPU_CH_FREQ(ch)  = pitch;
        SPU_CH_ADDR(ch)  = wantAddr;
        SPU_CH_VOL_L(ch) = volL;
        SPU_CH_VOL_R(ch) = volR;
        SPU_CH_ADSR1(ch) = adsr1;
        SPU_CH_ADSR2(ch) = adsr2;
        /* Voice regs read back their programmed value (volumes read the
         * live sweep value, so they are not compared). */
        if (SPU_CH_FREQ(ch) == pitch &&
            SPU_CH_ADDR(ch) == wantAddr &&
            SPU_CH_ADSR1(ch) == adsr1 &&
            SPU_CH_ADSR2(ch) == adsr2)
            return 1;
    }
    gSndDiagProgFail++;
    return 0;
}

static int spuVoiceKeyOnVerified(int ch)
{
    int attempt, i;

    if (!ps1SpuRegReadsOk()) {
        SpuSetKey(1, 1 << ch);
        return 1;
    }

    for (attempt = 0; attempt < 4; attempt++) {
        SpuSetKey(1, 1 << ch);
        /* Instant attack -> ENVX rises within a few 44.1 kHz envelope
         * ticks (~23 us each); ~300 uncached reads span plenty. If the
         * envelope never moves, the key-on write was swallowed. */
        for (i = 0; i < 300; i++) {
            if (SPU_CH_ADSR_VOL(ch) != 0)
                return 1;
        }
    }
    gSndDiagKeyFail++;
    return 0;
}

static int spuVoiceKeyOffVerified(int ch)
{
    int attempt, i;

    if (!ps1SpuRegReadsOk()) {
        SpuSetKey(0, 1 << ch);
        return 1;
    }

    for (attempt = 0; attempt < 8; attempt++) {
        SpuSetKey(0, 1 << ch);
        /* Release rate 0 -> envelope collapses within a few ticks. */
        for (i = 0; i < 300; i++) {
            if (SPU_CH_ADSR_VOL(ch) == 0)
                return 1;
        }
    }
    gSndDiagKeyFail++;
    return 0;
}

/* Windowed SPU RAM readback compare for uploads. The 2 KB window lives
 * in the tail of the borrowed scene-explorer chunk buffer (idle during
 * boot — layout documented in ps1_boot_progress.c); uploads only happen
 * in soundInit, squarely inside the boot window. Sizes are always
 * 64-byte aligned. */
#define SND_VERIFY_WIN_BYTES 2048u
static int spuUploadVerify(uint32_t spuAddr, const uint8_t *src, uint32_t size)
{
    extern void *grSceneExplorerChunkBorrow(uint32 *bytesOut);
    uint32 bufBytes = 0;
    uint8_t *win;
    uint32_t off = 0;

    if (!ps1SpuRegReadsOk())
        return 1;   /* readback would be garbage — trust the write */
    win = (uint8_t *)grSceneExplorerChunkBorrow(&bufBytes) + 8192u;
    if (bufBytes < 8192u + SND_VERIFY_WIN_BYTES)
        return 1;   /* no scratch — trust the write */

    while (off < size) {
        uint32_t chunk = size - off;
        if (chunk > SND_VERIFY_WIN_BYTES)
            chunk = SND_VERIFY_WIN_BYTES;
        if (!ps1SpuDmaRead(spuAddr + off, win, chunk))
            return 0;
        if (memcmp(win, src + off, chunk) != 0)
            return 0;
        off += chunk;
    }
    return 1;
}

int soundDiagSpuCnt(void)  { return (int)SPU_CTRL; }
int soundDiagSpuStat(void) { return (int)SPU_STAT; }
int soundDiagEnvx(int ch)  { return (int)SPU_CH_ADSR_VOL(ch); }
void soundDiagCounters(int *progFail, int *keyFail, int *upRetry, int *upBad)
{
    *progFail = gSndDiagProgFail;
    *keyFail  = gSndDiagKeyFail;
    *upRetry  = gSndDiagUploadRetry;
    *upBad    = gSndDiagUploadBad;
}

int soundDiagRegReadsOk(void)
{
    return ps1SpuRegReadsOk();
}

/* Ambience self-heal: the ocean loop occasionally dies mid-session on
 * console (cause not yet pinned — the re-key counter on the sound-test
 * screen measures how often this fires so the underlying bug stays
 * visible). Called once per displayed frame from the scene and pause
 * loops; checks every ~2 s. ENVX is ground truth for "voice alive". */
static uint16_t gSndDiagAmbRekey = 0;

int soundDiagAmbRekeys(void) { return gSndDiagAmbRekey; }

void soundAmbienceWatchdog(void)
{
    static uint32_t throttle = 0;

    if (!oceanLoaded || !oceanAmbientEnabled || soundMuted || !oceanPlaying)
        return;
    if (++throttle < 120)
        return;
    throttle = 0;
    if (!ps1SpuRegReadsOk())
        return;             /* envelope unreadable on this unit */
    if (SPU_CH_ADSR_VOL(OCEAN_AMBIENT_VOICE) != 0)
        return;
    gSndDiagAmbRekey++;
    oceanPlaying = 0;       /* force oceanAmbientStart through its guard */
    oceanAmbientStart();
}

/* Read big-endian uint32 from VAG header */
static uint32_t readBE32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  | (uint32_t)p[3];
}

/* Parse one effect VAG and upload it to SPU RAM. Takes ownership of
 * vagData (always freed). Returns 1 = loaded, 0 = skipped (bad data /
 * no memory / verify failed -> MISSING), -1 = out of SPU RAM (stop). */
static int soundUploadVagEffect(int i, uint8_t *vagData, uint32_t vagSize,
                                uint32_t *spuAddr)
{
    if (vagSize <= VAG_HEADER_SIZE) {
        free(vagData);
        return 0;
    }

    /* Parse VAG header — sample rate is big-endian at offset 16 */
    uint16_t sampleRate = (uint16_t)readBE32(vagData + 16);
    uint32_t adpcmSize = vagSize - VAG_HEADER_SIZE;
    /* SPU DMA moves data in 64-byte blocks; pad up or the final ADPCM
     * flag byte (end-of-sample) gets truncated and the voice never
     * stops, producing silence or noise instead of our sample. */
    uint32_t dmaSize = (adpcmSize + 63u) & ~63u;

    if (*spuAddr + dmaSize > SPU_RAM_SIZE_BYTES) {
        printf("SPU: out of RAM at sound %d\n", i);
        free(vagData);
        return -1;
    }

    uint8_t *dmaBuf = (uint8_t *)malloc(dmaSize);
    if (!dmaBuf) {
        free(vagData);
        return 0;
    }
    memcpy(dmaBuf, vagData + VAG_HEADER_SIZE, adpcmSize);
    if (dmaSize > adpcmSize)
        memset(dmaBuf + adpcmSize, 0, dmaSize - adpcmSize);

    /* Upload, then read back and compare. Console rounds showed uploads
     * landing corrupted in ways no completion wait fully excludes; a
     * sample that never verifies is dropped (MISSING in the sound test)
     * instead of shipping corrupt. */
    ps1BootProgress((uint8)(20 + i * 2));
    {
        int attempt, uploadOk = 0;
        for (attempt = 0; attempt < 3 && !uploadOk; attempt++) {
            if (attempt > 0)
                gSndDiagUploadRetry++;
            if (!ps1SpuDmaWrite(*spuAddr, dmaBuf, dmaSize))
                continue;
            uploadOk = spuUploadVerify(*spuAddr, dmaBuf, dmaSize);
        }
        free(dmaBuf);
        free(vagData);
        if (!uploadOk) {
            gSndDiagUploadBad++;
            return 0;   /* soundAddresses[i] stays 0 -> MISSING */
        }
    }

    soundAddresses[i] = *spuAddr;
    soundSizes[i] = adpcmSize;
    soundSampleRates[i] = sampleRate;
    soundPitches[i] = getSPUSampleRate(sampleRate);

    /* Advance by the DMA-aligned amount so the next sample does not
     * overlap the padding tail of this one. */
    *spuAddr += dmaSize;
    return 1;
}

/* Group sink for the verified continuous read: copy payload bytes into
 * the entry's staging buffer. Pure memcpy — no SPU work in the arrival
 * window, so the ring can never be lapped. */
struct SndPackReadCtx {
    uint8_t *dst;
    uint32_t cap;
};

static int soundPackGroupCopy(void *ud, const uint8_t *group,
                              uint32_t firstSector, uint32_t groupSectors)
{
    struct SndPackReadCtx *ctx = (struct SndPackReadCtx *)ud;
    uint32_t off = firstSector * 2048u;
    uint32_t bytes = groupSectors * 2048u;
    if (off >= ctx->cap)
        return 0;
    if (bytes > ctx->cap - off)
        bytes = ctx->cap - off;
    memcpy(ctx->dst + off, group, bytes);
    return 1;
}

/* Effects from SOUNDS.PAK: one locate, then sequential sector-aligned
 * reads of each entry (offsets adjacent on disc — near-zero seek cost).
 * Entry reads go through the VERIFIED continuous reader when the ring
 * buffer is available: every sector proves its address before its
 * bytes can reach the SPU (a silently wrong sector at boot = corrupt
 * samples forever). Chunked aligned reads remain the fallback.
 * Returns the number of effects loaded; 0 -> caller runs the per-file
 * fallback loop. */
static int soundLoadEffectsFromPack(uint32_t *spuAddr)
{
    CdlFILE pak;
    uint8_t *hdr;
    int count, e, loaded = 0;

    if (ps1_cdSearchFileQuiesced(&pak, "\\SND\\SOUNDS.PAK;1") == NULL)
        return 0;
    hdr = (uint8_t *)malloc(2048);
    if (hdr == NULL)
        return 0;
    if (!ps1_streamReadAlignedIntoFile(&pak, 0, 2048, hdr) ||
        hdr[0] != 'J' || hdr[1] != 'C' || hdr[2] != 'S' || hdr[3] != 'P' ||
        (hdr[4] | (hdr[5] << 8)) != 1) {
        free(hdr);
        return 0;
    }
    count = hdr[6] | (hdr[7] << 8);
    if (count <= 0 || count > 64) {
        free(hdr);
        return 0;
    }

    /* Ring for the verified reads. malloc'd (the explorer chunk buffer
     * is busy hosting the boot ship + sound-verify window during this
     * exact phase); NULL just means every entry uses the chunked path. */
    {
        extern int ps1CdReadContinuousInto(const CdlFILE *, uint32_t,
                                           uint32_t, uint8_t *, uint32_t,
                                           int (*)(void *, const uint8_t *,
                                                   uint32_t, uint32_t),
                                           void *);
        uint8_t *ring = (uint8_t *)malloc(5u * 2340u);

        for (e = 0; e < count; e++) {
            const uint8_t *ent = hdr + 8 + e * 8;
            int idx = ent[0];
            uint32_t offSec = (uint32_t)(ent[2] | (ent[3] << 8));
            uint32_t size = (uint32_t)ent[4] | ((uint32_t)ent[5] << 8) |
                            ((uint32_t)ent[6] << 16) | ((uint32_t)ent[7] << 24);
            uint32_t sizeSect = (size + 2047u) & ~2047u;
            uint8_t *vagData;
            int gotData = 0;
            int r;

            if (idx < 0 || idx >= MAX_SOUND_EFFECTS || size == 0 ||
                sizeSect > 64u * 1024u)
                continue;
            vagData = (uint8_t *)malloc(sizeSect);
            if (vagData == NULL)
                continue;
            if (ring != NULL) {
                struct SndPackReadCtx ctx;
                ctx.dst = vagData;
                ctx.cap = sizeSect;
                gotData = ps1CdReadContinuousInto(&pak, offSec,
                                                  sizeSect / 2048u,
                                                  ring, 5u,
                                                  soundPackGroupCopy, &ctx);
            }
            if (!gotData &&
                !ps1_streamReadAlignedIntoFile(&pak, offSec * 2048u, sizeSect,
                                               vagData)) {
                free(vagData);
                continue;
            }
            r = soundUploadVagEffect(idx, vagData, size, spuAddr);
            if (r < 0)
                break;
            if (r > 0)
                loaded++;
        }
        if (ring != NULL)
            free(ring);
    }
    free(hdr);
    return loaded;
}

/*
 * Initialize SPU audio system and load VAG files from CD into SPU RAM
 */
void soundInit()
{
    if (soundDisabled) {
        SOUND_DIAG_PRINTF("Sound disabled\n");
        return;
    }

    /* Initialize SPU */
    SpuInit();
    ps1SpuNoteInitDone();   /* probe SPU register-read health (gates all
                             * read-dependent verification below) */

    /* Set master volume */
    SpuSetCommonMasterVolume(0x3FFF, 0x3FFF);

    /* Clear sound tables */
    for (int i = 0; i < MAX_SOUND_EFFECTS; i++) {
        soundAddresses[i] = 0;
        soundSizes[i] = 0;
        soundSampleRates[i] = 0;
    }

    /* Load VAG files from CD into SPU RAM. Pack path first: SOUNDS.PAK
     * holds all effects behind ONE locate + sequential sector-aligned
     * reads (23 directory walks + seek cycles per boot -> 1 on a cold
     * drive). The per-file loop below is the fallback. */
    uint32_t spuAddr = SPU_DATA_START;
    int loaded = soundLoadEffectsFromPack(&spuAddr);

    for (int i = 0; loaded == 0 && i < MAX_SOUND_EFFECTS; i++) {
        char filename[] = "\\SND\\SOUND00.VAG;1";

        /* SOUND11/SOUND13 do not exist in the original game data (see
         * config/ps1/cd_layout.xml). Probing them anyway meant two
         * guaranteed lookup failures EVERY boot, feeding the CD
         * failure counters and the drive-recovery streak for nothing. */
        if (i == 11 || i == 13)
            continue;
        filename[10] = (char)('0' + ((i / 10) % 10));
        filename[11] = (char)('0' + (i % 10));

        uint32_t vagSize = 0;
        uint8_t *vagData = ps1_loadRawFile(filename, &vagSize);
        if (!vagData) continue;

        {
            int r = soundUploadVagEffect(i, vagData, vagSize, &spuAddr);
            if (r < 0)
                break;      /* out of SPU RAM */
            if (r > 0)
                loaded++;
        }
    }

    if (loaded > 0) {
        soundsLoaded = 1;
        SOUND_DIAG_PRINTF("SPU: loaded %d sounds\n", loaded);
    } else {
        SOUND_DIAG_PRINTF("SPU: no VAG files found\n");
    }

    /* Load OCEAN.VAG for the looping background ambience. Same DMA
     * pattern as the SFX loader; placed in SPU RAM right after the
     * last SFX. The VAG carries its own loop flags (0x06 on first
     * data block, 0x03 on last) so the SPU loops in hardware with
     * no need to set SPU_CH_LOOP_ADDR from the C side. */
    do {
        /* OCEAN.VAG is ~123 KB — far larger than the SFX (<=6 KB). The old
         * ps1_loadRawFile path malloc'd the whole file from the libc heap,
         * which is too tight at boot (the 1440 KB region buffer leaves
         * little) -> the malloc returned NULL, the ambience never loaded
         * (oceanLoaded stayed 0), and the "wave sounds" toggle was a silent
         * no-op (oceanAmbientStart bails on !oceanLoaded). soundInit runs
         * BEFORE the stable-shape band is reserved, so the CACHE region is
         * empty here — read the VAG straight into it and DMA from there.
         * Falls back to libc only for a mid-session re-init when CACHE is
         * occupied (boot, the path that matters, always uses CACHE). */
        /* ps1_streamResolveFile prepends '\' and appends ";1" itself, so
         * pass the BARE path (not the decorated "\SND\OCEAN.VAG;1"). */
        CdlFILE ocf;
        if (!ps1_streamResolveFile("SND\\OCEAN.VAG", &ocf)) {
            printf("SPU: OCEAN.VAG resolve failed\n");
            break;
        }
        uint32_t vagSize = (uint32_t)ocf.size;
        if (vagSize <= VAG_HEADER_SIZE)
            break;
        uint16_t sampleRate;
        uint32_t adpcmSize = vagSize - VAG_HEADER_SIZE;
        uint32_t dmaSize = (adpcmSize + 63u) & ~63u;

        if (spuAddr + dmaSize > SPU_RAM_SIZE_BYTES) {
            printf("SPU: out of RAM for OCEAN.VAG (need %lu, have %lu)\n",
                   (unsigned long)dmaSize,
                   (unsigned long)(SPU_RAM_SIZE_BYTES - spuAddr));
            break;
        }

        /* Sector-rounded buffer covering the header + DMA range, for the
         * sector-aligned CD read. */
        uint32_t bufBytes = (VAG_HEADER_SIZE + dmaSize + 2047u) & ~2047u;
        int vagRegion = (int)MEM_REGION_CACHE;
        uint8_t *vagData = (uint8_t *)memTryAlloc(MEM_REGION_CACHE, bufBytes,
                                                  "ocean-vag-load");
        if (vagData == NULL) {
            vagData = (uint8_t *)malloc(bufBytes);
            vagRegion = -1;
        }
        if (vagData == NULL) {
            SOUND_DIAG_PRINTF("SPU: OCEAN.VAG no buffer (%lu)\n",
                              (unsigned long)bufBytes);
            break;
        }
        if (!ps1_streamReadAlignedIntoFile(&ocf, 0, bufBytes, vagData)) {
            if (vagRegion == (int)MEM_REGION_CACHE) memFree(MEM_REGION_CACHE, vagData);
            else free(vagData);
            break;
        }

        sampleRate = (uint16_t)readBE32(vagData + 16);
        /* Zero the DMA padding tail beyond the ADPCM (in bounds: bufBytes is
         * sector-rounded above VAG_HEADER_SIZE + dmaSize). */
        if (dmaSize > adpcmSize)
            memset(vagData + VAG_HEADER_SIZE + adpcmSize, 0, dmaSize - adpcmSize);

        {
            int attempt, uploadOk = 0;
            for (attempt = 0; attempt < 2 && !uploadOk; attempt++) {
                if (attempt > 0)
                    gSndDiagUploadRetry++;
                if (!ps1SpuDmaWrite(spuAddr, vagData + VAG_HEADER_SIZE,
                                    dmaSize))
                    continue;
                uploadOk = spuUploadVerify(spuAddr, vagData + VAG_HEADER_SIZE,
                                           dmaSize);
            }
            if (!uploadOk)
                gSndDiagUploadBad++;
        }
        ps1BootProgress(74);

        oceanSpuAddr   = spuAddr;
        oceanAdpcmSize = adpcmSize;
        oceanPitch     = getSPUSampleRate(sampleRate);
        oceanLoaded    = 1;
        spuAddr += dmaSize;

        if (vagRegion == (int)MEM_REGION_CACHE) memFree(MEM_REGION_CACHE, vagData);
        else free(vagData);
        SOUND_DIAG_PRINTF("SPU: OCEAN.VAG loaded (%lu bytes ADPCM @ 0x%lx)\n",
                          (unsigned long)adpcmSize, (unsigned long)oceanSpuAddr);
    } while (0);

    soundUpdateSpuCacheWindow(spuAddr);

    /* Auto-start ambience only after saved settings have been loaded. */
    if (oceanLoaded && oceanAmbientEnabled && !soundMuted)
        oceanAmbientStart();
}


void oceanAmbientStart(void)
{
    if (!oceanLoaded || oceanPlaying || soundMuted)
        return;
    int ch = OCEAN_AMBIENT_VOICE;

    /* Key-off first so a re-keyed channel stops cleanly before the
     * new program write, then program + key with hardware verification
     * (see hardware-truth helpers above). ADSR1=0x00FF: instant attack,
     * no decay. */
    spuVoiceKeyOffVerified(ch);
    spuVoiceProgramVerified(ch, oceanPitch, oceanSpuAddr,
                            OCEAN_AMBIENT_VOLUME, OCEAN_AMBIENT_VOLUME,
                            0x00FF, 0x0000);
    spuVoiceKeyOnVerified(ch);
    oceanPlaying = 1;
}


void oceanAmbientStop(void)
{
    int ch = OCEAN_AMBIENT_VOICE;
    /* Deliberately NO oceanPlaying early-out: a swallowed key-off in a
     * stop-all path can leave the voice audibly keyed while the flag
     * says stopped, and Stop must always mean silence (the ENVX check
     * makes the already-silent case a fast no-op anyway).
     * Volume to zero first: even if the key-off write is swallowed the
     * voice goes inaudible immediately; the verified key-off then
     * retires it for real. */
    SPU_CH_VOL_L(ch) = 0;
    SPU_CH_VOL_R(ch) = 0;
    spuVoiceKeyOffVerified(ch);
    oceanPlaying = 0;
}


int oceanAmbientLoaded(void)
{
    return oceanLoaded;
}

/*
 * Shutdown audio system
 */
void soundEnd()
{
    if (soundDisabled) {
        return;
    }

    /* Stop all SPU channels (0xFFFFFF = all 24 channels) */
    soundStopAllVoices();
}

/*
 * Play sound effect by number using round-robin channel allocation
 */
void soundPlay(int nb)
{
    if (soundDisabled || soundMuted || !soundsLoaded) {
        return;
    }

    if (nb < 0 || nb >= MAX_SOUND_EFFECTS) {
        return;
    }

    if (soundAddresses[nb] == 0) {
        return;
    }

    int ch = nextChannel;
    nextChannel = (nextChannel + 1) % NUM_CHANNELS;

    /* Key-off first so a reused channel stops cleanly, then program and
     * key with hardware verification (see hardware-truth helpers above).
     * ADSR1=0x00FF: AR=0 -> instant attack; without it short samples
     * finish before the envelope ramps and you hear nothing. */
    spuVoiceKeyOffVerified(ch);
    spuVoiceProgramVerified(ch, soundPitches[nb], soundAddresses[nb],
                            SFX_VOLUME, SFX_VOLUME, 0x00FF, 0x0000);
    spuVoiceKeyOnVerified(ch);
}

/*
 * Stop a specific sound (key-off any channel playing it)
 */
void soundStop(int nb)
{
    if (soundDisabled || !soundsLoaded) return;
    if (nb < 0 || nb >= MAX_SOUND_EFFECTS) return;
    /* Key-off all channels that might be playing this sound.
     * We don't track which channel plays which sound, so stop all. */
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        SpuSetKey(0, 1 << ch);
    }
}

int soundEffectCount(void)
{
    return MAX_SOUND_EFFECTS;
}

int soundEffectLoaded(int nb)
{
    if (nb < 0 || nb >= MAX_SOUND_EFFECTS)
        return 0;
    return soundAddresses[nb] != 0 ? 1 : 0;
}

unsigned long soundEffectSizeBytes(int nb)
{
    if (nb < 0 || nb >= MAX_SOUND_EFFECTS)
        return 0;
    return (unsigned long)soundSizes[nb];
}

int soundEffectSampleRate(int nb)
{
    if (nb < 0 || nb >= MAX_SOUND_EFFECTS)
        return 0;
    return (int)soundSampleRates[nb];
}

unsigned long soundSpuDataEnd(void)
{
    return (unsigned long)soundSpuDataEndAddr;
}

unsigned long soundSpuCacheBase(void)
{
    return (unsigned long)soundSpuCacheBaseAddr;
}

unsigned long soundSpuCacheBytes(void)
{
    return (unsigned long)soundSpuCacheByteCount;
}

/*
 * Toggle sound mute on/off. PSn00bSDK's SpuSetCommonMasterVolume isn't
 * honored by DuckStation HLE; even direct master-vol register writes
 * weren't enough. So we go scorched-earth: zero every per-voice volume
 * register, the CD-mix volume registers, and the master volume — and
 * disable the SPU master enable in SPU_CTRL. Restore on unmute.
 *
 * Hardware regs (KSEG1, uncached):
 *   SPU voice 0..23: base 0xBF801C00, stride 0x10
 *     +0: vol L, +2: vol R, +4: pitch, +6: addr, +8: ADSR1, +A: ADSR2
 *   SPU_MAIN_VOL_L  = 0xBF801D80
 *   SPU_MAIN_VOL_R  = 0xBF801D82
 *   SPU_CD_VOL_L    = 0xBF801DB0
 *   SPU_CD_VOL_R    = 0xBF801DB2
 *   SPU_CTRL        = 0xBF801DAA  (bit 15 = SPU master enable)
 */
void soundMuteToggle(void)
{
    volatile uint16_t *masterL = (volatile uint16_t *)0xBF801D80;
    volatile uint16_t *masterR = (volatile uint16_t *)0xBF801D82;
    volatile uint16_t *cdL     = (volatile uint16_t *)0xBF801DB0;
    volatile uint16_t *cdR     = (volatile uint16_t *)0xBF801DB2;
    volatile uint16_t *spuCtrl = (volatile uint16_t *)0xBF801DAA;

    soundMuted = !soundMuted;
    if (soundMuted) {
        soundStopAllVoices();
        for (int v = 0; v < 24; v++) {
            volatile uint16_t *volL = (volatile uint16_t *)(0xBF801C00 + v * 0x10 + 0);
            volatile uint16_t *volR = (volatile uint16_t *)(0xBF801C00 + v * 0x10 + 2);
            *volL = 0;
            *volR = 0;
        }
        *masterL = 0;
        *masterR = 0;
        *cdL = 0;
        *cdR = 0;
        /* Clear SPU master enable bit (15) — disables all SPU output. */
        *spuCtrl = (uint16_t)(*spuCtrl & ~0x8000u);
    } else {
        *spuCtrl = (uint16_t)(*spuCtrl | 0x8000u);
        *masterL = 0x3FFF;
        *masterR = 0x3FFF;
        *cdL = 0x4000;
        *cdR = 0x4000;
        /* Voice volumes are reset by soundPlay on next playback;
         * we don't restore them here. Background loops that need to
         * resume must be re-keyed by their owner. */
        if (oceanLoaded && oceanAmbientEnabled && !oceanPlaying)
            oceanAmbientStart();
    }
}

/*
 * Load sound effect into SPU RAM (unused — loading happens in soundInit)
 */
int soundLoad(int nb, void *data, uint32_t size)
{
    (void)nb;
    (void)data;
    (void)size;
    return 0;
}
