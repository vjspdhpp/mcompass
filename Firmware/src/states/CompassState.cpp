#include "states/CompassState.h"
#include "context.h"
#include "states/CalibratingState.h"  // 用于状态切换
#include "states/FactoryResetState.h" // 用于状态切换

#include "gps_def.h"
#include "pixel_def.h"
#include "preference_def.h"
#include <esp_log.h>

void CompassState::onEnter(Context &context) {
  // 进入罗盘状态时, 确保数据源正确 (例如)
  if (context.getWorkType() == WorkType::SOUTH) {
    context.setSubscribeSource(Event::Source::SENSOR);
  }
}

void CompassState::onExit(Context &context) {
  // 离开罗盘状态时... (例如, 清理屏幕?)
  // FastLED.clear();
}

void CompassState::handleEvent(Context &context, Event::Body *evt) {
  static uint32_t last_update = 0;
  switch (evt->type) {
  case Event::Type::BUTTON_CLICK: {
    ESP_LOGI(getName(), "BUTTON_CLICK");
    auto workType = context.getWorkType();
    if (workType == WorkType::SPAWN) {
      context.setSubscribeSource(Event::Source::SENSOR);
      ESP_LOGI(getName(), "setWorkType to SOUTH");
      context.setWorkType(WorkType::SOUTH);
    } else if (workType == WorkType::SOUTH) {
      ESP_LOGI(getName(), "setWorkType to SPAWN");
      context.setWorkType(WorkType::SPAWN);
    }
    break;
  }
  case Event::Type::BUTTON_LONG_PRESS: {
    ESP_LOGI(getName(), "BUTTON_LONG_PRESS");
    auto deviceState = context.getDeviceState();
    auto workType = context.getWorkType();
    auto currentLoc = context.getCurrentLocation();
    if (workType == WorkType::SPAWN) {
      // 检查GPS坐标是否有效
      if (gps::isValidGPSLocation(currentLoc)) {
        preference::saveSpawnLocation(currentLoc);
        context.setSpawnLocation(currentLoc);
        ESP_LOGI(getName(), "Set spawn location to {%.2f,%.2f}",
                 currentLoc.latitude, currentLoc.longitude);
      } else {
        ESP_LOGW(getName(), "Can't set spawn location, invalid GPS data.");
      }
    } else if (workType == WorkType::SOUTH) {
      // 指南针模式下， 长按切换数据源
      if (context.getSubscribeSource() == Event::Source::SENSOR) {
        context.setSubscribeSource(Event::Source::NETHER);
        ESP_LOGI(getName(), "Switch Data Source to NETHER");
      } else if (context.getSubscribeSource() == Event::Source::NETHER) {
        context.setSubscribeSource(Event::Source::SENSOR);
        ESP_LOGI(getName(), "Switch Data Source to SENSOR");
      }
    }
    break;
  }
  case Event::Type::AZIMUTH: {
    // ESP_LOGI(getName(), "evt->azimuth.angle=%d",
    // evt->azimuth.angle);
    // 状态校验, 非COMPASS状态忽略方位角数据
    if (context.getDeviceState() != State::COMPASS)
      return;
    if (context.getWorkType() == WorkType::SPAWN) {
      pixel::setPointerColor(context.getColor().spawnColor);
      if (!context.getIsGPSFixed()) {
        // 当前位置无效, 显示来自Nether的方位角
        if (evt->source == Event::Source::NETHER) {
          pixel::showByAzimuth(evt->azimuth.angle);
        }
      } else {
        if (evt->source == Event::Source::SENSOR) {
          // 当前位置有效, 使用SENSOR数据计算目标位置方位角
          pixel::showFrameByLocation(context.getCurrentLocation().latitude,
                                     context.getCurrentLocation().longitude,
                                     context.getSpawnLocation().latitude,
                                     context.getSpawnLocation().longitude,
                                     evt->azimuth.angle);
        }
      }

    } else if (context.getWorkType() == WorkType::SOUTH) {
      static int lastAzimuth = 0;
      // 指南针模式下忽略非订阅的源,否则会受到随机数据影响
      if (context.getSubscribeSource() != evt->source)
        return;
      context.setAzimuth(evt->azimuth.angle);
      pixel::setPointerColor(context.getColor().southColor);
      pixel::showByAzimuth(evt->azimuth.angle);
      // 减少日志打印
      if (lastAzimuth != evt->azimuth.angle) {
        if (abs(lastAzimuth - evt->azimuth.angle) > 5) {
          ESP_LOGI(getName(), "SOUTH azimuth=%d evt->source=%d",
                   evt->azimuth.angle, evt->source);
        }
        lastAzimuth = evt->azimuth.angle;
      }
    } else {
      // MOD 模式, 只显示来自服务器的数据
      if (evt->source == Event::Source::WEB_SERVER) {
        pixel::showByAzimuth(evt->azimuth.angle);
      }
    }

    break;
  }

  case Event::Type::SENSOR_CALIBRATE: {
    ESP_LOGI(getName(), "SENSOR_CALIBRATE event received, switching state.");
    context.setState(new CalibratingState()); // 切换到校准状态
    break;
  }
  case Event::Type::FACTORY_RESET: {
    ESP_LOGI(getName(), "FACTORY_RESET event received, switching state.");
    context.setState(new FactoryResetState()); // 恢复出厂设置
    break;
  }
  case Event::Type::TEXT: {

    break;
  }

  // 其他事件 (例如 TEXT) 在此状态下被自动忽略
  default:
    break;
  }
}