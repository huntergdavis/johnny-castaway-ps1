---
layout: page
title: About
eyebrow: What this is . How it works
subtitle: A fan port of Sierra's Johnny Castaway to the original PlayStation, built as a hybrid host-and-replay pipeline.
description: A ground-up PS1 fan port of Sierra's 1992 Johnny Castaway screensaver. Hybrid host-capture and PS1-replay. Open source, GPL-3.0.
---

## In one paragraph

Sierra's *Johnny Castaway* is a 1992 Windows 3.1 screensaver about a
small man on a small island. This is a fan port of it to the original
PlayStation. It is a labor of love by Hunter Davis. Hunter does not own
or have a license to the Johnny Castaway character; the original
creator generously allows fan ports. If you paid for this, you were
cheated. The technical short version: the project does not run Sierra's
original ADS / TTM bytecode on the PS1 at all. A desktop host runs the
real engine, captures every visible foreground draw plus every
`PLAY_SAMPLE` event, and writes the result into a small per-scene
binary called an **FG2 pack**. The PS1 build loads packs from the disc
and replays them, while owning only the narrow runtime it must:
background, wave animation, holiday overlay, controller input, SPU
audio. That trade is why a 63-scene screensaver fits onto a CD-ROM and
inside 2&nbsp;MB of RAM at all.

Current release: `{{ site.release.tag }}`. Validated scenes:
`{{ site.release.scenes_validated }} / {{ site.release.scenes_total }}`
under the project's acceptance bar (pixel-perfect visuals plus synced
SFX, signed off by human visual and audible review across every
applicable variant).

## The hybrid pipeline (one sketch)

A *pack* is a small binary file that records every visible bitblit in a
scene -- what was drawn, where, when, and against what background --
plus a per-frame sound-event table. There is one high-tide pack and
one low-tide pack per scene. Each pack carries its own palette,
frame-timing table, base-frame full-render, and per-frame diff spans.
The PS1 build's job is to replay them in step with its own background
and overlay layers; it never has to interpret a Sierra bytecode op at
runtime.

That choice is the whole shape of the project. The PS1 has 2&nbsp;MB of
main RAM, 1&nbsp;MB of VRAM, 512&nbsp;KB of SPU RAM, and a 2x CD drive
with ~150ms cold-seek latency. Sierra's TTM/ADS interpreter on the
desktop side cheerfully resolves resources by name out of a flat
filesystem and replays prior scenes to establish state. None of that
is workable on a console with no syscall layer, no filesystem cache,
and a sprite engine that wants pre-mangled CLUT-indexed bitmaps. The
hybrid pipeline punts every piece of "smart" work back to the host
build where RAM and CPU are not constraints, and ships the PS1 a
deterministic flipbook to render.

```
[ original Sierra engine ]   --plays a scene-->  [ host capture ]
                                                    |
                                                    v
                                             [ FG2 packs:  one
                                               per ADS+tag ]
                                                    |
                                                    v
                       [ CD-ROM image ]   <-bundles--+
                                                    |
                                                    v
                                            [ PS1 GPU replay ]
```

The price: each scene needs a verified host capture and a successful
PS1 replay before it joins the validated count. There is no shortcut.
At `{{ site.release.tag }}`, all
**{{ site.release.scenes_validated }} of {{ site.release.scenes_total }}**
scenes are signed off — every row in the
[per-scene ledger]({{ '/scenes/' | relative_url }}) clears the
FISHING 1 bar across every applicable variant. The hard cluster
near the end was the foreground-only multi-view scenes
(`MISCGAG 1`, `MISCGAG 2`, `STAND 1`, the wide LILLIPUTIAN arrival),
which all needed the generic normal / far-left / far-right host
stitch before their packs replayed cleanly.

The second ledger — the
[performance battle card]({{ '/perf/' | relative_url }}) — is its
own bar, separate on purpose. At `{{ site.release.tag }}` the matrix
averages `{{ site.release.perf_target_speed_pct }}%` target speed
across the 120 timing-bearing scene/tide rows; the
[reference manual]({{ '/docs/performance/' | relative_url }})
explains what each column means and the
[retrospective on how it got there]({{ '/lab/from-87-to-99-5/' | relative_url }})
walks through which experiments landed and which didn't.

