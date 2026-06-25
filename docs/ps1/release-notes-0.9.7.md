# Johnny Castaway PS1 — v0.9.7

A visual-confidence + deep-soak release. 0.9.6 closed the memory-exhaustion BSOD
classes; 0.9.7 closes the last walk-slab crash class, makes the most demanding
scene (visitor3) render correctly even under extreme deep-soak memory pressure,
and fixes the remaining scene-set visual nits — validated by a **24-hour
continuous soak (12,512 scenes, all 126 variants) with zero crashes and zero
hangs**, during which the visitor3 full-reset fired **26 times and recovered to
a pixel-perfect render every time** (zero degraded declines).

## Fixes

### Memory: the inter-scene walk/raft load no longer halts
The 48 KB JOHNWALK / MRAFT load (slab reserve, slab re-alloc, per-load alloc,
and the `.BMP` fallbacks) used the *halting* `memAlloc`, so a deep-soak
fragmentation strand BSOD'd (`JCMEM CACHE-FAIL tag=johnwalk_spu_load`, observed
~28 h into a soak) instead of degrading. All five chokepoints are now no-halt;
a fragmented CACHE makes the walk gracefully skip (a teleport) and the raft draw
skip — never a crash.

### visitor3: pixel-perfect even under extreme deep-soak pressure
visitor3 (the game's most memory-demanding scene, ~400 KB clean rect) could
strand deep in a soak even after the withhold-rebuild, previously rendering a
degraded frame (ocean with the island and most of the ship missing). New
deepest recovery rung: a **full CACHE reset masked by the frog-clock loading
animation** — the region is safely defragmented to pristine (via the allocator's
*conditional* rewind, so a stuck resident degrades gracefully rather than
corrupting) and the scene reloads and renders the **normal pixel-perfect path**.
Because it fires reactively under real pressure, it doubles as a periodic
full defragmentation for the whole soak.

If even a pristine heap can't fit the scene (a TRANSIENT+CACHE capacity ceiling),
it now declines **crash-free *and* correct-looking**: the fallback re-composites
the island, raft, and holiday backdrop every frame instead of showing ocean
only.

### Visuals
- **stand-family opening flash** — STAND scenes briefly showed the empty island
  (Johnny absent) on the first frame; the packs now open on the frame Johnny is
  already present.
- **visitor3 trails** — the deep-soak decline path no longer smears the boat and
  island sprites every frame.

## Notes
Production-like build (no special boot flags). All recovery machinery
(withhold-rebuild, full-cache-reset, graceful decline) fired and recovered
during the validation soak; the full-cache-reset's pristine rewind eliminated
the heap-corruption class entirely.
