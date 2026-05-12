---
title: Scenes
eyebrow: Live ledger
subtitle: Every scene the game has, every one's status, no fudge.
redirect_from:
  - /docs/testing-status/
description: The Johnny Castaway scene ledger — 63 scenes, current validation state, last-verified release tag, per-scene case studies.
---

{% assign all_scenes      = site.data.scenes %}
{% assign validated_count = all_scenes | where: "status", "validated"  | size %}
{% assign bringup_count   = all_scenes | where: "status", "in-bringup" | size %}
{% assign pending_count   = all_scenes | where: "status", "pending"    | size %}
{% assign blocked_count   = all_scenes | where: "status", "blocked"    | size %}
{% assign total_count     = all_scenes | size %}

{%- comment -%}
  Schema.org ItemList. /scenes/ is a 63-item catalog of routed scenes;
  ItemList is the Schema.org type Google maps into rich-result
  carousels and AI agents consume as a structured catalog index.
  Items are emitted in the same family-then-tag order as the rendered
  table below (sorted_scenes), so the structured data and the visible
  catalog don't drift.

  `itemListOrder: ItemListUnordered` reflects that the multi-level
  family-then-tag sort isn't a single-key ranking — each ListItem
  still carries an explicit `position`, so consumers that care about
  presentation order have it. site_root construction mirrors
  _includes/json-ld.html for `--baseurl ""` durability.
{%- endcomment -%}
{%- assign jsonld_site_root = site.url | append: site.canonical_baseurl -%}
{%- assign jsonld_scenes = all_scenes | sort: "tag" | sort: "ads" -%}
<script type="application/ld+json">
{
  "@context": "https://schema.org",
  "@type": "ItemList",
  "name": "Johnny Castaway PS1 — Scene ledger",
  "description": {{ "All 63 routed scenes of the Johnny Castaway PS1 fan port, grouped by ADS family and ordered by tag." | jsonify }},
  "url": {{ jsonld_site_root | append: '/scenes/' | jsonify }},
  "inLanguage": "en",
  "numberOfItems": {{ jsonld_scenes.size }},
  "itemListOrder": "https://schema.org/ItemListUnordered",
  "isPartOf": {
    "@type": "WebSite",
    "name": {{ site.title | jsonify }},
    "url": {{ jsonld_site_root | append: '/' | jsonify }}
  },
  "itemListElement": [
  {%- for s in jsonld_scenes -%}
  {%- unless forloop.first %},{% endunless %}
    {
      "@type": "ListItem",
      "position": {{ forloop.index }},
      "name": {{ s.ads | append: ' ' | append: s.tag | jsonify }},
      "url": {{ jsonld_site_root | append: '/scenes/' | append: s.slug | append: '/' | jsonify }}
    }
  {%- endfor %}
  ]
}
</script>

The original Johnny Castaway shipped 63 scenes — short looped vignettes
the screensaver picked from at random. This port aims to reproduce all
63 in original order on the PlayStation 1, with no scenes added or
dropped. The captions and the holiday overlay set are the only things
expanded past the source material.

A scene counts as **validated** when it clears what the project calls
the **FISHING 1 bar**: pixel-perfect visuals against the host-captured
reference, plus SFX cues that land on the same engine ticks, across
every variant flag that applies to the scene (night palette, low-tide
shoreline, holiday overlay, raft-stage progression). That's a higher
bar than "it ran once and didn't crash" — at `{{ site.release.tag }}`
all 63 rows below clear it. The
[63-scene grind retrospective]({{ '/lab/the-63-scene-grind/' | relative_url }})
walks through how the last cluster (the foreground-only multi-view
scenes) cleared the bar. The one-line lead in each row's Notes column
was reconciled against on-PS1 playback in `v0.8.4-ps1`; the
[chapter-select grind retrospective]({{ '/lab/chapter-select-grind/' | relative_url }})
walks through what that loop caught (several caption-mapping audit
guesses had drifted from the gags the discs actually play). Each
row's per-scene page (linked from the table below) carries its own
captured-on-PS1 hero image plus a figcaption that names the gag and
any cross-essay role; the
[63 heroes retrospective]({{ '/lab/63-heroes/' | relative_url }})
walks through the frame-selection rules of thumb and the cross-link
clusters that emerged from writing one figcaption at a time.

