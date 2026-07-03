/*
 * PS1 SPU cache scratch window.
 *
 * The CPU cannot address SPU RAM directly, so this is a cold-storage cache:
 * callers DMA bytes from main RAM to unused SPU RAM, then DMA them back to
 * main RAM before CPU/GPU use. It is useful only for parking prefetched data
 * after the CD read has completed.
 */

#include <psxspu.h>
#include <psxgpu.h> /* VSync — bounded SPU transfer wait */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifndef PS1_SPU_CACHE_DIAG_LOGS
#define PS1_SPU_CACHE_DIAG_LOGS 0
#endif
#if PS1_SPU_CACHE_DIAG_LOGS
#include <stdio.h>
#endif

#include "mytypes.h"
#include "sound_ps1.h"
#include "ps1_spu_cache.h"

#if PS1_SPU_CACHE_DIAG_LOGS
#define SPU_CACHE_PRINTF(...) do { printf(__VA_ARGS__); } while (0)
#else
#define SPU_CACHE_PRINTF(...) do { } while (0)
#endif

static uint32 gSpuCacheBase = 0;
static uint32 gSpuCacheBytes = 0;
static int gSpuCacheReady = 0;

static uint32 ps1SpuCacheAlignUp(uint32 value)
{
    return (value + (PS1_SPU_CACHE_ALIGN - 1u)) & ~(PS1_SPU_CACHE_ALIGN - 1u);
}

static uint32 ps1SpuCacheAlignDown(uint32 value)
{
    return value & ~(PS1_SPU_CACHE_ALIGN - 1u);
}

static int ps1SpuCacheRangeOk(uint32 offset, uint32 size)
{
    if (!gSpuCacheReady || size == 0)
        return 0;
    if ((offset & (PS1_SPU_CACHE_ALIGN - 1u)) != 0 ||
        (size & (PS1_SPU_CACHE_ALIGN - 1u)) != 0)
        return 0;
    if (offset > gSpuCacheBytes)
        return 0;
    if (size > gSpuCacheBytes - offset)
        return 0;
    return 1;
}

void ps1SpuCacheInit(void)
{
    uint32 rawBase = (uint32)soundSpuCacheBase();
    uint32 rawBytes = (uint32)soundSpuCacheBytes();
    uint32 base = ps1SpuCacheAlignUp(rawBase);
    uint32 bytes = rawBytes;

    if (base > rawBase) {
        uint32 skipped = base - rawBase;
        bytes = (skipped < rawBytes) ? (rawBytes - skipped) : 0;
    }

    gSpuCacheBase = base;
    gSpuCacheBytes = ps1SpuCacheAlignDown(bytes);
    gSpuCacheReady = (gSpuCacheBase != 0 && gSpuCacheBytes >= PS1_SPU_CACHE_ALIGN);
}

int ps1SpuCacheReady(void)
{
    return gSpuCacheReady;
}

uint32 ps1SpuCacheBase(void)
{
    return gSpuCacheBase;
}

uint32 ps1SpuCacheCapacity(void)
{
    return gSpuCacheBytes;
}

/* DMA4 (SPU) channel control register. Bit 24 is the channel-busy flag,
 * set synchronously by the very CHCR write that kicks the transfer. */
#define SPU_DMA4_CHCR (*(volatile uint32_t *)0xBF8010C8)

/* Bounded two-phase SPU transfer wait, safe on real silicon.
 *
 * SPUSTAT bit 10 — the flag SpuIsTransferCompleted polls — ASSERTS WITH A
 * DELAY after a transfer kicks off on real hardware, while emulators
 * complete transfers synchronously. Polling it alone can therefore report
 * "completed" in the window before the flag asserts, letting the caller
 * reprogram the transfer registers while the previous DMA is still in
 * flight. Emulators never show the window, which is why boot VAG uploads,
 * walk-clean row captures and staging transfers were corrupted ONLY on
 * console (silent SFX, walk ghosts, dead ambience, staging fallbacks).
 *
 * Phase 1 waits on DMA4 CHCR.24 instead — set at kick time with no assert
 * delay, so it can never be observed clear before the transfer started.
 * Phase 2 then covers the SPU's FIFO-drain tail via SPUSTAT (writes only;
 * for reads the DMA delivering the last word IS completion). Phase 3
 * parks the transfer state machine back at Stop: the SDK's _dma_transfer
 * leaves SPUCNT in DMA-read/-write mode after a transfer, and a real SPU
 * idling in DMA-read mode services voice/key register writes unreliably
 * (emulators don't model this) — an SPU parked in read mode after the
 * first walk/staging read silently ate every later SpuSetKey: SFX and
 * sound-test key-ons, and the ambience toggle's key-off. All waits are
 * bounded (~2s ceilings) so a wedged SPU degrades instead of hanging. */
