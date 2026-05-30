idf_component_register(
    SRCS
        "main.cc"
        "console/console.cc"
        "console/cmd_bringup.cc"
        "lvgl_port/lvgl_init.cc"
        "ui/status_bar.cc"
        "ui/theme.cc"
        "assets/fonts/lxgw_wenkai_cjk_16.c"
        "app/input_manager.cc"
        "app/app_router.cc"
        "app/power_manager.cc"
        "util/rng.cc"
        "util/rand_daily.cc"
        "storage/stats.cc"
        "storage/settings.cc"
        "ble/ble_hid.cc"
        "wifi/wifi_sta.cc"
        "wifi/captive_portal.cc"
        "apps/home_menu/home_menu.cc"
        "apps/answer_book/answer_book.cc"
        "apps/decision/decision_apps.cc"
        "apps/ritual/ritual_apps.cc"
        "apps/tools/tool_apps.cc"
        "apps/ble_remote/ble_remote.cc"
        "apps/wifi_setup/wifi_setup_app.cc"
        "apps/settings/settings_app.cc"
    INCLUDE_DIRS
        "."
        "lvgl_alias"
    REQUIRES
        console
        esp_system
        esp_timer
        esp_app_format
        nvs_flash
        driver
        M5Unified
        lvgl__lvgl
        h2zero__esp-nimble-cpp
        esp_wifi
        esp_event
        esp_netif
        esp_http_server
        lwip
)

# When CONFIG_LV_CONF_SKIP=n, LVGL expects "lv_conf.h" on its include path.
# Add main/ to the LVGL component's includes so our project-local config wins,
# and define LV_CONF_INCLUDE_SIMPLE on the main component too (so every TU
# that includes <lvgl.h> sees the same config our LVGL build was compiled with).
idf_component_get_property(lvgl_lib lvgl__lvgl COMPONENT_LIB)
target_include_directories(${lvgl_lib} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(${lvgl_lib} PUBLIC "LV_CONF_INCLUDE_SIMPLE")
target_compile_definitions(${COMPONENT_LIB} PUBLIC "LV_CONF_INCLUDE_SIMPLE")
