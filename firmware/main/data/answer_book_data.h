#pragma once

// V1 seed pool — 50 hand-picked English answers in the "Magic 8 Ball" tone.
// Future work (P3+): grow to 350 + add a Chinese pool, sourced from
// content/answers_zh.yaml via tools/gen_answer_book.py.

namespace pocket {
namespace data {

inline constexpr const char* kAnswerBook[] = {
    "Yes",
    "No",
    "Maybe",
    "Ask later",
    "Definitely",
    "Don't count on it",
    "Signs point to yes",
    "Without a doubt",
    "Better not tell now",
    "Reply hazy, try again",
    "Concentrate and ask",
    "My sources say no",
    "Outlook is good",
    "Outlook not so good",
    "Cannot predict now",
    "Most likely",
    "Very doubtful",
    "Yes, in due time",
    "Trust your gut",
    "Sleep on it",
    "Do it",
    "Don't do it",
    "Wait one week",
    "Move forward boldly",
    "Hold your ground",
    "Listen, then decide",
    "Take the smaller step",
    "Take the bigger step",
    "Try the opposite",
    "Pick the harder path",
    "Pick the easier path",
    "Ask a friend first",
    "Decide alone",
    "Tomorrow looks better",
    "Today is your day",
    "Patience pays off",
    "Strike while it's warm",
    "Let it go",
    "Hold on tight",
    "Quietly observe",
    "Speak up now",
    "Save your energy",
    "Spend the energy now",
    "Sleep early tonight",
    "Walk before deciding",
    "Drink some water first",
    "Smile and try",
    "Breathe, then act",
    "Wait for the sign",
    "You already know",
};

inline constexpr int kAnswerBookCount =
    sizeof(kAnswerBook) / sizeof(kAnswerBook[0]);

}  // namespace data
}  // namespace pocket
