# Website Master Plan — Johnny Castaway PS1

This is the executable plan for the project's GitHub Pages site. It folds the
output of eight reviewer passes (correctness, style, presentation, branding,
voice, structure + affordance, flow, accessibility) plus an OSS-conventions
review and a release-engineering review. Read this before touching the
`website` branch; revisit after every major page lands.

Working branch: `website` (cut from `main` 2026-04-26).
Site URL: `https://hunterdavis.com/johnny-castaway-ps1/`. The apex
`hunterdavis.com` is already served via the user's GitHub user-pages
repo (CNAME lives there); GitHub resolves project pages under the same
hostname at `/<repo-name>/`. **No CNAME file inside this repo.**
`_config.yml` will set `url: https://hunterdavis.com` +
`baseurl: /johnny-castaway-ps1`. Every internal link uses
`{{ '/path' | relative_url }}` so the baseurl prefix lands correctly.
Source of truth for the site: `_site/` generated from this repo.

---

## 1. Goals & non-goals

**Goals**
- One canonical public surface for the project — supersedes the README hero
  block as the place a stranger lands first.
- Honest status everywhere — every "X / 63" number, every version string, every
  download link is generated at build time from a single source of truth.
- The 80+ docs in `docs/ps1/` are *navigable*, not a flat dump. Featured pages
  read like prose, dated worklogs read like a devlog, and the archaeology
  YAML/JSON becomes a real **narrative page** instead of a manifest dump.
- Every release auto-refreshes `/play/` (download links, version badge,
  release notes) without a hand edit.
- Accessibility built in from page one — pixel-art aesthetic does not get to
  excuse poor contrast, missing alt text, or autoplaying motion.

**Non-goals**
- Not a marketing site. No "experience the magic," no Patreon banner above
  the fold, no glassmorphism cards.
- **Not a community hub. Not a contribution surface.** This is an info dump —
  the project's story, the port's mechanics, the artifacts of the build. No
  Discord, no Discussions, no roadmap with target dates, no "claim this
  scene" CTA, no "contributors welcome" framing. The author isn't soliciting
  help; bug reports via GitHub issues are tolerated, not invited.
- Not a code mirror. The site links GitHub for code; it does not render
  syntax-highlighted source trees.
- Not a fan-tribute. The Johnny Castaway character belongs to its original
  creator; the site reflects that in its chrome and its disclaimer.

---

## 2. Sitemap

```
/                            home — hero, 3-up screenshots, status, latest devlog
├── /play/                   download + run guide (primary CTA target)
│   ├── /play/               latest .bin/.cue, DuckStation quickstart
│   ├── /play/releases/      auto from GH Releases (every v*-ps1 tag)
│   └── /play/controls/      pause menu, options, captions, set time/pos/seed
├── /about/                  what this is + how it works
│   ├── /about/              project pitch + hybrid-pipeline diagram
│   ├── /about/method/       host capture → FG2 packs → PS1 replay
│   ├── /about/status/       live ledger (component + scene rollup)
│   └── /about/history/      project-history.md, repackaged
├── /scenes/                 scene ledger as a sortable database table
│   └── /scenes/<slug>/      per-scene case study (e.g. /scenes/fishing-1/)
├── /devlog/                 reverse-chrono — feeds research/ + auto release posts
│   ├── /devlog/             paginated index, one-line summaries
│   ├── /devlog/<year>/<m>/  per-entry pages, each is a single rendered worklog
│   └── /devlog/v0.3.9-ps1/  auto-generated from a release body (see § 9)
├── /archaeology/            *narrative*, era-by-era — not a manifest dump
│   ├── /archaeology/        the prose narrative (5 chapters)
│   ├── /archaeology/timeline/   interactive era strip from timeline.yaml
│   ├── /archaeology/team/       newsroom-lab piece from team-perspective.yaml
│   └── /archaeology/data/       index of raw YAML/JSON for primary-source readers
└── /docs/                   reference manuals, merged from docs/ps1/
    ├── /docs/build/         build-system.md + toolchain-setup.md
    ├── /docs/captions/      caption corpus + audit YAML, rendered
    ├── /docs/holidays/      holidays-* files, merged with anchors
    ├── /docs/pause-menu/    pause-menu-design.md, current
    ├── /docs/freeplay/      freeplay-mode-design.md
    ├── /docs/regtest/       regtest-harness.md + regtest-quickstart.md
    ├── /docs/api/           api-mapping.md (SDL2 → PSn00bSDK)
    └── /docs/archive/       flat index — performance plans, hardware specs,
                             milestone reports, anything not promoted

(footer-linked, not in nav)
/faq/                        curated FAQ — author-written, not a forum
/credits/                    full acknowledgements + drawCredits text
/legal/                      GPL-3.0 + Sierra asset disclaimer
```

No `/contribute/` page. The development-workflow doc stays in `/docs/`
for the author's own reference, not surfaced as a public CTA.

**Top nav (6 items, exact labels):** `Play` · `About` · `Scenes` · `Devlog`
· `Archaeology` · `Docs`. GitHub icon outside the nav. No "Home" item — logo
returns to /. No mega-menu.

