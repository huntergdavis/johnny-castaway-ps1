---
layout: page
title: Archaeology
eyebrow: How a 1992 screensaver became a 2026 PS1 disc
subtitle: Five chapters. The dead ends count.
description: The full story of the Johnny Castaway PS1 fan port — from Sierra's 1992 screensaver through three prior reverse-engineered ports to the hybrid host-and-replay pipeline that made FISHING 1 work.
---

This is the long version. Twelve to fifteen minutes of reading, depending
on how fast you skim the part where the printf calls corrupt the runtime.
The short version lives on the [About page]({{ '/about/' | relative_url }})
and the per-scene ledger lives at [/scenes/]({{ '/scenes/' | relative_url }}).

The numbers, dates, and names below are real. Where something is unknown
or contested, that's stated. Where Hunter took a wrong turn, that's stated
too — the dead ends are what made the eventual method look obvious.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

<section class="chapter" markdown="1">
<span class="chapter-num">Chapter 1</span>
## The screensaver before the project

In November 1992, Sierra On-Line shipped *Johnny Castaway* as a Windows
3.1 screensaver. It ran on a stretch of PC hardware that was already
starting to feel old: a 386 with VGA, a
Sound Blaster if you were lucky, and 4 megabytes of RAM if you'd
splurged. The premise was small. A man is stranded on an island the
size of a kitchen rug. He fishes. He reads books. He waves at passing
ships that never stop. Holidays come and go and Johnny notices them in
small ways — a Christmas tree, a turkey, a pumpkin. The whole thing is
about ninety scenes deep, plays out in soft cartoon colors at 640x480,
and was meant to run on your monitor when you walked away from your
desk.

That last part matters. *Johnny Castaway* belongs to a genre that
doesn't really exist anymore: the screensaver as software product. In
1992 a CRT monitor that displayed the same pixels for hours could
literally burn the phosphor and leave a ghost image you could read
through the next morning's spreadsheet. Screensavers were necessary
infrastructure. So they became a market. *After Dark* shipped flying
toasters and aquariums and modular themes. *Idle PC* shipped puzzle
worlds. Berkeley Systems made millions on what was, technically,
software designed to do nothing.

Sierra's contribution to that market was Johnny. The pitch was
narrative. Most screensavers were patterns or kaleidoscopes; Johnny was
a *story* you watched in glances. You'd come back from lunch and Johnny
would be hauling something out of the surf. You'd come back from a
meeting and a shark would be circling. The animation work is hand-drawn
in the Sierra house style of the late King's Quest era. The engine
underneath is a small bytecode interpreter that walks through scripts
named `FISHING.ADS`, `WALKSTUF.ADS`, `MISCGAG.ADS`, calling animation
files (`TTM`) and resource files (`RES`) on demand.

The genre died for two reasons, both undramatic. LCD monitors don't
burn in the way CRTs did, so the *necessary infrastructure* argument
disappeared in the early 2000s. And the kind of person who would have
bought *After Dark* in 1993 had, by 2003, a web browser full of free
distractions. The market for screensaver-as-product collapsed faster
than the market for shareware. Most of the catalog from that era is
unrecoverable now — gone with the floppies, the boxes, the install
disks that nobody bothered to image. *Johnny Castaway* survived because
people loved it enough to keep their original CDs and, eventually, to
reverse-engineer the engine.

What's left of the original release is a `JOHNNY.EXE` and a directory
of `.ADS`, `.TTM`, and `.RES` files, plus a 286-era VGA palette and a
batch of digitized sound effects. That's the seed corpus for everything
downstream.
</section>

<section class="chapter" markdown="1">
<span class="chapter-num">Chapter 2</span>
## The first port

The work that made every later port possible was done by `jno6809`,
under the project name `jc_reborn`. It's an SDL2 reimplementation of
the original engine, written in plain C, available on GitHub. What
that codebase actually accomplished, in technical terms, is the part
nobody outside this small circle quite appreciates: it reads Sierra's
proprietary scripting bytecode and runs it.

