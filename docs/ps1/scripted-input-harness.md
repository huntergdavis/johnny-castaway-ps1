# PS1 Scripted Input Harness

> Rendered version: `/docs/scripted-input/` on the project website.

The scripted input harness lets the PS1 build press its own controller
buttons during a headless DuckStation run. It is used first for the pause
menu screenshot guide, but the useful idea is broader: any reproducible
controller route can become a test script, a TTY trace, and a set of captured
frames.

This is not a host keyboard macro. The script is embedded into the disc at
build time, parsed by the PS1 executable, and merged into the same active-high
button mask that real controller reads use. Menu code, Freeplay code, and
scene code all see ordinary pad input.

## Why it exists

The project needed a way to prove menu and controller behavior without a
human sitting in front of DuckStation:

- Start opens the pause menu after normal boot.
- D-pad navigation lands on the intended menu rows.
- Cross selects.
- Circle backs out.
- Freeplay, World Options, Accessibility, Sound Test, System, date, seed, and
  holiday pages can all be reached from a cold boot.
- A screenshot can be captured after a deliberate settle delay, not just at
  the nearest arbitrary frame interval.

The same mechanism is also a debugging tool. If a bug only appears after
"boot, wait, Start, Down twice, Cross, Circle, enter Freeplay again", that
route belongs in a pad script. The script removes timing guesswork from the
bug report.

## Runtime model

The flow is intentionally small:

1. `scripts/build-ps1.sh` reads `config/ps1/PADSCRIPT.TXT`.
2. Comment and blank lines are stripped.
3. The script text is embedded into `config/ps1/padscript_embedded.h`.
4. `BOOTMODE.TXT` enables the layer with `pad-script` or `pad-script-log`.
5. `src/platform/ps1/ps1_pad_script.c` parses the embedded script into fixed-size events.
6. `ps1PadButtonsWithAnalog()` reads the physical controller, folds the left
   analog stick into D-pad bits, then calls `ps1PadScriptMergeButtons()`.
7. Scripted buttons are ORed into the physical active-high mask.

Normal builds leave `PADSCRIPT.TXT` empty and omit the boot token. In that
case the runtime pays one disabled branch in `ps1PadScriptMergeButtons()` and
does not parse, log, allocate, or synthesize input.

## Key files

| File | Role |
|---|---|
| `config/ps1/PADSCRIPT.TXT` | Source script embedded into the PS1 executable at build time. Empty for normal builds. |
| `config/ps1/padscript_embedded.h` | Generated header produced by `scripts/build-ps1.sh`. |
| `src/platform/ps1/ps1_pad_script.c` | Parser, event scheduler, screenshot marker logger, and input merger. |
| `src/platform/ps1/ps1_pad_input.h` | Shared pad helper that folds analog into D-pad and merges scripted buttons. |
| `scripts/ps1-menu-input-harness.sh` | End-to-end menu screenshot route: stages boot/script files, builds, runs regtest, restores files. |
| `scripts/ps1-menu-harness-report.py` | Copies marker-aligned frames and rewrites the website menu guide. |
| `site/help/menu/index.md` | Generated player-facing menu guide. |

## Running the menu harness

Default full route:

```bash
./scripts/ps1-menu-input-harness.sh
```

That command temporarily writes:

- `config/ps1/BOOTMODE.TXT`
- `config/ps1/bootmode_embedded.h`
- `config/ps1/PADSCRIPT.TXT`
- `config/ps1/padscript_embedded.h`

It then rebuilds the PS1 executable, regenerates the CD image, runs
DuckStation regtest headlessly, extracts marker-aligned screenshots into
`site/assets/img/help/menu/`, rewrites `site/help/menu/index.md`, and restores
the staged files before exit.

Useful options:

```bash
# Faster screenshot settle window while tuning route timing.
./scripts/ps1-menu-input-harness.sh --settle-frames 30

# Keep the current disc image and only rerun the headless capture.
./scripts/ps1-menu-input-harness.sh --skip-build

# Print parsed events and button-mask transitions.
./scripts/ps1-menu-input-harness.sh --verbose

# Stream the full DuckStation console log instead of keeping it quiet.
./scripts/ps1-menu-input-harness.sh --raw-console

# Increase capture density or run length.
./scripts/ps1-menu-input-harness.sh --frames 15000 --interval 2
```

The current menu route starts from normal scene playback, waits 30 seconds,
presses Start, captures the main pause menu, walks into each major submenu,
and exits back out with Circle.

## Script syntax

