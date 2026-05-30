#pragma once

#include "../app_base.h"

#include <memory>

namespace pocket {

// Settings sub-menu: WiFi setup, About. BtnA scrolls, BtnB enters,
// long-press exits back to home.
std::unique_ptr<AppBase> make_settings_app();

}  // namespace pocket
