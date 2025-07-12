#!/bin/bash
# scripts/build.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_TYPE="${1:-Release}"
PRESET="${2:-default}"

echo "Building PixelTree..."
echo "Build type: $BUILD_TYPE"
echo "Preset: $PRESET"

cd "$PROJECT_ROOT"

# Configure
cmake --preset "$PRESET" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

# Build
cmake --build "build/$PRESET" --config "$BUILD_TYPE" -j$(nproc)

# Test (if enabled)
if [ "$3" = "test" ]; then
    echo "Running tests..."
    ctest --test-dir "build/$PRESET" --config "$BUILD_TYPE" --output-on-failure
fi

echo "Build complete!"