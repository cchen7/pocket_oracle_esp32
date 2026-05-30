#pragma once

// HID Report Map — two combined collections so a single BLE HID device
// can act as both a keyboard and a consumer-control device.
//
//   Report ID 1 : Boot-style keyboard input (8 bytes)
//                 [modifier, reserved, kc0, kc1, kc2, kc3, kc4, kc5]
//                 Sufficient for PgUp/PgDn/F5/Esc/Arrows etc.
//   Report ID 2 : Consumer control (2 bytes, little-endian usage code)
//                 Covers Play/Pause, Vol±, Next/Prev, Mute, etc.
//
// V1 of the page-flipper only needs Report 1, but wiring Report 2 now
// is essentially free and unblocks future "media remote" sub-modes
// without re-pairing.

#include <cstdint>

namespace pocket {
namespace ble {

constexpr uint8_t kKeyboardReportId = 1;
constexpr uint8_t kConsumerReportId = 2;

constexpr uint8_t kKeyboardReportSize = 8;
constexpr uint8_t kConsumerReportSize = 2;

// USB HID keyboard usage codes (page 0x07).
namespace key {
constexpr uint8_t kNone       = 0x00;
constexpr uint8_t kEnter      = 0x28;
constexpr uint8_t kEsc        = 0x29;
constexpr uint8_t kRightArrow = 0x4F;
constexpr uint8_t kLeftArrow  = 0x50;
constexpr uint8_t kPageUp     = 0x4B;
constexpr uint8_t kPageDown   = 0x4E;
constexpr uint8_t kF5         = 0x3E;  // start presentation
}  // namespace key

// USB HID Consumer Page (0x0C) usage codes.
namespace consumer {
constexpr uint16_t kPlayPause = 0x00CD;
constexpr uint16_t kNextTrack = 0x00B5;
constexpr uint16_t kPrevTrack = 0x00B6;
constexpr uint16_t kVolumeUp  = 0x00E9;
constexpr uint16_t kVolumeDn  = 0x00EA;
constexpr uint16_t kMute      = 0x00E2;
}  // namespace consumer

inline constexpr uint8_t kReportMap[] = {
    // ---- Keyboard (Report ID 1) ----
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, kKeyboardReportId,
    0x05, 0x07,        //   Usage Page (Key Codes)
    0x19, 0xE0,        //   Usage Minimum (LeftCtrl)
    0x29, 0xE7,        //   Usage Maximum (Right GUI)
    0x15, 0x00,        //   Logical Min (0)
    0x25, 0x01,        //   Logical Max (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data,Var,Abs)  -- modifier byte
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x01,        //   Input (Const)         -- reserved
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Min (0)
    0x25, 0x65,        //   Logical Max (101)
    0x05, 0x07,        //   Usage Page (Key Codes)
    0x19, 0x00,        //   Usage Min (0)
    0x29, 0x65,        //   Usage Max (101)
    0x81, 0x00,        //   Input (Data,Array)    -- 6 keycodes
    0xC0,              // End Collection

    // ---- Consumer Control (Report ID 2) ----
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, kConsumerReportId,
    0x15, 0x00,        //   Logical Min (0)
    0x26, 0xFF, 0x03,  //   Logical Max (1023)
    0x19, 0x00,        //   Usage Min (0)
    0x2A, 0xFF, 0x03,  //   Usage Max (1023)
    0x75, 0x10,        //   Report Size (16)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x00,        //   Input (Data,Array)
    0xC0,              // End Collection
};

constexpr uint16_t kReportMapSize = sizeof(kReportMap);

}  // namespace ble
}  // namespace pocket
