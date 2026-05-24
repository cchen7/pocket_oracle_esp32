#pragma once

// Deterministic random helpers seeded by the calendar day plus an app
// salt. Until SNTP lands in P5 we fall back to (uptime_seconds / 86400),
// which means everything reads as "day 0" until the first reboot crosses
// a day — but the API contract is stable.

#include <cstdint>

namespace pocket {

// Today's epoch day (UTC-ish). Returns a small integer; identical for
// every call during the same calendar day once SNTP is wired up.
uint32_t today_epoch_day();

// Stable index in [0, n) for the (today, salt) pair. Same salt + same
// day → same index. Use the salt to keep different decks (MBTI prompt
// vs fortune Do vs fortune Avoid) from rolling together.
uint32_t daily_index(uint32_t salt, uint32_t n);

}  // namespace pocket
