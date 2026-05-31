// Clock + Muyu (electronic wooden fish).
//
// Clock: displays HH:MM in display font with date+weekday under it. Pulls
// time from gettimeofday(); shows "--:--" placeholder until SNTP populates
// it (P7 will hook up WiFi + SNTP for real).
//
// Muyu: tap or shake = +1 merit + short tonal click. Cumulative count
// persists across reboots via storage/stats with key "muyu_total".

#include "tool_apps.h"

#include "../../audio/muyu_pcm.h"
#include "../../ui/theme.h"

#include "esp_log.h"
#include "lvgl.h"

#include <M5Unified.h>

#include <cstdio>
#include <ctime>

namespace pocket {

namespace {

// ----------- Clock -----------

class ClockApp final : public AppBase {
public:
    const char* name() const override { return "Clock"; }

    void on_enter(lv_obj_t* root) override
    {
        root_ = root;
        lv_obj_set_style_bg_color(root, theme::bg_primary(), LV_PART_MAIN);

        time_label_ = lv_label_create(root);
        lv_obj_set_style_text_color(time_label_, theme::ink_primary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(time_label_, theme::font_display(), LV_PART_MAIN);
        lv_obj_align(time_label_, LV_ALIGN_CENTER, 0, -8);

        date_label_ = lv_label_create(root);
        lv_obj_set_style_text_color(date_label_, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(date_label_, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(date_label_, LV_ALIGN_BOTTOM_MID, 0, -theme::SPACE_S);

        refresh();
        timer_ = lv_timer_create(&ClockApp::tick_thunk, 1000, this);
    }

    void on_exit() override
    {
        if (timer_) {
            lv_timer_delete(timer_);
            timer_ = nullptr;
        }
    }

    void on_event(InputEvent /*ev*/) override {}

private:
    static void tick_thunk(lv_timer_t* t)
    {
        static_cast<ClockApp*>(lv_timer_get_user_data(t))->refresh();
    }

    void refresh()
    {
        time_t now = time(nullptr);
        if (now <= 0) {
            lv_label_set_text(time_label_, "--:--");
            lv_label_set_text(date_label_, "等待同步");
            return;
        }
        struct tm lt;
        localtime_r(&now, &lt);

        char hhmm[8];
        std::snprintf(hhmm, sizeof(hhmm), "%02d:%02d",
                      lt.tm_hour, lt.tm_min);
        lv_label_set_text(time_label_, hhmm);

        static const char* kDow[] = {
            "周日", "周一", "周二", "周三", "周四", "周五", "周六"
        };
        char date[32];
        std::snprintf(date, sizeof(date), "%s  %04d-%02d-%02d",
                      kDow[lt.tm_wday], lt.tm_year + 1900,
                      lt.tm_mon + 1, lt.tm_mday);
        lv_label_set_text(date_label_, date);
    }

    lv_obj_t*    root_       = nullptr;
    lv_obj_t*    time_label_ = nullptr;
    lv_obj_t*    date_label_ = nullptr;
    lv_timer_t*  timer_      = nullptr;
};

// ----------- Muyu (electronic wooden fish) -----------

class MuyuApp final : public AppBase {
public:
    const char* name() const override { return "Muyu"; }

    void on_enter(lv_obj_t* root) override
    {
        lv_obj_set_style_bg_color(root, theme::bg_primary(), LV_PART_MAIN);

        // M5.begin() registers the StickS3 spk_enable callback. Calling
        // Speaker.begin() here fires the callback with enabled=true,
        // which powers PMIC bit 0x11.3 + writes the ES8311 init
        // bulk-data. setVolume(200) instead of 255 to keep the AW8737
        // amp gain a bit lower (the noise floor scales with amp gain).
        M5.Speaker.begin();
        M5.Speaker.setVolume(200);
        ESP_LOGI("APP_MUYU",
                 "speaker enabled=%d volume=%d",
                 M5.Speaker.isEnabled() ? 1 : 0,
                 (int)M5.Speaker.getVolume());

        // Wooden-fish glyph stand-in: a filled oval. Replace with a real
        // sprite once the icon set lands.
        fish_ = lv_obj_create(root);
        lv_obj_remove_style_all(fish_);
        lv_obj_set_size(fish_, 56, 36);
        lv_obj_set_style_bg_color(fish_,
                                  theme::ink_color(theme::ink::ZHUSHA),
                                  LV_PART_MAIN);
        lv_obj_set_style_bg_opa(fish_, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_radius(fish_, 18, LV_PART_MAIN);
        lv_obj_align(fish_, LV_ALIGN_CENTER, 0, 8);

        count_label_ = lv_label_create(root);
        lv_obj_set_style_text_color(count_label_,
                                    theme::ink_color(theme::ink::ZHUSHA),
                                    LV_PART_MAIN);
        lv_obj_set_style_text_font(count_label_, theme::font_display_themed(),
                                   LV_PART_MAIN);
        lv_obj_align(count_label_, LV_ALIGN_TOP_MID, 0,
                     theme::CONTENT_TOP + 2);

        merit_label_ = lv_label_create(root);
        lv_label_set_text(merit_label_, "功德");
        lv_obj_set_style_text_color(merit_label_, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(merit_label_, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(merit_label_, LV_ALIGN_TOP_MID, 0,
                     theme::CONTENT_TOP + 52);

        hint_ = lv_label_create(root);
        lv_label_set_text(hint_, "短按或摇  长按返回");
        lv_obj_set_style_text_color(hint_, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(hint_, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(hint_, LV_ALIGN_BOTTOM_MID, 0, -theme::SPACE_XS);

        // Per-session counter — resets to 0 each time the app is
        // entered. Cumulative-merit-across-sessions felt less satisfying
        // than the ritual of "this sitting".
        count_ = 0;
        refresh_count();
    }

    void on_exit() override
    {
        // No persistence — per-session counter (see on_enter comment).
        // Power down the codec rail so it stops humming when we go
        // back to home or any other app.
        M5.Speaker.end();
    }

    void on_event(InputEvent ev) override
    {
        if (ev != InputEvent::kButtonShortPress
            && ev != InputEvent::kButtonBShortPress
            && ev != InputEvent::kShake) {
            return;
        }
        ++count_;
        refresh_count();
        animate_strike();
        // Wooden-fish PCM: 16 kHz mono 16-bit, 120 ms, synthesized by
        // tools/gen_muyu_pcm.py. Replaces the older tone() placeholder
        // that sounded like a phone dial. stop_current_sound=true so
        // rapid strikes don't pile up in the queue.
        M5.Speaker.playRaw(audio::kMuyuPcm, audio::kMuyuPcmLen,
                           audio::kMuyuPcmSampleRate,
                           /*stereo=*/false, /*repeat=*/1,
                           /*channel=*/-1, /*stop_current=*/true);
    }

private:
    void refresh_count()
    {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%lu",
                      static_cast<unsigned long>(count_));
        lv_label_set_text(count_label_, buf);
    }

    void animate_strike()
    {
        if (!fish_) return;
        // Brief scale via opacity dip — quick visible feedback without
        // pulling in LVGL transform animations.
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, fish_);
        lv_anim_set_values(&a, 120, 255);
        lv_anim_set_time(&a, 140);
        lv_anim_set_exec_cb(&a, [](void* obj, int32_t v) {
            lv_obj_set_style_bg_opa(static_cast<lv_obj_t*>(obj),
                                    (lv_opa_t)v, LV_PART_MAIN);
        });
        lv_anim_start(&a);
    }

    lv_obj_t* fish_        = nullptr;
    lv_obj_t* count_label_ = nullptr;
    lv_obj_t* merit_label_ = nullptr;
    lv_obj_t* hint_        = nullptr;
    uint32_t  count_       = 0;
};

}  // namespace

std::unique_ptr<AppBase> make_clock_app() { return std::make_unique<ClockApp>(); }
std::unique_ptr<AppBase> make_muyu_app()  { return std::make_unique<MuyuApp>(); }

// ----------- Battery -----------

class BatteryApp final : public AppBase {
public:
    const char* name() const override { return "Battery"; }

    void on_enter(lv_obj_t* root) override
    {
        lv_obj_set_style_bg_color(root, theme::bg_primary(), LV_PART_MAIN);

        lv_obj_t* title = lv_label_create(root);
        lv_obj_set_style_text_font(title, theme::font_title_themed(), LV_PART_MAIN);
        lv_obj_set_style_text_color(title, theme::ink_color(theme::ink::YUANMO),
                                    LV_PART_MAIN);
        lv_label_set_text(title, "电量");
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, theme::CONTENT_TOP + 2);
        lv_obj_invalidate(title);

        pct_label_ = lv_label_create(root);
        lv_obj_set_style_text_color(pct_label_,
                                    theme::ink_color(theme::ink::YUANMO),
                                    LV_PART_MAIN);
        lv_obj_set_style_text_font(pct_label_, theme::font_display(), LV_PART_MAIN);
        lv_obj_align(pct_label_, LV_ALIGN_CENTER, 0, 4);

        sub_label_ = lv_label_create(root);
        lv_obj_set_style_text_color(sub_label_, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(sub_label_, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(sub_label_, LV_ALIGN_BOTTOM_MID, 0, -16);

        lv_obj_t* hint = lv_label_create(root);
        lv_label_set_text(hint, "长按返回");
        lv_obj_set_style_text_color(hint, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(hint, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -2);

        refresh();
        timer_ = lv_timer_create(&BatteryApp::tick_thunk, 1000, this);
    }

    void on_exit() override
    {
        if (timer_) { lv_timer_delete(timer_); timer_ = nullptr; }
    }

    void on_event(InputEvent /*ev*/) override {}

private:
    static void tick_thunk(lv_timer_t* t)
    {
        static_cast<BatteryApp*>(lv_timer_get_user_data(t))->refresh();
    }

    void refresh()
    {
        const int pct = M5.Power.getBatteryLevel();
        const int mv  = M5.Power.getBatteryVoltage();
        const bool charging = M5.Power.isCharging();

        char buf[16];
        std::snprintf(buf, sizeof(buf), "%d%%", pct);
        lv_label_set_text(pct_label_, buf);
        // Color cue: 朱砂 (warning) below 25%, otherwise 远墨 (primary).
        lv_obj_set_style_text_color(pct_label_,
            (pct >= 25) ? theme::ink_color(theme::ink::YUANMO)
                        : theme::ink_color(theme::ink::ZHUSHA),
            LV_PART_MAIN);

        char sub[40];
        std::snprintf(sub, sizeof(sub), "%s  %d.%02dV",
                      charging ? "充电中" : "放电",
                      mv / 1000, (mv % 1000) / 10);
        lv_label_set_text(sub_label_, sub);
    }

    lv_obj_t*   pct_label_ = nullptr;
    lv_obj_t*   sub_label_ = nullptr;
    lv_timer_t* timer_     = nullptr;
};

std::unique_ptr<AppBase> make_battery_app() { return std::make_unique<BatteryApp>(); }

}  // namespace pocket
