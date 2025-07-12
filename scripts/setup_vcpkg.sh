#!/bin/bash
# scripts/setup_vcpkg.sh

set -e

VCPKG_ROOT="${VCPKG_ROOT:-$HOME/vcpkg}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Clone vcpkg if not exists
if [ ! -d "$VCPKG_ROOT" ]; then
    echo "Cloning vcpkg..."
    git clone https://github.com/Microsoft/vcpkg.git "$VCPKG_ROOT"
fi

# Bootstrap vcpkg
if [ ! -f "$VCPKG_ROOT/vcpkg" ]; then
    echo "Bootstrapping vcpkg..."
    cd "$VCPKG_ROOT"
    ./bootstrap-vcpkg.sh
fi

# Install dependencies
echo "Installing dependencies..."
cd "$PROJECT_ROOT"
"$VCPKG_ROOT/vcpkg" install --triplet x64-linux

echo "vcpkg setup complete!"
echo "Set environment variable: export VCPKG_ROOT=$VCPKG_ROOT"