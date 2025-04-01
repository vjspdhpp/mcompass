#pragma once
#include "common.h"
#include "macro_def.h"

namespace mcompass {
namespace gps {
/**
 * @brief GPS初始化
 */
void init(Context *context);
/**
 * @brief GPS反初始化
 */
void deinit(Context *context);

/**
 * @brief 校验GPS坐标是否有效
 *
 * @param location
 * @return true
 * @return false
 */
bool isValidGPSLocation(Location location);
/**
 * @brief GPS 关闭
 */
void disable();
}  // namespace gps
}  // namespace mcompass
