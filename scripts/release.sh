#!/bin/bash
#
# release.sh - Build, version bump, and release PS1 build
#
# Usage: ./scripts/release.sh [message]
#   message: Optional release message (default: "PS1 release")
#
# This script:
#   1. Runs the full rebuild
#   2. Increments the patch version (e.g., 0.3.0 -> 0.3.1)

if [ "$(id -u)" = "0" ]; then
    echo "ERROR: Do not run this script as root/sudo." >&2
    exit 1
fi
#   3. Copies build artifacts to release/ folder
#   4. Commits changes and creates a git tag
#   5. Pushes to GitHub
#

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
VERSION_FILE="$PROJECT_DIR/VERSION"
RELEASE_DIR="$PROJECT_DIR/release"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}======================================"
echo "PS1 Release Script"
echo -e "======================================${NC}"

# Get release message
RELEASE_MSG="${1:-PS1 release}"

# Read current version
if [[ ! -f "$VERSION_FILE" ]]; then
    echo -e "${RED}ERROR: VERSION file not found at $VERSION_FILE${NC}"
    exit 1
fi

CURRENT_VERSION=$(cat "$VERSION_FILE" | tr -d '[:space:]')
echo -e "${YELLOW}Current version: $CURRENT_VERSION${NC}"

# Parse version components (MAJOR.MINOR.PATCH)
IFS='.' read -r MAJOR MINOR PATCH <<< "$CURRENT_VERSION"

# Increment patch version
NEW_PATCH=$((PATCH + 1))
NEW_VERSION="$MAJOR.$MINOR.$NEW_PATCH"
TAG_NAME="v${NEW_VERSION}-ps1"

echo -e "${GREEN}New version: $NEW_VERSION${NC}"
echo -e "${GREEN}Tag name: $TAG_NAME${NC}"

# Check if tag already exists
if git rev-parse "$TAG_NAME" >/dev/null 2>&1; then
    echo -e "${RED}ERROR: Tag $TAG_NAME already exists!${NC}"
    exit 1
fi

# Step 1: Run the build (build + CD image only; do not launch the emulator)
echo ""
echo -e "${YELLOW}=== Step 1: Building PS1 executable ===${NC}"
"$SCRIPT_DIR/build-ps1.sh" clean
"$SCRIPT_DIR/make-cd-image.sh"

# Check build artifacts exist
if [[ ! -f "$PROJECT_DIR/jcreborn.bin" ]] || [[ ! -f "$PROJECT_DIR/jcreborn.cue" ]]; then
    echo -e "${RED}ERROR: Build artifacts not found (jcreborn.bin/cue)${NC}"
    exit 1
fi

# Step 2: Copy artifacts to release folder
echo ""
echo -e "${YELLOW}=== Step 2: Copying build artifacts to release/ ===${NC}"
mkdir -p "$RELEASE_DIR"
cp "$PROJECT_DIR/jcreborn.bin" "$RELEASE_DIR/"
cp "$PROJECT_DIR/jcreborn.cue" "$RELEASE_DIR/"
echo "Copied jcreborn.bin and jcreborn.cue to release/"

# Step 3: Update VERSION file
echo ""
echo -e "${YELLOW}=== Step 3: Updating VERSION file ===${NC}"
echo "$NEW_VERSION" > "$VERSION_FILE"
echo "Updated VERSION to $NEW_VERSION"

# Step 4: Git commit
echo ""
echo -e "${YELLOW}=== Step 4: Committing changes ===${NC}"
cd "$PROJECT_DIR"
git add VERSION release/jcreborn.bin release/jcreborn.cue

git commit -m "$(cat <<EOF
release: $TAG_NAME - $RELEASE_MSG

🤖 Generated with [Claude Code](https://claude.com/claude-code)

Co-Authored-By: Claude Opus 4.5 <noreply@anthropic.com>
EOF
)"

# Step 5: Create tag
echo ""
echo -e "${YELLOW}=== Step 5: Creating git tag ===${NC}"
git tag -a "$TAG_NAME" -m "$RELEASE_MSG"
echo "Created tag: $TAG_NAME"

# Step 6: Push to GitHub
echo ""
echo -e "${YELLOW}=== Step 6: Pushing to GitHub ===${NC}"
CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)
git push origin "$CURRENT_BRANCH"
git push origin "$TAG_NAME"

# Step 7: Create a GitHub Release with ONLY jcreborn.bin + jcreborn.cue
# as downloadable assets. Requires the `gh` CLI to be authenticated
# (`gh auth login` once per machine). GitHub will still auto-attach
# "Source code (zip|tar.gz)" — that behaviour is baked into the
# platform and cannot be disabled — but the ISO files become the
# primary download and the notes point at them.
echo ""
echo -e "${YELLOW}=== Step 7: Creating GitHub Release with ISO attached ===${NC}"
GH_BIN="$(command -v gh 2>/dev/null || echo "$HOME/bin/gh")"
if [ ! -x "$GH_BIN" ]; then
    echo -e "${YELLOW}gh CLI not found — skipping GitHub Release creation.${NC}"
    echo -e "${YELLOW}Install it from https://cli.github.com/ and run \`gh auth login\`,${NC}"
    echo -e "${YELLOW}then rerun:  \`$GH_BIN release create $TAG_NAME release/jcreborn.bin release/jcreborn.cue --title \"$TAG_NAME\" --notes \"$RELEASE_MSG\"\`${NC}"
else
    if ! "$GH_BIN" auth status >/dev/null 2>&1; then
        echo -e "${YELLOW}gh is installed but not authenticated. Run \`gh auth login\` and then:${NC}"
        echo -e "${YELLOW}  $GH_BIN release create $TAG_NAME release/jcreborn.bin release/jcreborn.cue --title \"$TAG_NAME\" --notes \"$RELEASE_MSG\"${NC}"
    else
        NOTES="$RELEASE_MSG

Download **jcreborn.bin** + **jcreborn.cue** below and load \`jcreborn.cue\` in
a PS1 emulator (DuckStation recommended). The source-code archives that
GitHub auto-attaches are the full repo snapshot at this tag and are not
needed to play the release."
        "$GH_BIN" release create "$TAG_NAME" \
            "$PROJECT_DIR/release/jcreborn.bin" \
            "$PROJECT_DIR/release/jcreborn.cue" \
            --title "$TAG_NAME" \
            --notes "$NOTES"
    fi
fi

echo ""
echo -e "${GREEN}======================================"
echo "Release complete!"
echo "======================================"
echo -e "Version: $NEW_VERSION"
echo -e "Tag: $TAG_NAME"
echo -e "Message: $RELEASE_MSG${NC}"
