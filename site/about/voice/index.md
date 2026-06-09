---
title: Voice
eyebrow: How this site is written
subtitle: The editorial standard behind every page on this domain.
description: Voice and tone guide for the Johnny Castaway PS1 site — the rhythm, vocabulary, and disciplines the prose holds itself to. Written so contributors can match it.
---

This page exists because the site has a deliberate voice and somebody is going to want to write a new page someday. The shorter version of what follows: read the in-game credits screen, then write like that. The longer version is below.

This page is the public distillation. The 450-line internal source-of-truth that drafting agents read before new prose gets generated lives at [`site/_includes/voice-guide.md`]({{ site.github_url }}/blob/main/site/_includes/voice-guide.md) — it's anchored on a verbatim re-read of [hunterdavis.com](https://hunterdavis.com/) and carries the longer rationale for each rule here.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## The voice in one paragraph

First person, plainspoken, often mildly self-deprecating about the artifact and quietly confident about the choice. Sentences are short or medium; long sentences appear, but they earn it by carrying actual content. Numbers carry the weight that adjectives don't. Dead ends are named, often with a one-word lampshade. Realizations are named too. *Johnny Castaway* is treated as a recurring craft obsession, not a one-time curiosity. The phrases **"a labor of love"** and **"if you paid for this, you were cheated"** come straight off the in-game Credits screen and are load-bearing — every prose surface on this site has to match them, not the other way around.

## The drawCredits floor

The irreducible voice sample is the four-line in-game Credits screen, in `src/pause_menu/pause_menu.c::drawCredits`:

> A labor of love by Hunter Davis.
>
> Hunter does not own or have a license to the Johnny Castaway character. The original creator generously allows fan ports.
>
> If you paid for this, you were cheated. Open source and free.
> github.com/huntergdavis/johnny-castaway-ps1

If a sentence on this site fails to make sense alongside that block, it's wrong. That's the whole test.

## Traits to keep

- **First person singular.** "I", almost always. "We" only when speaking for an actual lineage (jno6809, JCOS, the toolchain authors), and even then "I" is often clearer.
- **Lists with conversational bodies.** Numbered lists in particular, with items that read like sentences someone actually said, not bullet-deck fragments.
- **Show the constraint, then the realization.** Walk the reader through *why* something was hard before reporting that it worked.
- **Specific numbers.** `64 MB`, `350 KB`, `2 watchers, 8 stars, 1 fork (mine)`. Numbers are the voice's currency.
- **Parentheticals as asides.** `(remember those!)`, `(mine)`. They're how the voice breathes.
- **Em-dashes and short interruptions.** Whole sentences are allowed to end on a comma into another short clause.
- **Honest scope statements.** "Out of scope for this initial release" — followed by an actual list. Never "stay tuned!"
- **Self-deprecation that's also confidence.** Diminutive language about the thing, absolute confidence about the choice. The Credits screen is the canonical example.
- **Sentence fragments as topic markers.** "Conway's Law." Full stop. Then the elaboration.
- **Negation as definition.** *Not X. Y.* This pattern already runs throughout the site ("Not an emulator. […] It replays packs.") and is correct — it's the native rhythm.
- **Specific named technical vocabulary, no scaffolding.** `FntFlush`, dirty-rect bookkeeping, SPI tx_len, ADS / TTM bytecode, CLUT-indexed sprite formats. The reader will follow.
- **Comma splices.** Don't "fix" the conversational comma splices into formal prose; they *are* the rhythm.

## Traits to avoid

- **Performative enthusiasm.** *delighted*, *thrilled*, *excited to announce*. Announce things by saying *"Announcing X"* and describing what X does in the second sentence.
- **Marketing transitions.** *But that's not all!* / *Without further ado* / *Now let's see* / *Let's dive in* / *Stay tuned* / *We've got something special*. None of these appear on the project's source blog. None should appear here.
- **Hyperbolic adjectives.** *elegant*, *powerful*, *seamless*, *blazing-fast*, *world-class*, *enterprise-grade*, *industry-leading*, *revolutionary*, *stunning*. The space these occupy is very narrow, and the project's actual prose ("rock-solid 60 FPS", "splendidly", "criminally under-exposed") is not in it.
- **Triplets and corporate parallelism.** *It's fast, it's flexible, it's free* — the cadence is fine when the items are diminutive ("Useless, silly, simplistic"), wrong when used to brag.
- **"We" used to fake a team.** There is one author. The Credits screen says so.
- **Apologizing for scope or pace** — *just a small project*, *still rough*, *early days*. Small things are shipped on purpose and framed as small joys; the prose should match.
- **Flattening technical concepts** further than the source voice would. The project's blog posts publish C, talk about VRAM in 1 MB, list ADPCM nibble pair order. The site doesn't need to oversimplify.
- **Roadmapping the page.** *In this post we'll explore…* — just write the page.
- **"Journey" as a metaphor for the project arc.** Overused everywhere, never on this site.
- **Closing rhetorical questions.** *Pretty cool, right?* / *Sound familiar?* Inside-paragraph rhetorical questions as setup for technical answers ("Why? Because the SPU HLE doesn't honor it.") are fine. Closing on one is what to avoid.

