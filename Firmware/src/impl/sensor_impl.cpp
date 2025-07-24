#include "board.h"
#include <Arduino.h>
#include <math.h>

using namespace mcompass;
static const char *TAG = "Compass";

// 根据 DEFAULT_SENSOR_MODEL 的值来选择包含哪个驱动文件
#if DEFAULT_SENSOR_MODEL == SENSOR_MODEL_QMC5883L
#include "QMC5883LCompass.h"
typedef QMC5883LCompass Compass; // 创建统一别名
#define CHIP_ID 0xFF             // QMC5883L 的芯片IDx
#elif DEFAULT_SENSOR_MODEL == SENSOR_MODEL_QMC5883P
#include "QMC5883PCompass.h"
typedef QMC5883PCompass Compass; // 创建统一别名
#define CHIP_ID 0x80 // QMC5883P 的芯片ID
#else
#error "Invalid or unknown DEFAULT_SENSOR_MODEL specified."
#endif

Compass qmc5883; // 创建传感器对象

static bool compassAvailable = false;

void sensor::init(Context *context) {
#ifdef CONFIG_IDF_TARGET_ESP32C3
  qmc5883.init();
  uint8_t chipID = qmc5883.chipID();
  ESP_LOGW(TAG, "Chip ID =%x", chipID);
#elif CONFIG_IDF_TARGET_ESP32S3
  // 用来测试的M5Stack Cardputer 没有QMC5883L芯片,所以不进行初始化
  uint8_t chipID = CHIP_ID;
  ESP_LOGW(TAG, "Skip compass init on ESP32S3");
#endif
  if (chipID == CHIP_ID) {
    compassAvailable = true;
    context->setHasSensor(true);
  } else {
    ESP_LOGE(TAG, "Sensor init failed");
    context->setHasSensor(false);
    context->setDeviceState(State::INFO);
    Event::Body event;
    event.type = Event::Type::TEXT;
    event.source = Event::Source::SENSOR;
    memcpy(event.TEXT.text, SENSOR_ERROR, sizeof(SENSOR_ERROR));
    ESP_ERROR_CHECK(esp_event_post_to(context->getEventLoop(), MCOMPASS_EVENT,
                                      0, &event, sizeof(event), 0));
    return;
  }
  // 还原传感器的校准数据
  preference::CalibrationData data = preference::getCalibration();
  if (data.offsets[0] != 0 || data.offsets[1] != 0 || data.offsets[2] != 0 ||
      data.scales[0] != 0 || data.scales[1] != 0 || data.scales[2] != 0) {
    ESP_LOGW(TAG, "restore CalibrationOffsets(%f, %f,%f)", data.offsets[0],
             data.offsets[1], data.offsets[2]);
    ESP_LOGW(TAG, "restore CalibrationScales(%f, %f,%f)", data.scales[0],
             data.scales[1], data.scales[2]);
    qmc5883.setCalibrationOffsets(data.offsets[0], data.offsets[1],
                                  data.offsets[2]);
    qmc5883.setCalibrationScales(data.scales[0], data.scales[1],
                                 data.scales[2]);
  }
}

void sensor::calibrate() {
  if (!compassAvailable)
    return;
  qmc5883.calibrate();
  ESP_LOGW(TAG, "setCalibrationOffsets(%f, %f,%f)",
           qmc5883.getCalibrationOffset(0), qmc5883.getCalibrationOffset(1),
           qmc5883.getCalibrationOffset(2));
  ESP_LOGW(TAG, "setCalibrationScales(%f, %f,%f)",
           qmc5883.getCalibrationScale(0), qmc5883.getCalibrationScale(1),
           qmc5883.getCalibrationScale(2));
  preference::CalibrationData data;
  data.offsets[0] = qmc5883.getCalibrationOffset(0);
  data.offsets[1] = qmc5883.getCalibrationOffset(1);
  data.offsets[2] = qmc5883.getCalibrationOffset(2);
  data.scales[0] = qmc5883.getCalibrationScale(0);
  data.scales[1] = qmc5883.getCalibrationScale(1);
  data.scales[2] = qmc5883.getCalibrationScale(2);
  preference::setCalibration(data);
  ESP_LOGW(TAG, "Calibration data saved to preferences");
}

/**
 * @brief 获取当前方位角
 */

int sensor::getAzimuth() {
  if (!sensor::available()) {
    return 0;
  }
#ifdef CONFIG_IDF_TARGET_ESP32C3
  qmc5883.read();
  int azimuth = qmc5883.getAzimuth();
  if (azimuth < 0) {
    azimuth += 360;
  }
#elif CONFIG_IDF_TARGET_ESP32S3
  static int azimuth = 0;
  azimuth++;
  azimuth %= 360;
#endif

  // QMC5883P的坐标系和QMC5883L不同,需要进行调整
  #if DEFAULT_SENSOR_MODEL == SENSOR_MODEL_QMC5883P
  azimuth = azimuth + 90;
  if (azimuth > 360) {
    azimuth -= 360;
  }
  #endif

  return azimuth;
}

bool sensor::available() { return compassAvailable; }
