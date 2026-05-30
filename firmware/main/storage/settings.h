#pragma once

// Generic NVS-backed key/value store for user preferences and configuration
// that has to survive reboots — WiFi credentials, brightness, dark mode,
// MBTI, language, etc.
//
// All keys live in NVS namespace "settings". Two value types are supported:
//   - u32: brightness, volume, boolean flags
//   - str: SSID, password, MBTI code, language tag
//
// String keys silently truncate at kMaxStrLen (NVS itself caps at 4000B,
// but we pre-allocate small bounded buffers).
//
// Threading: NVS API is internally locked; calls are safe from any task.
// Loss tolerance: failures are logged via ESP_LOGW and the call returns
// the default — callers should never read settings on a hot path that
// can't tolerate a momentary stale value (boot-time read is the norm).

#include <cstdint>
#include <cstddef>
#include <string>

namespace pocket {
namespace settings {

// Hard cap for any single string value (incl. null terminator). 64 covers
// WiFi SSID (32 max per spec) and password (63 max for WPA2-PSK + null).
// Keys that need bigger payloads should use a different namespace.
constexpr std::size_t kMaxStrLen = 64;

// Returns true iff `key` is present in the settings namespace.
bool has(const char* key);

// Removes `key`. No-op if missing. Returns true on success.
bool erase(const char* key);

// ---- u32 ----
uint32_t get_u32(const char* key, uint32_t fallback = 0);
void     set_u32(const char* key, uint32_t value);

// ---- string ----
// Read into out_buf (kMaxStrLen capacity assumed by caller). Returns true
// if a value was loaded; false leaves out_buf as an empty string.
bool get_str(const char* key, char* out_buf, std::size_t buf_len);

// Convenience: returns std::string, empty if unset.
std::string get_str(const char* key);

void set_str(const char* key, const char* value);

}  // namespace settings
}  // namespace pocket