int ps1SpuTransferWaitBounded(int isWrite)
{
    volatile uint16_t *spucnt  = (volatile uint16_t *)0xBF801DAA;
    volatile uint16_t *spustat = (volatile uint16_t *)0xBF801DAE;
    int ok = 1;
    int i;

    if (SPU_DMA4_CHCR & (1u << 24)) {
        for (i = 0; i < 200000; i++) {
            if ((SPU_DMA4_CHCR & (1u << 24)) == 0u)
                break;
        }
        for (i = 0; i < 120 && (SPU_DMA4_CHCR & (1u << 24)); i++)
            VSync(0);
        if (SPU_DMA4_CHCR & (1u << 24))
            return 0;
    }

    if (isWrite) {
        /* Tiny transfers can complete the DMA before SPUSTAT.10 has even
         * asserted, making "clear" ambiguous (drained vs not-yet-started).
         * Give the flag a short window to assert; if it never does within
         * the window the FIFO tail (<=64 bytes) has long since drained. */
        for (i = 0; i < 256; i++) {
            if (!SpuIsTransferCompleted(SPU_TRANSFER_PEEK))
                break;
        }
        ok = 0;
        for (i = 0; i < 200000 && !ok; i++)
            ok = SpuIsTransferCompleted(SPU_TRANSFER_PEEK);
        for (i = 0; i < 120 && !ok; i++) {
            VSync(0);
            ok = SpuIsTransferCompleted(SPU_TRANSFER_PEEK);
        }
    }

    /* Park the transfer state machine at Stop and wait for the mode ack
     * (SPUSTAT bits 4-5 mirror the applied SPUCNT mode). */
    *spucnt = (uint16_t)(*spucnt & ~0x0030u);
    for (i = 0; i < 200000; i++) {
        if ((*spustat & 0x0030u) == 0u)
            break;
    }
    return ok;
}

static int spuCacheWaitTransferBounded(int isWrite)
{
    if (!ps1SpuTransferWaitBounded(isWrite)) {
        /* Mark the cache unusable so every caller takes its documented
         * CD-fallback path instead of piling onto a wedged SPU. */
        gSpuCacheReady = 0;
        return 0;
    }
    return 1;
}

int ps1SpuCacheWrite(uint32 offset, const void *src, uint32 size)
{
    if (!gSpuCacheReady || !src || !ps1SpuCacheRangeOk(offset, size))
        return 0;

    SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
    SpuSetTransferStartAddr(gSpuCacheBase + offset);
    SpuWrite((uint32_t *)src, size);
    return spuCacheWaitTransferBounded(1);
}

int ps1SpuCacheRead(uint32 offset, void *dst, uint32 size)
{
    if (!gSpuCacheReady || !dst || !ps1SpuCacheRangeOk(offset, size))
        return 0;

    SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
    SpuSetTransferStartAddr(gSpuCacheBase + offset);
    SpuRead((uint32_t *)dst, size);
    return spuCacheWaitTransferBounded(0);
}

int ps1SpuCacheSelfTest(void)
{
    uint32 testSize;
    uint8 *src;
    uint8 *dst;
    uint32 sum = 0;
    int ok = 0;

    ps1SpuCacheInit();
    testSize = gSpuCacheBytes;
    if (testSize > 4096u)
        testSize = 4096u;
    testSize = ps1SpuCacheAlignDown(testSize);

    if (!gSpuCacheReady || testSize < PS1_SPU_CACHE_ALIGN) {
        SPU_CACHE_PRINTF("JCSPUCACHE fail rc=1 base=%lu bytes=%lu\n",
                         (unsigned long)gSpuCacheBase,
                         (unsigned long)gSpuCacheBytes);
        return 0;
    }

    src = (uint8 *)malloc(testSize);
    dst = (uint8 *)malloc(testSize);
    if (!src || !dst) {
        if (src) free(src);
        if (dst) free(dst);
        SPU_CACHE_PRINTF("JCSPUCACHE fail rc=2 base=%lu bytes=%lu\n",
                         (unsigned long)gSpuCacheBase,
                         (unsigned long)gSpuCacheBytes);
        return 0;
    }

    for (uint32 i = 0; i < testSize; i++) {
        uint8 value = (uint8)(((i * 37u) + (i >> 3) + 0x5au) & 0xffu);
        src[i] = value;
        dst[i] = 0;
        sum = (sum << 5) ^ (sum >> 27) ^ (uint32)value;
    }

    ok = ps1SpuCacheWrite(0, src, testSize) &&
         ps1SpuCacheRead(0, dst, testSize) &&
         memcmp(src, dst, testSize) == 0;

    SPU_CACHE_PRINTF("JCSPUCACHE %s base=%lu bytes=%lu test=%lu sum=%08lx\n",
                     ok ? "ok" : "fail",
                     (unsigned long)gSpuCacheBase,
                     (unsigned long)gSpuCacheBytes,
                     (unsigned long)testSize,
                     (unsigned long)sum);

    free(dst);
    free(src);
    return ok;
}
