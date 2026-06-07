# PS1 Async Loading Failed Experiments

Branch: `ps1-async-wave-loading`

Date started: 2026-06-04

This file records approaches that looked promising but should not be revived
without a new reason. The current retained branch direction is faster
same-key ocean setup reuse, not ticking water through blocking scene setup.

## Grouped Loading-Wave Save-Under Tick

Goal: keep the island shoreline visibly animating while foreground setup reads
were pending.

Shape:

- Added a `loading-waves` boot-token path that installed a CD read idle hook.
- Used caller-owned save-under memory for the active high-tide or low-tide wave
  group.
- Restored the previous wave rectangles, drew all active wave sprites for the
  phase together, and uploaded during `CdReadSync(1, NULL)` polling.

Evidence:

- `fishing1` high tide and low tide could tick waves during setup without
  frame mismatches in short runs.
- `walkstuf1` and `visitor3` exposed allocator pressure around runtime clean
  snapshots.

Failure:

- It did not solve the actual wall-clock problem. The loading pause was still
  visible, and in some transitions the water only caught up after loading.
- The approach consumed memory at exactly the wrong time: scene-boundary setup
  buffers, clean snapshots, and wave save-under buffers all competed for the
  same limited PS1 regions.
- It increased transition complexity without eliminating the blocking read
  window.

Decision:

- Reverted. Do not keep wave tick code in `island.*` or framebuffer helpers for
  this path.
- Preserve the allocator lesson: clean snapshots must not depend on hidden
  reusable setup residency staying live.

## Direct Early-Overlay Framebuffer Wave Tick

Goal: cover the earlier frozen read window by preparing the wave state at the
end of one scene and ticking it directly in the framebuffer while the next
scene started loading.

Shape:

- Captured the clean wave group at end-of-scene cleanup.
- Copied active tide `BACKGRND.PSB` wave sprite pixels into a compact shadow.
- Began a direct framebuffer CD-idle hook before `fgRuntimeReset`,
  `BACKGRND.PSB` preload, and `grLoadScreen`.

Evidence:

- Visible-loop `fishing1 lowtide` produced markers showing the hook could run
  during early setup reads, including `loading-waves-direct-begin`,
  `loading-waves-switch`, and `loading-waves-end`.
- This proved the PS1 can do useful non-CD work while `CdReadSync(1, NULL)` is
  polling.

Failure:

- It regressed the user-visible transition. The wall-clock pause still felt
  roughly four seconds, and Johnny could disappear before the next foreground
  became ready.
- Wave phases could visually catch up too fast after loading, which made the
  transition look less correct than the baseline.
- The proof required extra clean, shadow, and scratch buffers across the scene
  boundary. That blocked normal CACHE rewinds and pushed the branch into memory
  failures, including primitive-buffer boot failures and later CACHE clean-rect
  blue screens.
- Executable size and runtime bookkeeping both grew for a feature that did not
  reduce the actual transition cost.

Decision:

- Reverted. The direct framebuffer wave tick, shadow copy, save-under API, and
  special memory rebalance are not part of the retained branch.
- The only lesson kept is architectural: asynchronous or overlapped reads are
  useful if they remove scene setup from the transition, not if they merely try
  to animate over a still-blocking transition.

## Side-Buffer Next-Stage Proof

Goal: preload the next scene pack into a separate 128KB side buffer before the
transition, then adopt it during setup.

Evidence:

- Repeated `fishing1 lowtide` setup dropped from about 262 VBlanks to about
  73-75 VBlanks when paired with reusable clean screen/backdrop state.

Failure:

- Not soakable. A visible run blue-screened after about 522 emulator seconds:

```text
JCBSOD-FATAL CACHE exhausted (region+libc both): req=131072
```

Decision:

- Reverted as a memory shape. Keep the performance target, but use the existing
  `fg-stream-window` in place instead of allocating an additional side buffer.

## Retained Direction

The useful path is the in-place `fg-stream-window` stage plus reusable clean
screen/backdrop state:

- Common repeated `fishing1 lowtide` path measured around `setup_vb=86-87`,
  roughly 1.4 seconds at 60Hz.
- No direct water ticking is required if the scene transition itself becomes
  short enough.
- Future work should aim to start lookahead earlier, tighten first-frame actor
  readiness, and eventually generalize the ring-buffer or stream-window model
  beyond same-key ocean scenes.
