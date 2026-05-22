---
layout: page
title: The dunking bird
eyebrow: Lab · tools
subtitle: A dumb hack that keeps the loop running.
description: Dunking bird is a small piece of software the author wrote to keep long-running unattended jobs alive. It got used a lot during the Johnny Castaway PS1 port.
date: 2026-04-26
image: /assets/img/dunking-bird-2026-05-06.jpg
image_alt: A novelty thirty-cent dunking-bird desk toy positioned to tap a keyboard — the literal hardware behind the long-unattended-runs trick the essay names.
image_width: 1100
image_height: 522
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## What it is

<figure>
  <picture>
    <source type="image/webp" srcset="{{ '/assets/img/dunking-bird-2026-05-06.webp' | relative_url }}" />
    <img src="{{ '/assets/img/dunking-bird-2026-05-06.jpg' | relative_url }}"
         width="1100" height="522"
         fetchpriority="high" decoding="async"
         alt="The Dunking Bird application window: a two-row task list. Row 1: jc_reborn : node — Konsole, Min 40.0, prompt 'continue to imp...'. Row 2: jc_reborn_claude_2 : clau... , Min 20.0, prompt 'You are a profe...'. Status bar: '√ 2 dunkers (1 running)'." />
  </picture>
  <figcaption>Dunking Bird mid-double-dunk on 2026-05-06. Two agent slots queued at 40-min and 20-min intervals. The full screenshot in context is at <a href="{{ '/about/dev-environment/' | relative_url }}">/about/dev-environment/</a>.</figcaption>
</figure>

Dunking bird is a small piece of software I wrote. It is not advanced
development. It is not a research artifact. It is a dumb hack that pokes
multiple LLM agents in parallel so they keep working instead of going
idle and falling over.

It is named after the desktop novelty toy — the glass bird with the felt
head that, when wet, repeatedly tips toward a glass of water. The toy is a
heat engine: evaporation cools the upper bulb, vapor pressure shifts the
liquid up, the bird tips, the head re-wets, repeat. It's a satisfying piece
of physics on a coffee table. The software does the analogous thing in
software: it taps things at the right rhythm so they stay alive.

The capability that matters here is the multi-agent part. One keep-alive
script for one terminal is a thing every sysadmin has written. Dunking
bird drives several agents at once — different sessions, different work
queues, different long-running loops — and keeps all of them productive
without a person at any of their keyboards. That is the part that turned
out to be useful on this project.

A longer write-up of the program lives at
[`hunterdavis.com/2026/03/21/announcing-dunking-bird.html`](https://hunterdavis.com/2026/03/21/announcing-dunking-bird.html).
This page is just about why it kept showing up on this project.

## Why this project needed one

Performance work on the PS1 port is unglamorous. Most of it is the same
loop, run a thousand times: build, boot a headless DuckStation under
Docker, run a scene to [scene-end]({{ '/docs/glossary/#scene-end' | relative_url }}), capture [`JCPERF2`]({{ '/docs/glossary/#jcperf' | relative_url }}) lines,
parse them, summarize, decide whether the change beat the baseline,
record the result, change one thing, do it again. The loop is mostly
deterministic, and most iterations are uninteresting.

The full
[experiment ledger](https://github.com/huntergdavis/johnny-castaway-ps1/blob/main/docs/ps1/performance-experiment-log.md)
on the perf branch has a few hundred entries. Most of them are
single-knob probes — window size 22 KB, slack guard 4 [VBlanks]({{ '/docs/glossary/#vblank' | relative_url }}),
that kind of thing. Each individual run takes minutes. The decision
that follows takes seconds. The cumulative time spent staring at a
terminal waiting for the next "do not promote" line is real, and that
is the time that needs an automation that does not require a person
to be present.

Dunking bird does not run experiments. Other scripts do that. What it
does is keep several agents in motion at the same time — the perf
experiment driver in one session, a content-authoring agent on a
different page in another, a regtest harness wrapper somewhere else —
so I am not the rate limit on any of them. None of those sessions has
to be watched in real time. The bird taps each one in turn. They keep
going. I sleep.

## The honest limits

It is not original. Variants of this idea have been used by every
sysadmin who ever needed an SSH session to live longer than a corporate
network's idle timer. Mouse jigglers exist on Amazon for ten dollars.
The novelty here is small: I wrote my own, I named it after a glass
bird, I use it on this project a lot, and it has a tiny landing page
on my site. That is the entire surface area of the contribution.

It does not make a build faster. It does not make a perf experiment
better. It does not change what the loop measures or how the harness
gates results. The actual perf work — the
[experiment log](https://github.com/huntergdavis/johnny-castaway-ps1/blob/main/docs/ps1/performance-experiment-log.md),
the [methodology](https://github.com/huntergdavis/johnny-castaway-ps1/blob/main/docs/ps1/performance-optimization-plan.md),
the [regtest harness]({{ '/docs/regtest/' | relative_url }}) — all
exists independent of any keep-alive. Dunking bird only saves the
cost of a person sitting in a chair while the harness does its thing.

## Where it shows up on the perf branch

It is used during long
[regression-as-lifestyle]({{ '/lab/regression-as-lifestyle/' | relative_url }})
runs and during
[LLM-assisted]({{ '/lab/llm-pass/' | relative_url }}) iteration loops
where the agent is making small changes, testing them, and waiting on
results. When a session has to live across a coffee break, an errand,
or a night of sleep, the dunking bird is what keeps it intact.

It is not the reason any of the perf wins on this branch are real. The
[hallucination engineering]({{ '/lab/hallucination-engineering/' | relative_url }})
discipline, the strict acceptance gates in the harness, and the human
re-review of every promoted experiment are. The dunking bird only kept
the lights on while those happened.

## What it teaches about advanced development

Probably not much. The honest framing is: when a workflow has a long
unattended phase, the cheapest way to make it survive that phase is
often a tiny piece of software whose only job is to act like a person
sitting at a keyboard. There is no better answer in 2026 than there
was in 1996. The keep-alive script is a recurring pattern in this
craft, and the recurring pattern is worth naming, but it is not a
breakthrough.

The reason this gets a page on the site at all is that it is part of
the actual story of how the PS1 port got worked on, and the project
voice is plainspoken about how the work happens. Pretending the loops
are watched manually would be a small lie. Pretending the dunking bird
is sophisticated would be a different one. So: it's a dumb hack, it
works, and it is part of why the perf branch's experiment log got as
long as it did.

## Cross-links

- [The dev environment, photographed]({{ '/about/dev-environment/' | relative_url }}) —
  the bird mid-double-dunk, in context with the rest of the workflow.
- [AI sub-agents on this project]({{ '/docs/agents/' | relative_url }}) —
  the agents the bird keeps tapping; what they actually wrote and
  what they explicitly didn't.
- [The LLM pass]({{ '/lab/llm-pass/' | relative_url }}) — what the
  pair-programming loop actually does between dunking-bird ticks.
- [Hallucination engineering]({{ '/lab/hallucination-engineering/' | relative_url }}) —
  the discipline that makes LLM-assisted runs trustworthy at all.
- [The 24/7 build farm]({{ '/lab/build-farm/' | relative_url }}) —
  what the bird keeps fed.
- [Regression as a lifestyle]({{ '/lab/regression-as-lifestyle/' | relative_url }}) —
  the harness on the other end of the unattended loop.
- [Performance loop]({{ '/hack/performance-loop/' | relative_url }}) —
  the curious-hacker view of why the loop matters.
