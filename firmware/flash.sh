#!/usr/bin/env bash
# Flash Pocket Oracle to M5StickS3 over USB-C
set -euo pipefail

cd "$(dirname "$0")"

if ! command -v idf.py >/dev/null 2>&1; then
    echo "ERROR: idf.py not found. Source ESP-IDF env first:"
    echo "  source ~/Local/ESP32-proj/esp-idf-v5.4.2/export.sh"
    exit 1
fi

PORT="${1:-}"

if [ -z "$PORT" ]; then
    # Auto-detect macOS USB modem
    PORT="$(ls /dev/cu.usbmodem* 2>/dev/null | head -n1 || true)"
    if [ -z "$PORT" ]; then
        # Linux ACM
        PORT="$(ls /dev/ttyACM* 2>/dev/null | head -n1 || true)"
    fi
fi

if [ -z "$PORT" ]; then
    echo "ERROR: no serial port found. Pass one explicitly: ./flash.sh /dev/cu.usbmodemXXXX"
    exit 1
fi

echo "Flashing to $PORT ..."
idf.py -B build-pocket -p "$PORT" flash
