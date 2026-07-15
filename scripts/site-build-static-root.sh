#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

python3 "$ROOT/scripts/site-generate-library.py"

cd "$ROOT/site"
if [ -z "${BUNDLE_PATH:-}" ] && [ -d "$ROOT/site/vendor/bundle" ]; then
  export BUNDLE_PATH="vendor/bundle"
fi
BUNDLE_BIN="${BUNDLE_BIN:-bundle}"
if ! command -v "$BUNDLE_BIN" >/dev/null 2>&1 && command -v bundle3.2 >/dev/null 2>&1; then
  BUNDLE_BIN="bundle3.2"
fi

"$BUNDLE_BIN" exec jekyll build \
  --trace \
  --baseurl "" \
  --destination "$ROOT/docs"

python3 "$ROOT/scripts/site-relativize-build.py" "$ROOT/docs"
# Strip Jekyll's site-root feed.xml (the site ships dedicated
# /devlog/feed.xml and /lab/feed.xml; the default at /feed.xml is
# noise). robots.txt is intentionally kept — the hand-rolled
# site/robots.txt emits a Sitemap directive using canonical_baseurl
# so the build's `--baseurl ""` override doesn't strip the project
# prefix from the sitemap URL.
rm -f "$ROOT/docs/feed.xml"

# A11y: kramdown markdown-table headers render as bare `<th>` without
# scope. WCAG H63 wants column headers to declare scope="col" so screen
# readers correctly associate header→cell relationships. Hand-written
# tables across the site already set this attribute (commits 0aa242991
# and 5796b7569); this pass normalizes the kramdown ones too. Safe
# because every `<th>` in the site's rendered HTML is a column header
# (no row-headers in use). Skip preserved project research where we
# don't own the markup.
#
# Two passes: bare `<th>` (no attrs) AND `<th style="text-align: ...">`.
# kramdown emits the style form for any column with `:---:` / `:---` /
# `---:` markers in the source, which the original regex missed —
# left 423 right-aligned column headers across the site without
# scope. Naturally idempotent: the second pattern looks for
# `<th style="..."">` ending immediately at the close-bracket, so
# a re-run on already-scope'd output (`<th style="..." scope="col">`)
# doesn't match.
find "$ROOT/docs" -type f -name '*.html' \
  -not -path "$ROOT/docs/ps1/*" \
  -not -path "$ROOT/docs/archive/*" \
  -not -path "$ROOT/docs/general/*" \
  -not -path "$ROOT/docs/readme/*" \
  -not -path "$ROOT/docs/vendor/*" \
  -exec perl -pi -e 's|<th>|<th scope="col">|g; s|<th style="([^"]*)">|<th style="$1" scope="col">|g;' {} +

# Whitespace-normalize only the website output, NOT the preserved project
# research living at docs/ps1/, docs/archive/, docs/general/, docs/readme/.
find "$ROOT/docs" -type f \( -name '*.html' -o -name '*.css' -o -name '*.xml' -o -name '*.json' \) \
  -not -path "$ROOT/docs/ps1/*" \
  -not -path "$ROOT/docs/archive/*" \
  -not -path "$ROOT/docs/general/*" \
  -not -path "$ROOT/docs/readme/*" \
  -not -path "$ROOT/docs/vendor/*" \
  -exec perl -0pi -e 's/[ \t]+$//mg; s/\n+\z/\n/s' {} +

# Final pass: red-team the website output. Catches broken local links,
# missing fragment anchors (e.g. a glossary cross-ref pointing at a
# section id that doesn't exist), and <img> without alt. Excludes the
# same preserved-research trees the perl passes above skip — those
# subtrees pre-date the site's authoring conventions and are tracked
# separately in the site-improvement backlog. A broken anchor that
# silently ships once cost a follow-up commit (a82c12982); making the
# build fail on it instead means the next typo gets caught at build
# time rather than after merge.
python3 "$ROOT/scripts/site-redteam.py" "$ROOT/docs" \
  --baseurl "/johnny-castaway-ps1" \
  --exclude 'ps1/*' \
  --exclude 'archive/*' \
  --exclude 'general/*' \
  --exclude 'readme/*' \
  --exclude 'vendor/*'
