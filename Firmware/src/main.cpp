#include <FastLED.h>
#include <NimBLEDevice.h>
#include <Preferences.h>
#include <QMC5883LCompass.h>
#include <esp_log.h>
#include <esp_task_wdt.h>

#include "func.h"

static const char *TAG = "MAIN";
static Context *context;

void setup() {
  // 延时,用于一些特殊情况下能够重新烧录
  delay(2000);
  // 初始化硬件相关, 返回Context
  context = Board::init();
  // 显示指针启动动画, 顺便检测要不要校准, 由于locationTask已经开始执行,
  // 如果在bootAnimation的1.5秒内GPS有数据,视作有GPS, context->hasGPS被置为true
  Pixel::bootAnimation(Board::calibrateCheck);
  // 创建位置任务
  xTaskCreate(GPS::locationTask, "locationTask", 4096, context, 6,
              &context->gpsTaskHandle);
  ESP_LOGW(TAG, "xTaskCreate done.locationTask");
  // 创建显示任务
  xTaskCreate(Board::buttonTask, "buttonTask", 4096, context, 6, NULL);
  ESP_LOGW(TAG, "xTaskCreate done.button");
  xTaskCreate(Pixel::pixelTask, "pixelTask", 4096, context, 1, NULL);
  ESP_LOGW(TAG, "xTaskCreate done.pixelTask %p", context);
  if (!context->hasGPS) {
    // 没有GPS的话会默认进入指南模式
    context->deviceType = CompassType::SouthCompass;
    ESP_LOGW(TAG, "GPS Module Not Found!");
  }
  // 按需校准
  if (context->deviceState == CompassState::STATE_CALIBRATE) {
    Compass::calibrateCompass();
  }
  // 校准结束
  context->deviceState = STATE_COMPASS;
  ESP_LOGW(TAG, "setup done!");
}

void loop() {
  delay(1000);
  // 服务自身决定是否需要真的关闭
  if (context->useWiFi) {
    // 关闭本地网页服务
    CompassServer::endWebServer();
  } else {
    CompassBLE::disable();
  }
  // 启动后60秒内没有检测到GPS模块, 关闭GPS的TASK
  if (millis() < DEFAULT_GPS_DETECT_TIMEOUT || context->hasGPS) {
    return;
  }
  if (context->gpsTaskHandle != NULL) {
    ESP_LOGW(TAG, "Delete GPS Task");
    vTaskDelete(context->gpsTaskHandle);
    context->gpsTaskHandle = NULL;
  }
}
