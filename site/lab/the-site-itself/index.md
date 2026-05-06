---
title: The site itself, as a small program
eyebrow: Methodology · the site
subtitle: A handful of decisions that keep this Jekyll deployment portable, low-noise, and free of plugins it doesn't need.
description: How the Johnny Castaway PS1 site is built — the canonical_baseurl trick, the path-portable build, the no-plugin Atom feed, the pager pattern shared across catalogs, and the build-stamp coarsening that kills per-commit churn.
date: 2026-05-05
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

Why blank the baseurl at build time? Because every URL in every page then comes out root-relative starting at `/`, and a small Python pass — `scripts/site-relativize-build.py` — rewrites those to file-relative paths (`./play/`, `../assets/css/main.css`). The output bundle has no embedded knowledge of where it lives. It can be served at `/johnny-castaway-ps1/`, at `/`, at `/anywhere/`, and every internal link resolves against the actual served path.

That's a useful property for a project hosted at GitHub Pages, where the publish prefix isn't stable across renames or forks. It's also a useful property if anyone ever clones the bundle to host it as a backup, or if the canonical URL ever moves.

The cost: any URL that genuinely needs to be absolute — for crawlers, RSS readers, social previews, redirect targets — has to bypass the relativizer.

## The `canonical_baseurl` workaround

Several pages can't rely on relative URLs:

- The 404 page is served at any URL depth on the project (any not-found path under `/johnny-castaway-ps1/...` lands here). A relative `./play/` from a page that lives at `/johnny-castaway-ps1/404.html` would resolve against the *requested* URL, not the served file's location, so a 404 at `/scenes/typo/foo/` would point its nav at `/scenes/typo/foo/play/`. Broken.
- The Atom feed and JSON Feed have to work in feed readers that fetch them and need full URLs to link back to the site.
- The JSON-LD structured data is consumed by search engines and AI agents, which need fully qualified URIs.
- The redirect HTML pages (from `redirect_from:` frontmatter) emit `<meta http-equiv="refresh">` URLs that browsers resolve as absolute.

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

## The build script removes three files

```bash
# At the end of scripts/site-build-static-root.sh
rm -f "$ROOT/docs/feed.xml" "$ROOT/docs/sitemap.xml" "$ROOT/docs/robots.txt"
```

A standard Jekyll setup with `jekyll-feed`, `jekyll-sitemap`, and the gem-default `robots.txt` would produce all three at the root of `docs/`. On a project page hosted under a user page, those three files at the project's deploy root would conflict with whatever the user-pages repo serves at the apex domain. Specifically:

- `hunterdavis.com/feed.xml` is the user-pages site's job, not this project's.
- `hunterdavis.com/sitemap.xml` likewise.
- `hunterdavis.com/robots.txt` is one file per site; the apex must own it.

