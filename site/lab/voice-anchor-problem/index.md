---
layout: page
title: The voice anchor problem
eyebrow: Lab . Prose engineering
subtitle: Keeping a giant AI-assisted site from sounding like a brochure.
description: A prose-process essay about the Johnny Castaway PS1 site's voice guide, drawCredits anchor, Hunter Davis writing style, and AI-drafted documentation review.
date: 2026-04-26
---

## The problem

AI prose has a smell.

It is not always bad. Sometimes it is clear and useful. But it has tells:
"seamless", "deep dive", "robust", "journey", "we're excited", "this
showcases". It rounds off every edge. It hates saying "this failed." It loves
ending paragraphs with a little trumpet.

That is not the voice of this project.

The voice of this project is the pause-menu Credits screen:

> A labor of love by Hunter Davis.
> Hunter does not own or have a license to the Johnny Castaway character.
> The original creator generously allows fan ports.
> If you paid for this, you were cheated. Open source and free.

Short. Plain. A little funny. Legally careful without sounding like a lawyer.
That is the anchor.

## Why it matters

The site is large now. About pages, archaeology chapters, 63 scene pages, 36
holiday pages, lab essays, generated source wrappers, regtest case pages. Some
of that prose started as agent drafts. Some of it came from old worklogs. Some
of it is generated from structured files.

Without a voice anchor, the site becomes a committee. The About page sounds
like a museum placard. The Lab sounds like a conference talk. The docs sound
like a SaaS onboarding funnel. Nobody wants that. I certainly don't.

So the voice guide exists. It is not public navigation. It is an internal
tool, checked into `site/_includes/voice-guide.md`, read before new prose gets
written.

## The Hunter shape

The source samples are my project posts: Dreamcast, embedded Linux live CD,
text edition, weird one-off tools, the dunking bird. The pattern is pretty
consistent:

- first person
- specific numbers
- named constraints
- dead ends admitted
- jokes that are small, not performative
- no corporate "we"
- no fake excitement
- no apology for small projects

The rhythm is: I tried a thing. It broke. Bummer. I tried the next thing.
Lightbulb moment. Here are the files. Here is the release. If you paid for
this, you were cheated.

That works for Johnny because Johnny is small and serious at the same time.
The island is silly. The port is not.

## Editing the agents

The first AI draft usually gets the facts mostly right and the voice mostly
wrong. That is fine. The first draft is not the artifact.

The edit pass removes:

- fake "we"
- superlatives
- rhetorical endings
- claims not tied to source files
- paragraphs that say "this is important" instead of showing the mechanism
- cute transitions

Then the page gets more numbers. More filenames. More "what failed." More
"what I actually did."

The result still may have started as an AI draft. It should not read like one.

## Generated pages have a voice too

Even the generated source-library pages have to talk like the rest of the
site. They say "this is the shelf page" instead of "welcome to an exciting
overview." They name the source path. They say whether a file is active docs,
archive, or generated archaeology. They do not pretend a generated wrapper is
hand-authored prose.

That honesty is part of the voice.

## The rule

If a sentence would look stupid on the PS1 pause menu, it probably does not
belong on the site.

Not every sentence has to be short enough to fit there. This page itself
would be unreadable in 30-character lines. But the spirit has to survive that
compression. Plain. Specific. Free. Slightly annoyed by nonsense.

That's the voice anchor problem, and that is the solution.

## Cross-links

- [Credits]({{ '/credits/' | relative_url }})
- [AI sub-agents on this project]({{ '/docs/agents/' | relative_url }})
- [The LLM pass]({{ '/lab/llm-pass/' | relative_url }})
- [About Hunter Davis](https://hunterdavis.com/)
