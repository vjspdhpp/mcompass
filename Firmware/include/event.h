#pragma once
#include <esp_event.h>
#include <stdint.h>

ESP_EVENT_DECLARE_BASE(MCOMPASS_EVENT);

namespace Event {

// 消息类型
enum class Type {
  AZIMUTH,             // 方位角
  TEXT,                // 文字
  BUTTON_CLICK,        // 单击
  BUTTON_LONG_PRESS,   // 长按
  BUTTON_MULTI_CLICK,  // 多次点击
  SENSOR_CALIBRATE,    // 传感器校准
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
    struct {  // 方位角
      int angle;
    } azimuth;
    struct {  // 文字
      char text[64];
    } TEXT;
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
