#include <Arduino.h>
#include <QMC5883LCompass.h>
#include <math.h>

#include "func.h"

static const char *TAG = "Compass";
QMC5883LCompass qmc5883;

static bool compassAvailable = false;

void sensor::init(mcompass::Context *context) {
  qmc5883.init();
  uint8_t chipID = qmc5883.chipID();
  ESP_LOGW(TAG, "Chip ID =%x", chipID);
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
  qmc5883.read();
  int azimuth = qmc5883.getAzimuth();
  if (azimuth < 0) {
    azimuth += 360;
  }
  return azimuth;
}

bool sensor::isCompassAvailable() { return compassAvailable; }