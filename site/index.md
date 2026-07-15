---
layout: home
title: Johnny Castaway PS1
description: A ground-up PlayStation 1 port of Sierra's 1992 Johnny Castaway screensaver. Hybrid host-and-replay pipeline. Open source, GPL-3.0.
---

<section class="hero" aria-labelledby="hero-title">
  <div class="hero-frame">
    {%- comment -%}
      <picture> wraps the hero so modern browsers pull the lossless
      WebP (73 KB) instead of the PNG (147 KB) — same pixels, half
      the bytes. The fallback <img> keeps the PNG for browsers that
      don't recognize <source type="image/webp"> (Chrome <32, Safari
      <14, Firefox <65, IE). The preload in _includes/head.html
      points at the WebP with type="image/webp" so modern browsers
      preload the file they will actually use; older browsers skip
      that preload (type mismatch) and fetch the PNG normally via
      the fallback path.
    {%- endcomment -%}
    <picture>
      <source type="image/webp"
              srcset="{{ '/assets/img/johnny6-ps1-date-dream.webp' | relative_url }}" />
      <img src="{{ '/assets/img/johnny6-ps1-date-dream.png' | relative_url }}"
           width="1127" height="677"
           fetchpriority="high"
           decoding="async"
           alt="JOHNNY 6 running on PS1: Johnny dreams about his island date." />
    </picture>
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

{%- comment -%}
  Trio of below-fold scene captures, each wrapped in <picture> so
  modern browsers pull the lossless WebP (~half the bytes) and
  older browsers fall through to the PNG. Same pattern as the hero
  picture element above. loading="lazy" keeps each below-fold fetch
  deferred until layout reaches it; the <img> retains the width/
  height/alt attributes so CLS and screen-reader behavior are
  unchanged. Combined trio drops from ~889 KB PNG to ~473 KB WebP
  for browsers that accept it (~47% reduction).
{%- endcomment -%}
<section class="trio" aria-label="Selected PS1 captures">
  <figure>
    <a href="{{ '/scenes/activity9/' | relative_url }}">
      <picture>
        <source type="image/webp" srcset="{{ '/assets/img/activity9-ps1-boat.webp' | relative_url }}" />
        <img src="{{ '/assets/img/activity9-ps1-boat.png' | relative_url }}" width="1127" height="677" loading="lazy" decoding="async" alt="ACTIVITY 9 running on PS1: Johnny rain-dances while a boat carrying a couple passes the island." />
      </picture>
      <figcaption>ACTIVITY 9 · rain dance, boat passes</figcaption>
    </a>
  </figure>
  <figure>
    <a href="{{ '/scenes/johnny1/' | relative_url }}">
      <picture>
        <source type="image/webp" srcset="{{ '/assets/img/johnny1-ps1-frog-clock.webp' | relative_url }}" />
        <img src="{{ '/assets/img/johnny1-ps1-frog-clock.png' | relative_url }}" width="1127" height="677" loading="lazy" decoding="async" alt="Frog clock loading transition running on PS1, between two scenes." />
      </picture>
      <figcaption>JOHNNY 1 · frog clock</figcaption>
    </a>
  </figure>
  <figure>
    <a href="{{ '/scenes/fishing1/' | relative_url }}">
      <picture>
        <source type="image/webp" srcset="{{ '/assets/img/fishing1-ps1-cast.webp' | relative_url }}" />
        <img src="{{ '/assets/img/fishing1-ps1-cast.png' | relative_url }}" width="1127" height="677" loading="lazy" decoding="async" alt="FISHING 1 running on PS1: Johnny casts a fishing line off the island, sun overhead, palm tree in frame." />
      </picture>
      <figcaption>FISHING 1 · reference</figcaption>
    </a>
  </figure>
</section>

