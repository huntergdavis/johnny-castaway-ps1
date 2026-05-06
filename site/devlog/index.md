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

{% assign posts_by_month = site.posts | group_by_exp: "p", "p.date | date: '%Y-%m'" %}

<nav class="scenes-jump" aria-label="Jump to month">
  <span class="scenes-jump-label">Jump to:</span>
  {% for group in posts_by_month %}<a href="#month-{{ group.name }}">{{ group.items.first.date | date: "%b %Y" }}</a>{% unless forloop.last %} · {% endunless %}{% endfor %}
</nav>

{% for group in posts_by_month %}
<h2 id="month-{{ group.name }}">{{ group.items.first.date | date: "%B %Y" }}</h2>

<ul class="devlog-list">
  {% for post in group.items %}
  <li>
    <time datetime="{{ post.date | date: '%Y-%m-%d' }}">{{ post.date | date: "%Y-%m-%d" }}</time>
    <div>
      <a href="{{ post.url | relative_url }}">{{ post.title }}</a>
      {% if post.editor_note %}<span class="summary">{{ post.editor_note }}</span>{% endif %}
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
