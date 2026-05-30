// app_router — owns the active app + dispatches input events.
// Holds the LVGL mutex around app lifecycle calls so app code can call
// lv_* freely without worrying about thread safety with lvgl_task.

#include "app_router.h"

#include "input_manager.h"
#include "../apps/app_base.h"
#include "../apps/home_menu/home_menu.h"
#include "../lvgl_port/lvgl_init.h"
#include "../ui/theme.h"

#include "esp_log.h"
#include "lvgl.h"

namespace pocket {

namespace {

constexpr const char* TAG = "ROUTER";

std::unique_ptr<AppBase> s_active;
lv_obj_t* s_root = nullptr;

void on_input_event(InputEvent ev, void* /*user*/)
{
    // Long-press on EITHER physical button = global back-to-home.
    // (In the home menu the names start with 'H' so this is a no-op
    // and falls through to on_event for the app to handle if it wants.)
    if (ev == InputEvent::kButtonLongPress || ev == InputEvent::kButtonBLongPress) {
        if (s_active && s_active->name()[0] != 'H') {
            app_router_go_home();
            return;
        }
    }
    if (s_active) {
        lvgl_lock();
        s_active->on_event(ev);
        lvgl_unlock();
    }
}

}  // namespace

void app_router_push(std::unique_ptr<AppBase> app)
{
    if (!app) return;

    lvgl_lock();

    if (s_active) {
        s_active->on_exit();
        s_active.reset();
    }

    // Rebuild a fresh root container; cheaper than diff'ing widgets.
    if (s_root) {
        lv_obj_delete(s_root);
        s_root = nullptr;
    }
    s_root = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(s_root);
    lv_obj_set_size(s_root, theme::SCREEN_W, theme::SCREEN_H);
    lv_obj_set_pos(s_root, 0, 0);
    lv_obj_set_style_bg_color(s_root, theme::bg_primary(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_root, LV_OPA_COVER, LV_PART_MAIN);

    s_active = std::move(app);
    ESP_LOGI(TAG, "entering %s", s_active->name());
    s_active->on_enter(s_root);

    lvgl_unlock();
}

void app_router_go_home()
{
    app_router_push(make_home_menu());
}

void app_router_init()
{
    // Paint base screen to our dark palette so transient gaps don't flash white.
    lvgl_lock();
    lv_obj_set_style_bg_color(lv_screen_active(), theme::bg_primary(), LV_PART_MAIN);
    lvgl_unlock();

    input_manager_set_listener(on_input_event, nullptr);
    app_router_go_home();
}

}  // namespace pocket