<figure style="margin: 2rem auto; max-width: 24rem; border: 3px solid var(--frame-out); outline: 1px solid var(--frame-in); background: var(--jc-blue-deep);" aria-label="Scene Explorer running on PS1">
  <a href="{{ '/docs/pause-menu/#scene-explorer' | relative_url }}">
    <picture>
      <source type="image/webp" srcset="{{ '/assets/img/scene-explorer-fishing5.webp' | relative_url }}" />
      <img src="{{ '/assets/img/scene-explorer-fishing5.png' | relative_url }}"
           width="640" height="448"
           loading="lazy"
           decoding="async"
           style="display: block; width: 100%; height: auto;"
           alt="Scene Explorer on PS1: top band reads SCENE EXPLORER, then cursor position 5/63 with a validated marker, then FISHING 5 — Eaten by a shark, then Family Fishing with 69 frames; the captured-on-PS1 thumbnail of FISHING 5 (shark on the right side of the island chewing Johnny) sits in the middle; bottom band reads Pack FG/FISHING5.FG2 with navigation hints LEFT/RIGHT scene, L1/R1 family, X play, Triangle loop, O back." />
    </picture>
  </a>
</figure>

<aside class="status-strip" aria-label="Project status">
  <span><strong>Latest:</strong> <a href="{{ '/releases/' | relative_url }}">{{ site.release.tag }}</a> <small>· <time datetime="{{ site.release.release_date }}">{{ site.release.release_date }}</time></small></span>
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
running on [real hardware]({{ '/play/#real-ps1-hardware' | relative_url }}) or on [DuckStation]({{ '/docs/glossary/#duckstation' | relative_url }}). It is a fan project: the
character belongs to the original creator, and the site's chrome and
its [legal page]({{ '/legal/' | relative_url }}) reflect that.

All **{{ site.release.scenes_validated }} of {{ site.release.scenes_total }}**
routed scenes the original game had are validated under the
[FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }})
— pixel-perfect visuals plus synced SFX, signed off by human
review across every applicable variant. Current mainline work is
bugfixing, performance, and feature polish; the
[`v0.8.1` line]({{ '/lab/v081-mary4-freeze/' | relative_url }})
keeps long randomized runs stable, the [`v0.8.0` performance
baseline]({{ '/releases/#v080-ps1--complete-scene-performance-baseline' | relative_url }})
promoted the headless optimization methodology, the
[`v0.8.2`]({{ '/releases/#v082-ps1--visitor3-guarded-read-performance' | relative_url }}) + [`v0.8.3`]({{ '/releases/#v083-ps1--walkstuf1-compact-foreground-performance' | relative_url }}) follow-ons closed the VISITOR3 and WALKSTUF1
outliers, [`v0.8.4`]({{ '/releases/#v084-ps1--custom-chapter-select-thumbnails-for-all-63-scenes' | relative_url }}) walked all 63 packs on hardware to ship custom
chapter-select thumbnails plus a scene-page reconciliation against the
on-PS1 packs, [`v0.8.5`]({{ '/releases/#v085-ps1--full-126-row-headless-performance-matrix' | relative_url }}) promotes the full 126-row timing-bearing
matrix, [`v0.8.6`]({{ '/releases/#v086-ps1--walkstuf1--visitor3-setup-segment-compaction-follow-through' | relative_url }}) lands the WALKSTUF1 / VISITOR3 setup-segment
compaction follow-through, [`v0.8.7`]({{ '/releases/#v087-ps1--deterministic-bootmode-scene-selection--scene-explorer-preview-stability' | relative_url }}) hardens deterministic
scene booting plus Scene Explorer preview loading, [`v0.8.8`]({{ '/releases/#v088-ps1--visitor5-high-retained-read-promotion' | relative_url }}) promotes VISITOR5 high into green, [`v0.8.9`]({{ '/releases/#v089-ps1--walkstuf1-low-in-place-payload-reductions' | relative_url }}) promotes VISITOR5 low plus the first W1-low in-place payload lane, [`v0.8.10`]({{ '/releases/#v0810-ps1--walkstuf1-low-no-shift-payload-follow-through' | relative_url }}) carries that no-shift WALKSTUF1 low baseline through frame `76` with active payload `879801 -> 801103`, [`v0.8.11`]({{ '/releases/#v0811-ps1--lazy-stream-buffer-release-regression-fix' | relative_url }}) restores lazy stream allocation after the release-merge heap pin briefly broke W1-low clean-rect allocation, and [`v0.8.12`]({{ '/releases/#v0812-ps1--walkstuf1-low-frame77frame130-payload-trims' | relative_url }}) extends the W1-low no-shift lane through frames `77` and `130` with active payload `879801 -> 799694` — the public battle card now averages
[`{{ site.release.perf_target_speed_pct }}%` target speed]({{ '/docs/glossary/#target-speed' | relative_url }})
across the timing-bearing rows (public-capped; the
optimization-side raw signed average is past target). The two
ledgers live separately: the
[scene ledger]({{ '/scenes/' | relative_url }}) tracks visual
signoff and the
[performance battle card]({{ '/perf/' | relative_url }}) tracks
headless DuckStation timing for every scene/tide variant.

