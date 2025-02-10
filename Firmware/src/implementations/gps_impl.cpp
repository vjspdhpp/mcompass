#include <Arduino.h>
#include <TinyGPSPlus.h>

#include "func.h"
#include "utils.h"
static HardwareSerial GPSSerial(0);
static TinyGPSPlus tinygps;

static const char *TAG = "GPS";
// GPS休眠配置表
static const mcompass::SleepConfig sleepConfigs[] = {
    {10.0f, 0, true},          // 在10KM距离内，不休眠
    {50.0f, 5 * 60, false},    // 超过50KM，休眠5分钟
    {100.0f, 10 * 60, false},  // 超过100KM，休眠10分钟
    {200.0f, 15 * 60, false},  // 超过200KM，休眠15分钟
};

// GPS休眠时间
static uint32_t gpsSleepInterval = 60 * 60;  // 单位:秒

void gps::init(mcompass::Context *context) {
  // 配置GPS串口
  GPSSerial.begin(9600, SERIAL_8N1, RX, TX);
  // 设置串口缓冲区大小
  GPSSerial.setRxBufferSize(1024);
  // 启动GPS,用于GPS存在性检测
  digitalWrite(GPS_EN_PIN, LOW);
  // 创建GPS串口数据读取Timer
  // TODO 或许可以参考NMEA Parser Example优化此处
  esp_timer_handle_t gps_timer;
  esp_timer_create_args_t timer_args = {
      .callback =
          [](void *arg) {
            while (GPSSerial.available() > 0) {
              char t = GPSSerial.read();
              if (tinygps.encode(t)) {
                // 有串口数据, 说明可能接了GPS
                auto context = static_cast<mcompass::Context *>(arg);
                context->setDetectGPS(true);
                // 有效的GPS编码数据
                if (!tinygps.location.isValid()) {
                  return;
                }
                ESP_LOGD(TAG, "Location:  %f, %f", tinygps.location.lat(),
                         tinygps.location.lng());
                mcompass::Location lastestLocation;
                lastestLocation.latitude =
                    static_cast<float>(tinygps.location.lat());
                lastestLocation.longitude =
                    static_cast<float>(tinygps.location.lng());
                // 坐标有效情况下更新本地坐标
                context->setCurrentLoc(lastestLocation);
                // 计算两地距离
                auto currentLoc = context->getCurrentLoc();
                auto targetLoc = context->getTargetLoc();
                double distance = utils::complexDistance(
                    currentLoc.latitude, currentLoc.longitude,
                    targetLoc.latitude, targetLoc.longitude);
                ESP_LOGI(TAG, "%f km to target.\n", distance);
                // 获取最接近的临界值
                float threshholdDistance = 0;
                size_t sleepConfigSize =
                    sizeof(sleepConfigs) / sizeof(mcompass::SleepConfig);
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
                    if (sleepConfigs[i].gpsPowerEn) {
                      digitalWrite(GPS_EN_PIN, LOW);
                    } else {
                      digitalWrite(GPS_EN_PIN, HIGH);
                    }
                    ESP_LOGI(TAG, "GPS Sleep %d seconds\n", gpsSleepInterval);
                    break;
                  }
                }
              } else {
                ESP_LOGD(TAG, "INVALID GPS DATA");
              }

              if (tinygps.date.isValid()) {
                ESP_LOGD(TAG, "Date:  %d/%d/%d", tinygps.date.year(),
                         tinygps.date.month(), tinygps.date.day());
              }
              if (tinygps.time.isValid()) {
                ESP_LOGD(TAG, "Time:  %d/%d/%d.%d", tinygps.time.hour(),
                         tinygps.time.minute(), tinygps.time.second(),
                         tinygps.time.centisecond());
              }
              if (gpsSleepInterval == 0) {
                digitalWrite(GPS_EN_PIN, LOW);
                gpsSleepInterval = 60 * 60;
              } else {
                gpsSleepInterval--;
              }
            }
          },
      .arg = context,
      .name = "gps_timer"};

  ESP_ERROR_CHECK(esp_timer_create(&timer_args, &gps_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(
      gps_timer, 1000000));  // 1 second in microseconds
}

/**
 * @brief GPS 关闭
 */
void gps::disable() { digitalWrite(GPS_EN_PIN, HIGH); }

bool gps::isValidGPSLocation(mcompass::Location location) {
  if (location.latitude >= -90 && location.latitude <= 90 &&
      location.longitude >= -180 && location.longitude <= 180) {
    return true;
  }
  return false;
}
