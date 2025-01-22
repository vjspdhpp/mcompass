#pragma once
#include "macro_def.h"
/// @brief 罗盘状态
enum CompassState {
  STATE_LOST_BEARING = 0,   // 丢失方位
  STATE_WAIT_GPS = 1,       // 等待GPS信号
  STATE_COMPASS = 2,        // 罗盘工作状态
  STATE_CONNECT_WIFI = 3,   // 连接WiFi中
  STATE_CALIBRATE = 4,      // 校准中
  STATE_SERVER_COLORS = 10, // 网页颜色选项卡激活
  STATE_SERVER_INDEX,       // 控制指定索引的frame
  STATE_SERVER_WIFI,        // 网页WiFi选项卡激活
  STATE_SERVER_SPAWN,       // 网页出生点选项卡激活
  STATE_SERVER_INFO,        // 网页信息选项卡激活
  STATE_GAME_COMPASS = 100, // 游戏联动罗盘中
  STATE_HOTSPOT = 200       // WiFi热点状态中
};

/// @brief 罗盘工作模式
enum CompassType {
  LocationCompass, // 地点指针
  SouthCompass,    // 指南针
};

/// @brief 坐标
struct Location {
  float latitude;
  float longitude;
};

// 定义距离阈值和对应休眠时间的结构
struct SleepConfig {
  float distanceThreshold; // 距离阈值(KM)
  int sleepInterval;       // 休眠时间(秒)
  bool gpsEnable;          // GPS启用状态, 默认开启
};

/// @brief 颜色配置
struct PointerColor {
  int spawnColor;
  int southColor;
};

struct Context {
  CompassState deviceState;
  CompassState lastDeviceState;
  CompassType deviceType;
  bool forceTheNether; // 强制进入下界
  bool hasGPS;
  bool useWiFi = DEFAULT_SERVER_MODE;
  PointerColor color;
  uint8_t animationFrameIndex;
  Location targetLoc;
  Location currentLoc;
  // 定位任务Handle
  TaskHandle_t gpsTaskHandle = NULL;
};