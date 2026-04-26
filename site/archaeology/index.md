---
title: Archaeology
eyebrow: How a 1992 screensaver became a 2026 PS1 disc
subtitle: Five chapters. Not a manifest dump.
---

This page becomes a *narrative* in P4 — five chapters drawing on
`archaeology/timeline.yaml`, `team-perspective.yaml`, and
`tools.yaml`. Hand-written prose, not a YAML dump.

The chapters in rough form:

1. **The screensaver before the project** — what *Johnny Castaway*
   was when Sierra shipped it in 1992, what *After Dark* meant,
   why the genre died.
2. **The first port** — `jno6809/jc_reborn`, the SDL2 port that
   made everything else possible by reading the original engine's
   bytecode.
3. **Across the ports** — `nivs1978/JCOS`, `xesf/Castaway`, what
   each one preserved or revealed.
4. **The PS1 question** — why this hardware, how the hybrid
   pipeline emerged, what the early prototypes got wrong.
5. **Where it stands now** — the {{ site.release.scenes_validated }}/{{ site.release.scenes_total }}
   count, the captions, the holidays, what's left to do or
   deliberately skip.

Companion surfaces (also P4):

- **[/archaeology/timeline/]({{ '/archaeology/' | relative_url }})** — an
  era-strip rendered from `timeline.yaml`.
- **[/archaeology/team/]({{ '/archaeology/' | relative_url }})** — a
  newsroom-lab piece reading `team-perspective.yaml` for what
  each contributor (this project, the prior ports, the upstream
  engine readers) brought.
- **[/archaeology/data/]({{ '/archaeology/' | relative_url }})** — the
  raw YAML / JSON, for primary-source readers.

Until the chapters land, the dated worklogs at [/devlog/]({{ '/devlog/' | relative_url }})
are the most honest source on how the project got here.