The original `JOHNNY.EXE` is not a video player. It's an interpreter.
The `.ADS` files are *animation description scripts* — a small,
stack-based bytecode that says things like "play TTM scene 17," "wait
for a sound to finish," "branch on holiday flag," "loop until input."
The `.TTM` files are the animations themselves, also bytecode, with
opcodes like `DRAW_PIXEL`, `DRAW_LINE`, `LOAD_SPRITE`, `BLIT`, and
`PLAY_SAMPLE`. The `.RES` files hold sprite atlases and palettes. The
engine binds these together with a global "island state" — what time
of day it is, what the tide is doing, which holiday is active, how far
along the raft Johnny is building.

`jc_reborn` works out the bytecode. It walks every opcode. It reads
the resource format. It manages the dirty-rect bookkeeping the
original engine relied on to redraw only the pixels that changed,
because in 1992 a 640x480 full-screen blit was expensive enough to
matter. It does all this on top of SDL2 so it can run on a modern
desktop, in a window, with the fan switching on after about thirty
seconds because the SDL surface lock isn't doing the original engine
any favors.

You can quibble with parts of the implementation — the threading
model, the audio mixer, how aggressively the dirty rectangles are
coalesced — but the foundational work is solid, and once it exists,
every subsequent port is a translation problem rather than a
reverse-engineering problem. This PS1 project's host build is a
direct fork of `jc_reborn`. The ADS, TTM, and RES interpreters are
that work, with PS1-specific glue around the edges.

`jno6809` did the difficult part. The credits page on the disc
mentions it; this site mentions it; the source headers preserve the
original copyright notice. The point is: there is a single moment
where someone reads a binary format and writes the parser, and after
that everyone else is doing applied work. That moment was 2017.
</section>

<section class="chapter" markdown="1">
<span class="chapter-num">Chapter 3</span>
## Across the ports

`jc_reborn` is the canonical decoding effort, but it isn't the only
one. Two other projects worked the same problem from different angles,
and a fourth source kept the documentation trail intact.

**`nivs1978/JCOS`** is an alternate decoding effort, written
independently, that approaches the bytecode from a slightly different
direction. The two interpreters do not always agree on edge cases.
That disagreement is useful — when this project's PS1 host capture
disagreed with what a scene was supposed to look like, having a second
reference implementation was a way to ask *whose interpretation is
wrong, mine or the engine's?* without having to crack the original
1992 binary in a disassembler. JCOS rendered slightly different output
on a handful of opcodes; the differences are catalogued in the host
capture's frame-meta JSON files.

**`xesf/Castaway`** is a smaller reverse-engineering effort, more
notes than running code, but it preserves things the running ports
elide. Some of the engine's quirks — the way it handles certain rare
opcodes, the unusual tide-state flag, the off-by-one in palette index
zero — appear in `xesf`'s notes before they appear anywhere else. When
this project hit a TTM opcode that the SDL2 path was silently
ignoring, `xesf`'s reading of the bytecode was the way to confirm
"that's not garbage, the original engine actually does that."

**The Sierra Chest archive** is a community-run repository of Sierra
materials — manuals, box copy, install disk images, advertising. It's
the place where the original `JOHNNY.EXE` and resource files came
from in the form this project uses them. Without the Chest there's no
way to verify a fresh dump against a known-good copy. Without that
verification, every later "is the bytecode being read correctly"
question becomes harder to answer.

None of these efforts are this project. None of them ported anything
to the PS1, or wanted to. But the PS1 port stands on what each one
preserved. The right way to think of the prior ports is as a
*bibliography*: a row of books on the shelf you keep reaching for,
each open to the chapter you need.
</section>

<section class="chapter" markdown="1">
<span class="chapter-num">Chapter 4</span>
## The PS1 question

The first commit on the PS1 branch landed on **2025-10-18**. The
question that started it was specific: could a 1994 console run a
1992 screensaver?

On paper the fit looks suspiciously good. The PlayStation 1 has 2 MB
of main RAM. *Johnny Castaway*'s desktop port, after Hunter's earlier
`4mb2025` memory-optimization branch, peaks at about 350 KB. The
console runs a native 640x480 mode. The original game was authored at
640x480. The PS1's hardware sprite engine is built for exactly the
kind of indexed-palette compositing that Johnny's sprites already use.
On those numbers, the port is supposed to be easy.

It was not easy. It was, in fact, several months of dead ends.

The first prototype tried the obvious thing. Take `jc_reborn`'s SDL2
codebase, swap SDL for PSn00bSDK, swap `fopen` for `CdRead`, and let
the existing ADS/TTM/RES interpreters run on the [MIPS]({{ '/docs/glossary/#mips' | relative_url }}) R3000A. The
build came up. Resources loaded from the CD. Scenes ran. And then,
roughly, everything went wrong:

