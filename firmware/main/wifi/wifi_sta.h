#pragma once

// WiFi station (STA) mode for normal operation. Auto-connects on boot
// using credentials persisted by the captive-portal provisioner; once
// associated and DHCP'd, kicks off SNTP so gettimeofday() gives wall-
// clock time for the clock + daily-seed apps.
//
// Coexistence: the ESP32-S3 has a single 2.4 GHz radio shared with BLE.
// Bringing up STA while the BLE HID app is active is fine for the radio
// scheduler but eats RAM — for V1 we keep both up at once and accept the
// memory cost; BLE/WiFi mutex is deferred (see todo.md P6.5).

#include "esp_err.h"

#include <cstdint>

namespace pocket {
namespace wifi {

enum class StaState : uint8_t {
    kInactive,    // driver not initialized
    kNoCreds,     // initialized but no SSID in NVS
    kConnecting,  // associating / DHCP in progress
    kConnected,   // got IP, ready
    kFailed,      // last connect attempt failed; backing off
};

// Bring up the WiFi station. Idempotent. If creds are present in NVS
// ("settings"/ssid + pass) starts a connect attempt; otherwise stays in
// kNoCreds. Returns ESP_OK on init success regardless of association.
esp_err_t sta_init();

// Stop the station + free heap + driver. Used before entering captive-
// portal AP mode (we run AP-only, not APSTA, to simplify).
void sta_deinit();

// Re-read creds from NVS and kick a fresh connection attempt. Call after
// the captive portal saves new credentials.
void sta_reconnect();

StaState state();

// Returns the most recent associated SSID (empty if never connected
// this boot). Useful for the Settings → About screen.
const char* current_ssid();

// Returns the most recent assigned IPv4 as a "x.x.x.x" string, or "" if
// not connected.
const char* current_ip();

}  // namespace wifi
}  // namespace pocket
