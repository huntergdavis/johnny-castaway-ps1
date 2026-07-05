# Real-Hardware Testing Lessons — PS1

Hard-won rules from the 0.9.13/0.9.14 release marathon (July 2026):
twenty burned CD-Rs, one worn launch-era drive, and a week of chasing
graphical bands, dead audio, buzzing, freezes, and red screens that no
emulator ever reproduced. Every rule below was paid for with a coaster.

**Audience:** future agents and humans writing PS1 (or any retro
console) code that must survive real silicon. If you only read one
section, read [The Prime Directives](#the-prime-directives).

---

## The Prime Directives

1. **Emulators validate logic, never hardware.** DuckStation completes
   transfers synchronously, reports status honestly, and delivers
   interrupts on schedule. Real silicon does none of these reliably.
   A change that is pixel-perfect in the emulator has told you nothing
   about the console. Plan for console verification of anything that
   touches GPU transfers, SPU transfers, CD reads, or DMA.

2. **Status bits lie; DMA channel-busy is CPU-side truth.** Three
   independent "transfer complete" signals assert EARLY on real
   silicon: `SPUSTAT` bit 10, `DrawSync(0)`, and `CdReadSync`. The one
   trustworthy completion signal is the DMA channel's CHCR busy bit
   (bit 24), read from the CPU side:

   | Channel | Purpose | MADR | BCR | **CHCR** |
   |---------|---------|------|-----|----------|
   | DMA2 | GPU | 0x1F8010A0 | 0xA4 | **0x1F8010A8** |
   | DMA3 | CD-ROM | 0x1F8010B0 | 0xB4 | **0x1F8010B8** |
   | DMA4 | SPU | 0x1F8010C0 | 0xC4 | **0x1F8010C8** |

   **Verify the constant against this table the day you write it.**
   We shipped `0xB0` (MADR) where `0xB8` (CHCR) was meant. Bit 24 of a
   RAM address is always zero, so the "drain" returned instantly for
   thirteen consecutive builds while we chased its symptoms under
   other names. One byte. A week.

3. **A diagnostic counter that is always zero is a smell, not a
   comfort.** Our R counter (DMA-drain expiries) read zero forever —
   because it watched the wrong register. If a counter never fires
   across conditions where you'd expect occasional hits, suspect the
   counter before trusting the subsystem.

4. **Every wait must be bounded.** An unbounded `DrawSync(0)` or
   `CdReadSync(0)` on wedged hardware is a freeze with the music still
   playing (SPU voices are autonomous — audio surviving a freeze means
   the CPU is spinning, usually in exactly such a wait). Bound every
   wait, and size bounds in *frames*, not seconds: a 6-second timeout
   that re-fires every frame is a slideshow the user reports as
   "frozen, shows the old menu, then the scene, not moving."

5. **One change per burn.** CD-Rs are finite and console time is the
   scarcest resource in the loop. When two changes ship together and a
   symptom appears, you cannot attribute it. Our fastest progress came
   from single-purpose builds; our worst regressions from sweeps.

---

## GPU: the queue, the FIFO, and direct writes

- **`DrawSync(0)` early-returns on real silicon.** Proven three
  independent times. Never rely on it alone before touching GP0.
- **The SDK draw queue is invisible to hardware polls.** A
  queued-not-yet-started `LoadImage` shows idle on CHCR and satisfies
  DrawSync — then auto-starts from the DMA IRQ in the middle of your
  direct GP0 writes. Drain the queue itself with `DrawSync(1)`
  (queue-length query) before any direct-GP0 sequence.
- **Full quiesce ordering:** queue empty (`DrawSync(1)==0`) → DMA2
  CHCR bit 24 clear → GPUSTAT ready bits **sequentially** (bit 26
  idle first, then bit 28 DMA-ready). Testing `(1<<26)|(1<<28)` as an
  OR passes mid-transfer, when bit 28 (wants data) is up while bit 26
  is down.
- **Writing the last word is not completion.** After a CPU-driven A0h
  upload, the FIFO still holds the tail. Returning and letting the
  frame loop fire `DrawOTag` wedges the GPU (DMA2 into a GPU still in
  receive mode). Close out every upload: bounded poll for GPUSTAT
  bit 26 before returning.
- **Un-wedge with `ResetGraph(1)`, never raw GP1 01h.** The raw
  command-buffer reset leaves the SDK's queue counter stuck non-zero
  forever (its completion IRQ never fires), and every subsequent
  bounded drain runs to timeout — the crawl that looks like a freeze.
  `ResetGraph(1)` cancels the queue AND resets the FIFO with
  consistent bookkeeping.
- **GPU FILL granularity:** FILL operates on 16-pixel-aligned columns;
  sub-16px fills silently no-op. Use exact pixel uploads for smaller
  erases.
- **Single-framebuffer designs inherit debris.** Staged transitions
  that avoid full-screen repaints will carry any seeded garbage across
  scenes indefinitely. Anything that paints outside the per-scene
  repaint set (boot indicators, menus, beacons) must clean up with the
  same queue-drain discipline it paints with.

## CD-ROM: the drive is an unreliable narrator

- **Every `CdRead` ends with an async `CdlPause` whose completion IRQ
  resets the parameter FIFO.** Any `CdlSetloc` issued in that window
  has its target silently discarded — the next read comes from the
  wrong place with **no error and no failed status**. Guard EVERY
  Setloc with a bounded command-idle wait (`CdSync(1)` until
  `CdlComplete`/`CdlDiskError`). The race window scales with drive
  fatigue: symptoms that worsen over a session are its signature.
- **`CdlReadN` continuous reads: one data-ready event is NOT one
  sector.** Under IRQ latency events coalesce, and a worn drive's
  silent retry re-seeks and re-delivers earlier sectors. Both slip the
  data stream against your sector count with zero error indication.
  The ONLY defense is per-sector identity: read in RAW mode
  (`CdlModeSize`, 2340-byte sectors), where each sector carries its
  own BCD MSF address at bytes 0-2 (data payload at offset 12), and
  verify every header against the expected LBA before using the
  payload. `CdlGetlocL` position audits after the fact are NOT
  reliable (stale data after a pause) — we shipped one that vetoed
  every good read.
- **Sequential streaming is drive-gentle; seeks and stop/start cycles
  are what kill worn sleds.** Inventory your workloads: our previews
  were 15 seek+stop/start cycles each (pathological); scene streaming
  read 512 KB per command (already fine). Optimize the pathological
  path, leave the near-optimal ones alone.
- **Adapt to measured behavior, not platform detection.** Time the
  first real load: fast (emulator, healthy drive) → snappy UX; slow →
  long debounces so rapid UI navigation touches the drive zero times.
- **Boot is the coldest, least reliable read window.** A just-
  chainloaded drive misses completion IRQs. Every boot-path read needs
  bounded sync + patient retries + a controller-only reset between
  attempts (a full state reset may free buffers still in use — check
  what your reset function tears down before calling it mid-read).
- **Non-critical reads must not feed recovery machinery.** Browse-time
  thumbnail failures fed our scene-abort streak and beacon: the menu
  painted distress bars and the next scene got force-reset. Wrap
  cosmetic reads in a mode that skips abort accounting while keeping
  drive-health tracking.
- **Policy: I/O failure never halts.** A blue screen over a retryable
  read throws away a recoverable session. Degrade: retry bounded →
  skip/black-fill → scene change with memory wipe. Reserve halts for
  data-structure corruption (genuine bugs).

## SPU and interrupt-side memory safety

- **SPU transfers share the status-asserts-early race** — use a
  two-phase wait (see `ps1SpuTransferWaitBounded`) and verify uploads
  by readback where the content matters; drop what never verifies
  rather than shipping corrupt samples.
- **IRQ-written result buffers must be `static`, never stack.**
  `CdControl(cmd, param, result)` writes `result` from the completion
  IRQ, which can land after your bounded wait gave up — into a dead
  stack frame. Eight bytes of delayed corruption into whoever owns
  that memory next. This class presents as "impossible" distant
  symptoms.
- **Never reprogram an active DMA channel.** Issuing the next read
  while DMA3 is still writing sends sectors to wild RAM addresses.
  This one failure produced: dead ambience, a horrifying buzz (the
  self-heal re-keyed a voice over corrupted sound tables), wandering
  graphical bands (corrupted clean-rect strips), and intermittent
  red-screen boots. **Memory corruption presents far from its cause;
  when symptoms look unrelated and impossible, hunt for a wild
  writer.**

## Diagnosing on a console with no debugger

- **Put counters on screen, and show explicit zeros.** "No diag line"
  is weak evidence; `P0 Q0 R0 W0 c-1 D0 S1` is strong evidence. Our
  breakthrough decodes all came from the user reading 7 digits off a
  CRT. Mind the minus sign — users drop it ("c1" was `c-1` twice).
- **Design counters to partition the failure space.** Ours split:
  GPU pace ceiling (P), quiesce/queue expiry (Q), DMA drain expiry
  (R), GPU un-wedges (W), CD failures (D), menu lifecycle (S). Every
  console report then *excludes* mechanisms — that exclusion power is
  the whole value.
- **A RAM-sourced test pattern through the same output path splits the
  world.** Our □ test card (pattern clean = data path corrupts;
  pattern broken = output path corrupts) settled in one button press
  what three builds of theorizing couldn't.
- **User observations are precision instruments.** "The band moved to
  the top after tree scenes" identified the clean-rect system. "Audio
  still playing" proved the CPU alive. "Second visit breaks" (S2)
  fingered the cache-hit path. "It's a repeat of the band from halfway
  up" described stream slippage exactly. Ask for the diag line, the
  position, the content, and the sequence — and treat photos as gold.
- **Visible distress beats silent grinding.** The red beacon (direct
  GP0 bar at top-left, growing with CD failure streak) tells the user
  "retrying, wait" vs "dead, reset" — which also protects the
  hardware from mid-recovery power cycles.

## Process lessons

- **When you cache a function, inventory its side effects first.** Our
  locate cache skipped an async-read drain that "resolve then read"
  callers silently depended on. Three "new" bugs, one missing side
  effect.
- **When a fix doesn't change the symptom, question whether the fix
  ever executed.** We attributed the surviving strip to five different
  mechanisms before discovering the original fix was a no-op (wrong
  register). "Fix landed but symptom persists" should trigger "prove
  the fix runs," not "invent the next theory."
- **Keep a per-symptom timeline across builds.** bt16-clean/bt17-dirty
  isolated a one-commit cause twice. The user's memory of "which build
  did what" was the most valuable dataset of the week.
- **EXE static-image budget is a real ceiling** — diagnostics-heavy
  test builds ride ~10 KB above release. Budget for it; keep a list of
  trimmable fat (debug prim buffers) for when a feature needs bytes.
- **Watchdogs and self-heals mask root causes — instrument them.**
  Count every recovery (our W counter). A self-heal that fires often
  is a bug report, not a success story.

## The symptom → mechanism cheat sheet

| Console symptom | First suspect |
|---|---|
| Frozen picture, audio still playing | Unbounded wait on wedged GPU (DrawSync class) |
| Horizontal band, content shifted/rotated | Words injected into an active GP0 upload (queue/FIFO race) |
| Band whose content repeats from N rows up | Data stream slipped vs sector count (CD re-delivery/coalesce) |
| Band at a consistent position, worsens over session | Setloc discarded by pause-IRQ FIFO reset |
| Debris band that follows each scene's action area | Corrupted saved-restore buffers (clean rects) → hunt a wild writer |
| Debris that persists across scenes, healed by menu | Seeded outside the partial-repaint set (staged transitions) |
| Audio dies, later returns as loud buzzing | Corrupted sound tables/SPU state → wild DMA writer |
| Works second boot, not first | Cold-drive missed IRQ + unbounded boot wait |
| Same failure at a moving boot-progress position | Media/drive region, not code (position = progress map) |
| "Impossible" combination, all counters clean | The counter watches the wrong thing, or memory corruption |

---

*Companion docs: `docs/ps1/TESTING.md` (harness),
`site/_posts/2026-07-05-what-twenty-coasters-taught-us.md` (narrative
devblog). Agent memory: `explorer-hardening-final`,
`spu-transfer-wait-hardware`.*
