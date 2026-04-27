---
title: Credits
eyebrow: A labor of love
subtitle: The full acknowledgements behind a small port.
---

## In the game

The pause menu's Credits page reads, verbatim:

> A labor of love by Hunter Davis.
>
> Hunter does not own or have a license to the Johnny Castaway
> character. The original creator generously allows fan ports.
>
> If you paid for this, you were cheated.
> Open source and free.
> github.com/huntergdavis/johnny-castaway-ps1

That's the whole credits screen on the disc. The list below is
the longer version — the people whose work this project stands on.

## Original game

- **Sierra On-Line** — the original *Johnny Castaway* (1992),
  the character, all original art and audio. The *Castaway*
  character is theirs, not this project's. This site reflects
  that in its chrome and its [legal page]({{ '/legal/' | relative_url }}).

## Prior ports (the giants whose shoulders this stands on)

- **`jno6809/jc_reborn`** — the SDL2 port that decoded the original
  engine's ADS / TTM / RES bytecode. This project's host build
  is a fork of that work.
- **`nivs1978/JCOS`** — an alternate decoding effort that helped
  cross-validate frames.
- **`xesf/Castaway`** — additional reverse-engineering notes.
- **The Sierra Chest archive** — preserved manuals and box copy.

## Toolchain

- **PSn00bSDK** (Lameguy64 et al.) — the open-source PS1 SDK that
  makes a project like this thinkable in 2026.
- **DuckStation** (Connor McLaughlin et al.) — the emulator that
  every commit gets tested against.
- **mkpsxiso** — `.bin/.cue` packing.
- **spicyjpeg's pad-poll example** — the SPI driver used here is
  derived from that MPL-licensed code (see `src/spi.c`).
- **Meeus / Jones / Butcher** — the Easter algorithm used for
  movable holidays.

## This port

- **Hunter Davis** — wrote it, broke it, fixed it again.

## Closed captions

The caption text was authored fresh for this port from scene
content, not lifted from any prior corpus. The [caption audit]({{ site.github_url }}/blob/main/docs/ps1/caption-audit-2026-04-26.yaml)
shows the confidence level of every ADS-tag → caption mapping.

---

The site is text-only on purpose — no portraits, no contributor
avatars. The drawCredits voice is plainspoken, and the chrome
matches.
