#include <Arduino.h>
#include <OneButton.h>
#include <esp_log.h>
#include <esp_wifi.h>

#include "func.h"

static const char *TAG = "BOARD";
static OneButton button(CALIBRATE_PIN, true);
static Context context;

static void setupContext() {
  context.currentLoc = {.latitude = DEFAULT_INVALID_LOCATION_VALUE,
                        .longitude = DEFAULT_INVALID_LOCATION_VALUE};
  context.targetLoc = {.latitude = 43.0f, .longitude = 126.0f};
  context.deviceState = CompassState::STATE_LOST_BEARING;
  context.lastDeviceState = CompassState::STATE_LOST_BEARING;
  context.deviceType = CompassType::LocationCompass;
  context.animationFrameIndex = 0;
  context.forceTheNether = false;
}

// 校准检测
void Board::calibrateCheck() {
  if (digitalRead(CALIBRATE_PIN) == LOW) {
    context.deviceState = CompassState::STATE_CALIBRATE;
  }
}

Context *Board::init() {
  setupContext();
  pinMode(CALIBRATE_PIN, INPUT_PULLUP);
  pinMode(GPS_EN_PIN, OUTPUT);
  digitalWrite(GPS_EN_PIN, HIGH);

  Serial.begin(115200);
  Pixel::init(&context);
  Compass::init(&context);
  GPS::init(&context);

  button.attachClick(
      [](void *scope) {
        switch (context.deviceState) {
          case CompassState::STATE_COMPASS: {
            if (context.deviceType == CompassType::LocationCompass) {
              context.deviceType = CompassType::SouthCompass;
            } else {
              context.deviceType = CompassType::LocationCompass;
            }
            ESP_LOGI(TAG, "Toggle Compass Type to %s",
                     context.deviceType == CompassType::LocationCompass
                         ? "LocationCompass"
                         : "SouthCompass");
            break;
          }

          default:
            break;
        }
      },
      &button);
  button.attachLongPressStart(
      [](void *scope) {
        switch (context.deviceState) {
          case CompassState::STATE_COMPASS: {
            if (context.deviceType == CompassType::LocationCompass) {
              // 设置当前地点为Home
              // 检查GPS状态
              if (context.currentLoc.latitude < 500.0f) {
                Preference::saveHomeLocation(context.currentLoc);
                memcpy(&context.targetLoc, &context.currentLoc,
                       sizeof(Location));
                ESP_LOGI(TAG, "Set Home");
              } else {
                ESP_LOGI(TAG, "Can't set home, no GPS data.");
              }
            } else {
              // 指南针模式下长按切换到theNether
              context.forceTheNether = !context.forceTheNether;
            }
            break;
          }
          case CompassState::STATE_CONNECT_WIFI: {
            ESP_LOGW(TAG, "Clear WiFi");
            // 清空WiFi配置
            wifi_config_t config;
            esp_wifi_set_config(WIFI_IF_STA, &config);
            delay(3000);
            esp_restart();
          }

          default:
            break;
        }
      },
      &button);

  bool useWiFi = false;
  Preference::getWebServerConfig(useWiFi);
  if (useWiFi) {
    CompassServer::init(&context);
  } else {
    CompassBLE::init(&context);
  }
  context.deviceState = STATE_COMPASS;

  // 获取目标位置
  Preference::getHomeLocation(context.targetLoc);
  ESP_LOGI(TAG, "target Location:%f,%f ", context.targetLoc.latitude,
           context.targetLoc.longitude);
  return &context;
}

void Board::buttonTask(void *pvParameters) {
  while (1) {
    button.tick();
    delay(10);
  }
}