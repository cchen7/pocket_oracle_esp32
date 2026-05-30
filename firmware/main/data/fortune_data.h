#pragma once

// 今日运势池 — 宜 / 忌 / 幸运色。由 util/rand_daily 按日期种子选择，
// 同一天显示同一签。短按/摇可临时换种子换签（娱乐性质）。

#include <cstdint>

namespace pocket {
namespace data {

inline constexpr const char* kFortuneDo[] = {
    "启程", "聆听", "发布", "步行",
    "回电", "储蓄", "计划",
    "休息", "阅读", "烹饪",
    "分享", "运动", "婉拒",
    "致歉", "练习", "拜访",
    "整理", "联络", "一试", "微笑",
};

inline constexpr const char* kFortuneAvoid[] = {
    "急躁", "攀比", "挥霍", "争辩",
    "刷屏", "熬夜", "失联",
    "强求", "杞忧", "敷衍",
    "分心", "饿议", "怒决",
    "借贷", "空诺", "抢话",
    "久坐", "夜阅", "极端", "复贪",
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
    {"朱砂",  0xC0392B},
    {"珊瑚",  0xE67E22},
    {"琥珀",  0xD9B978},
    {"橄榄",  0x808000},
    {"苍青",  0x97B498},
    {"鸭青",  0x008080},
    {"天青",  0x6FA8DC},
    {"靛蓝",  0x4B5DAB},
    {"紫绛",  0x8E44AD},
    {"蔷薇",  0xE7A6B8},
    {"缃色",  0xD8C078},
    {"月白",  0xF2EEE6},
};

inline constexpr int kLuckyColorCount =
    sizeof(kLuckyColors) / sizeof(kLuckyColors[0]);

}  // namespace data
}  // namespace pocket
