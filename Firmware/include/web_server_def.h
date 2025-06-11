#pragma once

#include "common.h"
#include "macro_def.h"

namespace mcompass {
namespace web_server {

/**
 * @brief Web初始化
 */
void init(Context *context);

/**
 * @brief 关闭本地网页服务
 */
void endServer();

/**
 * @brief 开启热点
 */
void createAccessPoint(const char *ssid = "The Lost Compass");

/**
 * @brief 关闭热点
 */
void endAccessPoint();
}  // namespace web_server
}  // namespace mcompass
