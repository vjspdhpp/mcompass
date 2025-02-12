#include <OneButton.h>

#include "board.h"

using namespace mcompass;
static const char *TAG = "Button";

static OneButton buttonInstance(CALIBRATE_PIN, true);

void button::init(Context *context) {
  ESP_LOGI(TAG, "Button init %p", context);
  auto ctx = context;
  /////////////////////// 初始化按钮 ///////////////////////
  // 单击事件
  buttonInstance.attachClick(
      [](void *ctx) {
        auto context = static_cast<Context *>(ctx);
        Event::Body event;
        event.type = Event::Type::BUTTON_CLICK;
        event.source = Event::Source::BUTTON;
        auto eventLoop = context->getEventLoop();
        ESP_ERROR_CHECK(esp_event_post_to(eventLoop, MCOMPASS_EVENT, 0, &event,
                                          sizeof(event), portMAX_DELAY));
      },
      ctx);
  // 多次点击
  buttonInstance.attachMultiClick(
      [](void *ctx) {
        if (buttonInstance.getNumberClicks() == 5) {
          auto context = static_cast<Context *>(ctx);
          Event::Body event;
          event.type = Event::Type::BUTTON_MULTI_CLICK;
          event.source = Event::Source::BUTTON;
          auto eventLoop = context->getEventLoop();
          ESP_ERROR_CHECK(esp_event_post_to(eventLoop, MCOMPASS_EVENT, 0,
                                            &event, sizeof(event),
                                            portMAX_DELAY));
        } else if (buttonInstance.getNumberClicks() == 3) {
          auto context = static_cast<Context *>(ctx);
          Event::Body event;
          event.type = Event::Type::SENSOR_CALIBRATE;
          event.source = Event::Source::BUTTON;
          auto eventLoop = context->getEventLoop();
          ESP_ERROR_CHECK(esp_event_post_to(eventLoop, MCOMPASS_EVENT, 0,
                                            &event, sizeof(event),
                                            portMAX_DELAY));
        }
      },
      ctx);
  // 长按事件
  buttonInstance.attachLongPressStart(
      [](void *ctx) {
        auto context = static_cast<Context *>(ctx);
        Event::Body event;
        event.type = Event::Type::BUTTON_LONG_PRESS;
        event.source = Event::Source::BUTTON;
        auto eventLoop = context->getEventLoop();
        ESP_ERROR_CHECK(esp_event_post_to(eventLoop, MCOMPASS_EVENT, 0, &event,
                                          sizeof(event), portMAX_DELAY));
      },
      ctx);
}

void button::tick() { buttonInstance.tick(); }