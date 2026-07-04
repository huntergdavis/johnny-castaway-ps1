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

/* DMA4 (SPU) channel registers. CHCR bit 24 is the channel-busy flag,
 * set synchronously by the very CHCR write that kicks the transfer.
 * These are CPU-side registers: readable even on consoles whose SPU
 * register reads are broken (see gSpuRegReadsOk). */
#define SPU_DMA4_MADR (*(volatile uint32_t *)0xBF8010C0)
#define SPU_DMA4_BCR  (*(volatile uint32_t *)0xBF8010C4)
#define SPU_DMA4_CHCR (*(volatile uint32_t *)0xBF8010C8)

/* Raw SPU registers used by the write-only transfer core. */
#define JC_SPU_ADDR_REG  (*(volatile uint16_t *)0xBF801DA6)
#define JC_SPU_CTRL_REG  (*(volatile uint16_t *)0xBF801DAA)
#define JC_SPU_STAT_REG  (*(volatile uint16_t *)0xBF801DAE)
#define JC_SPU_DELAY_REG (*(volatile uint32_t *)0xBF801014)

/* Console truth, learned in the burntest rounds: on (at least) tonyhax-
 * chainloaded units, SPU register READS can return bus garbage (0xFFFF)
 * while writes work fine. Every read-modify-write and every status poll
 * on an SPU register is then poison: the SDK's _dma_transfer RMWs
 * SPUCNT (writing garbage back) and polls SPUSTAT (never "completes").
 * The transfer core below is WRITE-ONLY w.r.t. SPU registers: SPUCNT
 * goes through a software shadow, completion is tracked on the DMA4
 * CHCR (CPU-side, always readable), and SPUSTAT is polled only when a
 * boot-time probe said reads are healthy. */
static uint16 gSpuCntShadow = 0xC001;
static int gSpuRegReadsOk = 1;

void ps1SpuNoteInitDone(void)
{
    /* Called right after SpuInit() (which leaves SPUCNT = 0xC001).
     * If reading it back doesn't match, this unit's SPU register reads
     * are broken and all read-dependent logic must stand down. */
    gSpuCntShadow = 0xC001;
    gSpuRegReadsOk = (JC_SPU_CTRL_REG == 0xC001) ? 1 : 0;
}

int ps1SpuRegReadsOk(void)
{
    return gSpuRegReadsOk;
}

/* Wait for SPUSTAT to acknowledge a transfer-mode change when reads are
 * healthy; a short fixed settle otherwise (the ack takes microseconds). */
static void spuModeSettle(uint16 wantMode)
{
    int i;
    if (gSpuRegReadsOk) {
        for (i = 0; i < 200000; i++) {
            if ((JC_SPU_STAT_REG & 0x0030u) == wantMode)
                return;
        }
    } else {
        for (i = 0; i < 3000; i++)
            (void)SPU_DMA4_CHCR;   /* uncached bus reads as a delay */
    }
}

/* Write-only SPU DMA transfer. bytes must be 64-aligned (16-word DMA
 * blocks — the SPU FIFO handshake requirement). Returns 0 on a wedged
 * channel (aborted via CHCR so the next transfer starts clean). */
static int spuDmaTransfer(uint32 spuByteAddr, void *ram, uint32 bytes,
                          int isWrite)
{
    int i;

    if (bytes == 0u || (bytes & 63u) != 0u || ((uint32)ram & 3u) != 0u)
        return 0;

    /* Park at Stop from the shadow, never from a register read. */
    JC_SPU_CTRL_REG = (uint16)(gSpuCntShadow & ~0x0030u);
    spuModeSettle(0x0000);

    /* SPU bus delay: absolute canonical values; DMA reads need the
     * read-direction pattern in bits 24-27 (what the SDK derives by
     * RMW — garbage when reads are broken). */
    JC_SPU_DELAY_REG = isWrite ? 0x200931E1u : 0x220931E1u;

    JC_SPU_ADDR_REG = (uint16)(spuByteAddr >> 3);
    JC_SPU_CTRL_REG = (uint16)((gSpuCntShadow & ~0x0030u) |
                               (isWrite ? 0x0020u : 0x0030u));
    spuModeSettle(isWrite ? 0x0020u : 0x0030u);

    SPU_DMA4_MADR = (uint32)ram & 0x00FFFFFFu;
    SPU_DMA4_BCR  = 16u | ((bytes >> 6) << 16);
    SPU_DMA4_CHCR = isWrite ? 0x01000201u : 0x01000200u;

    for (i = 0; i < 400000 && (SPU_DMA4_CHCR & (1u << 24)); i++)
        ;
    for (i = 0; i < 120 && (SPU_DMA4_CHCR & (1u << 24)); i++)
        VSync(0);
    if (SPU_DMA4_CHCR & (1u << 24)) {
        /* Wedged: abort the channel so the NEXT transfer isn't kicked
         * on top of a busy one (that cascade is what killed all audio
         * in burntest6). */
        SPU_DMA4_CHCR = 0x00000201u;
        JC_SPU_CTRL_REG = (uint16)(gSpuCntShadow & ~0x0030u);
        JC_SPU_DELAY_REG = 0x200931E1u;
        return 0;
    }

    if (isWrite) {
        /* FIFO drain tail (<=64 bytes past DMA completion). */
        if (gSpuRegReadsOk) {
            for (i = 0; i < 256; i++) {
                if (!SpuIsTransferCompleted(SPU_TRANSFER_PEEK))
                    break;
            }
            for (i = 0; i < 200000; i++) {
                if (SpuIsTransferCompleted(SPU_TRANSFER_PEEK))
                    break;
            }
        } else {
            for (i = 0; i < 4000; i++)
                (void)SPU_DMA4_CHCR;
        }
    }

    JC_SPU_CTRL_REG = (uint16)(gSpuCntShadow & ~0x0030u);
    spuModeSettle(0x0000);
    JC_SPU_DELAY_REG = 0x200931E1u;
    return 1;
}

int ps1SpuDmaWrite(uint32 spuByteAddr, const void *src, uint32 bytes)
{
    return spuDmaTransfer(spuByteAddr, (void *)src, bytes, 1);
}

int ps1SpuDmaRead(uint32 spuByteAddr, void *dst, uint32 bytes)
{
    return spuDmaTransfer(spuByteAddr, dst, bytes, 0);
}

int ps1SpuCacheWrite(uint32 offset, const void *src, uint32 size)
{
    if (!gSpuCacheReady || !src || !ps1SpuCacheRangeOk(offset, size))
        return 0;

    if (!ps1SpuDmaWrite(gSpuCacheBase + offset, src, size)) {
        /* Mark the cache unusable so every caller takes its documented
         * CD-fallback path instead of piling onto a wedged SPU. */
        gSpuCacheReady = 0;
        return 0;
    }
    return 1;
}

int ps1SpuCacheRead(uint32 offset, void *dst, uint32 size)
{
    if (!gSpuCacheReady || !dst || !ps1SpuCacheRangeOk(offset, size))
        return 0;

    if (!ps1SpuDmaRead(gSpuCacheBase + offset, dst, size)) {
        gSpuCacheReady = 0;
        return 0;
    }
    return 1;
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
