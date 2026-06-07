---
layout: page
title: Lab
eyebrow: Magazine · Methodology and reflection
subtitle: How a one-person PS1 fan port actually gets built. Seventeen feature articles on the craft, the failures, and the tools.
description: The Lab section of the Johnny Castaway PS1 site — long-form essays on methodology, infrastructure, war stories, and the LLM-augmented dev practice behind a one-person fan port.
date: 2026-04-26
---

A labor of love by Hunter Davis. The pages collected here are not the
devlog and they are not the docs. The devlog is reverse-chronological,
unedited, dated; it is what was in my head on a particular Tuesday. The
docs are reference manuals; they tell you what `FntFlush` does and how
the regtest harness loads a BIOS. The Lab is the third thing. It's
where I write down *how the work actually gets done* — the tools, the
habits, the tricks, the things that surprised me, the things that
failed.

I've been porting Johnny Castaway to one platform or another for years
now. Dreamcast, embedded Linux on framebuffer-console boxes, RetroFW
handhelds, a receipt-printer text edition. The PS1 port is another
expression of an existing practice. What's changed about the practice
in the last six months is mostly the *pace* — sub-agents draft code and
prose in parallel, a Docker farm runs builds and regressions while I
sleep, a thirty-cent novelty bird taps a keyboard when I'm not at it.
This section is where I write all that down so I'll remember next time.

A few of these articles were drafted by AI sub-agents and then
human-edited, in the same workflow described inside them. That
disclosure is part of the methodology, not an apology for it. The
voice anchor for the whole site — the four-line [`drawCredits`]({{ '/docs/glossary/#drawcredits' | relative_url }}) text in
`src/pause_menu/pause_menu.c` — is hand-written and load-bearing; everything
downstream of it gets edited until it sounds like the same person. The
[voice-anchor article]({{ '/lab/voice-anchor-problem/' | relative_url }})
goes into how that pipeline works in practice.

