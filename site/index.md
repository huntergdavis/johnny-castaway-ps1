---
layout: home
title: Johnny Castaway PS1
description: A ground-up PlayStation 1 port of Sierra's 1992 Johnny Castaway screensaver. Hybrid host-and-replay pipeline. Open source, GPL-3.0.
---

<section class="hero" aria-labelledby="hero-title">
  <div class="hero-frame">
    <img src="{{ '/assets/img/johnny6-ps1-date-dream.png' | relative_url }}"
         width="1127" height="677"
         fetchpriority="high"
         decoding="async"
         alt="JOHNNY 6 running on PS1: Johnny dreams about his island date." />
  </div>
  <div class="hero-text">
    <p class="hero-eyebrow">A fan port · v{{ site.release.version }}</p>
    <h1 id="hero-title">{{ site.title }}</h1>
    <p class="tagline">{{ site.tagline }}</p>
    <div class="hero-cta">
      <a class="btn btn--primary" href="{{ '/play/' | relative_url }}">Download &amp; play</a>
      <a class="btn" href="{{ '/about/' | relative_url }}">How it works</a>
      <a class="btn-tertiary" href="{{ '/play/#emulator' | relative_url }}">Need an emulator?</a>
    </div>
  </div>
</section>

<section class="trio" aria-label="Selected PS1 captures">
  <figure>
    <img src="{{ '/assets/img/activity9-ps1-boat.png' | relative_url }}" width="1127" height="677" loading="lazy" decoding="async" alt="ACTIVITY 9 running on PS1: Johnny bathes while a boat passes the island." />
    <figcaption><a href="{{ '/scenes/activity9/' | relative_url }}">ACTIVITY 9 · boat pass</a></figcaption>
  </figure>
  <figure>
    <img src="{{ '/assets/img/johnny1-ps1-frog-clock.png' | relative_url }}" width="1127" height="677" loading="lazy" decoding="async" alt="Frog clock loading transition running on PS1, between two scenes." />
    <figcaption><a href="{{ '/scenes/johnny1/' | relative_url }}">JOHNNY 1 · frog clock</a></figcaption>
  </figure>
  <figure>
    <img src="{{ '/assets/img/fishing1-ps1-cast.png' | relative_url }}" width="1127" height="677" loading="lazy" decoding="async" alt="Johnny casting a fishing line off the island, sun overhead, palm tree in frame." />
    <figcaption><a href="{{ '/scenes/fishing1/' | relative_url }}">FISHING 1 · reference</a></figcaption>
  </figure>
</section>

<aside class="status-strip" aria-label="Project status">
  <span><strong>Latest:</strong> <a href="{{ '/releases/' | relative_url }}">{{ site.release.tag }}</a></span>
  <a class="pill pill--ok" href="{{ '/scenes/' | relative_url }}">{{ site.release.scenes_validated }} / {{ site.release.scenes_total }} scenes validated</a>
  <a class="pill pill--ok" href="{{ '/perf/' | relative_url }}">{{ site.release.perf_target_speed_pct }}% target speed</a>
  <a class="pill pill--info" href="{{ '/docs/build/' | relative_url }}">PSn00bSDK 0.24</a>
  <a class="pill pill--info" href="{{ '/play/#emulator' | relative_url }}">DuckStation</a>
  <a class="pill pill--info" href="{{ '/docs/holidays/' | relative_url }}">36 holidays</a>
  <a class="pill pill--info" href="{{ '/legal/' | relative_url }}">GPL-3.0</a>
</aside>

<section class="page" markdown="1">

## What this is

Sierra's *Johnny Castaway* (1992) is a screensaver about a man stranded
on a tiny island. It runs in tiny vignettes — a fishing line, a passing
ship, a holiday decoration — quietly, all day.

This is a port of those vignettes to the original Sony PlayStation,
running on real hardware or on DuckStation. It is a fan project: the
character belongs to the original creator, and the site's chrome and
its [legal page]({{ '/legal/' | relative_url }}) reflect that.

