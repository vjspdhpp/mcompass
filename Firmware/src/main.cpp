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
  // 创建显示任务
  xTaskCreate(Pixel::pixelTask, "pixelTask", 4096, NULL, 2, NULL);
  // 创建位置任务
  xTaskCreate(GPS::locationTask, "locationTask", 4096, NULL, 2,
              &context->gpsTaskHandle);
  xTaskCreate(Board::buttonTask, "buttonTask", 4096, NULL, 2, NULL);
  // 显示指针启动动画, 顺便检测要不要校准, locationTask已经开始执行,
  // 如果在bootAnimation的1.5秒内GPS有数据,视作有GPS,context->hasGPS被置为true
  Pixel::bootAnimation(Board::calibrateCheck);
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
  context->deviceState = context->hasGPS ? STATE_WAIT_GPS : STATE_COMPASS;
}

void loop() {
  delay(1000);
  // 启动后120s未检测到任何连接,关闭热点或断开WiFi连接
  if (millis() > DEFAULT_SERVER_TIMEOUT && CompassServer::shouldStopServer()) {
    // 关闭本地网页服务
    CompassServer::endWebServer();
  }
  // 启动后60秒内没有检测到GPS模块, 关闭GPS的TASK
  if (millis() > DEFAULT_GPS_DETECT_TIMEOUT && !context->hasGPS) {
    if (context->gpsTaskHandle != NULL) {
      ESP_LOGW(TAG, "Delete GPS Task");
      vTaskDelete(context->gpsTaskHandle);
      context->gpsTaskHandle = NULL;
    }
  }
}
