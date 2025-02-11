#include <FastLED.h>
#include <NimBLEDevice.h>
#include <Preferences.h>
#include <QMC5883LCompass.h>
#include <esp_log.h>
#include <esp_task_wdt.h>

#include "event.h"
#include "board.h"

using namespace mcompass;

ESP_EVENT_DEFINE_BASE(MCOMPASS_EVENT);

static const char *TAG = "MAIN";

void event_dispatcher(void *handler_arg, esp_event_base_t base, int32_t id,
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
    case Event::Type::MARQUEE:
      // 状态校验, 非INFO状态,忽略MARQUEE
      if (context.getDeviceState() != State::INFO) return;
      ESP_LOGI(TAG, "MARQUEE %s", evt->marquee.text);
      break;
    default:
      break;
  }
}
void setup() {
  // 延时,用于一些特殊情况下能够重新烧录
  delay(2000);
  Context &context = Context::getInstance();
  auto eventLoop = context.getEventLoop();

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
                                                  event_dispatcher, NULL));
  ESP_LOGI(TAG, "Event loop created %p", eventLoop);

  // 初始化硬件相关, 返回Context
  board::init();
}

void loop() { delay(200); }