All **{{ site.release.scenes_validated }} of {{ site.release.scenes_total }}**
routed scenes the original game had are validated under the
[FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }})
— pixel-perfect visuals plus synced SFX, signed off by human
review across every applicable variant. Current mainline work is
bugfixing, performance, and feature polish; the
[`v0.8.1` line]({{ '/lab/v081-mary4-freeze/' | relative_url }})
keeps long randomized runs stable while the `v0.8.0` performance
baseline closes the last bit of speed gap, and most scenes already
run at native rate. The two ledgers live separately: the
[scene ledger]({{ '/scenes/' | relative_url }}) tracks visual
signoff and the
[performance battle card]({{ '/perf/' | relative_url }}) tracks
headless DuckStation timing for every scene/tide variant.

## How it works (the short version)

A *[host build]({{ '/docs/glossary/#host-build' | relative_url }})* of the original engine plays each scene under capture
mode and dumps **FG2 packs** — small binary files that record every
visible draw, every sound trigger, every frame timing. The PS1 build
loads those packs from the disc and replays them against its own
background, wave animation, holiday overlay, captions, and SPU audio.

The PS1 never interprets Sierra's bytecode at runtime. That's the whole
trick. A 1992 screensaver fits onto a CD-ROM and inside 2 MB of RAM
because all of the smart work is done on a desktop and pre-baked.

The full deep-dive — pack format, hardware gotchas, the [SPI pad-poll
fix that cost two days]({{ '/lab/two-day-spi-bug/' | relative_url }}), the [dirty-rect bookkeeping]({{ '/docs/glossary/#dirty-rect' | relative_url }}) that wiped framebuffers
on resume — lives at **[/about/method/]({{ '/about/method/' | relative_url }})**.

</section>

## Where to start

<ul class="doc-grid">
  <li>
    <a href="{{ '/help/' | relative_url }}">Help guide</a>
    <p>Controls, menu screenshots, and the scripted-input test path that proves the menu still works headlessly.</p>
  </li>
  <li>
    <a href="{{ '/play/' | relative_url }}">Just play it</a>
    <p>Latest <code>.bin</code> + <code>.cue</code>, DuckStation quickstart, controller map. Five minutes.</p>
  </li>
  <li>
    <a href="{{ '/faq/' | relative_url }}">FAQ</a>
    <p>Author-written answers to the recurring questions: what this is, why PS1, is this legal, do I need Sierra files, does it run at native rate, where do I file bugs.</p>
  </li>
  <li>
    <a href="{{ '/about/method/' | relative_url }}">How the port works</a>
    <p>The hybrid pipeline, the pack format, hardware constraints, and the things that broke on the way.</p>
  </li>
  <li>
    <a href="{{ '/scenes/' | relative_url }}">Live scene ledger</a>
    <p>All 63 scenes with visual-signoff status, per-scene case studies, and a family jump nav. The visual bar.</p>
  </li>
  <li>
    <a href="{{ '/perf/' | relative_url }}">Headless-perf battle card</a>
    <p>The other bar: 126 scene/tide variants timed against target frame budget. Sortable headers, color-coded Target Speed (≥99% green, ≥80% yellow, &lt;80% red). Currently averaging {{ site.release.perf_target_speed_pct }}% target speed.</p>
  </li>
  <li>
    <a href="{{ '/archaeology/' | relative_url }}">The full story</a>
    <p>Five chapters: 1992 Sierra, the prior reverse-engineering ports, the false starts, and the hybrid pivot.</p>
  </li>
  <li>
    <a href="{{ '/lab/' | relative_url }}">Magazine-length Lab</a>
    <p>Fifteen feature essays: post-validation perf retrospectives, the soak loop and the v0.8.1 freeze, the 63-scene grind, the pixel-perfect pivot, the two-day SPI bug, the site as a small program, voice + hallucination engineering, the build farm, the dunking bird, why this is the fifth port, holiday codegen, and what fan-porting in public looks like.</p>
  </li>
  <li>
    <a href="{{ '/hack/' | relative_url }}">Curious hacker path</a>
    <p>For readers who want to learn C, port Johnny to another target, or understand the debugging loops that unlocked the PS1 build.</p>
  </li>
  <li>
    <a href="{{ '/devlog/' | relative_url }}">Unedited devlog</a>
    <p>The dated worklogs that drove each phase. Verbatim, no hindsight, dead ends preserved.</p>
  </li>
  <li>
    <a href="{{ '/docs/' | relative_url }}">Reference docs</a>
    <p>Eighteen reference manuals: build, captions, holidays, pause menu, freeplay, story-loop walks, regtest, scripted input, performance, hardware, audio, infrastructure, file formats, AI sub-agents, vision-classifier, the SDL2 → PSn00bSDK API mapping, dev workflow, and a glossary.</p>
  </li>
  <li>
    <a href="{{ '/source/' | relative_url }}">Source library</a>
    <p>Every Markdown file outside the site wrapped into a public shelf page, from current manuals to old research fossils.</p>
  </li>
  <li>
    <a href="{{ '/resources/' | relative_url }}">Resource catalog</a>
    <p>Bitmap, ADS, TTM, sound, sprite-bank, and foreground-pack inventory with direct source links.</p>
  </li>
  <li>
    <a href="{{ '/about/dev-environment/' | relative_url }}">The dev environment, photographed</a>
    <p>One screenshot, six windows: the Dunking Bird auto-poker, the fresh editor, two LLM sub-agents (Claude + Codex), DuckStation running the latest build, and bottom-monitor telemetry — KDE Plasma on KDE Neon.</p>
  </li>
