#pragma once

#define NUM_LEDS 42
#define MAX_FRAME_INDEX 26

/// 引脚定义
#define DATA_PIN 6
#define CALIBRATE_PIN 9
#define GPS_EN_PIN 0

/// 默认值
// 指针颜色
#define DEFAULT_NEEDLE_COLOR 0xff1414 // 指针红
// 默认亮度
#define DEFAULT_BRIGHTNESS 64
// 默认模式
#define DEFAULT_SERVER_MODE 1 // 蓝牙模式

// 默认连接连接WiFi等待时间 10秒
#define DEFAULT_WIFI_CONNECT_TIME 10 * 1000
// 默认无client连接关闭web server时间 120个tick, 一个tick对应一秒钟
#define DEFAULT_SERVER_TICK_COUNT 120
// 默认检测不到GPS,关闭GPS供电时间
#define DEFAULT_GPS_DETECT_TIMEOUT 60 * 1000

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

// 默认初始的坐标值
#define DEFAULT_INVALID_LOCATION_VALUE 255.0f