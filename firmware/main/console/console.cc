// USB-CDC service console.
// Minimal P0 implementation: help, version, reboot.
// Later phases will add status, bat, heap, tasks, log, brightness, ble, wifi, nvs, app go, poweroff.

#include "console.h"
#include "cmd_bringup.h"

#include "esp_console.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_idf_version.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstdio>

namespace pocket {

namespace {

constexpr const char* TAG = "CONSOLE";
constexpr const char* PROMPT = "pocket> ";

int cmd_version(int /*argc*/, char** /*argv*/)
{
    esp_chip_info_t chip;
    esp_chip_info(&chip);
    printf("Pocket Oracle firmware\n");
    printf("  build       : %s %s\n", __DATE__, __TIME__);
    printf("  IDF version : %s\n", esp_get_idf_version());
    printf("  chip        : ESP32-S3 rev v%d.%d, %d cores\n",
           chip.revision / 100, chip.revision % 100, chip.cores);
    return 0;
}

int cmd_reboot(int /*argc*/, char** /*argv*/)
{
    printf("Rebooting...\n");
    fflush(stdout);
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_restart();
    return 0;  // unreachable
}

void register_commands()
{
    const esp_console_cmd_t version_cmd = {
        .command = "version",
        .help = "Show firmware version and chip info",
        .hint = nullptr,
        .func = &cmd_version,
        .argtable = nullptr,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&version_cmd));

    const esp_console_cmd_t reboot_cmd = {
        .command = "reboot",
        .help = "Restart the device",
        .hint = nullptr,
        .func = &cmd_reboot,
        .argtable = nullptr,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&reboot_cmd));

    // esp_console provides `help` automatically.
    esp_console_register_help_command();
}

}  // namespace

void console_start()
{
    esp_console_repl_t* repl = nullptr;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = PROMPT;
    repl_config.max_cmdline_length = 256;
    repl_config.task_stack_size = 4096;
    repl_config.task_priority = 2;

    esp_console_dev_usb_serial_jtag_config_t hw_config =
        ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(
        &hw_config, &repl_config, &repl));

    register_commands();
    register_bringup_commands();

    ESP_ERROR_CHECK(esp_console_start_repl(repl));
    ESP_LOGI(TAG, "USB-CDC console ready — type 'help' over USB-Serial-JTAG");
}

}  // namespace pocket
