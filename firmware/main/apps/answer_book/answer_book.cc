// Answer Book — pulls a random line from kAnswerBook. Single-press or shake
// pulls a fresh one. Long-press returns to home (handled globally).

#include "answer_book.h"

#include "../../data/answer_book_data.h"
#include "../../ui/theme.h"
#include "../../util/rng.h"

#include "lvgl.h"

namespace pocket {

namespace {

class AnswerBookApp final : public AppBase {
public:
    const char* name() const override { return "Answer"; }

    void on_enter(lv_obj_t* root) override
    {
        lv_obj_set_style_bg_color(root, theme::bg_primary(), LV_PART_MAIN);

        caption_ = lv_label_create(root);
        lv_obj_set_style_text_font(caption_, theme::font_title_themed(), LV_PART_MAIN);
        lv_obj_set_style_text_color(caption_, theme::ink_primary(), LV_PART_MAIN);
        lv_label_set_text(caption_, "今日一问");
        lv_obj_align(caption_, LV_ALIGN_TOP_MID, 0, theme::CONTENT_TOP + 2);
        lv_obj_invalidate(caption_);

        answer_ = lv_label_create(root);
        lv_label_set_long_mode(answer_, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(answer_, theme::SCREEN_W - 2 * theme::SPACE_M);
        lv_obj_set_style_text_align(answer_, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_color(answer_, theme::ink_color(theme::ink::DAIZI),
                                    LV_PART_MAIN);
        lv_obj_set_style_text_font(answer_, theme::font_body(), LV_PART_MAIN);
        lv_obj_align(answer_, LV_ALIGN_CENTER, 0, 8);

        hint_ = lv_label_create(root);
        lv_label_set_text(hint_, "短按或摇  长按返回");
        lv_obj_set_style_text_color(hint_, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(hint_, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(hint_, LV_ALIGN_BOTTOM_MID, 0, -theme::SPACE_XS);

        pick_new();
    }

    void on_event(InputEvent ev) override
    {
        if (ev == InputEvent::kButtonShortPress
            || ev == InputEvent::kButtonBShortPress
            || ev == InputEvent::kShake) {
            pick_new();
        }
    }

private:
    void pick_new()
    {
        const uint32_t idx = rand_below(data::kAnswerBookCount);
        lv_label_set_text(answer_, data::kAnswerBook[idx]);
    }

    lv_obj_t* caption_ = nullptr;
    lv_obj_t* answer_  = nullptr;
    lv_obj_t* hint_    = nullptr;
};

}  // namespace

std::unique_ptr<AppBase> make_answer_book_app()
{
    return std::make_unique<AnswerBookApp>();
}

}  // namespace pocket
