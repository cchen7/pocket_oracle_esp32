#pragma once

namespace pocket {

// Register the `bringup *` family of console commands used in P1 to exercise
// each on-board peripheral (LCD/buttons/IMU/speaker/power/RTC) from the host.
// Compile out with -DPOCKET_DISABLE_BRINGUP for production builds.
void register_bringup_commands();

}  // namespace pocket
