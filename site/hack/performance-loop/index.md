---
layout: page
title: The Performance Loop
eyebrow: Curious hacker path
subtitle: Printf, telemetry, headless DuckStation, and the long run.
description: The performance testing loop behind the Johnny Castaway PS1 port.
---

Performance work on this project became useful only when it became boring.
Run the same scene. Capture the same frames. Record the same counters. Change
one thing. Run it again. Leave it running long enough for rare failures to show
up.

The project needed unattended repetition before it could distinguish a real
stability improvement from one good boot. The loops have to run long enough
for the rare failures to surface, which means leaving them running for days
on end without a person at the keyboard.

## Why Printf Mattered

For a while, `printf` was both the obvious tool and part of the problem. Too
much unbounded TTY output could destabilize the runtime. That forced the
project into visual debugging first. Later, bounded performance logs came
back as [`JCPERF2`]({{ '/docs/glossary/#jcperf' | relative_url }}) lines with
explicit levels; the older `JCPERF` summary is still available for archaeology
or compatibility by compiling with `PS1_PERF_LEGACY_TRACE=1`:

- Summary logs for scene setup and teardown.
- Detail logs for frame behavior.
- Debug logs only when the run is short enough to survive the noise.

That one change turned "I think the renderer is slow here" into a trace that
could be compared across builds.

## The Loop That Actually Worked

The useful loop combines:

- [Regression testing]({{ '/docs/regtest/' | relative_url }}) for
  repeatable boots and captures.
- [The headless-perf battle card]({{ '/perf/' | relative_url }})
  for the live timing matrix — 126 scene/tide variants with
  sortable, color-coded `target_speed` cells.
- [Performance docs]({{ '/docs/performance/' | relative_url }}) for
  counter definitions and known bottlenecks.
- [Regtest reference cases]({{ '/archaeology/regtest-references/cases/' | relative_url }})
  for frozen host baselines.
- [Visual debugging]({{ '/hack/visual-debugging/' | relative_url }})
  for the cases where numbers pass and pixels fail.
- [Lab retrospectives]({{ '/lab/' | relative_url }}) for the human
  side of keeping a one-person build farm honest. Two specific
  ones to start with:
  [from 87 to 99.5]({{ '/lab/from-87-to-99-5/' | relative_url }})
  on the post-validation perf arc, and
  [the v0.8.1 MARY 4 freeze]({{ '/lab/v081-mary4-freeze/' | relative_url }})
  on the
  [soak loop]({{ '/docs/glossary/#soak-test' | relative_url }})
  catching what the matrix can't.

The matrix and the soak are not redundant. The matrix runs each
variant deterministically from a clean boot — perfect for
"is this scene slow on its own?" The soak runs the actual
screensaver loop randomized for hours — perfect for "is this
scene broken by its neighbors?" A release that promotes
performance without exercising both is a release that ships the
next state-coupling bug, signed off and timed.

This is the same lesson as the rest of the port: make the feedback loop narrow
enough that the machine can answer. The PS1 is not vague. It does exactly what
you told it to do. The hard part is arranging your tools so you can hear the
answer.

## Related pages

- [Hack: start here]({{ '/hack/start-here/' | relative_url }})
  — first-day flow if you haven't already done the build and
  boot loop.
- [Hack: learn C from Johnny]({{ '/hack/learn-c/' | relative_url }})
  — the `printf`-as-tool-with-blast-radius habit reads
  differently after the visual-debugging detour above.
- [Hack: visual debugging]({{ '/hack/visual-debugging/' | relative_url }})
  — the screenshot-and-overlay loop the matrix runs alongside.
- [Hack: memory wars]({{ '/hack/memory-wars/' | relative_url }})
  — most of the matrix's interesting failures trace back to
  the budget overruns this page's siblings document.
- [Hack: port to a new platform]({{ '/hack/port-to-a-new-platform/' | relative_url }})
  — the loop discipline travels even when the toolchain
  doesn't.
- [Performance battle card]({{ '/perf/' | relative_url }}) —
  live 126-variant matrix the loop above writes into.
- [Performance reference]({{ '/docs/performance/' | relative_url }})
  — column meanings, counter definitions, experiment-log
  discipline.
- [Lab: from 87 to 99.5]({{ '/lab/from-87-to-99-5/' | relative_url }})
  — the retrospective on the optimization arc this loop drove.
- [Lab: build farm]({{ '/lab/build-farm/' | relative_url }})
  — the 24/7 Docker-runner machinery wrapped around the loop.
- [Lab: dunking bird]({{ '/lab/dunking-bird/' | relative_url }})
  — the parallel-agent infrastructure that keeps the loop
  productive between human review passes.
- [Glossary: JCPERF2 / legacy JCPERF]({{ '/docs/glossary/#jcperf' | relative_url }})
  · [Glossary: scene-end]({{ '/docs/glossary/#scene-end' | relative_url }})
  · [Glossary: soak-test]({{ '/docs/glossary/#soak-test' | relative_url }})
