#include <Arduino.h>
#include <esp_log.h>
#include <esp_wifi.h>

#include "board.h"
#include "event.h"

using namespace mcompass;
static const char *TAG = "BOARD";
static uint32_t last_time = 0;
Context &context = Context::getInstance();
static void setupContext() {
  preference::init(&context);
  // 输出上下文内容
  ESP_LOGI(TAG,
           "Context {\n "
           "ServerMode:%d\n PointerColor{%x,%x}\n Brightness:%d\n "
           "SpawnLocation:{latitude:%.2f,longitude:%.2f}\n WiFi:%s\n Model:%d\n"
           "}",
           context.getServerMode(), context.getColor().spawnColor,
           context.getColor().southColor, context.getBrightness(),
           context.getSpawnLocation().latitude,
           context.getSpawnLocation().longitude, context.getSsid(),
           context.getModel());
}

// 校准检测
void calibrateCheck() {
  if (digitalRead(CALIBRATE_PIN) == LOW) {
    Context::getInstance().setDeviceState(State::CALIBRATE);
  }
}

void board::init() {
  Serial.begin(115200);
  ESP_LOGI(TAG, "Board init %p", &context);
  // 初始化上下文
  setupContext();
  // 设置引脚模式
  pinMode(CALIBRATE_PIN, INPUT_PULLUP);
  pinMode(GPS_EN_PIN, OUTPUT);
  // 关闭GPS电源
  digitalWrite(GPS_EN_PIN, HIGH);
  // 初始化串口
  Serial.begin(115200);
  // 初始化LED
  pixel::init(&context);
  // 初始化按钮
  button::init(&context);
  // 初始化罗盘传感器
  sensor::init(&context);
  // GPS型号才需要初始化GPS
  if (context.isGPSModel()) {
    gps::init(&context);
  }

  /////////////////////// 根据服务器模式初始化 ///////////////////////
  context.getServerMode() == ServerMode::BLE ? ble_server::init(&context)
                                             : web_server::init(&context);

  /////////////////////// 创建按钮定时器 ///////////////////////
  esp_timer_handle_t timer;
  esp_timer_create_args_t timer_args = {
      .callback = [](void *) { button::tick(); },
      .arg = nullptr,
      .dispatch_method = ESP_TIMER_TASK,
      .name = "button_timer",
      .skip_unhandled_events = true};
  esp_timer_create(&timer_args, &timer);
  esp_timer_start_periodic(timer, 10000);  // 10ms = 10000us
  /////////////////////// 创建传感器定时器 ///////////////////////
  esp_timer_handle_t sensor_timer;
  esp_timer_create_args_t sensor_timer_args = {
      .callback =
          [](void *) {
            auto azimuth = sensor::getAzimuth();
            Event::Body event;
            event.type = Event::Type::AZIMUTH;
            event.source = Event::Source::SENSOR;
            event.azimuth.angle = azimuth;
            ESP_ERROR_CHECK(esp_event_post_to(context.getEventLoop(),
                                              MCOMPASS_EVENT, 0, &event,
                                              sizeof(event), 0));
          },
      .arg = nullptr,
      .dispatch_method = ESP_TIMER_TASK,
      .name = "sensor_timer",
      .skip_unhandled_events = true};
  esp_timer_create(&sensor_timer_args, &sensor_timer);
  esp_timer_start_periodic(sensor_timer, 20000);
}