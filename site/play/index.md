---
title: Play
eyebrow: Download · Run
subtitle: Latest build, quickstart, controls.
description: Download the latest Johnny Castaway PS1 fan port — .bin / .cue pair, DuckStation quickstart, controller map, freeplay controls, and the smoke-test path on real PS1 hardware via TonyHax.
---

{%- comment -%}
  Schema.org HowTo structured data for the DuckStation quickstart.
  Maps 1:1 to the prose steps in the "## Quickstart (DuckStation)"
  section below. Google + agent crawlers consume HowTo for "how do
  I run X" intents.

  Steps are hand-mirrored next to the prose they describe — same
  co-location discipline as the FAQPage on /faq/, Dataset on /perf/,
  ItemList on /scenes/. If the quickstart steps change, this block
  updates in the same commit.

  site_root and quickstart_url match the canonical_baseurl pattern
  used everywhere else so absolute URLs survive `--baseurl ""`.
{%- endcomment -%}
{%- assign play_site_root = site.url | append: site.canonical_baseurl -%}
{%- assign play_quickstart_url = play_site_root | append: '/play/#emulator' -%}
<script type="application/ld+json">
{
  "@context": "https://schema.org",
  "@type": "HowTo",
  "name": "How to play Johnny Castaway PS1 in DuckStation",
  "description": {{ "Download the .bin / .cue pair from the latest release, open the .cue in DuckStation, and let the screensaver idle. No menu to begin — the game IS the screensaver." | jsonify }},
  "totalTime": "PT2M",
  "url": {{ play_quickstart_url | jsonify }},
  "inLanguage": "en",
  "tool": [
    { "@type": "HowToTool", "name": "DuckStation emulator", "url": "https://www.duckstation.org/" },
    { "@type": "HowToTool", "name": "PlayStation 1 BIOS file (e.g. scph1001.bin) — required by DuckStation, not shipped by this project" }
  ],
  "supply": [
    { "@type": "HowToSupply", "name": "johnnycastawayps1.bin (~76 MiB disc image)" },
    { "@type": "HowToSupply", "name": "johnnycastawayps1.cue (71 B cue sheet)" }
  ],
  "step": [
    {
      "@type": "HowToStep",
      "position": 1,
      "name": "Install DuckStation",
      "text": "Install DuckStation on your platform of choice from duckstation.org.",
      "url": {{ play_quickstart_url | jsonify }}
    },
    {
      "@type": "HowToStep",
      "position": 2,
      "name": "Place files",
      "text": "Drop johnnycastawayps1.cue and johnnycastawayps1.bin into the same folder.",
      "url": {{ play_quickstart_url | jsonify }}
    },
    {
      "@type": "HowToStep",
      "position": 3,
      "name": "Open the disc image",
      "text": "In DuckStation: File → Start File… → pick johnnycastawayps1.cue.",
      "url": {{ play_quickstart_url | jsonify }}
    },
    {
      "@type": "HowToStep",
      "position": 4,
      "name": "Watch",
      "text": "Hit start. The game runs as a screensaver — let it idle and the scenes will cycle on their own. There is no menu to begin; the game is the screensaver.",
      "url": {{ play_quickstart_url | jsonify }}
    }
  ]
}
</script>

{%- comment -%}
  Schema.org SoftwareSourceCode. The home page emits SoftwareApplication
  describing the playable artifact; /play/ is also the install entrypoint
  for the source-code repository — `git clone` builds the same disc image
  the .bin/.cue download serves. SourceCode is the Schema.org type for
  source-code-hosting pages, distinct from SoftwareApplication's
  playable-artifact framing. Both records are accurate facets of this
  project.

  programmingLanguage = C (the runtime is C against PSn00bSDK; host
  build pipeline scripts are Python but not the shipped artifact).
  runtimePlatform names the actual target. codeRepository + author +
  license + version mirror the same canonical values the home-page
  SoftwareApplication block uses, so JSON-LD consumers that merge
  records by URL get consistent author/license/version triples.
{%- endcomment -%}
{%- assign play_repo_url = site.github_url -%}
<script type="application/ld+json">
{
  "@context": "https://schema.org",
  "@type": "SoftwareSourceCode",
  "name": {{ site.title | jsonify }},
  "description": {{ "Source code for the Johnny Castaway PS1 fan port. C against PSn00bSDK, build pipeline in Docker, regtest harness in headless DuckStation. Open source under GPL-3.0." | jsonify }},
  "url": {{ play_site_root | append: '/play/' | jsonify }},
  "codeRepository": {{ play_repo_url | jsonify }},
  "programmingLanguage": "C",
  "runtimePlatform": "Sony PlayStation",
  "targetProduct": {{ play_repo_url | append: '/releases/tag/' | append: site.release.tag | jsonify }},
  "license": "https://www.gnu.org/licenses/gpl-3.0.html",
  "isAccessibleForFree": true,
  "version": {{ site.release.tag | jsonify }},
  {%- if site.release.release_date %}
  "dateModified": {{ site.release.release_date | jsonify }},
  {%- endif %}
  "author": {
    "@type": "Person",
    "name": {{ site.author | jsonify }},
    "url": "https://hunterdavis.com/"
  },
  "isPartOf": {
    "@type": "WebSite",
    "name": {{ site.title | jsonify }},
    "url": {{ play_site_root | append: '/' | jsonify }}
  }
}
</script>

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## Latest build