**Breadcrumbs**: yes on `/docs/*`, `/archaeology/*`, `/scenes/<slug>/`. No on
home, /play, /about landing, /devlog index. Devlog entries get a date trail
(`Devlog › 2026 › April`).

**Search**: in scope. Pagefind (static, zero infra, builds at `jekyll build`
time, ships JS+index). `/` keybind to focus search; no separate `/search/`
route. Skip Algolia (overkill, API-key churn) and Lunr (relevance degrades
past ~30 docs and we'll have ~50+ rendered).

---

## 3. What lives where (the 80+ docs land)

**Featured / promoted (rewritten as polished pages):**
- `current-status.md` → `/about/status/` (Jekyll include + auto status pill)
- `scene-status.md` → `_data/scenes.yml` + `/scenes/` (table) + `/scenes/<slug>/`
- `project-history.md` → `/about/history/`
- `pause-menu-design.md` → `/docs/pause-menu/`
- `freeplay-mode-design.md` → `/docs/freeplay/`
- `holidays-*.md` (4 files) → merged into `/docs/holidays/`
- `regtest-harness.md` + `regtest-quickstart.md` → merged into `/docs/regtest/`
- `build-system.md` + `toolchain-setup.md` → merged into `/docs/build/`
- `caption-audit-2026-04-26.yaml` → rendered into `/docs/captions/`
- `hardware-specs.md` → callout in `/about/method/`, full at `/docs/build/`
- `api-mapping.md` → `/docs/api/`
- `development-workflow.md` → `/docs/dev-workflow/` (author's own runbook;
  not surfaced as a contributor CTA — see § 1 non-goals)
- `TESTING.md` → callout in `/docs/dev-workflow/`
- `milestones-YYYY-MM-DD.md` → glob into `_posts/milestones/` (separate from
  `/devlog/`)

**Devlog (verbatim, chronological):** all 28 `docs/ps1/research/*.md`. Don't
rewrite. Frame each with a one-paragraph editor's note ("This is an unedited
dev log from <date>. The dead ends matter."). Reverse-chrono index page.

**Archive (listed, not in nav):** `performance-experiment-log.md`,
`performance-optimization-plan.md`, `audio-optimization-spec.md`,
`visual-detection-spec.md`, `holidays-implementation-plan.md` (now superseded),
old per-day `milestones-*.md` after they're surfaced once.

**Drop / never surface:**
- `pause-toggles-captions-handoff.md` (this work is shipped; deleted in
  the same commit that lands this plan).
- `TODO.md` (private working surface).
- `ps1-branch-cleanup-plan.yaml` (operational).
- `archaeology/retired-scripts/`, `archaeology/retired-src/`,
  `archaeology/host-script-review/`, `archaeology/regtest-references/`,
  `archaeology/vision-artifacts/`, `archaeology/binary-library-*` — git
  history, not documentation.

---

## 4. Brand identity

### 4.1 Name & wordmark
- Repo + `<h1>`: `Johnny Castaway — PlayStation 1`.
- `<title>`: `Johnny Castaway PS1`.
- `og:title` / social cards: `Johnny Castaway PS1 — a fan port`.
- Logotype: `JOHNNY CASTAWAY` in pause-menu typography, `PS1` smaller all-caps
  beneath. Don't invent a new name. Don't remix the Sierra logo.

### 4.2 Tagline
> A ground-up PlayStation 1 port of Sierra's *Johnny Castaway* — one scene at a time.

### 4.3 Palette (six values, pulled from the actual game)
| Token | Hex | Use |
|---|---|---|
| `--bg-deep`     | `#0D1B2A` | page bg (dark default), CRT bezel outer |
| `--accent-blue` | `#1F4FD6` | links, panel headers, Sierra royal |
| `--accent-cyan` | `#39C7E0` | hover, separator rules, focus ring |
| `--accent-sand` | `#F2C94C` | milestones, badges, scene-name eyebrows |
| `--accent-palm` | `#2DAA4F` | success / validated-scene pills |
| `--bg-cream`    | `#F4EAD5` | body bg in light mode, body text on dark |

Light-mode option: `#F4EAD5` background, `#0D1B2A` text, same accents. Avoid
pure white (it fights the screenshots). Status pills must carry text or shape
in addition to color (WCAG 1.4.1).

### 4.4 Typography
- **Display** (h1/h2/site title): `IBM Plex Mono` 600.
- **Body**: `Inter` 16–17px, 1.55 line-height. (Or `iA Writer Quattro` if we
  want the typewriter mood throughout.)
- **Pixel accent** (eyebrows, badges, in-game-style callouts only — never
  body): `VT323` or `Press Start 2P`. 3–5 appearances per page max.
- **Code**: `IBM Plex Mono` or `JetBrains Mono`, 14px, with disambiguated
  `0/O 1/l/I`.

### 4.5 Signature motifs (pick two, repeat)
1. **Cyan separator bar** (`--accent-cyan`, 1 px) — beneath every H1, top of
   every footer, between sections. Centered yellow 8 px diamond optional.
2. **Holiday-emblem accent** — one 16 px emblem from
   `docs/ps1/holidays-emblems/` rotates by season (anchor / palm /
   lighthouse / snowflake) at top-right of each page. Calendar feel without
   animation.

### 4.6 Pixel-art treatment
`image-rendering: pixelated; image-rendering: crisp-edges` everywhere. Source
frames are 640×480; on a 1280 px content column, render hero shots at 2×
nominal (1280×960 logical, scaled from 640×480 in CSS). Inline trio shots
(cast/raft/night) at 320×240 logical (~31% column width) with 2× lightbox on
click. Frames get a "CRT bezel": 2 px solid `--bg-deep` outer + 1 px inner
`--accent-cyan`. No drop shadows, no rounded corners > 2 px, no glow.

### 4.7 Hero composition
**Static layout, not a GIF.** A wide letterbox of `fishing1-ps1-cast.png` at
native 2× sits *above* the headline. Headline + tagline + dual CTA + tertiary
"Need an emulator?" link sit below. **Three-screenshot trio** (cast / raft /
night) at equal width forms the second viewport. A short looping muted MP4 of
FISHING 1 cast→catch→release lives in scroll #2 with `prefers-reduced-motion`
fallback, never autoplays above the fold.

### 4.8 Favicon, og:image, social card
- **Favicon (32×32, 16×16):** the palm-tree emblem from
  `docs/ps1/holidays-emblems/holiday-emblems-sheet.png`, transparent.
- **og:image (1200×630):** crop of `docs/readme/fishing1-ps1-cast.png` with a
  bottom band — cyan separator + white mono text "Johnny Castaway PS1 · fan
  port". Do not upscale-smooth the pixel grid.
- **Twitter card:** `summary_large_image` pointing at the same og:image.

### 4.9 Brand DON'Ts
1. No flat-design / Vercel-template aesthetic (gradient mesh, glass cards,
   8 rem hero typography). Looks like an AI startup.
2. No neon-cyber / vaporwave palette. Castaway is daylight VGA, not Miami Vice.
3. No body-copy pixel font and no global CRT scanline overlay. Pixel typography
   belongs in eyebrows, badges, pause-menu callouts only.
4. Don't lean on the Sierra logo or the original Johnny sprite as a header
   mark — the disclaimer says "no license to the character," the chrome must
   not contradict.

---

## 5. Voice

**Anchor:** the in-game `drawCredits` text in `src/pause_menu/pause_menu.c`. *"A labor of
love by Hunter Davis. … If you paid for this, you were cheated."* Plainspoken,
dry, sincere, no superlatives. That's the heart.

**Two registers, one author**:
- `/`, `/play/`, `/about/`, `/credits/`: **drawCredits register** — short
  sentences, low jargon, occasionally dry.
- `/about/method/`, `/archaeology/`, `/scenes/`, `/docs/*`:
  **README register** — full technical vocabulary, declarative, links to
  source.
- `/devlog/`: **leave the worklogs raw**. Frame each with a one-paragraph
  editor's note up top. Don't sanitize — the rawness is the value.

**Five voice rules**
1. First person singular ("I") for choices and feelings; "we" only when
   crediting collaborators.
2. No marketing superlatives. If a thing is fast, give the number.
3. Humor: dry and sparing. One per page max, in the drawCredits key.
4. Show the constraint before the solution.
5. Average sentence ~15 words; hard cap ~30. Break long sentences.

**Anti-patterns**
- Never apologize for scope or pace ("still rough", "early days").
- Never write passive voice to dodge ownership.
- Never sound like a pitch deck.

**Sample openings** (same author, three surfaces):

> **Home (/):** Johnny Castaway, the 1992 Sierra screensaver, ported to the
> PlayStation 1. It boots, it loops, and you can play it on real hardware.

> **Archaeology (/archaeology/the-614kb-problem/):** A full-frame video
> pipeline would have needed roughly 38 megabytes of frames per scene. The
> PS1 has two megabytes of RAM. That math is where the hybrid pack format
> starts.

> **Download (/play/):** Grab the .bin and .cue, point DuckStation at the
> cue, and you're in. No installer, no account, no telemetry — it's a CD
> image.

---

## 6. Page-by-page content briefs

### 6.1 `/` — home

Above the fold (single 100vh):
- Letterbox: `fishing1-ps1-cast.png`, 2× pixelated, CRT bezel.
- H1: **Johnny Castaway, on a PlayStation.**
- Subhead (~70 col): A faithful PS1 port of Sierra's 1992 screen saver. Burn
  the disc, boot the emulator, and put him back on his island.
- Primary CTA (solid, palm-green): **Download v{version} (.bin / .cue, ~3 MB)**
- Secondary CTA (ghost): View on GitHub.
- Tertiary link: Need an emulator? → DuckStation.
- Trust strip below CTAs: version badge `v{tag}` · GPL-3.0 · `{validated}/63
  scenes pixel-validated` pill · GitHub stars.

First scroll (next ~1.5 viewports):
- Strip 1 — looping muted MP4 of FISHING 1 (~6 s), poster image, manual play
  honoring `prefers-reduced-motion`. Caption: "FISHING 1, captured from
  DuckStation."
- Strip 2 — three typographic cards (no icons, no gradients):
  - Hybrid pipeline: host captures, PS1 replays.
  - {validated} of 63 scenes fully validated under the reference bar.
  - Runs on real PS1 hardware budget (2 MB RAM, 1 MB VRAM).
- Strip 3 — `pause-menu.png` at 70% with caption: "Press START — sound,
  day/night, holiday, captions, memcard save."
- Latest devlog: 3 most recent posts, each a date + 1-line summary.

### 6.2 `/play/`

Above-the-fold: literal version string + .bin/.cue download buttons.
Below: minimal DuckStation quickstart (Flatpak install copypasta + "open the
.cue" line). Controller-mapping table. Link out to `/play/controls/` for the
full pause-menu surface. Link to `/play/releases/` for tag history.

### 6.3 `/play/releases/`

Generated from `_data/release.yml`. One card per release: tag, date, body
(rendered from GitHub Release notes), bin/cue URLs. Newest first. Pinned
"latest" card at the top.

### 6.4 `/play/controls/`

Pause-menu surface in detail: main menu, Options sub-screen (Sound, Day/Night,
Tide, Raft, Holiday, Captions, Perf Log, Set Time, Set Island Pos, Set RNG
Seed), Credits, Reset Current Scene, Next Scene. Screenshots from
`docs/readme/`.

### 6.5 `/about/`

Project pitch (drawCredits register, ~3 paragraphs). One hybrid-pipeline
diagram (host capture → FG2 pack → PS1 replay → SFX → screen). CTA out to
`/about/method/` for the deeper read, `/about/history/` for the journey, and
`/about/status/` for the live ledger.

### 6.6 `/about/method/`

The technical pitch. README's "Method" section, expanded. Hardware specs
callout (2 MB / 1 MB / 512 KB / 640×480 NTSC). The 614 KB × 63 = gigabytes
math. Why hybrid, not from-scratch. Links to source files on GitHub for every
named module (`foreground_pilot.c`, `spi.c`, `ps1_captions.c`, etc).

### 6.7 `/about/status/`

Status widget (current release, validated count, last update) +
component-level ledger from `current-status.md`. Live; reads from
`_data/release.yml` and `_data/scenes.yml`.

### 6.8 `/about/history/`

Repackaged `project-history.md`. Why PS1, the dead ends, the lineage. Footer
acknowledgements teaser, with full credits at `/credits/`.

### 6.9 `/scenes/`

Sortable, filterable table backed by `_data/scenes.yml` (auto-generated from
`scene-status.md`). Columns: ADS, tag, slug, visuals (✅/⏳), SFX, variants,
last-verified, notes. Row click → `/scenes/<slug>/`. The scene-name search
keybind (`/`) lands here.

### 6.10 `/scenes/<slug>/` — per-scene case study

Page exists for *every* scene, validated or not. For ⏳ scenes: a plain
"Not yet validated" line, ADS tag, and FG2 pack basenames if known —
**no "Claim this scene" CTA, no issue-template button** (§ 1 non-goals;
the scene-bringup template doesn't exist). For ✅ scenes: hero screenshot,
variant gallery, ADS+tag, FG2 pack basenames, what "validated" means,
link to the captioning entry.

### 6.11 `/devlog/`

Reverse-chronological feed of:
- All 28 `docs/ps1/research/*.md` worklogs (verbatim, with a one-paragraph
  editor's note up top).
- All `milestones-YYYY-MM-DD.md` posts.
- Auto-generated per-release posts (see § 9).

Index page: paginated, one-line summary per entry. Date breadcrumb on
detail pages, no sidebar (single-column reading view), prev/next pager.

### 6.12 `/archaeology/`

**Narrative**, not a manifest dump. Five chapters mapped to
`archaeology/timeline.yaml` eras:
1. Embedded bootstrap.
2. Restore pipeline.
3. Validation maximalism.
4. Binary-library archaeology.
5. Foreground-playback pivot.

Each chapter pulls dates from `timeline.yaml`, characters from
`team-perspective.yaml`, tooling from `tools.yaml`, and ends with a
"Primary sources" footer linking the specific YAML stanzas on GitHub.
Retired scripts/src trees mentioned in prose ("87 scripts retired in the
pivot") but not enumerated.

`/archaeology/timeline/` — interactive era strip.
`/archaeology/team/` — newsroom-lab piece from `team-perspective.yaml`.
`/archaeology/data/` — index of every raw artifact for primary-source readers.

### 6.13 `/docs/*`

Reference manuals only. Each `/docs/*` page has a sticky right-rail TOC
(auto-generated from `##/###` for any page > ~600 words) and a left-rail
section sidebar listing siblings. "Edit this page on GitHub" + "View source
on GitHub" links in every footer.

### 6.14 `/credits/`

The drawCredits text reproduced verbatim, plus full acknowledgements
(`jno6809/jc_reborn`, `nivs1978/JCOS`, `xesf/Castaway`, Sierra Chest,
PSn00bSDK), GPL-3.0 line. Hunter shows up here once with a footer line,
not a bio page.

### 6.15 `/docs/dev-workflow/` (author's runbook, not a contribute CTA)

`development-workflow.md` lives under `/docs/` like every other reference
manual — it's the author's own per-scene loop, kept current because the
author runs through it. Not framed as "how to contribute," not linked
from the home page, not promoted in the nav. A reader who lands on it is
either the author or someone curious about the process; either is fine.
No marketing copy, no "Welcome aboard!", no flowcharts.

### 6.16 `/legal/` (footer link)

GPL-3.0 + the asset/character disclaimer:
> Johnny Castaway, the character, screensaver, and original Sierra assets
> are © Sierra On-Line and not licensed under GPL. This project ships only
> the code that drives the port and requires users to supply their own
> original Sierra data files (`RESOURCE.MAP` / `RESOURCE.001`).

### 6.17 `/faq/` (footer link)

The site's only "answers from the author" surface. **An info dump, not
a community channel** — curated questions and answers written by the
author, not crowdsourced, not soliciting more. Scannable: each question
is an H3, each answer is two or three sentences max, grouped into 3
sections — *About* / *Running it* / *Original game*.

Seed list (subject to author final pass):
- *About* — "What is this?", "Why PS1?", "Will you port other Sierra
  screensavers?", "Is this legal?", "Why is it only `{validated}/63`
  scenes?".
- *Running it* — "How do I run it?", "Do I need original Sierra files?",
  "Does it work on real PS1 hardware?", "Which emulators are supported?",
  "Where do I file bugs?" (link to issues, framed as *if you must*).
- *Original game* — "Where does the caption text come from?", "Why are
  there 35 holidays now instead of 4?", "What's faithful and what's
  added?".

No *Contributing* section — see § 1 non-goals. No "ask a question" link.
No comment box. The page is the entire conversation.

---

## 7. Reader-journey table

| Persona | First click | Next clicks | MUST have |
|---|---|---|---|
| Casual nostalgic | hero "Download" CTA | DuckStation note → controller map → variant gallery | one looping muted MP4 + one big Download button above the fold |
| PS1 homebrew dev | "How it works" / Method | hardware specs → api-mapping → source links | a "PS1 gotchas we hit" page (SPI tx_len=5, FntFlush, SPU HLE, TTY printf) |
| Sierra archivist | Closed captions / Faithfulness | scene ledger → caption-audit → holidays expansion | scene-by-scene gallery with "original / preserved / added" tri-color |
| Process voyeur | Project history / Devlog | dated worklog → history phase narrative → timeline | reverse-chrono devlog with one-line summaries |

**Note: no contributor persona by design.** The site assumes its readers
are here to *learn about the project*, not to ship code with the author.
A "claim this scene" CTA, an "edit this page" footer, a "good first issue"
funnel — all skipped on purpose (§ 1 non-goals). If the project ever does
attract regular contributors, this section gets revisited.

**Cross-cutting affordances**
- Persistent right-rail status widget (release tag · scenes validated · last
  updated). Reads from `_data/release.yml` and `_data/scenes.yml`.
- "View source on GitHub" link in the footer (read-only; not "Edit this
  page" — see persona note above).
- Global `/` keybind to focus a scene-slug search (Pagefind-backed).

**Flow danger zones to engineer around**
1. The "method" wall — never lead with "TTM/ADS interpreter / FG2 base-diff
   spans" on the landing path. Method content lives below the fold or behind
   `/about/method/`.
2. Research-as-dump — `/devlog/` MUST be a rendered chronological index, not
   a flat file list. The ALL_CAPS_DATED filenames are unreadable.
3. Status-number whiplash — historical worklogs quote `25/63`, `60/63`,
   `63/63` from prior validation regimes. Every dated/historical doc must
   auto-inject "Historical — current bar is `{validated}/63`" banner with
   link to live status. Coupled with `noindex` and sitemap exclusion (§ 11).

---

## 8. Tech stack

- **Generator**: Jekyll (matches GitHub Pages native, no Node toolchain).
- **Theme**: hand-rolled minimal — start from `minima` and replace the layout
  + scss. No `just-the-docs`, `docusaurus`, `mkdocs-material` (each erases
  the project's voice).
- **Deployment**: GitHub Pages via `actions/deploy-pages@v4`.
- **Search**: Pagefind (built into the workflow, ships JS+index alongside).
- **CI**: GitHub Actions, single workflow `.github/workflows/site.yml`.
- **Lighthouse / a11y check**: `pa11y-ci` step in the workflow on PRs.
- **Link check**: `lychee` in CI.
- **Image pipeline**: keep PNGs as-is (don't transform pixel art); only run
  `oxipng` for losseless size on hero shots.
- **Pre-commit hook** (in repo, not site-only): runs alt-text linter +
  hard-coded-version detector (§ 11).

---

## 9. Release-pipeline integration

### 9.1 Source of truth (per content stream)

| Stream | SoT | Site sync strategy |
|---|---|---|
| Release version + tag + asset URLs | latest GitHub Release, fallback `VERSION` | `_data/release.yml` written by Actions step (`gh release view --json …`) |
| Per-scene ledger | `docs/ps1/scene-status.md` (rendered to `_data/scenes.yml` at build) | Liquid templates render `/scenes/` and `/scenes/<slug>/` |
| Component-level status narrative | `docs/ps1/current-status.md` | Jekyll include |
| Captions corpus + ADS+tag map | `caption-audit-2026-04-26.yaml` (NOT the README table — it rots) | Generated `/docs/captions/` page |
| Milestones | `milestones-YYYY-MM-DD.md` glob + front matter | Index + per-post |
| Devlog | `docs/ps1/research/*.md` glob | Index + per-post |
| Archaeology narrative | `archaeology/timeline.yaml` + `team-perspective.yaml` + `tools.yaml` | `/archaeology/` chapters render from these |

### 9.2 GitHub Actions workflow (`.github/workflows/site.yml`)

Triggers:
```yaml
on:
  push:
    branches: [main]
    tags: ['v*-ps1']
  workflow_dispatch:
  release:
    types: [published]
  schedule:
    - cron: '0 9 * * *'    # nightly self-heal
```

Steps (concise):
1. `actions/checkout@v4` with `fetch-depth: 0` (need tags).
2. `gh release view --json tagName,name,publishedAt,assets,body` → write
   `_data/release.json` + `_data/release.yml`.
3. Parse `docs/ps1/scene-status.md` → `_data/scenes.yml`.
4. Auto-generate per-release `_posts/$(date +%F)-release-${TAG}.md` from the
   release body (idempotent — skip if file exists).
5. `ruby/setup-ruby@v1` + `bundle exec jekyll build`.
6. Pagefind index step.
7. `actions/upload-pages-artifact@v3` → `actions/deploy-pages@v4`.
8. Concurrency group `pages`, `cancel-in-progress: false`. Never cancel a
   release-triggered deploy.

### 9.3 `scripts/release.sh` modifications

Three small changes:

1. After step 7 (GitHub Release publish), append:
   ```bash
   gh workflow run site.yml --ref main || true
   ```
   Belt-and-braces — `release: published` covers most cases, but a manual
   kick guarantees the new VERSION commit on `main` is what Pages serves.

2. Step 7 currently inlines `RELEASE_NOTES` to `gh release create --notes`.
   Switch to `--notes-file release/NOTES-${TAG_NAME}.md` so the same file
   feeds the per-release devlog post (no `gh api` round-trip in the
   workflow).

3. Add a `trap` on non-zero exit between step 3 and step 7 that restores
   `VERSION` from `git show HEAD~1:VERSION` and deletes the local tag.
   Today the script can leave `VERSION` ahead of the last real release if
   it half-fails.

### 9.4 Failure-mode fallbacks

1. **No release yet / `gh release view` fails.** Site falls back to
   `VERSION` and synthesizes `tag = "v$(cat VERSION)-ps1"`, `assets = []`,
   sets `_data/release.yml.pending = true`. Layout renders "Build pending —
   grab from Releases page" instead of dead asset links.
2. **Tag exists but assets missing** (`gh` not authed during release).
   Surface raw `https://github.com/.../raw/{tag}/jcreborn.{bin,cue}` as
   secondary asset list.
3. **Workflow itself fails.** Previous deploy stays live
   (`cancel-in-progress: false`); nightly cron self-heals within 24 h.

---

## 10. Accessibility commitments

- **Color contrast**: WCAG AA. 4.5:1 body, 3:1 large text and UI. Test against
  the deep-blue/cream palette; muted text floor is `#d4c5a0` on `--bg-deep`.
- **Pixel-art alt text**: every screenshot is content. Describe what the player
  sees. Holiday emblems include the holiday name and grid position. Decorative
  chrome only gets `alt=""`.
- **Typography**: pixel/bitmap fonts only in headings, eyebrows, badges,
  pause-menu callouts. Body copy is humanist sans ≥ 16 px.
- **Captions page**: model best practice. Plain-text transcript, downloadable
  caption source, any demo MP4 carries `<track kind="captions">` and a visible
  transcript below the player.
- **Keyboard**: visible focus ring on every interactive element. Skip-to-main
  link first focusable. Lightbox dismissable with Escape, focus returns to
  trigger. Code-block "copy" button reachable.
- **Motion**: no autoplaying video or GIFs. Posters with explicit play; honor
  `prefers-reduced-motion: reduce` (cap transitions ≤ 200 ms, disable parallax
  / marquee). No flashing > 3 Hz.
- **Semantic HTML**: one `<h1>` per page. Landmarks `<header> <nav> <main
  id="main"> <aside> <footer>`. Tables use `<th scope>` and `<caption>`.
  Figures use `<figure>/<figcaption>`.

CI gate: `pa11y-ci` against the built site on every PR.

---

## 11. Doc-hygiene rules the site forces back into the repo

These are non-negotiable; the build fails without them.

1. **Front matter on every `docs/ps1/*.md`.** Required keys:
   `title`, `last_updated` (ISO date), `status: current|historical|design`,
   `nav_order`. Pages with `status: historical` auto-inject the "Historical —
   current bar is `{validated}/63`" banner and ship `noindex` + sitemap
   exclusion.
2. **Alt-text discipline.** Every `<img>` in `README.md` and `docs/*.md`
   carries a non-empty `alt=`. CI greps for empty `![]()` and `<img>` without
   `alt=`.
3. **Internal-link checking.** `lychee` over `README.md` + `docs/` in CI.
4. **Single-source assertions.** Pre-commit / CI rule fails the build if
   any hard-coded `v0\.[0-9]+\.[0-9]+-ps1` shows up outside `VERSION`,
   `release.sh`, `CHANGELOG`, and the auto-rewritten stanzas. Same for the
   current validated-scene count outside `scene-status.md`.
5. **Caption tables generated, not edited.** Forbid hand-edits to the FISHING
   captions table in `README.md` once the YAML is canonical — the site builds
   it; humans edit `caption-audit-2026-04-26.yaml`.
6. **`scene-status.md` is the single SoT for the validated count.** All
   downstream stops (`current-status.md`, README hero, site status pill)
   derive from it. The `release.sh` post-step rewrites the version stanza
   in `current-status.md` and `scene-status.md` from `VERSION` before the
   release commit.

---

## 12. OSS surfaces (minimal, info-dump posture)

The user has been explicit (§ 1 non-goals): this is an info dump, not a
collaboration surface. The OSS scaffolding is therefore **minimal**:
license + citation + a single bug-report template. No CONTRIBUTING.md,
no Code of Conduct, no PR template, no issue templates that imply
"please claim this scene."

| File | Path | Site link | Why kept |
|---|---|---|---|
| `LICENSE` (GPL-3.0) | repo root | `/legal/` | legally required |
| `SECURITY.md` | repo root | site footer | one-liner: "email me" |
| `CITATION.cff` | repo root | linked from `/about/` | for academic / archival cite |
| `CHANGELOG.md` (or generated) | repo root | `/play/releases/` | release readers want this |
| `.github/ISSUE_TEMPLATE/bug.yml` | repo | linked from `/faq/` "Where do I file bugs?" | tolerated, not invited |

**Explicitly NOT added** (and why):
- `CONTRIBUTING.md` — there is no contributor process to document.
- `CODE_OF_CONDUCT.md` — no community to govern.
- `.github/PULL_REQUEST_TEMPLATE.md` — implies expected PR volume that
  isn't real.
- `.github/ISSUE_TEMPLATE/scene-bringup.yml` — the "claim a scene" funnel
  was killed with the contributor persona.
- `.github/ISSUE_TEMPLATE/pack-mismatch.yml` — same; hyper-specific issue
  templates assume a contributor population that doesn't exist.

No labels created beyond GitHub defaults. **No Discussions.** Issues
remain enabled because turning them off entirely sends a hostile signal,
but the project doesn't promote them anywhere except the FAQ's *Where
do I file bugs?* answer.

**Badges (concrete)** on README + site header:
- `License: GPL-3.0`
- `Latest release` (auto from GH releases)
- `Scenes validated {n}/63`
- `Built with PSn00bSDK`
- `Runs on DuckStation`
- `Reference scene: FISHING 1`

Skip CI badges until there's actual public CI signal. No "Build: passing"
on an empty workflow.

**OSS anti-patterns to avoid**
1. Public roadmap with target dates (becomes a stick). The scene ledger is
   honest enough.
2. A contributor funnel that nobody walks. Better to skip CONTRIBUTING.md
   entirely than write one that decays unread.
3. Pretending there's a community when there isn't and won't be — no
   Discord badge, no "1000 contributors!", no `governance.md`, no
   Discussions tab, no "good first issue" label. The `/faq/` page is the
   only conversation surface, and it has one author.

**README rebalance** (to ship < 150 lines):
- Keep: 1-paragraph pitch, hero trio, Download links, Quick start, Original
  data files table, license + lineage.
- Move to site: hardware target, full controller mapping, captions design,
  full doc index, scene-ledger embeds, method deep-dive.
- Both: status badge row (links out for full ledger), acknowledgements
  (short on README, full on `/credits/`).
- **Skip entirely:** "Contributing" section, "Roadmap" section, "Star
  history" widget, "Contributors" avatars block.

---

## 13. Phasing

**P1 — MVP shell (1–2 days).**
- Branch `website` already cut. Drop the stale handoff doc.
- Jekyll skeleton: `_config.yml`, six top-nav pages, layout + base scss
  (palette + type), home hero, /play download buttons (hard-code version
  for first build), /credits, /legal.
- `.github/workflows/site.yml` wired to push-on-main + workflow_dispatch.
  Pages enabled.
- Acceptance: site is live at the Pages URL, /play works, /credits and
  /legal are correct, /about lands.

**P2 — Content surfaces (2–3 days).**
- `/scenes/` table from `_data/scenes.yml` (parser added).
- `/scenes/<slug>/` template + per-scene pages for ALL 63 scenes.
- `/devlog/` index + per-post pages (28 worklogs + milestones).
- `/about/method/`, `/about/status/`, `/about/history/`.
- Pagefind integrated; `/` keybind.
- Acceptance: every doc target page has content and is linked from nav.

**P3 — Release-pipeline integration (1 day).**
- Workflow steps 2 + 4 added (release-data ingest, per-release devlog
  generator).
- `scripts/release.sh` patched (workflow kick, --notes-file, trap).
- Acceptance: cut a no-op `v0.3.11-ps1-test` tag, watch site auto-redeploy
  with new version + asset links.

**P4 — Archaeology narrative (2 days).**
- `/archaeology/` 5 chapters written from `timeline.yaml` /
  `team-perspective.yaml` / `tools.yaml`. Hand-written prose, not a YAML
  dump.
- `/archaeology/timeline/` strip + `/archaeology/team/` piece +
  `/archaeology/data/` raw index.
- Acceptance: a stranger can read `/archaeology/` start-to-finish and come
  away with the project's story.

**P5 — Repo hygiene (½ day).**
- `SECURITY.md` (one-liner, email contact), `CITATION.cff` added at root.
- `LICENSE` already present (GPL-3.0); `/legal/` page links to it.
- Single `bug.yml` issue template. No PR template, no CONTRIBUTING.md,
  no CODE_OF_CONDUCT — see § 12.
- Pre-commit hooks for alt-text + version-string lint.
- README rebalanced (< 150 lines), no Contributing section.
- `/faq/` populated with author-written answers.
- Acceptance: a stranger lands on the repo, in 30 seconds knows what
  this is, where to download, and that they're reading documentation —
  not joining a project.

**P6 — Polish + a11y gate (1 day).**
- `pa11y-ci` step. Lighthouse at 95+ across all four scores on the home page.
- Lychee link-check.
- Reduced-motion polish; honor `prefers-color-scheme`.
- Final visual pass: ensure the cyan separator + holiday emblem motif lands
  on every page.
- Acceptance: green CI on the site workflow. No "launch announcement" —
  the site ships when it ships, the URL is the announcement.

**Total: ~8–10 working days.** Each phase is a commit on `website`. P1 + P2
+ P3 are the merge-to-main gate; P4 + P5 + P6 can land separately.

---

## 14. Open questions

Five questions were posed. Four have been answered (decisions captured
inline below); one remains.

1. **Site URL — DECIDED.** `https://hunterdavis.com/johnny-castaway-ps1/`,
   resolved via the user's existing apex CNAME in the user-pages repo. No
   CNAME inside this repo. `_config.yml` uses
   `url: https://hunterdavis.com` + `baseurl: /johnny-castaway-ps1`. All
   internal links use `{{ '/path' | relative_url }}`.
2. **Light/dark mode — DECIDED.** Honor `prefers-color-scheme`. No manual
   toggle in the chrome. Implementation: CSS custom properties scoped to
   `:root` for dark (default for `prefers-color-scheme: dark`), overridden
   inside `@media (prefers-color-scheme: light) { :root { … } }`. The
   palette in § 4.3 already lists both modes; the cream `#F4EAD5` becomes
   the body background under light, `#0D1B2A` the text.
3. **`/credits/` photos — DECIDED.** Text-only. No portraits. Matches the
   drawCredits voice and avoids attribution / image-rights overhead. The
   acknowledgements list is grouped by tier (upstream engine work, asset
   preservation, toolchain, individuals) the same way the existing README
   section does.
4. **Per-scene case studies — DECIDED.** All 63 get a stub page on launch.
   `_data/scenes.yml` is the only thing that varies row-to-row; the page
   template renders both validated and unvalidated scenes from the same
   data. **No "Claim this scene" CTA** — the page is informational only
   (see Q5 / § 1 non-goals). A `⏳` row simply reads "Not yet validated."
5. **Community surface — DECIDED. No community surface at all.**
   The user's framing: *"this is an info dump, not a place for
   collaboration. I'm not asking for help, and nobody else will ever
   want to contribute anyway, that'd be cool but not in 1000 years it's
   just too rare."* That settles a lot of downstream questions:
   - No GitHub Discussions.
   - No CONTRIBUTING.md, no CODE_OF_CONDUCT.md, no PR template, no
     scene-bringup issue template.
   - No "Contributing" persona in the reader-journey table.
   - No "claim this scene" CTAs anywhere.
   - No `/contribute/` page; `development-workflow.md` lives quietly
     under `/docs/dev-workflow/` as the author's own runbook.
   - No "Edit this page" footer.

   **What replaces it: `/faq/` (§ 6.17).** Three sections — *About* /
   *Running it* / *Original game* — with author-written answers. Issues
   stay enabled (turning them off is hostile) but get exactly one
   mention, in the FAQ's "Where do I file bugs?" answer. The site
   doesn't pretend to be more populated than it is, and the chrome
   doesn't beg for contributions that won't come.

---

## 15. Reviewer reports (sources)

This plan is the synthesis of nine focused passes. The full reports are not
checked into the repo (they live in agent transcripts), but the headline
findings are folded above. If something here surprises, the original lenses
were:

1. Doc inventory (Explore agent).
2. Correctness — sync, drift, single-source rules.
3. Visual style — palette, type, pixel-art treatment.
4. Presentation — landing page, hero, CTAs.
5. Branding — name, motifs, disclaimer placement, social card.
6. Voice — registers, rules, anti-patterns, sample openings.
7. Structure & affordance — sitemap, nav, search, breadcrumbs.
8. Reader flow — five personas, MUST-haves, danger zones.
9. Accessibility — WCAG AA, motion policy, semantic HTML.
10. OSS conventions — `CONTRIBUTING`, templates, badges, anti-patterns.
11. Release engineering — Actions workflow, `release.sh` edits, fallbacks.

Anyone picking this up next: read sections 1, 2, 9, 11, 13 first. Then look
at the page-by-page briefs in § 6. Everything else is calibration.
