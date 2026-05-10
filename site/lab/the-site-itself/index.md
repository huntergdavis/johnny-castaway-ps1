---
title: The site itself, as a small program
eyebrow: Methodology · the site
subtitle: A handful of decisions that keep this Jekyll deployment portable, low-noise, and free of plugins it doesn't need.
description: How the Johnny Castaway PS1 site is built — the canonical_baseurl trick, the path-portable build, the no-plugin Atom feed, the pager pattern shared across catalogs, and the build-stamp coarsening that kills per-commit churn.
date: 2026-05-05
image: /assets/img/stand15-ps1-spyglass.png
image_alt: STAND 15 — Johnny on the leftmost shoreline holding a spyglass to his eye, focused on the horizon. The close-inspection-of-small-decisions register the essay walks. Captured on PS1 hardware via the validation harness.
image_width: 961
image_height: 720
---

The PS1 port has a website. The website has its own engineering choices, and most of them aren't documented anywhere because nobody asks. This page is for me, six months from now, when I'm wondering why the build script does that one thing.

The site is Jekyll, hosted on GitHub Pages, served at `hunterdavis.com/johnny-castaway-ps1/` as a project page beneath a separate user-pages site. That last bit — *project* page beneath a *user* page — is where almost every interesting decision comes from.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## The path-portable build

Jekyll's `relative_url` filter takes `site.baseurl` from `_config.yml`. In the deployed config that's `/johnny-castaway-ps1`. In the build pipeline it isn't:

```bash
bundle exec jekyll build --trace --baseurl "" --destination "$ROOT/docs"
```

Why blank the baseurl at build time? Because every URL in every page then comes out root-relative starting at `/`, and a small Python pass — [`scripts/site-relativize-build.py`]({{ site.github_url }}/blob/main/scripts/site-relativize-build.py) — rewrites those to file-relative paths (`./play/`, `../assets/css/main.css`). The output bundle has no embedded knowledge of where it lives. It can be served at `/johnny-castaway-ps1/`, at `/`, at `/anywhere/`, and every internal link resolves against the actual served path.

That's a useful property for a project hosted at GitHub Pages, where the publish prefix isn't stable across renames or forks. It's also a useful property if anyone ever clones the bundle to host it as a backup, or if the canonical URL ever moves.

The cost: any URL that genuinely needs to be absolute — for crawlers, RSS readers, social previews, redirect targets — has to bypass the relativizer.

## The `canonical_baseurl` workaround

Several pages can't rely on relative URLs:

- The 404 page is served at any URL depth on the project (any not-found path under `/johnny-castaway-ps1/...` lands here). A relative `./play/` from a page that lives at `/johnny-castaway-ps1/404.html` would resolve against the *requested* URL, not the served file's location, so a 404 at `/scenes/typo/foo/` would point its nav at `/scenes/typo/foo/play/`. Broken.
- The Atom feed and JSON Feed have to work in feed readers that fetch them and need full URLs to link back to the site.
- The JSON-LD structured data is consumed by search engines and AI agents, which need fully qualified URIs.
- The redirect HTML pages (from `redirect_from:` frontmatter) emit `<meta http-equiv="refresh">` URLs that browsers resolve as absolute.
- The Open Graph and Twitter Card meta tags (`og:url`, `og:image`, `twitter:image`) are read by Slack, Discord, Facebook, X, and assorted link-previewer crawlers fetching the page out-of-band. They require fully qualified URLs and don't resolve relative paths against any reasonable context. (This one was missed for a while: the meta tags shipped through `relative_url` and rendered as `./` and `../../assets/...` post-relativize, silently breaking every social preview until somebody actually inspected the rendered HTML.) The head template also emits `og:image:width` / `og:image:height` so consumers can size the preview slot before the image fetch lands — `1200×630` for the default branding card; per-asset values from `image_width` / `image_height` frontmatter on the five pages that override `page.image`. Lab essays and devlog posts additionally promote to `og:type=article` and emit the OG Article extension fields (`article:published_time` from `page.date`, `article:author`, `article:section` of `Lab` or `Devlog`) so dated-article cards surface authorship and freshness; the index pages and reference manuals stay `og:type=website`. The head also emits `og:site_name` so the site identifier renders above the per-page title in cards (without it, consumers fall back to the URL host), `og:locale=en_US` to match the `inLanguage="en"` set in JSON-LD, and `twitter:image:alt` because Twitter/X does not fall back to `og:image:alt` and screen-reader users on those platforms otherwise heard nothing for the social card image.

