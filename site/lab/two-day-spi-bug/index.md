---
layout: page
title: The two-day SPI bug
eyebrow: Lab · War story
subtitle: The controller worked everywhere except where it mattered.
description: A debugging war story about the PS1 controller SPI/SIO0 polling bug that blocked the Johnny Castaway PS1 pause menu.
date: 2026-04-26
image: /assets/img/help/menu/controls.png
image_alt: The Controls submenu inside the in-game pause menu — the surface that depends on every controller pad-poll byte landing. The two-day SPI bug was that DuckStation only delivered button bytes when the full five-byte sequence was sent.
image_width: 640
image_height: 448
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## The symptom

The pause menu needed the controller. Start to pause. D-pad to move. Cross to
select. Basic stuff.

The [PSn00bSDK]({{ '/docs/glossary/#psn00bsdk' | relative_url }}) pad path looked like the obvious answer: `InitPAD`, `StartPAD`,
read `pad_buff`, move on. It worked in examples. It looked correct. It did not
work reliably in this runtime.

Sometimes every button read as released. Sometimes every button read as
pressed. Sometimes the pad looked alive until the scene runtime started doing
real work. The worst kind of bug: plausible, intermittent, and blocking.

## The false comfort of examples

The fallback was spicyjpeg's direct SIO0 polling example. Good code, clear
license, close to the metal. The project uses a derived version in [`src/platform/ps1/spi.c`]({{ site.github_url }}/blob/main/src/platform/ps1/spi.c).

The example uses a 4-byte poll transmission. On this setup - PSn00bSDK 0.24
and DuckStation - that was not enough. The emulator did not deliver the actual
button bytes unless the full 5-byte sequence was sent.

Four bytes: `0xFFFF`.
Five bytes: real buttons.

Yeah.

## Why it took two days

Because every individual layer looked reasonable.

The SDK example was reasonable. The emulator behavior was reasonable if you
read it as "the transaction is not complete yet." The controller protocol is
old enough that every reference document phrases the sequence slightly
differently. The runtime had enough other moving parts that it was easy to
suspect timing, interrupts, pad mode, or memory corruption.

The fix was one number. Finding the number was the work.

## What changed after it worked

Once controller input was trustworthy, the pause menu became real. Sound mute.
Day/night. Holiday selection. Tide. Raft. Captions. Set Time. Credits. Debug
info. All of that UI work depends on a boring fact: when the player presses
Down, the runtime sees Down.

This is the kind of bug that never gets a glamorous commit title and absolutely
defines the product.

## Cross-links

- [`src/platform/ps1/spi.c`]({{ site.github_url }}/blob/main/src/platform/ps1/spi.c)
- [Pause menu docs]({{ '/docs/pause-menu/' | relative_url }})
- [Freeplay mode]({{ '/docs/freeplay/' | relative_url }}) — the
  largest live consumer of the controller path this bug blocked.
- [Scripted input harness]({{ '/docs/scripted-input/' | relative_url }})
  — the headless test harness that merges pad-script bytes into
  the same active-high mask the SPI driver fills.
- [PS1 hardware constraints]({{ '/docs/hardware/' | relative_url }})
- [API mapping (SDL2 → PSn00bSDK)]({{ '/docs/api/' | relative_url }})
  — the Input section maps `SDL_PollEvent` to the SPI driver
  this article is the war story for.
- [Glossary: tx_len]({{ '/docs/glossary/#tx-len' | relative_url }})
  — the one-byte fix at the center of the bug.
- [Build infrastructure]({{ '/docs/infrastructure/' | relative_url }})
