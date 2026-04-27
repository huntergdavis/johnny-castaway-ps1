---
layout: page
title: The two-day SPI bug
eyebrow: Lab . War story
subtitle: The controller worked everywhere except where it mattered.
description: A debugging war story about the PS1 controller SPI/SIO0 polling bug that blocked the Johnny Castaway PS1 pause menu.
date: 2026-04-26
---

## The symptom

The pause menu needed the controller. Start to pause. D-pad to move. Cross to
select. Basic stuff.

The PSn00bSDK pad path looked like the obvious answer: `InitPAD`, `StartPAD`,
read `pad_buff`, move on. It worked in examples. It looked correct. It did not
work reliably in this runtime.

Sometimes every button read as released. Sometimes every button read as
pressed. Sometimes the pad looked alive until the scene runtime started doing
real work. The worst kind of bug: plausible, intermittent, and blocking.

## The false comfort of examples

The fallback was spicyjpeg's direct SIO0 polling example. Good code, clear
license, close to the metal. The project uses a derived version in `src/spi.c`.

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

- [`src/spi.c`]({{ site.github_url }}/blob/main/src/spi.c)
- [Pause menu docs]({{ '/docs/pause-menu/' | relative_url }})
- [PS1 hardware constraints]({{ '/docs/hardware/' | relative_url }})
- [Build infrastructure]({{ '/docs/infrastructure/' | relative_url }})
