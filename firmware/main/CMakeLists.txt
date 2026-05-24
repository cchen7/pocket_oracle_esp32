idf_component_register(
    SRCS
        "main.cc"
        "console/console.cc"
        "console/cmd_bringup.cc"
        "lvgl_port/lvgl_init.cc"
    INCLUDE_DIRS
        "."
        "lvgl_alias"
    REQUIRES
        console
        esp_system
        esp_timer
        nvs_flash
        driver
        M5Unified
        lvgl__lvgl
)

# When CONFIG_LV_CONF_SKIP=n, LVGL expects "lv_conf.h" on its include path.
# Add main/ to the LVGL component's includes so our project-local config wins,
# and define LV_CONF_INCLUDE_SIMPLE on the main component too (so every TU
# that includes <lvgl.h> sees the same config our LVGL build was compiled with).
idf_component_get_property(lvgl_lib lvgl__lvgl COMPONENT_LIB)
target_include_directories(${lvgl_lib} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(${lvgl_lib} PUBLIC "LV_CONF_INCLUDE_SIMPLE")
target_compile_definitions(${COMPONENT_LIB} PUBLIC "LV_CONF_INCLUDE_SIMPLE")
