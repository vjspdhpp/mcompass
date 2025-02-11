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
    case Event::Type::AZIMUTH:
      context.setAzimuth(evt->azimuth.angle);
      // 状态校验, 非COMPASS状态忽略方位角数据
      if (context.getDeviceState() != State::COMPASS) return;
      // 校验订阅数据源头, 忽略非订阅的源
      if (context.getSubscribeSource() != evt->source) return;
      // 校验刷新频率, 限制帧率30Hz
      if (millis() - last_update < 33) return;
      pixel::showByAzimuth(evt->azimuth.angle);
      last_update = millis();
      break;
    case Event::Type::TEXT:
      // 状态校验, 非INFO状态,忽略TEXT
      if (context.getDeviceState() != State::INFO) return;
      ESP_LOGW(TAG, "TEXT %s", evt->TEXT.text);
      break;
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
  ESP_LOGI(TAG,
           "Context {\n "
           "ServerMode:%d\n PointerColor{0x%x,0x%x}\n Brightness:%d\n "
           "SpawnLocation:{latitude:%.2f,longitude:%.2f}\n WiFi:%s\n Model:%s\n "
           "HasSensor:%d\n detectGPS:%d\n "
           "}",
           context.getServerMode(), context.getColor().spawnColor,
           context.getColor().southColor, context.getBrightness(),
           context.getSpawnLocation().latitude,
           context.getSpawnLocation().longitude, context.getSsid(),
           context.getModel() == Model::GPS ? "GPS" : "LITE",
           context.getHasSensor(), context.getDetectGPS());

  ESP_LOGW(TAG, "Board initialized");
}

void loop() { delay(200); }
