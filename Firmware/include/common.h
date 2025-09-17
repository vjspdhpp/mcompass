#pragma once
#include "common.h"
#include "event.h"
#include "macro_def.h"

namespace mcompass {

/// 罗盘型号
enum class Model {
  LITE = 0, // 标准版， 无GPS的版本
  GPS = 1,  // GPS版本
};

/// @brief 罗盘状态
enum class State {
  STARTING = -1, // 启动中
  CALIBRATE = 0, // 校准
  COMPASS = 2,   // 可以指示方位的工作状态
  INFO = 3,      // 显示文字的状态,不再处理方位角数据
  FATAL = 10,    // 严重错误,无法忽略
};

/// @brief 罗盘工作模式
enum class WorkType {
  SPAWN, // 出生指针
  SOUTH, // 指南针
  MOD,
};

/// @brief 坐标
struct Location {
  float latitude;  // 维度
  float longitude; // 经度
};

/// @brief 定义距离阈值和对应休眠时间的结构
struct SleepConfig {
  float distanceThreshold; // 距离阈值(KM)
  int sleepInterval;       // 休眠时间(秒)
  bool gpsPowerEn;         // GPS启用状态, 默认开启
};

/// @brief 颜色配置
struct PointerColor {
  int spawnColor = DEFAULT_POINTER_COLOR; // 出生点颜色
  int southColor = DEFAULT_POINTER_COLOR; // 指南针颜色
};

/// @brief 服务器模式
enum class ServerMode {
  WIFI = 0, // 网页服务器
  BLE = 1,  // 蓝牙服务器
};

/// 传感器型号
enum class SensorModel {
  UNKNOWN = -1,  // 未知, 说明没有适配,
  QMC5883L = 0,  // 初代芯片,已经停产,立创也不售卖
  QMC5883P = 1,  // 替代芯片QMC5883P
  MMC5883MA = 2, // 替代芯片MMC5883MA, 有人反馈买QMC5883P,但是收到了MMC5883MA,
                 // 好在他们引脚兼容
};

class Context;
} // namespace mcompass
