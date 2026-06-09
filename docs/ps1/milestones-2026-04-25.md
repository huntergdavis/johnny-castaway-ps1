# Milestones — 2026-04-25

A single-day batch that pushed the PS1 port across several long-standing
unknowns: working TTY, an instrumentable perf module wired into a Docker
regtest harness, a real on-PS1 pause menu, a holiday-coverage design
expansion from 4 to 35, a hand-rolled SPI controller driver replacing the
broken BIOS pad path, and the start of memory-card-backed user settings.
What follows is the day's archeology in bullets, with links to the
locked-in design docs.

## 1. Working PS1 `printf()` to TTY

- PSn00bSDK + DuckStation now reliably emits `printf()` output to TTY logs.
  Earlier in the project this had been intermittent / unreliable — the same
  call would surface in some build/runtime configurations and silently drop
  in others.
- This is the single largest unblocker for the rest of today's work.
  Instrument-based perf debugging, JCPERF/JCPERF2 line mining, and
  one-shot snapshots like `JCPAUSE` are all only useful once printf is
  trustworthy.
- See `docs/ps1/current-status.md` "Known limitations" for the older
  caveat (gated probes only); that paragraph is now stale and is being
  updated alongside this milestone.

## 2. Performance baseline + Docker regtest pipeline

- New `src/platform/ps1/ps1_perf.c/h` module landed with a level-gated logging API:
  `ps1PerfSetLevel(level)` switches between OFF / SUMMARY / DETAIL /
  DEBUG. Boot-token aliases (`perf`, `perf-detail`, `perf-debug`) are
  parsed by the existing BOOTMODE.TXT path so an external operator can
  pick a level without recompiling.
- TTY emits structured `JCPERF` / `JCPERF2` lines per scene (loop_vb,
  target_vb, blocking_vb, hidden_vb, due_misses, render/held entry
  counts, CD bytes, restore/upload bytes, etc.). These lines are
  designed to be mined by an external agent — every field is a stable,
  greppable key=value pair.
- `scripts/regtest-*.sh` ride on top of the existing DuckStation-headless
  Docker image. Frames dump on a fixed interval, TTY captures to stdout,
  state hashes capture at exit. Combined, this gives a single command
  that produces both visual diff inputs and machine-readable perf
  baselines for a regression run.
- See [performance-optimization-plan.md](performance-optimization-plan.md)
  for the optimization backlog being driven by these numbers.

## 3. Pause menu — first real user-facing UI (SHIPPED)

- Pressing **Start** now opens an overlay during scene playback. Items:
  Resume / Sound / Day-Night override (AUTO/DAY/NIGHT) / Holiday override
  (AUTO/NONE/HALLOWEEN/ST PATRICK/CHRISTMAS/NEW YEAR) / Save Settings to
  Memcard / Reset Screensaver Loop / Next Scene / TTY Perf Log
  (OFF/SUMMARY/DETAIL/DEBUG) / Debug Info / Set Time-Date.
- **Custom font path (shipped)**: embedded 8x8 ASCII glyph table
  (`pmFontBits[96][8]` in `src/pause_menu/pause_menu.c`), pre-doubled to 16x16 at
  upload time (`pmUploadFont`), drawn glyph-by-glyph via manual SPRT
  primitives (`pmTextDrawChar`). PSn00bSDK's `FntFlush` was empirically
  broken in the scene-runtime context — it accumulates primitives
  without producing any visible pixels — so the in-tree font path
  replaces it entirely.
- **Panel rendering (shipped)**: translucent dim quad (POLY_F4
  semi-trans black) under an opaque purple panel with 12-pixel chamfered
  corner cutouts (3-rect rounded-rectangle look), drawn over the live
  scene every paused frame.
- **Sound mute (shipped)**: `SpuSetCommonMasterVolume` is not honored by
  DuckStation HLE, so the mute path zeros every voice volume + CD vol +
  master vol and clears the master-enable bit in `SPU_CTRL` via direct
  register writes.
