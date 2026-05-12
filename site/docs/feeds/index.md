---
title: Feeds &amp; well-known endpoints
eyebrow: Reference · discovery
subtitle: Every machine-readable URL on the site — feeds, sitemap, manifest, robots.txt, security.txt, humans.txt, and the JSON-LD records inside each page's head.
description: Reference for every machine-readable endpoint on the Johnny Castaway PS1 site — Atom + JSON feeds for the devlog and lab, hand-rolled sitemap, robots.txt, RFC 9116 security.txt, humans.txt, the W3C web app manifest, and the Schema.org JSON-LD records emitted in every page's head.
---

A labor of love by Hunter Davis. Most of the site is HTML for humans;
this page lists everything that isn't. Each URL has a stable path, a
documented purpose, and an auto-discovery entry in [`_includes/head.html`]({{ site.github_url }}/blob/main/site/_includes/head.html)
so crawlers and feed readers can find it without scraping the body.

If you paid for this, you were cheated. Open source and free.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## Feeds

Four feeds — Atom + JSON for two collections. Both formats carry the
same set of items per collection; pick the one your reader supports.
The devlog feed embeds full post bodies inside `<content>`; the lab
feed is headlines + summaries only (a Jekyll constraint — iterating
`site.html_pages` from another page's render doesn't guarantee each
entry's `.content` has been processed yet, so the feed publishes the
description instead of risking partial bodies).

| Path                | Format         | MIME                       | Body            | Auto-discovery `<link rel="alternate">` title |
| ------------------- | -------------- | -------------------------- | --------------- | --------------------------------------------- |
| `/devlog/feed.xml`  | Atom 1.0       | `application/atom+xml`     | full            | *Devlog (Atom)*                               |
| `/devlog/feed.json` | JSON Feed 1.1  | `application/feed+json`    | full            | *Devlog (JSON Feed)*                          |
| `/lab/feed.xml`     | Atom 1.0       | `application/atom+xml`     | summary only    | *Lab (Atom)*                                  |
| `/lab/feed.json`    | JSON Feed 1.1  | `application/feed+json`    | summary only    | *Lab (JSON Feed)*                             |

All four feeds emit absolute URLs (built from `site.url` +
`site.canonical_baseurl`) so the build's path-portable `--baseurl ""`
override doesn't strip the project prefix from the entry links.

## Discovery

The endpoints that tell crawlers what to crawl and where to find the rest.

[`/sitemap.xml`]({{ '/sitemap.xml' | relative_url }})
: Hand-rolled XML sitemap — no `jekyll-sitemap` plugin (its default
  output would emit URLs without the `/johnny-castaway-ps1` prefix
  because the build runs with `--baseurl ""`). Pages opt out by
  setting `sitemap: false` in front matter; the four feeds, the
  404, robots.txt, security.txt, humans.txt, and the manifest all do.
  Uses `site.canonical_baseurl` directly so absolute URLs survive.

[`/robots.txt`]({{ '/robots.txt' | relative_url }})
: Permissive (`User-agent: *` / `Allow: /`). The only directive that
  matters is the absolute `Sitemap:` line pointing at the sitemap above.
  Auto-generated wrapper pages under `/source/`, `/resources/`, and
  `/archaeology/regtest-references/cases/` opt out of indexing via a
  `<meta name="robots" content="noindex">` tag in their head — not a
  `Disallow:` block here — so the rule stays consistent across crawlers
  that handle the two differently.

[`/.well-known/security.txt`]({{ '/.well-known/security.txt' | relative_url }})
: RFC 9116 responsible-disclosure pointer. Two `Contact:` lines —
  GitHub Issues for public reports, GitHub Security Advisories for
  private. The `Expires:` field forward-dates one year on every
  rebuild via Liquid date math (`site.time + 31536000`), coarsened
  to `T00:00:00Z` so same-day rebuilds don't churn the file.

## Identity

The endpoints that say who wrote this and what it is.

