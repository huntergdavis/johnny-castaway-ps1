---
title: FAQ
eyebrow: Author-written answers
subtitle: This is the only conversation surface. There's no comments box, no Discord, no Discussions tab — read on.
description: Frequently asked questions about the Johnny Castaway PS1 fan port — what it is, why PS1, legality, sponsorship stance, how to run it, emulator support, real hardware, target speed, where to file bugs, and what's added vs preserved from the original 1992 Sierra screensaver.
---

{%- comment -%}
  Schema.org FAQPage structured data. Mirrors the 17 H3 questions
  on this page with 1–2 sentence summary answers. Google retired
  FAQ rich results for general sites in 2023, but Bing, AI agents,
  and embedded knowledge graphs still consume FAQPage. Hand-mirrored
  here next to the questions so updates stay in one place. Liquid
  variables (counts, release tag) are jsonify'd to survive any
  punctuation; literal strings are pre-escaped to plain text.
{%- endcomment -%}
<script type="application/ld+json">
{
  "@context": "https://schema.org",
  "@type": "FAQPage",
  "inLanguage": "en",
  "mainEntity": [
    {
      "@type": "Question",
      "name": "What is this?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": {{ "A port of Sierra's 1992 Johnny Castaway screensaver to the original PlayStation. It runs on real hardware and on DuckStation. " | append: site.release.scenes_validated | append: " of " | append: site.release.scenes_total | append: " scenes are validated as of " | append: site.release.tag | append: "." | jsonify }}
      }
    },
    {
      "@type": "Question",
      "name": "Why PS1?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "The hardware has roughly the same headroom as a 1992 PC, but a totally different graphics pipeline. The fun was in the gap — making a 1992 Windows screensaver render natively on a PS1 GPU that doesn't know what BMPs are."
      }
    },
    {
      "@type": "Question",
      "name": "Will you port other Sierra screensavers?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "No plans. Johnny Castaway is a special case for the author. If someone else wants to take what's here and port Surf's Up that would be cool, but it's not on this project's roadmap."
      }
    },
    {
      "@type": "Question",
      "name": "Is this legal?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "The code is GPL-3.0 and entirely original (or derived from other GPL/MPL code with attribution). The Johnny Castaway character and original Sierra assets are not included with the released disc image; the host build requires the user to supply their own original Sierra data files. The released .bin/.cue contains pre-baked playback packs only."
      }
    },
    {
      "@type": "Question",
      "name": "Can I sponsor or donate to this project?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "No. The project does not accept sponsorships, tips, or donations. The in-game credits read in part 'if you paid for this, you were cheated. Open source and free.' — that's the whole stance. The code is on GitHub under GPL-3.0; the disc image is a free download from the GitHub release page. There is no Patreon, GitHub Sponsors, Ko-fi, or PayPal link, and there will not be."
      }
    },
    {
      "@type": "Question",
      "name": {{ "What does '" | append: site.release.scenes_validated | append: " / " | append: site.release.scenes_total | append: " scenes validated' mean?" | jsonify }},
      "acceptedAnswer": {
        "@type": "Answer",
        "text": {{ "Each scene clears the project's FISHING 1 bar — pixel-perfect visuals against the host capture, plus synced SFX, signed off by human review across every applicable variant flag (night, low-tide, holiday, raft-stage). It is a stricter bar than 'it ran once without crashing.' At " | append: site.release.tag | append: " every row clears the bar. Performance is a separate ledger." | jsonify }}
      }
    },
    {
      "@type": "Question",
      "name": "How do I run it?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "Download the .bin and .cue, drop both files into a folder, open the .cue in DuckStation. That's the whole quickstart."
      }
    },
    {
      "@type": "Question",
      "name": "Do I need original Sierra files?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "Not for the released .bin/.cue. End users get pre-baked playback packs on the disc image and never need a Sierra file. Yes for the host build used during development: you need RESOURCE.MAP and RESOURCE.001 from a legal Johnny Castaway 1.0 install."
      }
    },
    {
      "@type": "Question",
      "name": "Does it work on real PS1 hardware?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "Yes — smoke-tested on a SCPH-7501 via the TonyHax softmod path. Your mileage may vary; treat any boot success as a small miracle."
      }
    },
    {
      "@type": "Question",
      "name": "Which emulators are supported?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "DuckStation is the only one tested every commit. PCSX-Redux should work. ePSXe is unverified. The release smoke gauntlet runs in headless DuckStation, so that one is the reference."
      }
    },
    {
      "@type": "Question",
      "name": "Does it run at native rate?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": {{ "At " | append: site.release.tag | append: " the headless-perf battle card averages " | append: site.release.perf_target_speed_pct | append: "% target speed across all 126 timing-bearing scene/tide rows — close enough that most scenes hit their original frame budget on PS1 hardware. The remaining gap is concentrated in a small set of high-leverage scenes." | jsonify }}
      }
    },
    {
      "@type": "Question",
      "name": "Where do I file bugs?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "If you must, the issue tracker is on GitHub. Bugs are tolerated, not invited. There is no contributor onboarding process and no 'good first issue' label. When you file, name the device (emulator + version, or real hardware model) — the devices reference lists which paths are tested every commit vs unverified."
      }
    },
    {
      "@type": "Question",
      "name": "Where does the caption text come from?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "The closed-caption text was authored fresh for this port from scene content. It is not lifted from any prior corpus. The caption audit shows the confidence level of every ADS-tag to caption mapping."
      }
    },
    {
      "@type": "Question",
      "name": "Why does the scene title differ from the on-screen caption sometimes?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "The on-screen caption text is fixed and the scene-page title is what the pack actually plays on PS1. They lined up sequentially for most scenes but the v0.8.4-ps1 chapter-select grind found the audit's caption-to-scene mapping was wrong on several rows. FISHING 2 is the canonical example: the on-screen caption says he catches a boot, but the on-PS1 pack reels in a Titanic life preserver. Repointing the runtime caption mapping is open work."
      }
    },
    {
      "@type": "Question",
      "name": "Why are there 36 holidays now instead of 4?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "The original Sierra game had 4 baked-in holiday decorations (Christmas, New Year, Halloween, St. Patrick's Day). This port extends that to 36 US holidays via a code-generated table and a pure-algorithm date core (Meeus for Easter, Nth-weekday math for the others). No external date library, no expiring tables, works for 100+ years."
      }
    },
    {
      "@type": "Question",
      "name": "How do I jump to a specific scene?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "Open the pause menu with Start, choose Scene Explorer, and step through with LEFT/RIGHT (one scene at a time) or L1/R1 (one scene family at a time). Cross plays the highlighted scene once; Triangle loops it. Each entry shows a captured-on-PS1 thumbnail and the scene title; the 63 thumbnails were each captured during the v0.8.4-ps1 chapter-select grind."
      }
    },
    {
      "@type": "Question",
      "name": "What's faithful to the original, and what's added?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "Faithful: every scene the original had, in the original order, with the original variants. The art is unchanged. Added: a real pause menu reachable with Start (the original had none) with sub-screens for Scene Set, Scene Explorer, Freeplay Options, Controls, World Options, Holidays, Set Island Position, Accessibility, Sound Test, System, Set Time/Date, Set RNG Seed; closed captions; thirty-two additional holidays; story-loop walking between scenes; freeplay/debug mode where the player drives Johnny directly; optional ocean-ambience loop; Scene Set pool selector; frog-clock loading transitions; full Credits page."
      }
    }
  ]
}
</script>

