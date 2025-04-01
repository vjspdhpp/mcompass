#include <OneButton.h>
#include <WiFi.h>

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
        if (buttonInstance.getNumberClicks() == 8) {
          // 八次点击,恢复出厂设置
          auto context = static_cast<Context *>(ctx);
          Event::Body event;
          event.type = Event::Type::FACTORY_RESET;
          event.source = Event::Source::BUTTON;
          auto eventLoop = context->getEventLoop();
          ESP_ERROR_CHECK(esp_event_post_to(eventLoop, MCOMPASS_EVENT, 0,
                                            &event, sizeof(event),
                                            portMAX_DELAY));
        } else if (buttonInstance.getNumberClicks() == 6) {
          // 六次点击,传感器校准
          auto context = static_cast<Context *>(ctx);
          Event::Body event;
          event.type = Event::Type::SENSOR_CALIBRATE;
          event.source = Event::Source::BUTTON;
          auto eventLoop = context->getEventLoop();
          ESP_ERROR_CHECK(esp_event_post_to(eventLoop, MCOMPASS_EVENT, 0,
                                            &event, sizeof(event),
                                            portMAX_DELAY));
        } else if (buttonInstance.getNumberClicks() == 4) {
          // 四次点击,显示IP
          if (WiFi.getMode() != WIFI_AP && WiFi.localIP() == INADDR_NONE) {
            ESP_LOGI(TAG, "WiFi is not connected, skip show ip");
            return;
          }
          auto context = static_cast<Context *>(ctx);
          Event::Body event;
          event.type = Event::Type::TEXT;
          event.source = Event::Source::BUTTON;
          // 如果是AP模式, 显示AP的IP
          if (WiFi.getMode() == WIFI_AP) {
            strcpy(event.TEXT.text, WiFi.softAPIP().toString().c_str());
          } else {
            strcpy(event.TEXT.text, WiFi.localIP().toString().c_str());
          }
          auto eventLoop = context->getEventLoop();
          context->setDeviceState(State::INFO);
          ESP_ERROR_CHECK(esp_event_post_to(eventLoop, MCOMPASS_EVENT, 0,
                                            &event, sizeof(event),
                                            portMAX_DELAY));
          // 定时器, 5秒后,退出IP展示
          esp_timer_handle_t timer;
          esp_timer_create_args_t timer_args = {
              .callback =
                  [](void *arg) {
                    auto context = static_cast<Context *>(arg);
                    context->setDeviceState(context->getLastDeviceState());
                    ESP_LOGI(TAG, "Exit IP show");
                  },
              .arg = context,
              .dispatch_method = ESP_TIMER_TASK,
              .name = "show_ip_timer",
              .skip_unhandled_events = true};
          esp_timer_create(&timer_args, &timer);
          esp_timer_start_once(timer, 5000000);  // 5秒 = 5000000us
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