- **Day/Night and Holiday overrides (shipped)**: flow into
  `fgLoopApplyVariant` in `src/jc_reborn.c` with priority
  `explicit menu override > Set Time/Date soft override > random`.
- Files: `src/pause_menu/pause_menu.c/h`. Design: [pause-menu-design.md](pause-menu-design.md).

## 4. Holiday expansion — 4 → 35 design

- The existing four holidays (Halloween, St Patrick's, Christmas, New
  Year) covered fewer than half of the calendar months. The new design
  doc proposes 35 US-centric holidays with at least one per month and
  an average of about three.
- Mix is federal + pop-culture + seasonal (e.g. Pi Day, Talk Like a
  Pirate Day, Star Wars Day, Moon Landing Day) so the screensaver
  always feels seasonally alive rather than dead-stretches between the
  four legacy events.
- Design: [holidays-expansion-design.md](holidays-expansion-design.md).
  Implementation is not in this batch.

## 5. SPI direct controller polling

- The BIOS pad system (`InitPAD` / `StartPAD`) was empirically broken in
  our PSn00bSDK 0.24 + DuckStation environment. Replaced with PSn00bSDK's
  reference SPI driver — timer-2 + SIO0 IRQ-driven, polling at 250 Hz.
- One critical bug found while porting: spicyjpeg's reference uses
  `tx_len=4`, but DuckStation's controller emulation only delivers
  button bytes when the **full** 5-byte poll sequence comes from the
  TX buffer (`tx_len=5`). That single off-by-one is what surfaced as
  "Start does nothing" during early pause-menu wiring.
- Files: `src/platform/ps1/spi.c/h`, integrated into `src/platform/ps1/events_ps1.c`. The pause
  menu, day-night override, and any future input feature all sit on top
  of this driver.

## 6. Memcard support (SHIPPED)

- `src/platform/ps1/memcard.c/h` saves user pause-menu settings (sound mute,
  day/night override, holiday override, soft time/date) to a single 8 KB
  block on memcard slot 1, file name `BASLUS-99999JCREB`.
- **Avoids the BIOS card driver entirely** (`InitCARD` / `StartCARD` /
  `_bu_init` / file API). The BIOS driver installs a persistent VBlank
  handler that fights our 250 Hz SPI controller-polling timer and tanks
  scene framerate. Instead, memcard I/O rides on top of our existing
  SPI driver via `SPI_CreateRequest` queue + raw byte sequencing per
  the nocash memcard protocol.
- During memcard ops the SPI poll rate drops 250 Hz → 50 Hz: a single
  card transaction is ~5.5 ms, longer than the 4 ms timer-2 tick that
  drives the controller poller, so we widen the tick to give the card
  exclusive SPI bus time.
- Save block carries a proper PS1 save header (SC magic + title + icon
  frame count + 16-color CLUT) with a hand-drawn 16x16 palm-tree icon,
  so the save shows up in the BIOS Memory Card Manager like a normal
  game save.
- Boot-time auto-load: if a save exists, settings are restored before
  the scene loop starts. The Save Settings menu entry writes; reads
  happen automatically at boot.

## 7. TTY log prefixes

- The complete set of greppable log-line prefixes after this batch:
  `JCSPI` (controller driver), `JCPAD` (pad layer), `JCBOOT` (boot /
  bootmode), `JCPERF` / `JCPERF2` (perf module), `JCPAUSE` (pause-menu
  one-shot snapshot), `JCMC` (memcard). All are stable key=value pairs
  intended for external perf-debugging agents.

---

## Pointers

- Design backbone: [pause-menu-design.md](pause-menu-design.md),
  [holidays-expansion-design.md](holidays-expansion-design.md),
  [performance-optimization-plan.md](performance-optimization-plan.md)
- Status frame: [current-status.md](current-status.md), [scene-status.md](scene-status.md)
- Workflow: [development-workflow.md](development-workflow.md)