This page is information, not a forum. Questions and answers are
written by the author, not crowdsourced. Three sections: about
the project, running it, and the original game.

---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## About

### What is this?

A port of Sierra's 1992 *Johnny Castaway* screensaver to the
original PlayStation. It runs on real hardware and on
[DuckStation]({{ '/docs/glossary/#duckstation' | relative_url }}). {{ site.release.scenes_validated }} of {{ site.release.scenes_total }}
scenes are validated as of {{ site.release.tag }}.

### Why PS1?

The hardware has roughly the same headroom as a 1992 PC, but a
totally different graphics pipeline. The fun was in the gap —
making a 1992 Windows screensaver render natively on a PS1 GPU
that doesn't know what BMPs are.

### Will you port other Sierra screensavers?

No plans. *Johnny Castaway* is a special case for me; I'm not
shopping for the next one. If someone else wants to take what's
here and port *Surf's Up*, that would be cool, but it's not
where I'm spending evenings.

### Is this legal?

The *code* in the repository is GPL-3.0 and entirely original
(or derived from other GPL/MPL code with attribution). The
*Johnny Castaway character and original Sierra assets* are not
included with the released disc image; the host build requires
the user to supply their own original Sierra data files
(`RESOURCE.MAP`, `RESOURCE.001`). The released `.bin/.cue`
contains pre-baked playback packs — derived data, no Sierra
source files. See [/legal/]({{ '/legal/' | relative_url }}) for
the licensing companion and
[/lab/fan-port-in-public/]({{ '/lab/fan-port-in-public/' | relative_url }})
for the long-form essay on what "shipping a fan port in 2026"
actually looks like — Sierra's permission, the GPL-3.0 / MPL-2.0
license stack, and the "if you paid for this, you were cheated"
voice that keeps the whole thing honest.