**{{ site.release.tag }}** ({%- if site.release.release_date -%}released <time datetime="{{ site.release.release_date }}">{{ site.release.release_date }}</time>{%- endif -%}) — short notes on
this and earlier versions are at [/releases/]({{ '/releases/' | relative_url }});
the GitHub release pages are at
[{{ site.github_url }}/releases/tag/{{ site.release.tag }}]({{ site.github_url }}/releases/tag/{{ site.release.tag }}).

<p class="hero-cta" markdown="0">
  <a class="btn btn--primary" href="{{ site.github_url }}/releases/download/{{ site.release.tag }}/johnnycastawayps1.bin">Download <code>.bin</code> <small>(~76&nbsp;MiB)</small></a>
  <a class="btn"              href="{{ site.github_url }}/releases/download/{{ site.release.tag }}/johnnycastawayps1.cue">Download <code>.cue</code> <small>(71&nbsp;B)</small></a>
  <a class="btn btn--small"   href="{{ '/releases/' | relative_url }}">Release notes</a>
</p>

The CD image ships as a **.bin / .cue** pair. Both files belong in
the same directory. Sizes shown are for the
[`{{ site.release.tag }}`]({{ site.github_url }}/releases/tag/{{ site.release.tag }})
upload — the `.bin` is ~79&nbsp;MB (~76&nbsp;MiB), mostly [FG2 pack]({{ '/docs/glossary/#fg2-pack' | relative_url }})
payload routed onto the disc.

## Quickstart (DuckStation) {#emulator}