The source of truth for this ledger is
[`docs/ps1/scene-status.md`]({{ site.github_url }}/blob/main/docs/ps1/scene-status.md)
in the project repo. The table on this page is generated from
[`site/_data/scenes.yml`]({{ site.github_url }}/blob/main/site/_data/scenes.yml),
which is parsed from that file. Current tally: **{{ validated_count }} of
{{ total_count }} scenes** validated. `{{ site.release.tag }}` is the
release line the validated rows were last verified on. For the longer
explanation of how a scene moves from `pending` to `validated`, see
[the method]({{ '/about/method/' | relative_url }}) and
[the regtest harness]({{ '/docs/regtest/' | relative_url }}). The older
host-capture baselines have their own
[regtest case shelf]({{ '/archaeology/regtest-references/cases/' | relative_url }}),
which is useful when a scene needs to be compared against the frozen desktop
reference instead of the PS1 replay path.

## At a glance

<ul class="scene-counts">
  <li><span class="scene-status ok">validated</span> &nbsp; {{ validated_count }} / {{ total_count }}</li>
  {%- if bringup_count > 0 %}
  <li><span class="scene-status wip">in-bring-up</span> &nbsp; {{ bringup_count }}</li>
  {%- endif %}
  {%- if pending_count > 0 %}
  <li><span class="scene-status pending">pending</span> &nbsp; {{ pending_count }}</li>
  {%- endif %}
  {%- if blocked_count > 0 %}
  <li><span class="scene-status blocked">blocked</span> &nbsp; {{ blocked_count }}</li>
  {%- endif %}
</ul>

## The ledger

Sorted by ADS file then by tag. Scene names link to the per-scene
case study — every scene has one, naming the variants and the
host-vs-PS1 reference frames where applicable.

{% assign sorted_scenes = all_scenes | sort: "tag" | sort: "ads" %}
{% assign families = sorted_scenes | map: "ads" | uniq %}