`site.baseurl` is empty during the build. So is `site.url + site.baseurl`. The fix is a separate config key that the build can't override:

```yaml
url: "https://hunterdavis.com"
baseurl: "/johnny-castaway-ps1"

# Stable canonical prefix that does NOT get overridden at build time.
canonical_baseurl: "/johnny-castaway-ps1"
```

The pages that need absolute URLs join the configured site URL with
`canonical_baseurl` and the target path. Those URLs start with `https://`,
which the relativizer's `is_external` check leaves alone. So the absolute
URLs pass through untouched while every other path on the page gets
relativized.

Yes, the prefix is duplicated in two config keys. That duplication is intentional: the regular `baseurl` participates in Jekyll's link-resolution machinery and gets blanked by build-time CLI flags, and the `canonical_baseurl` doesn't. They serve different jobs.

## The build script removes two files

```bash
# At the end of scripts/site-build-static-root.sh
rm -f "$ROOT/docs/feed.xml" "$ROOT/docs/robots.txt"
```

A standard Jekyll setup with `jekyll-feed` and the gem-default
`robots.txt` would produce both at the root of `docs/`. On a project
page hosted under a user page, those files at the project's deploy
root would conflict with whatever the user-pages repo serves at the
apex domain:

- `hunterdavis.com/feed.xml` is the user-pages site's job, not this project's.
- `hunterdavis.com/robots.txt` is one file per site; the apex must own it.

