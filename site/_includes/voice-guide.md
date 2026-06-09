<!--
  voice-guide.md
  ---------------------------------------------------------------
  Internal voice + tone reference for the Johnny Castaway PS1
  site. Not rendered. Layouts never include this file. Read
  before writing any new prose for the site.

  This guide is anchored on a verbatim re-read of hunterdavis.com
  on 2026-04-26. The previous draft was anchored on his
  leadership-writing posts only, which understated his project
  voice; this revision restores the project-post register
  (announcements, port writeups, dead-end lampshades) which is
  the closer match for what most of this site is doing.
  ---------------------------------------------------------------
-->

---
# Voice & tone guide — internal reference, not rendered.
---

# Hunter Davis voice guide

## What this is

Distilled from a re-read of hunterdavis.com on 2026-04-26. URLs
fetched (in order; ones that returned the strongest verbatim
prose are starred):

- `https://hunterdavis.com/` — homepage / blog index
- `https://hunterdavis.com/page2/` — second page; the 2021
  Johnny Castaway porting cluster lives here
- `https://hunterdavis.com/about/` — 404
- `https://hunterdavis.com/my-story/` — 404
- *`https://hunterdavis.com/2021/03/10/johnny_castaway_native_port_and_livcd.html`
  — Johnny Castaway native live CD (the strongest single voice
  sample for this site's subject matter)
- *`https://hunterdavis.com/2024/12/29/announcing-tps.html`
  — Team Planning Simulator announcement (best short cadence
  sample)
- *`https://hunterdavis.com/2024/11/07/announcing-tui000.html`
  — Tui000 announcement (Johnny Castaway lineage, "small joys")
- `https://hunterdavis.com/2024/11/16/announcing-labrync.html`
- `https://hunterdavis.com/2025/10/05/a-bunch-of-one-off-projects-for-2025.html`
- `https://hunterdavis.com/2026/03/21/announcing-dunking-bird.html`
  — also references the PS1 Johnny port directly
- `https://hunterdavis.com/2021/03/21/johnny_dreamcastaway_released.html`
- `https://hunterdavis.com/2021/12/11/johnny-castaway-text-edition.html`
- `https://hunterdavis.com/2020/12/20/hacking-atari-pong-jr-mini.html`

In-house anchors:

- `src/pause_menu/pause_menu.c::drawCredits` — the four-line in-game credits
  text. This is the irreducible voice: what survives at ~30 chars
  per line on a PS1 panel.
- `docs/ps1/website-plan.md` § 5 — the project's existing
  drawCredits-based voice anchor.

Use this guide as the voice anchor for any new prose authored on
this site. Read it BEFORE writing the next page.

## The anchor in one paragraph

Hunter writes the way he talks. First person, plainspoken, often
mildly self-deprecating about the artifact and quietly confident
about the choice. Sentences are short or medium; long sentences
appear, but they earn it by carrying actual content. Numbers
carry the weight that adjectives don't. Every project post on
hunterdavis.com has a numbered list inside it where the items
read like spoken asides ("Mouse support! Not likely.", "Colors
are… supported"). Dead ends are named, often with a one-word
lampshade ("Bummer,…"). Realizations are named too ("Lightbulb
moment!"). Johnny Castaway is treated as a recurring craft
obsession, not a one-time curiosity. The phrases "labor of love"
and "if you paid for this, you were cheated" come straight off
the in-game Credits and are load-bearing — the site's voice has
to match them, not the other way around.

## Specific traits to keep

- **First person singular.** "I", almost always. "We" only when
  speaking for an actual lineage (jno6809, JCOS, the toolchain
  authors), and even then "I" is often clearer.
- **Lists with conversational item bodies.** Numbered lists in
  particular, with items that read like sentences someone
  actually said, not bullet-deck fragments.
- **Show the constraint, then the realization.** The Dreamcast
  live-CD post literally says "Lightbulb moment!" twice and
  walks the reader through the constraint first. The PS1 port
  surfaces should be willing to do the same.
- **Specific numbers.** "64 mb", "350 KB", "640x480", "2 watchers,
  8 stars, 1 fork (mine)". Numbers are the voice's currency.
- **Parentheticals as asides.** "(remember those!)", "(mine)",
  "(SDL 1.2 only allowed static linking if you buy a pro
  license, boo!)". They're how the voice breathes.
- **Em-dashes and short interruptions.** "Bummer, SDL 2.0 doesn't
  support console framebuffer any longer." Whole sentences end
  on a comma into another short clause.
- **Honest scope statements.** "Out of scope for this initial
  release" followed by an actual list. Never "stay tuned!"
- **Self-deprecation that is also confidence.** "useless, silly,
  simplistic, exactly what I had envisioned" is the ideal
  cadence — diminutive language about the thing, absolute
  confidence about the choice.
- **Sentence fragments as topic markers.** "Conway's Law." Full
  stop. Then the elaboration. (From the leadership writing —
  carries over to project posts too.)
- **Negation as definition.** "Not X. Y." This pattern already
  appears on the site ("Not an emulator. […] It replays packs.")
  and is correct — it's Hunter's native rhythm.
- **"In my experience" framing without the words.** Assert from
  observation, not from sources. State engineering claims as
  observed truths.
- **Comma splices and conversational rhythm.** Don't "fix" the
  conversational comma splices in his voice into formal prose;
  they are the rhythm.
- **Specific named technical vocabulary, no scaffolding.**
  `FntFlush`, dirty-rect bookkeeping, SPI tx_len, ADS/TTM
  bytecode, CLUT-indexed sprite formats. The reader will
  follow.
- **The prior-ports lineage when load-bearing.** When the PS1's
  choices are inherited from the Dreamcast / RetroFW / live CD
  / text edition era, name them. Don't itemise on every page.
- **`drawCredits` floor.** Any sentence on this site that fails
  the drawCredits register check is wrong. "A labor of love"
  and "If you paid for this, you were cheated. Open source and
  free." can both recur on the chrome surfaces.

## Specific traits to avoid

- **"We're excited to announce…"** / "delighted" / "thrilled" —
  not the voice. Hunter announces things by saying "Announcing
  X" and describing what X does in the second sentence.
- **Rhetorical questions ending paragraphs** ("Pretty cool,
  right?", "Sound familiar?"). Inside-paragraph rhetorical
  questions as setup for technical answers ("Why? Because the
  SPU HLE doesn't honor it.") are fine. Closing on one is what
  to avoid.
- **Marketing transitions.** "But that's not all!", "Without
  further ado", "Now let's see", "Let's dive in", "Stay tuned",
  "We've got something special". None appear on hunterdavis.com.
  None should appear here.
- **Hyperbolic adjectives.** "elegant", "powerful", "blazing-
  fast", "world-class", "seamless", "delightful", "battle-
  tested", "enterprise-grade", "lightning quick", "industry-
  leading", "revolutionary", "stunning". The space these occupy
  is very narrow, and Hunter's prose ("rock-solid 60 FPS",
  "splendidly", "criminally under-exposed") is not in it.
- **Triplets and corporate parallelism** — "It's fast, it's
  flexible, it's free." Hunter's "Useless, silly, simplistic"
  uses the same beat to describe a thing diminutively, not to
  brag.
- **"We" used to fake a team.** There is one author. The credits
  page on the disc says so.
- **Apologizing for scope or pace** ("just a small project",
  "still rough", "early days"). Hunter ships small things on
  purpose and frames them as small joys; the site matches.
- **Flattening technical concepts** further than Hunter would.
  He writes C in his blog posts. He talks about VRAM in 1 MB.
  He lists ADPCM nibble pair order. The PS1 port pages don't
  need to oversimplify.
- **"In this post we'll explore…"** No roadmapping the page;
  just write the page.
- **"Journey" as a metaphor for the project arc.** Overused.
  Not in the source samples.

## Idioms / phrases / words Hunter actually uses

Direct quotes from hunterdavis.com posts (verbatim, in quotes):

- "A labor of love by Hunter Davis." (drawCredits, in
  `pause_menu.c`)
- "If you paid for this, you were cheated. Open source and free."
  (drawCredits)
- "didn't take me too long to port. It's working splendidly!"
  (Dreamcast post)
- "A glorious internal 640x480 screen resolution and a rock-solid
  60 FPS"
- "DreamSDK really is quite nice as far as development
  environments go."
- "Lightbulb moment!" (live-CD post; used twice)
- "Bummer, SDL 2.0 doesn't support console framebuffer any
  longer."
- "I love the Sega Dreamcast. So Much."
- "I had already ported Johnny to as many platforms as I was
  interested in. What I really missed was that feeling of fun
  and writing software."
- "Some of my best work was made famous not by my original
  intention. Rather, it was the creativity of others that
  brought purpose to 'meaningless' software."
- "I keep coming back to the terminal."
- "useless, silly, simplistic, exactly what I had envisioned"
- "Now is the time for small joys, for silver linings."
- "little things that bring me joy. Things I might dip in and
  add a feature to once a year, for 30 years. That's what my
  internet is all about."
- "So few, so laughably few. But hey, enough to honestly help
  in some situations."
- "It falls squarely into the bucket of 'can't you just use a
  spreadsheet?' Yes. Of course you can"
- "The humble spreadsheet will outlive us all."
- "Sometimes you just write a quick simulator on a Sunday night
  to help you clear your head"
- "I'm committing to releasing more things that spark joy"
- "Hey, progress is progress :D"
- "criminally under-exposed for how good it is" (about
  jc_reborn — the seed for this entire project)
- "this is a fully baked re-implementation"
- "Immediately made me smile in a way I hadn't in some time."
- "I did what anyone in my situation would do, I set out to
  cross-compile"
- "Why? These re-implementations used resource-intensive
  high-level languages and libraries."
- "the ole T.P.S. report"
- "Out of scope for this initial release" (followed by an
  actual list)
- "Pretty straightforward!"
- "And that's about it!"
- "Anyway, [next thing]"
- "And hey, [next sentence]"
- "Yeah." (as a one-word sentence, often paragraph-opening)
- "Also ported to python cause why not I guess?"
- "Colors are… supported"
- "I've been enjoying and porting Johnny Castaway to new
  systems for many years now." (text-edition post)
- "I've been programming for about 30 years now" (about-me
  cadence — "about" + round number, not "29.4 years")

## Words / phrases that don't fit

Banned-word list, with rationale:

- *delightful, delighted, thrilled, excited* — Hunter doesn't
  perform enthusiasm; he describes what he made.
- *elegant, powerful, seamless, blazing-fast, lightning quick,
  rock star, world-class, best-in-class, enterprise-grade,
  industry-leading, mission-critical, battle-tested* — sales
  vocabulary. This is a fan port, not a product.
- *journey* (as a noun for the project arc), *stack* (in the
  "tech stack" sense as branding), *unlock* (as a verb meaning
  enable), *empower, leverage, robust, cutting-edge,
  ecosystem, synergy, stakeholders, holistic* — corporate
  vocabulary, off-voice.
- "But that's not all!", "Without further ado", "Now let's
  see", "Let's dive in", "We've got something special",
  "Stay tuned" — marketing transitions. Unused on
  hunterdavis.com. Unwelcome here.
- "experience the magic", "fall in love", "next level" — none
  belong on this site.
- "we're proud to" / "we're excited to" / "I'm proud to share" —
  Hunter announces by describing.
- "Modern web standards", "best practices" — vague appeals to
  authority. State the specific practice or skip it.
- "Just a small project / just a little thing" used
  apologetically. Hunter says "small joys" celebratorily. The
  site should match.

## How Hunter handles his expertise

Hunter has a long technical CV: ~30 years of programming, a
Master's in CS (Indiana, scientific computing + AI), VP-level
engineering leadership, and a hobby track record that includes
multiple prior Johnny Castaway ports. He does not lead with any
of this on his blog. He leads with what the new thing is and
*why he wanted to make it*, and the credentials show up by
accident, embedded in detail. The Dreamcast post is one sentence
in before he says "I love the Sega Dreamcast. So Much." — and
that lands as enthusiasm, not qualification.

Carry that confidence in the prose. Hardware constraints can be
stated as plain facts. Named technical vocabulary
(`FntFlush`, dirty-rect bookkeeping, SPI tx_len, ADS/TTM,
CLUT-indexed sprites) does not need scaffolding. Mistakes are
reported as facts, named precisely. The voice never says "I'm
an expert"; it just writes the way an expert writes — dense,
specific, declarative, no padding.

For this site:

- The home page does NOT get a "by Hunter Davis, who has ported
  Johnny to N platforms" bio block. That would lead with
  credentials, which Hunter doesn't do.
- The Credits page lists the prior ports as part of the
  lineage, links hunterdavis.com once as the longer version,
  and stops.
- The History page can mention "this isn't Hunter's first port
  of Johnny" because the prior ports literally explain why the
  PS1 was tractable. That's content, not bio.
- "About 30 years now" cadence — when prose needs to gesture at
  duration, prefer round numbers and "about". Avoid spurious
  precision ("29.4 years", "since 1995-04").

## How Hunter handles failure / dead ends

He names them out loud, often with a one-word lampshade. Direct
samples:

- "Bummer, SDL 2.0 doesn't support console framebuffer any
  longer."
- "Mouse support! Not likely. Export SDL_NOMOUSE=1"
- "I've found out something that many Dreamcast developers know
  very well, it's very hard to debug i/o in an emulator!"
- "These re-implementations used resource-intensive high-level
  languages and libraries. They wouldn't run particularly well
  on a 25 year old PC!"

The PS1 port has more dead ends than any prior project — the
restore-pilot era, the 63/63 false summit, the spicyjpeg
tx_len=4 debacle, the FntFlush fight, the printf-corrupts-the-
runtime stretch. None get hidden. Voice rule: if a dead end cost
real time, it gets at least one sentence, and the sentence does
not apologize. A comma-spliced "Bummer," is on-voice. "Stay
tuned for fixes!" is not.

## How Hunter handles Johnny Castaway specifically

This is a deep cut. Quote evidence:

- 2021 cluster: he was already porting Johnny to multiple
  platforms — Dreamcast, RetroFW handhelds (LDK, RG-300, RS97),
  embedded Linux live CD bootable on a 2001 Thinkpad with 256
  mb of RAM, "half-working ports" on Xbox, and a bash text
  edition that runs on receipt printers.
- "I've been enjoying and porting Johnny Castaway to new
  systems for many years now." (text-edition post,
  2021-12-11)
- "I had already ported Johnny to as many platforms as I was
  interested in. What I really missed was that feeling of fun
  and writing software." (Tui000 post, 2024)
- The live-CD post calls jc_reborn "criminally under-exposed
  for how good it is (2 watchers, 8 stars, 1 fork (mine))" —
  the whole lineage in one parenthetical.
- The Tui000 post directly cites Johnny as inspiration for why
  a "passive game / screensaver" is worth making at all. The
  shape of his appreciation: small, looping, doesn't ask
  anything of you, brings small joys.
- The Dunking Bird post (2026) names "developing a PS1 version
  of Johnny Castaway for approximately one year" as the
  motivating project. The PS1 port is the latest entry in a
  practice that was already well underway.

The PS1 port is *another* expression of an existing practice,
not a novel one-off. The voice should imply continuity, not
novelty. Avoid "I'm excited to announce" or "this is my new
project." Prefer "this is the PS1 entry in a long line of
Johnny ports" or — more often — just describe the work without
framing it as a debut.

When the site talks about Johnny:

- Don't frame it as "Sierra's cute screensaver" or "a charming
  little screensaver." Hunter's relationship is more specific.
- "Small joys" is on-voice and on-theme. So is "watch Johnny
  on the island."
- Talking about which prior ports Hunter has done is allowed
  and useful. Talking about Hunter's reasons (small joys, the
  feeling of fun and writing software, the terminal) is also
  allowed.
- jno6809's `jc_reborn` should be credited generously every
  time it comes up. That generosity is part of the voice.

## Three sample paragraphs that sound like Hunter writing about this PS1 port

Gold-standard reference. Not shipped on the site verbatim;
content agents writing other paragraphs should aim for this
register.

> A friend asked why the PS1 and not, say, the Dreamcast — fair
> question, since I already ported Johnny to the Dreamcast in
> 2021 (and to embedded Linux, and to a handful of RetroFW
> handhelds, and to a bash script that runs on receipt
> printers). The honest answer is that the PS1's GPU is unlike
> anything else I'd shipped Johnny on. No CPU framebuffer, no
> `LoadImage` you can write to and forget, no SDL underneath
> you. You build an ordering table of primitives and the GPU
> draws what's in it. Figuring out how a 1992 Sierra
> screensaver fits inside that model was the part I wanted to
> know.

> The first prototype tried the obvious thing. Take jno6809's
> beautiful `jc_reborn` (still criminally under-exposed for
> how good it is), swap SDL2 for PSn00bSDK, swap `fopen` for
> `CdRead`, and let the existing ADS/TTM interpreter run on
> the MIPS R3000A. Builds, boots, draws something. And then
> everything goes wrong in slightly different ways for two
> months. The interpreter trusted state the PS1 didn't have.
> The pad library didn't auto-poll. The font path was
> empirically broken in scene context. Bummer.

> Lightbulb moment, eventually: don't run Sierra's bytecode on
> the PS1 at all. Run it on the host, like every prior port
> did, but capture every visible draw and every `PLAY_SAMPLE`
> event, encode the result into a small per-scene binary, ship
> those binaries on the disc. The PS1 becomes a player, not an
> interpreter. That's the shape of FG2 packs and the shape of
> the rest of the project. It is not the elegant solution. It
> is the one that closes.

## Audit-and-edit checklist

When passing over an existing site page, weigh against this
order. If a sentence fails, edit it.

1. **Does this sentence sound like Hunter writing?** If it
   sounds like a generic devblog or a press release, rewrite.
   Aim for declarative + observed + occasionally aphoristic.
2. **Does it imply Hunter is doing another expression of an
   existing practice?** If it implies novelty, debut, or
   beginner's uncertainty, rewrite. The PS1 port is a
   continuation.
3. **Does it use named technical vocabulary correctly?** If it
   hedges or generalizes ("the system", "the hardware"), name
   the actual thing.
4. **Would it survive the drawCredits floor?** If you imagine
   the sentence shrunk to a four-line panel at ~30 chars per
   line, does it still mean what you wanted? If it would
   disappear, it was probably padding.
5. **Marketing-vocabulary scan.** Strip the banned-word list.
6. **Comma-splice / negation patterns preserved.** Don't "fix"
   conversational comma splices and "Not X. Y." constructions.
   They are the native rhythm.
7. **Rhetorical-question scan.** No paragraph closes on a
   rhetorical question.
8. **Triplet scan.** A trio of adjectives or short clauses is
   only allowed if it is diminutive ("useless, silly,
   simplistic"), not promotional.

## Anti-patterns

- "We're excited to announce…" — never. Even "Excited to
  share…" is wrong here.
- "But that's not all!" / "Without further ado" / "Let's dive
  in" / "Stay tuned!" — never.
- Rhetorical questions ending a paragraph — never.
- "Delighted", "powerful", "elegant solution", "blazing-fast",
  "world-class", "best-in-class", "industry-leading",
  "mission-critical", "battle-tested" — never.
- Marketing-speak transitions and corporate-deck triplets —
  never.
- "We" used to fake a team — never.
- Apologizing for the project's smallness — never.
- Flattening technical concepts further than the source
  warrants — never.
- "Sierra's cute screensaver" / "Sierra's adorable little
  game" — too distant. The relationship is more specific than
  that.
- "Modern web standards" / "best practices" / "industry-
  leading" — vague appeals to authority.
- "Journey" as a project metaphor — overused; off-voice.

<!-- end voice-guide.md -->
