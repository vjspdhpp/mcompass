#include <Arduino.h>
#include <TinyGPSPlus.h>

#include "func.h"
static HardwareSerial GPSSerial(0);
static TinyGPSPlus gps;

static const char *TAG = "GPS";
// GPS休眠配置表
static const SleepConfig sleepConfigs[] = {
    {10.0f, 0, true},         // 在10KM距离内，不休眠
    {50.0f, 5 * 60, false},   // 超过50KM，休眠5分钟
    {100.0f, 10 * 60, false}, // 超过100KM，休眠10分钟
    {200.0f, 15 * 60, false}, // 超过200KM，休眠15分钟
};

// GPS休眠时间
static uint32_t gpsSleepInterval = 60 * 60; // 单位:秒

void GPS::init(Context *context) {
  // 配置GPS串口
  GPSSerial.begin(9600, SERIAL_8N1, RX, TX);
  // 启动GPS,用于GPS存在性检测
  digitalWrite(GPS_EN_PIN, LOW);
}

/**
 * @brief GPS 关闭
 */
void GPS::disable() { digitalWrite(GPS_EN_PIN, HIGH); }

/**
 * @brief 位置任务
 *
 */
void GPS::locationTask(void *pvParameters) {
  Context *context = (Context *)pvParameters;
  // esp_task_wdt_init(30, false);
  while (1) {
    while (GPSSerial.available() > 0) {
      char t = GPSSerial.read();
      if (gps.encode(t)) {
        // 有串口数据, 说明可能接了GPS
        context->hasGPS = true;
        // 有效的GPS编码数据
        if (gps.location.isValid()) {
          ESP_LOGD(TAG, "Location:  %f, %f", gps.location.lat(),
                   gps.location.lng());
          // 坐标有效情况下更新本地坐标
          context->currentLoc.latitude = static_cast<float>(gps.location.lat());
          context->currentLoc.longitude =
              static_cast<float>(gps.location.lng());
          // 计算两地距离
          double distance = Compass::complexDistance(
              context->currentLoc.latitude, context->currentLoc.longitude,
              context->targetLoc.latitude, context->targetLoc.longitude);
          ESP_LOGI(TAG, "%f km to target.\n", distance);
          // 获取最接近的临界值
          float threshholdDistance = 0;
          size_t sleepConfigSize = sizeof(sleepConfigs) / sizeof(SleepConfig);
          for (int i = sleepConfigSize - 1; i >= 0; i--) {
            if (distance >= sleepConfigs[i].distanceThreshold) {
              threshholdDistance = sleepConfigs[i].distanceThreshold;
              ESP_LOGI(TAG, "use threshold %f km", threshholdDistance);
              break;
            }
          }
          float modDistance = fmod(distance, threshholdDistance);
          // 根据距离调整GPS休眠时间,
          for (int i = 0; i < sleepConfigSize; i++) {
            if (modDistance <= sleepConfigs[i].distanceThreshold) {
              gpsSleepInterval = sleepConfigs[i].sleepInterval;
              if (sleepConfigs[i].gpsEnable) {
                digitalWrite(GPS_EN_PIN, LOW);
              } else {
                digitalWrite(GPS_EN_PIN, HIGH);
              }
              ESP_LOGI(TAG, "GPS Sleep %d seconds\n", gpsSleepInterval);
              break;
            }
          }
        }
      } else {
        ESP_LOGD(TAG, "INVALID GPS DATA");
      }

      if (gps.date.isValid()) {
        ESP_LOGD(TAG, "Date:  %d/%d/%d", gps.date.year(), gps.date.month(),
                 gps.date.day());
      }
      if (gps.time.isValid()) {
        ESP_LOGD(TAG, "Time:  %d/%d/%d.%d", gps.time.hour(), gps.time.minute(),
                 gps.time.second(), gps.time.centisecond());
      }
      if (gpsSleepInterval == 0) {
        digitalWrite(GPS_EN_PIN, LOW);
        gpsSleepInterval = 60 * 60;
      } else {
        gpsSleepInterval--;
      }
    }
    delay(1000);
  }
  delay(1000);
}
