// Persistent status bar — see status_bar.h.
//
// Pinned to the top of the LVGL active screen at z-order above app roots,
// so it survives app_router teardown. A 2 s lv_timer refreshes battery
// percentage (via M5.Power) and uptime (formatted from FreeRTOS ticks).

#include "status_bar.h"

#include "theme.h"
#include "../lvgl_port/lvgl_init.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <M5Unified.h>

#include <cstdio>
#include <ctime>

namespace pocket {

namespace {

lv_obj_t* s_bar     = nullptr;
lv_obj_t* s_label_l = nullptr;
lv_obj_t* s_label_r = nullptr;

void apply_theme();

void refresh(lv_timer_t* t);

void apply_theme()
{
    if (!s_bar) return;
    lv_obj_set_style_bg_color(s_bar, theme::bg_primary(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_bar, LV_OPA_COVER, LV_PART_MAIN);
    if (s_label_l) {
        lv_obj_set_style_text_color(s_label_l, theme::ink_secondary(),
                                    LV_PART_MAIN);
        lv_obj_set_style_text_font(s_label_l, theme::font_caption(),
                                   LV_PART_MAIN);
    }
    if (s_label_r) {
        lv_obj_set_style_text_color(s_label_r, theme::ink_secondary(),
                                    LV_PART_MAIN);
        lv_obj_set_style_text_font(s_label_r, theme::font_caption(),
                                   LV_PART_MAIN);
    }
    // Force immediate redraw — LVGL on lv_layer_top doesn't always
    // pick up style changes on the next frame without an explicit hint.
    lv_obj_invalidate(s_bar);
}

void refresh(lv_timer_t* /*t*/)
{
    if (!s_label_l || !s_label_r) return;

    // Pick up any theme change since last tick (acts as a safety net
    // for callers that change theme without calling apply_theme()).
    apply_theme();

    // Show wall-clock HH:MM once SNTP has set the system clock; fall
    // back to uptime mm:ss until then so a fresh boot (or offline use)
    // still shows something.
    char left[16];
    const time_t now = time(nullptr);
    // time() returns ~0 until SNTP fires. Once synced it's well past
    // 2001 (epoch 1e9 = Sep 2001), use that as the "is real" cutoff.
    if (now > 1000000000) {
        struct tm lt;
        localtime_r(&now, &lt);
        std::snprintf(left, sizeof(left), "%02d:%02d", lt.tm_hour, lt.tm_min);
    } else {
        const uint32_t up_s = xTaskGetTickCount() * portTICK_PERIOD_MS / 1000;
        std::snprintf(left, sizeof(left), "%02u:%02u",
                      static_cast<unsigned>((up_s / 60) % 100),
                      static_cast<unsigned>(up_s % 60));
    }
    lv_label_set_text(s_label_l, left);

    const int pct = M5.Power.getBatteryLevel();
    char right[16];
    std::snprintf(right, sizeof(right), "%d%%", pct);
    lv_label_set_text(s_label_r, right);

    const lv_color_t c = (pct >= 25) ? theme::ink_secondary() : theme::accent_warn();
    lv_obj_set_style_text_color(s_label_r, c, LV_PART_MAIN);
}

}  // namespace

void status_bar_init()
{
    lvgl_lock();

    s_bar = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(s_bar);
    lv_obj_set_size(s_bar, theme::SCREEN_W, theme::STATUS_BAR_H);
    lv_obj_set_pos(s_bar, 0, 0);
    lv_obj_set_style_bg_color(s_bar, theme::bg_primary(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_pad_left(s_bar, theme::SPACE_S, LV_PART_MAIN);
    lv_obj_set_style_pad_right(s_bar, theme::SPACE_S, LV_PART_MAIN);

    s_label_l = lv_label_create(s_bar);
    lv_label_set_text(s_label_l, "--:--");
    lv_obj_set_style_text_color(s_label_l, theme::ink_secondary(), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_label_l, theme::font_caption(), LV_PART_MAIN);
    lv_obj_align(s_label_l, LV_ALIGN_LEFT_MID, 0, 0);

    s_label_r = lv_label_create(s_bar);
    lv_label_set_text(s_label_r, "--%");
    lv_obj_set_style_text_color(s_label_r, theme::ink_secondary(), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_label_r, theme::font_caption(), LV_PART_MAIN);
    lv_obj_align(s_label_r, LV_ALIGN_RIGHT_MID, 0, 0);

    lv_timer_create(refresh, 2000, nullptr);
    refresh(nullptr);  // first paint immediately

    lvgl_unlock();
}

void status_bar_apply_theme()
{
    lvgl_lock();
    apply_theme();
    lvgl_unlock();
}

}  // namespace pocket
