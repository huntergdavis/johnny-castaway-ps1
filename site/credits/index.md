---
title: Credits
eyebrow: A labor of love
subtitle: The full acknowledgements behind a small port.
description: Acknowledgements for the Johnny Castaway PS1 fan port — Sierra On-Line for the original screensaver, the prior reverse-engineering ports (jno6809/jc_reborn, JCOS, xesf/Castaway), the toolchain authors (PSn00bSDK, DuckStation, mkpsxiso, spicyjpeg), the AI sub-agents that drafted text and emblem sprites, the four self-hosted SIL OFL fonts, and the CC0 ocean-ambience source.
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## In the game

The pause menu's Credits page reads, verbatim:

> A labor of love by Hunter Davis.
>
> Hunter does not own or have a license to the Johnny Castaway
> character. The original creator generously allows fan ports.
>
> If you paid for this, you were cheated.
> Open source and free.
> github.com/huntergdavis/johnny-castaway-ps1

That's the whole credits screen on the disc. The list below is
the longer version — the people whose work this project stands on.

## Original game

- **Sierra On-Line** — the original *Johnny Castaway* (1992),
  the character, all original art and audio. The *Castaway*
  character is theirs, not this project's. This site reflects
  that in its chrome and its [legal page]({{ '/legal/' | relative_url }}).

## Prior ports (the giants whose shoulders this stands on)

- **[`jno6809/jc_reborn`](https://github.com/jno6809/jc_reborn)**
  (Jérémie Guillaume) — the SDL2 port that decoded the original
  engine's ADS / TTM / RES bytecode. This project's host build
  is a fork of that work.
- **[`nivs1978/JCOS`](https://github.com/nivs1978/Johnny-Castaway-Open-Source)**
  (Hans Milling) — an alternate decoding effort that helped
  cross-validate frames.
- **[`xesf/Castaway`](https://github.com/xesf/castaway)**
  (Alexandre Fontoura) — additional reverse-engineering notes.
- **The Sierra Chest archive** — preserved manuals and box copy.

## Toolchain

- **PSn00bSDK** (Lameguy64 et al.) — the open-source PS1 SDK that
  makes a project like this thinkable in 2026.
- **DuckStation** (Connor McLaughlin et al.) — the emulator that
  every commit gets tested against.
- **mkpsxiso** (Lameguy64) — `.bin/.cue` packing.
- **spicyjpeg's pad-poll example** — the SPI driver used here is
  derived from that MPL-licensed code (see `src/platform/ps1/spi.c`).
- **Meeus / Jones / Butcher** — the Easter algorithm used for
  movable holidays.

## This port

- **Hunter Davis** — wrote it, broke it, fixed it again.

## AI sub-agents

LLM sub-agents — **Claude** (Anthropic), **Gemini** (Google), and
**Codex** (OpenAI) — were all used extensively across this
project. They drafted text, scaffolded code, ran parallel research
passes, and generated the holiday-emblem sprite primitives. They
do not own decisions, do not validate scenes, and do not sign off
on releases. Every agent-drafted page on this site got at least
one factual edit on review; a few got rewritten substantially.
The disclosure surface is documented at
[/docs/agents/]({{ '/docs/agents/' | relative_url }}); every
page's footer carries the short version.

## Closed captions

The caption text was authored fresh for this port from scene
content, not lifted from any prior corpus. The [caption audit]({{ site.github_url }}/blob/main/docs/ps1/caption-audit-2026-04-26.yaml)
shows the confidence level of every ADS-tag → caption mapping.

## Site typography

This site self-hosts four font families as woff2 (seven weight files
in total). All are released
under the [SIL Open Font License](https://scripts.sil.org/OFL),
which doesn't strictly require attribution but the project's voice
does:

- **Bitter** (Sol Matas, Huerta Tipográfica) — display serif, used for headings.
- **Inter** (Rasmus Andersson) — body sans-serif.
- **IBM Plex Mono** (Mike Abbink + Bold Monday for IBM) — code blocks and the build-stamp meta.
- **VT323** (Peter Hull) — pixel display font, used for eyebrows, jump-nav labels, and pager hints.

Subsetted to Latin and stored under `site/assets/fonts/`. The
full SIL OFL v1.1 license body and each family's upstream
copyright line ship beside the woff2 files at
[`LICENSE-FONTS.txt`]({{ site.github_url }}/blob/main/site/assets/fonts/LICENSE-FONTS.txt).

## Ocean ambience

The looping background ocean track shipped with `v0.6.0-ps1`
(toggle: **Pause → Accessibility → Ocean**) is sourced from
[**BigSoundBank.com** sound 0266 — "Sea: Waves"](https://bigsoundbank.com/sea-waves-s0266.html),
released under **CC0 / public domain**. The on-disc `OCEAN.VAG` is a
20-second seamless loop derived from that recording, downsampled to
11.025 kHz mono and encoded to Sony VAG ADPCM. The seam is hidden by
an equal-power crossfade with the recording's natural continuation,
so the SPU's hardware loop reads as unbroken ocean rather than a
wraparound. Encoding pipeline lives in `scratch/ocean-ambience/`;
design rationale in [background-music-feasibility.md]({{ site.github_url }}/blob/main/docs/ps1/background-music-feasibility.md).

## Related pages

- [Legal]({{ '/legal/' | relative_url }}) — the licensing
  companion to this attribution: GPL-3.0 on the project's own
  code, MPL-2.0 on the SPI driver derivation, the Sierra
  character disclaimer, the takedown procedure.
- [Voice]({{ '/about/voice/' | relative_url }}) — the editorial
  standard the credits voice anchors. Why this page reads
  plainspoken instead of marketing-speak.
- [humans.txt]({{ '/humans.txt' | relative_url }}) — the same
  attribution surface in the IndieWeb-standard humans.txt
  format, mirroring this page's content.
- [Feeds &amp; well-known endpoints]({{ '/docs/feeds/' | relative_url }})
  — the reference for every machine-readable URL on the site;
  humans.txt is one entry in a longer list that also covers
  robots.txt, RFC 9116 security.txt, the W3C web manifest, the
  Atom + JSON feeds for the devlog and lab, and the Schema.org
  JSON-LD records emitted in every page's head.
- [FAQ — Is this legal?]({{ '/faq/#is-this-legal' | relative_url }})
  — the short author-written restatement of the fan-port
  stance.
- [About — Pointing at this work]({{ '/about/#pointing-at-this-work' | relative_url }})
  — one-line web reference + BibTeX block for linking the project
  from a blog post, paper, or retro-dev podcast's notes file. The
  version pin matters; this is an active project.

---

The site is text-only on purpose — no portraits, no contributor
avatars. The [drawCredits]({{ '/docs/glossary/#drawcredits' | relative_url }}) voice is plainspoken, and the chrome
matches.
