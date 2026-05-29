#!/bin/bash
# Conference Badge Build Script for Electronic Cats Minino
# ESP-IDF 5.x - Complete build/flash/monitor workflow

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR"

echo "=== Conference Badge Build Script ==="
echo "Project: conference_badge"
echo "Target: ESP32-C6 (Minino)"
echo "Display: SH1106 128x64 OLED"

source "$HOME/.zshrc.esp_idf"
use_idf5

cd "$PROJECT_DIR"

case "${1:-build}" in
    build)
        echo "Building..."
        idf.py build
        echo "Build complete: build/conference_badge.bin"
        ;;
    flash)
        echo "Flashing..."
        idf.py flash
        ;;
    monitor)
        echo "Starting serial monitor..."
        idf.py monitor
        ;;
    all)
        echo "Building..."
        idf.py build
        echo "Flashing..."
        idf.py flash
        echo "Starting monitor (Ctrl+] to exit)..."
        idf.py monitor
        ;;
    clean)
        echo "Cleaning..."
        idf.py fullclean
        ;;
    *)
        echo "Usage: $0 {build|flash|monitor|all|clean}"
        exit 1
        ;;
esac