## What this isn't

A few things this project is deliberately not trying to be:

- **Not an emulator.** It does not run Sierra's original Win16
  binary. It does not interpret ADS or TTM bytecode on the PS1. It
  replays packs.
- **Not a re-creation.** No one is rewriting the engine in pure C++
  and calling it homage. The packs are derived from the real
  desktop runtime running real Sierra data files.
- **Not a polished commercial product.** It boots in DuckStation,
  it should boot on a real PS1, the regtest harness is the source
  of truth for what works, and the visible bugs are documented
  rather than papered over.
- **Not a community hub.** Issues and PRs on
  [{{ site.repo }}]({{ site.github_url }}) are welcome but
  unscheduled. There is no Discord, no roadmap survey, no roadmap
  voting, no Patreon.

## Where to go from here

- [/about/method/]({{ '/about/method/' | relative_url }}) -- the
  technical deep-dive: pipeline, pack format, hardware gotchas
  hit on the way (SPI pad polling, `FntFlush`, dirty-rect
  bookkeeping, SPU HLE divergence, TTY printf).
- [/about/status/]({{ '/about/status/' | relative_url }}) --
  component-level status. Renderer, audio, input, captions,
  holidays, pause menu, memcard, regtest harness, host capture,
  CD packaging.
- [/about/history/]({{ '/about/history/' | relative_url }}) --
  the timeline. Pre-port era, first PS1 attempts, the hybrid
  pivot, the 63-scene grind, where it stands at
  `{{ site.release.tag }}`.
- [/about/voice/]({{ '/about/voice/' | relative_url }}) -- the
  editorial standard the prose on this site holds itself to.
  Read it before writing a new page.
- [/about/dev-environment/]({{ '/about/dev-environment/' | relative_url }})
  -- the actual desk behind the work, photographed. Two LLM
  sub-agents, the dunking bird auto-poker, DuckStation,
  editor/terminal column, build farm, bottom-monitor telemetry.
- [/scenes/]({{ '/scenes/' | relative_url }}) -- live per-scene
  ledger. All 63 rows clear the FISHING 1 bar; family jump nav,
  per-scene case studies, last-verified release tag.
- [/perf/]({{ '/perf/' | relative_url }}) -- the second ledger:
  126-variant headless-perf battle card with sortable columns
  and color-coded Target Speed cells.
- [/devlog/]({{ '/devlog/' | relative_url }}) -- the dated
  worklogs that drove each phase, in original form.
- [/archaeology/]({{ '/archaeology/' | relative_url }}) -- older
  status surfaces, retired tooling, the harness era, the
  restore-pilot era, and other paths that did not become the
  active methodology.
- [/lab/]({{ '/lab/' | relative_url }}) -- fifteen feature-length
  retrospectives, newest first. The
  [post-validation performance loop]({{ '/lab/from-87-to-99-5/' | relative_url }}),
  [the site itself as a small program]({{ '/lab/the-site-itself/' | relative_url }}),
  [the 24/7 build farm]({{ '/lab/build-farm/' | relative_url }}),
  [the 63-scene grind]({{ '/lab/the-63-scene-grind/' | relative_url }}),
  the LLM pass, hallucination engineering, regression as a
  lifestyle, and the rest.
- [/hack/]({{ '/hack/' | relative_url }}) -- a learning path for
  hackers who want to read the C, port to another machine, or
  understand the debugging loops.
- [/source/]({{ '/source/' | relative_url }}) and
  [/resources/]({{ '/resources/' | relative_url }}) -- the complete
  documentation shelf and asset catalog.

The repository is at
[{{ site.repo }}]({{ site.github_url }}). Open source under GPL-3.0,
inherited from upstream
[jno6809/jc_reborn](https://github.com/jno6809/jc_reborn) -- without
that engine decode this port would not exist.