### Can I sponsor or donate to this project?

No. The project does not accept sponsorships, tips, or donations.
The [in-game credits]({{ '/credits/' | relative_url }}) read in
part "if you paid for this, you were cheated. Open source and
free." — that's the whole stance. The code lives on
[GitHub]({{ site.github_url }}) under
[GPL-3.0]({{ '/legal/' | relative_url }}); the
[disc image]({{ '/play/' | relative_url }}) is a free download
from the GitHub release page. There is no Patreon, GitHub
Sponsors, Ko-fi, or PayPal link, and there will not be. The work
is the point.

### What does "{{ site.release.scenes_validated }} / {{ site.release.scenes_total }} scenes validated" mean?

Each scene needs a clean host capture, a clean PS1 replay
through every variant, and a no-corruption second pass at
native resolution. That's the project's "[FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }})" —
pixel-perfect visuals plus synced SFX, signed off by human
review across every applicable variant flag (night, low-tide,
holiday, raft-stage). It is a stricter bar than "it ran once
without crashing." The [scene ledger]({{ '/scenes/' | relative_url }})
is the source of truth; at `{{ site.release.tag }}` every row
clears the bar. Performance — whether each scene hits its
target frame budget — is a separate ledger at
[/perf/]({{ '/perf/' | relative_url }}) on purpose.

---

## Running it

### How do I run it?

Download the [.bin and .cue]({{ '/play/' | relative_url }}),
drop both files into a folder, open the `.cue` in DuckStation.
That's the whole quickstart.

### Do I need original Sierra files?

**Not for the released `.bin/.cue`.** End users get pre-baked
playback packs on the disc image and never need a Sierra file.

**Yes, for the host build** used during development. If you're
compiling from source and want host-mode capture, you need
`RESOURCE.MAP` and `RESOURCE.001` from a legal *Johnny Castaway*
1.0 install. The repository does not ship them.

### Does it work on real PS1 hardware?

