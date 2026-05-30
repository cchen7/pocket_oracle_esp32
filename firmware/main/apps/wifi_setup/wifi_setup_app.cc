// WiFi Setup — drives the captive-portal flow from the device side.
//
// Lifecycle:
//   enter      -> stop STA, start portal (AP + DNS + HTTP)
//   poll 1 Hz  -> watch portal state; on kCredsReceived stop portal +
//                 restart STA + pop back to settings
//   exit       -> portal_stop() if still running; sta_reconnect() so
//                 normal operation resumes regardless of whether the
//                 user submitted credentials or backed out

#include "wifi_setup_app.h"

#include "../../app/app_router.h"
#include "../../ui/theme.h"
#include "../../wifi/captive_portal.h"
#include "../../wifi/wifi_sta.h"

#include "esp_log.h"
#include "lvgl.h"

namespace pocket {

namespace {

constexpr const char* TAG = "APP_WIFI";

class WiFiSetupApp final : public AppBase {
public:
    const char* name() const override { return "WiFi"; }

    void on_enter(lv_obj_t* root) override
    {
        lv_obj_set_style_bg_color(root, theme::bg_primary(), LV_PART_MAIN);

        lv_obj_t* title = lv_label_create(root);
        lv_label_set_text(title, "WiFi Setup");
        lv_obj_set_style_text_color(title, theme::ink_primary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(title, theme::font_title(), LV_PART_MAIN);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, theme::CONTENT_TOP + 2);

        ap_label_ = lv_label_create(root);
        lv_obj_set_style_text_color(ap_label_, theme::accent_main(), LV_PART_MAIN);
        lv_obj_set_style_text_font(ap_label_, theme::font_body(), LV_PART_MAIN);
        lv_obj_align(ap_label_, LV_ALIGN_CENTER, 0, -10);

        status_label_ = lv_label_create(root);
        lv_obj_set_style_text_color(status_label_, theme::ink_secondary(),
                                    LV_PART_MAIN);
        lv_obj_set_style_text_font(status_label_, theme::font_caption(),
                                   LV_PART_MAIN);
        lv_obj_set_width(status_label_, theme::SCREEN_W - 16);
        lv_label_set_long_mode(status_label_, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_align(status_label_, LV_TEXT_ALIGN_CENTER,
                                    LV_PART_MAIN);
        lv_obj_align(status_label_, LV_ALIGN_CENTER, 0, 16);

        lv_obj_t* hint = lv_label_create(root);
        lv_label_set_text(hint, "hold A/B to cancel");
        lv_obj_set_style_text_color(hint, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(hint, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -2);

        // Tear down STA (if up) so we can switch to AP-only mode for
        // provisioning. Frees the radio + ~30 KB.
        wifi::sta_deinit();

        if (wifi::portal_start() != ESP_OK) {
            lv_label_set_text(ap_label_, "ERROR");
            lv_label_set_text(status_label_, "captive portal failed to start");
            return;
        }

        lv_label_set_text(ap_label_, wifi::portal_ap_ssid());
        lv_label_set_text(status_label_,
            "1) Join this WiFi from your phone\n"
            "2) A page should pop up to enter your home WiFi");

        timer_ = lv_timer_create(&WiFiSetupApp::tick_thunk, 800, this);
    }

    void on_exit() override
    {
        if (timer_) {
            lv_timer_delete(timer_);
            timer_ = nullptr;
        }
        wifi::portal_stop();
        // Whether the user submitted creds or just held to cancel, we
        // re-attempt STA. If creds were saved this time we'll connect;
        // if not, sta_init() lands in kNoCreds and stays idle.
        wifi::sta_init();
    }

private:
    static void tick_thunk(lv_timer_t* t)
    {
        static_cast<WiFiSetupApp*>(lv_timer_get_user_data(t))->poll();
    }

    void poll()
    {
        if (wifi::portal_state() != wifi::PortalState::kCredsReceived) return;
        if (handled_creds_) return;
        handled_creds_ = true;

        ESP_LOGI(TAG, "creds received, returning to settings");
        lv_label_set_text(status_label_,
                          "Saved. Reconnecting to your network...");
        // Pop back to home; on_exit() does the portal_stop + sta_init dance.
        // (We don't have a generic "pop to previous app" yet, so home is the
        // simplest landing — user can re-enter Settings to verify connection.)
        app_router_go_home();
    }

    lv_obj_t*   ap_label_     = nullptr;
    lv_obj_t*   status_label_ = nullptr;
    lv_timer_t* timer_        = nullptr;
    bool        handled_creds_ = false;
};

}  // namespace

std::unique_ptr<AppBase> make_wifi_setup_app()
{
    return std::make_unique<WiFiSetupApp>();
}

}  // namespace pocket
