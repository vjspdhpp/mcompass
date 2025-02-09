#include <FastLED.h>
#include <NimBLEDevice.h>
#include <Preferences.h>
#include <QMC5883LCompass.h>
#include <esp_log.h>
#include <esp_task_wdt.h>

#include "event.h"
#include "func.h"

ESP_EVENT_DEFINE_BASE(MCOMPASS_EVENT);

static const char *TAG = "MAIN";
static mcompass::Context *context;

void event_dispatcher(void *handler_arg, esp_event_base_t base, int32_t id,
                      void *event_data) {
  static uint32_t last_update = 0;
  Event::Body *evt = (Event::Body *)event_data;

  switch (evt->type) {
    case Event::Type::AZIMUTH:
      if (context->deviceState == mcompass::State::COMPASS) {
        if (millis() - last_update >= 33) {  // 30Hz刷新率
          pixel::showByAzimuth(evt->azimuth.angle);
          last_update = millis();
        }
      }
      break;
    case Event::Type::MARQUEE:
      // 根据文本数据进行处理，如显示跑马灯效果
      break;
    default:
      break;
  }
}
void setup() {
  // 延时,用于一些特殊情况下能够重新烧录
  delay(2000);
  Serial.begin(115200);
  // 初始化硬件相关, 返回Context
  context = board::init();
  // 创建事件循环（这里使用自定义事件循环）
  esp_event_loop_args_t loop_args = {
      .queue_size = 128,
      .task_name = "event_loop",
      .task_priority = configMAX_PRIORITIES - 1,
      .task_stack_size = 2048,
  };

  esp_event_loop_create(&loop_args, &context->eventLoop);

  // 注册事件处理程序
  esp_event_handler_register_with(context->eventLoop, MCOMPASS_EVENT, 0,
                                  event_dispatcher, NULL);
}

void loop() { delay(200); }
