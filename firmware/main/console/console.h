#pragma once

namespace pocket {

// Start the USB-CDC service console.
// Spawns a low-priority REPL task that reads commands from USB-Serial-JTAG
// and dispatches to handlers registered with esp_console.
void console_start();

}  // namespace pocket
