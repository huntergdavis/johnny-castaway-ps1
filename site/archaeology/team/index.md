---
layout: page
title: The Team
eyebrow: One human, a long bibliography, some AI sub-agents
subtitle: A small newsroom-lab, with a single byline.
description: An honest accounting of who actually contributed to the Johnny Castaway PS1 fan port — the author, the prior reverse-engineering ports, the toolchain authors, and the AI assistants that helped draft.
---

The PS1 branch reads like the notebook of a small newsroom-lab:
several distinct voices, all aligned on the same standard. The voice
on a March agent prompt — sceptical, refusing to count black frames
as wins — is recognizably the same project as the voice in the
April foreground-pilot worklog that decides the captured pixels are
the answer. The shared values aren't framed as a manifesto but they
show up in every commit message: *evidence before confidence*,
*reproducibility before storytelling*, *human review before claimed
validation*, *memory and runtime reality over elegant theory*,
*bespoke pragmatism when generic frameworks stop paying off*,
*preserve the paper trail even when the direction changes*.

The team that produced that work is not traditional. There is one
human author. There are several prior open-source projects that this
port stands on. There are a handful of AI assistants that helped
draft, debug, and pattern-match through the long reverse-engineering
stretches. That's it. The rest of this page names what each
contributor actually did, because the line between "wrote it,"
"authored the work it depends on," and "helped the author think"
matters and is worth being honest about.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## The author

**Hunter Davis** wrote the PS1 port. That includes every line in
`graphics_ps1.c`, `cdrom_ps1.c`, `sound_ps1.c`, `events_ps1.c`,
`spi.c`, `pause_menu.c`, `foreground_pilot.c`, the offline pipeline
scripts under `scripts/`, the documentation, and the website you're
reading this on. Hunter also did the human signoff for `FISHING 1`
and `FISHING 2` — sat in front of DuckStation, watched every
variant, listened for missing or misaligned SFX, and decided when
each was good enough to promote. The git history is sole-author. So
is the credits screen on the disc.

The team-perspective archaeology document describes Hunter playing
several roles across the project's six-month arc: *operator-editor*
who rejects convenient wins, *embedded porter* who respects the
exact build scripts, *instrumentarian* who builds debug overlays,
*validator-skeptic* who refuses to count title frames as scenes,
*archaeology engineer* who maintains the historical binary library,
*bespoke scene engineer* who authored the foreground playback
pivot. Those roles are different jobs in the same skull. The
unifying read: different voices handled different phases, but they
stayed aligned on one ethic — make the machine legible, distrust
convenient narratives, keep pushing until the screen tells the
truth.

## The prior ports

Three projects did the reverse-engineering this port stands on.
None of them are part of this project's team in the day-to-day
sense; all of them are part of the bibliography in the load-bearing
sense.

**`jno6809/jc_reborn`** is the SDL2 port that decoded the original
engine's bytecode — ADS, TTM, RES interpreters, the dirty-rect
machinery, the resource format. Without that work the PS1 host build
doesn't exist. This project's host capture is a fork of it.

**`nivs1978/JCOS`** is an alternate decoding effort, useful for
cross-validating frames where the bytecode could be read more than
one way. When this project's capture disagreed with what a scene
should look like, having a second reference implementation was the
fast way to ask which interpretation was correct.

**`xesf/Castaway`** is more notes than running code, but the notes
caught engine quirks that running interpreters elide. Several rare
opcodes and edge cases got resolved by comparing against `xesf`'s
reading.

**The Sierra Chest archive** kept the original `JOHNNY.EXE` and
resource files in a verifiable form. Without that there's no way to
tell whether a decoding mismatch is the engine being weird or the
input being corrupt.

## The toolchain authors

The PS1 build is impossible without **PSn00bSDK** (Lameguy64 et
al.), the open-source PS1 SDK that replaces Sony's proprietary
toolchain. The SPI pad driver in `src/platform/ps1/spi.c` is derived from
**spicyjpeg's** MPL-licensed pad-poll example — with the
DuckStation-specific `tx_len=5` fix on top. The emulator that every
commit gets tested against is **DuckStation** (Connor McLaughlin et
al.). The CD image is packed by **mkpsxiso**. The Easter algorithm
that drives the movable-feast holidays is **Meeus / Jones /
Butcher**. Each one of those names solved a problem that would
otherwise have eaten a month.

## The AI assistants

This project used AI assistants for drafting, debugging, and
pattern-matching through the reverse-engineering stretches. That's
worth naming because pretending otherwise would be dishonest and
because the *kind* of help matters. The assistants drafted
documentation, summarized long worklogs, helped read SIO0 datasheet
fragments, and pattern-matched against PS1-era reference code when
something on DuckStation behaved differently from what the
spicyjpeg example expected. They did not author scenes. They did
not decide what counted as validated. They did not press signoff.
The validator-skeptic role — the one that refused to count black
frames as wins — was always Hunter's, and the AI assistants
sometimes had to be reminded that *running* and *correct* are not
the same thing. The drafts on this site went through human editing
before they shipped.

## The reader

This is a small distinction but worth making. The project's "team"
also implicitly includes the reader of the GitHub issues, the
DuckStation forum thread, the friend who said the early build's
sky color looked off. None of those names appear in the
contributors graph. They're part of the loop anyway.

---

The team is small. The bibliography is long. The work is sole-author
in the sense the credits page means it: one human accountable for
every shipped commit. Everything else is generously donated upstream
work this port could not have existed without. The site doesn't put
anyone's photo on it. Hunter's name is on the credits screen. The
prior ports are linked from the [Credits page]({{ '/credits/' | relative_url }}).
That's the whole accounting.

## Related pages

- [Credits]({{ '/credits/' | relative_url }}) — the full
  acknowledgements page; toolchain authors, prior ports,
  AI sub-agents, fonts, ocean-ambience source.
- [About: Voice]({{ '/about/voice/' | relative_url }}) —
  the editorial standard the prose-drafted-by-agents had
  to be edited up to.
- [Docs: AI sub-agents]({{ '/docs/agents/' | relative_url }})
  — honest accounting of where the agents helped on this
  project and where they didn't.
- [Lab: the LLM pass]({{ '/lab/llm-pass/' | relative_url }})
  — methodology essay on running parallel agents under
  human review.
- [Lab: voice-anchor problem]({{ '/lab/voice-anchor-problem/' | relative_url }})
  — long-form retrospective on keeping a single voice
  across human + agent prose.
- [Lab: dunking-bird]({{ '/lab/dunking-bird/' | relative_url }})
  — the auto-poker that keeps the agents productive
  between human review passes.
