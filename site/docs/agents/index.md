---
layout: page
title: AI sub-agents on this project
eyebrow: Process
subtitle: How LLMs were actually used. The honest version.
description: An honest accounting of where and how AI sub-agents were used on the Johnny Castaway PS1 fan port — captions, holiday emblems, this website, code review — and where they were not.
---

A labor of love by Hunter Davis. This page describes how AI sub-agents have
actually been used on the PS1 port. It is not a sales pitch for AI tooling
and it is not a denial that AI was used. The project's voice is plainspoken;
pretending the author wrote every word would not be plainspoken. If you paid
for this, you were cheated. Open source and free.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## The lay of the land

This is a one-person fan port. Hunter writes code, decides what scenes ship,
runs the validation passes, signs off on each release, and writes the
pause-menu Credits text. He is also the only operator of any AI tooling
involved.

The project uses AI sub-agents — primarily Claude and Codex, on separate
worktrees against separate branches, with occasional excursions into other
models for specific jobs — as an unusual member of the team. The agents
draft text, scaffold code, run parallel research passes against
documentation, and rubber-duck design questions. They do not own decisions.
They do not pick what's worth shipping. They do not validate scenes. The
chrome of the product — the on-screen pause menu, the merge bar that gates
a scene as validated, the website's information architecture — is the
author's call.

The setup is on the record at
[/about/dev-environment/]({{ '/about/dev-environment/' | relative_url }})
— a photograph of the desk shows the two agent prompt windows next to
the [Dunking Bird auto-poker]({{ '/lab/dunking-bird/' | relative_url }})
that keeps them moving and the DuckStation window where every visual
signoff still lands.

The claim being made on this page is narrow: AI helped with first drafts,
mechanical work, and parallel exploration. The author kept the pen.

## What agents actually wrote

A few specific places where AI-drafted output is in the shipping product or
the public site, with the author as the editor and final reviewer.

### The closed-caption corpus

The PS1 port supports closed captions. The caption corpus describes what
each scene is doing in plain English, keyed by ADS family and scene tag, so
players who can't hear the SFX still get the gag. Writing 60-plus captions
from a cold start is the kind of mechanical drafting work AI does well.

A sub-agent produced the first-draft caption text for every routed scene
plus a confidence rating per caption. The audit lives in the repo at
[`docs/ps1/caption-audit-2026-04-26.yaml`]({{ site.github_url }}/blob/main/docs/ps1/caption-audit-2026-04-26.yaml).
The breakdown was 30 HIGH, 21 MED, and 12 LOW confidence ratings — meaning
the agent was reasonably sure about half of its own output, ambivalent
about a third, and explicitly flagging the rest as needing human review.

The author then went through every caption and edited or replaced the ones
that were wrong. Several scenes had captions that were technically
plausible but wrong about what was actually happening in the gag — the
agent had inferred from the scene name and family rather than the actual
sprite content. Those got rewritten by hand. The final caption set is a
human-edited version of the agent's draft. See
[Closed captions]({{ '/docs/captions/' | relative_url }}) for the runtime
side and the audit breakdown in detail.

### The holiday emblem sprites

The 2026-04 holiday expansion grew the project from four shipped
holidays to thirty-six. The four original Sierra holidays
(IDs 1–4: Halloween, St. Patrick's, Christmas, New Year's) keep
their original full-island sprites; the thirty-two added holidays
(IDs 5–36) each get a 32&times;32 transparent emblem sprite in
the 16-color EGA/VGA palette, drawn in a Sierra-screensaver-
adjacent style: chunky pixels, bold outlines, recognizable at
32 pixels.

Drawing 32 thematically distinct sprites in a constrained palette
is the kind of work that stops being interesting after the third
sprite. A sub-agent authored Python+PIL scripts that generated the
emblems algorithmically — primitive shapes, palette-locked colors,
a per-holiday "compose this from these primitives" routine. The
output is the sprite sheet at
[`docs/ps1/holidays-emblems/holiday-emblems-sheet.png`]({{ site.github_url }}/blob/main/docs/ps1/holidays-emblems/holiday-emblems-sheet.png).
The script lives at
[`scripts/holidays-emblem-sheet.py`]({{ site.github_url }}/blob/main/scripts/holidays-emblem-sheet.py).

The author reviewed every emblem at full size, rejected several, and asked
the agent to redraw them with different primitives. Some emblems went
through three or four iterations before the author was happy. A few were
abandoned and drawn by hand. The shipping sheet is a curated subset of
agent-generated art with human edits on top.

### This website's first-draft prose

The page you are reading now started as a sub-agent draft. So did most of
the long-form pages under `/docs/`, the [About]({{ '/about/' | relative_url }})
narrative, and the chapter pages under [History]({{ '/about/history/' | relative_url }}).

The author writes prompts that establish voice constraints — plainspoken,
no marketing fluff, no rhetorical flourishes, no superlatives — and the
agent writes a long-form draft against those constraints. The author
then reads the draft, rewrites the parts that drift off-voice or invent
things, fills in details the agent didn't know, deletes the cute-but-empty
sentences, and signs off.

This page is meta about that process, but every other page on this site
went through it.

### The archaeology narrative chapters

The [history]({{ '/about/history/' | relative_url }}) section narrates how
the port came to exist — the original Sierra screensaver, the upstream
Amiga port, the desktop reimplementation, the first PS1 attempt that didn't
work, the second one that did. Sub-agents drafted those chapters with the
author's notes and commit history as source material. The author signed
off on each chapter and rewrote the parts that read as confident-sounding
fiction rather than what actually happened.

## What agents helped with but didn't own

The line is blurrier here. These are cases where an agent contributed
to work that ended up in the project, but the substantive decision-making
was the author's.

