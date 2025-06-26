#pragma once

///////////////////// 宏定义 ///////////////////////
#define NUM_LEDS 42
#define MAX_FRAME_INDEX 26
#define TIME_ZONE (+8)   // Beijing Time
#define YEAR_BASE (2000) // date in GPS starts from 2000
///////////////////// 引脚定义 ///////////////////////
#define DATA_PIN 6
#if defined(CONFIG_IDF_TARGET_ESP32C3)
#define CALIBRATE_PIN 9
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
#define CALIBRATE_PIN 41
#else
#error "Unsupported target"
#endif

#define GPS_EN_PIN 0

///////////////////// 默认值 ///////////////////////
#define EARTH_RADIUS 6371.0 // 地球半径（单位：公里）
// 指针颜色
#define DEFAULT_POINTER_COLOR 0xFF1414 // 指针红
// 默认亮度
#define DEFAULT_BRIGHTNESS 56

// 默认连接连接WiFi等待时间 15秒
#define DEFAULT_WIFI_CONNECT_TIME 15
// 默认无client连接关闭web server时间
#define DEFAULT_SERVER_TIMEOUT 120
// 默认检测不到GPS,关闭GPS供电时间
#define DEFAULT_GPS_DETECT_TIMEOUT 30

// 默认初始的坐标值
#define DEFAULT_INVALID_LOCATION_VALUE 255.0f

// 默认服务器模式
#ifndef DEFAULT_SERVER_MODE
#define DEFAULT_SERVER_MODE mcompass::ServerMode::BLE
#endif

// 默认型号
#ifndef DEFAULT_MODEL
#define DEFAULT_MODEL mcompass::Model::LITE
#endif


#define SENSOR_MODEL_QMC5883L 0 // 初代芯片,已经停产,立创也不售卖
#define SENSOR_MODEL_QMC5883P 1 // 替代芯片
// 默认传感器型号
#ifndef DEFAULT_SENSOR_MODEL
#define DEFAULT_SENSOR_MODEL SENSOR_MODEL_QMC5883P
#endif

///////////////////// 版本信息 ///////////////////////
#if !defined(BUILD_VERSION)
#define BUILD_VERSION "UNKNOWN"
#endif // MACRO

#if !defined(GIT_BRANCH)
#define GIT_BRANCH "UNKNOWN"
#endif // MACRO

#if !defined(GIT_COMMIT)
#define GIT_COMMIT "UNKNOWN"
#endif // MACRO

#define INFO_JSON                                                              \
  "{\"buildDate\":\"" __DATE__ "\",\"buildTime\":\"" __TIME__                  \
  "\",\"buildVersion\":\"" BUILD_VERSION "\",\"gitBranch\":\"" GIT_BRANCH      \
  "\",\"gitCommit\":\"" GIT_COMMIT "\"}"

///////////////////// 蓝牙相关 ///////////////////////
/* 基础配置 */
#define BASE_SERVICE_UUID (uint16_t)0xf000
#define COLOR_CHARACTERISITC_UUID (uint16_t)(BASE_SERVICE_UUID + 1)  // 指针颜色
#define AZIMUTH_CHARACHERSITC_UUID (uint16_t)(BASE_SERVICE_UUID + 2) // 方位角
#define SPAWN_CHARACTERISTIC_UUID                                              \
  (uint16_t)(BASE_SERVICE_UUID + 3)                                // 出生点信息
#define INFO_CHARACTERISTIC_UUID (uint16_t)(BASE_SERVICE_UUID + 4) // 设备信息
#define BRIGHTNESS_CHARACTERISTIC_UUID                                         \
  (uint16_t)(BASE_SERVICE_UUID + 5) // 亮度控制
#define CALIBRATE_CHARACTERISTIC_UUID                                          \
  (uint16_t)(BASE_SERVICE_UUID + 6)                                  // 请求校准
#define REBOOT_CHARACTERISTIC_UUID (uint16_t)(BASE_SERVICE_UUID + 7) // 重启设备
#define SERVER_MODE_CHARACTERISTIC_UUID                                        \
  (uint16_t)(BASE_SERVICE_UUID + 8) // 服务器模式
#define CUSTOM_MODEL_CHARACTERISTIC_UUID                                       \
  (uint16_t)(BASE_SERVICE_UUID + 9) // 自定义型号

/** 高级配置  */
#define ADVANCED_SERVICE_UUID (uint16_t)0xfa00
#define VIRTUAL_LOCATION_CHARACTERISTIC_UUID                                   \
  (uint16_t)(ADVANCED_SERVICE_UUID + 1) // 虚拟坐标
#define VIRTUAL_AZIMUTH_CHARACTERISTIC_UUID                                    \
  (uint16_t)(ADVANCED_SERVICE_UUID + 2) // 虚拟方位角

///////////////////// 配置相关 ///////////////////////
#define PREFERENCE_NAME "mcompass" // 配置文件名称

#define LATITUDE_KEY "latitude"           // 纬度Key
#define LONGTITUDE_KEY "longitude"        // 经度Key
#define SPAWN_COLOR_KEY "spawn_color"     // 出生针颜色
#define SOUTH_COLOR_KEY "south_color"     // 指南针颜色
#define SERVER_MODE_KEY "server_mode"     // 配置模式
#define WIFI_SSID_KEY "SSID"              // WiFi账号
#define WIFI_PWD_KEY "PWD"                // WiFi账号
#define BRIGHTNESS_KEY "brightness"       // 亮度
#define MODEL_KEY "model_key"             // 型号
#define CALIBRATION_KEY "calibration_key" // 校准数据

///////////////////// 错误信息 ///////////////////////
#define SENSOR_ERROR "Sensor Error 100"           // 传感器错误
#define FILE_SYSTEM_ERROR "File System Error 101" // 文件系统错误