The deletion is preventative — neither file actually gets generated today (the plugins aren't enabled), but if a future change pulls in `jekyll-feed` they'd land in the wrong namespace. The `rm` keeps the boundary clean.

The site's own feed lives one level down the tree at `/devlog/feed.xml` (Atom) and `/devlog/feed.json` (JSON Feed). Below the delete line.

The third file in this list used to be `sitemap.xml` and the `rm` originally removed all three. That changed when the site grew a hand-rolled `/sitemap.xml` (around 260 URLs at the current release — down from ~600 because the /source/ wrapper shelf was excluded once it picked up `noindex, follow` so the sitemap stopped advertising URLs the head told crawlers not to index), generated from a Liquid template at `site/sitemap.xml` that uses `site.canonical_baseurl` directly so it survives the `--baseurl ""` build override. Pages opt out via `sitemap: false` front matter (the feeds, the sitemap itself, the 404, redirect stubs). `lastmod` uses `page.date` when present and falls back to the build-day stamp. The `<link rel="sitemap">` autodiscovery tag in `_includes/head.html` points at it. The `rm` line stopped touching `sitemap.xml` so the hand-rolled one survives the build pass.

The build script also runs two perl post-process passes after Jekyll and the relativizer. One is purely cosmetic — strip trailing whitespace, normalize file-trailing newlines — to keep git diffs minimal. The other is an a11y normalization: every `<th>` in the rendered HTML gets `scope="col"` added. Kramdown markdown tables across 460+ pages emit bare `<th>` cells; WCAG H63 wants column headers to declare scope so screen readers correctly associate header→cell relationships when navigating across rows. Adding the attribute in source markdown isn't reasonable across 460 surfaces, and kramdown has no scope-emit option. A single regex pass at the end of the build is the right place: `s|<th>|<th scope="col">|g`. Safe because the site has no row-headers in use; already-marked cells don't match. Skip the preserved project research paths where we don't own the markup. After the pass: ~700 `<th scope="col">` cells across the rendered output, zero bare `<th>`. The same idiom — site-wide HTML normalization in one perl pass — is where similar future adjustments should land.

## Hand-rolled feeds, no plugin

`jekyll-feed` would have done it in one line of Gemfile. Two reasons it isn't there:

- The plugin emits a top-level `feed.xml`, which gets removed for the reason above.
- The site already has the existing manual head template with explicit OG /
  Twitter meta. Adding the `seo` Liquid tag would double-emit half of that and
  require a refactor to reconcile.

So the feeds are a Liquid template plus an XML/JSON skeleton, in `site/devlog/feed.xml` and `site/devlog/feed.json`. They iterate `site.posts`, escape strings via `xml_escape` (Atom) or `jsonify` (JSON Feed), use absolute URLs via `site.canonical_baseurl`, and carry full HTML post content in CDATA (Atom) or as a JSON string field (JSON Feed). About thirty lines each. They get auto-discovery `<link rel="alternate">` tags in the head, validated with `xml.etree` and `json.load` respectively.

The Lab section has its own Atom feed at `/lab/feed.xml` and a JSON Feed counterpart at `/lab/feed.json`. Same pattern, with one wrinkle: lab essays are pages, not posts, so the feed iterates `site.html_pages | sort: 'date' | reverse` and filters to URLs starting with `/lab/`. Embedding `essay.content` in `<![CDATA[...]]>` *should* work the way it does for posts, and it doesn't. Jekyll guarantees `site.posts` are rendered before any other page consumes their `.content`; it doesn't make that guarantee for `site.html_pages`. The first build of the lab feed shipped with raw Markdown and un-rendered Liquid in every `<content>` block. The fix is to drop the body. Atom 1.0 explicitly allows a feed with `<summary>` and no `<content>`, which is the headlines-and-link-back pattern most readers expect for long-form articles anyway. JSON Feed 1.1 has the same allowance — `summary` without `content_html`. Both Lab feeds ship the headlines-and-summary pivot together; the summary text comes from `page.description` (the same string the meta tag uses), with a fallback to `page.subtitle`.

`jekyll-redirect-from` *is* in the Gemfile, because the redirect HTML pages it generates are tedious to write by hand and the plugin's `redirect_from:` frontmatter API is already in use on `scenes/index.md`. There was a bug there, though — the plugin's `absolute_url(to)` honors `site.baseurl`, which the build wipes, so every redirect was silently pointed at `hunterdavis.com/...` (the user-pages root) instead of `hunterdavis.com/johnny-castaway-ps1/...`. The fix is a custom `_layouts/redirect.html` override that strips `site.url` from `page.redirect.to` and rebuilds the URL through `site.canonical_baseurl`. External redirect targets (URLs that don't start with `site.url`) pass through unchanged.

## The pager pattern, shared across four catalogs

The site has four indexed catalogs: 63 scenes, 23 devlog posts, 63 regtest case references, 15 lab essays. Each was, at some point, a wall of leaves you could only enter via the index page and exit by going back. So each got a prev/up/next pager:

- Scene pages compute prev/next from `_data/scenes.yml`, sorted by `sort: 'tag' | sort: 'ads'` (the same order the index renders).
- Devlog posts use Jekyll's built-in `page.previous` / `page.next`. Caveat: those are sourced from the posts collection's `docs` array, which is sorted oldest-first, so `page.previous` is the *older* post and `page.next` is the *newer* one. Labels here say "older" and "newer" by direction in time, not "prev" and "next" by Jekyll's array semantics — the convention is too easy to invert.
- Regtest case pages compute prev/next from `site.pages` filtered by URL
  prefix, lex-sorted (matching the index table). The case shelf detail pages
  live under `_layouts/page.html`, which conditionally includes the case pager
  only when the URL is under the cases path. Whitespace-control on the Liquid
  `if` block keeps non-case pages byte-identical.
- Lab essays compute prev/next the way devlog posts would if Jekyll's
  built-in `page.previous` / `page.next` worked for them — but lab
  essays live under `site.html_pages` (layout: page) rather than
  `site.posts`, so the built-in doesn't apply. Same flag-tracking
  walker the regtest case pager uses, sorted by `page.date` ascending,
  with the older/newer label convention from the devlog pager. The
  head-pagination include uses the same walker on the same sorted
  list, so head-level `<link rel="prev">` and the body-level
  `<a rel="prev">` always land on identical pairs.

All four pagers reuse one CSS class — `.scene-pager` — because the layout is identical (3-col grid, collapses to prev|next over up on narrow viewports). The class name has lost its specificity but the structure is right. Renaming to `.page-pager` is on the backlog.

Above that, a 30-line progressive-enhancement script (`assets/js/key-nav.js`) listens for ArrowLeft/ArrowRight and follows the page's `<a rel="prev">` / `<a rel="next">` links. It doesn't know which pager fired — it queries by `rel` attribute. Skip-out conditions: any modifier key, focus inside an editable element. Works on any future pager that emits the same `rel` attributes without needing a code update.

## The build stamp and the git churn it caused

Every page carries:

```html
<meta name="generator" content="Jekyll 4.4.1; johnny-castaway-ps1 v0.7.2; built 2026-05-06" />
```

That stamp is forensically useful when something breaks on a deployed page and you want to know which build produced it. The first version of this stamp embedded a full ISO-8601 timestamp with second precision. The result: every site rebuild re-diffed all 587 HTML pages, even if the actual change was one line of CSS. Git commits became noise: 590 files changed every time, the diff would have to scroll past 587 trivial timestamp updates to find the real change.

Coarsening the stamp to `%Y-%m-%d` dropped the per-commit churn to zero for in-day rebuilds. Every page that didn't actually change is byte-identical between builds. The first commit after the change (a small new content addition) showed exactly 4 files changed instead of 590 — the win the coarsening was reaching for.

## Structured data without `jekyll-seo-tag`

`jekyll-seo-tag` is in the Gemfile but the `seo` Liquid tag is never invoked,
so the plugin emits nothing. The manual head template handles `<title>`, OG,
Twitter card, canonical, the `theme-color` light/dark pair plus the matching
`color-scheme: light dark` meta (so native UA widgets — scrollbars, form
controls, address-bar tint between navigations — honor the user's
`prefers-color-scheme`), favicons, fonts, the build stamp, the feed
auto-discovery, the humans.txt link, and a separate include for JSON-LD.

The JSON-LD include uses the multi-block strategy: each schema type gets its own `<script type="application/ld+json">` tag. Crawlers merge multiple blocks per page, so there's no comma juggling between conditionally-emitted records. Six record types ship today:

- `WebSite` on every page.
- `SoftwareApplication` only on the home page (the project is a piece of software).
- `BreadcrumbList` on every non-home page; positions are derived from splitting `page.url` on `/`, with cumulative trail and titlecased segment labels. The leaf segment uses `page.title` rather than slug-capitalization so Google's rich-result trail reads `Home > Lab > The two-day SPI bug` instead of `Home > Lab > Two day spi bug`.
- `BlogPosting` only on devlog posts.
- `Article` only on lab essays — URL prefix `/lab/`, excluding the `/lab/` index, requiring `page.date`. Lab essays are dated long-form content, exactly the surface Google's Article structured-data guidance targets, but they live in `site.html_pages` rather than `site.posts` so the `BlogPosting` predicate doesn't catch them.
- `FAQPage` only on `/faq/`, mirroring the page's 14 H3 questions with summary answers. Google retired generic-site FAQ rich results in 2023, but Bing, AI agents, and knowledge graphs still consume FAQPage; zero user-visible bytes.

`Article` and `BlogPosting` both also carry `wordCount` and `timeRequired` (ISO-8601 `PT[N]M`) — the same counts the `~N min read · M words` page-header hint exposes visibly. Computed once at the top of the include and reused across both records.

All user strings flow through `jsonify` so titles and descriptions with quotes, backslashes, or em-dashes can't break the JSON. Validated with strict `json.loads` across home / a devlog post / about / a scene / a regtest case page / `/faq/`.

## The 404 page's problem

GitHub Pages serves `/404.html` from the publish root for any not-found URL within the project's prefix. The 404 file lives at `/johnny-castaway-ps1/404.html` and is served when a user hits `/johnny-castaway-ps1/typo/foo/bar/`. The browser resolves relative URLs against the requested URL, not the served file's location, so a relative `./play/` in the 404's nav would point at `/johnny-castaway-ps1/typo/foo/bar/play/`. That doesn't exist either.

The 404 page is therefore self-contained: `layout: null` (skips the standard chrome), inline minimal CSS (no external stylesheet to also possibly fail), and absolute URLs everywhere via `site.canonical_baseurl`. It uses the original Sierra "The End" scroll graphic as the hero — Johnny waving from his island at sunset is exactly the right vibe for *the page got marooned*.

## A few small extras

- A `humans.txt` at the publish root mirrors the in-game credits voice ([`drawCredits`]({{ '/docs/glossary/#drawcredits' | relative_url }})) and lists prior ports, toolchain, this site's standards, and the dynamic release/build fields. Auto-discoverable via `<link rel="author" type="text/plain">`.
- A `@media print` block in `main.scss` flattens the palette to black-on-white, strips chrome, surfaces link URLs via `a::after`, sets `@page` margins, and hints page-break-avoidance on headings, code blocks, figures. Long worklogs save as clean PDFs without any setup.
- A custom `404.html` script reads `window.location.pathname` and renders it as `Tried: /typo/foo/` so a reader can see what was attempted. Degrades cleanly if JS is off.
- The skip link at the top of every page (`<a class="skip-link" href="#main">`) carries `tabindex="-1"` on its target `<main>` element. Without it, browsers scroll the viewport on activation but leave keyboard focus on the link itself, so the very next Tab dumps users back into the header. The matching CSS rule `main:focus { outline: none }` suppresses the otherwise-giant focus ring around the entire content area — the viewport scroll is the focus indicator, not an outline.
- Scene pages surface their `last_verified` field from `_data/scenes.yml` as a `<time class="scene-verified" datetime="YYYY-MM-DD">` element in the eyebrow row, parallel to the JSON-LD that crawlers consume but visible-and-machine-readable for humans and assistive tech. The one canary scene whose `last_verified` is a release tag (`v0.3.6-ps1`, predating the per-scene daily-validation phase) downgrades to a styled `<span>` since HTML5's `datetime` attribute requires an ISO-shaped value.
- Lab essays render a visible `Published <time datetime="…">…</time>` line in the page header above the existing reading-time hint. The frontmatter already carried `date:` for JSON-LD; the visible echo means a reader landing cold on a war-story retrospective can see at a glance whether they're reading a 3-day-old or a 3-month-old essay without scrolling to the meta layer.

## The auto-generated pages and the f-string rule

Three big surfaces under `site/` aren't hand-written:
`site/source/index.md` (a wrapper page for every Markdown file outside
the website tree), `site/resources/index.md` (the asset catalog with
seven section tables), and
`site/archaeology/regtest-references/cases/index.md` plus its 63
per-case detail pages. They're emitted by
[`scripts/site-generate-library.py`]({{ site.github_url }}/blob/main/scripts/site-generate-library.py) on every build, before Jekyll runs.

The catch is a foot-gun for any future improvement: editing those
`.md` files in place looks fine in `git diff`, builds locally, then
gets silently wiped on the next build because the generator
regenerates them. I learned this the obvious way — added a TOC block
to `site/resources/index.md`, ran the build, watched the TOC vanish.

The rule the project follows now: any change to those three surfaces
goes into the generator's f-string template, not the rendered
markdown. The cost of remembering this once is one merge; the cost of
shipping a "fix" that quietly disappears on the next build is one
honestly-confused contributor and a half-hour of debugging.

The pattern looks like this — note the doubled `{{:toc}}` because
the f-string consumes one pair of braces, leaving Liquid the rest:

```python
index = f"""---
layout: page
title: Resource catalog
...
---

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{{:toc}}
</details>

{resource_sections}
"""
```

Same trick for the case-shelf family jump nav (`<nav class="scenes-jump">`
with per-family counts and `id="ads-<family>"` on the first row of each
group), and for the `<caption class="visually-hidden">` per-table a11y
captions on `/resources/`. All four shipped through the generator
template, not the markdown.

## The chapter-select manifest gap and the in-loop tool

The `v0.8.4-ps1` chapter-select grind shipped a custom thumbnail and a
reconciled scene-page lead for all 63 scenes, plus one bug-fix nobody
expected: a third of the thumbnail SCRs were on disk but never made it
onto the CD because nothing referenced them. The CD ISO is built from
`config/ps1/cd_layout.xml`, which lists every file by name. The
thumbnail-builder script wrote `SX*.SCR` files into the host
filesystem, but only 42 of the 63 had ever been added to the manifest;
the other 21 were silent passengers on disk that the build skipped.
The user found this by walking [Scene Explorer]({{ '/docs/glossary/#scene-explorer' | relative_url }}) and reporting "stand 2-5
and 58-63 don't load."

The site-engineering takeaway is small but concrete: when one source
of truth (the host filesystem) emits files and a different source of
truth (the manifest) enumerates which of them ship, a parity check is
worth keeping. A one-line shell pipeline — `comm -23 <(ls
jc_resources/extracted/scr/SX*.SCR | xargs -n1 basename | sort)
<(grep -oE "SX[A-Z]+[0-9]+\.SCR" config/ps1/cd_layout.xml | sort -u)`
— would have caught the gap before any user did. That check is a
candidate for the build script's pre-flight cluster, alongside the
existing site-redteam pass.

The other small piece worth recording: the loop's 5-surface helper at
[`scripts/apply-scene-correction.py`]({{ site.github_url }}/blob/main/scripts/apply-scene-correction.py) updates the per-scene `index.md`,
the scenes-data YAML, the scene-status table, the thumbnail SCR, and
a local progress tracker in one pass. Every write is an exact-string
match, deliberately — re-running the helper on an already-corrected
scene fails noisily because the old strings aren't there to match.
That's not an accident; it's the design. When the cost of a silent
re-run is "your prior fix is gone and you don't know," idempotent
failure is more honest than idempotent success.

The same pattern shows up in this site's redirect override (the
custom `_layouts/redirect.html` strips a known-stale prefix and
fails on any URL without it) and in the per-scene OG-image
overrides (the head template skips the override if `page.image` is
unset rather than guessing). Different surfaces, same instinct: a
loud failure beats a quiet wrong answer.

## The shape

None of this is novel work. Every piece is a Jekyll trick somebody else has done somewhere. The point of writing it down here is that, taken together, these pieces make the site ship-stable, path-portable, low-noise in git, and cheap to extend — and any future me adding a new section to the site will see the existing patterns and follow them instead of inventing a new one. The site is a small program. It rewards being treated like one.

## Cross-links

- [/sitemap.xml]({{ '/sitemap.xml' | relative_url }}) — the
  hand-rolled sitemap this article documents.
- [/devlog/feed.xml]({{ '/devlog/feed.xml' | relative_url }}) and
  [/devlog/feed.json]({{ '/devlog/feed.json' | relative_url }}) —
  the no-plugin Atom + JSON Feed pair.
- [/lab/feed.xml]({{ '/lab/feed.xml' | relative_url }}) and
  [/lab/feed.json]({{ '/lab/feed.json' | relative_url }}) — the
  Lab section's headlines-and-summary Atom + JSON Feed pair; the
  site.html_pages variant of the same pattern.
- [/humans.txt]({{ '/humans.txt' | relative_url }}) — the credits-
  voice humans.txt file the article describes.
- [404 page]({{ '/404.html' | relative_url }}) —
  the self-contained fallback page described above.
- [/about/voice/]({{ '/about/voice/' | relative_url }}) — the
  prose-side companion to this article's mechanics-side
  discipline.
- [Lab: the dunking bird]({{ '/lab/dunking-bird/' | relative_url }}) —
  the related "small program that rewards being treated like
  one" pattern, applied to keeping LLM agents productive.
