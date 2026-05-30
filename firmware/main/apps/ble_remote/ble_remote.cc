// BLE Remote — single-button slide-deck remote.
//
// On enter: brings up the BLE HID stack and shows live pairing status.
// V1 maps:
//   tap   -> Page Down
//   shake -> Page Up
// On exit: tears the stack down to free the radio (P7 WiFi reuses it).

#include "ble_remote.h"

#include "../../ble/ble_hid.h"
#include "../../ble/hid_report_map.h"
#include "../../ui/theme.h"

#include "esp_log.h"
#include "lvgl.h"

namespace pocket {

namespace {

constexpr const char* TAG       = "APP_BLE";
constexpr const char* kDevName  = "Pocket Oracle";

const char* status_text(ble::ConnState s)
{
    switch (s) {
        case ble::ConnState::kInactive:    return "未启动";
        case ble::ConnState::kAdvertising: return "配对中";
        case ble::ConnState::kConnected:   return "已连接";
    }
    return "?";
}

lv_color_t status_color(ble::ConnState s)
{
    switch (s) {
        case ble::ConnState::kConnected:   return theme::ink_color(theme::ink::CANGCUI);
        case ble::ConnState::kAdvertising: return theme::ink_color(theme::ink::QINGMO);
        case ble::ConnState::kInactive:    return theme::ink_color(theme::ink::ZHUSHA);
    }
    return theme::ink_secondary();
}

class BleRemoteApp final : public AppBase {
public:
    const char* name() const override { return "BLE"; }

    void on_enter(lv_obj_t* root) override
    {
        lv_obj_set_style_bg_color(root, theme::bg_primary(), LV_PART_MAIN);

        // Title.
        lv_obj_t* title = lv_label_create(root);
        lv_obj_set_style_text_font(title, theme::font_title_themed(), LV_PART_MAIN);
        lv_obj_set_style_text_color(title, theme::ink_primary(), LV_PART_MAIN);
        lv_label_set_text(title, "蓝牙翻页");
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, theme::CONTENT_TOP + 2);
        lv_obj_invalidate(title);

        // Status line (live-updates).
        status_label_ = lv_label_create(root);
        lv_obj_set_style_text_color(status_label_,
                                    theme::ink_color(theme::ink::QINGMO),
                                    LV_PART_MAIN);
        lv_obj_set_style_text_font(status_label_, theme::font_body(),
                                   LV_PART_MAIN);
        lv_obj_align(status_label_, LV_ALIGN_CENTER, 0, 8);

        // Last-action feedback line.
        action_label_ = lv_label_create(root);
        lv_label_set_text(action_label_, "");
        lv_obj_set_style_text_color(action_label_, theme::ink_secondary(),
                                    LV_PART_MAIN);
        lv_obj_set_style_text_font(action_label_, theme::font_caption(),
                                   LV_PART_MAIN);
        lv_obj_align(action_label_, LV_ALIGN_CENTER, 0, 32);

        // Bottom hint.
        lv_obj_t* hint = lv_label_create(root);
        lv_label_set_text(hint, "短按下页  侧键上页  长按返回");
        lv_obj_set_style_text_color(hint, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(hint, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -2);

        if (!ble::init(kDevName)) {
            ESP_LOGE(TAG, "ble::init failed");
        }

        refresh_status();
        timer_ = lv_timer_create(&BleRemoteApp::tick_thunk, 500, this);
    }

    void on_event(InputEvent ev) override
    {
        // Arrow keys instead of PgDn/PgUp: macOS Keynote/PPT, iOS
        // Keynote/PPT, Reveal.js, Kindle and basically every slide-like
        // app accept Right/Left for next/prev. iOS apps in particular
        // ignore PgDn/PgUp — empirically verified, that was the reason
        // for the V1 mid-flight pivot.
        //
        // 2-button mapping (post P1.9 corrigendum): BtnA -> next,
        // BtnB -> prev, shake -> prev (BtnB alias for the wrist flick
        // crowd).
        if (ev == InputEvent::kButtonShortPress) {
            ble::send_keyboard(ble::key::kRightArrow);
            set_action("下页");
        } else if (ev == InputEvent::kButtonBShortPress
                   || ev == InputEvent::kShake) {
            ble::send_keyboard(ble::key::kLeftArrow);
            set_action("上页");
        }
    }

    void on_exit() override
    {
        if (timer_) {
            lv_timer_delete(timer_);
            timer_ = nullptr;
        }
        // Tear the stack down so the radio and ~50KB are free for whoever
        // owns the next screen (typically WiFi via Settings → WiFi setup).
        ble::deinit();
    }

private:
    static void tick_thunk(lv_timer_t* t)
    {
        static_cast<BleRemoteApp*>(lv_timer_get_user_data(t))->refresh_status();
    }

    void refresh_status()
    {
        const auto s = ble::state();
        lv_label_set_text(status_label_, status_text(s));
        lv_obj_set_style_text_color(status_label_, status_color(s),
                                    LV_PART_MAIN);
    }

    void set_action(const char* what)
    {
        if (!action_label_) return;
        if (ble::is_connected()) {
            lv_label_set_text_fmt(action_label_, "已发送 %s", what);
            lv_obj_set_style_text_color(action_label_, theme::ink_secondary(),
                                        LV_PART_MAIN);
        } else {
            lv_label_set_text_fmt(action_label_, "%s (未配对)", what);
            lv_obj_set_style_text_color(action_label_,
                                        theme::ink_color(theme::ink::ZHUSHA),
                                        LV_PART_MAIN);
        }
    }

    lv_obj_t*   status_label_ = nullptr;
    lv_obj_t*   action_label_ = nullptr;
    lv_timer_t* timer_        = nullptr;
};

}  // namespace

std::unique_ptr<AppBase> make_ble_remote_app()
{
    return std::make_unique<BleRemoteApp>();
}

}  // namespace pocket
