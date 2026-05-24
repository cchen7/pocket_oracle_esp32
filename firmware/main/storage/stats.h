#pragma once

// Tiny NVS-backed counters for app state that has to survive reboots —
// e.g. the muyu merit count. Keeps a single namespace "stats" with
// uint32_t values keyed by name.

#include <cstdint>

namespace pocket {
namespace stats {

// Returns the stored value, or 0 if the key is missing / NVS unavailable.
uint32_t get(const char* key);

// Persists value. Logs and continues on failure.
void set(const char* key, uint32_t value);

}  // namespace stats
}  // namespace pocket
