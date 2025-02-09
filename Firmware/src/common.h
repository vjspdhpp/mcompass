#pragma once
#include "common.h"
#include "event.h"
#include "macro_def.h"

namespace mcompass {

/// 罗盘型号
enum Model {
  LITE = 0,  // 标准版， 无GPS的版本
  GPS = 1,   // GPS版本
};

/// @brief 罗盘状态
enum State {
  UNDEFINED = -1,      // 未定义
  LOST_BEARING = 0,    // 丢失方位
  WAIT_GPS = 1,        // 等待GPS信号
  COMPASS = 2,         // 罗盘工作状态
  CONNECT_WIFI = 3,    // 连接WiFi中
  CALIBRATE = 4,       // 校准中
  SERVER_COLORS = 10,  // 网页颜色选项卡激活
  SERVER_INDEX,        // 控制指定索引的frame
  SERVER_WIFI,         // 网页WiFi选项卡激活
  SERVER_SPAWN,        // 网页出生点选项卡激活
  SERVER_INFO,         // 网页信息选项卡激活
  GAME_COMPASS = 100,  // 游戏联动罗盘中
  HOTSPOT = 200        // WiFi热点状态中
};

/// @brief 罗盘工作模式
enum WorkType {
  SPAWN,  // 地点指针
  SOUTH,  // 指南针
};

/// @brief 坐标
struct Location {
  float latitude;
  float longitude;
};

// 定义距离阈值和对应休眠时间的结构
struct SleepConfig {
  float distanceThreshold;  // 距离阈值(KM)
  int sleepInterval;        // 休眠时间(秒)
  bool gpsPowerEn;          // GPS启用状态, 默认开启
};

/// @brief 颜色配置
struct PointerColor {
  int spawnColor = DEFAULT_POINTER_COLOR;  // 出生点颜色
  int southColor = DEFAULT_POINTER_COLOR;  // 指南针颜色
};

/// @brief 服务器模式
enum ServerMode {
  WIFI = 0,
  BLE = 1,
};

/// @brief 上下文
struct Context {
  Model model = DEFAULT_MODEL;               // 罗盘型号
  State deviceState = State::COMPASS;        // 设备状态
  State lastDeviceState = State::UNDEFINED;  // 上一次设备状态
  WorkType workType =
      model == Model::GPS ? WorkType::SPAWN : WorkType::SOUTH;  // 罗盘的类型
  bool detectGPS = false;                                       // 是否检测到GPS
  bool hasSensor = false;                       // 是否检测到传感器
  PointerColor color;                           // 指针颜色， 也会用作文字的颜色
  Location currentLoc;                          // 当前位置
  Location targetLoc{39.908692f, 116.397477f};  // 目标位置, 默认设置到天安门
  ServerMode serverMode = DEFAULT_SERVER_MODE;  // 服务器模式
  uint8_t brightness = DEFAULT_BRIGHTNESS;      // 亮度
  String ssid = "";                             // WiFi SSID
  String password = "";                         // WiFi 密码
  Event::Source subscribeSource = Event::Source::SENSOR;  // 订阅的事件源
  int azimuth = 0;                                        // 方位角
  int lastAzimuth = 0;                                    // 上一次方位角
  esp_event_loop_handle_t eventLoop;                      // 事件循环
};

}  // namespace mcompass
