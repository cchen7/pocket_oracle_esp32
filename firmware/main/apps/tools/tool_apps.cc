// Clock + Muyu (electronic wooden fish).
//
// Clock: displays HH:MM in display font with date+weekday under it. Pulls
// time from gettimeofday(); shows "--:--" placeholder until SNTP populates
// it (P7 will hook up WiFi + SNTP for real).
//
// Muyu: tap or shake = +1 merit + short tonal click. Cumulative count
// persists across reboots via storage/stats with key "muyu_total".

#include "tool_apps.h"

#include "../../storage/stats.h"
#include "../../ui/theme.h"

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
            lv_label_set_text(date_label_, "no time sync");
            return;
        }
        struct tm lt;
        localtime_r(&now, &lt);

        char hhmm[8];
        std::snprintf(hhmm, sizeof(hhmm), "%02d:%02d",
                      lt.tm_hour, lt.tm_min);
        lv_label_set_text(time_label_, hhmm);

        static const char* kDow[] = {
            "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
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

        // Speaker: bump volume so the wooden-fish click is audible above
        // the speaker's own bias hum. M5Unified defaults to ~64; the
        // amplifier idle is noisy at that level so user perception of a
        // short tone is poor.
        M5.Speaker.setVolume(160);

        // Wooden-fish glyph stand-in: a filled oval. Replace with a real
        // sprite once the icon set lands.
        fish_ = lv_obj_create(root);
        lv_obj_remove_style_all(fish_);
        lv_obj_set_size(fish_, 56, 36);
        lv_obj_set_style_bg_color(fish_, theme::accent_main(), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(fish_, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_radius(fish_, 18, LV_PART_MAIN);
        lv_obj_align(fish_, LV_ALIGN_CENTER, 0, 8);

        count_label_ = lv_label_create(root);
        lv_obj_set_style_text_color(count_label_, theme::accent_main(), LV_PART_MAIN);
        lv_obj_set_style_text_font(count_label_, theme::font_title(), LV_PART_MAIN);
        lv_obj_align(count_label_, LV_ALIGN_TOP_MID, 0,
                     theme::CONTENT_TOP + 2);

        merit_label_ = lv_label_create(root);
        lv_label_set_text(merit_label_, "merit");
        lv_obj_set_style_text_color(merit_label_, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(merit_label_, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(merit_label_, LV_ALIGN_TOP_MID, 0,
                     theme::CONTENT_TOP + 32);

        hint_ = lv_label_create(root);
        lv_label_set_text(hint_, "tap or shake \xc2\xb7 hold back");
        lv_obj_set_style_text_color(hint_, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(hint_, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(hint_, LV_ALIGN_BOTTOM_MID, 0, -theme::SPACE_XS);

        count_ = stats::get("muyu_total");
        refresh_count();
    }

    void on_exit() override
    {
        stats::set("muyu_total", count_);
    }

    void on_event(InputEvent ev) override
    {
        if (ev != InputEvent::kButtonShortPress && ev != InputEvent::kShake) {
            return;
        }
        ++count_;
        refresh_count();
        animate_strike();
        // Short 800 Hz click — wooden-fish placeholder until we ship PCM
        // samples in P9. Falls back silently if the speaker is muted.
        // 30 ms wasn't enough to overcome the amp turn-on; 120 ms is the
        // shortest a hand-test reliably heard above ambient.
        M5.Speaker.tone(800.0f, 120);

        // Commit on every 5th strike so we don't thrash flash but also
        // don't lose more than a handful on power loss.
        if ((count_ % 5) == 0) {
            stats::set("muyu_total", count_);
        }
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

}  // namespace pocket