- **[TTY]({{ '/docs/glossary/#tty' | relative_url }}) logging corrupted the runtime.** The host port leans heavily
  on `printf` and `vprintf` for tracing. On the PS1 those calls,
  routed through DuckStation's TTY surface, were not safe in their
  unbounded form. Long scene playback would destabilize. The interim
  fix was to disable text logging entirely (`debugMode=0`) and debug
  by drawing colored pixels into the framebuffer with `LoadImage`.
  Reliable bounded `printf` did not arrive until **2026-04-25**.

- **The SPI pad-poll fix took two days.** The PSn00bSDK pad library
  (`InitPAD` / `StartPAD`) was unusable in the
  PSn00bSDK 0.24 + DuckStation environment. The replacement was a
  direct SIO0 driver derived from spicyjpeg's MPL-licensed example.
  Spicyjpeg's reference code uses a 4-byte poll transmission. On
  DuckStation, that returns `0xFFFF` for every button. The console
  only delivers the actual button bytes when the full 5-byte poll
  sequence is presented; `tx_len` had to be 5, not 4. Two days
  reading the SIO0 datasheet and the DuckStation source to figure out
  which side was wrong. (Both, depending on how you look at it.)

- **Dirty-rect bookkeeping fought the pause menu.** The optimization
  that makes the engine fast on 1992 hardware — only redraw the
  rectangles that changed since last frame — broke when the pause
  menu opened and closed mid-scene. `grRestoreBgTiles` was wiping
  `currDirty` without preserving `prevDirty`, so the resume after
  pause would paint a new frame on top of stale tiles. The fix is a
  helper, `grForceFullRedrawNextFrame`, that the pause-menu close
  path now calls. That's not interesting unless you'd seen the
  artifact, which was a row of Johnnys ghosting across the beach.

- **VRAM corruption across scenes.** The PS1 has 1 MB of VRAM. Sprite
  uploads through `LoadImage` need to be tracked because the GPU does
  not give that memory back when you're done with it. Early builds
  would run one scene fine, transition to the next, and produce
  garbage in the upper half of the framebuffer where a previous
  scene's atlas hadn't been evicted. The eventual fix involves a
  small VRAM allocator with LRU eviction and explicit pinning for
  resources that need to stay resident.

By **early 2026** it was clear the obvious approach — run Sierra's
bytecode on the PS1 — was working in degenerate cases and failing in
real ones. There was a long detour through what came to be called the
*restore pilot* era: an offline pipeline that analyzed every scene
ahead of time, produced "restore specs" declaring which resources and
dirty regions a scene actually needed, clustered them into 34
contracts, and emitted a 1000-line auto-generated C header. That work
was real; it produced 25 scenes that *looked* validated by
March 21, 2026. Then 27 in a research snapshot. Then **63 of 63** on
March 29, in a harness validation log that — read carefully later —
was counting black frames and ocean-only frames as wins.

That 63/63 is now flagged in the project's own status doc as a
*false summit*. It mattered to admit that. The harness was honest
about what it measured; the measurement was the wrong thing.

The pivot came on **2026-04-12**. The realization, which feels
obvious in retrospect and was not obvious at the time, is: *don't run
Sierra's bytecode on the PS1 at all*. Run it on the host. Capture
exactly what pixels get drawn — every `DRAW_LINE`, `DRAW_PIXEL`,
sprite blit, and `PLAY_SAMPLE` event the TTM interpreter emits —
into a small binary file per scene, per tide variant. Ship those
files on the disc. The PS1 runtime becomes a player: it loads the
correct *foreground pack*, walks through the captured draw events,
fires SPU voices on the captured audio cues, and otherwise owns only
the narrow native runtime — background tiles, wave animation, holiday
overlays, input, pause menu.

This is the *hybrid pipeline*. Internally it's called [`fgpilot`]({{ '/docs/glossary/#fgpilot' | relative_url }})
because that's what the directory was named when it was a pilot
experiment that didn't yet work. The name stuck. The project is
gradually migrating it to *PS1 scene playback* in operator-facing
docs. The internal name is in [`foreground_pilot.c`]({{ site.github_url }}/blob/main/src/foreground_pilot.c) and is going to
stay there until someone has time to rename two thousand lines of C.

