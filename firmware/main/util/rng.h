#pragma once

// Lightweight RNG helpers wrapping esp_random() so app code doesn't reach
// for ESP-IDF headers directly.

#include <cstdint>

namespace pocket {

// Uniform integer in [0, n). Returns 0 when n == 0.
uint32_t rand_below(uint32_t n);

}  // namespace pocket
