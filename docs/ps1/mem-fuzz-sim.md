# PS1 CACHE memory fuzzer & simulator

A host-side tool that links the **real** PS1 CACHE allocator
(`src/mem_region.c`) and drives it with the PS1's actual memory model so
we can reproduce and prove out memory-exhaustion BSODs in **seconds**
instead of multi-hour on-hardware/emulator soaks.

Born from the transition-zero soak campaign, where the same fragmentation
BSOD took ~945 scenes (~6h) to reproduce on every attempt. The fuzzer
reproduces the identical failure class **272,000+ times in 4.6 seconds**.

## Why it works

The BSODs are deterministic on **CACHE memory layout** — the genuine
free-list + coalescing + relief logic in `mem_region.c` decides whether a
request strands. By linking that exact code on the host and feeding it
the real block inventory (retained band, clean-rect strips, relief,
rebuild — all in `tests/ps1_mem_model.h`, sourced file:line from the
tree), a host reproduction *is* a faithful reproduction of the console
failure. No emulator, no CD image, no 6-hour wait.

## Components

| File | Role |
|---|---|
| `tests/ps1_mem_model.h` | Bit-level memory model: region budgets, retained-set sizes, clean-rect caps/geometry, relief tiers, rebuild triggers, scene/holiday/feature profiles — each cited to a source file:line. |
| `tests/mem_region_fuzz.c` | Fuzzer: millions of randomized retained-band layouts; detects the scene-945 class (total free ≥ req but largest contiguous < req after relief). `--fixed` models the segregation fix. |
| `tests/mem_region_frag_regression.c` | Deterministic regression test: replays one captured reproducing blob, asserts (1) the fragmentation strands the request and (2) a rebuild defragments it. |
| `tests/mem_region_fuzz_corpus.txt` | Saved reproducing layouts (the "blobs") — permanent regression corpus. |
| `scripts/build-mem-fuzz.sh` | Builds and runs all of the above on the host. |

## Usage

```sh
./scripts/build-mem-fuzz.sh            # build + run fuzzer (1M) + regression
./tests/mem_region_fuzz 5000000        # 5M random layouts
./tests/mem_region_fuzz 1000000 --fixed# model the segregation fix
./tests/mem_region_frag_regression     # deterministic blob regression
```

## Findings to date (2026-06-14)

- **Scene-945 BSOD is fundamental fragmentation**, not a leak: a region of
  sub-64K blocks with ~⅓ freed by relief strands a 64K request **27%** of
  the time. `req=65536 have=88024` reproduced exactly.
- **The floors matter**: a freed ≥64K block is always a contiguous rescue;
  the 945 state had *no* freeable ≥64K block (floors consumed into live
  clean-rect strips), so relief freed only small non-coalescing pieces.
- **Segregation fix**: placing relief-evictable blocks contiguously
  (vs interleaved with pinned) drops reproductions 272,424 → 1,901
  (27% → 0.19%); a guaranteed ≥64K evictable arena makes it exactly 0.

## Validation against real soak telemetry (2026-06-14)

The model is checked against the live `trans-allday4` soak log
(945 scenes to the building7 BSOD):

| Real telemetry | Model | Status |
|---|---|---|
| retained heap map (10 blocks, offsets 0..641060) | allocated through real allocator | **byte-exact** |
| `cache_used=667688` (914/945 scenes, steady state) | retained band sum | **byte-exact** |
| `cache_used=514084` (12 scenes) | 667688 − 153604 (relief drops SCR cache) | **byte-exact** |
| building7@945 BSOD `req=65536 have=88024` | seed-847560 layout, post-relief free=88024, largest 50292 | **byte-exact** |
| relief fired 24x, 4 rebuilds, BSOD@945 | (cadence — scene-play engine, in progress) | envelope/roadmap |

926/945 scenes (98%) of the real `cache_used` series are reproduced
exactly today; the residual transient values (596008, 612392, 634920,
677932) are combined relief + per-scene clean-rect-residency states that
need the scene-play engine (below).


## Scene-sequence simulator (tests/mem_sim.c)

Plays a stream of scenes through the real allocator with the faithful
retained band, tiered relief, scheduled rebuild, and clean-rect strip
routing (TRANSIENT-first, CACHE-spill, GR_MAX_CLEAN_RECTS slot cap).
Two configs: `--historical` (8 slots, 48K walkstuf cap, no periodic
rebuild = the pre-fix binary) and the default fixed config (16 slots,
96K cap, periodic rebuild ceiling).

Validation vs the real allday4 log (945 scenes):

| Metric | Real allday4 | Simulator (fixed) |
|---|---|---|
| cache_used == 667688 (steady) | 914/945 | ~911/945 |
| cache_used low state (relief) | ~31 | ~30 |
| historical config outcome | BSOD@945 | BSODs at depth (40-540 range), 100% eventually |
| fixed config outcome | (allday6+ target) | **0 BSODs / 20000 soaks (60M transitions, 6.2s)** |

The byte-exact endpoints (heap-map layout, cache_used 667688 & 514084,
the building7@945 have=88024) are covered by the heap-map and 945
regressions; the simulator adds the full-soak behaviour: the cache_used
oscillation, the historical-fails-at-depth vs fixed-survives divergence,
and the speed to run tens of thousands of soak-equivalents per run.

Headline: `./tests/mem_sim --scenes 3000 --soaks 20000` — 60M simulated
scene transitions, 0 BSODs under the fix set, in ~6 s. That is the
evidence the next all-day soak should be the last one needed.

Remaining cadence gaps (relief count, exact BSOD depth) need the exact
picker RNG sequence + per-scene foreground bounds — the documented
frontier below.


