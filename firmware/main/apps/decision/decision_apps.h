#pragma once

#include "../app_base.h"
#include <memory>

namespace pocket {

std::unique_ptr<AppBase> make_coin_app();
std::unique_ptr<AppBase> make_dice_app();
std::unique_ptr<AppBase> make_random10_app();
std::unique_ptr<AppBase> make_yesno_app();

}  // namespace pocket
