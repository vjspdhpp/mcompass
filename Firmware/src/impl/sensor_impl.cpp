#include <Arduino.h>
#include <QMC5883LCompass.h>
#include <math.h>

#include "board.h"

using namespace mcompass;
static const char *TAG = "Compass";
QMC5883LCompass qmc5883;

static bool compassAvailable = false;

void sensor::init(Context *context) {
#ifdef CONFIG_IDF_TARGET_ESP32C3
  qmc5883.init();
  uint8_t chipID = qmc5883.chipID();
  ESP_LOGW(TAG, "Chip ID =%x", chipID);
#elif CONFIG_IDF_TARGET_ESP32S3
  uint8_t chipID = 0xff;
  ESP_LOGW(TAG, "Skip compass init on ESP32S3");
#endif
  if (chipID == 0xff) {
    compassAvailable = true;
    context->setHasSensor(true);
  }
}

void sensor::calibrateCompass() {
  if (!compassAvailable) return;
  qmc5883.calibrate();
  ESP_LOGW(TAG, "setCalibrationOffsets(%f, %f,%f)",
           qmc5883.getCalibrationOffset(0), qmc5883.getCalibrationOffset(1),
           qmc5883.getCalibrationOffset(2));
  ESP_LOGW(TAG, "setCalibrationScales(%f, %f,%f)",
           qmc5883.getCalibrationScale(0), qmc5883.getCalibrationScale(1),
           qmc5883.getCalibrationScale(2));
  qmc5883.setCalibrationOffsets(qmc5883.getCalibrationOffset(0),
                                qmc5883.getCalibrationOffset(1),
                                qmc5883.getCalibrationOffset(2));
  qmc5883.setCalibrationScales(qmc5883.getCalibrationScale(0),
                               qmc5883.getCalibrationScale(1),
                               qmc5883.getCalibrationScale(2));
}

/**
 * @brief 获取当前方位角
 */

int sensor::getAzimuth() {
  if (!compassAvailable) return 0;
#ifdef CONFIG_IDF_TARGET_ESP32C3
  int azimuth = qmc5883.getAzimuth();
  if (azimuth < 0) {
    azimuth += 360;
  }
#elif CONFIG_IDF_TARGET_ESP32S3
  int azimuth = random(0, 360);
#endif
  return azimuth;
}

bool sensor::isCompassAvailable() { return compassAvailable; }