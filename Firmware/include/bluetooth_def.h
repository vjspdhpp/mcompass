#pragma once

#include "common.h"
#include "macro_def.h"

namespace mcompass {
namespace ble_server {
/**
 * @brief 蓝牙初始化
 */
void init(Context *context);
/**
 * @brief 关闭蓝牙
 */
void deinit(Context *context);
}  // namespace ble_server
}  // namespace mcompass