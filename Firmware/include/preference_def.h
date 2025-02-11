#pragma once
#include "common.h"
#include "macro_def.h"

namespace mcompass {

namespace preference {
/**
 * @brief 初始化
 */
void init(Context *context);
/**
 * @brief 保存目标位置
 */
void saveSpawnLocation(Location location);
/**
 * @brief 获取目标位置
 */
void getSpawnLocation(Location &location);
/**
 * @brief 保存指针颜色
 */
void savePointerColor(PointerColor color);
/**
 * @brief 获取指针颜色
 */
void getPointerColor(PointerColor &color);
/**
 * @brief 设置服务器模式
 */
void setServerMode(ServerMode serverMode);
/**
 * @brief 获取服务器模式
 */
void getServerMode(ServerMode &serverMode);

/**
 * @brief 设置LED亮度
 */
void setBrightness(uint8_t setBrightness);

/**
 * @brief 获取当前LED亮度
 */
void getBrightness(uint8_t &setBrightness);

/**
 * @brief 设置Wi-Fi SSID和密码
 */
void setWiFiCredentials(String ssid, String password);

/**
 * @brief 获取当前的Wi-Fi SSID和密码
 */
void getWiFiCredentials(String &ssid, String &password);

/**
 * @brief 设置自定义设备型号
 */
void setCustomDeviceModel(Model model);

/**
 * @brief 获取自定义设备型号
 */
void getCustomDeviceModel(Model &model);

/**
 * @brief 设置出厂设置
 */
void factoryReset();
}  // namespace preference
}  // namespace mcompass