## Phrases that fit

These are paraphrasable lines from the project's source voice. Cite them, echo their cadence, or use the ones that match the page's content.

- *"A labor of love by Hunter Davis."*
- *"If you paid for this, you were cheated. Open source and free."*
- *"didn't take me too long to port. It's working splendidly!"*
- *"A glorious internal 640x480 screen resolution and a rock-solid 60 FPS"*
- *"Lightbulb moment!"*
- *"Bummer, SDL 2.0 doesn't support console framebuffer any longer."*
- *"useless, silly, simplistic, exactly what I had envisioned"*
- *"Now is the time for small joys, for silver linings."*
- *"little things that bring me joy. Things I might dip in and add a feature to once a year, for 30 years."*
- *"So few, so laughably few. But hey, enough to honestly help in some situations."*
- *"criminally under-exposed for how good it is"* (about jc_reborn — the seed for this entire project)
- *"Out of scope for this initial release"* (followed by an actual list)
- *"Pretty straightforward!"*
- *"And that's about it!"*
- *"Hey, progress is progress :D"*

## Words to skip

A short banned-word list, with the reason rather than the prohibition:

- *delightful, delighted, thrilled, excited* — the voice doesn't perform enthusiasm; it describes what got made.
- *elegant, powerful, seamless, blazing-fast, world-class, best-in-class, enterprise-grade, industry-leading, mission-critical, battle-tested* — sales vocabulary. This is a fan port, not a product.
- *journey* (as a noun for the project arc), *unlock* (as a verb for *enable*), *empower, leverage, robust, cutting-edge, ecosystem, synergy, stakeholders, holistic* — corporate vocabulary; off-voice.
- *experience the magic, fall in love, next level* — none belong here.
- *modern web standards, best practices* — vague appeals to authority. State the specific practice or skip it.
- *just a small project, still rough, early days* used apologetically — small things ship on purpose. Match that.

## How expertise is handled

The project author has a long technical CV. None of it leads. The blog posts open with what the new thing *is* and *why somebody wanted to make it*, and credentials show up by accident, embedded in detail. Site prose carries the same posture: hardware constraints stated as plain facts; named technical vocabulary used without scaffolding; mistakes reported as facts, named precisely. The voice never says *"I'm an expert"* — it just writes the way an expert writes: dense, specific, declarative, no padding.

Concretely:

- The home page doesn't get a *"by Hunter Davis, who has ported Johnny to N platforms"* bio block.
- The [Credits]({{ '/credits/' | relative_url }}) page lists prior ports as part of the lineage, links hunterdavis.com once as the longer version, and stops.
- The [History]({{ '/about/history/' | relative_url }}) page can mention *"this isn't Hunter's first port of Johnny"* because the prior ports literally explain why the PS1 was tractable. That's content, not bio.
- When prose needs to gesture at duration, prefer round numbers and *"about"*. Avoid spurious precision (*29.4 years*, *since 1995-04*).

## How dead ends are handled

Out loud, often with a one-word lampshade. *"Bummer."* *"Lightbulb moment!"* The full arc — *here's what I tried, here's what didn't work, here's what made me realize the right thing* — is a recurring shape on the project's blog and shows up on this site as the reason the [archaeology]({{ '/archaeology/' | relative_url }}) section exists. Dead ends are content. They aren't apologies.

## A test for any new sentence

Three quick checks before publishing prose on this site:

1. Could this sentence appear, unedited, on hunterdavis.com? If not, it's off-voice.
2. Does it sit comfortably next to *"if you paid for this, you were cheated"*? If the contrast is jarring, the sentence is the problem.
3. Did it earn its length? Long sentences are fine; long sentences without specific content are not.

That's it. The disc plays. That's what mattered.

## Further reading

- [The voice anchor problem]({{ '/lab/voice-anchor-problem/' | relative_url }})
  — the long-form retrospective this guide is the public-facing
  distillation of. Read this guide first, that essay if you want
  the why behind the rules.
- [`site/_includes/voice-guide.md`]({{ site.github_url }}/blob/main/site/_includes/voice-guide.md)
  — the 450-line internal source-of-truth this page is distilled
  from. Carries the full per-rule rationale, the original
  hunterdavis.com 2026-04-26 anchor notes, and the specific phrases
  that earned their way into the canonical list. Drafting agents
  read this before generating new prose for the site.
- [AI sub-agents on this project]({{ '/docs/agents/' | relative_url }})
  — what gets drafted by an LLM and what doesn't, with the voice
  guide as the editing-pass standard.
- [Glossary: drawCredits]({{ '/docs/glossary/#drawcredits' | relative_url }})
  — the four-line in-game credits text the whole voice anchors
  on.
