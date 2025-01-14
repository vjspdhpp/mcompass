#pragma once

#define NUM_LEDS 42
#define MAX_FRAME_INDEX 26

/// 引脚定义
#define DATA_PIN 6
#define CALIBRATE_PIN 9
#define GPS_EN_PIN 0

#define DEFAULT_NEEDLE_COLOR 0xff1414

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