The first scene that worked, end to end, under this method was
**FISHING 1**. By **2026-04-21** it was pixel-perfect across all
variants — night, low tide, holiday overlay, raft-stage. By
**2026-04-22** it had captured SFX replaying through the SPU with a
3-frame delay constant to align key-on with the visible frame. The
release tag is `v0.3.6-ps1`, commit `f2737253`. That's the moment the
project's status page counts as the first real validated scene.

A second scene, **FISHING 2**, was promoted on **2026-04-24**. As of
the current release `{{ site.release.tag }}`, the count is
**{{ site.release.scenes_validated }} / {{ site.release.scenes_total }}**.

The bar for "validated" is human visual signoff plus audible SFX
signoff, across every applicable variant of a scene. It is not a
machine metric. There is no automated test that decides what passes.
That is on purpose — the harness era proved that automation can be
honest about the wrong question.
</section>

<section class="chapter" markdown="1">
<span class="chapter-num">Chapter 5</span>
## Where it stands now

The disc plays. That's the headline.

`{{ site.release.tag }}` boots on DuckStation, loads from a ~79 MB CD
image, plays every scene the original game routed pixel-perfect with
synced SFX, and idles cleanly through long randomized sessions. The
PS1 executable is around 208 KiB after the legacy ADS/TTM/FG1 runtime
paths were removed; the rest of the disc is FG2/FGP3 foreground pack
payload, the title raw, palette tables, the [ocean ambience]({{ '/docs/glossary/#ocean-ambience' | relative_url }}) VAG, and
the resource map. The full FG2 corpus — 126 packs covering every
scene's high-tide and low-tide variants — is routed onto the CD at
release; what gets loaded at runtime is whichever the screensaver
loop picks next.

Around the validated-scene work, several other surfaces shipped:

**The 36 holidays expansion.** The original game has four baked-in
holiday overlays — Christmas, New Year, Halloween, St. Patrick's Day.
The PS1 port expands that to 36, including movable feasts that need a
real Easter calculation. The Easter algorithm in use here is the
Meeus / Jones / Butcher formulation, accurate from 1900 forward. The
overlay is composited into the foreground replay path so it survives
every scene that ships with a `holiday` variant. The retrospective on
the codegen pipeline is at
[/lab/35-holidays-codegen/]({{ '/lab/35-holidays-codegen/' | relative_url }}).

**Closed captions.** The captions on this port were authored fresh
from scene content. They are not lifted from any prior corpus. There's
a confidence audit at
[caption-audit-2026-04-26.yaml]({{ site.github_url }}/blob/main/docs/ps1/caption-audit-2026-04-26.yaml)
that grades each ADS-tag → caption mapping. Some are high confidence
("Johnny is fishing"). Some are guesses. The guesses are marked.

**The pause menu.** Press Start during any scene and you get an
overlay menu reachable through twelve sub-screens — Scene Set, Scene
Explorer (the chapter-select grid), Freeplay Options, Controls, World
Options, Holidays, Set Island Position, Accessibility, Sound Test,
System, Set Time / Date, and Set RNG Seed. It uses a custom embedded 8x8 ASCII font because PSn00bSDK's
`FntFlush` is empirically broken in the scene-runtime context: the
primitives accumulate but no pixels appear. POLY_F4 quads handle the
dim and the panel chrome. User settings persist to a memory card save
at `bu00:` through [`memcard.c`]({{ site.github_url }}/blob/main/src/memcard.c); the persisted set covers sound mute,
captions, holiday mode, ocean ambience, and the rest of the
pause-menu toggles.

**Audio.** All 23 VAG sound effects are preloaded into SPU RAM at
boot. `soundPlay(nb)` rotates them through 8 round-robin voices.
`SpuSetCommonMasterVolume` is not honored by DuckStation's HLE SPU —
the mute path writes the master-volume registers directly. The VAG
encoder in `scripts/wav2vag.py` and the SPU upload path in
`sound_ps1.c` were debugged over the course of `v0.3.6-ps1`
(commit `355227fa`) for issues including shift-exponent inversion,
ADPCM nibble-pair order, SPU DMA 64-byte alignment, and ADSR1
attack-rate orientation. Each of those was its own afternoon.

