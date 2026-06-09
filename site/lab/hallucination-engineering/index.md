---
layout: page
title: Hallucination engineering
eyebrow: Lab · Advanced development techniques
subtitle: The failure mode is not that the model is weird. The failure mode is that it sounds normal.
description: "A practical essay on controlling LLM hallucinations in a real C/PS1 port: source-first prompts, generated indexes, build gates, and review discipline."
date: 2026-04-26
image: /assets/img/walkstuf3-ps1-jog.png
image_alt: WALKSTUF 3 — Johnny mid-stride at night doing exercise laps. The canonical caption-mismap scene; the kind of confident-wrong answer the essay names. Captured on PS1 hardware via the validation harness.
image_width: 961
image_height: 720
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## The expensive kind of wrong

The dangerous LLM answer is not the surreal one. The dangerous answer is the
one that almost looks like the codebase.

It names `foregroundPilotRuntimeCaptionStart()` because the surrounding code
has `foregroundPilotRuntimeStart()`, `foregroundPilotRuntimeAdvance()`, and
`foregroundPilotRuntimeCompose()`. It suggests a `--holiday-id` flag because
there is a `holiday` boot token and a pause-menu override. It says the memory
card saves captions because that would be a sensible thing to save.

All of those can be false. They are not obviously false. That's why they cost
time.

Hallucination engineering is the practice of making that failure mode cheap
to detect.

## Source first

The first rule is simple: the agent has to read the files.

Not "you know C". Not "this is probably a Jekyll site". The actual files.
[`src/pause_menu/pause_menu.c`]({{ site.github_url }}/blob/main/src/pause_menu/pause_menu.c).
[`src/jc_reborn.c`]({{ site.github_url }}/blob/main/src/jc_reborn.c).
[`CMakeLists.txt`]({{ site.github_url }}/blob/main/CMakeLists.txt).
[`holidays.yml`]({{ site.github_url }}/blob/main/holidays.yml).
The generated pages under [`site/docs/holidays/calendar/`]({{ site.github_url }}/tree/main/site/docs/holidays/calendar/).
[The build script]({{ site.github_url }}/blob/main/scripts/build-ps1.sh).
[The release script]({{ site.github_url }}/blob/main/scripts/release.sh).
If the task touches a subsystem, the agent reads the
subsystem before it writes.

That does two things. It gives the model real names and real constraints. It
also gives me a review handle: if the answer does not cite or edit the files
I know it needed, I treat the answer as suspect.

## The generated shelves

The source library on this site is not just for readers. It is also an
anti-hallucination tool.

Every Markdown source file gets a web wrapper with a source path, outline,
word count, and category. The regtest references get one page per preserved
case. The resource catalog lists the BMPs, ADS scripts, TTM animations, VAG
samples, PSB sprite banks, and [FG2 packs]({{ '/docs/glossary/#fg2-pack' | relative_url }}). That gives the next agent something
specific to search instead of asking it to infer the archive from memory.

Generated shelves are not glamorous. They are the difference between "there
might be a doc about this" and "the source-library page says this file exists
and here is the heading outline."

## Build gates beat confidence

The compiler is still the best hallucination detector ever shipped.

For code changes, the answer does not count until the build runs. On this
project that usually means:

```bash
./scripts/build-ps1.sh
./scripts/make-cd-image.sh
```

For site changes:

```bash
cd site
bundle exec jekyll build
```

For holiday changes:

```bash
python3 scripts/holidays-test.py
./scripts/holidays-build-all.sh
```

A model can sound very sure about a Liquid include. Jekyll does not care. A
model can be very confident about a struct field. GCC does not care. Good.

## Runtime gates matter too

Some hallucinations compile.

The PS1 pad bug compiled. The wrong `tx_len` compiled. The pause-menu dirty
rectangle bug compiled. Caption text mapped to the wrong scene compiles
perfectly and is still wrong.

That's why this project has visual gates. DuckStation captures. Host
references. Frame metadata. Perf logs. Holiday sprite sheets. Human review on
the actual emulator window. A passing build means the artifact is syntactically
real. It does not mean it is right.

## Prompt shape that works

The prompts that work here are ugly in the way good engineering documents are
ugly:

- "Only edit these files."
- "Do not touch [`src/scene/holidays.c`]({{ site.github_url }}/blob/main/src/scene/holidays.c)."
- "Index 0 is transparent."
- "Build must pass."
- "If you cannot find the function, say that."
- "Cite the file and line you used."

The prompts that fail are soft:

- "Make it better."
- "Be comprehensive."
- "Use your judgment."
- "Write in my voice."

Those are goals, not constraints. Goals need constraints under them.

## The one human rule

No agent merges itself.

That sounds obvious until you run six agents in parallel and one of them
produces exactly the page you wanted. The temptation is to trust the shape of
the output. Don't. Read it. Build it. Search for broken links. Ask where every
claim came from.

I use agents because they make the work faster. I keep the review bar because
the project has my name on the Credits screen.

## Cross-links

- [The LLM pass]({{ '/lab/llm-pass/' | relative_url }})
- [AI sub-agents on this project]({{ '/docs/agents/' | relative_url }})
- [The voice anchor problem]({{ '/lab/voice-anchor-problem/' | relative_url }})
  — the prose-side companion. Hallucination engineering is the
  code-side discipline; voice anchoring is how the same agents
  draft text without drifting off-voice.
- [Voice guide]({{ '/about/voice/' | relative_url }}) — the
  editorial bar agent-drafted prose has to survive.
- [Regression as a lifestyle]({{ '/lab/regression-as-lifestyle/' | relative_url }})
- [Source library]({{ '/source/' | relative_url }})
