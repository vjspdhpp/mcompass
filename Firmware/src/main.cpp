#include <FastLED.h>
#include <NimBLEDevice.h>
#include <Preferences.h>
#include <QMC5883LCompass.h>
#include <esp_log.h>
#include <esp_task_wdt.h>

#include "board.h"
#include "event.h"

using namespace mcompass;

ESP_EVENT_DEFINE_BASE(MCOMPASS_EVENT);

static const char *TAG = "MAIN";
esp_event_loop_handle_t eventLoop;

void dispatcher(void *handler_arg, esp_event_base_t base, int32_t id,
                void *event_data) {
  static uint32_t last_update = 0;
  Event::Body *evt = (Event::Body *)event_data;
  Context &context = Context::getInstance();
  switch (evt->type) {
    case Event::Type::BUTTON_CLICK: {
      ESP_LOGI(TAG, "BUTTON_CLICK");
      auto deviceState = context.getDeviceState();
      auto workType = context.getWorkType();
      switch (deviceState) {
        case State::COMPASS: {
          // 切换罗盘工作类型
          ESP_LOGI(TAG, "Toggle WorkType to %s",
                   workType == WorkType::SPAWN ? "SPAWN" : "SOUTH");
          context.toggleWorkType();
          break;
        }
        default:
          break;
      }
    } break;
    case Event::Type::BUTTON_LONG_PRESS: {
      ESP_LOGI(TAG, "BUTTON_LONG_PRESS");
      auto deviceState = context.getDeviceState();
      auto workType = context.getWorkType();
      auto currentLoc = context.getCurrentLocation();
      switch (deviceState) {
        // COMPASS模式下响应长按事件
        case State::COMPASS: {
          // 出生针模式下， 长按设置新的出生点
          if (workType == WorkType::SPAWN) {
            // 检查GPS坐标是否有效
            if (gps::isValidGPSLocation(currentLoc)) {
              preference::saveSpawnLocation(currentLoc);
              context.setSpawnLocation(currentLoc);
              ESP_LOGI(TAG, "Set spawn location to {%.2f,%.2f}",
                       currentLoc.latitude, currentLoc.longitude);
            } else {
              ESP_LOGW(TAG, "Can't set spawn location, invalid GPS data.");
            }
          } else if (workType == WorkType::SOUTH) {
            // 指南针模式下， 长按切换数据源到nether
            ESP_LOGI(TAG, "Switch Data Source to Nether");
            context.setSubscribeSource(Event::Source::NETHER);
          }
          break;
        }
        default:
          break;
      }
    } break;
    case Event::Type::BUTTON_MULTI_CLICK: {
      ESP_LOGI(TAG, "BUTTON_MULTI_CLICK");
      ESP_LOGI(TAG, "xTaskCreate");
      // 启动异步倒计时任务
      xTaskCreate(
          [](void *ctx) {
            ESP_LOGI(TAG, "cast context %p", ctx);
            auto context = static_cast<Context *>(ctx);
            auto deviceState = context->getDeviceState();
            ESP_LOGI(TAG, "deviceState=%d", deviceState);
            if (deviceState != State::COMPASS) {
              vTaskDelete(NULL);
              return;
            }
            context->setDeviceState(State::INFO);
            ESP_LOGW(TAG, "Factory Reset!!!");
            // 恢复出厂设置
            // 倒计时3秒
            pixel::counterDown(3);
            preference::factoryReset();
            // 重启
            esp_restart();
          },
          "factory_reset", 2048, &context, 1, NULL);
    } break;
    case Event::Type::AZIMUTH: {
      // ESP_LOGI(TAG, "evt->azimuth.angle=%d",
      // evt->azimuth.angle);
      context.setAzimuth(evt->azimuth.angle);
      // 状态校验, 非COMPASS状态忽略方位角数据
      if (context.getDeviceState() != State::COMPASS) return;
      // 校验订阅数据源头, 忽略非订阅的源
      if (context.getSubscribeSource() != evt->source) return;
      // 校验刷新频率, 限制帧率30Hz
      if (millis() - last_update < 33) return;
      pixel::setPointerColor(context.getColor().southColor);
      pixel::showByAzimuth(360 - evt->azimuth.angle);
      last_update = millis();
    } break;
    case Event::Type::TEXT: {  // 状态校验, 非INFO状态,忽略TEXT
      if (context.getDeviceState() != State::INFO) return;
      ESP_LOGW(TAG, "TEXT %s", evt->TEXT.text);
    } break;
    default:
      break;
  }
}
void setup() {
  // 延时,用于一些特殊情况下能够重新烧录
  delay(500);
  Context &context = Context::getInstance();
  esp_event_loop_args_t loop_args = {
      .queue_size = 128,
      .task_name = "event_loop",
      .task_priority = configMAX_PRIORITIES - 1,
      .task_stack_size = 2048,
  };
  ESP_ERROR_CHECK(esp_event_loop_create(&loop_args, &eventLoop));
  context.setEventLoop(eventLoop);
  // 注册事件处理程序
  ESP_ERROR_CHECK(esp_event_handler_register_with(eventLoop, MCOMPASS_EVENT, 0,
                                                  dispatcher, NULL));
  ESP_LOGI(TAG, "Event loop created %p", eventLoop);

  // 初始化硬件
  board::init();

  // 输出上下文内容
  ESP_LOGI(
      TAG,
      "Context {\n "
      "ServerMode:%d\n PointerColor{0x%x,0x%x}\n Brightness:%d\n "
      "SpawnLocation:{latitude:%.2f,longitude:%.2f}\n WiFi:%s\n Model:%s\n "
      "HasSensor:%d\n detectGPS:%d\n "
      "}",
      context.getServerMode(), context.getColor().spawnColor,
      context.getColor().southColor, context.getBrightness(),
      context.getSpawnLocation().latitude, context.getSpawnLocation().longitude,
      context.getSsid(), context.getModel() == Model::GPS ? "GPS" : "LITE",
      context.getHasSensor(), context.getDetectGPS());

  ESP_LOGW(TAG, "Board initialized");
}

void loop() { delay(200); }
