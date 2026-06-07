# PS1 Async Wave Loading Branch Plan

Branch: `ps1-async-wave-loading`

Date: 2026-06-04

## Scope

This branch is an isolated experiment. It must not change release-line behavior
unless explicitly enabled by a boot token or build flag.

The branch started as a proof that the PS1 could keep shoreline waves visibly
animating while the next foreground scene loaded. That direct approach has been
reverted because it did not reduce the actual wall-clock transition and created
allocator risk. Failed wave-tick attempts are recorded in
`docs/ps1/async-wave-loading-failed-experiments.md`.

The retained goal is now narrower and more useful: make same-key ocean scene
transitions short enough that the water does not need a special loading-time
animation path.

## Historical Phase 1: Wave Loading Proof

This was the original proof target on normal island/ocean foreground scenes
where the runtime wave system already exists. It has been reverted; the retained
branch work is the preload/reuse path below.

Original target behavior:

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

Original implementation boundaries:

- Add a small, opt-in loading-wave tick that only touches RAM/VRAM:
  restore/tick wave sprite, upload wave-region rows, wait one VBlank.
- Do not allocate in the loading tick.
- Do not call CD APIs in the loading tick.
- Do not keep the previous scene alive while loading the next scene in Phase 1.
  Current scene-boundary cleanup is load-bearing for long-run stability.

Original acceptance checks:

- Headless perf/regtest boots complete with feature disabled and enabled.
- Wave-enabled loading runs do not produce `JCBSOD`, CD failures, clean-rect
  failures, or frame mismatch counters.
- Captures show wave phase changes during setup/loading, not just after the
  first foreground frame.
- First displayed foreground frame is clean: no stale wave strip, no missing
  island/raft/holiday pixels, no foreground residue.

Current retained proof hook:

- Boot token: `loading-waves` (aliases: `load-waves`, `async-load-waves`).
  The token name is historical; it now enables the same-key preload/reuse proof,
  not direct wave ticking during reads.
- Default behavior is unchanged; `loading-waves-off`/`no-loading-waves` can
  explicitly clear the flag in diagnostic boot strings.
- The retained proof borrows the existing `fg-stream-window` after the current
  scene no longer needs it, then lets the next scene adopt that window in place.
  Matching same-key ocean scenes can reuse `BACKGRND.PSB` and a clean screen
  baseline instead of fully reloading the static island backdrop.
- Removed wave-tick experiments include the grouped save-under hook and direct
  framebuffer shadow hook. Their evidence and failure reasons live in the
  failed-experiments file.
- The walkstuf1 crash also exposed a non-water allocator issue: reusable
  setup residency can still be live when the runtime clean snapshot is saved.
  If the clean snapshot is larger than remaining TRANSIENT space, the branch
  now drops CACHE-backed setup residency before `grSaveCleanBgRects`, so a
  hidden read cache cannot starve the correctness-critical clean baseline.

## 2026-06-06 Optimization Pivot

The branch now has a second, faster proof path behind the same `loading-waves`
boot token. Instead of trying to tick waves through every blocking setup read,
it keeps validated ocean setup state alive across matching island scenes:

- The current safer proof borrows the existing 128KB `fg-stream-window` after
  the current scene has no more payload reads. The next scene adopts that
  window in place after `fgRuntimeReset`, avoiding the extra side-buffer
  allocation and copy.
- If the next scene has the same reusable ocean key
  (`scene/island/tide/night/raft/holiday/draw-offset`), the branch keeps the
  existing `BACKGRND.PSB` slot and the clean screen snapshot. The next scene can
  restore the saved clean rect pixels instead of reloading `OCEAN00.SCR` and
  rebuilding the static island backdrop.
- This is intentionally narrow. It is a same-key ocean-scene proof, not a
  general scene-streaming architecture yet.

Measured visible-loop `fishing1 lowtide` results:

- Baseline repeated scene setup: `setup_vb=262-263`, `screen_vb=84`,
  `backdrop_vb=83`, `setup_reads=13`, `setup_bytes=396792`,
  `setup_read_vb=154-155`.
- Side-stage plus retained `BACKGRND.PSB`: `setup_vb=191`, `screen_vb=84`,
  `backdrop_vb=36`, `setup_reads=11`, `setup_bytes=172544`,
  `setup_read_vb=95`.
- Side-buffer stage plus reusable clean screen/backdrop: `setup_vb=73-75`,
  `screen_vb=8-9`, `backdrop_vb=0`, `clean_rect_vb=0`,
  `setup_reads=2`, `setup_bytes=8228`, `setup_read_vb=19-21`.
- In-place `fg-stream-window` stage plus reusable clean screen/backdrop:
  common repeated path `setup_vb=86-87`, `screen_vb=8-10`,
  `backdrop_vb=0-1`, `clean_rect_vb=0`, `setup_reads=3`,
  `setup_bytes=73764`, `setup_read_vb=36`.

At 60Hz, the fastest side-buffer proof dropped repeated transition setup from
roughly 4.4 seconds to roughly 1.2 seconds, but it was not soakable. It crashed
after about 522 emulator seconds with:

```text
JCBSOD-FATAL CACHE exhausted (region+libc both): req=131072
```

The safer in-place proof drops the common repeated transition to roughly 1.4
seconds, while avoiding the extra `fg-next-stage` CACHE allocation. A
720-second visible run reached the watchdog without `JCBSOD`, `cd_fail`, or
`frame_mismatch`, and continued through emulator time ~718s.

This matches the subjective "feels faster" result, but it is not release-ready
yet:

- The in-place proof still keeps about 300-345KB of CACHE live across the
  boundary and logs `JCMEM CACHE-rewind-skip`.
- The reusable clean path occasionally falls back to a rebuild
  (`setup_vb=187-220`) when the clean/background state is not reusable; this is
  still faster than the original repeated-scene baseline but needs a cleaner
  invalidation story.
- It is only valid while the ocean reuse key matches. Broader scene pairs need
  a real lookahead key and invalidation story.
- Before a week-long soak, the branch should either restore the normal
  scene-boundary CACHE rewind or prove that every retained buffer has a bounded,
  single-owner lifetime.

Longer term, this path is closer to the desired ring-buffer/lookahead design:
stream only the setup ranges needed by the next scene while earlier scene state
is still useful, avoid full next-scene residency, and reuse stable background
state only when a strict key says the pixels are identical.

## Phase 2: Single-Flight Async CD Primitive

After the retained preload/reuse proof is stable, add a narrow async CD API
that current blocking callers do not use by default.

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

Once the same-key preload/reuse path is stable, this branch can evaluate
whether non-blocking read machinery improves scene timing or memory pressure
beyond the current in-place `fg-stream-window` proof.

Potential payoffs:

- Hide part of CD latency behind render/prepare/loading visual work.
- Reduce runtime `blocking_vb` in scenes with tight CD deadlines.
- Replace some retained setup/read-ahead buffers with async streaming.
- Recover W1-high timing lost by disabling optional segment 4 without bringing
  back that TRANSIENT memory risk.
- Lower CACHE/TRANSIENT residency by keeping fewer "just in case" read windows.

This optimization work should stay separate from release-line fixes.
Performance changes should be measured and accepted only after the retained
preload path is boring and reliable.

## Non-Goals

- Full async conversion of all CD reads.
- Preserving the old scene's exact final frame while the next scene loads.
- Changing default release behavior.
- Reintroducing a loading-time wave tick while the transition still blocks.
