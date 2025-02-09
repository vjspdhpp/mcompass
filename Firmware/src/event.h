#pragma once
#include <esp_event.h>
#include <stdint.h>

ESP_EVENT_DECLARE_BASE(MCOMPASS_EVENT);

namespace Event {

// 消息类型
enum Type {
  AZIMUTH,  // 方位角
  MARQUEE,  // 跑马灯
};

// 消息源
enum Source {
  BUTTON,      // 按钮
  SENSOR,      // 传感器
  WEB_SERVER,  // Web服务器
  BLE,         // BLE
  GPS,         // GPS
  OTHER,       // 其他
  NETHER,      // 地狱
};

// 事件结构
struct Body {
  Type type;      // 消息类型
  Source source;  // 消息源头
  union {
    struct {  // Azimuth 类型的数据
      float angle;
    } azimuth;
    struct {  //  Marquee 类型的数据
      char text[64];
    } marquee;
  };
};
/**
 *  @brief Type 转换为 const char *
 */
const char* TypeToString(Type type);

/**
 *  @brief Source 转换为 const char *
 */
const char* SourceToString(Source source);
}  // namespace Event
