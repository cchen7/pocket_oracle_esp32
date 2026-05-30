// Home menu — single-card carousel.
//
// Each home position is a full-screen 240x135 PNG cover (rendered by
// tools/gen_covers.py at design time, embedded as RGB565 in
// assets/home_covers.h). Navigation:
//
//   BtnA short  -> next cover (slides left, ease-out 280ms)
//   shake       -> next cover (BtnA alias)
//   BtnB short  -> enter the app the current cover represents
//
// The persistent status bar overlays the top 16 px of every screen
// including this one, so the cover art deliberately leaves that band
// empty (see tools/gen_covers.py).

#include "home_menu.h"

#include "../../app/app_router.h"
#include "../../assets/home_covers.h"
#include "../../ui/theme.h"
#include "../answer_book/answer_book.h"
#include "../ble_remote/ble_remote.h"
#include "../decision/decision_apps.h"
#include "../ritual/ritual_apps.h"
#include "../settings/settings_app.h"
#include "../tools/tool_apps.h"

#include "esp_log.h"
#include "lvgl.h"

namespace pocket {

namespace {

constexpr const char* TAG = "HOME";
constexpr int kCount = assets::kHomeCoverCount;
constexpr uint32_t kSlideMs = 280;

// Forward declarations of all per-app factories so the carousel can
// dispatch them by cursor index without pulling in a registry.
std::unique_ptr<AppBase> stub_app(const char* label);

class StubApp final : public AppBase {
public:
    explicit StubApp(const char* label) : label_(label) {}
    const char* name() const override { return label_; }
    void on_enter(lv_obj_t* root) override
    {
        lv_obj_set_style_bg_color(root, theme::bg_primary(), LV_PART_MAIN);

        lv_obj_t* title = lv_label_create(root);
        lv_label_set_text(title, label_);
        lv_obj_set_style_text_color(title, theme::accent_main(), LV_PART_MAIN);
        lv_obj_set_style_text_font(title, theme::font_title(), LV_PART_MAIN);
        lv_obj_align(title, LV_ALIGN_CENTER, 0, -8);

        lv_obj_t* sub = lv_label_create(root);
        lv_label_set_text(sub, "Coming soon");
        lv_obj_set_style_text_color(sub, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(sub, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(sub, LV_ALIGN_CENTER, 0, 14);
    }
private:
    const char* label_;
};

std::unique_ptr<AppBase> stub_app(const char* label)
{
    return std::make_unique<StubApp>(label);
}

const char* kStubLabels[kCount] = {
    "Answer", "Coin", "Dice", "Random",
    "Yes/No", "MBTI", "Fortune", "Clock",
    "Muyu", "BLE", "Battery", "Settings",
};

class HomeMenuApp final : public AppBase {
public:
    const char* name() const override { return "Home"; }

    void on_enter(lv_obj_t* root) override
    {
        root_ = root;
        lv_obj_set_style_bg_color(root, theme::bg_primary(), LV_PART_MAIN);
        lv_obj_set_style_pad_all(root, 0, LV_PART_MAIN);

        current_ = make_cover(cursor_);
        lv_obj_set_pos(current_, 0, 0);
    }

    void on_event(InputEvent ev) override
    {
        if (animating_) return;
        switch (ev) {
            case InputEvent::kShake:
            case InputEvent::kButtonShortPress:
                advance(+1);
                break;
            case InputEvent::kButtonBShortPress:
                ESP_LOGD(TAG, "enter cursor=%d", cursor_);
                app_router_push(make_for_cursor(cursor_));
                break;
            default: break;
        }
    }

    void on_exit() override
    {
        // Router clears the root container; our pointers will dangle.
        current_ = nullptr;
        root_    = nullptr;
        animating_ = false;
    }

private:
    lv_obj_t* make_cover(int idx) const
    {
        lv_obj_t* img = lv_image_create(root_);
        lv_image_set_src(img, theme::current().home_covers[idx]);
        return img;
    }

    void advance(int dir)
    {
        const int new_cursor = (cursor_ + dir + kCount) % kCount;
        const int32_t W = theme::SCREEN_W;

        lv_obj_t* incoming = make_cover(new_cursor);
        lv_obj_set_pos(incoming, dir > 0 ? W : -W, 0);

        animating_ = true;

        // Slide outgoing off-screen, then delete it on completion.
        animate_x(current_, 0, dir > 0 ? -W : W, /*delete_after=*/true);
        // Slide incoming into view. Use this animation's completed
        // callback to clear the animating_ guard.
        animate_x(incoming, dir > 0 ? W : -W, 0, /*delete_after=*/false);

        current_ = incoming;
        cursor_  = new_cursor;
    }

    void animate_x(lv_obj_t* obj, int32_t from, int32_t to, bool delete_after)
    {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, obj);
        lv_anim_set_values(&a, from, to);
        lv_anim_set_duration(&a, kSlideMs);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
        lv_anim_set_exec_cb(&a, [](void* o, int32_t v) {
            lv_obj_set_x(static_cast<lv_obj_t*>(o), v);
        });
        if (delete_after) {
            lv_anim_set_completed_cb(&a, [](lv_anim_t* an) {
                lv_obj_delete(static_cast<lv_obj_t*>(an->var));
            });
        } else {
            // The incoming animation owns the "clear guard" step.
            lv_anim_set_user_data(&a, this);
            lv_anim_set_completed_cb(&a, [](lv_anim_t* an) {
                auto* self = static_cast<HomeMenuApp*>(lv_anim_get_user_data(an));
                if (self) self->animating_ = false;
            });
        }
        lv_anim_start(&a);
    }

    std::unique_ptr<AppBase> make_for_cursor(int idx) const
    {
        switch (idx) {
            case 0:  return make_answer_book_app();
            case 1:  return make_coin_app();
            case 2:  return make_dice_app();
            case 3:  return make_random10_app();
            case 4:  return make_yesno_app();
            case 5:  return make_mbti_app();
            case 6:  return make_fortune_app();
            case 7:  return make_clock_app();
            case 8:  return make_muyu_app();
            case 9:  return make_ble_remote_app();
            case 11: return make_settings_app();
            default: return stub_app(kStubLabels[idx]);
        }
    }

    lv_obj_t* root_      = nullptr;
    lv_obj_t* current_   = nullptr;
    int       cursor_    = 0;
    bool      animating_ = false;
};

}  // namespace

std::unique_ptr<AppBase> make_home_menu()
{
    return std::make_unique<HomeMenuApp>();
}

}  // namespace pocket