<nav class="scenes-jump" aria-label="Jump to ADS family">
  <span class="scenes-jump-label">Jump to:</span>
  {%- comment -%}
    Per-family count appended after the ADS name so a reader
    knows what's in each section before clicking. Counts come
    from the same sorted_scenes assign used to build the table,
    so they stay in sync if scenes.yml changes.
  {%- endcomment -%}
  {% for fam in families %}{% assign fam_count = sorted_scenes | where: "ads", fam | size %}<a href="#ads-{{ fam | downcase }}">{{ fam }} <span class="scenes-jump-count">({{ fam_count }})</span></a>{% unless forloop.last %} · {% endunless %}{% endfor %}
  &nbsp;·&nbsp;
  {%- comment -%}
    Random-scene affordance. `data-random-scene-trigger` is the hook
    that /assets/js/random-scene.js looks for. The `hidden` attribute
    keeps the link out of the no-JS view (a "feeling lucky" link
    that doesn't actually randomize would be a regression on /scenes/);
    the JS removes `hidden` only after it has at least one scene URL
    to navigate to. Anchor href is harmless — same page — so even if
    a screen reader exposes the hidden anchor accidentally, clicking
    it is a no-op.
  {%- endcomment -%}
  <a href="#" data-random-scene-trigger hidden>Random scene →</a>
</nav>

<table class="scene-table">
  <caption class="visually-hidden">
    Scene ledger at {{ site.release.tag }}: {{ validated_count }} of
    {{ total_count }} scenes validated under the FISHING 1 bar.
    Columns are ADS-and-tag, scene name, status, last-verified
    release tag, and notes. Rows are grouped by ADS family; use
    the family jump-nav above to skip to a section.
  </caption>
  <thead>
    <tr>
      <th scope="col" class="scene-tag">ADS · tag</th>
      <th scope="col" class="scene-name">Scene</th>
      <th scope="col" class="scene-status">Status</th>
      <th scope="col">Last verified</th>
      <th scope="col">Notes</th>
    </tr>
  </thead>
  <tbody>
    {%- comment -%}
      Two anchor systems coexist on this table:
      - Per-scene row anchor `id="scene-<slug>"` on every <tr> so any
        scene row is deep-linkable (e.g. /scenes/#scene-mary4). The
        :target CSS rule highlights the targeted row; the existing
        scroll-margin-top: 5rem on tr[id] clears the sticky header.
      - Family-group anchor `<a id="ads-<family>">` injected into the
        first cell of each family's first row, kept for the
        scenes-jump nav and any external `#ads-<family>` refs. It's
        visually-hidden but reachable as a fragment target. Empty
        text plus aria-hidden so screen readers walking the row
        don't double-announce the family name. Pulled out of the
        previous one-id-per-tr design so both anchor systems can
        coexist without collision (HTML disallows multiple ids per
        element).
    {%- endcomment -%}
    {% assign current_ads = "" %}
    {% for s in sorted_scenes %}
      {% if s.status == "validated"  %}{% assign cls = "ok"      %}{% endif %}
      {% if s.status == "in-bringup" %}{% assign cls = "wip"     %}{% endif %}
      {% if s.status == "pending"    %}{% assign cls = "pending" %}{% endif %}
      {% if s.status == "blocked"    %}{% assign cls = "blocked" %}{% endif %}
      {% if s.ads != current_ads %}
        {% assign current_ads = s.ads %}
        {% assign family_anchor = true %}
      {% else %}
        {% assign family_anchor = false %}
      {% endif %}
      <tr id="scene-{{ s.slug }}">
        <td class="scene-tag">{% if family_anchor %}<a id="ads-{{ s.ads | downcase }}" class="visually-hidden" aria-hidden="true"></a>{% endif %}{{ s.ads }} {{ s.tag }}</td>
        <td class="scene-name"><a href="{{ '/scenes/' | append: s.slug | append: '/' | relative_url }}">{{ s.slug }}</a></td>
        <td class="scene-status {{ cls }}">{{ s.status }}</td>
        <td>{% if s.last_verified != "" %}{% if s.last_verified contains '-ps1' %}<code>{{ s.last_verified }}</code>{% else %}<time datetime="{{ s.last_verified }}"><code>{{ s.last_verified }}</code></time>{% endif %}{% else %}—{% endif %}</td>
        <td>{{ s.notes }}</td>
      </tr>
    {% endfor %}
  </tbody>
</table>

## Performance battle card

The headless-perf battle card now lives on its own page at
[/perf/]({{ '/perf/' | relative_url }}). Clicking a column header
there sorts the 126-variant matrix by that field; the Target Speed
cells stay color-coded green / yellow / red so a glance gives the
shape.

Validation (the table above) is one bar; performance (the battle
card) is the other. The two ledgers stay separate on purpose —
their failure modes are uncorrelated.

## How to read this

**Status column.**

- `validated` — clears the FISHING 1 bar across every applicable
  variant. Frame-diff and SFX-cue diff both clean.
- `in-bring-up` — the scene's [FG2 pack]({{ '/docs/glossary/#fg2-pack' | relative_url }}) loops without dropping frames
  and the tide variant looks correct, but it has not yet been signed
  off as fishing-1-bar-equivalent. The next likely promotion to
  `validated`.
- `pending` — has not been put through the regtest harness yet, or
  has been but isn't passing. The default state for any scene that
  hasn't had attention paid to it.
- `blocked` — currently can't even be brought up for evaluation. Some
  upstream issue (asset routing, dispatch table, or harness bug) is
  preventing the scene from running at all. Until that's lifted, the
  scene can't be validated.

**Variant flags** (used on the per-scene pages, derived from the source
table):

- `night` — dusk/night palette swap ([BOOTMODE]({{ '/docs/glossary/#bootmode' | relative_url }}) `night 1`).
- `low-tide` — tide-state variant; the shoreline geometry shifts
  (BOOTMODE `lowtide 1`).
- `holiday` — holiday overlay variants (christmas, halloween, etc.;
  BOOTMODE `holiday N`).
- `raft-stage` — cumulative raft-build state; the raft sprite gains
  parts as the player progresses (BOOTMODE `raft-stage N`).

For the caption text, mapping confidences, and which captions are
"least-bad guesses" rather than confirmed matches, see the
[caption audit]({{ '/docs/captions/' | relative_url }}). For the running
log of which scenes graduated and when, see
[the devlog]({{ '/devlog/' | relative_url }}). For source-level artifacts,
start at the [source library]({{ '/source/' | relative_url }}) and
[resource catalog]({{ '/resources/' | relative_url }}).
