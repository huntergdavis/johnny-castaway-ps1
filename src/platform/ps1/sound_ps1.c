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

static void soundStopAllVoices(void)
{
    volatile uint16_t *keyOff1 = (volatile uint16_t *)0xBF801D8C;
    volatile uint16_t *keyOff2 = (volatile uint16_t *)0xBF801D8E;
    *keyOff1 = 0xFFFFu;
    *keyOff2 = 0x00FFu;
    oceanPlaying = 0;
}

/* Read big-endian uint32 from VAG header */
static uint32_t readBE32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  | (uint32_t)p[3];
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

    /* Set master volume */
    SpuSetCommonMasterVolume(0x3FFF, 0x3FFF);

    /* Clear sound tables */
    for (int i = 0; i < MAX_SOUND_EFFECTS; i++) {
        soundAddresses[i] = 0;
        soundSizes[i] = 0;
        soundSampleRates[i] = 0;
    }

    /* Load VAG files from CD into SPU RAM */
    uint32_t spuAddr = SPU_DATA_START;
    int loaded = 0;

    for (int i = 0; i < MAX_SOUND_EFFECTS; i++) {
        char filename[32];
        sprintf(filename, "\\SND\\SOUND%02d.VAG;1", i);

        uint32_t vagSize = 0;
        uint8_t *vagData = ps1_loadRawFile(filename, &vagSize);
        if (!vagData) continue;

        if (vagSize <= VAG_HEADER_SIZE) {
            free(vagData);
            continue;
        }

        /* Parse VAG header — sample rate is big-endian at offset 16 */
        uint16_t sampleRate = (uint16_t)readBE32(vagData + 16);
        uint32_t adpcmSize = vagSize - VAG_HEADER_SIZE;
        /* SPU DMA moves data in 64-byte blocks; pad up or the final ADPCM
         * flag byte (end-of-sample) gets truncated and the voice never
         * stops, producing silence or noise instead of our sample. */
        uint32_t dmaSize = (adpcmSize + 63u) & ~63u;

        /* Check SPU RAM overflow (512KB total) */
        if (spuAddr + dmaSize > 512 * 1024) {
            printf("SPU: out of RAM at sound %d\n", i);
            free(vagData);
            break;
        }

        uint8_t *dmaBuf = (uint8_t *)malloc(dmaSize);
        if (!dmaBuf) {
            free(vagData);
            continue;
        }
        memcpy(dmaBuf, vagData + VAG_HEADER_SIZE, adpcmSize);
        if (dmaSize > adpcmSize)
            memset(dmaBuf + adpcmSize, 0, dmaSize - adpcmSize);

        /* Upload ADPCM data (skip VAG header) to SPU RAM */
        SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
        SpuSetTransferStartAddr(spuAddr);
        SpuWrite((uint32_t *)dmaBuf, dmaSize);
        SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
        free(dmaBuf);

        soundAddresses[i] = spuAddr;
        soundSizes[i] = adpcmSize;
        soundSampleRates[i] = sampleRate;
        soundPitches[i] = getSPUSampleRate(sampleRate);

        /* Advance by the DMA-aligned amount so the next sample does not
         * overlap the padding tail of this one. */
        spuAddr += dmaSize;

        free(vagData);
        loaded++;
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
        uint32_t vagSize = 0;
        uint8_t *vagData = ps1_loadRawFile("\\SND\\OCEAN.VAG;1", &vagSize);
        if (!vagData) {
            SOUND_DIAG_PRINTF("SPU: OCEAN.VAG not found\n");
            break;
        }
        if (vagSize <= VAG_HEADER_SIZE) {
            free(vagData);
            break;
        }
        uint16_t sampleRate = (uint16_t)readBE32(vagData + 16);
        uint32_t adpcmSize = vagSize - VAG_HEADER_SIZE;
        uint32_t dmaSize = (adpcmSize + 63u) & ~63u;

        if (spuAddr + dmaSize > 512 * 1024) {
            printf("SPU: out of RAM for OCEAN.VAG (need %lu, have %lu)\n",
                   (unsigned long)dmaSize,
                   (unsigned long)(512u * 1024u - spuAddr));
            free(vagData);
            break;
        }

        uint8_t *dmaBuf = (uint8_t *)malloc(dmaSize);
        if (!dmaBuf) { free(vagData); break; }
        memcpy(dmaBuf, vagData + VAG_HEADER_SIZE, adpcmSize);
        if (dmaSize > adpcmSize)
            memset(dmaBuf + adpcmSize, 0, dmaSize - adpcmSize);

        SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
        SpuSetTransferStartAddr(spuAddr);
        SpuWrite((uint32_t *)dmaBuf, dmaSize);
        SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
        free(dmaBuf);

        oceanSpuAddr   = spuAddr;
        oceanAdpcmSize = adpcmSize;
        oceanPitch     = getSPUSampleRate(sampleRate);
        oceanLoaded    = 1;
        spuAddr += dmaSize;

        free(vagData);
        SOUND_DIAG_PRINTF("SPU: OCEAN.VAG loaded (%lu bytes ADPCM @ 0x%lx, %u Hz)\n",
                          (unsigned long)adpcmSize, (unsigned long)oceanSpuAddr,
                          (unsigned)sampleRate);
    } while (0);

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
     * new program write. (Mirrors the SFX path's pre-key-off
     * convention.) */
    SpuSetKey(0, 1 << ch);

    SPU_CH_FREQ(ch)  = oceanPitch;
    SPU_CH_ADDR(ch)  = getSPUAddr(oceanSpuAddr);
    SPU_CH_VOL_L(ch) = OCEAN_AMBIENT_VOLUME;
    SPU_CH_VOL_R(ch) = OCEAN_AMBIENT_VOLUME;
    /* ADSR1=0x00FF: attack 0 (instant), no decay. Same envelope as SFX
     * for now — a slow attack via ADSR1's AR field would soften the
     * key-on, but on ocean material the audible difference is
     * negligible because the loop's first sec is itself a crossfade
     * tail with low amplitude. */
    SPU_CH_ADSR1(ch) = 0x00FF;
    SPU_CH_ADSR2(ch) = 0x0000;

    SpuSetKey(1, 1 << ch);
    oceanPlaying = 1;
}


void oceanAmbientStop(void)
{
    if (!oceanPlaying)
        return;
    int ch = OCEAN_AMBIENT_VOICE;
    SpuSetKey(0, 1 << ch);
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

    /* Key-off first so a reused channel stops cleanly before we reprogram it. */
    SpuSetKey(0, 1 << ch);

    /* Direct register writes mirroring PSn00bSDK's vagsample example.
     * ADSR1=0x00FF (AR=0 → instant attack; without this, short samples
     * finish before the envelope ramps up and you hear nothing).
     * ADSR2=0x0000 (no sustain/release curve). */
    SPU_CH_FREQ(ch)  = soundPitches[nb];
    SPU_CH_ADDR(ch)  = getSPUAddr(soundAddresses[nb]);
    SPU_CH_VOL_L(ch) = SFX_VOLUME;
    SPU_CH_VOL_R(ch) = SFX_VOLUME;
    SPU_CH_ADSR1(ch) = 0x00FF;
    SPU_CH_ADSR2(ch) = 0x0000;

    /* Start playback */
    SpuSetKey(1, 1 << ch);
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
