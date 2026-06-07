# Hand-Rolled MIPS Assembly — Feasibility Research

Date: 2026-04-30
Branch context: `walk-implementation-20260429` (read-only research; no code changes)
Author: research pass for the active perf iteration.

## Question

> Walks/scenes are now processing-bound rather than CD-bound. Could we
> hand-roll assembly for our most common operations and get a real
> speed-up? Is this viable on PS1?

Short answer: **yes, viable, modest gains — but cheaper wins exist
first.** Inline assembly is well-supported by PSn00bSDK + GCC and there
is real headroom in the per-pixel composite paths. The realistic ceiling
for asm-only changes is ~20-40% on the loops we'd target. Getting a
similar gain by widening the data path (lw/sh in C, palette in
scratchpad, per-file `-O2` instead of `-Os`) takes a fraction of the
engineering risk and most of the win is independent of whether we
actually drop to asm.

## Hardware Refresh

R3000A on the PS1:

- 33.8688 MHz, single-issue, in-order, 5-stage pipeline.
- 4 KB I-cache, **no D-cache** — only a 1 KB scratchpad at `0x1F800000`.
- Branch delay slot (the instruction *after* a branch always executes).
- Load delay slot (the result of a load is **not** available the very
  next cycle; the slot is filled by the assembler if you let it).
- No branch prediction, no out-of-order, no speculative execution.
- Multiply / divide are on a separate unit with multi-cycle latency
  (`mflo` / `mfhi` interlock).

Practical implications for hot loops:

- Memory bandwidth is the actual bottleneck for any compositor at 16bpp.
  RAM-to-RAM throughput in a tight `lw`/`sw` loop is ~30-40 MB/s; a
  byte-level loop is ~6-10 MB/s. The 4× ratio is the single biggest
  lever we have.
- Word-aligned bulk copy is the regime where MIPS shines. Byte/halfword
  loops with a load-use stall every iteration are where C compilers tend
  to leave cycles on the table.
- The scratchpad is a real perf tool — `0x1F800000`-`0x1F8003FF` is
  effectively L1 cache the program controls. Putting the active 16-entry
  palette and a small scratch buffer there during compose can avoid
  uncached-RAM stalls.

## Toolchain and Inline Asm Support

PSn00bSDK ships with GCC for `mipsel-none-elf` (PSn00bSDK 0.24, GCC
13.x). Inline asm is the standard `__asm__ volatile (...)` form with
`"r"` / `"=r"` constraints; PSn00bSDK's own headers
(`scratch/psn00b-src/libpsn00b/include/inline_c.h`,
`psxgte.h`) use it heavily for GTE intrinsics. Our codebase already has
five inline asm sites (`src/platform/ps1/events_ps1.c`, `src/platform/ps1/spi.c`) for COP0 / GP /
SP register access.

Mechanically there is no friction. Per-file optimization flags also
work — we currently mix `-Os` for size-sensitive TUs with the default
`-O2` for the active hot path. Both can be raised on a per-function
basis with `__attribute__((optimize("O2")))` (already used in
`grUpdateDisplay` / `grDrawBackground`).

## Hot Path Inventory

From `src/graphics_ps1/graphics_ps1.c` plus the JCPERF2 phase tags in `src/platform/ps1/ps1_perf.c`:

| Phase | Function | Per-frame cost | Where the cycles go |
|---|---|---|---|
| Compose | `compositePsbSpanFwd` / `compositeIndexedSpanFwd` | High when sprite count is high | 1B packed read → palette LUT → 16-bit store, with a transparent-pixel branch |
| Compose | `grCompositePacked4OpaqueRun` | Hot | Same shape, no transparent branch (PAL4 spans are pre-stripped) |
| Compose | `grCompositePacked4TemporalResidualToBackground` | Per-frame, fishing1 | Wraps the opaque-run helper |
| Restore | `grCleanRectCopyIn` / `grRestoreBgFromRects` | Per-frame, gated by prevDirty | Per-row `memcpy` into bgTile, possibly two halves per row (left/right tile split) |
| Upload | `grDrawBackground` → `LoadImage` | Per-frame, gated by union(prev,curr) dirty | DMA from bgTile to VRAM; CPU side is the band/rect computation |
| Composite (sprite) | `grCompositeToBackground` / `Flip` | Per sprite per frame | Calls into the indexed/PSB span helpers above |

A reasonable JCPERF2 sample for fishing1 (no holiday, night, lowtide,
raft 4) showed `loop_vb=1221`, `restore_calls=195`, `compose_calls=195`,
`upload_calls=155` for ~155 frames of scene playback. Compose runs
roughly once per frame; restore and upload are gated by dirty rows.

## Where Asm Actually Helps

### A. Inner pixel loops (real win, modest absolute size)

The current `compositePsbSpanFwd` 2-pixel inner loop compiles to
roughly:

