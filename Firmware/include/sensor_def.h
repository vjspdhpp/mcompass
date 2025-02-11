#pragma once
#include "common.h"
#include "macro_def.h"

namespace mcompass {
namespace sensor {
/**
 * @brief 校准罗盘
 */
void calibrateCompass();

/**
 * @brief 获取当前方位角
 */
int getAzimuth();

/**
 * @brief comass可用
 */
bool isCompassAvailable();

/**
 * @brief QMC5883初始化
 */
void init(Context *context);

}  // namespace sensor

}  // namespace mcompass