Yes — smoke-tested on a SCPH-7501 via the
[TonyHax](https://github.com/socram8888/tonyhax) softmod path.
Your mileage may vary; treat any boot success as a small miracle.
The full device matrix — tested cadence, console model, boot path,
media — is at
[/docs/devices/]({{ '/docs/devices/' | relative_url }}).

### Which emulators are supported?

DuckStation is the only one tested every commit. PCSX-Redux
should work. ePSXe is unverified. The release smoke gauntlet
runs in headless DuckStation, so that one is the reference.
[/docs/devices/]({{ '/docs/devices/' | relative_url }}) has the
fields-table reference: tested cadence, platforms, BIOS, and the
should-work-unverified and unsupported lists.

### Does it run at native rate?

At {{ site.release.tag }} the headless-perf battle card averages
**[{{ site.release.perf_target_speed_pct }}% target speed]({{ '/docs/glossary/#target-speed' | relative_url }})** across
all 126 timing-bearing scene/tide rows — close enough that most
scenes hit their original frame budget on PS1 hardware. The
remaining gap is concentrated in a small set of high-leverage
scenes (wide-action, clean-rect-heavy frames) and is the active
work between milestone tags. The
[battle card]({{ '/perf/' | relative_url }}) shows the
per-scene timing — sortable, color-coded; the
[retrospective]({{ '/lab/from-87-to-99-5/' | relative_url }})
walks through how the matrix moved here from the compact
baseline of 87.1%.

### Where do I file bugs?

If you must, the issue tracker is at
[{{ site.repo }}/issues]({{ site.github_url }}/issues). Bugs are
tolerated, not invited. There is no contributor onboarding
process and no "good first issue" label — see the [non-goals on
About]({{ '/about/#what-this-isnt' | relative_url }}). When you
file, name the device (emulator + version, or real hardware
model) — the
[devices reference]({{ '/docs/devices/' | relative_url }}) lists
which paths are tested every commit vs unverified, so the report
lands faster if it pins which it was.

For security-relevant reports (build/release supply-chain concerns,
the published `.bin` / `.cue` disc image being mishandled by a
download mirror), prefer the
[GitHub Security Advisories]({{ site.github_url }}/security/advisories/new)
channel named in
[`/.well-known/security.txt`]({{ '/.well-known/security.txt' | relative_url }}).
That's the RFC 9116 path for private disclosures; public issues
remain on the tracker above.

---

## Original game

### Where does the caption text come from?

The closed-caption text was authored fresh for this port from
scene content. It is not lifted from any prior corpus. The
full reference manual is at
[/docs/captions/]({{ '/docs/captions/' | relative_url }}); the
[caption audit]({{ site.github_url }}/blob/main/docs/ps1/caption-audit-2026-04-26.yaml)
shows the confidence level of every ADS-tag → caption mapping
(30 HIGH / 21 MED / 12 LOW as of {{ site.release.tag }}).

### Why does the scene title differ from the on-screen caption sometimes?

Two different sources of truth, and they don't always agree.

The **scene-page title** at `/scenes/<slug>/` is what the pack
actually plays on PS1, confirmed during the
[v0.8.4-ps1 chapter-select grind]({{ '/lab/chapter-select-grind/' | relative_url }}).
The **on-screen caption** that draws in the dark band at the bottom of
the framebuffer is whatever `captionSceneMap[]` in
[`src/platform/ps1/ps1_captions.c`]({{ site.github_url }}/blob/main/src/platform/ps1/ps1_captions.c)
routes that ADS+tag to.

For most scenes those line up. For a few they don't, because the
2026-04-26 caption audit picked its mapping from text alone — without
runtime evidence of which pack played which gag. The clearest example
is `FISHING 2`: the on-screen caption block says "He catches a boot,"
but the on-PS1 pack reels in a Titanic-stenciled life preserver. The
"boot" line actually fits `MARY 2`, where Mary the mermaid swims up
while Johnny is fishing and he ends up reeling in a boot after the
confusion.

The post-validation runtime corrections section at
[/docs/captions/]({{ '/docs/captions/#post-validation-runtime-corrections-v084-ps1' | relative_url }})
lists the named mismaps. Repointing `captionSceneMap[]` so the
on-screen caption matches the on-PS1 gag is open work — `v0.8.4-ps1`
fixed the website's *description* of every scene, not the runtime
mapping itself.

### Why are there 36 holidays now instead of 4?

The original Sierra game had 4 baked-in holiday decorations
(Christmas, New Year, Halloween, St. Patrick's Day). This port
extends that to 36 US holidays via a code-generated table and a
pure-algorithm date core (Meeus for Easter, Nth-weekday math for
the others). No external date library, no expiring tables, works
for 100+ years. The full reference manual is at
[/docs/holidays/]({{ '/docs/holidays/' | relative_url }}); see
`holidays.yml` and `src/scene/holidays.c` in the repository for the
source data and the codegen output.

### How do I jump to a specific scene?

Open the pause menu with **Start** and choose **Scene Explorer**.
Step through with **LEFT / RIGHT** (one scene at a time) or
**L1 / R1** (one scene family at a time — Fishing, Johnny, Mary,
Visitor, Activity, Stand, Walkstuf, etc.). **Cross** plays the
highlighted scene once; **Triangle** plays it on a loop.
**Circle** or **Start** backs out of the picker.

Each entry shows a captured-on-PS1 thumbnail and the scene title.
The 63 thumbnails ship as `SCR\SX<abbrev><tag>.SCR` files on the
disc; each one was captured during the
[`v0.8.4-ps1` chapter-select grind]({{ '/lab/chapter-select-grind/' | relative_url }})
so the picker is showing real on-PS1 footage of the pack, not a
generated frame. Full reference at
[/docs/pause-menu/#scene-explorer]({{ '/docs/pause-menu/#scene-explorer' | relative_url }}).

### What's faithful to the original, and what's added?

**Faithful**: every scene the original had, in the original
order, with the original variants. The art is unchanged. The
holidays' visual style matches the existing 4 (Sierra retained
the original sprites; the new ones were authored to fit).

**Added**: a real [pause menu]({{ '/docs/pause-menu/' | relative_url }})
reachable with Start (the original had none), with sub-screens
for Scene Set, Scene Explorer, Freeplay Options, Controls,
World Options, Holidays, Set Island Position, Accessibility,
Sound Test, System, Set Time / Date, and Set RNG Seed. Closed captions
(off by default; see [/docs/captions/]({{ '/docs/captions/' | relative_url }})).
[Thirty-two additional holidays]({{ '/docs/holidays/' | relative_url }})
via a code-generated table and pure-algorithm date core.
[Story-loop walking]({{ '/docs/walks/' | relative_url }})
between scenes (v0.4.20-ps1) — Johnny no longer teleports.
[Freeplay / debug mode]({{ '/docs/freeplay/' | relative_url }})
(v0.5.0-ps1) where the player drives Johnny directly, with gag
and visitor catalogs. Optional
[ocean-ambience loop]({{ '/releases/#v060-ps1--ocean-ambience' | relative_url }})
on a dedicated SPU voice (v0.6.0-ps1). Scene Set pool selector for
filtering the random rotation by family. Frog-clock loading
transitions between scene swaps. A full
[Credits page]({{ '/credits/' | relative_url }}) that names the
prior ports and toolchain authors this build stands on.

## Related pages

- [Play]({{ '/play/' | relative_url }}) — the canonical
  download + quickstart page for the "How do I run it?" answer.
- [Scenes]({{ '/scenes/' | relative_url }}) — live ledger
  with per-scene case studies for the "What does
  {{ site.release.scenes_validated }} / {{ site.release.scenes_total }} validated mean?" answer.
- [Performance battle card]({{ '/perf/' | relative_url }}) —
  126-variant matrix with sortable, color-coded target speed
  for the "Does it run at native rate?" answer.
- [Legal]({{ '/legal/' | relative_url }}) — the canonical
  licensing surface (GPL-3.0 + Sierra disclaimer + takedown
  procedure) for the "Is this legal?" answer.
- [Credits]({{ '/credits/' | relative_url }}) — the full
  attribution behind the "What's faithful, what's added?"
  answer above: prior ports, toolchain authors, AI sub-agents,
  fonts, ocean ambience source.
- [About]({{ '/about/' | relative_url }}) — the project
  overview if you want the long-form version of "What is
  this?".
- [Docs]({{ '/docs/' | relative_url }}) — twenty reference
  manuals behind every "how does X work?" question (build,
  captions, holidays, pause menu, freeplay, walks, regtest,
  scripted input, performance, hardware, devices, audio,
  infrastructure, file formats, AI sub-agents,
  vision-classifier, API mapping, dev workflow, feeds +
  well-known endpoints, glossary).
- [Devlog]({{ '/devlog/' | relative_url }}) — dated, unedited
  worklogs for the day-by-day "when did X happen and what was
  tried first?" question.
- [Lab]({{ '/lab/' | relative_url }}) — seventeen feature-length
  retrospectives for the "but why did it end up that way?"
  question, with the benefit of hindsight the worklogs above
  deliberately lack.
- [Glossary]({{ '/docs/glossary/' | relative_url }}) —
  vocabulary anchor for terms (`FG2 pack`, `FISHING 1 bar`,
  `target speed`, `host build`) used throughout the answers
  above.
