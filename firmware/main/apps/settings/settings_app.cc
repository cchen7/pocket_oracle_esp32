// Settings sub-menu.
//
// V1 only ships WiFi + About; brightness/volume/dark-mode/MBTI/language
// items will land on top of storage/settings once their owning subsystems
// exist.

#include "settings_app.h"

#include "../../app/app_router.h"
#include "../../ui/theme.h"
#include "../../wifi/wifi_sta.h"
#include "../wifi_setup/wifi_setup_app.h"

#include "esp_app_desc.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "lvgl.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace pocket {

namespace {

struct Item {
    const char* label;
};

const Item kItems[] = {
    { "WiFi Setup" },
    { "About"      },
};
constexpr int kCount = sizeof(kItems) / sizeof(kItems[0]);

// ---------- About sub-screen ----------

class AboutApp final : public AppBase {
public:
    const char* name() const override { return "About"; }

    void on_enter(lv_obj_t* root) override
    {
        lv_obj_set_style_bg_color(root, theme::bg_primary(), LV_PART_MAIN);

        lv_obj_t* title = lv_label_create(root);
        lv_label_set_text(title, "About");
        lv_obj_set_style_text_color(title, theme::ink_primary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(title, theme::font_title(), LV_PART_MAIN);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, theme::CONTENT_TOP + 2);

        info_ = lv_label_create(root);
        lv_obj_set_style_text_color(info_, theme::ink_secondary(),
                                    LV_PART_MAIN);
        lv_obj_set_style_text_font(info_, theme::font_caption(), LV_PART_MAIN);
        lv_obj_set_width(info_, theme::SCREEN_W - 12);
        lv_obj_align(info_, LV_ALIGN_CENTER, 0, 8);
        lv_obj_set_style_text_align(info_, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);

        refresh();
        timer_ = lv_timer_create(&AboutApp::tick_thunk, 2000, this);
    }

    void on_exit() override
    {
        if (timer_) { lv_timer_delete(timer_); timer_ = nullptr; }
    }

private:
    static void tick_thunk(lv_timer_t* t)
    {
        static_cast<AboutApp*>(lv_timer_get_user_data(t))->refresh();
    }

    void refresh()
    {
        const esp_app_desc_t* desc = esp_app_get_description();
        const char* wifi_status = "off";
        switch (wifi::state()) {
            case wifi::StaState::kInactive:   wifi_status = "off";        break;
            case wifi::StaState::kNoCreds:    wifi_status = "no WiFi";    break;
            case wifi::StaState::kConnecting: wifi_status = "connecting"; break;
            case wifi::StaState::kConnected:  wifi_status = wifi::current_ssid(); break;
            case wifi::StaState::kFailed:     wifi_status = "failed";     break;
        }
        const uint32_t up_s   = xTaskGetTickCount() * portTICK_PERIOD_MS / 1000;
        const uint32_t free_k = heap_caps_get_free_size(MALLOC_CAP_DEFAULT) / 1024;

        char buf[160];
        snprintf(buf, sizeof(buf),
                 "fw: %s\nwifi: %s\nip: %s\nfree: %u KB\nuptime: %u s",
                 desc ? desc->version : "?",
                 wifi_status,
                 wifi::current_ip(),
                 static_cast<unsigned>(free_k),
                 static_cast<unsigned>(up_s));
        lv_label_set_text(info_, buf);
    }

    lv_obj_t*   info_  = nullptr;
    lv_timer_t* timer_ = nullptr;
};

std::unique_ptr<AppBase> make_about_app()
{
    return std::make_unique<AboutApp>();
}

// ---------- Settings root ----------

class SettingsApp final : public AppBase {
public:
    const char* name() const override { return "Settings"; }

    void on_enter(lv_obj_t* root) override
    {
        lv_obj_set_style_bg_color(root, theme::bg_primary(), LV_PART_MAIN);

        lv_obj_t* title = lv_label_create(root);
        lv_label_set_text(title, "Settings");
        lv_obj_set_style_text_color(title, theme::ink_primary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(title, theme::font_title(), LV_PART_MAIN);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, theme::CONTENT_TOP + 2);

        for (int i = 0; i < kCount; ++i) {
            lv_obj_t* row = lv_obj_create(root);
            lv_obj_remove_style_all(row);
            lv_obj_set_size(row, theme::SCREEN_W - 24, 26);
            lv_obj_align(row, LV_ALIGN_CENTER, 0, -10 + i * 30);
            lv_obj_set_style_radius(row, 6, LV_PART_MAIN);
            lv_obj_set_style_bg_color(row, theme::bg_elevated(), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(row, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_border_width(row, 2, LV_PART_MAIN);
            lv_obj_set_style_border_color(row, theme::bg_elevated(),
                                          LV_PART_MAIN);

            lv_obj_t* lbl = lv_label_create(row);
            lv_label_set_text(lbl, kItems[i].label);
            lv_obj_set_style_text_color(lbl, theme::ink_primary(), LV_PART_MAIN);
            lv_obj_set_style_text_font(lbl, theme::font_body(), LV_PART_MAIN);
            lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 8, 0);

            rows_[i] = row;
        }

        lv_obj_t* hint = lv_label_create(root);
        lv_label_set_text(hint, "A=next  B=enter  hold=back");
        lv_obj_set_style_text_color(hint, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(hint, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -2);

        repaint();
    }

    void on_event(InputEvent ev) override
    {
        switch (ev) {
            case InputEvent::kShake:
            case InputEvent::kButtonShortPress:
                cursor_ = (cursor_ + 1) % kCount;
                repaint();
                break;
            case InputEvent::kButtonBShortPress:
                if (cursor_ == 0) {
                    app_router_push(make_wifi_setup_app());
                } else if (cursor_ == 1) {
                    app_router_push(make_about_app());
                }
                break;
            default: break;
        }
    }

private:
    void repaint()
    {
        for (int i = 0; i < kCount; ++i) {
            lv_color_t border = (i == cursor_) ? theme::accent_main()
                                               : theme::bg_elevated();
            lv_obj_set_style_border_color(rows_[i], border, LV_PART_MAIN);
        }
    }

    lv_obj_t* rows_[kCount] = {nullptr};
    int       cursor_        = 0;
};

}  // namespace

std::unique_ptr<AppBase> make_settings_app()
{
    return std::make_unique<SettingsApp>();
}

}  // namespace pocket
