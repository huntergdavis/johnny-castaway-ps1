---
layout: post
title: "Scripted pad input, menu screenshots, and testing the UI like a player"
date: 2026-05-01
summary: "The next test harness teaches the PS1 build to press its own controller buttons, capture every major pause-menu screen, and publish the result as player help."
---

Freeplay made the pause menu important enough that screenshots are no
longer decoration. The menu is now a piece of the machine: it enters and
exits Freeplay, opens the gag and visitor catalogs, changes the island's
world state, plays sound effects, edits date and RNG seed, and turns
captions on and off.

So the test harness grew a controller.

The PS1 build now has an opt-in pad-script layer. At build time,
`config/ps1/PADSCRIPT.TXT` is embedded beside `BOOTMODE.TXT`. At runtime it
does nothing unless the boot string includes `pad-script` or
`pad-script-log`. When enabled, it merges scripted button masks into the
same active-high controller value that the real pad uses. Start is still
Start. Cross is still Cross. The menu code does not get a fake API.

That small choice matters. A host-side key injector would test DuckStation
window focus. This tests Johnny.

The first harness script boots a normal foreground scene, waits 30 seconds,
presses Start, walks the major pause-menu screens, and drops `JCPADSHOT`
markers into the PS1 TTY log. DuckStation regtest captures frames every few
VBlanks. Each marker is delayed until the target menu has had time to draw,
and the reporter copies the first dumped frame at or after that point before
rewriting the [menu help guide]({{ '/help/menu/' | relative_url }}).

That gives the project a new kind of documentation: generated from the
running disc, checked by the same deterministic tooling we use for
regression work, and useful to a human who just wants to know what the
System menu does.

The loop is deliberately boring:

```text
wait 30s
tap START
tap DOWN
tap DOWN
tap CROSS
shot freeplay-options 30
```

The power is in the boring part. Once a menu path can be written down as
button presses, it can become a test case, a screenshot, and a release
artifact. If a future refactor breaks Circle as Back, the harness gets
stuck. If a sub-screen grows too tall, the screenshot makes it obvious. If
Start stops working on the SPI pad path again, no guide page gets produced.

That is the whole philosophy of this port in miniature: make the PlayStation
do the thing, make it repeatable, and then let the artifact tell the truth.

The full operational reference is now
[Scripted input harness]({{ '/docs/scripted-input/' | relative_url }}).
