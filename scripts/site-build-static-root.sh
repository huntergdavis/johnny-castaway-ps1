#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

python3 "$ROOT/scripts/site-generate-library.py"

cd "$ROOT/site"
BUNDLE_BIN="${BUNDLE_BIN:-bundle}"
if ! command -v "$BUNDLE_BIN" >/dev/null 2>&1 && command -v bundle3.2 >/dev/null 2>&1; then
  BUNDLE_BIN="bundle3.2"
fi

"$BUNDLE_BIN" exec jekyll build \
  --trace \
  --baseurl "" \
  --destination "$ROOT/www"

python3 "$ROOT/scripts/site-relativize-build.py" "$ROOT/www"
rm -f "$ROOT/www/feed.xml" "$ROOT/www/sitemap.xml" "$ROOT/www/robots.txt"
find "$ROOT/www" -type f \( -name '*.html' -o -name '*.css' -o -name '*.xml' -o -name '*.json' \) \
  -exec perl -0pi -e 's/[ \t]+$//mg; s/\n+\z/\n/s' {} +
