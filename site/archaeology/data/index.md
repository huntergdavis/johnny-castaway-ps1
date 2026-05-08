---
layout: page
title: Primary Sources
eyebrow: Read the YAML, not the prose
subtitle: The raw archaeology files behind the narrative — what each one contains and why you'd open it.
description: A pointer index to the primary-source YAML, JSON, and Markdown files in docs/ps1/archaeology/ that drive the Johnny Castaway PS1 narrative pages.
---

The narrative pages on this site — [the five chapters]({{ '/archaeology/' | relative_url }}),
[the era timeline]({{ '/archaeology/timeline/' | relative_url }}),
[the team page]({{ '/archaeology/team/' | relative_url }}) — are
written from a small set of structured source documents that live
under `docs/ps1/archaeology/` in the project repo. Those files are
what get edited when the truth changes; the prose is downstream.

If you want to verify a date, an era boundary, a deprecated tool's
last touched commit, or a status-surface meaning that's been
demoted, the primary sources below are the right place to look. They
are also, on their own, a reasonable read — written in the same
plain register as the prose, without the narrative connective
tissue.

## The files

### timeline.yaml

[timeline.yaml]({{ site.github_url }}/blob/main/docs/ps1/archaeology/timeline.yaml)
breaks the PS1 branch into five eras with date ranges, summary
sentences, dispositions (preserve / archive / promote), and — for
the foreground playback era — a milestone commit list with SHAs and
messages. It's the structured source for the
[era timeline page]({{ '/archaeology/timeline/' | relative_url }}).
Open this when you want the exact commits that walked `FISHING 1`
from "scaffold" to "pixel-perfect with full SFX," or when you need
to know which part of the project history is officially demoted to
*archaeology*.

### team-perspective.yaml

[team-perspective.yaml]({{ site.github_url }}/blob/main/docs/ps1/archaeology/team-perspective.yaml)
catalogues the six roles the author played across the project's
phases — operator-editor, embedded porter, instrumentarian,
validator-skeptic, archaeology engineer, bespoke scene engineer —
with the strongest evidence file for each. It also lists the
project's shared values in their original yaml-list form, which is
where the website's voice was anchored. Open this when you want to
see which docs and skills correspond to which role, or when you
want the canonical phrasing of the project's ethic without the
narrative wrapper.

### tools.yaml

[tools.yaml]({{ site.github_url }}/blob/main/docs/ps1/archaeology/tools.yaml)
catalogues every script in the repo with a policy: `keep_live`,
`keep_but_demote_from_primary_docs`, or `archive_candidate`. Each
entry has a role hint and, where relevant, a last-touched commit
and date. It's the source of truth for "is this script still part
of the operator loop, or is it historical?" The currently-live
scripts include `build-host.sh`, `build-ps1.sh`,
`make-cd-image.sh`, `rebuild-and-let-run.sh`,
`capture-host-scene.sh`, `export-scene-foreground-pilot.sh`,
`build-scene-foreground-pack.py`, `wav2vag.py`, and a handful of
others that drive the per-scene bring-up loop.

### blog-source-map.md

[blog-source-map.md]({{ site.github_url }}/blob/main/docs/ps1/archaeology/blog-source-map.md)
is a pointer map — not raw data, but a curated list of the branch's
strongest narrative sources, with one-line annotations. It points at
the early branch voice, the validation skepticism agent prompt, the
**OFFLINE_SCENE_PLAYBACK_PIVOT** research note that announced the
methodology shift, the FISHING 1 pixel-perfect plan, and the water
animation worklog. Open this when you want to read the project's
own prose at the moments when the direction was changing.

### assumptions.yaml

[assumptions.yaml]({{ site.github_url }}/blob/main/docs/ps1/archaeology/assumptions.yaml)
is the project's honest list of things it used to believe. Each
entry has the assumption, why it made sense at the time, what
changed, and the replacement insight. The assumptions catalogued
include "broad automation would be the primary truth surface,"
"generic restore and replay contracts would be the finish line,"
"if the static analyzer said the content fit then runtime memory
would be fine," and "fgpilot was just a temporary experiment name."
Open this when you want to understand why the project went down
particular roads before it turned around.

### memory-constraints.md

[memory-constraints.md]({{ site.github_url }}/blob/main/docs/ps1/archaeology/memory-constraints.md)
is a short prose document that explains the two facts that shaped
every decision: 2 MB main RAM, 1 MB VRAM, native 640x480. It walks
through the math on why full-screen prerender for every scene was a
non-starter (a single 16-bit frame is about 614400 bytes), why
static fit numbers proved weaker than they looked once runtime
allocation timing entered the picture, and how the wave-asset path
ended up dictating preload order. Open this when you want the
quantitative version of the chapter-4 narrative.

### reporter-notes.md

[reporter-notes.md]({{ site.github_url }}/blob/main/docs/ps1/archaeology/reporter-notes.md)
is a short essay framing the cleanest read of the branch as one
team that *kept changing jobs, not standards*. It identifies the
April 12–22 window as the decisive turn from "trying to prove a
universal replay theory" to "tightening a bespoke line that
actually closes." Open this when you want the framing distilled to
five paragraphs.

### status-surfaces.yaml

[status-surfaces.yaml]({{ site.github_url }}/blob/main/docs/ps1/archaeology/status-surfaces.yaml)
catalogues every "N of 63" number this project has ever published —
25/63, 27/63, 63/63, 57/63, 60/63, 1/63, 2/63 — with the date each
was claimed, what it meant at the time, the source documents and
commits, and the demotion reason. The 63/63 from
**2026-03-29** is explicitly preserved as a *historical false
summit*. Open this when you want to confirm that yes, the project's
status numbers really did go down before they went up, and the
reasons are documented.

---

There are other supporting files in `docs/ps1/archaeology/` —
`binary-library-index.csv`, `binary-library-manifest.json`, the
`vision-artifacts/` and `regtest-references/` corpora, the retired
script and source archives. Those are evidence rather than
narrative; the index above is the curated set that the website is
written from. The full directory listing is at
[docs/ps1/archaeology/]({{ site.github_url }}/tree/main/docs/ps1/archaeology).

## Drill-downs

For each of the larger evidence corpora, there is now a per-directory
narrative page that walks through the contents in plain prose. These
were written so a reader can understand each archive's purpose and
shape without opening every file.

- [The binary library]({{ '/archaeology/binary-library/' | relative_url }}) —
  the indexed record of every PS1 build the project produced (6,902
  entries, originally 118 GB of payloads, deleted on 2026-04-22 with
  the index preserved).
- [Regtest reference set]({{ '/archaeology/regtest-references/' | relative_url }}) —
  the sixty-three frozen scene captures the regtest harness compares
  fresh runs against, with a per-scene table.
- [Host-script review]({{ '/archaeology/host-script-review/' | relative_url }}) —
  the bundled snapshot of the host-side identification and
  expectation pipeline as it stood on 2026-04-04.
- [Retired scripts]({{ '/archaeology/retired-scripts/' | relative_url }}) —
  fourteen scripts from the binary-library regression era, each with
  a paragraph explaining what it did and why it was retired.
- [Retired source code]({{ '/archaeology/retired-src/' | relative_url }}) —
  three minimal C bring-up programs from late 2025 that proved the
  GPU and CD-ROM I/O before the full game integration began.
- [Vision-classifier artifacts]({{ '/archaeology/vision-artifacts/' | relative_url }}) —
  the reference-bank self-check across 63 scenes, the pipeline
  manifest, and a frozen FISHING 1 regtest run from the
  visual-detection-spec era.
