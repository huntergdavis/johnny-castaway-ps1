/*
 *  PS1 GPU ordering-table helpers.
 *
 *  PSn00bSDK's addPrim() macro is correct for PS1 display lists, but it
 *  casts OT entries to the full primitive tag struct. GCC 12 can warn that
 *  the cast partially extends past a one-word OT entry. These helpers perform
 *  the same 24-bit link update through a may_alias word view.
 */

#ifndef PS1_GPU_OT_H
#define PS1_GPU_OT_H

#include <stdint.h>

typedef uint32_t Ps1GpuOtWord __attribute__((__may_alias__));

static inline uint32_t ps1GpuOtAddr(const void *entry)
{
    const Ps1GpuOtWord *word = (const Ps1GpuOtWord *)entry;
    return (*word) & 0x00ffffffu;
}

static inline void ps1GpuOtSetAddr(void *entry, uint32_t addr)
{
    Ps1GpuOtWord *word = (Ps1GpuOtWord *)entry;
    *word = (*word & 0xff000000u) | (addr & 0x00ffffffu);
}

static inline void ps1GpuOtAddPrim(void *otEntry, void *prim)
{
    ps1GpuOtSetAddr(prim, ps1GpuOtAddr(otEntry));
    ps1GpuOtSetAddr(otEntry, (uint32_t)(uintptr_t)prim);
}

#endif /* PS1_GPU_OT_H */
