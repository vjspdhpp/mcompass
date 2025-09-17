#pragma once

#include "IState.h"
#include "common.h"
#include <Arduino.h>

namespace mcompass {

/// @brief 上下文
class Context {
public:
  // 获取单例对象
  static Context &getInstance() {
    static Context instance;
    return instance;
  }

  // 禁止拷贝构造和赋值操作
  //   Context(const Context &) = delete;
  //   Context &operator=(const Context &) = delete;

  void setModel(Model model);

  Model getModel() const;

  bool isModel(const Model model);

  bool isGPSModel();

  State getDeviceState() const;
  void setDeviceState(State state);

  State getLastDeviceState() const;
  void setLastDeviceState(State state);

  WorkType getWorkType() const;
  void setWorkType(WorkType wt);
  void toggleWorkType();
  bool getDetectGPS() const;
  void setDetectGPS(bool detect);

  SensorModel getSensorModel() const;
  void setSensorModel(SensorModel sm);

  bool getHasSensor() const;
  void setHasSensor(bool sensor);

  PointerColor getColor() const;
  void setColor(PointerColor c);

  Location getCurrentLocation() const;
  void setCurrentLocation(const Location &loc);

  Location getSpawnLocation() const;
  void setSpawnLocation(const Location &loc);

  ServerMode getServerMode() const;
  void setServerMode(ServerMode mode);

  uint8_t getBrightness() const;
  void setBrightness(uint8_t bright);

  String getSsid() const;
  void setSsid(const String &id);

  String getPassword() const;
  void setPassword(const String &pass);

  Event::Source getSubscribeSource() const;
  void setSubscribeSource(Event::Source src);

  int getAzimuth() const;
  void setAzimuth(int azi);

  int getLastAzimuth() const;
  void setLastAzimuth(int azi);

  esp_event_loop_handle_t getEventLoop();

  void setEventLoop(esp_event_loop_handle_t loop);

  void logSelf(char *buffer);

  void setIsGPSFixed(bool isFixed);
  bool getIsGPSFixed() const;

  void setState(IState *newState);

  IState *getCurrentState();

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
  bool hasSensor = true;
  PointerColor color;  // 默认构造，具体初值请根据需求设置
  Location currentLoc; // 当前位置，初值默认构造
  // 默认目标位置设置为天安门经纬度（示例值）
  Location spawnLocation{39.908692f, 116.397477f};
  ServerMode serverMode = DEFAULT_SERVER_MODE;
  SensorModel sensorModel = SensorModel::QMC5883L; // 传感器型号
  uint8_t brightness = DEFAULT_BRIGHTNESS;
  String ssid = "";
  String password = "";
  Event::Source subscribeSource = Event::Source::SENSOR;
  int azimuth = 0;
  int lastAzimuth = 0;
  volatile bool isGPSFixed = false;
  IState *m_currentState = nullptr;
  esp_event_loop_handle_t eventLoop;
};
} // namespace mcompass