[`/humans.txt`]({{ '/humans.txt' | relative_url }})
: [humanstxt.org](https://humanstxt.org) format. Voice mirrors the
  in-game `drawCredits` screen and the [/credits/]({{ '/credits/' | relative_url }})
  page. Dynamic fields (release tag, build day, scenes validated)
  rendered from Jekyll. Auto-discoverable via
  `<link rel="author" type="text/plain">`.

[`/site.webmanifest`]({{ '/site.webmanifest' | relative_url }})
: W3C web app manifest. The site is *not* a service-worker-backed
  PWA — `display: browser` keeps each page in normal Chrome — but
  the manifest gives mobile browsers a canonical source for
  "Install" / "Add to Home Screen" / app-switcher identity. Theme
  and background colors mirror the light-scheme CSS palette. Three
  icon entries (16/32/180) referencing the same branding assets
  the favicon and apple-touch-icon `<link>` tags use.
  Auto-discoverable via `<link rel="manifest">`.

## Structured data (in every page's `<head>`)

Every page emits one or more [Schema.org](https://schema.org)
JSON-LD records inside its head. Multiple `<script type="application/ld+json">`
blocks are valid — crawlers merge them — so each record is conditional
on the page kind:

| Record type           | Emitted on                                                  |
| --------------------- | ----------------------------------------------------------- |
| `WebSite`             | every page                                                  |
| `SoftwareApplication` | the home page only (`GameApplication` · `Screensaver`)      |
| `BreadcrumbList`      | every page except the home page                             |
| `BlogPosting`         | devlog posts (`layout: post` + `date`)                      |
| `Article`             | lab essays (`/lab/<slug>/` with `date`, excludes the index) |
| `CreativeWork`        | per-scene pages (`layout: scene`)                           |
| `FAQPage`             | [/faq/]({{ '/faq/' | relative_url }})                       |

The emission logic lives in
[`_includes/json-ld.html`]({{ site.github_url }}/blob/main/site/_includes/json-ld.html);
the FAQPage block is inline at the bottom of
[`site/faq/index.md`]({{ site.github_url }}/blob/main/site/faq/index.md)
because it iterates the page's own Q&amp;A list.

## Source on GitHub

- [`site/_includes/head.html`]({{ site.github_url }}/blob/main/site/_includes/head.html) — every auto-discovery `<link>` tag and JSON-LD entry point lives here.
- [`site/_includes/json-ld.html`]({{ site.github_url }}/blob/main/site/_includes/json-ld.html) — the multi-block JSON-LD emitter.
- [`site/devlog/feed.xml`]({{ site.github_url }}/blob/main/site/devlog/feed.xml) · [`site/devlog/feed.json`]({{ site.github_url }}/blob/main/site/devlog/feed.json) — devlog feed sources.
- [`site/lab/feed.xml`]({{ site.github_url }}/blob/main/site/lab/feed.xml) · [`site/lab/feed.json`]({{ site.github_url }}/blob/main/site/lab/feed.json) — lab feed sources.
- [`site/sitemap.xml`]({{ site.github_url }}/blob/main/site/sitemap.xml) — hand-rolled sitemap.
- [`site/robots.txt`]({{ site.github_url }}/blob/main/site/robots.txt) — robots.txt with absolute Sitemap directive.
- [`site/security.txt`]({{ site.github_url }}/blob/main/site/security.txt) — RFC 9116 source (rendered at `/.well-known/security.txt`).
- [`site/humans.txt`]({{ site.github_url }}/blob/main/site/humans.txt) — humans.txt source.
- [`site/site.webmanifest`]({{ site.github_url }}/blob/main/site/site.webmanifest) — web manifest source.

## Related pages

- [Lab: the site itself, as a small program]({{ '/lab/the-site-itself/' | relative_url }}) — the magazine treatment of the Jekyll deployment: canonical_baseurl, no-plugin feeds, the redirect override, the build-stamp coarsening pattern that the security.txt Expires field also uses.
- [Credits]({{ '/credits/' | relative_url }}) — the human-readable counterpart to humans.txt.
- [Legal]({{ '/legal/' | relative_url }}) — the security.txt `Policy:` target.
- [Glossary]({{ '/docs/glossary/' | relative_url }}) — the rest of the site's vocabulary.