```
lbu     packed, 0(src)        ; 2 cycles (load delay)
nop                           ; or filled by next instr
srl     idx0, packed, 4
andi    idx1, packed, 0x0F
sll     idx0w, idx0, 1
sll     idx1w, idx1, 1
addu    addr0, pal, idx0w
addu    addr1, pal, idx1w
lhu     p0, 0(addr0)          ; 2 cycles
lhu     p1, 0(addr1)          ; 2 cycles
beqz    p0, skip0
sh      p0, 0(dst)
skip0:
beqz    p1, skip1
sh      p1, 2(dst)
skip1:
addiu   dst, dst, 4
addiu   src, src, 1
bne     count, 0, loop
```

That's ~14-16 cycles per pixel pair when the transparent branches
predict the common case. A hand-tuned version can:

1. Drop the transparent branch entirely for spans that are guaranteed
   opaque (the PAL4 path already does this; PSB and indexed paths still
   have it).
2. **Read packed pixels by word** (`lw` = 4 packed bytes = 8 pixels per
   load instead of 1 byte = 2 pixels). Cuts source-side load count by
   4×.
3. **Pack two 16-bit stores into one word store** (`sw`) when the dst
   address is word-aligned and the two destination half-words are both
   non-transparent. Same idea — half the store count.
4. Software-pipeline so the load-delay slot is filled with useful work
   instead of a `nop`.

A hand-written 8-pixels-per-iteration loop that's word-aligned both
sides hits ~6-8 cycles per pixel pair, so ~2× over the current C inner
loop. On a sprite-heavy frame (say 100K composite pixel pairs) that's
~1 ms saved, ~6% of the 16 ms VBlank budget. Useful but not magic.

The **real** win in this area is usually a *C* refactor: read packed
pixels as `uint32`, store pixel pairs as `uint32` via `__attribute__
((aligned))` casts. That captures most of the asm speedup with no
target-specific code.

### B. Per-row memcpy in clean-rect restore (real win, hot)

`grCleanRectCopyIn` calls `memcpy` per row. newlib's MIPS memcpy is
generic and pays for alignment dispatch every call. For the restore
path our rows are always:

- 16-bit aligned (we control bgTile layout).
- Often word-aligned (when X starts on an even pixel).
- Bounded width (≤ 320 px = 640 bytes per left/right half).

A custom per-row word-loop with software pipelining, possibly with an
unrolled tail handler, runs at peak RAM bandwidth (~33 MB/s effective).
For the typical walk-frame restore (~30-50 dirty rows × 480 bytes ≈
20 KB) that's ~0.7 ms vs the C memcpy's likely 1.5-2 ms. **~1 ms
saved per frame** — meaningful when we're trying to hold 60 Hz inside a
16.7 ms VBlank.

### C. Palette LUT in scratchpad (compose only)

The 16-entry × 2-byte palette = 32 bytes. PS1 scratchpad is 1 KB. If we
copy the active palette to scratchpad at the start of each compose
pass, the inner loop's `lhu p, 0(palAddr)` becomes a guaranteed
single-cycle load instead of a potentially uncached RAM access. Only
helps if compose is the bottleneck *and* we're stalling on palette
reads (likely in temporal-residual scenes with many small spans).
Estimated win: 5-10% on the compose phase.

This is **not asm** per se — it's choosing where data lives. But it
combines naturally with hand-written inner loops.

## Where Asm Doesn't Help (or is the wrong tool)

1. **DMA-bound paths** (`LoadImage`, `CdRead`). The CPU is just
   triggering DMA. Hand-writing the trigger code in asm saves nothing.
2. **Already-vectorizable loops** that GCC compiles well at `-O2`. The
   PAL4 opaque-run helper at `-O2` is within ~10% of an asm rewrite.
3. **Branchy state machines** (walk.c `walkAnimate`). The compiler is
   fine here; the overhead is structural, not codegen.
4. **CD seek + read latency**. Asm doesn't make the disc spin faster.

## Cheaper Alternatives To Try First

Ordered by win-per-effort:

1. **Per-file `-O2` for graphics_ps1.c** if it isn't already.
   `CMakeLists.txt:86` lists files compiled `-Os`; check whether
   `graphics_ps1.c` is in that list. If yes, moving it to `-O2`
   typically gives 5-15% on hot loops with zero risk and a one-line
   change.
2. **Word-stride C refactor** of the indexed-span helpers. Read packed
   pixels as `uint32`, decode 8 nibbles per load, store pairs as
   `uint32`. Captures most of the asm gain; portable, debuggable.
3. **Palette in scratchpad** during compose (helper macro to memcpy 32
   bytes to `0x1F800000` at start of compose pass, restore palette
   pointer). 5-10% compose, low risk.
4. **Hand-rolled per-row copy** for `grCleanRectCopyIn` only. Small
   asm surface (~30 instructions). Replaces a hot `memcpy` with a
   bounded, word-aligned loop. ~1 ms per restored frame.
5. **Hand-rolled compose inner loop** for the remaining transparent-
   capable paths (`compositePsbSpanFwd`, `compositeIndexedSpanFwd`).
   Large asm surface (~80-150 instructions including alignment
   prologue/epilogue). Estimated 1-2 ms per frame.

Items 1-4 are all sub-1-day. Item 5 is multi-day with non-trivial
testing surface (correctness across stride/alignment cases, all four
fwd/rev variants).

