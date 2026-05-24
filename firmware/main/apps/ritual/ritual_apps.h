#pragma once

#include "../app_base.h"
#include <memory>

namespace pocket {

std::unique_ptr<AppBase> make_mbti_app();
std::unique_ptr<AppBase> make_fortune_app();

}  // namespace pocket