If you want the practical learning path rather than the magazine treatment,
start with the [Curious Hacker's Guide]({{ '/hack/' | relative_url }}). If you
want the raw documents behind the essays, use the
[source library]({{ '/source/' | relative_url }}) and the
[resource catalog]({{ '/resources/' | relative_url }}).

Subscribe via
<a href="{{ '/lab/feed.xml' | relative_url }}" type="application/atom+xml">Atom</a>
or
<a href="{{ '/lab/feed.json' | relative_url }}" type="application/feed+json">JSON Feed</a>;
both are reverse-chronological with rich summaries (no embedded
body — `site.html_pages` doesn't pre-render content reliably the
way `site.posts` does, so the feeds are the headline-and-link
pattern). Auto-discovery is wired into every page's `<head>`, so
most feed readers find them automatically. The
[/docs/feeds/]({{ '/docs/feeds/' | relative_url }})
reference page documents every machine-readable endpoint the site
ships, including the matching pair for the
[devlog]({{ '/devlog/' | relative_url }}).

## Contents

Seventeen feature articles, in reverse chronological order. The newest
sit on top; the foundational methodology pieces sit underneath. Each
card carries the date it landed.

<ul class="doc-grid">
  <li>
    <p class="lab-date"><time datetime="2026-05-09">2026-05-09</time></p>
    <a href="{{ '/lab/63-heroes/' | relative_url }}">63 heroes</a>
    <p>How every per-scene page on the site got its own captured-on-PS1 hero image, what frame-selection rules of thumb kept reappearing, and the cross-link cluster taxonomy that emerged from writing one figcaption at a time — variant pairs, story-arc sagas, theme threads, engineering-quirk lineages. Plus the one scene the chapter-select grind missed and how the regtest harness filled it in headlessly.</p>
  </li>
  <li>
    <p class="lab-date"><time datetime="2026-05-08">2026-05-08</time></p>
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">The chapter-select grind</a>
    <p>Walking all 63 packs on hardware again to ship custom on-PS1 chapter-select thumbnails — and the surprising number of <a href="{{ '/docs/glossary/#caption-mismap' | relative_url }}">caption-mismaps</a> the loop caught in the existing scene-page metadata. The validation bar guards pixel drift, not caption drift; this loop reconciled the prose against the discs that play.</p>
  </li>
  <li>
    <p class="lab-date"><time datetime="2026-05-06">2026-05-06</time></p>
    <a href="{{ '/lab/v081-mary4-freeze/' | relative_url }}">v0.8.1: what the soak found that the matrix didn't</a>
    <p>The post-v0.8.0 stability fix as a war story. A randomized soak surfaced a MARY 4 scene-load freeze the per-commit matrix never reached. The fix is a clean-rect pressure estimator that mirrors the actual save path; the discipline is the lesson — matrix and soak are not redundant.</p>
  </li>
  <li>
    <p class="lab-date"><time datetime="2026-05-06">2026-05-06</time></p>
    <a href="{{ '/lab/from-87-to-99-5/' | relative_url }}">From 87 to 99.5: the post-validation performance loop</a>
    <p>How the headless-perf battle card moved from +17.4% over target to +0.8% after every scene was already signed off — the single biggest unlock (clean-memory-relief drop-prefetch), the methodology, the accepted experiments, and the rejected ones.</p>
  </li>
  <li>
    <p class="lab-date"><time datetime="2026-05-05">2026-05-05</time></p>
    <a href="{{ '/lab/the-site-itself/' | relative_url }}">The site itself, as a small program</a>
    <p>The path-portable build, the canonical_baseurl trick, the no-plugin Atom feed, the pager pattern shared across catalogs, and the build-stamp coarsening that kills per-commit churn. A handful of decisions that keep the deployment stable and low-noise.</p>
  </li>
  <li>
    <p class="lab-date"><time datetime="2026-04-29">2026-04-29</time></p>
    <a href="{{ '/lab/build-farm/' | relative_url }}">The 24/7 build farm</a>
    <p>Docker, multiple branches, headless DuckStation. How a one-person project keeps a build matrix running around the clock.</p>
  </li>
  <li>
    <p class="lab-date"><time datetime="2026-04-26">2026-04-26</time></p>
    <a href="{{ '/lab/the-63-scene-grind/' | relative_url }}">The 63-scene grind</a>
    <p>From five signed off to all 63. What the daily loop looked like on a project of finite scenes and finite evenings, written from inside the grind.</p>
  </li>
  <li>
    <p class="lab-date"><time datetime="2026-04-26">2026-04-26</time></p>
    <a href="{{ '/lab/pixel-perfect-pivot/' | relative_url }}">The pivot that almost didn't happen</a>
    <p>How the project nearly shipped at "looks similar" before pixel-perfect became the bar. A retrospective on the choice that defined everything else.</p>
  </li>
  <li>
    <p class="lab-date"><time datetime="2026-04-26">2026-04-26</time></p>
    <a href="{{ '/lab/two-day-spi-bug/' | relative_url }}">The two-day SPI bug</a>
    <p>A war story about the PS1 controller pad-poll TX length. DuckStation only delivers button bytes when you send the full five-byte sequence. Spicyjpeg's example is wrong about that.</p>
  </li>
  <li>
    <p class="lab-date"><time datetime="2026-04-26">2026-04-26</time></p>
    <a href="{{ '/lab/35-holidays-codegen/' | relative_url }}">35 holidays in 4 weeks: a codegen study</a>
    <p>From four shipped holidays to thirty-six in a month, via Meeus, Zeller, and a YAML file that compiles to a C struct array.</p>
  </li>
  <li>
    <p class="lab-date"><time datetime="2026-04-26">2026-04-26</time></p>
    <a href="{{ '/lab/why-i-keep-porting-johnny/' | relative_url }}">Why I keep porting Johnny Castaway</a>
    <p>Dreamcast, picture frame, embedded Linux, PS1. The same screensaver, four times. What I'm actually doing when I do it again.</p>
  </li>
  <li>
    <p class="lab-date"><time datetime="2026-04-26">2026-04-26</time></p>
    <a href="{{ '/lab/fan-port-in-public/' | relative_url }}">Building a fan port in public</a>
    <p>Disclaimers, the original creator, GPL-3.0 inheritance, "if you paid for this, you were cheated." How shipping a fan port works without lawyers in 2026.</p>
  </li>
  <li>
    <p class="lab-date"><time datetime="2026-04-26">2026-04-26</time></p>
    <a href="{{ '/lab/dunking-bird/' | relative_url }}">The dunking bird</a>
    <p>A dumb hack the author wrote that pokes multiple LLM agents in parallel so they keep working instead of going idle. Used a lot during the perf branch's long unattended runs.</p>
  </li>
  <li>
    <p class="lab-date"><time datetime="2026-04-26">2026-04-26</time></p>
    <a href="{{ '/lab/llm-pass/' | relative_url }}">The LLM pass</a>
    <p>How a one-person fan port uses AI sub-agents without lying about it. The methodology essay; the docs version is at <a href="{{ '/docs/agents/' | relative_url }}">/docs/agents/</a>.</p>
  </li>
  <li>
    <p class="lab-date"><time datetime="2026-04-26">2026-04-26</time></p>
    <a href="{{ '/lab/hallucination-engineering/' | relative_url }}">Hallucination engineering</a>
    <p>Mitigating LLM-driven dev's specific failure mode: confident wrong answers. War stories, mitigations, what the year of practice teaches.</p>
  </li>
  <li>
    <p class="lab-date"><time datetime="2026-04-26">2026-04-26</time></p>
    <a href="{{ '/lab/regression-as-lifestyle/' | relative_url }}">Regression as a lifestyle</a>
    <p>Continuous regtest is not a CI feature. On this project it's a way of working — frame-by-frame screenshot diffs are the heartbeat.</p>
  </li>
  <li>
    <p class="lab-date"><time datetime="2026-04-26">2026-04-26</time></p>
    <a href="{{ '/lab/voice-anchor-problem/' | relative_url }}">The voice anchor problem</a>
    <p>Keeping a project's voice consistent when half the prose was drafted by AI sub-agents. The drawCredits text, the voice guide, the audit pass.</p>
  </li>
</ul>

## What this section is not

It's not a contributor onboarding. It's not a tutorial series. It's not
content marketing for the project. There is no "in this section we'll
explore" framing, no roadmap of future articles, no call to action at
the bottom. It is the methodology I would write up if a future me were
porting Johnny Castaway again to a fifth platform and wanted to know
what worked, and that future me is the primary reader. You're welcome
to read along.

For the live status of the port, see
[/about/status/]({{ '/about/status/' | relative_url }}). For the
day-by-day worklogs, see
[/devlog/]({{ '/devlog/' | relative_url }}). For reference docs, see
[/docs/]({{ '/docs/' | relative_url }}). For a guided path through the
codebase, see [/hack/]({{ '/hack/' | relative_url }}).
