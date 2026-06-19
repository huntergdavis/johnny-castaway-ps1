#!/bin/bash
# Create PS1 CD Image - Uses mkpsxiso to create johnnycastawayps1.bin/.cue
# Usage: ./make-cd-image.sh

set -e  # Exit on error

cd "$(dirname "$0")/.."  # Change to project root
# shellcheck source=./docker-common.sh
source "scripts/docker-common.sh"
docker_init

# Remove old CD image files to prevent mkpsxiso hang
echo "=== Removing old CD image files ==="
rm -f johnnycastawayps1.bin johnnycastawayps1.cue

echo "=== Creating PS1 CD image with mkpsxiso ==="
"${DOCKER_CMD[@]}" run --rm --platform linux/amd64 \
    -v "$PWD":/project \
    jc-reborn-ps1-dev:amd64 \
    mkpsxiso -y /project/config/ps1/cd_layout.xml

echo ""
echo "=== CD image created ==="
ls -lh johnnycastawayps1.bin johnnycastawayps1.cue

exit 0
