#pragma once

#include "../app_base.h"
#include <memory>

namespace pocket {

std::unique_ptr<AppBase> make_clock_app();
std::unique_ptr<AppBase> make_muyu_app();

}  // namespace pocket
