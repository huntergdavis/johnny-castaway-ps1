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

# Step 5: Create tag pointing at a minimal orphan commit whose TREE
# contains only jcreborn.bin + jcreborn.cue. Pure git — no GitHub CLI
# required. GitHub's auto-generated "Source code (zip|tar.gz)" download
# for the tag is built from the tag-commit's tree, so with this approach
# downloading the release = downloading just the two ISO files.
echo ""
echo -e "${YELLOW}=== Step 5: Creating minimal-tree release tag ===${NC}"
BIN_BLOB=$(git hash-object -w "$RELEASE_DIR/jcreborn.bin")
CUE_BLOB=$(git hash-object -w "$RELEASE_DIR/jcreborn.cue")
TAG_TREE_SHA=$(printf '100644 blob %s\tjcreborn.bin\n100644 blob %s\tjcreborn.cue\n' \
    "$BIN_BLOB" "$CUE_BLOB" | git mktree)
# Record the main-branch release commit as the tag commit's parent so
# provenance is preserved (git log --all, tag -> commit -> parent chain).
MAIN_RELEASE_SHA=$(git rev-parse HEAD)
TAG_COMMIT_MSG="$TAG_NAME: $RELEASE_MSG

Tree = jcreborn.bin + jcreborn.cue only.
Full source at parent commit $MAIN_RELEASE_SHA."
TAG_COMMIT_SHA=$(printf '%s' "$TAG_COMMIT_MSG" | git commit-tree "$TAG_TREE_SHA" -p "$MAIN_RELEASE_SHA")
git tag -a "$TAG_NAME" -m "$RELEASE_MSG" "$TAG_COMMIT_SHA"
echo "Created tag: $TAG_NAME -> $TAG_COMMIT_SHA (tree has 2 files)"

# Step 6: Push to GitHub
echo ""
echo -e "${YELLOW}=== Step 6: Pushing to GitHub ===${NC}"
CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)
git push origin "$CURRENT_BRANCH"
git push origin "$TAG_NAME"

echo ""
echo -e "${GREEN}======================================"
echo "Release complete!"
echo "======================================"
echo -e "Version: $NEW_VERSION"
echo -e "Tag: $TAG_NAME"
echo -e "Message: $RELEASE_MSG${NC}"
