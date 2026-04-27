---
layout: page
title: The LLM pass
eyebrow: Lab . Advanced development techniques
subtitle: How I use AI sub-agents on a one-person PS1 port without pretending they are magic.
description: "A methodology essay on the LLM-assisted development workflow behind Johnny Castaway PS1: parallel agents, review loops, hallucination control, and where the human stays in charge."
date: 2026-04-26
---

## The honest shape of it

This project uses LLMs. A lot.

Not in the hand-wavy way where somebody says "AI-powered" and then points
at a chatbot. I mean the practical, slightly boring version: I ask a model
to read six files while I read six different files. I ask one agent to draft
the first cut of 63 scene pages while another extracts timing data. I ask a
third to check whether the first two contradicted the code. Then I read the
diff, delete the confident nonsense, keep the mechanical work that is right,
and ship the part I am willing to sign my name to.

That is the LLM pass. It is not an author. It is not a maintainer. It is a
very fast junior engineer with infinite patience, no project memory unless I
give it one, and an alarming willingness to invent a function that does not
exist if I ask the question poorly.

## Why it helps here

Johnny Castaway PS1 is a strange target for AI assistance because the hard
part is not one giant algorithm. The hard part is surface area.

There are Sierra resource formats, ADS scripts, TTM animation commands,
PSn00bSDK GPU rules, SPU sample layout, CD-ROM seek behavior, Docker image
setup, DuckStation TTY logging, 63 scenes, 36 holidays, captions, pause menu
state, memory-card persistence, and a regtest harness that grew like ivy over
four months. A human can understand all of it, but not all at once.

Sub-agents are useful when the work decomposes cleanly:

1. One agent inventories `docs/ps1/research/generated`.
2. One agent drafts per-scene pages from `site/_data/scenes.yml`.
3. One agent audits a holiday table against `holidays.yml`.
4. One agent runs a build and reports the real compiler output.
5. I stay on the critical path: reading the code, making the judgment calls,
   and deciding what ships.

The win is parallel attention, not replacement judgment.

## The loop

The working loop looks like this:

1. Write down the artifact I want: "a page for every holiday", "a caption
   audit", "a source-library wrapper for every Markdown file".
2. Give the agent the source files and the rules. Not vibes. Files and rules.
3. Let it produce a first draft or a bounded patch.
4. Build, run, or render the artifact.
5. Read the output as if a stranger submitted it.
6. Keep what is true. Rewrite what is merely fluent.

That last step is the whole thing. If you skip it, the project rots.

An LLM will happily write a beautiful paragraph about a PS1 DMA path that the
code does not use. It will write a CLI flag that sounds plausible. It will
name an "existing" helper because the surrounding helpers suggest the name
should exist. It is not lying. It is doing autocomplete with a law degree.

So the review pass is not optional. It is the price of admission.

## What it did well

Captions were a perfect first job. The source material was visual and finite:
63 scenes, each with a short gag. The agent could draft a caption corpus and
attach confidence levels. I could then review the lines in the emulator and
replace the ones that inferred from the scene family instead of the actual
animation.

Holiday emblems were similar. The agent was good at generating small,
palette-locked visual primitives: a football, a tiny chalkboard with pi, a
leaf, a little guitar. It was bad at remembering that the sprite is an overlay
and not a whole island scene. The review loop caught that. The final output is
not "AI art" in the lazy sense; it is a generated sprite sheet constrained by a
very specific runtime format and then reviewed cell by cell.

This website is the third example. Agents can write the first draft of a
history page quickly. They can also write three paragraphs of museum-label
prose that sound like a school district grant application. The voice guide is
there because the first draft is never the final draft.

## What it did badly

Three recurring failures:

- **Confident nonexistence.** The model describes code that would be nice to
  have, not code that exists.
- **Tone drift.** It slides toward "we are excited" prose unless the voice
  anchor is repeated over and over.
- **False completion.** It says "all scenes" when it touched the index and
  not the per-scene pages.

The mitigation is boring: source links, grep, build logs, generated indexes,
and a human who refuses to merge a pretty sentence until it survives contact
with the tree.

## The useful mental model

Treat an LLM pass like a compiler pass.

It takes structured input. It emits an artifact. The artifact is not trusted
until the next pass verifies it. Some passes optimize. Some passes lint. Some
passes produce garbage and get thrown away. Nobody asks the compiler whether
the program is worth writing.

That is the arrangement here. The agents increase throughput. They do not own
the taste, the release bar, the legal posture, or the Credits screen.

## Cross-links

- [AI sub-agents on this project]({{ '/docs/agents/' | relative_url }})
- [Hallucination engineering]({{ '/lab/hallucination-engineering/' | relative_url }})
- [The voice anchor problem]({{ '/lab/voice-anchor-problem/' | relative_url }})
- [The dunking bird]({{ '/lab/dunking-bird/' | relative_url }})
- [Source library]({{ '/source/' | relative_url }})
