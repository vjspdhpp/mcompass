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
  QMC5883L = 0,  // 初代芯片,已经停产,立创也不售卖
  QMC5883P = 1,  // 替代芯片
};

/// @brief 上下文
class Context {
public:
  // 获取单例对象
  static Context &getInstance() {
    static Context instance;
    return instance;
  }

  // 禁止拷贝构造和赋值操作
  Context(const Context &) = delete;
  Context &operator=(const Context &) = delete;

  void setModel(Model model) { this->model = model; }

  Model getModel() const { return model; }

  bool isModel(const Model model) { return this->model == model; }

  bool isGPSModel() { return this->isModel(mcompass::Model::GPS); }

  State getDeviceState() const { return deviceState; }
  void setDeviceState(State state) { deviceState = state; }

  State getLastDeviceState() const { return lastDeviceState; }
  void setLastDeviceState(State state) { lastDeviceState = state; }

  WorkType getWorkType() const { return workType; }
  void setWorkType(WorkType wt) { workType = wt; }
  void toggleWorkType() {
    setWorkType(workType == mcompass::WorkType::SPAWN
                    ? mcompass::WorkType::SOUTH
                    : mcompass::WorkType::SPAWN);
  }

  bool getDetectGPS() const { return detectGPS; }
  void setDetectGPS(bool detect) { detectGPS = detect; }

  int getSensorModel() const { return DEFAULT_SENSOR_MODEL; }

  bool getHasSensor() const { return hasSensor; }
  void setHasSensor(bool sensor) { hasSensor = sensor; }

  PointerColor getColor() const { return color; }
  void setColor(PointerColor c) { color = c; }

  Location getCurrentLocation() const { return currentLoc; }
  void setCurrentLocation(const Location &loc) { currentLoc = loc; }

  Location getSpawnLocation() const { return spawnLocation; }
  void setSpawnLocation(const Location &loc) { spawnLocation = loc; }

  ServerMode getServerMode() const { return serverMode; }
  void setServerMode(ServerMode mode) { serverMode = mode; }

  uint8_t getBrightness() const { return brightness; }
  void setBrightness(uint8_t bright) { brightness = bright; }

  String getSsid() const { return ssid; }
  void setSsid(const String &id) { ssid = id; }

  String getPassword() const { return password; }
  void setPassword(const String &pass) { password = pass; }

  Event::Source getSubscribeSource() const { return subscribeSource; }
  void setSubscribeSource(Event::Source src) { subscribeSource = src; }

  int getAzimuth() const { return azimuth; }
  void setAzimuth(int azi) {
    setLastAzimuth(azimuth);
    azimuth = azi;
  }

  int getLastAzimuth() const { return lastAzimuth; }
  void setLastAzimuth(int azi) { lastAzimuth = azi; }

  esp_event_loop_handle_t getEventLoop() { return eventLoop; }

  void setEventLoop(esp_event_loop_handle_t loop) { eventLoop = loop; }

private:
  // 私有构造函数，确保外部不能直接创建对象
  Context() = default;
  ~Context() = default;

  // 内部变量定义
  Model model;
  State deviceState = State::COMPASS;
  State lastDeviceState = State::STARTING;
  // 根据 model 判断默认的工作类型
  WorkType workType = (model == Model::GPS ? WorkType::SPAWN : WorkType::SOUTH);
  bool detectGPS = false;
  bool hasSensor = false;
  PointerColor color;  // 默认构造，具体初值请根据需求设置
  Location currentLoc; // 当前位置，初值默认构造
  // 默认目标位置设置为天安门经纬度（示例值）
  Location spawnLocation{39.908692f, 116.397477f};
  ServerMode serverMode = DEFAULT_SERVER_MODE;
  uint8_t brightness = DEFAULT_BRIGHTNESS;
  String ssid = "";
  String password = "";
  Event::Source subscribeSource = Event::Source::SENSOR;
  int azimuth = 0;
  int lastAzimuth = 0;
  esp_event_loop_handle_t eventLoop;
};

} // namespace mcompass
