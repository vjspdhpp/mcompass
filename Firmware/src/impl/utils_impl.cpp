#include <Arduino.h>
#include <soc/usb_serial_jtag_reg.h>

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "common.h"
#include "macro_def.h"
#include "utils.h"
std::string utils::toHexString(int spawnColor) {
  // 提取RGB分量
  int red = (spawnColor >> 16) & 0xFF;
  int green = (spawnColor >> 8) & 0xFF;
  int blue = spawnColor & 0xFF;

  // 预分配足够的空间
  std::string result(7, '#');

  // 将RGB分量转换为16进制字符串
  snprintf(&result[1], 7, "%02X%02X%02X", red, green, blue);

  return result;
}

int utils::fromHexString(const std::string &hexColor) {
  // 去掉开头的 '#'（如果有）
  std::string hex = hexColor;
  if (!hex.empty() && hex[0] == '#') {
    hex.erase(hex.begin());
  }

  // 确保字符串长度是 6（RRGGBB）
  if (hex.length() != 6) {
    return 0;
  }

  // 将字符串解析为整数
  unsigned int colorValue;
  std::stringstream ss;
  ss << std::hex << hex; // 将 16 进制字符串转换为整数
  ss >> colorValue;

  return static_cast<int>(colorValue);
}

bool utils::isPluggedUSB(void) {
  uint32_t *aa = (uint32_t *)USB_SERIAL_JTAG_FRAM_NUM_REG;
  uint32_t first = *aa;
  delay(10);
  return (*aa - first) != 0;
}

std::vector<std::string> utils::split(std::string &s,
                                      const std::string &delimiter) {
  std::vector<std::string> tokens;
  size_t pos = 0;
  std::string token;
  while ((pos = s.find(delimiter)) != std::string::npos) {
    token = s.substr(0, pos);
    tokens.push_back(token);
    s.erase(0, pos + delimiter.length());
  }
  tokens.push_back(s);

  return tokens;
}

/// https://johnnyqian.net/blog/gps-locator.html

// 将角度转换为弧度
static double toRadians(double degrees) { return degrees * PI / 180.0; }

// 计算两点之间的方位角
double utils::calculateBearing(double lat1, double lon1, double lat2,
                               double lon2) {
  double radLat1 = toRadians(lat1);
  double radLat2 = toRadians(lat2);
  double radLon1 = toRadians(lon1);
  double radLon2 = toRadians(lon2);

  double deltaLon = radLon2 - radLon1;

  double numerator = sin(deltaLon) * cos(radLat2);
  double denominator =
      cos(radLat1) * sin(radLat2) - sin(radLat1) * cos(radLat2) * cos(deltaLon);

  double x = atan2(fabs(numerator), fabs(denominator));
  double result = x;

  if (lon2 > lon1) { // 右半球
    if (lat2 > lat1) // 第一象限
      result = x;
    else if (lat2 < lat1) // 第四象限
      result = PI - x;
    else
      result = PI / 2;      // x轴正方向
  } else if (lon2 < lon1) { // 左半球
    if (lat2 > lat1)        // 第二象限
      result = 2 * PI - x;
    else if (lat2 < lat1) // 第三象限
      result = PI + x;
    else
      result = 3 * PI / 2; // x轴负方向
  } else {                 // 相同经度
    if (lat2 > lat1)       // y轴正方向
      result = 0;
    else if (lat2 < lat1) // y轴负方向
      result = PI;
    else {
      ESP_LOGW("Utils", "Warning Arriving at the destination Arriving.");
      result = 0;
    }
  }

  return result * 180.0 / PI;
}

// 使用Haversine公式计算两点间的球面距离
double utils::complexDistance(double lat1, double lon1, double lat2,
                              double lon2) {
  double dLat = toRadians(lat2 - lat1);
  double dLon = toRadians(lon2 - lon1);

  double havLat = sin(dLat / 2);
  double havLon = sin(dLon / 2);

  double a = havLat * havLat +
             cos(toRadians(lat1)) * cos(toRadians(lat2)) * havLon * havLon;

  return 2 * EARTH_RADIUS * atan2(sqrt(a), sqrt(1 - a));
}

double utils::simplifiedDistance(double lat1, double lon1, double lat2,
                                 double lon2) {
  double avgLat = toRadians(lat1 + lat2) / 2.0;
  double disLat = EARTH_RADIUS * cos(avgLat) * toRadians(lon1 - lon2);
  double disLon = EARTH_RADIUS * toRadians(lat1 - lat2);

  return sqrt(disLat * disLat + disLon * disLon);
}

std::string utils::workType2Str(mcompass::WorkType workType) {
  std::string workTypeStr;
  switch (workType) {
  case mcompass::WorkType::SPAWN:
    workTypeStr = "Spawn";
    break;
  case mcompass::WorkType::SOUTH:
    workTypeStr = "South";
    break;
  default:
    workTypeStr = "Unknown";
    break;
  }
  return workTypeStr.c_str();
}

std::string utils::sensorModel2Str(mcompass::SensorModel model) {
  std::string sensorModel2Str;
  switch (model) {
  case mcompass::SensorModel::QMC5883L:
    sensorModel2Str = "QMC5883L";
    break;
  case mcompass::SensorModel::QMC5883P:
    sensorModel2Str = "QMC5883P";
    break;
  case mcompass::SensorModel::MMC5883MA:
    sensorModel2Str = "MMC5883MA";
    break;
  default:
    sensorModel2Str = "Unknown";
  }
  return sensorModel2Str;
}