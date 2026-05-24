#!/usr/bin/env bash
# Build Pocket Oracle firmware
set -euo pipefail

cd "$(dirname "$0")"

if ! command -v idf.py >/dev/null 2>&1; then
    echo "ERROR: idf.py not found. Source ESP-IDF env first:"
    echo "  source ~/Local/ESP32-proj/esp-idf-v5.4.2/export.sh"
    exit 1
fi

idf.py -B build-pocket build "$@"