</ul>

<section class="page" markdown="1">

## What's faithful, what's added

**Faithful to 1992.** Every scene the original game has, in original
order, with the same variants the original randomized between. The art
is unchanged. The 4 original holiday decorations (Christmas, New Year,
Halloween, St. Patrick's Day) keep their original sprites.

**Added on top.** [Story-loop walking]({{ '/releases/#v0420-ps1--story-loop-walking' | relative_url }})
between scenes — Johnny no longer teleports; he walks the
original Sierra route table from one scene's end to the next
scene's start, with palm-tree occlusion and ocean animation
preserved across the walk.
[Freeplay/debug mode]({{ '/releases/#v050-ps1--freeplay-and-debug-mode' | relative_url }}),
where Johnny can be walked directly with the controller and
debug-selected gags, visitors, sound effects, holidays, tide,
raft, and day/night state. Closed captions for every scene (off
by default, in a [fresh-authored corpus]({{ '/docs/captions/' | relative_url }})
from scene content — not lifted from any prior project). A
[holiday calendar expanded from 4 to **36 holidays**]({{ '/docs/holidays/' | relative_url }})
with movable feasts computed by pure algorithm — Meeus for Easter,
Nth-weekday-of-month for the rest, no expiring date tables. A
[pause menu]({{ '/docs/pause-menu/' | relative_url }}) reachable
with Start (the original had none), with sub-screens for Scene
Set, Freeplay Options, Controls, World Options, Holidays, Set
Island Position, Accessibility, Sound Test, System, Set Time/Date,
and Set RNG Seed. An optional
[ocean-ambience loop]({{ '/releases/#v060-ps1--ocean-ambience' | relative_url }})
on a dedicated SPU voice. [Frog-clock loading transitions]({{ '/docs/glossary/#frog-clock' | relative_url }}) between
scene swaps. The website
[credits]({{ '/credits/' | relative_url }}) and
[legal]({{ '/legal/' | relative_url }}) pages name exactly what's
owed to whom.

The full menu of what's added vs preserved lives at
[/about/]({{ '/about/' | relative_url }}). The implementations live at
[/docs/]({{ '/docs/' | relative_url }}), the complete documentation shelf is
[/source/]({{ '/source/' | relative_url }}), and the runtime assets are indexed
at [/resources/]({{ '/resources/' | relative_url }}).

## The shortest possible welcome

The project's credits text reads, verbatim:

> A labor of love by Hunter Davis.
>
> Hunter does not own or have a license to the Johnny Castaway character.
> The original creator generously allows fan ports.
>
> If you paid for this, you were cheated.
> Open source and free.
> github.com/huntergdavis/johnny-castaway-ps1

That text is the voice of the project credits. It is also the voice of
this whole site. There's no marketing copy, no "experience the magic,"
no Patreon banner above the fold. It's a small project about a small
man on a small island. The disc plays. That's what mattered.

</section>

## Latest from the lab

The Lab is the magazine — feature-length retrospectives on the
methodology, the war stories, and the choices that defined the
work. Five most recent, newest first:

{% assign lab_pages = site.pages | where_exp: "p", "p.url contains '/lab/'" | where_exp: "p", "p.url != '/lab/'" | where_exp: "p", "p.date" | sort: "date" | reverse %}

<ul class="devlog-list">
  {% for article in lab_pages limit:5 %}
  {%- assign a_words = article.content | strip_html | number_of_words -%}
  {%- assign a_min = a_words | divided_by: 250 -%}
  {%- if a_min < 1 %}{%- assign a_min = 1 -%}{% endif %}
  <li>
    <time datetime="{{ article.date | date: '%Y-%m-%d' }}">{{ article.date | date: "%Y-%m-%d" }}</time>
    <div>
      <a href="{{ article.url | relative_url }}">{{ article.title }}</a>
      {% if article.subtitle %}<span class="summary">{{ article.subtitle }}</span>{% endif %}
      <span class="summary devlog-read-time">~{{ a_min }} min read · {{ a_words }} words</span>
    </div>
  </li>
  {% endfor %}
</ul>

<p>
  <a class="btn btn--small" href="{{ '/lab/' | relative_url }}">Browse all lab articles &rarr;</a>
</p>

## Latest from the devlog

The devlog is the verbatim worklog — what was in the author's
head on a particular day, with the dead ends preserved. Five
most recent posts, newest first:

<ul class="devlog-list">
  {% for post in site.posts limit:5 %}
  {%- assign p_words = post.content | strip_html | number_of_words -%}
  {%- assign p_min = p_words | divided_by: 250 -%}
  {%- if p_min < 1 %}{%- assign p_min = 1 -%}{% endif %}
  <li>
    <time datetime="{{ post.date | date: '%Y-%m-%d' }}">{{ post.date | date: "%Y-%m-%d" }}</time>
    <div>
      <a href="{{ post.url | relative_url }}">{{ post.title }}</a>
      {% if post.editor_note %}<span class="summary">{{ post.editor_note }}</span>{% endif %}
      <span class="summary devlog-read-time">~{{ p_min }} min read · {{ p_words }} words</span>
    </div>
  </li>
  {% else %}
  <li>
    <time datetime="2026-04-26">2026-04-26</time>
    <div>
      <a href="{{ '/devlog/' | relative_url }}">Devlog index</a>
      <span class="summary">Worklogs land here as they get wrapped from <code>docs/ps1/research/</code>.</span>
    </div>
  </li>
  {% endfor %}
</ul>

<p>
  <a class="btn btn--small" href="{{ '/devlog/' | relative_url }}">Browse all worklogs &rarr;</a>
</p>

<section class="page" markdown="1">

## The disclaimer, plainly

*Johnny Castaway*, the character, the screensaver, and the original
Sierra art and audio are © Sierra On-Line and not licensed under GPL.
This project ships only the code that drives the port. The released
`.bin` / `.cue` contains pre-baked playback packs — derived data, no
Sierra source files. The host build, used in development, requires
the original Sierra data files (`RESOURCE.MAP`, `RESOURCE.001`) which
the user supplies themselves.

Full text at [/legal/]({{ '/legal/' | relative_url }}).

</section>
