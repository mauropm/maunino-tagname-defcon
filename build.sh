#!/bin/bash
# Build script for screen_test firmware
# Automatically activates ESP-IDF 5 environment

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR"

echo "=== Screen Test Build Script ==="
echo "Project directory: $PROJECT_DIR"

# Source ESP-IDF environment
if [ -f "$HOME/.zshrc.esp_idf" ]; then
    echo "Activating ESP-IDF 5 environment..."
    source "$HOME/.zshrc.esp_idf"
    use_idf5
else
    echo "ESP-IDF environment configuration not found"
    echo "Please ensure ~/.zshrc.esp_idf exists"
    exit 1
fi

# Navigate to project directory
cd "$PROJECT_DIR"

# Build the project
echo "Building project..."
idf.py build

echo ""
echo "=== Build Complete ==="
echo "Firmware binary: build/screen_test.bin"
echo "To flash: ./flash.sh"
echo "To monitor: ./monitor.sh"
