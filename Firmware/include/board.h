#pragma once

#include <Arduino.h>

#include "bluetooth_def.h"
#include "button_def.h"
#include "common.h"
#include "gps_def.h"
#include "macro_def.h"
#include "pixel_def.h"
#include "preference_def.h"
#include "sensor_def.h"
#include "utils.h"
#include "web_server_def.h"

namespace mcompass {
namespace board {
/**
 * @brief 板子硬件初始化
 */
void init();
}  // namespace board
}  // namespace mcompass
