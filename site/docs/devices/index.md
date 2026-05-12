---
title: Devices — what it runs on
eyebrow: Reference · device matrix
subtitle: The emulator and hardware combinations the port has actually been tested against, plus the BIOS requirement and the paths the project has never tried.
description: Reference for the devices the Johnny Castaway PS1 fan port has been tested against — DuckStation as the reference emulator, PCSX-Redux as a should-work, ePSXe as unverified, real PlayStation 1 (SCPH-7501) via TonyHax as the smoke-tested hardware path, plus the BIOS requirement and unconfirmed paths.
---

A labor of love by Hunter Davis. The FAQ has short answers to "does it
run on a real PS1?" and "which emulators work?"; this page is the
honest long form. It lists what's actually been booted, what's
expected to work but isn't covered by every-commit testing, and what
the project has never tried.

If you paid for this, you were cheated. Open source and free.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## Tested · emulator

[DuckStation]({{ '/docs/glossary/#duckstation' | relative_url }})
is the reference emulator. Every commit gets booted in it, the
[headless regression harness]({{ '/docs/regtest/' | relative_url }})
runs in it via `-batch -nogui`, and the
[`{{ site.release.perf_target_speed_pct }}%` target-speed]({{ '/docs/glossary/#target-speed' | relative_url }})
rollup the
[battle card]({{ '/perf/' | relative_url }})
publishes is measured against it. If DuckStation reports a regression,
it's treated as a real bug. Other emulators are not in the every-commit
gate.

| Field             | Value                                                                                                                                   |
| ----------------- | --------------------------------------------------------------------------------------------------------------------------------------- |
| Project           | DuckStation                                                                                                                             |
| Tested cadence    | Every commit (manual smoke) + every release via headless regtest                                                                        |
| Platforms tested  | Linux (developer's daily driver)                                                                                                        |
| Other platforms   | DuckStation upstream supports Windows + macOS + Android; not gated by this project                                                      |
| BIOS              | Real PlayStation BIOS (e.g. `scph1001.bin`) required — DuckStation refuses to boot without one (see `BIOS errors` in [`/docs/regtest/`]({{ '/docs/regtest/' | relative_url }})) |
| Notable upstream  | [duckstation.org](https://www.duckstation.org/)                                                                                         |

## Tested · hardware

A real PlayStation 1 has been smoke-tested via the
[TonyHax]({{ '/docs/glossary/#tonyhax' | relative_url }})
softmod path. This is not a long-term soak — boot success and a few
minutes of scene cycling is the bar — but it confirms the build
clears the actual hardware envelope, not just DuckStation's HLE
approximation.

| Field          | Value                                                          |
| -------------- | -------------------------------------------------------------- |
| Console        | Sony PlayStation, model `SCPH-7501` (US, late-revision PSone-era motherboard) |
| Boot path      | TonyHax softmod ([`socram8888/tonyhax`](https://github.com/socram8888/tonyhax)) |
| Media          | Burned CD-R from the released `.bin`/`.cue` pair                |
| Tested cadence | Per-milestone (not every commit)                                |
| Notes          | Treat any boot success as a small miracle — the path is fragile by 2026 standards (1990s-era laser, decades-old drive, softmod brittleness). The [SPI driver]({{ '/lab/two-day-spi-bug/' | relative_url }}) story is one example of what shifts between HLE and metal. |

The build is expected to run on any retail PlayStation 1 from the
TonyHax-supported set, but only the SCPH-7501 listed above has been
verified. PS2 and PS3 backwards-compatibility modes have not been
tested and are not covered by the FISHING 1 bar.

## Should work · unverified

These paths are expected to work but the project does not run them
on every commit, so treat regressions as worth reporting.

- **PCSX-Redux** — A modern PS1 emulator with PSn00bSDK-friendly
  debug surfaces. Has not been part of the smoke gauntlet but is
  expected to boot. If a bug is DuckStation-specific, PCSX-Redux is
  the obvious second opinion.
- **PS2 backwards-compat** — The retail PS2's PS1 mode runs original
  discs natively. The TonyHax path used here is the homebrew route;
  PS2-via-backwards-compat is plausible if a `.bin`/`.cue` is burned
  to a CD-R, but it has not been tested.
- **PS3 backwards-compat** — Limited to early SKUs (CECHA / CECHB)
  with hardware PS1 emulation. Untested.
- **MemcardPro / similar SD-backed memory cards** — The runtime
  reads/writes one block of memcard data for accessibility-toggle
  persistence. Behavior on SD-backed cards is unverified but expected
  to work (the read/write path is standard `MemCardRead` /
  `MemCardWrite`).

## Unsupported / out of scope

- **ePSXe** — Unverified. Not part of the regtest gate and not on
  the smoke list. PRs that target ePSXe-specific bugs are not
  in scope — the engineering bar is "passes DuckStation."
- **No-BIOS emulators** (some HLE setups) — The project relies on
  PSn00bSDK and the standard `BIOS()` syscalls; running without a
  real BIOS is unsupported and will probably crash early.
- **Streaming services / cloud retro players** — Out of scope.

## BIOS requirement

DuckStation requires a real PlayStation 1 BIOS file. The project
does not ship one — it's a copyrighted Sony component. Dump from
your own console or source from your own physical hardware. The
`BIOS errors` callout in
[`/docs/regtest/`]({{ '/docs/regtest/' | relative_url }})
covers the symptoms when the BIOS is missing or wrong-region.

The `scph1001.bin` filename appears in regtest docs because that's
the file the project's CI matrix points at; any equivalent retail
BIOS (`SCPH-101`, `SCPH-7001`, etc.) should work.

## Related pages

- [Play]({{ '/play/' | relative_url }}) — the download + quickstart entrypoint that links here for "does it run on X?" questions.
- [FAQ]({{ '/faq/' | relative_url }}) — short-form answers for the same audience.
- [Regression testing]({{ '/docs/regtest/' | relative_url }}) — the harness DuckStation is exercised by.
- [Hardware]({{ '/docs/hardware/' | relative_url }}) — the PS1 envelope this port targets (33.8688 MHz MIPS, 2 MB RAM, 1 MB VRAM, 512 KB SPU, 2× CD drive at 300 KB/s).
- [Lab: the two-day SPI bug]({{ '/lab/two-day-spi-bug/' | relative_url }}) — a real story about what shifts between DuckStation HLE and real silicon.
- [Glossary]({{ '/docs/glossary/' | relative_url }}) — [`DuckStation`]({{ '/docs/glossary/#duckstation' | relative_url }}), [`TonyHax`]({{ '/docs/glossary/#tonyhax' | relative_url }}), [`target speed`]({{ '/docs/glossary/#target-speed' | relative_url }}), [`fishing1-bar`]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).
