#pragma once

#include "../app_base.h"

#include <memory>

namespace pocket {

// WiFi captive-portal setup screen. On enter: stops STA, brings up the
// portal AP + DNS hijack + HTTP form. Polls portal state. On success
// (credentials saved): stops the portal, restarts STA against the new
// creds, and pops back to the previous screen.
std::unique_ptr<AppBase> make_wifi_setup_app();

}  // namespace pocket
