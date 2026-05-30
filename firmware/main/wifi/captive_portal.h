#pragma once

// Captive-portal WiFi provisioner.
//
// Brings up a temporary SoftAP ("PocketOracle-XXXX", open, no password)
// so a phone/laptop can connect to it. A wildcard DNS hijack resolves
// every domain to our AP IP, which triggers iOS/Android/Win captive-
// portal popups; an esp_http_server serves a form for SSID + password.
//
// On form submission the credentials are written to NVS (via storage/
// settings: "wifi_ssid" + "wifi_pass") and `state()` flips to
// kCredsReceived. The owning UI (apps/wifi_setup) is expected to poll
// state(), call portal_stop(), then wifi::sta_reconnect() to bring up
// the real STA connection.

#include "esp_err.h"

#include <cstdint>

namespace pocket {
namespace wifi {

enum class PortalState : uint8_t {
    kInactive,
    kWaitingForUser,    // AP up, no submission yet
    kCredsReceived,     // form posted, NVS saved
};

// Start AP + DNS + HTTP. Idempotent. Returns ESP_OK on success.
esp_err_t portal_start();

// Stop everything; tears down HTTP, DNS task, and the AP interface.
void portal_stop();

PortalState portal_state();

// Human-readable AP SSID ("PocketOracle-A1B2"). Empty if not started.
const char* portal_ap_ssid();

}  // namespace wifi
}  // namespace pocket
