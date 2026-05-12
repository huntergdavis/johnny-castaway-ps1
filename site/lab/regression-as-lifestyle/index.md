---
layout: page
title: Regression as a lifestyle
eyebrow: Lab · Advanced development techniques
subtitle: On this port, tests are not a CI checkbox. They are how the work thinks.
description: "A methodology essay about the Johnny Castaway PS1 regression suite: host references, DuckStation capture, frame metadata, perf logs, and human review."
date: 2026-04-26
image: /assets/img/visitor3-ps1-perspective.png
image_alt: VISITOR 3 — the canonical two-red-rows perf-battle multi-view stitch scene. The kind of regression artifact the headless harness captures and the essay describes. Captured on PS1 hardware via the validation harness.
image_width: 961
image_height: 720
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## The project is a regression problem

Johnny Castaway PS1 is not trying to invent a new game. It is trying to make a
1992 screensaver run on a 1994 console without losing the timing, the jokes,
or the tiny visual tells that make the original feel like itself.

That makes the whole project a regression problem.

There is always an old truth: the host interpreter, the original resource
files, the archived reference frames, the previous PS1 release that did not
flicker. Every change is measured against that old truth. If the new runtime is
faster but the fishing line lands one frame late, the new runtime is wrong.

## The layers

There are several test surfaces because no single one is enough.

- **Unit-ish C tests** cover utilities and host-side pieces where normal
  testing makes sense.
- **Holiday tests** verify date algorithms: Easter, Nth weekday, Election Day,
  fixed dates, no duplicate IDs, original holidays preserved.
- **Host captures** preserve what the decoded Sierra scenes do on the desktop.
- **PS1 regtests** boot DuckStation, capture frames, parse logs, and compare
  against expectations.
- **Vision classifier experiments** tried to identify scene families from
  screenshots when raw pixel diff was too brittle.
- **Human review** remains the merge bar for a validated scene.

That last line is not romantic. It is practical. Some bugs are obvious only
when you watch the gag. A caption can be one scene off and still look like
valid text. A sound can be synchronized enough for a hash and still feel late.

## Host truth

The host build is the nearest thing this project has to a source oracle. It
can run the decoded ADS/TTM scripts on a desktop, capture frames, and emit
metadata. Those artifacts are why the PS1 runtime can be judged at all.

The host references are not perfect. They are still an interpretation of the
Sierra engine, not the original 1992 binary running on Windows 3.1. But they
are stable, inspectable, and close enough to make progress. More importantly,
they are saved. A baseline that lives only in memory is not a baseline.

## PS1 truth

The PS1 run is the thing that matters. The pack can be correct and the runtime
can still fail because the CD read came late, the texture upload hit the wrong
VRAM page, or the dirty-rect restore left a stale tile on the sand.

That is why the regtest harness boots the actual disc image in DuckStation.
It exercises the CD layout, the PS1 executable, the runtime allocators, the
SPU path, and the pause-menu side effects. It is slower than a host test. Good.
It catches different bugs.

## What changed the work

The biggest unlock was structured `printf`.

Early on, logging was dangerous. Too much text through DuckStation's [TTY]({{ '/docs/glossary/#tty' | relative_url }}) path
changed timing or destabilized the run. That forced a weird era of framebuffer
debugging: colored bars, one-pixel markers, screenshots decoded after the
fact. Once bounded, gated TTY logging became reliable, performance work
changed shape. The runtime could say exactly how many [VBlanks]({{ '/docs/glossary/#vblank' | relative_url }}) went to render,
restore, present wait, prefetch overrun, and blocking CD reads.

That is when the project stopped arguing by vibe.

The next unlock was scripted controller input. Menus are not validated by
hoping a human remembers the route. A pad script boots the disc, waits,
presses Start, walks the menu, and drops `JCPADSHOT` markers where the
screenshots should land. The result is documentation that is also a test:
the [menu help guide]({{ '/help/menu/' | relative_url }}) is made from real
PS1 framebuffer captures, not a hand-maintained diagram.

## The lifestyle part

Regression testing is usually presented as something you add after the
interesting work. Here it is the interesting work. Every scene is finite.
Every holiday is finite. Every source artifact is finite. The job is to keep
turning unknowns into rows in a table until there are no more rows left.

It sounds tedious. It is tedious. It is also how a one-person port avoids
pretending.

## Cross-links

- [Regression testing docs]({{ '/docs/regtest/' | relative_url }})
- [Scripted input harness]({{ '/docs/scripted-input/' | relative_url }})
- [Regtest reference cases]({{ '/archaeology/regtest-references/cases/' | relative_url }})
- [Vision-classifier work]({{ '/docs/vision/' | relative_url }})
- [Performance battle card]({{ '/perf/' | relative_url }}) —
  the second ledger; same headless harness, just measuring time
  instead of pixels.
- [The 63-scene grind]({{ '/lab/the-63-scene-grind/' | relative_url }})
- [From 87 to 99.5]({{ '/lab/from-87-to-99-5/' | relative_url }}) —
  the regression discipline applied to perf optimization.
- [v0.8.1: what the soak found that the matrix didn't]({{ '/lab/v081-mary4-freeze/' | relative_url }}) —
  the third loop on top of the per-commit gate: long-run
  randomized [soak-test]({{ '/docs/glossary/#soak-test' | relative_url }}).
- [Scene ledger]({{ '/scenes/' | relative_url }})
