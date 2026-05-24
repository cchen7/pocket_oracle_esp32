#pragma once

// 16 MBTI types x 8 hand-curated daily prompts. Until SNTP lands, the
// active prompt is selected from this pool by a date-or-uptime fallback
// seed in util/rand_daily; once the real clock is up, it stabilizes to
// one prompt per (date, type) pair.

namespace pocket {
namespace data {

struct MbtiType {
    const char* code;            // "INFP"
    const char* nickname;        // "Mediator"
    const char* prompts[8];
};

inline constexpr MbtiType kMbti[] = {
    {"INTJ", "Architect", {
        "Trust your plan today — it's farther along than it feels.",
        "Cut one task that doesn't move the bigger goal.",
        "Sketch the system end-to-end before adding detail.",
        "Say no once today; protect your deep block.",
        "Ask: which decision compounds for a year?",
        "Stop optimizing what should be deleted.",
        "Tell one person what you actually think.",
        "Take one small action — analysis is enough.",
    }},
    {"INTP", "Logician", {
        "Pick one idea and ship a 60% version.",
        "Time-box the rabbit hole to 30 minutes.",
        "Write your thinking down — it sharpens it.",
        "Ask, don't speculate, when info is missing.",
        "One conversation will save you a day of thought.",
        "Choose the boring solution today.",
        "Stop debating, start prototyping.",
        "Rest your mind — clarity comes off-keyboard.",
    }},
    {"ENTJ", "Commander", {
        "Lead by clarifying, not by pushing.",
        "Pick the next concrete deliverable, name an owner.",
        "Listen for the gap, not the disagreement.",
        "Slow down once; speed up everywhere else.",
        "Your standards are a gift if explained.",
        "Delegate the urgent; own the important.",
        "Decide. Reversible beats perfect.",
        "Praise specifically — vague feels hollow.",
    }},
    {"ENTP", "Debater", {
        "Finish the thing you started two weeks ago.",
        "Argue both sides on paper, then choose.",
        "Constraints today; novelty tomorrow.",
        "Don't pitch — just ship a small slice.",
        "One follow-through compounds; ten ideas don't.",
        "Listen to someone you usually interrupt.",
        "Stop polishing the joke; deliver the point.",
        "Pick one promise and keep it cleanly.",
    }},
    {"INFJ", "Advocate", {
        "Say what you mean — kindly, but clearly.",
        "Protect 30 minutes of solitude today.",
        "The pattern you see is real; act on it.",
        "Stop carrying what isn't yours.",
        "Set one boundary you've been meaning to set.",
        "Write the note you've been drafting in your head.",
        "Help one person; rest after.",
        "Your insight is enough; you don't owe an essay.",
    }},
    {"INFP", "Mediator", {
        "Translate the feeling into one concrete step.",
        "Done beats perfect; just press send.",
        "Honor a small ritual today.",
        "Spend 15 minutes on something purely creative.",
        "Say a soft no without explaining.",
        "Trust the value of slow.",
        "Reach out to someone you've been thinking of.",
        "Notice what you defended; that's the value.",
    }},
    {"ENFJ", "Protagonist", {
        "Lead by asking better questions.",
        "Recharge before you give again.",
        "Say the praise out loud, not just in your head.",
        "Let someone struggle a little — it's care.",
        "Decline one task that's not yours.",
        "Share the credit before you take it.",
        "One real conversation > five updates.",
        "Pick your group's next small win.",
    }},
    {"ENFP", "Campaigner", {
        "Finish one idea before chasing the next.",
        "Bring the energy; let the plan be boring.",
        "Park ten ideas; ship one.",
        "Write down what excited you today.",
        "Ask before pitching — match the room.",
        "Rest is the productive choice today.",
        "Say the warm thing; it lands.",
        "Pick the smallest courageous move.",
    }},
    {"ISTJ", "Logistician", {
        "Trust your checklist; you've earned the calm.",
        "One quiet improvement today, not a reform.",
        "Document what you fixed; you'll thank yourself.",
        "Help once where no one's asked yet.",
        "Steady is a strategy.",
        "Decline what doesn't fit the plan.",
        "Verify the source before you worry.",
        "Take the long view — you're good at it.",
    }},
    {"ISFJ", "Defender", {
        "Care for yourself the way you care for others.",
        "Say the small thank-you out loud.",
        "Protect time on your own calendar.",
        "Notice what's right before adjusting what isn't.",
        "Tend the one corner you can today.",
        "Ask for help before it's needed.",
        "Slow down — you're already enough.",
        "Pick the gentle, firm answer.",
    }},
    {"ESTJ", "Executive", {
        "Lead with the next concrete step.",
        "Ask before assigning.",
        "Cut one meeting that could be a memo.",
        "Acknowledge the win before the gap.",
        "Decide; iterate later.",
        "Slow once to listen, then move fast.",
        "Trust your team's judgment today.",
        "Praise out loud, correct in private.",
    }},
    {"ESFJ", "Consul", {
        "Tend the group's morale; you do it well.",
        "Say the appreciative thing first.",
        "Hold one boundary kindly.",
        "Ask, don't assume, what someone needs.",
        "Plan the gathering; people will come.",
        "Save energy for what matters most tonight.",
        "Decline gracefully — that's love too.",
        "Make a small moment feel special.",
    }},
    {"ISTP", "Virtuoso", {
        "Fix one thing properly, not five quickly.",
        "Sleep on the leap of faith.",
        "Make the small useful tool you keep imagining.",
        "Show up where you said you would.",
        "Practice ten minutes; gain a month.",
        "Say the thing once — clearly.",
        "Take care of the tool that takes care of you.",
        "Disengage from the rabbit hole on a timer.",
    }},
    {"ISFP", "Adventurer", {
        "Make something with your hands today.",
        "Your taste is the strategy — trust it.",
        "Move your body before you decide.",
        "Notice the texture of the day.",
        "One quiet, brave move.",
        "Decline the loud option.",
        "Spend time outside, even briefly.",
        "Share the small beautiful thing.",
    }},
    {"ESTP", "Entrepreneur", {
        "Move first; refine later.",
        "Take the meeting in person.",
        "Channel the energy — pick the target.",
        "Hear the room before pushing.",
        "Sleep eight hours; you're stronger for it.",
        "Cash the small win; build the next.",
        "Make the bold call you're avoiding.",
        "Lead the warm-up, not the showdown.",
    }},
    {"ESFP", "Entertainer", {
        "Show up — your presence is the value.",
        "Plan the smallest next step today.",
        "Save a little for tomorrow's energy.",
        "Be the one to make people laugh well.",
        "Say the kind thing you're already thinking.",
        "Move your body; the mood follows.",
        "Pick one practical task and finish it.",
        "Let yourself be quiet for an hour.",
    }},
};

inline constexpr int kMbtiCount = sizeof(kMbti) / sizeof(kMbti[0]);
inline constexpr int kMbtiPromptsPer =
    sizeof(kMbti[0].prompts) / sizeof(kMbti[0].prompts[0]);

}  // namespace data
}  // namespace pocket