`PADSCRIPT.TXT` is line-oriented. Blank lines and lines beginning with `#`
are ignored by the build embedder. Commands are case-insensitive.

```text
wait 30s
tap START
tap DOWN
hold R1+RIGHT 12
shot pause-main 30
```

Durations are frames by default. A trailing `s` means seconds at 60 Hz.

| Command | Form | Meaning |
|---|---|---|
| `wait` | `wait 30s` or `wait 1800` | Advance the script cursor without pressing buttons. |
| `tap` / `press` | `tap CROSS 12` | Hold a button mask for N frames, then insert an 8-frame gap. Default hold is 12 frames. |
| `hold` | `hold R1+RIGHT 45` | Hold a button mask for N frames without adding the tap gap afterward. |
| `shot` / `screenshot` / `mark` | `shot system 30` | Emit `JCPADSHOT label=system ...` after an optional settle delay. |

Supported button names:

```text
START SELECT
UP DOWN LEFT RIGHT
CROSS X CIRCLE O TRIANGLE SQUARE
L1 R1 L2 R2
```

Button combinations use `+` or `,`:

```text
hold R1+RIGHT 12
hold L1,R1,L2,R2 60
```

## Screenshot markers

`shot` does not capture a frame by itself. It prints a marker to PS1 TTY:

```text
JCPADSHOT label=world-options frame=5875 tick=5875
```

DuckStation regtest captures PNGs at the configured interval. The reporter
then selects the first captured frame at or after the marker target. This is
why the menu harness can press a button, wait for the real PS1 framebuffer to
settle, and publish a screenshot that shows the intended screen instead of
the previous or next one.

If the marker and nearest frame are too far apart, the reporter fails rather
than silently publishing the wrong image. Increase capture density with
`--interval 2` or tune the route with `--settle-frames`.

## Direct custom route

For a one-off bug repro, edit `config/ps1/PADSCRIPT.TXT`:

```text
wait 30s
tap START
shot pause-main 30
tap DOWN
tap DOWN
tap CROSS
shot freeplay-options 30
tap CIRCLE
shot pause-main-return 30
```

Then enable it in `config/ps1/BOOTMODE.TXT`:

```text
fgpilot fishing1 pad-script
```

For parser and mask logging:

```text
fgpilot fishing1 pad-script-log
```

Build and run through the normal path:

```bash
./scripts/build-ps1.sh
./scripts/make-cd-image.sh
./scripts/run-regtest.sh --frames 4200 --dumpinterval 5 --dumpdir scratch/pad-repro
```

When the route is done, clear `PADSCRIPT.TXT` or remove the `pad-script`
token. The menu harness restores staged files automatically; hand-written
experiments do not.

## Debugging failures

If a route lands on the wrong page:

1. Rerun with `--verbose` or `pad-script-log`.
2. Check `JCPADSCRIPT event=...` lines to confirm the parser saw the route you
   intended.
3. Check `JCPADSCRIPT buttons ... physical= script= merged=` lines to confirm
   the mask changed at the expected frame.
4. Check `JCPADSHOT` lines to confirm screenshot labels are after the menu
   transition, not on the same frame as the transition button.
5. Increase `--settle-frames` if the screen is correct but the capture is
   early.
6. Increase capture density with `--interval 2` if the marker falls between
   sparse dumped frames.

If Start never opens the menu, fall back to pad diagnostics:

```text
padtest
pad-diag
```

Those boot tokens exercise the lower-level SPI pad path. The scripted layer
can prove menu behavior, but it cannot prove a broken physical controller
cable or pad poll.

## Guardrails

- Keep scripts deterministic. Avoid routes that depend on random scene
  selection unless you also set `seed`.
- Keep normal `PADSCRIPT.TXT` empty.
- Do not use per-frame `printf()` as a substitute for this harness. Script
  markers are sparse; per-frame text logging changes timing and creates huge
  DuckStation logs.
- Use `scratch/` for custom output. Do not dump long runs into `/tmp`.
- Prefer `shot` labels that match page IDs or bug names:
  `pause-main`, `sound-test`, `freeplay-reenter`, `bug-20260501-menu-back`.

## Current generated artifacts

The current website menu guide is generated by this path:

```bash
./scripts/ps1-menu-input-harness.sh
./scripts/site-build-static-root.sh
```

The first command updates the source screenshots and
`site/help/menu/index.md`. The second command builds the static GitHub Pages
output under `docs/`, with relative links and the repository-root
`index.html` redirect preserved.

