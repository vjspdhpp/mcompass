#pragma once

#include <Arduino.h>

#include "common.h"
#include "macro_def.h"

namespace Board {
/**
 * @brief 板子硬件初始化
 */
Context *init();
void calibrateCheck();
/**
 * @brief 按钮任务
 */
void buttonTask(void *pvParameters);
}  // namespace Board

namespace Preference {
/**
 * @brief 保存目标位置
 */
void saveHomeLocation(Location location);
/**
 * @brief 获取目标位置
 */
void getHomeLocation(Location &location);
/**
 * @brief 保存指针颜色
 */
void saveNeedleColor(NeedleColor color);
/**
 * @brief 获取指针颜色
 */
void getNeedleColor(NeedleColor &color);
/**
 * @brief 网页服务开关
 * @param useWiFi 使用网页进行配置
 */
void setWebServerConfig(bool useWiFi);
/**
 * @brief 获取网页服务开关
 */
void getWebServerConfig(bool &useWiFi);
}  // namespace Preference

namespace Compass {
/**
 * @brief 计算方位角
 * @param latA 目标位置纬度
 * @param lonA 目标位置经度
 * @param latB 当前位置纬度
 * @param lonB 当前位置经度
 * @return 方位角
 */
double calculateBearing(double lat1, double lon1, double lat2, double lon2);
double complexDistance(double lat1, double lon1, double lat2, double lon2);

double simplifiedDistance(double lat1, double lon1, double lat2, double lon2);

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

}  // namespace Compass

namespace GPS {
/**
 * @brief GPS初始化
 */
void init(Context *context);
/**
 * @brief GPS反初始化
 */
void deinit(Context *context);
/**
 * @brief 位置任务
 */
void locationTask(void *pvParameters);

}  // namespace GPS

namespace Pixel {
/**
 * @brief LED初始化
 */
void init(Context *context);
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
void showFrame(int index, int overrideColor = DEFAULT_NEEDLE_COLOR);
/**
 * @brief 根据方位角显示帧
 * @param azimuth 方位角 范围应当是0~360
 */
void showFrameByAzimuth(float azimuth,
                        int overrideColor = DEFAULT_NEEDLE_COLOR);
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
 * @brief 显示任务
 */
void pixelTask(void *pvParameters);
}  // namespace Pixel

namespace CompassServer {

/**
 * @brief Web初始化
 */
void init(Context *context);

/**
 * @brief 关闭本地网页服务
 */
void endWebServer();

/**
 * @brief 是否可以关闭网页服务
 */
bool shouldStopServer();

/**
 * @brief 开启热点
 */
void localHotspot(const char *ssid = "The Lost Compass");

/**
 * @brief 关闭热点
 */
void stopHotspot();
}  // namespace CompassServer

namespace CompassBLE {

/**
 * @brief 蓝牙初始化
 */
void init(Context *context);
void bleTask(void *pvParameters);
}  // namespace CompassBLE