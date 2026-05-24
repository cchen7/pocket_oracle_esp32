#pragma once

// Daily fortune pool — "do" / "avoid" / lucky color. Seeded via
// util/rand_daily so the same calendar day shows the same fortune.

#include <cstdint>

namespace pocket {
namespace data {

inline constexpr const char* kFortuneDo[] = {
    "Start", "Listen", "Ship", "Walk",
    "Call back", "Save", "Plan",
    "Rest", "Read", "Cook",
    "Share", "Move", "Decline",
    "Apologize", "Practice", "Visit",
    "Tidy up", "Reach out", "Try once", "Smile",
};

inline constexpr const char* kFortuneAvoid[] = {
    "Rush", "Compare", "Overspend", "Argue",
    "Doom-scroll", "Skip rest", "Ghost",
    "Force it", "Worry early", "Auto-replies",
    "Multitask", "Negotiate hungry", "Decide angry",
    "New debt", "Big promises", "Speak first",
    "Sit too long", "Late-night news", "All-or-nothing", "Indulge twice",
};

inline constexpr int kFortuneDoCount =
    sizeof(kFortuneDo) / sizeof(kFortuneDo[0]);
inline constexpr int kFortuneAvoidCount =
    sizeof(kFortuneAvoid) / sizeof(kFortuneAvoid[0]);

struct LuckyColor {
    const char* name;
    uint32_t    rgb;
};

inline constexpr LuckyColor kLuckyColors[] = {
    {"Crimson",  0xC0392B},
    {"Coral",    0xE67E22},
    {"Amber",    0xD9B978},
    {"Olive",    0x808000},
    {"Sage",     0x97B498},
    {"Teal",     0x008080},
    {"Sky",      0x6FA8DC},
    {"Indigo",   0x4B5DAB},
    {"Violet",   0x8E44AD},
    {"Rose",     0xE7A6B8},
    {"Stone",    0x7F7F7F},
    {"Bone",     0xF2EEE6},
};

inline constexpr int kLuckyColorCount =
    sizeof(kLuckyColors) / sizeof(kLuckyColors[0]);

}  // namespace data
}  // namespace pocket
