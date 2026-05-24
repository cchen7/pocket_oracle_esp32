#pragma once

#include "../app_base.h"

#include <memory>

namespace pocket {

// Home — 4 x 3 card grid of all 12 apps. KEY1 short = enter, shake = cursor
// right. Builds the same lightweight stub app for unfinished entries.
std::unique_ptr<AppBase> make_home_menu();

}  // namespace pocket
