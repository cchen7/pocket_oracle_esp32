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
        case ble::ConnState::kInactive:    return "off";
        case ble::ConnState::kAdvertising: return "pairing...";
        case ble::ConnState::kConnected:   return "connected";
    }
    return "?";
}

lv_color_t status_color(ble::ConnState s)
{
    switch (s) {
        case ble::ConnState::kConnected:   return theme::accent_main();
        case ble::ConnState::kAdvertising: return theme::accent_calm();
        case ble::ConnState::kInactive:    return theme::accent_warn();
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
        lv_label_set_text(title, "BLE Remote");
        lv_obj_set_style_text_color(title, theme::ink_primary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(title, theme::font_title(), LV_PART_MAIN);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, theme::CONTENT_TOP + 2);

        // Status line (live-updates).
        status_label_ = lv_label_create(root);
        lv_obj_set_style_text_color(status_label_, theme::accent_calm(),
                                    LV_PART_MAIN);
        lv_obj_set_style_text_font(status_label_, theme::font_body(),
                                   LV_PART_MAIN);
        lv_obj_align(status_label_, LV_ALIGN_CENTER, 0, 4);

        // Last-action feedback line.
        action_label_ = lv_label_create(root);
        lv_label_set_text(action_label_, "");
        lv_obj_set_style_text_color(action_label_, theme::ink_secondary(),
                                    LV_PART_MAIN);
        lv_obj_set_style_text_font(action_label_, theme::font_caption(),
                                   LV_PART_MAIN);
        lv_obj_align(action_label_, LV_ALIGN_CENTER, 0, 26);

        // Bottom hint.
        lv_obj_t* hint = lv_label_create(root);
        lv_label_set_text(hint, "tap=PgDn  shake=PgUp  hold=back");
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
        if (ev == InputEvent::kButtonShortPress) {
            ble::send_keyboard(ble::key::kPageDown);
            set_action("PgDn");
        } else if (ev == InputEvent::kShake) {
            ble::send_keyboard(ble::key::kPageUp);
            set_action("PgUp");
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
            lv_label_set_text_fmt(action_label_, "sent %s", what);
            lv_obj_set_style_text_color(action_label_, theme::ink_secondary(),
                                        LV_PART_MAIN);
        } else {
            lv_label_set_text_fmt(action_label_, "%s (not paired)", what);
            lv_obj_set_style_text_color(action_label_, theme::accent_warn(),
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
