#include <Arduino.h>
#include <OneButton.h>
#include <esp_log.h>
#include <esp_wifi.h>

#include "event.h"
#include "func.h"

static const char *TAG = "BOARD";
static OneButton button(CALIBRATE_PIN, true);
static mcompass::Context context;
static uint32_t last_time = 0;

static void setupContext() {
  preference::init(&context);
  // 输出上下文内容
  ESP_LOGI(TAG,
           "Context {\n "
           "ServerMode:%d\n PointerColor{%x,%x}\n Brightness:%d\n "
           "HomeLocation:{latitude:%.2f,longitude:%.2f}\n WiFi:%s\n Model:%d\n"
           "}",
           context.serverMode, context.color.spawnColor,
           context.color.southColor, context.brightness,
           context.targetLoc.latitude, context.targetLoc.longitude,
           context.ssid, context.model);
}

// 校准检测
void calibrateCheck() {
  if (digitalRead(CALIBRATE_PIN) == LOW) {
    context.deviceState = mcompass::State::CALIBRATE;
  }
}

mcompass::Context *board::init() {
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
  // 初始化罗盘传感器
  sensor::init(&context);
  // GPS型号才需要初始化GPS
  if (context.model == mcompass::Model::GPS) {
    gps::init(&context);
  }
  /////////////////////// 初始化按钮 ///////////////////////
  // 单击事件
  button.attachClick(
      [](void *scope) {
        switch (context.deviceState) {
          // COMPASS模式下响应单击事件
          case mcompass::State::COMPASS: {
            // 切换罗盘工作类型
            mcompass::WorkType workType =
                context.workType == mcompass::WorkType::SPAWN
                    ? mcompass::WorkType::SOUTH
                    : mcompass::WorkType::SPAWN;
            ESP_LOGI(TAG, "Toggle Compass Type to %s",
                     workType == mcompass::WorkType::SPAWN ? "LocationCompass"
                                                           : "SouthCompass");
            context.workType = workType;
            break;
          }

          default:
            break;
        }
      },
      &button);
  // 长按事件
  button.attachLongPressStart(
      [](void *scope) {
        switch (context.deviceState) {
          // COMPASS模式下响应长按事件
          case mcompass::State::COMPASS: {
            // 出生针模式下， 长按设置新的出生点
            if (context.workType == mcompass::WorkType::SPAWN) {
              // 检查GPS坐标是否有效
              if (gps::isValidGPSLocation(context.currentLoc)) {
                preference::saveHomeLocation(context.currentLoc);
                memcpy(&context.targetLoc, &context.currentLoc,
                       sizeof(mcompass::Location));
                ESP_LOGI(TAG, "Set New Home Location to {%.2f,%.2f}",
                         context.currentLoc.latitude,
                         context.currentLoc.longitude);
              } else {
                ESP_LOGW(TAG, "Can't set home, invalid GPS data.");
              }
            } else if (context.workType == mcompass::WorkType::SOUTH) {
              // 指南针模式下， 长按切换数据源到nether
              ESP_LOGI(TAG, "Switch Data Source to Nether");
              context.subscribeSource = Event::Source::NETHER;
            }
            break;
          }
          // 连接WiFi模式下， 长按清空所有数据
          case mcompass::State::CONNECT_WIFI: {
            ESP_LOGW(TAG, "Clear WiFi");
            // 配置恢复出厂设置
            // 倒计时3秒
            pixel::counterDown(3);
            preference::factoryReset();
            // 重启
            esp_restart();
            break;
          }

          default:
            break;
        }
      },
      &button);

  /////////////////////// 根据服务器模式初始化 ///////////////////////
  context.serverMode == mcompass::ServerMode::BLE ? ble_server::init(&context)
                                                  : web_server::init(&context);

  /////////////////////// 创建按钮定时器 ///////////////////////
  esp_timer_handle_t timer;
  esp_timer_create_args_t timer_args = {
      .callback = [](void *) { button.tick(); },
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
            context.lastAzimuth = context.azimuth;
            context.azimuth = sensor::getAzimuth();
            Event::Body event;
            event.type = Event::Type::AZIMUTH;
            event.source = Event::Source::SENSOR;
            event.azimuth.angle = context.azimuth;
            esp_event_post_to(context.eventLoop, MCOMPASS_EVENT, 0, &event,
                              sizeof(event), 0);
          },
      .arg = nullptr,
      .dispatch_method = ESP_TIMER_TASK,
      .name = "sensor_timer",
      .skip_unhandled_events = true};
  esp_timer_create(&sensor_timer_args, &sensor_timer);
  esp_timer_start_periodic(sensor_timer, 10000);  // 10ms = 10000us
  return &context;
}