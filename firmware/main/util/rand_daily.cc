#include "rand_daily.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <ctime>

namespace pocket {

namespace {
// 64-bit splitmix to spread day x salt across the output domain.
uint64_t splitmix64(uint64_t x)
{
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}
}  // namespace

uint32_t today_epoch_day()
{
    time_t now = time(nullptr);
    if (now > 0) {
        return static_cast<uint32_t>(now / 86400);
    }
    // Pre-SNTP fallback: bucket by uptime so most app sessions get a
    // stable "today" while the user plays with it.
    const uint32_t up_s = xTaskGetTickCount() * portTICK_PERIOD_MS / 1000;
    return up_s / 86400;
}

uint32_t daily_index(uint32_t salt, uint32_t n)
{
    if (n == 0) return 0;
    const uint64_t mix = splitmix64(
        (uint64_t)today_epoch_day() << 32 | (uint64_t)salt);
    return static_cast<uint32_t>(mix % n);
}

}  // namespace pocket