## How it works (the short version)

A *[host build]({{ '/docs/glossary/#host-build' | relative_url }})* of the original engine plays each scene under capture
mode and dumps **[FG2 packs]({{ '/docs/glossary/#fg2-pack' | relative_url }})** — small binary files that record every
visible draw, every sound trigger, every frame timing. The PS1 build
loads those packs from the disc and replays them against its own
background, wave animation, holiday overlay, captions, and SPU audio.

The PS1 never interprets Sierra's bytecode at runtime. That's the whole
trick. A 1992 screensaver fits onto a CD-ROM and inside [2 MB of RAM]({{ '/docs/hardware/' | relative_url }})
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
    <p>Run it <a href="{{ '/play/online/' | relative_url }}">straight in your browser</a> — or grab the latest <code>.bin</code> + <code>.cue</code> with the DuckStation quickstart and controller map. Five minutes.</p>
  </li>
  <li>
    <a href="{{ '/faq/' | relative_url }}">FAQ</a>
    <p>Author-written answers to the recurring questions: what this is, why PS1, is this legal, can I sponsor or donate, do I need Sierra files, does it run at native rate, where do I file bugs.</p>
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
    <p>The other bar: 126 scene/tide variants timed against target frame budget. Sortable headers, color-coded Target Speed (≥99% green, ≥95% yellow, ≥90% orange, &lt;90% red). Currently averaging {{ site.release.perf_target_speed_pct }}% target speed.</p>
  </li>
  <li>
    <a href="{{ '/archaeology/' | relative_url }}">The full story</a>
    <p>Five chapters: 1992 Sierra, the prior reverse-engineering ports, the false starts, and the hybrid pivot.</p>
  </li>
  <li>
    <a href="{{ '/lab/' | relative_url }}">Magazine-length Lab</a>
    <p>Seventeen feature essays: the per-scene hero rollout retrospective, post-validation perf retrospectives, the soak loop and the v0.8.1 freeze, the chapter-select grind, the 63-scene grind, regression-as-lifestyle, the pixel-perfect pivot, the two-day SPI bug, the site as a small program, voice + hallucination engineering, the LLM pass, the build farm, the dunking bird, why this is the fifth port, holiday codegen, and what fan-porting in public looks like.</p>
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
    <p>Twenty reference manuals: build, captions, holidays, pause menu, freeplay, story-loop walks, regtest, scripted input, performance, hardware, devices, audio, infrastructure, file formats, AI sub-agents, vision-classifier, the SDL2 → PSn00bSDK API mapping, dev workflow, feeds &amp; well-known endpoints, and a glossary.</p>
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
Set, Scene Explorer (the chapter-select grid with on-PS1-captured
thumbnails for every scene), Freeplay Options, Controls, World
Options, Holidays, Set Island Position, Accessibility, Sound Test,
System, Set Time/Date, and Set RNG Seed. An optional
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
