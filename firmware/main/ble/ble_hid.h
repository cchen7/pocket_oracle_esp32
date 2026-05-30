#pragma once

// BLE HID (Human Interface Device) — keyboard + consumer-control combo.
//
// Advertises as "Pocket Oracle" with HID appearance. Host pairs once
// (just-works, no PIN) and can then send Page Up / Page Down / Play /
// Volume / etc. from the device.
//
// V1 only uses the keyboard report (PgUp/PgDn) for the page-flipper app;
// consumer codes are wired but unused. Multiple connections are not
// supported — only one host at a time (NimBLE supports more, but it
// complicates the UI with no clear win for V1).
//
// Threading: ble_hid_init() must be called once on the main task. After
// that the NimBLE host owns its own task; send_keyboard()/send_consumer()
// are safe to call from any task.

#include <cstdint>

namespace pocket {
namespace ble {

enum class ConnState : uint8_t {
    kInactive,    // stack not initialized
    kAdvertising, // not yet bonded / not yet connected
    kConnected,   // central is connected and bonded
};

// Bring up NimBLE, register HID + battery + DIS services, start advertising.
// Idempotent: subsequent calls are no-ops. Returns true on success.
bool init(const char* device_name);

// Tear down the BLE stack and stop the radio. Frees ~50KB. Call before
// starting WiFi (the ESP32-S3 has a single 2.4 GHz radio and BLE/WiFi
// coexistence has hard scheduling tradeoffs we don't want for V1).
void deinit();

// Current connection state. Polling-friendly; the UI just reads this
// every refresh tick to update its status line.
ConnState state();

// Convenience: returns true iff state() == kConnected.
bool is_connected();

// Send a single keyboard chord then release. `modifiers` is the standard
// HID modifier byte (Ctrl=0x01, Shift=0x02, Alt=0x04, GUI=0x08, etc.).
// `keycode` is a USB HID keyboard usage (see hid_report_map.h::key).
// No-ops if not connected.
void send_keyboard(uint8_t keycode, uint8_t modifiers = 0);

// Send a single consumer-control code then release. `usage` is from
// Consumer Page 0x0C (see hid_report_map.h::consumer). No-op if not connected.
void send_consumer(uint16_t usage);

}  // namespace ble
}  // namespace pocket
