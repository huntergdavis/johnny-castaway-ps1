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
rm -f "$ROOT/docs/feed.xml" "$ROOT/docs/robots.txt"

# A11y: kramdown markdown-table headers render as bare `<th>` without
# scope. WCAG H63 wants column headers to declare scope="col" so screen
# readers correctly associate header→cell relationships. Hand-written
# tables across the site already set this attribute (commits 0aa242991
# and 5796b7569); this pass normalizes the kramdown ones too. Safe
# because every `<th>` in the site's rendered HTML is a column header
# (no row-headers in use). Skip preserved project research where we
# don't own the markup.
find "$ROOT/docs" -type f -name '*.html' \
  -not -path "$ROOT/docs/ps1/*" \
  -not -path "$ROOT/docs/archive/*" \
  -not -path "$ROOT/docs/general/*" \
  -not -path "$ROOT/docs/readme/*" \
  -exec perl -pi -e 's|<th>|<th scope="col">|g' {} +

# Whitespace-normalize only the website output, NOT the preserved project
# research living at docs/ps1/, docs/archive/, docs/general/, docs/readme/.
find "$ROOT/docs" -type f \( -name '*.html' -o -name '*.css' -o -name '*.xml' -o -name '*.json' \) \
  -not -path "$ROOT/docs/ps1/*" \
  -not -path "$ROOT/docs/archive/*" \
  -not -path "$ROOT/docs/general/*" \
  -not -path "$ROOT/docs/readme/*" \
  -exec perl -0pi -e 's/[ \t]+$//mg; s/\n+\z/\n/s' {} +
