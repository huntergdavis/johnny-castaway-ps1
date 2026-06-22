# Johnny Castaway PS1 — v0.9.6

A deep-soak stability release. Where 0.9.5 fixed the first wave of CACHE
exhaustion BSODs, 0.9.6 closes the remaining deep-soak failure classes —
validated by a **~15-hour continuous soak (7,800+ scenes) with zero crashes and
zero hangs**, during which the recovery machinery fired ~65 times (50 memory
withhold-rebuilds, 15 graceful clean-rect declines) and recovered every time.

## Fixes

### Memory: the whole best-effort CACHE-alloc class is now graceful
Several CACHE allocations NULL-checked their result ("failure tolerated") but
used the *halting* `memAlloc`, so the guard was dead code and a deep-soak
fragmentation strand BSOD'd instead of degrading. All such allocations now use
no-halt / recover-via-withhold-rebuild:
- per-scene **stream-window** grow (the `fg-stream-window req=98304` BSOD)
- clean-rect **floor-slab** top-up + boot/rebuild pre-park
- scene-setup **alignment scratch**
- the reserve-stable-shape window/frame/prefetch buffers

A strand now triggers a mandatory-only withhold-rebuild (≈249 KB contiguous) and
retries; a true ceiling scene declines its clean rect and renders crash-free.

### CD-ROM: recover instead of freezing
A multi-hour run could wedge on the CD drive (a likely DuckStation deep-run
emulation artifact). Two recoveries added:
- **explicit drive error** (status=0x03 latched → every command times out):
  after a few consecutive failures, re-init the controller to clear it.
- **silent read stall** (a read stuck PENDING forever, no error): the async-read
  poll now self-heals after ~15 s — `CdInit` reset + retry — instead of spinning
  in an untimed loop.

### Diagnostics
A CACHE-exhaustion BSOD now names the failing allocation (`JCMEM CACHE-FAIL
tag=…`), and the memory simulator + fuzz gate model the per-scene frame buffer
(previously a blind spot).

## Notes
Production-like build (no special boot flags). The CD failure modes did not
recur in the validation soak (intermittent emulator behaviour); the recoveries
are in place if they return.
