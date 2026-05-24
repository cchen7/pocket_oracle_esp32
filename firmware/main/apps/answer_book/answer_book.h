#pragma once

#include "../app_base.h"
#include <memory>

namespace pocket {
std::unique_ptr<AppBase> make_answer_book_app();
}  // namespace pocket
