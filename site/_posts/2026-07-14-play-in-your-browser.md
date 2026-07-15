---
layout: post
title: "Johnny now runs in your browser — July 14, 2026"
date: 2026-07-14
editor_note: "The v1.0.0 disc now boots straight from the website: a pinned EmulatorJS build of PCSX-ReARMed with an HLE BIOS streams the release .bin/.cue into the page, no emulator install and no BIOS hunt. This post covers how the player is wired and what the patched core fixes."
description: "Play Johnny Castaway PS1 in the browser: how the EmulatorJS + patched PCSX-ReARMed player works, how deploys pull the release disc, and what the keyboard maps to."
---

The shortest path to Johnny just got a lot shorter. As of today the
**[browser player]({{ '/play/online/' | relative_url }})** boots the latest
release disc directly on the website — no emulator install, no BIOS file, no
download-and-unzip step. Click launch, wait for the ~76&nbsp;MiB image to
stream in, and the screensaver starts doing what it has always done best:
ignoring you, beautifully.

## How it's wired

The player is the same system we built for our Zoomies PS1 project, ported
whole:

- **A pinned [EmulatorJS](https://emulatorjs.org/) 4.2.3 runtime** with a
  **patched PCSX-ReARMed core**, vendored into the site. The patch matters:
  upstream's interpreter drops PlayStation hardware writes made through KSEG
  aliases (PSn00bSDK uses those for the SPU), which would leave SPU control
  and volume registers silently at zero in the browser build. The patch also
  repairs Level-A XA decoding and routes the core's audio through a master
  gain node so the page's volume and mute controls actually work.
- **An HLE BIOS.** Nothing BIOS-shaped ships with the site or lives in the
  repository — the core's high-level BIOS emulation boots the disc.
- **Release-time disc delivery.** The repository never carries a disc image.
  At deploy time the Pages workflow downloads the latest GitHub release's
  `.bin`/`.cue` into the site artifact and writes a small `metadata.json`
  describing the build; the player reads that to know what to stream. Ship a
  new release, and the browser player serves it on the next deploy — nothing
  to remember.
- **A local prototype loop.** `./scripts/run-web.sh` in the repo serves the
  site plus the current development disc over the LAN with a fresh build
  identifier per rebuild, so a work-in-progress image is playable in a
  browser tab seconds after `make-cd-image.sh` finishes.

## Controls

The screensaver needs no input — watching is the intended API. When you do
want to drive it, the keyboard maps to the pad: **Enter** opens the pause
menu (Start), **arrow keys** are the D-pad, **K** is Cross, **L** is Circle,
**Backspace** is Select, and the Freeplay walk-speed and world-toggle combos
are listed on the player page. A connected gamepad is picked up
automatically.

## The honest caveat

This project's soul is real-hardware fidelity — address-verified CD
streaming, bounded waits, a launch-era console on the bench. A browser
WebAssembly core with an HLE BIOS is *not* that bar, and we're not claiming
it is. The [scene ledger]({{ '/scenes/' | relative_url }}) bar is still
judged on hardware and DuckStation. The browser player is the welcome mat:
the fastest possible way to see why this thing is worth a CD-R.
