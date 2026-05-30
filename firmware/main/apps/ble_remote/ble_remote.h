#pragma once

#include "../app_base.h"

#include <memory>

namespace pocket {

// Single-button BLE remote. V1 = PPT mode only:
//   tap   -> Page Down
//   shake -> Page Up
//   hold  -> exit (handled globally by app_router)
std::unique_ptr<AppBase> make_ble_remote_app();

}  // namespace pocket