1. Install [DuckStation](https://www.duckstation.org/) on your
   platform of choice.
2. Drop `johnnycastawayps1.cue` and `johnnycastawayps1.bin` into a folder.
3. In DuckStation: *File → Start File…* → pick `johnnycastawayps1.cue`.
4. Hit start. The game runs as a screensaver — let it idle and the
   scenes will cycle on their own.

There is no menu to "begin." The game *is* the screensaver.

DuckStation is the every-commit reference; for other emulators, real
PS1 hardware via TonyHax, PS2/PS3 backwards-compat, and the BIOS
requirement, see [/docs/devices/]({{ '/docs/devices/' | relative_url }}).

## Verify your download {#verify}

{%- comment -%}
  Per-release maintenance: the two SHA-256 values below come from
  `site.release.sha256_bin` / `site.release.sha256_cue` in _config.yml.
  On every milestone release tag bump (release_date / tag / version),
  recompute via:

      curl -sL https://github.com/.../releases/download/<tag>/johnnycastawayps1.bin | sha256sum
      curl -sL https://github.com/.../releases/download/<tag>/johnnycastawayps1.cue | sha256sum

  and update those two fields in _config.yml's release: block.
  Future improvement: scripts/release.sh writes a sidecar `.sha256`
  file beside the .bin/.cue artifacts and the build reads from it.
{%- endcomment -%}
Optional but recommended if you downloaded the disc image from a mirror
or anywhere other than the official
[GitHub release page]({{ site.github_url }}/releases/tag/{{ site.release.tag }}).
For **`{{ site.release.tag }}`** the SHA-256 hashes are:

| File          | SHA-256                                                            |
| ------------- | ------------------------------------------------------------------ |
| `johnnycastawayps1.bin` | `{{ site.release.sha256_bin }}` |
| `johnnycastawayps1.cue` | `{{ site.release.sha256_cue }}` |

To check your local copies, run:

```bash
sha256sum johnnycastawayps1.bin johnnycastawayps1.cue
```

(macOS: `shasum -a 256 johnnycastawayps1.bin johnnycastawayps1.cue`. Windows PowerShell:
`Get-FileHash johnnycastawayps1.bin -Algorithm SHA256`.)

If a hash differs from the values above, the file was altered in
transit or by a mirror — re-download from the GitHub release page
linked above. The hashes are pinned to **`{{ site.release.tag }}`**;
older or newer releases will have different values.

## Original Sierra data files

This port ships only the code that drives playback. The original
Sierra data files are **not** included for license reasons.

For development builds (host capture mode), you'll need:

| File          | Original-Sierra source         |
|---------------|--------------------------------|
| `RESOURCE.MAP`| Johnny Castaway 1.0 install    |
| `RESOURCE.001`| Johnny Castaway 1.0 install    |

For the PS1 build, **everything you need is on the disc image** —
the host pipeline pre-bakes the playback packs. End users do not
need any Sierra files to play the .bin/.cue.

## Controls {#controls}

The game runs without any input by default. The pause menu is
optional and reachable with **Start**.

| Button            | Action                                           |
|-------------------|--------------------------------------------------|
| Start             | Open pause menu / resume                         |
| D-pad / left analog | Move cursor or adjust values                   |
| Cross             | Confirm / select                                 |
| Circle            | Back from any menu or submenu                    |

Inside the pause menu you can: enter or exit [Freeplay]({{ '/docs/glossary/#freeplay' | relative_url }}), choose a
Scene Set (All Scenes, Fishing Only, Johnny Stories, Mary Visits,
Visitors, Activities, or Misc & Suzy), open the
[Scene Explorer]({{ '/docs/glossary/#scene-explorer' | relative_url }})
chapter-select grid to jump straight to any of the 63 scenes (each
with an on-PS1-captured thumbnail), mute sound, toggle closed
captions, force day/night, tide, raft, and holidays, advance to
the next scene, set the in-game date, move the island anchor, set
the RNG seed, and open the sound test.

## Freeplay controls

Freeplay launches from the pause menu. It is the direct-control Johnny
mode added in [`v0.5.0-ps1`]({{ '/releases/#v050-ps1--freeplay-and-debug-mode' | relative_url }}).

| Button | Action |
|---|---|
| D-pad / left analog | Walk Johnny. Movement cancels the current action immediately. |
| L2 held | Slow walk. |
| R2 held | Fast walk. |
| Select | Clear the screen, cancel transient actions, and rebuild the island. |
| R1 + Up | Toggle day/night. |
| R1 + Down | Toggle high/low tide. |
| R1 + Left | Cycle raft stage. |
| R1 + Right | Cycle holiday overlay. |
| Start | Open pause menu. |

Fishing is reached through the pause menu's **Scene Set →
Fishing Only** option rather than a dedicated joypad button —
the Circle-as-fish freeplay action shipped in `v0.5.0-ps1` and
was retired during the Scene Set rewrite. See
[/docs/freeplay/#fishing]({{ '/docs/freeplay/#fishing' | relative_url }}).

The full implementation notes live at
[Menu help guide]({{ '/help/menu/' | relative_url }}),
[Freeplay and debug mode]({{ '/docs/freeplay/' | relative_url }}), and
[Pause menu]({{ '/docs/pause-menu/' | relative_url }}).

## Real PS1 hardware

It runs on real hardware — the build has been smoke-tested on a
SCPH-7501 via the [TonyHax](https://github.com/socram8888/tonyhax)
softmod path. No officially-licensed disc, no modchip required.
Your mileage may vary; treat any boot success as a small miracle.

## Where to file a bug

The issue tracker is at [{{ site.repo }}/issues]({{ site.github_url }}/issues).
Bugs are tolerated, not invited — there's no contributor onboarding,
just one author and a tip jar that doesn't exist. See the [FAQ]({{ '/faq/' | relative_url }}).

For security-relevant reports (build/release supply-chain concerns,
the published `.bin` / `.cue` disc image being mishandled by a
download mirror), prefer the
[GitHub Security Advisories]({{ site.github_url }}/security/advisories/new)
channel named in
[`/.well-known/security.txt`]({{ '/.well-known/security.txt' | relative_url }}).

## Related pages

- [FAQ]({{ '/faq/' | relative_url }}) — author-written answers to
  the common "what is this?" / "do I need Sierra files?" / "does
  it work on real hardware?" questions.
- [Releases]({{ '/releases/' | relative_url }}) — short notes on
  every tagged version, with links to the full release notes.
- [Help: menu screenshots]({{ '/help/menu/' | relative_url }}) —
  every pause-menu sub-screen captured from the PS1 build.
- [Freeplay reference]({{ '/docs/freeplay/' | relative_url }})
  and [Pause menu reference]({{ '/docs/pause-menu/' | relative_url }})
  — the full controls and sub-screen surface.
- [Scene ledger]({{ '/scenes/' | relative_url }}) — every scene
  the original game had, validation status, last-verified release tag.
- [Performance battle card]({{ '/perf/' | relative_url }}) — the
  second ledger: 126-variant headless-DuckStation timing matrix
  for "does this scene hit its target frame budget on PS1
  hardware?", sortable with color-coded target speed cells.
- [About]({{ '/about/' | relative_url }}) — the long-form
  project overview for the "wait, what *is* this?" question
  before you click Download.
