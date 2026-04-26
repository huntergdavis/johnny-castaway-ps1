---
layout: home
title: Johnny Castaway PS1
description: A ground-up PlayStation 1 port of Sierra's Johnny Castaway screensaver.
---

<section class="hero">
  <div class="hero-frame">
    <img src="{{ '/assets/img/fishing1-ps1-cast.png' | relative_url }}"
         alt="Johnny stands on the island fishing while a Sierra-style cloud drifts past, rendered on a PS1." />
  </div>
  <div class="hero-text">
    <p class="hero-eyebrow">A fan port · v{{ site.release.version }}</p>
    <h1>{{ site.title }}</h1>
    <p class="tagline">{{ site.tagline }}</p>
    <div class="hero-cta">
      <a class="btn btn--primary" href="{{ '/play/' | relative_url }}">Download &amp; play</a>
      <a class="btn" href="{{ '/about/' | relative_url }}">How it works</a>
      <a class="btn-tertiary" href="{{ '/play/#emulator' | relative_url }}">Need an emulator?</a>
    </div>
  </div>
</section>

<section class="trio">
  <figure>
    <img src="{{ '/assets/img/fishing1-ps1-cast.png' | relative_url }}" alt="Johnny casting a line." />
    <figcaption>FISHING 1 · cast</figcaption>
  </figure>
  <figure>
    <img src="{{ '/assets/img/fishing1-ps1-raft.png' | relative_url }}" alt="A life raft drifts past the island." />
    <figcaption>FISHING 2 · raft</figcaption>
  </figure>
  <figure>
    <img src="{{ '/assets/img/fishing1-ps1-night.png' | relative_url }}" alt="The island at night with a starry sky." />
    <figcaption>night</figcaption>
  </figure>
</section>

<aside class="status-strip" aria-label="Project status">
  <span><strong>Latest:</strong> {{ site.release.tag }}</span>
  <span class="pill pill--ok">{{ site.release.scenes_validated }} / {{ site.release.scenes_total }} scenes validated</span>
  <span class="pill pill--info">PSn00bSDK</span>
  <span class="pill pill--info">DuckStation</span>
  <span class="pill pill--info">GPL-3.0</span>
</aside>

<section class="page" markdown="1">

## What this is

Sierra's *Johnny Castaway* (1992) is a screensaver about a man stranded
on a tiny island. It runs in tiny vignettes — a fishing line, a passing
ship, a holiday decoration — quietly, all day.

This is a port of those vignettes to the original Sony PlayStation,
running on real hardware (or DuckStation). It's a fan project: the
character belongs to the original creator, and this site reflects that
in its chrome and its disclaimer.

It's also incomplete on purpose. Out of 63 scenes, **{{ site.release.scenes_validated }}
are validated** as of {{ site.release.tag }}. The [scene ledger]({{ '/scenes/' | relative_url }})
shows what's done, what's pending, and what "validated" actually means here.

## How it works (the short version)

A host build of the original engine plays each scene under capture
mode and dumps **packs** — small binary files that record what
the scene drew, when, and against what background. The PS1 port
plays back those packs against the same backgrounds. No interpreter
on the PS1, no SDL2 dependency — just GPU primitives, a CD-ROM, and
a bit of stubborn typing.

The full deep dive lives at **[/about/method/]({{ '/about/' | relative_url }})**.

## The shortest possible welcome

> A labor of love by Hunter Davis.
>
> Hunter does not own or have a license to the Johnny Castaway character.
> The original creator generously allows fan ports.
>
> If you paid for this, you were cheated.
> Open source and free.

That text ships inside the game's pause-menu Credits. It's also the
voice of this whole site.

</section>
