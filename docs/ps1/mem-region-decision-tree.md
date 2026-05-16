# Memory region decision tree

Companion to [memory-region-allocator-plan.md](./memory-region-allocator-plan.md).

You're adding an allocation site. Walk the tree below to pick the
region. If none fit, the allocation probably shouldn't exist — ask in
PR review before bypassing this.

```
┌─ How long does the allocation live? ───────────────────────────────┐
│                                                                    │
│  Single function call, freed before return?                        │
│      → TRANSIENT.  Scene-relative scratch (e.g., decompress        │
│                    buffer, BMP→surface conversion staging).        │
│                    Wiped wholesale by memSceneReset.               │
│                                                                    │
│  One scene, freed at fgRuntimeReset?                               │
│      → TRANSIENT.  Entry tables, sound events, setup segment,      │
│                    clean-rect snapshots.                           │
│                                                                    │
│  Multiple scenes, can be evicted by LRU when memory's tight?       │
│      → CACHE.      Resource blobs (BMP/TTM/SCR/ADS uncompressed    │
│                    data). The current LRU + pinResource policy     │
│                    handles eviction.                               │
│                                                                    │
│  Whole program lifetime, allocated once at boot, never freed?      │
│      → BOOT.       Engine state, font tables, audio mixer,         │
│                    surface pool, walk-clean buffer, pre-sized      │
│                    frame buffers, resource struct arrays,          │
│                    pause-menu state, grBackgroundSfc backing.      │
│                                                                    │
│  Per-frame, freed before next frame?                               │
│      → TRANSIENT.  Treated identically to per-scene; the wipe is   │
│                    cheap. (If your allocation is in a tight per-   │
│                    frame loop, prefer a stack buffer or a pre-     │
│                    allocated BOOT scratch over allocating each     │
│                    frame.)                                         │
│                                                                    │
└────────────────────────────────────────────────────────────────────┘
```

## Common cases worked through

### "I need a 4 KB decompression scratch in my new scene loader"

Lifetime: single function call. → **TRANSIENT** (annotated
`INIT_FULL_WRITE` — the loader writes every byte before reading).

```c
uint8 *scratch = memAlloc(MEM_REGION_TRANSIENT, 4096, "mySceneDecompress");
/* decompress into scratch */
/* scratch lives until memSceneReset; no explicit free needed. */
```

### "My new audio effect needs a 16 KB ring buffer for its lifetime"

Lifetime: whole program. Audio effect is initialized at boot, runs
forever. → **BOOT** (annotated `INIT_ZEROED` — bump allocator does
not zero, so caller must memset if it depends on zero-init).

```c
uint8 *ring = memAlloc(MEM_REGION_BOOT, 16 * 1024, "audioEffectRing");
memset(ring, 0, 16 * 1024);  /* INIT_ZEROED */
gMyEffect.ring = ring;
```

### "I need to cache a procedurally-generated palette across scenes"

If "across scenes" means "until evicted under pressure" → **CACHE**
(needs a pin/unpin contract via `pinResource`/`unpinResource`). If it
means "always available" → **BOOT**. The decision is whether you can
afford to regenerate it on the rare cache miss.

```c
uint16 *palette = memAlloc(MEM_REGION_CACHE, 256 * sizeof(uint16),
                           "myCachedPalette");
```

### "My new TTM opcode needs a 256 B temp during opcode dispatch"

Lifetime: a few instructions. → use a **stack buffer**, not the
allocator. Allocator is for things larger than one stack frame or
that outlive one function.

## Holiday variants

Variant-specific allocations follow the same lifetime rules as base-
scene allocations. Three cases (M15):

- **Variant-specific resource blob** (e.g., a holiday-only BMP/TTM/SCR/ADS):
  → **CACHE** with `pinResource`/`unpinResource` while the variant is
  active. Evicts when not in use.
- **Variant-specific per-scene scratch** (e.g., a holiday's per-frame
  decode buffer): → **TRANSIENT**. Same as base-scene scratch; wiped at
  `fgRuntimeReset`.
- **Variant-specific long-lived state** (rare, design-smell): → **BOOT**
  *only* if it genuinely lives the program's whole runtime, not just
  while the variant is active. Usually means the resource should have
  been CACHE'd instead — challenge the design before writing the alloc.

If the variant changes *peak memory demands* (more concurrent threads,
bigger BMPs, larger clean-rect), regenerate `pack_header_metrics.h`
after adding the variant. See
[adding-new-scenes-memory.md](./adding-new-scenes-memory.md).

## Prohibitions

- **No macros that expand to `memAlloc(...)`.** Each call site must be
  textually a `memAlloc(REGION, size, tag)` call so the rationale
  comment + CI gate work. Wrapping macros hide call sites from review
  and trip the Python rationale gate.
- **No *runtime* conditional region choice.** A given source-level
  allocation site must always go to the same region at runtime. If
  you find yourself writing
  `memAlloc(someCondition ? BOOT : TRANSIENT, ...)`, the design is
  wrong — split into two sites or rethink the lifetime.

  *Build-time* `#ifdef PS1_BUILD` / `#ifndef PS1_BUILD` branches that
  produce **distinct textual call sites per platform** are fine — each
  branch is reviewed independently and gets its own
  `MEM_REGION_RATIONALE` comment. The prohibition is on runtime
  branching, not platform conditionals (M17).

## Process notes (M16)

Adding a new `memAlloc` site requires:

1. Pick the region from the tree above.
2. Write a one-line `MEM_REGION_RATIONALE: ...` comment immediately
   above the call (within 3 lines), explaining the lifetime choice.
3. Annotate the call site with `INIT_ZEROED`, `INIT_FULL_WRITE`, or
   `INIT_NONE` per the plan's allocation contract.

This is ~2-3 minutes of work per site and is enforced by CI. Worth it
— the alternative is a maintenance fog where five years from now no
one remembers why your buffer is in BOOT instead of TRANSIENT.

## When none of these fit

You probably need a different design, not a new region. Likely causes:

- **The lifetime is "depends on a flag at runtime."** Pick the longer
  of the two possibilities. If it might live a whole session, BOOT.
- **The allocation is variable-size and might be huge.** Look at
  the resource cache (CACHE) — that's what it's for, including the
  ~300 KB SCR backgrounds.
- **The allocation happens in an interrupt handler.** Don't. The
  allocator is main-thread-only. Pre-allocate in BOOT, hand a
  pointer to the ISR.

## Sanity check before merging

- Did you update `pack_header_metrics.h` if your allocation changed any
  per-scene worst-case footprint? See
  [adding-new-scenes-memory.md](./adding-new-scenes-memory.md).
- Did you annotate the call site `INIT_ZEROED` / `INIT_FULL_WRITE` /
  `INIT_NONE`? CI grep gate requires it.
- Does the call site pass a useful tag string for JCMEM telemetry?
- Did you cross-check against the budget table in the plan? If you
  added a new BOOT allocation > a few KB, the BOOT budget may need
  to be raised — coordinate with whoever owns the budget math.
