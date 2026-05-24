#include "rng.h"

#include "esp_random.h"

namespace pocket {

uint32_t rand_below(uint32_t n)
{
    if (n == 0) return 0;
    // Rejection sampling against the largest multiple of n that fits in 32
    // bits — uniform up to the tiniest measurable bias.
    const uint32_t limit = (UINT32_MAX / n) * n;
    uint32_t r;
    do {
        r = esp_random();
    } while (r >= limit);
    return r % n;
}

}  // namespace pocket
