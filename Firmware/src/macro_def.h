#pragma once

///////////////////// 宏定义 ///////////////////////
#define NUM_LEDS 42
#define MAX_FRAME_INDEX 26

///////////////////// 引脚定义 ///////////////////////
#define DATA_PIN 6
#define CALIBRATE_PIN 9
#define GPS_EN_PIN 0

///////////////////// 默认值 ///////////////////////
#define EARTH_RADIUS 6371.0  // 地球半径（单位：公里）
// 指针颜色
#define DEFAULT_POINTER_COLOR 0xFF1414  // 指针红
// 默认亮度
#define DEFAULT_BRIGHTNESS 56

// 默认连接连接WiFi等待时间 10秒
#define DEFAULT_WIFI_CONNECT_TIME 10 * 1000
// 默认无client连接关闭web server时间 120个tick, 一个tick对应一秒钟
#define DEFAULT_SERVER_TICK_COUNT 120
// 默认检测不到GPS,关闭GPS供电时间
#define DEFAULT_GPS_DETECT_TIMEOUT 60 * 1000

// 默认初始的坐标值
#define DEFAULT_INVALID_LOCATION_VALUE 255.0f

// 默认服务器模式
#define DEFAULT_SERVER_MODE mcompass::ServerMode::WIFI

// 默认型号
#ifndef DEFAULT_MODEL
#define DEFAULT_MODEL mcompass::Model::GPS
#endif

///////////////////// 版本信息 ///////////////////////
#if !defined(BUILD_VERSION)
#define BUILD_VERSION "UNKNOWN"
#endif  // MACRO

#if !defined(GIT_BRANCH)
#define GIT_BRANCH "UNKNOWN"
#endif  // MACRO

#if !defined(GIT_COMMIT)
#define GIT_COMMIT "UNKNOWN"
#endif  // MACRO

#define INFO_JSON                                                         \
  "{\"buildDate\":\"" __DATE__ "\",\"buildTime\":\"" __TIME__             \
  "\",\"buildVersion\":\"" BUILD_VERSION "\",\"gitBranch\":\"" GIT_BRANCH \
  "\",\"gitCommit\":\"" GIT_COMMIT "\"}"

///////////////////// 蓝牙相关 ///////////////////////
/* 基础配置 */
#define BASE_SERVICE_UUID (uint16_t)0xf000
#define COLOR_CHARACTERISITC_UUID (uint16_t)(BASE_SERVICE_UUID + 1)  // 指针颜色
#define AZIMUTH_CHARACHERSITC_UUID (uint16_t)(BASE_SERVICE_UUID + 2)  // 方位角
#define SPAWN_CHARACTERISTIC_UUID \
  (uint16_t)(BASE_SERVICE_UUID + 3)  // 出生点信息
#define INFO_CHARACTERISTIC_UUID (uint16_t)(BASE_SERVICE_UUID + 4)  // 设备信息

/** 设备操作 */
#define CONTROL_SERVICE_UUID (uint16_t)0xf100
#define CALIBRATE_CHARACTERISTIC_UUID \
  (uint16_t)(CONTROL_SERVICE_UUID + 1)  // 请求校准
#define REBOOT_CHARACTERISTIC_UUID \
  (uint16_t)(CONTROL_SERVICE_UUID + 2)  // 重启设备

/** 高级配置  */
#define ADVANCED_SERVICE_UUID (uint16_t)0xfa00
#define VIRTUAL_LOCATION_CHARACHTERISTIC_UUID \
  (uint16_t)(ADVANCED_SERVICE_UUID + 1)  // 虚拟坐标
#define VIRTUAL_AZIMUTH_CHARACHTERISTIC_UUID \
  (uint16_t)(ADVANCED_SERVICE_UUID + 2)  // 虚拟方位角
#define WEB_SERVER_CHARACHTERISTIC_UUID \
  (uint16_t)(ADVANCED_SERVICE_UUID + 3)  // 启用服务器API和网页服务
#define BRIGHTNESS_CHARACHTERISTIC_UUID \
  (uint16_t)(ADVANCED_SERVICE_UUID + 4)  // 亮度控制

///////////////////// 配置相关 ///////////////////////
#define PREFERENCE_NAME "mcompass"  // 配置文件名称

#define LATITUDE_KEY "latitude"        // 纬度Key
#define LONGTITUDE_KEY "longitude"     // 经度Key
#define SPAWN_COLOR_KEY "spawn_color"  // 出生针颜色
#define SOUTH_COLOR_KEY "south_color"  // 指南针颜色
#define SERVER_MODE_KEY "server_mode"  // 配置模式
#define WIFI_SSID_KEY "SSID"           // WiFi账号
#define WIFI_PWD_KEY "PWD"             // WiFi账号
#define BRIGHTNESS_KEY "brightness"    // 亮度
#define MODEL_KEY "model_key"          // 型号