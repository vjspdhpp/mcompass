#pragma once

#include "common.h"

/// 函数声明
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
 * @brief 保存指针颜色
 */
void getNeedleColor(NeedleColor &color);

/**
 * @brief 计算方位角
 * @param latA 目标位置纬度
 * @param lonA 目标位置经度
 * @param latB 当前位置纬度
 * @param lonB 当前位置经度
 * @return 方位角
 */
// float calculateBearing(float latA, float lonA, float latB, float lonB);
double calculateBearing(double lat1, double lon1, double lat2, double lon2);
double complexDistance(double lat1, double lon1, double lat2, double lon2);

double simplifiedDistance(double lat1, double lon1, double lat2, double lon2);

/**
 * @brief 校准罗盘
 */
void calibrateCompass();
/**
 * @brief 显示任务
 */
void displayTask(void *pvParameters);
/**
 * @brief 位置任务
 */
void locationTask(void *pvParameters);
/**
 * @brief 按钮任务
 */
void buttonTask(void *pvParameters);
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
void showFrame(int index, int overrideColor = 0xff1414);
/**
 * @brief 根据方位角显示帧
 * @param azimuth 方位角 范围应当是0~360
 */
void showFrameByAzimuth(float azimuth);
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
 * @brief 初始化服务器相关
 */
void setupServer();

/**
 * @brief 关闭本地网页服务
 */
void endWebServer();

/**
 * @brief 是否可以关闭网页服务
 */
bool shouldStopServer();

/**
 * @brief 初始化蓝牙
 */
void initBleServer();
void ble_loop();
/**
 * @brief 开启热点
 */
void localHotspot(const char *ssid = "The Lost Compass");

/**
 * @brief 关闭热点
 */
void stopHotspot();