**Performance instrumentation.** `ps1_perf.c` emits the level-gated
`JCPERF2` summary on the TTY by default; legacy `JCPERF` is compile-gated
behind `PS1_PERF_LEGACY_TRACE=1`. The level is set by
`ps1PerfSetLevel(OFF / SUMMARY / DETAIL / DEBUG)`. Per-frame state
that used to require visual debugging now also has a text breadcrumb
trail. The [visual telemetry overlay]({{ '/docs/glossary/#telemetry-overlay' | relative_url }}) (5 panels) is still the right
tool for per-frame state; the text logs are for setup and teardown.

What's left is performance polish on a small set of high-leverage
rows, not more scenes. The
[scene ledger]({{ '/scenes/' | relative_url }}) reads
**{{ site.release.scenes_validated }} of {{ site.release.scenes_total }}**
clear under the
[FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }})
across every applicable variant — the daily-loop story is in
[/lab/the-63-scene-grind/]({{ '/lab/the-63-scene-grind/' | relative_url }}).
The
[headless-perf battle card]({{ '/perf/' | relative_url }}) averages
**{{ site.release.perf_target_speed_pct }}% target speed** across
all 126 timing-bearing scene/tide rows; the post-validation
optimization loop that closed the gap from `87.1%` is documented
at [/lab/from-87-to-99-5/]({{ '/lab/from-87-to-99-5/' | relative_url }}).
The remaining gap lives in `VISITOR 3` (high and low) and a yellow
cluster of clean-rect-heavy and wide-action scenes.

Real hardware shipped. The build has been smoke-tested on a
SCPH-7501 via the [TonyHax](https://github.com/socram8888/tonyhax)
softmod path; long-term hardware soak observations are still on
the wishlist. The known limitations section in
[current-status.md]({{ site.github_url }}/blob/main/docs/ps1/current-status.md)
is the honest list of what is broken in ways that the project knows
about.

The disc plays. The work is open source under GPL-3.0 and free at
[github.com]({{ site.github_url }}). The credits screen on the disc
itself reads:

> A labor of love by Hunter Davis.
>
> Hunter does not own or have a license to the Johnny Castaway
> character. The original creator generously allows fan ports.
>
> If you paid for this, you were cheated.
> Open source and free.

That's the whole thing. That's the project. The site's
[Credits page]({{ '/credits/' | relative_url }}) lists the longer
version — the prior ports, the toolchain authors, the algorithm
references. None of this would exist without that bibliography. None
of it would have shipped without one person deciding to keep going
after each milestone turned out not to be the end.

A 1992 screensaver runs on a 1994 console in 2026. The hardware
matched. The bytecode was decoded long ago. The hard part was
admitting which problem the PS1 was actually being asked to solve.
</section>

---

If you want the dated worklogs and the day-by-day progression rather
than the narrative, those live at [/devlog/]({{ '/devlog/' | relative_url }}).
The supporting archaeology — the timeline, the team, the raw source
documents — is split across [the era timeline]({{ '/archaeology/timeline/' | relative_url }}),
[the team page]({{ '/archaeology/team/' | relative_url }}), and
[the primary-source index]({{ '/archaeology/data/' | relative_url }}).
The frozen-era shelves — kept findable so earlier methodologies
don't vanish into commit history — include
[the binary-library era]({{ '/archaeology/binary-library/' | relative_url }})
(the retired heavy-regression corpus),
[the host-script review]({{ '/archaeology/host-script-review/' | relative_url }})
(the host-side identification pipeline as it stood before the
bespoke-scene loop took over),
[the retired scripts]({{ '/archaeology/retired-scripts/' | relative_url }})
(fourteen tools from the binary-library era),
[the retired sources]({{ '/archaeology/retired-src/' | relative_url }})
(three minimal bring-up programs from PS1 first-boot), and
[the vision-classifier artifacts]({{ '/archaeology/vision-artifacts/' | relative_url }})
(reference-bank self-check, pipeline manifest, captured regtest run).
For the exhaustive shelves, use the [source library]({{ '/source/' | relative_url }}),
the [resource catalog]({{ '/resources/' | relative_url }}), and the
[regtest case pages]({{ '/archaeology/regtest-references/cases/' | relative_url }}).
For the "how would I learn from this?" path, use
[/hack/]({{ '/hack/' | relative_url }}).