The deletion is preventative — none of those files actually get generated today (the plugins aren't enabled), but if a future change pulls in `jekyll-feed` they'd land in the wrong namespace. The `rm` keeps the boundary clean.

The site's own feed lives one level down the tree at `/devlog/feed.xml` (Atom) and `/devlog/feed.json` (JSON Feed). Below the delete line.

## Hand-rolled feeds, no plugin

`jekyll-feed` would have done it in one line of Gemfile. Two reasons it isn't there:

- The plugin emits a top-level `feed.xml`, which gets removed for the reason above.
- The site already has the existing manual head template with explicit OG /
  Twitter meta. Adding the `seo` Liquid tag would double-emit half of that and
  require a refactor to reconcile.

So the feeds are a Liquid template plus an XML/JSON skeleton, in `site/devlog/feed.xml` and `site/devlog/feed.json`. They iterate `site.posts`, escape strings via `xml_escape` (Atom) or `jsonify` (JSON Feed), use absolute URLs via `site.canonical_baseurl`, and carry full HTML post content in CDATA (Atom) or as a JSON string field (JSON Feed). About thirty lines each. They get auto-discovery `<link rel="alternate">` tags in the head, validated with `xml.etree` and `json.load` respectively.

`jekyll-redirect-from` *is* in the Gemfile, because the redirect HTML pages it generates are tedious to write by hand and the plugin's `redirect_from:` frontmatter API is already in use on `scenes/index.md`. There was a bug there, though — the plugin's `absolute_url(to)` honors `site.baseurl`, which the build wipes, so every redirect was silently pointed at `hunterdavis.com/...` (the user-pages root) instead of `hunterdavis.com/johnny-castaway-ps1/...`. The fix is a custom `_layouts/redirect.html` override that strips `site.url` from `page.redirect.to` and rebuilds the URL through `site.canonical_baseurl`. External redirect targets (URLs that don't start with `site.url`) pass through unchanged.

## The pager pattern, shared across three catalogs

The site has three indexed catalogs: 63 scenes, 23 devlog posts, 63 regtest case references. Each was, at some point, a wall of leaves you could only enter via the index page and exit by going back. So each got a prev/up/next pager:

- Scene pages compute prev/next from `_data/scenes.yml`, sorted by `sort: 'tag' | sort: 'ads'` (the same order the index renders).
- Devlog posts use Jekyll's built-in `page.previous` / `page.next`. Caveat: those are sourced from the posts collection's `docs` array, which is sorted oldest-first, so `page.previous` is the *older* post and `page.next` is the *newer* one. Labels here say "older" and "newer" by direction in time, not "prev" and "next" by Jekyll's array semantics — the convention is too easy to invert.
- Regtest case pages compute prev/next from `site.pages` filtered by URL
  prefix, lex-sorted (matching the index table). The case shelf detail pages
  live under `_layouts/page.html`, which conditionally includes the case pager
  only when the URL is under the cases path. Whitespace-control on the Liquid
  `if` block keeps non-case pages byte-identical.

All three pagers reuse one CSS class — `.scene-pager` — because the layout is identical (3-col grid, collapses to prev|next over up on narrow viewports). The class name has lost its specificity but the structure is right. Renaming to `.page-pager` is on the backlog.

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
Twitter card, canonical, theme-color, favicons, fonts, the build stamp, the
feed auto-discovery, the humans.txt link, and a separate include for JSON-LD.

The JSON-LD include uses the multi-block strategy: each schema type gets its own `<script type="application/ld+json">` tag. Crawlers merge multiple blocks per page, so there's no comma juggling between conditionally-emitted records. Four record types ship today:

- `WebSite` on every page.
- `SoftwareApplication` only on the home page (the project is a piece of software).
- `BreadcrumbList` on every non-home page; positions are derived from splitting `page.url` on `/`, with cumulative trail and titlecased segment labels.
- `BlogPosting` only on devlog posts.

All user strings flow through `jsonify` so titles and descriptions with quotes, backslashes, or em-dashes can't break the JSON. Validated with strict `json.loads` across home / a devlog post / about / a scene / a regtest case page.

## The 404 page's problem

GitHub Pages serves `/404.html` from the publish root for any not-found URL within the project's prefix. The 404 file lives at `/johnny-castaway-ps1/404.html` and is served when a user hits `/johnny-castaway-ps1/typo/foo/bar/`. The browser resolves relative URLs against the requested URL, not the served file's location, so a relative `./play/` in the 404's nav would point at `/johnny-castaway-ps1/typo/foo/bar/play/`. That doesn't exist either.

The 404 page is therefore self-contained: `layout: null` (skips the standard chrome), inline minimal CSS (no external stylesheet to also possibly fail), and absolute URLs everywhere via `site.canonical_baseurl`. It uses the original Sierra "The End" scroll graphic as the hero — Johnny waving from his island at sunset is exactly the right vibe for *the page got marooned*.

## Three small extras

- A `humans.txt` at the publish root mirrors the in-game credits voice (`drawCredits`) and lists prior ports, toolchain, this site's standards, and the dynamic release/build fields. Auto-discoverable via `<link rel="author" type="text/plain">`.
- A `@media print` block in `main.scss` flattens the palette to black-on-white, strips chrome, surfaces link URLs via `a::after`, sets `@page` margins, and hints page-break-avoidance on headings, code blocks, figures. Long worklogs save as clean PDFs without any setup.
- A custom `404.html` script reads `window.location.pathname` and renders it as `Tried: /typo/foo/` so a reader can see what was attempted. Degrades cleanly if JS is off.

## The shape

None of this is novel work. Every piece is a Jekyll trick somebody else has done somewhere. The point of writing it down here is that, taken together, these pieces make the site ship-stable, path-portable, low-noise in git, and cheap to extend — and any future me adding a new section to the site will see the existing patterns and follow them instead of inventing a new one. The site is a small program. It rewards being treated like one.
