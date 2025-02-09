#pragma once

#include <Arduino.h>

#include "common.h"
#include "macro_def.h"

namespace board {
/**
 * @brief 板子硬件初始化
 */
mcompass::Context *init();
}  // namespace board

namespace preference {
/**
 * @brief 初始化
 */
void init(mcompass::Context *context);
/**
 * @brief 保存目标位置
 */
void saveHomeLocation(mcompass::Location location);
/**
 * @brief 获取目标位置
 */
void getHomeLocation(mcompass::Location &location);
/**
 * @brief 保存指针颜色
 */
void savePointerColor(mcompass::PointerColor color);
/**
 * @brief 获取指针颜色
 */
void getPointerColor(mcompass::PointerColor &color);
/**
 * @brief 设置服务器模式
 */
void setServerMode(mcompass::ServerMode serverMode);
/**
 * @brief 获取服务器模式
 */
void getServerMode(mcompass::ServerMode &serverMode);

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
void setCustomDeviceModel(mcompass::Model model);

/**
 * @brief 获取自定义设备型号
 */
void getCustomDeviceModel(mcompass::Model &model);

/**
 * @brief 设置出厂设置
 */
void factoryReset();
}  // namespace preference

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
void init(mcompass::Context *context);

}  // namespace mcompass

namespace gps {
/**
 * @brief GPS初始化
 */
void init(mcompass::Context *context);
/**
 * @brief GPS反初始化
 */
void deinit(mcompass::Context *context);

/**
 * @brief 校验GPS坐标是否有效
 *
 * @param location
 * @return true
 * @return false
 */
bool isValidGPSLocation(mcompass::Location location);
/**
 * @brief GPS 关闭
 */
void disable();
}  // namespace gps

namespace pixel {
/**
 * @brief LED初始化
 */
void init(mcompass::Context *context);
/**
 * @brief 启动动画
 */
void bootAnimation(void (*callbackFn)() = nullptr);

/**
 * @brief 丢失方位
 */
void lostBearing();
/**
 * @brief 地狱, 无GPS信号
 */
void theNether();
/**
 * @brief 显示帧
 * @param index 帧索引
 * @param overrideColor 重载颜色, 用来覆盖指针颜色, 默认红色指针
 */
void showFrame(int index);
/**
 * @brief 显示方位角
 * @param azimuth 方位角 范围应当是0~360
 */
void showByAzimuth(float azimuth);
/**
 * @brief 根据方位角显示帧
 * @param bearing 方位角
 * @param azimuth 当前罗盘方位角
 */
void showFrameByBearing(float bearing, int azimuth);
/**
 * @brief 根据位置显示帧
 * @param latitudeA 目标位置纬度
 * @param longitudeA 目标位置经度
 * @param latitudeB 当前位置纬度
 * @param longitudeB 当前位置经度
 * @param azimuth 当前罗盘方位角
 */
void showFrameByLocation(float latA, float lonA, float latB, float lonB,
                         int azimuth);
/**
 * @brief 热点
 */
void showHotspot();

/**
 * @brief 显示纯色
 * @param color 颜色
 */
void showSolid(int color);
/**
 * @brief 连接WiFi
 */
void showConnectingWifi();
/**
 * @brief 服务器颜色
 */
void showServerColors();
/**
 * @brief 服务器WiFi
 */
void showServerWifi();
/**
 * @brief 服务器生成
 */
void showServerSpawn();
/**
 * @brief 服务器信息
 */
void showServerInfo();
/**
 * @brief 绘制字符
 */
void drawChar(char c, int startX, int startY, uint32_t color);
/**
 * @brief 显示任务
 */
void pixelTask(void *pvParameters);

/**
 * @brief 设置亮度
 */
void setBrightness(uint8_t brightness);

/**
 * @brief 设置指针颜色
 */
void setPointerColor(uint32_t pointColor);

/**
 * @brief 倒计时
 */
void counterDown(int seconds);
}  // namespace pixel

namespace web_server {

/**
 * @brief Web初始化
 */
void init(mcompass::Context *context);

/**
 * @brief 关闭本地网页服务
 */
void endServer();

/**
 * @brief 是否可以关闭网页服务
 */
bool shouldStop();

/**
 * @brief 开启热点
 */
void startHotspot(const char *ssid = "The Lost Compass");

/**
 * @brief 关闭热点
 */
void stopHotspot();
}  // namespace web_server

namespace ble_server {

/**
 * @brief 蓝牙初始化
 */
void init(mcompass::Context *context);
/**
 * @brief 蓝牙task
 */
void bleTask(void *pvParameters);
/**
 * @brief 关闭蓝牙
 */
void disable();
}  // namespace ble_server