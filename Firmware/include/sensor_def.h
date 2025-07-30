#pragma once
#include "common.h"
#include "macro_def.h"

namespace mcompass {
namespace sensor {
/**
 * @brief 校准罗盘
 */
void calibrate();

/**
 * @brief 获取当前方位角
 */
int getAzimuth();

/**
 * @brief 传感器可用状态
 */
bool available();

/**
 * @brief QMC5883初始化
 */
void init(Context *context);

} // namespace sensor

} // namespace mcompass
