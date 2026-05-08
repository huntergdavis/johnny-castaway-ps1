---
title: FAQ
eyebrow: Author-written answers
subtitle: This is the only conversation surface. There's no comments box, no Discord, no Discussions tab — read on.
description: Frequently asked questions about the Johnny Castaway PS1 fan port — what it is, why PS1, legality, how to run it, emulator support, real hardware, target speed, where to file bugs, and what's added vs preserved from the original 1992 Sierra screensaver.
---

{%- comment -%}
  Schema.org FAQPage structured data. Mirrors the 14 H3 questions
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
        "text": {{ "At " | append: site.release.tag | append: " the headless-perf battle card averages " | append: site.release.perf_target_speed_pct | append: "% target speed across the 120 timing-bearing scene/tide rows — close enough that most scenes hit their original frame budget on PS1 hardware. The remaining gap is concentrated in a small set of high-leverage scenes." | jsonify }}
      }
    },
    {
      "@type": "Question",
      "name": "Where do I file bugs?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "If you must, the issue tracker is on GitHub. Bugs are tolerated, not invited. There is no contributor onboarding process and no 'good first issue' label."
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
      "name": "Why are there 36 holidays now instead of 4?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "The original Sierra game had 4 baked-in holiday decorations (Christmas, New Year, Halloween, St. Patrick's Day). This port extends that to 36 US holidays via a code-generated table and a pure-algorithm date core (Meeus for Easter, Nth-weekday math for the others). No external date library, no expiring tables, works for 100+ years."
      }
    },
    {
      "@type": "Question",
      "name": "What's faithful to the original, and what's added?",
      "acceptedAnswer": {
        "@type": "Answer",
        "text": "Faithful: every scene the original had, in the original order, with the original variants. The art is unchanged. Added: a real pause menu reachable with Start (the original had none) with sub-screens for Scene Set, Freeplay Options, Controls, World Options, Holidays, Set Island Position, Accessibility, Sound Test, System, Set Time/Date, Set RNG Seed; closed captions; thirty-two additional holidays; story-loop walking between scenes; freeplay/debug mode where the player drives Johnny directly; optional ocean-ambience loop; Scene Set pool selector; frog-clock loading transitions; full Credits page."
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
DuckStation. {{ site.release.scenes_validated }} of {{ site.release.scenes_total }}
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
source files. See [/legal/]({{ '/legal/' | relative_url }}).

### What does "{{ site.release.scenes_validated }} / {{ site.release.scenes_total }} scenes validated" mean?

Each scene needs a clean host capture, a clean PS1 replay
through every variant, and a no-corruption second pass at
native resolution. That's the project's "FISHING 1 bar" —
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

### Which emulators are supported?

DuckStation is the only one tested every commit. PCSX-Redux
should work. ePSXe is unverified. The release smoke gauntlet
runs in headless DuckStation, so that one is the reference.

### Does it run at native rate?

At {{ site.release.tag }} the headless-perf battle card averages
**[{{ site.release.perf_target_speed_pct }}% target speed]({{ '/docs/glossary/#target-speed' | relative_url }})** across
the 120 timing-bearing scene/tide rows — close enough that most
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
About]({{ '/about/#what-this-isnt' | relative_url }}).

---

## Original game

### Where does the caption text come from?

The closed-caption text was authored fresh for this port from
scene content. It is not lifted from any prior corpus. The
[caption audit]({{ site.github_url }}/blob/main/docs/ps1/caption-audit-2026-04-26.yaml)
shows the confidence level of every ADS-tag → caption mapping
(30 HIGH / 21 MED / 12 LOW as of {{ site.release.tag }}).

### Why are there 36 holidays now instead of 4?

The original Sierra game had 4 baked-in holiday decorations
(Christmas, New Year, Halloween, St. Patrick's Day). This port
extends that to 36 US holidays via a code-generated table and a
pure-algorithm date core (Meeus for Easter, Nth-weekday math for
the others). No external date library, no expiring tables, works
for 100+ years. See `holidays.yml` and `src/holidays.c` in the
repository.

### What's faithful to the original, and what's added?

**Faithful**: every scene the original had, in the original
order, with the original variants. The art is unchanged. The
holidays' visual style matches the existing 4 (Sierra retained
the original sprites; the new ones were authored to fit).

**Added**: a real [pause menu]({{ '/docs/pause-menu/' | relative_url }})
reachable with Start (the original had none), with sub-screens
for Scene Set, Freeplay Options, Controls, World Options,
Holidays, Set Island Position, Accessibility, Sound Test,
System, Set Time / Date, and Set RNG Seed. Closed captions
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
- [Glossary]({{ '/docs/glossary/' | relative_url }}) —
  vocabulary anchor for terms (`FG2 pack`, `FISHING 1 bar`,
  `target speed`, `host build`) used throughout the answers
  above.