## Risk and Maintenance Cost

The honest cost of hand-rolled asm at this scale:

- **Test surface** at least 4× (fwd/rev × indexed/PSB × aligned/
  unaligned). Pixel-perfect divergence from the C version is what gets
  caught on day 12, not day 1.
- **Compiler version drift**. Inline asm with explicit register names
  pins us to a clobber list that can break with a GCC upgrade. The
  PSn00bSDK GTE macros pin specific COP2 register numbers and have
  survived multiple GCC bumps, so this is manageable but real.
- **Capture / regtest harness** doesn't currently diff at the per-pixel
  composite level — visual hashes are full-frame. A compositor regression
  in a low-coverage corner case wouldn't be caught by the
  `--require-improvement` gate; only by a human eye.

That last item is the most underrated. The walk subsystem already
exposed how easy it is to ship a visual regression that the perf gate
clears. Hand-rolled asm magnifies that risk because the code reads
linearly but failure modes are non-linear (one off-by-one in a delay
slot, one mis-clobbered register).

## Recommended Plan If We Pursue Asm

If the user decides this *is* the next direction, the order I'd take it:

1. **Measurement gate first.** Add a JCPERF2 counter for cycles-spent-
   in-compose vs cycles-spent-in-restore vs upload, gated to a perf-
   debug build only. We have phase markers but they're VBlank-resolution
   (~16 ms granularity). For asm work we want sub-ms.
2. **Pick one path** — `grCleanRectCopyIn` is the highest win-per-line,
   shallowest risk, smallest surface.
3. **Word-stride C refactor first**, measure. If the gate clears it,
   that's free win, ship and move on.
4. **Asm pass only if the C refactor leaves cycles on the table.** Keep
   the asm and C versions both compiled, switchable via a build flag,
   so the regtest harness can A/B them in CI.
5. **Lock in scratchpad** for the 32-byte active palette + maybe a
   64-byte scratch row buffer. Document the scratchpad layout in
   `docs/ps1/hardware-specs.md`.
6. **Compose-loop asm last**, only if 1-5 don't get us to target.

## Concrete Targets and Estimated Wins

The numbers below are from inspection + per-cycle accounting, not
measurement. JCPERF2 has VBlank granularity, so anything < 1 VBlank
(16.7 ms) is below current perf-gate resolution and would need finer
counters to validate.

| Target | Effort | Wall-clock win/frame | Confidence |
|---|---|---|---|
| `graphics_ps1.c` `-O2` instead of `-Os` | 1 line | 1-3 ms | High |
| Word-stride compose C refactor | 1 day | 0.5-2 ms | High |
| Palette in scratchpad | 0.5 day | 0.2-0.8 ms | Medium |
| Hand-rolled `grCleanRectCopyIn` row copy | 1 day | 0.5-1.5 ms | High |
| Hand-rolled `compositePsbSpanFwd` | 3-5 days | 1-2 ms | Medium |
| Hand-rolled `compositeIndexedSpanFwd` | 3-5 days | 1-2 ms | Medium |
| Hand-rolled `compositePacked4OpaqueRun` | 2-3 days | 0.5-1 ms | Medium |
| **Total upper bound (everything done well)** | — | **5-12 ms / frame** | — |

That upper bound is roughly the difference between "60 Hz with
occasional stutter" and "60 Hz steady" on a sprite-heavy frame. Real,
not transformative.

## Verdict

**Hand-rolled asm is technically viable and would give real but
modest gains.** The toolchain supports it cleanly and PSn00bSDK
already uses the same idioms.

But the way to get most of that benefit is:

1. Confirm `graphics_ps1.c` is on `-O2` (likely already is).
2. Word-stride the composite inner loops in *C*.
3. Move the active palette into scratchpad during compose.
4. Hand-roll `grCleanRectCopyIn`'s per-row copy (small, contained).

Only after that should we consider asm-rewriting the larger compose
helpers. Each of items 1-4 is a self-contained promotion; the risk
profile is similar to any other entry on the
`docs/ps1/performance-experiment-log.md` ladder.

If we reach the asm step, scope it to *one* function, hide it behind a
build flag, and keep the C reference compiled so regtest can A/B
trivially.

## Files Worth Re-Reading Before Starting Asm Work

- `src/graphics_ps1/graphics_ps1.c` — `compositeIndexedSpanFwd` (1349), `compositePsbSpanFwd` (1423), `grCompositePacked4OpaqueRun` (1824), `grCleanRectCopyIn` (~2914).
- `src/platform/ps1/ps1_perf.c` — phase markers and JCPERF2 counter shape if a finer counter is needed.
- `scratch/psn00b-src/libpsn00b/include/inline_c.h` — reference for the inline-asm conventions PSn00bSDK uses.
- `scratch/psn00b-src/libpsn00b/include/psxpress.h` — scratchpad usage example (DCT decompression).
- `docs/ps1/performance-optimization-plan.md` and `docs/ps1/performance-experiment-log.md` — what's already been tried, what regressed.
- `docs/ps1/hardware-specs.md` — scratchpad address, GTE registers, COP0 reference.
