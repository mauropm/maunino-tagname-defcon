#!/bin/bash
# Clean script for screen_test firmware

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR"

echo "=== Screen Test Clean Script ==="
echo "Project directory: $PROJECT_DIR"

# Navigate to project directory
cd "$PROJECT_DIR"

# Clean build artifacts
echo "Cleaning build artifacts..."
idf.py clean

# Remove build directory
if [ -d "build" ]; then
    echo "Removing build directory..."
    rm -rf build
fi

echo ""
echo "=== Clean Complete ==="
