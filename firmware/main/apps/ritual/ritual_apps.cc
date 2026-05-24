// MBTI 16-personality daily prompts + 宜/忌/lucky color fortune.
// Both apps select content with util/rand_daily so the same calendar day
// yields the same result; shake = cycle to next type (MBTI) or re-roll
// the day's seed (Fortune, for fun). Tap = re-roll for variety.

#include "ritual_apps.h"

#include "../../data/mbti_data.h"
#include "../../data/fortune_data.h"
#include "../../ui/theme.h"
#include "../../util/rand_daily.h"
#include "../../util/rng.h"

#include "lvgl.h"

#include <cstdio>

namespace pocket {

namespace {

// ----------- MBTI -----------

class MbtiApp final : public AppBase {
public:
    const char* name() const override { return "MBTI"; }

    void on_enter(lv_obj_t* root) override
    {
        lv_obj_set_style_bg_color(root, theme::bg_primary(), LV_PART_MAIN);

        // Pick the type we'll show today. P7 will let the user pin one;
        // until then we walk through all 16 by daily seed.
        type_idx_ = daily_index(/*salt=*/0xA1B2u, data::kMbtiCount);

        code_ = lv_label_create(root);
        lv_obj_set_style_text_color(code_, theme::accent_main(), LV_PART_MAIN);
        lv_obj_set_style_text_font(code_, theme::font_title(), LV_PART_MAIN);
        lv_obj_align(code_, LV_ALIGN_TOP_LEFT,
                     theme::SPACE_S, theme::CONTENT_TOP + 2);

        nickname_ = lv_label_create(root);
        lv_obj_set_style_text_color(nickname_, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(nickname_, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(nickname_, LV_ALIGN_TOP_RIGHT,
                     -theme::SPACE_S, theme::CONTENT_TOP + 6);

        prompt_ = lv_label_create(root);
        lv_label_set_long_mode(prompt_, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(prompt_, theme::SCREEN_W - 2 * theme::SPACE_M);
        lv_obj_set_style_text_color(prompt_, theme::ink_primary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(prompt_, theme::font_body(), LV_PART_MAIN);
        lv_obj_set_style_text_align(prompt_, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_align(prompt_, LV_ALIGN_CENTER, 0, 4);

        hint_ = lv_label_create(root);
        lv_label_set_text(hint_, "tap=prompt \xc2\xb7 shake=type \xc2\xb7 hold back");
        lv_obj_set_style_text_color(hint_, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(hint_, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(hint_, LV_ALIGN_BOTTOM_MID, 0, -theme::SPACE_XS);

        repaint();
    }

    void on_event(InputEvent ev) override
    {
        if (ev == InputEvent::kShake) {
            type_idx_ = (type_idx_ + 1) % data::kMbtiCount;
            repaint();
        } else if (ev == InputEvent::kButtonShortPress) {
            prompt_idx_ = (prompt_idx_ + 1 + rand_below(data::kMbtiPromptsPer - 1))
                          % data::kMbtiPromptsPer;
            repaint_prompt_only();
        }
    }

private:
    void repaint()
    {
        const auto& t = data::kMbti[type_idx_];
        lv_label_set_text(code_, t.code);
        lv_label_set_text(nickname_, t.nickname);
        prompt_idx_ = daily_index(/*salt=*/0xC0DEu ^ type_idx_,
                                  data::kMbtiPromptsPer);
        repaint_prompt_only();
    }
    void repaint_prompt_only()
    {
        lv_label_set_text(prompt_, data::kMbti[type_idx_].prompts[prompt_idx_]);
    }

    lv_obj_t* code_     = nullptr;
    lv_obj_t* nickname_ = nullptr;
    lv_obj_t* prompt_   = nullptr;
    lv_obj_t* hint_     = nullptr;
    int type_idx_   = 0;
    int prompt_idx_ = 0;
};

// ----------- Fortune -----------

class FortuneApp final : public AppBase {
public:
    const char* name() const override { return "Fortune"; }

    void on_enter(lv_obj_t* root) override
    {
        lv_obj_set_style_bg_color(root, theme::bg_primary(), LV_PART_MAIN);

        // Two columns side by side.
        const int32_t col_w = theme::SCREEN_W / 2;

        do_label_ = lv_label_create(root);
        lv_label_set_text(do_label_, "DO");
        lv_obj_set_style_text_color(do_label_, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(do_label_, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(do_label_, LV_ALIGN_TOP_LEFT,
                     theme::SPACE_S, theme::CONTENT_TOP + 2);

        avoid_label_ = lv_label_create(root);
        lv_label_set_text(avoid_label_, "AVOID");
        lv_obj_set_style_text_color(avoid_label_, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(avoid_label_, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(avoid_label_, LV_ALIGN_TOP_RIGHT,
                     -theme::SPACE_S, theme::CONTENT_TOP + 2);

        do_text_ = lv_label_create(root);
        lv_label_set_long_mode(do_text_, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(do_text_, col_w - 2 * theme::SPACE_S);
        lv_obj_set_style_text_color(do_text_, theme::accent_main(), LV_PART_MAIN);
        lv_obj_set_style_text_font(do_text_, theme::font_title(), LV_PART_MAIN);
        lv_obj_set_style_text_align(do_text_, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_align(do_text_, LV_ALIGN_LEFT_MID, theme::SPACE_S, -theme::SPACE_S);

        avoid_text_ = lv_label_create(root);
        lv_label_set_long_mode(avoid_text_, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(avoid_text_, col_w - 2 * theme::SPACE_S);
        lv_obj_set_style_text_color(avoid_text_, theme::accent_warn(), LV_PART_MAIN);
        lv_obj_set_style_text_font(avoid_text_, theme::font_title(), LV_PART_MAIN);
        lv_obj_set_style_text_align(avoid_text_, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_align(avoid_text_, LV_ALIGN_RIGHT_MID, -theme::SPACE_S, -theme::SPACE_S);

        color_block_ = lv_obj_create(root);
        lv_obj_remove_style_all(color_block_);
        lv_obj_set_size(color_block_, 14, 14);
        lv_obj_set_style_radius(color_block_, 3, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(color_block_, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_align(color_block_, LV_ALIGN_BOTTOM_LEFT,
                     theme::SPACE_S, -theme::SPACE_XS);

        color_text_ = lv_label_create(root);
        lv_obj_set_style_text_color(color_text_, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(color_text_, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(color_text_, LV_ALIGN_BOTTOM_LEFT,
                     theme::SPACE_S + 20, -theme::SPACE_XS);

        repaint();
    }

    void on_event(InputEvent ev) override
    {
        if (ev == InputEvent::kShake || ev == InputEvent::kButtonShortPress) {
            // Re-roll the salt — pure entertainment value, since
            // canonical fortune is daily.
            salt_offset_ = static_cast<int>(rand_below(0x10000));
            repaint();
        }
    }

private:
    void repaint()
    {
        const int do_i = static_cast<int>(daily_index(
            0xD0u + salt_offset_, data::kFortuneDoCount));
        const int av_i = static_cast<int>(daily_index(
            0xAAu + salt_offset_, data::kFortuneAvoidCount));
        const int cl_i = static_cast<int>(daily_index(
            0xC1u + salt_offset_, data::kLuckyColorCount));

        lv_label_set_text(do_text_, data::kFortuneDo[do_i]);
        lv_label_set_text(avoid_text_, data::kFortuneAvoid[av_i]);

        const auto& c = data::kLuckyColors[cl_i];
        lv_obj_set_style_bg_color(color_block_,
                                  lv_color_hex(c.rgb), LV_PART_MAIN);
        char buf[40];
        std::snprintf(buf, sizeof(buf), "Lucky color: %s", c.name);
        lv_label_set_text(color_text_, buf);
    }

    lv_obj_t* do_label_    = nullptr;
    lv_obj_t* do_text_     = nullptr;
    lv_obj_t* avoid_label_ = nullptr;
    lv_obj_t* avoid_text_  = nullptr;
    lv_obj_t* color_block_ = nullptr;
    lv_obj_t* color_text_  = nullptr;
    int salt_offset_ = 0;
};

}  // namespace

std::unique_ptr<AppBase> make_mbti_app()    { return std::make_unique<MbtiApp>(); }
std::unique_ptr<AppBase> make_fortune_app() { return std::make_unique<FortuneApp>(); }

}  // namespace pocket
