---
title: FAQ
eyebrow: Author-written answers
subtitle: This is the only conversation surface. There's no comments box, no Discord, no Discussions tab — read on.
---

This page is information, not a forum. Questions and answers are
written by the author, not crowdsourced. Three sections: about
the project, running it, and the original game.

---

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

### Why is it only {{ site.release.scenes_validated }} / {{ site.release.scenes_total }} scenes?

Each scene needs a clean host capture, a clean PS1 replay
through every variant, and a no-corruption second pass at
native resolution. That's a stricter bar than "it ran once
without crashing." The [scene ledger]({{ '/scenes/' | relative_url }})
tracks the count.

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

### Where do I file bugs?

If you must, the issue tracker is at
[{{ site.repo }}/issues]({{ site.github_url }}/issues). Bugs are
tolerated, not invited. There is no contributor onboarding
process and no "good first issue" label — see the [non-goals on
About]({{ '/about/' | relative_url }}).

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

**Added**: closed captions (off by default), 32 additional
holidays, a pause menu with Sound / Day-Night / Tide / Raft /
Captions toggles, a Set Time / Set Island Pos / Set RNG Seed
editor for testing, and a full Credits page.
