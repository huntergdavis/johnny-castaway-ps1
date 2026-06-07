---
layout: page
title: v0.8.1 — what the soak found that the matrix didn't
eyebrow: Lab · stability war story
subtitle: A randomized DuckStation soak found a scene-load freeze the per-commit perf matrix never reached. The fix is small. The discipline is the lesson.
description: How a long-run randomized DuckStation soak surfaced a MARY 4 scene-load freeze in the Johnny Castaway PS1 fan port that the bounded headless-perf matrix never exercised, and the clean-rect pressure estimator that v0.8.1-ps1 shipped to fix it.
date: 2026-05-06
image: /assets/img/mary4-ps1-raft-heartbreak.png
image_alt: Johnny on the island shoreline with Mary on a raft pulling away — the MARY 4 raft-heartbreak scene that surfaced the v0.8.1 soak-only freeze. Captured on PS1 hardware via the validation harness.
image_width: 961
image_height: 720
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

`v0.8.0-ps1` was the post-validation performance baseline. All 63
scenes signed off; `+0.9%` over target / [`99.5%` target speed]({{ '/docs/glossary/#target-speed' | relative_url }}) across
the 120 timing-bearing rows; the [retrospective]({{ '/lab/from-87-to-99-5/' | relative_url }})
walks through how the matrix moved there. That looked like the end
of the speed arc.

It wasn't. Within hours of the v0.8.0 disc going up, a randomized
long-run [soak]({{ '/docs/glossary/#soak-test' | relative_url }})
froze. `v0.8.1-ps1` is the fix. The story is short.

## The shape of the bug

The screensaver picks scenes at random. The soak ran for an
extended stretch, the picker eventually selected `MARY 4`, and the
boot proceeded only as far as:

```
JCPICK ... mary4
JCRECT 2-rect lower=(0,190,608,166) upper=(0,92,361,98)
```

The runtime then stopped. No crash, no panic. Stuck on the
clean-rect save before scene playback could start. Heap fragmented,
allocation refused, the runtime had nowhere to go — but the
[matrix]({{ '/perf/' | relative_url }}) had nothing flagged on
`MARY 4`. The perf rows for that scene came in green.

## Why the matrix missed it

The matrix runs each (scene, tide) variant from a deterministic
boot, scene by scene, with the same routing every time. That is
what makes its numbers comparable — every row is the same kind of
run. It is also what makes it a poor fit for finding a bug that
needs *prior heap state plus a particular random scene pick*. The
matrix never enters the random-position playback path with another
scene's residual pressure on the heap.

The soak does, by definition. That is the entire point of running
one.

## The actual fix

`v0.8.0`'s clean-rect pressure estimator used `fgBoundsW *
fgBoundsH * 2`. Cheap, wrong: it ignored the
[ocean wave band]({{ '/docs/glossary/#wave-band' | relative_url }})
expansion and the upper/lower split rect that wide-action scenes
emit. On `MARY 4`'s random-load path the real clean save was over
`256 KiB`; the estimator under-counted and didn't release optional
prefetch/walk memory in time.

`v0.8.1`'s [`foreground_pilot.c`]({{ site.github_url }}/blob/main/src/foreground_pilot/foreground_pilot.c) now computes pressure through one
helper that mirrors the clean-save path — pack bounds, ocean wave
band, clamping, low-tide split. `fgPlayOceanRuntimeScene()` calls
that estimate before allocating, so wide scenes drop optional
pressure before the heap fragments. The fix covers fourteen
non-exempt random-position scenes (`activity1`, `activity5`,
`fishing1`, `fishing2`, `fishing3`, `fishing7`, `fishing8`,
`johnny2`, `johnny4`, `johnny5`, `mary4`, `mary5`, `visitor4`,
`walkstuf3`) — not a `MARY 4`-specific bandage.

The `VISITOR 3` high/low refresh on the same commit stayed exact
to the v0.8.0 matrix baseline. The performance line did not move.

## What the discipline says

The project keeps two acceptance bars on purpose: visual signoff
and headless perf timing. The two-ledger framing on
[/about/]({{ '/about/' | relative_url }}) is the public version
of that. `v0.8.1` adds a third loop that keeps both honest:

- **Visual signoff** runs to the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}). Per-scene human
  review. Catches pixel and SFX wrong.
- **Headless matrix** runs every (scene, tide) variant
  deterministically. Catches timing wrong.
- **Soak** runs the actual screensaver loop randomized for hours.
  Catches state-coupling wrong — bugs that need a particular
  history to appear.

The matrix and the soak are not redundant. They are tuned for
different bugs. The matrix proves that no scene is slow on its own;
the soak proves that no scene is broken by its neighbors. A release
that promotes performance without exercising the soak is a release
that ships the next `MARY 4` freeze, signed off and timed.

## The receipts

- [v0.8.1-ps1 release notes]({{ '/source/docs/ps1/release-notes-0.8.1/' | relative_url }})
- [v0.8.1-ps1 GitHub release]({{ site.github_url }}/releases/tag/v0.8.1-ps1)
- [`src/foreground_pilot/foreground_pilot.c`]({{ site.github_url }}/blob/main/src/foreground_pilot/foreground_pilot.c) — the file the fix lives in; the centralized pressure helper the body describes (`fgPlayOceanRuntimeScene` allocation site).
- [Glossary: soak-test]({{ '/docs/glossary/#soak-test' | relative_url }})
- [Glossary: clean-rect]({{ '/docs/glossary/#clean-rect' | relative_url }})
- [/perf/]({{ '/perf/' | relative_url }}) — the matrix the row of `MARY 4` came in green on
- [/lab/from-87-to-99-5/]({{ '/lab/from-87-to-99-5/' | relative_url }}) — the broader perf retrospective; v0.8.1 is the stability follow-on that left its matrix mean untouched
- [/lab/the-63-scene-grind/]({{ '/lab/the-63-scene-grind/' | relative_url }}) — the visual-validation grind that came before all the perf work; v0.7.0 capped it
- [/scenes/mary4/]({{ '/scenes/mary4/' | relative_url }}) — the canonical scene the soak named
- [/scenes/building4/]({{ '/scenes/building4/' | relative_url }}) — the v0.8.0 prequel; same memory-pressure machinery, one generation earlier
- [/docs/walks/]({{ '/docs/walks/' | relative_url }}) — reference manual for the walk subsystem whose persistent clean buffer the "prefetch/walk memory" line above belongs to; Evolution by release covers the v0.8.0 retry path and v0.8.1 pressure fix in one place
