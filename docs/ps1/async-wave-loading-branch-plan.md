# PS1 Async Wave Loading Branch Plan

Branch: `ps1-async-wave-loading`

Date: 2026-06-04

## Scope

This branch is an isolated experiment. It must not change release-line behavior
unless explicitly enabled by a boot token or build flag.

The first goal is to prove that the PS1 can keep shoreline waves visibly
animating while the next foreground scene is loading. Performance optimization
from non-blocking reads is a later branch goal, not the first success criterion.

## Phase 1: Wave Loading Proof

Prove the technique on normal island/ocean foreground scenes where the runtime
wave system already exists.

Target behavior:

- During scene setup, once the new ocean/island backdrop is ready, the wave
  strip continues to animate while CD reads for the foreground pack are pending.
- The first scene frame still enters cleanly after loading.
- No old-scene preservation is required for the proof. Showing the next scene's
  ocean/island backdrop with animated waves is acceptable.
- No release-line behavior changes when the feature flag is off.

Good first scenes:

- `fishing1` high and low: common island backdrop, known wave region, moderate
  setup.
- `stand8` or another no-stitch STAND scene: already depends on runtime waves.
- `walkstuf1` high: useful stress case after Phase 1 is stable, but not the
  first proof because it is timing-sensitive and currently soaking a memory fix.

Implementation boundaries:

- Add a small, opt-in loading-wave tick that only touches RAM/VRAM:
  restore/tick wave sprite, upload wave-region rows, wait one VBlank.
- Do not allocate in the loading tick.
- Do not call CD APIs in the loading tick.
- Do not keep the previous scene alive while loading the next scene in Phase 1.
  Current scene-boundary cleanup is load-bearing for long-run stability.

Acceptance checks:

- Headless perf/regtest boots complete with feature disabled and enabled.
- Wave-enabled loading runs do not produce `JCBSOD`, CD failures, clean-rect
  failures, or frame mismatch counters.
- Captures show wave phase changes during setup/loading, not just after the
  first foreground frame.
- First displayed foreground frame is clean: no stale wave strip, no missing
  island/raft/holiday pixels, no foreground residue.

## Phase 2: Single-Flight Async CD Primitive

After the visual proof target is clear, add a narrow async CD API that current
blocking callers do not use by default.

Initial API shape:

- Begin one sector-aligned read into caller-owned stable memory.
- Poll with `CdReadSync(1, NULL)`.
- Finish or timeout/cancel.
- Reject a second begin while a read is in flight.

Rules:

- One CD operation in flight at a time.
- No `CdSearchFile`, `CdControl(CdlSetloc)`, or other resource load while a
  read is pending.
- Destination memory must remain valid until finish.
- Existing blocking wrappers stay intact.

First integration target:

- Foreground setup reads that happen after `grLoadScreen()` and
  `fgBackdropEnableWaveBackdrop()`, especially aligned setup-segment reads.

## Phase 3: Instrumentation

Replace the currently zeroed async perf line with real counters once Phase 2 is
wired:

- `async_start`
- `async_poll`
- `async_done`
- `async_timeout`
- `async_cancel`
- `async_wave_ticks`
- `async_blocking_vb`

Also log a concise TTY marker when the feature is active, for example:

```text
JCASYNC loading-waves scene=fishing1 read=fgSetupSegmentBuffer ticks=12
```

## Later Branch Goal: Optimization

Once waves are proven, this branch can evaluate whether the same non-blocking
read machinery improves scene timing or memory pressure.

Potential payoffs:

- Hide part of CD latency behind render/prepare/loading visual work.
- Reduce runtime `blocking_vb` in scenes with tight CD deadlines.
- Replace some retained setup/read-ahead buffers with async streaming.
- Recover W1-high timing lost by disabling optional segment 4 without bringing
  back that TRANSIENT memory risk.
- Lower CACHE/TRANSIENT residency by keeping fewer "just in case" read windows.

This optimization work should stay separate from the Phase 1 wave proof. A
successful Phase 1 can be judged purely on visible loading polish and stability;
performance changes should be measured and accepted only after the wave path is
boring and reliable.

## Non-Goals For The First Proof

- Full async conversion of all CD reads.
- Preserving the old scene's exact final frame while the next scene loads.
- Loading resources from CD inside the loading-wave tick.
- Changing default release behavior.
- Fixing W1-high timing regression as part of the first wave proof.