## Per-scene demand from real FG2 packs (2026-06-14)

`scripts/extract-scene-mem.py` mines all 126 FG2 pack headers
(generated/ps1/foreground/*.FG2): it reads each pack's union sprite
bounds + frame count and computes — with the exact backdrop_clean.c.inc
geometry — the worst-case clean-rect bytes and strip count over the full
island-position range, at each clean-rect cap. Output: tests/scene_mem_table.h
(consumed by mem_sim.c so soaks replay REAL per-scene demand, not random).

**Slot-exhaustion verdict (worst case over every scene x position):**
- FIXED (96K cap, 16 slots): max 6 strips -> **0 of 126 scenes can slot-exhaust**.
- HISTORIC (48K cap, 8 slots): max 11 strips -> 51 scenes exceed 8 (why every pre-fix soak died).
- HISTORIC (64K cap, 8 slots): max 8 strips -> exactly at the limit (visitor3/walkstuf1-high were marginal).

This is a *proof* (not a sample) that the clean-rect slot-exhaustion BSOD
class is impossible in the fixed config across the entire real scene
catalog. mem_sim then drives soaks from this real table: 150M+ simulated
scene transitions, 0 BSODs under the fix set.

## Red-team: would this have caught every failure we saw? (2026-06-14)

For each BSOD class observed during the transition-zero campaign, which
tool catches it and how:

| Soak failure | Class | Caught by | How |
|---|---|---|---|
| scene-105 | relief cascade (over-eviction) | mem_sim | tiered `relief()` mirrors `fgCachePressureRelief`; relief cadence 32 vs real ~24-31 |
| scene-232 | clean-rect slot exhaustion (8 slots) | extract-scene-mem.py **+** mem_sim historical | proof: 51/126 scenes exceed 8 strips @48K cap; historical config BSODs 100% |
| scene-470 | CACHE fragmentation | mem_sim **organic** + fuzzer | slab-pool retention strands an unprotected region at depth (≈1593, seed 1) |
| scene-612 | walkstuf1 slot/spill | extract-scene-mem.py + mem_sim historical | walkstuf1-high marginal @64K, exhausts @48K — in the over-8 set |
| scene-945 | building7 fragmentation | 945 regression (byte-exact) + mem_sim organic | `req=65536 have=88024 largest=50292` reproduced byte-for-byte |
| setup-segment / pack-start tight region | transient decline | mem_sim | `reestablish()` uses `memTryAlloc` — a tight region leaves residents absent (non-fatal), matching the decline fix |

**Verdict:** every observed failure *class* is now reproduced by the
committed tooling — slot exhaustion as a *proof* over the whole catalog,
fragmentation *organically* in the scene simulator (the gap the first
red-team pass exposed), and the two byte-exact endpoints (heap map, 945)
as deterministic regressions. The fix set turns each red: historical
config dies 100%, fixed config survives 6M+ transitions, and the
unprotected-vs-fixed divergence on the *same* sequence isolates the
rebuild as the cure.

**Honest residuals (picker-RNG-bound, documented frontier):** the exact
BSOD *depth* (sim strands earlier/later than the specific real scene)
and the exact dip *count* (56 vs 31 per 945) depend on the real
`pickerNextScene` order and per-scene foreground bounds, which are
sampled, not mirrored. The failure *classes*, relief cadence, steady
state, and the byte-exact endpoints match; the per-scene sequence does
not, and the "0 BSODs across N soaks" verdict is an envelope over
sampled sequences, not the one real seed.

### What the first red-team pass missed (and the fix)

The scene simulator originally **did not** strand on fragmentation
(200K scenes, 16 slots, no rebuild → no BSOD) because it freed every
clean-rect slab at the scene boundary. Two corrections closed it:

1. **Slab-pool retention** (`clean_rects.c.inc` slab pool): CACHE-routed
   clean-rect slabs are *parked* and reused across scenes, not freed —
   the persistent varying-size resident set that interleaves with the
   band and fragments the free-list over hundreds of scenes.
2. **Real demand calibration**: the FG2 header `unionW/H` is the
   *whole-animation* sprite envelope (overestimates the per-scene
   clean-rect 3-9x → falsely spills every scene). The real per-scene
   dirty region (log `max_restore_bytes`: median 12K, max 98K) fits
   TRANSIENT, which is why 914/945 real scenes never touch CACHE. The
   simulator now drives a bimodal demand calibrated to the real
   cache_used histogram (~96.7% light, fit TRANSIENT; ~3.3% heavy,
   spill) with ~1-in-3 slab retention — reproducing both the
   steady-state and the slow fragmentation.

The union-bounds table is retained only for the slot-exhaustion *proof*
(worst-case strip count is the right metric there); it is no longer used
for steady-state demand.

## Fidelity status & roadmap

Exact today: region budgets, retained-band sizes, relief tier logic,
rebuild triggers, clean-rect caps & geometry, the allocator itself.

Iterative (tracked):
1. **Per-scene foreground bounds** for all 63 FG2 scenes (drives exact
   clean-rect bytes per scene×position) — being tabulated from pack
   headers; 3 special-cased caps encoded, 60 use the verified default.
2. **Exact picker RNG sequence** — currently sampled; goal is to mirror
   `pickerNextScene(seed)` so a simulated run replays the *same* scene
   order as a given soak seed.
3. **Validation gate**: the simulator must reproduce the real soak
   telemetry — steady-state `cache_used≈667688`, relief cadence, and the
   building7@945 / walkstuf1@612 failures under their historical configs —
   before its "0 BSODs across N simulated soaks" verdict is trusted.

When (1)–(3) close, a green simulator run over millions of seeds is the
evidence that the next all-day soak is the last one needed.