- **Code review.** Sub-agents have read pull requests and pointed out
  bugs, missing edge cases, and style inconsistencies. The author has
  agreed with some of the feedback and ignored some. The agent does not
  approve the merge.
- **Regression-test failure triage.** When a regtest run fails, an agent
  can read the harness output, the TTY log, and the per-frame PNG diffs
  and produce a triage summary much faster than the author can. The
  author reads the summary, decides whether the failure is real, and
  fixes the underlying issue.
- **"Find the file that does X."** The codebase has 130 scripts and
  several hundred source files between the host and PS1 builds. Agents
  are good at searching across that surface and answering "where is the
  CD prefetch state machine" or "which script generates the foreground
  packs" without the author having to remember.
- **Debugging logs.** When a perf experiment regresses, an agent can
  diff the JSONL logs against baseline and produce a structured summary
  of what changed. The decision to promote or reject the experiment is
  the author's, working from that summary plus their own read.
- **Performance experiment hypothesis generation.** Some of the
  experiments listed in the [perf log]({{ '/docs/performance/' | relative_url }})
  were ideas an agent suggested as plausible. The author ran them.
  Roughly half were rejected at the gate; the perf log is a complete
  record of what stuck and what didn't.

## What agents didn't do

Some of these are explicit firewalls. Some are just things the author
hasn't delegated.

- **The decisions.** What the project is, what it isn't, when to cut a
  release, when a scene is validated, when to declare an experiment
  done. All of that is the author's call.
- **The merge bar.** A scene is validated when the author watches it on
  emulator (and ideally hardware) and signs off on the visual + audible
  pass. No vision classifier and no test harness gates this; the human
  does.
- **The pause-menu Credits text.** The four-line credits screen was
  written by the author, by hand, before any AI involvement on this
  project. It reads:

  > A labor of love by Hunter Davis.
  > Hunter does not own or have a license to the Johnny Castaway character. The original creator generously allows fan ports.
  > If you paid for this, you were cheated.
  > Open source and free.

  That voice is the project's voice. The website's voice constraints
  are derived from that anchor, not the other way around. It is not
  draftable, and it is not editable by an agent.
- **Naming the project.** What does "fan port" mean? What does the
  project owe the original creator? What scope of polish does shipping
  imply? Those are the author's questions to answer. Agents are
  helpful tools for stress-testing answers; they are not where the
  answers come from.
- **Per-scene visual sign-off.** The author watches every validated
  scene end-to-end on emulator before promoting it. There is no
  shortcut.

## The honest limits

Three places where AI tooling has fallen short on this project, written
plainly so the disclosure is balanced.

**Agents invent things.** This is the single most expensive failure
mode. An agent will produce confident-sounding documentation for code
that doesn't exist if you let it. The author has caught this multiple
times: a draft that described a function that wasn't there, a docs
page that listed a CLI flag that was never implemented, a script
description that named a behavior the script did not have. Every
agent-drafted page on this site got at least one factual edit in the
review pass, and a few got rewritten substantially because the draft
was confidently wrong.

**Agents get tone wrong on the first try.** The voice constraints on
this project are aggressive — no superlatives, no marketing language,
no first-person plural to fake a team, no rhetorical questions ending
paragraphs. Agents drift back to default prose unless reminded. The
first draft of any long-form page reliably has a sentence ending in
"…and that's what makes this project special." (or similar) which
gets deleted in review. The author has stopped expecting the first
draft to land on voice and now treats voice editing as a guaranteed
pass.

**Agents are bad judges of their own confidence.** The captions audit
that produced 30 HIGH / 21 MED / 12 LOW ratings was useful, but the
author found errors in the HIGH-confidence captions at roughly the
same rate as in the MED ones. The LOW ones were genuinely worse.
Treat agent-stated confidence as a coarse signal that flags clear
weakness, not as a reliable measure of correctness.

The author re-reads everything. Not as a workflow optimization — as a
correctness requirement.

## Why disclose this

The project's voice is plainspoken. The pause-menu Credits screen reads
"A labor of love by Hunter Davis"; everything downstream of that —
including this website — has to mean it. Pretending the author wrote
every word of every long-form page would not be plainspoken. It would
be dishonest in a small but compounding way.

The disclosure also lets the reader calibrate. A page that started as
an AI draft and got human-edited is different from a page the author
sat down and wrote from scratch. Both can be useful. They are not the
same artifact, and the reader is entitled to know which is which.

If a future contribution to the codebase comes from someone other
than the author, they will be credited by name in commits and on the
relevant page. The same standard applies to AI sub-agents: the work
they did is identified, and the editorial responsibility for what
shipped is the author's.

## Related pages

- [Method]({{ '/about/method/' | relative_url }}) — how the project
  decides what's worth shipping.
- [Voice guide]({{ '/about/voice/' | relative_url }}) — the
  editorial standard agent drafts get edited against. Read this
  before writing anything that ships under the project's name.
- [The voice-anchor problem]({{ '/lab/voice-anchor-problem/' | relative_url }})
  — the deeper retrospective on how the four-line `drawCredits`
  text propagates voice constraints downstream.
- [The dunking bird]({{ '/lab/dunking-bird/' | relative_url }}) —
  the parallel-agent infrastructure that keeps two LLM sessions
  productive without constant operator attention.
- [Closed captions]({{ '/docs/captions/' | relative_url }}) — the
  caption corpus the audit produced.
- [Holidays]({{ '/docs/holidays/' | relative_url }}) — the 36-holiday
  expansion the emblem sheet supports.
- [Vision-classifier work]({{ '/docs/vision/' | relative_url }}) —
  another place AI runs at process boundaries, on the host validation
  side.
- [Devlog]({{ '/devlog/' | relative_url }}) — the running worklog.
