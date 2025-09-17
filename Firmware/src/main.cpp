#include <FastLED.h>
#include <NimBLEDevice.h>
#include <Preferences.h>
#include <QMC5883LCompass.h>
#include <esp_log.h>
#include <esp_task_wdt.h>

#include "board.h"
#include "context.h"
#include "event.h"
using namespace mcompass;

ESP_EVENT_DEFINE_BASE(MCOMPASS_EVENT);

static const char *TAG = "MAIN";
esp_event_loop_handle_t eventLoop;

void dispatcher(void *handler_arg, esp_event_base_t base, int32_t id,
                void *event_data) {
  Event::Body *evt = (Event::Body *)event_data;
  Context &context = Context::getInstance();

  if (context.getCurrentState()) {
    context.getCurrentState()->handleEvent(context, evt);
  }
}

void setup() {
  // 延时,用于一些特殊情况下能够重新烧录
  delay(1000);
  Context &context = Context::getInstance();
  esp_event_loop_args_t loop_args = {
      .queue_size = 128,
      .task_name = "event_loop",
      .task_priority = configMAX_PRIORITIES - 1,
      .task_stack_size = 1024 * 8,
  };
  ESP_ERROR_CHECK(esp_event_loop_create(&loop_args, &eventLoop));
  context.setEventLoop(eventLoop);
  // 注册事件处理程序
  ESP_ERROR_CHECK(esp_event_handler_register_with(eventLoop, MCOMPASS_EVENT, 0,
                                                  dispatcher, NULL));
  ESP_LOGI(TAG, "Event loop created %p", eventLoop);

  // 初始化硬件
  board::init();
}

void loop() { vTaskDelay(pdMS_TO_TICKS(2000)); }
