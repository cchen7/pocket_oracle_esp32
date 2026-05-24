// Coin / Dice / 1..10 / Yes/No — four siblings sharing the same UI shape:
// caption near top, big-text result in the center, hint at the bottom. Each
// app overrides pick_new() with its specific random function.

#include "decision_apps.h"

#include "../../ui/theme.h"
#include "../../util/rng.h"

#include "lvgl.h"

#include <cstdio>
#include <cstring>

namespace pocket {

namespace {

// Generic single-result app. Subclasses implement caption() + pick_new().
class SimpleResultApp : public AppBase {
public:
    void on_enter(lv_obj_t* root) override
    {
        lv_obj_set_style_bg_color(root, theme::bg_primary(), LV_PART_MAIN);

        caption_ = lv_label_create(root);
        lv_label_set_text(caption_, caption_text());
        lv_obj_set_style_text_color(caption_, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(caption_, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(caption_, LV_ALIGN_TOP_MID, 0, theme::CONTENT_TOP + theme::SPACE_XS);

        result_ = lv_label_create(root);
        lv_label_set_long_mode(result_, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(result_, theme::SCREEN_W - 2 * theme::SPACE_M);
        lv_obj_set_style_text_align(result_, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_color(result_, theme::accent_main(), LV_PART_MAIN);
        lv_obj_set_style_text_font(result_, theme::font_display(), LV_PART_MAIN);
        lv_obj_align(result_, LV_ALIGN_CENTER, 0, -4);

        subresult_ = lv_label_create(root);
        lv_label_set_text(subresult_, "");
        lv_obj_set_style_text_color(subresult_, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(subresult_, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(subresult_, LV_ALIGN_CENTER, 0, theme::SPACE_M + 2);

        hint_ = lv_label_create(root);
        lv_label_set_text(hint_, hint_text());
        lv_obj_set_style_text_color(hint_, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(hint_, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(hint_, LV_ALIGN_BOTTOM_MID, 0, -theme::SPACE_XS);

        on_initial_pick();
    }

    void on_event(InputEvent ev) override
    {
        if (ev == InputEvent::kButtonShortPress || ev == InputEvent::kShake) {
            on_initial_pick();
            flash_feedback();
        }
    }

protected:
    virtual const char* caption_text() const = 0;
    virtual const char* hint_text() const
    {
        return "tap or shake \xc2\xb7 hold to go back";
    }
    virtual void on_initial_pick() = 0;

    void set_caption(const char* text)
    {
        if (caption_) lv_label_set_text(caption_, text);
    }
    void set_result(const char* text)
    {
        if (result_) lv_label_set_text(result_, text);
    }
    void set_subresult(const char* text)
    {
        if (subresult_) lv_label_set_text(subresult_, text);
    }
    void set_result_color(uint32_t hex)
    {
        if (result_) lv_obj_set_style_text_color(result_, lv_color_hex(hex), LV_PART_MAIN);
    }

    // Quick visual confirmation that a shake/tap was registered even when
    // the random result happens to repeat (50% for Coin, ~10% for 1..10).
    // Opa range is 0..255 (uint8_t) — going to 256 wraps to 0 and the
    // label disappears, so cap at 255.
    void flash_feedback()
    {
        if (!result_) return;
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, result_);
        lv_anim_set_values(&a, 140, 255);
        lv_anim_set_time(&a, 180);
        lv_anim_set_exec_cb(&a, [](void* obj, int32_t v) {
            lv_obj_set_style_text_opa(static_cast<lv_obj_t*>(obj),
                                      (lv_opa_t)v, LV_PART_MAIN);
        });
        lv_anim_start(&a);
    }

private:
    lv_obj_t* caption_   = nullptr;
    lv_obj_t* result_    = nullptr;
    lv_obj_t* subresult_ = nullptr;
    lv_obj_t* hint_      = nullptr;
};

// ----------- Coin -----------

class CoinApp final : public SimpleResultApp {
public:
    const char* name() const override { return "Coin"; }

protected:
    const char* caption_text() const override { return "Flip a coin"; }
    void on_initial_pick() override
    {
        const bool heads = rand_below(2) == 0;
        set_result(heads ? "HEADS" : "TAILS");
        set_subresult("");
        set_result_color(heads ? theme::COLOR_ACCENT_MAIN
                               : theme::COLOR_ACCENT_CALM);
    }
};

// ----------- Random 1..10 -----------

class Random10App final : public SimpleResultApp {
public:
    const char* name() const override { return "1..10"; }

protected:
    const char* caption_text() const override { return "Pick a number"; }
    void on_initial_pick() override
    {
        const uint32_t n = rand_below(10) + 1;
        char buf[8];
        std::snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(n));
        set_result(buf);
        set_subresult("");
        set_result_color(theme::COLOR_ACCENT_MAIN);
    }
};

// ----------- Yes / No -----------

class YesNoApp final : public SimpleResultApp {
public:
    const char* name() const override { return "Yes/No"; }

protected:
    const char* caption_text() const override { return "Yes or No?"; }
    void on_initial_pick() override
    {
        const bool yes = rand_below(2) == 0;
        set_result(yes ? "YES" : "NO");
        set_subresult("");
        set_result_color(yes ? theme::COLOR_ACCENT_MAIN
                             : theme::COLOR_ACCENT_WARN);
    }
};

// ----------- Dice (1 / 3 / 5 / 9) -----------

class DiceApp final : public SimpleResultApp {
public:
    const char* name() const override { return "Dice"; }

    // Override on_event so shake means "next count" (no roll) and tap
    // means "roll the current count". Re-using SimpleResultApp would
    // have shake roll too, which is what we had — but users found the
    // double-meaning confusing.
    void on_event(InputEvent ev) override
    {
        if (ev == InputEvent::kShake) {
            count_idx_ = (count_idx_ + 1) % kNumCounts;
            refresh_caption();
            roll();
            flash_feedback();
        } else if (ev == InputEvent::kButtonShortPress) {
            roll();
            flash_feedback();
        }
    }

protected:
    const char* caption_text() const override
    {
        return current_caption();
    }
    const char* hint_text() const override
    {
        return "tap=roll \xc2\xb7 shake=count \xc2\xb7 hold back";
    }
    void on_initial_pick() override { roll(); }

private:
    static constexpr int kCounts[] = {1, 3, 5, 9};
    static constexpr int kNumCounts = sizeof(kCounts) / sizeof(kCounts[0]);

    const char* current_caption() const
    {
        std::snprintf(caption_buf_, sizeof(caption_buf_),
                      "%d dice", kCounts[count_idx_]);
        return caption_buf_;
    }
    void refresh_caption() { set_caption(current_caption()); }

    void roll()
    {
        const int n = kCounts[count_idx_];
        int rolls[9] = {0};
        int total = 0;
        for (int i = 0; i < n; ++i) {
            rolls[i] = static_cast<int>(rand_below(6)) + 1;
            total += rolls[i];
        }

        if (n == 1) {
            char buf[8];
            std::snprintf(buf, sizeof(buf), "%d", rolls[0]);
            set_result(buf);
            set_subresult("");
        } else {
            char total_buf[8];
            std::snprintf(total_buf, sizeof(total_buf), "%d", total);
            set_result(total_buf);

            char sub[64];
            int pos = 0;
            for (int i = 0; i < n && pos < (int)sizeof(sub) - 4; ++i) {
                pos += std::snprintf(sub + pos, sizeof(sub) - pos,
                                     (i == 0) ? "%d" : "+%d", rolls[i]);
            }
            set_subresult(sub);
        }
        set_result_color(theme::COLOR_ACCENT_MAIN);
    }

    mutable char caption_buf_[16] = {0};
    int  count_idx_ = 0;
};

}  // namespace

std::unique_ptr<AppBase> make_coin_app()     { return std::make_unique<CoinApp>(); }
std::unique_ptr<AppBase> make_dice_app()     { return std::make_unique<DiceApp>(); }
std::unique_ptr<AppBase> make_random10_app() { return std::make_unique<Random10App>(); }
std::unique_ptr<AppBase> make_yesno_app()    { return std::make_unique<YesNoApp>(); }

}  // namespace pocket
