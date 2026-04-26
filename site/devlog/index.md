---
title: Devlog
eyebrow: Reverse-chronological
subtitle: Worklogs, milestones, release notes — written as they happened.
---

The devlog index lands here in P2. It will surface, in
reverse-chronological order:

- **28 dated worklogs** from `docs/ps1/research/` — verbatim,
  with a one-paragraph editor's note up top ("This is an
  unedited worklog from {date}. The dead ends matter.").
- **Milestone reports** (`milestones-YYYY-MM-DD.md` files), each
  a single-page summary of "where we got to that week."
- **Per-release posts**, auto-generated from the GitHub Release
  body when a `v*-ps1` tag ships. See [release notes for {{ site.release.tag }}]({{ site.github_url }}/releases/tag/{{ site.release.tag }}).

The point of keeping the worklogs verbatim is that the dead ends
mattered as much as the wins. If you're reading the devlog to
learn how the project got here, the wins are not the lesson.

For now, the [GitHub commit history]({{ site.github_url }}/commits/main)
is the most current view. RSS will live at
[/devlog/feed.xml]({{ '/devlog/feed.xml' | relative_url }}) once
the index is populated.
