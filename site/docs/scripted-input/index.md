---
layout: page
title: Scripted input harness
eyebrow: Reference
subtitle: Headless controller routes for menu testing, screenshot capture, and repeatable PS1 bug repros.
description: "How Johnny Castaway PS1 scripts controller input inside the PS1 executable, drives DuckStation regtest, and captures marker-aligned menu screenshots."
---

A labor of love by Hunter Davis. This is the harness that lets the PS1 build
press its own controller buttons. It is small, but it changes the kind of bugs
that can be chased: instead of saying "open the menu and go down twice," the
project can write that route down, boot the disc, run DuckStation headlessly,
and collect the exact framebuffer that appeared half a second after the input.

This is not a desktop macro. The script lives on the disc image. The PS1
runtime parses it and merges those buttons into the same active-high pad mask
used by a real controller. The pause menu does not know it is being tested.
Freeplay does not know it is being tested. That is the point.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## The fast path

```bash
./scripts/ps1-menu-input-harness.sh
```

That command stages a temporary boot mode and pad script, rebuilds the disc,
runs `duckstation-regtest`, captures the major pause-menu screens, rewrites
[the menu help guide]({{ '/help/menu/' | relative_url }}), and restores the
staged files when it exits.

Useful variants:

```bash
./scripts/ps1-menu-input-harness.sh --settle-frames 30
./scripts/ps1-menu-input-harness.sh --verbose
./scripts/ps1-menu-input-harness.sh --skip-build
./scripts/ps1-menu-input-harness.sh --frames 15000 --interval 2
```

`--verbose` switches the boot token from `pad-script` to `pad-script-log`,
which prints parsed events and merged button masks. `--settle-frames` controls
how long the route waits after a transition before emitting a screenshot
marker. `--interval` controls how often DuckStation dumps frames.

## Script language

`config/ps1/PADSCRIPT.TXT` is embedded at build time by
`scripts/build-ps1.sh`. Blank lines and comments are stripped. Commands are
case-insensitive.

```text
wait 30s
tap START
tap DOWN
tap DOWN
tap CROSS
shot freeplay-options 30
```

Durations are frames unless they end in `s`, which means seconds at 60 Hz.

| Command | Example | What it does |
|---|---|---|
| `wait` | `wait 30s` | Advances the script clock with no input. |
| `tap` / `press` | `tap CROSS 12` | Holds a button mask briefly, then inserts an 8-frame gap. |
| `hold` | `hold R1+RIGHT 45` | Holds a mask without adding the tap gap afterward. |
| `shot` / `screenshot` / `mark` | `shot system 30` | Prints a labeled `JCPADSHOT` marker after an optional settle delay. |

Supported buttons are `START`, `SELECT`, D-pad directions, `CROSS` / `X`,
`CIRCLE` / `O`, `TRIANGLE`, `SQUARE`, `L1`, `R1`, `L2`, and `R2`.
Combinations use `+` or `,`.

## Why `shot` exists

DuckStation regtest captures frames at an interval. The PS1 script cannot
force DuckStation to dump a PNG on that exact [VBlank]({{ '/docs/glossary/#vblank' | relative_url }}), so it does the next best
thing: it prints a sparse marker into TTY.

```text
JCPADSHOT label=world-options frame=5875 tick=5875
```

`scripts/ps1-menu-harness-report.py` reads those markers, finds the first
captured PNG at or after each marker, copies it into
`site/assets/img/help/menu/`, and records the frame delta in the generated
page. If the nearest capture is too far away, the reporter fails. Wrong
screenshots are worse than missing screenshots.

## When to use it

Use scripted input for problems that are really input paths:

- Start no longer opens the pause menu.
- Circle no longer backs out from one submenu.
- A pause-menu refactor moved a row and a scripted route lands on the wrong
  page.
- Freeplay enters, exits, and then fails to enter on the second attempt.
- A world option applies from `R1 + D-pad` but not from the menu.
- A bug report needs exact timing: boot, wait 30 seconds, Start, Down, Down,
  Cross, wait, Circle.

Use scene regtest or the perf matrix for scene timing. Use host capture when
the question is "what did the original scene draw?" Use human visual + audible
signoff for scene certification. The pad harness is for deterministic input
routes and the screenshots those routes produce.

## Files worth reading

| File | Why |
|---|---|
| [`src/platform/ps1/ps1_pad_script.c`]({{ site.github_url }}/blob/main/src/platform/ps1/ps1_pad_script.c) | Parser, scheduler, TTY markers, button-mask merge. |
| [`src/platform/ps1/ps1_pad_input.h`]({{ site.github_url }}/blob/main/src/platform/ps1/ps1_pad_input.h) | Shared helper that folds analog stick into D-pad and calls the script merger. |
| [`config/ps1/PADSCRIPT.TXT`]({{ site.github_url }}/blob/main/config/ps1/PADSCRIPT.TXT) | Empty normal-build script source. |
| [`scripts/ps1-menu-input-harness.sh`]({{ site.github_url }}/blob/main/scripts/ps1-menu-input-harness.sh) | End-to-end menu route runner. |
| [`scripts/ps1-menu-harness-report.py`]({{ site.github_url }}/blob/main/scripts/ps1-menu-harness-report.py) | Marker-aligned screenshot extraction and help-page generator. |
| [`docs/ps1/scripted-input-harness.md`]({{ site.github_url }}/blob/main/docs/ps1/scripted-input-harness.md) | Full source runbook. |

## The discipline

Keep `PADSCRIPT.TXT` empty for normal builds. Keep routes deterministic. Avoid
per-frame logging. Put custom output under `scratch/`. Give screenshot labels
boring names that can become page IDs or bug IDs.

This harness is useful because it is boring. It takes the kind of manual
testing that usually disappears into memory and turns it into a file, a log,
a screenshot, and a page on the website.

## Related pages

- [Pause menu]({{ '/docs/pause-menu/' | relative_url }}) — the
  surface most pad-script routes traverse.
- [Help: menu screenshots]({{ '/help/menu/' | relative_url }}) —
  what this harness writes when it runs end-to-end.
- [Regression testing]({{ '/docs/regtest/' | relative_url }}) —
  the headless DuckStation runner that consumes the pad script
  + screenshot markers.
- [Glossary: BOOTMODE.TXT]({{ '/docs/glossary/#bootmode' | relative_url }})
  — the boot-token surface that selects `pad-script` and
  `pad-script-log` modes.
- [Glossary: PADSCRIPT.TXT]({{ '/docs/glossary/#padscript' | relative_url }})
  — the script-source file this page documents; embedded at
  build time by `scripts/build-ps1.sh`.
- [Glossary: JCPADSHOT]({{ '/docs/glossary/#jcpadshot' | relative_url }})
  — the TTY screenshot-marker line the `shot` / `screenshot` /
  `mark` commands emit, consumed by the harness to copy captured
  frames into the menu help guide.
- [Devlog: scripted-pad menu harness]({{ '/devlog/scripted-pad-menu-harness/' | relative_url }})
  — the implementation narrative.
- [Lab: the two-day SPI bug]({{ '/lab/two-day-spi-bug/' | relative_url }})
  — war story for the SIO0 polling fix that this harness's
  pad-byte merge writes into.
- [Lab: regression as a lifestyle]({{ '/lab/regression-as-lifestyle/' | relative_url }})
  — magazine treatment of why scripted-input + frame-diff
  regression are how this project gets work done. Reciprocal
  of the link from there to here.

