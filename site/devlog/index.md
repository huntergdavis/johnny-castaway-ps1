---
title: Devlog
eyebrow: Reverse-chronological
subtitle: Worklogs as they happened. The dead ends matter.
description: Devlog of the Johnny Castaway PS1 fan port — unedited worklogs and milestones.
---

This is the unedited worklog stream for the PS1 port. The posts below
are the same plain-text notes that were written at the keyboard on the
day the work happened. They were not cleaned up before being published
here. They were not rewritten with the benefit of hindsight. The dates
on them are the dates the work was done.

The reason verbatim matters is that most of the time on this project
has been spent on dead ends. A scene that reads "verified" in a status
table today only got there after several attempts that did not. A
single-line timing bug took a week of plans and counter-plans to
pin down. If the only artifacts left over are the wins, the actual
shape of the work is invisible — and the actual shape of the work is
the only useful thing to hand to the next person trying something
similar.

So expect plans that did not survive contact, status snapshots that
were superseded a week later, and worklogs that are honest about
what was tried and what failed. Each post carries an editor's note
above the original text giving brief context, but the body itself
is the source file from `docs/ps1/research/`, preserved as it was
written. A link back to the source on GitHub sits at the bottom of
every post.

Subscribe via
<a href="{{ '/devlog/feed.xml' | relative_url }}" type="application/atom+xml">Atom</a>
or
<a href="{{ '/devlog/feed.json' | relative_url }}" type="application/feed+json">JSON Feed</a>;
both carry full-content posts with absolute URLs. Auto-discovery
is wired into every page's `<head>`, so most feed readers find
them automatically.

{% assign posts_by_month = site.posts | group_by_exp: "p", "p.date | date: '%Y-%m'" %}

<nav class="scenes-jump" aria-label="Jump to month">
  <span class="scenes-jump-label">Jump to:</span>
  {%- comment -%}
    Per-month post count appended after each month name so a reader
    can gauge depth before clicking. Matches the per-family pattern
    on /scenes/ jump nav. Reuses .scenes-jump-count for styling.
  {%- endcomment -%}
  {% for group in posts_by_month %}<a href="#month-{{ group.name }}">{{ group.items.first.date | date: "%b %Y" }} <span class="scenes-jump-count">({{ group.items | size }})</span></a>{% unless forloop.last %} · {% endunless %}{% endfor %}
</nav>

{% for group in posts_by_month %}
<h2 id="month-{{ group.name }}">{{ group.items.first.date | date: "%B %Y" }}</h2>

<ul class="devlog-list">
  {% for post in group.items %}
  {%- assign p_words = post.content | strip_html | number_of_words -%}
  {%- assign p_min = p_words | divided_by: 250 -%}
  {%- if p_min < 1 %}{%- assign p_min = 1 -%}{% endif %}
  <li>
    <time datetime="{{ post.date | date: '%Y-%m-%d' }}">{{ post.date | date: "%Y-%m-%d" }}</time>
    <div>
      <a href="{{ post.url | relative_url }}">{{ post.title }}</a>
      {% if post.editor_note %}<span class="summary">{{ post.editor_note }}</span>{% endif %}
      <span class="summary devlog-read-time">~{{ p_min }} min read · {{ p_words }} words</span>
    </div>
  </li>
  {% endfor %}
</ul>
{% endfor %}

<p class="callout">
  Older material that predates the worklog stream — earlier plans,
  superseded designs, archived experiments — lives under
  <a href="{{ site.github_url }}/tree/main/docs/ps1/research/archive">docs/ps1/research/archive/</a>
  on GitHub. It is not surfaced here on purpose; the index above is the
  active record.
</p>
