#include <QMC5883LCompass.h>
#include <math.h>

#include "func.h"


#define EARTH_RADIUS 6371.0  // 地球半径（单位：公里）

QMC5883LCompass compass;

static bool compassAvailable = false;

void Compass::calibrateCompass() {
  if (!compassAvailable) return;
  compass.calibrate();
  Serial.println();
  Serial.print("compass.setCalibrationOffsets(");
  Serial.print(compass.getCalibrationOffset(0));
  Serial.print(", ");
  Serial.print(compass.getCalibrationOffset(1));
  Serial.print(", ");
  Serial.print(compass.getCalibrationOffset(2));
  Serial.println(");");
  Serial.print("compass.setCalibrationScales(");
  Serial.print(compass.getCalibrationScale(0));
  Serial.print(", ");
  Serial.print(compass.getCalibrationScale(1));
  Serial.print(", ");
  Serial.print(compass.getCalibrationScale(2));
  Serial.println(");");
  compass.setCalibrationOffsets(compass.getCalibrationOffset(0),
                                compass.getCalibrationOffset(1),
                                compass.getCalibrationOffset(2));
  compass.setCalibrationScales(compass.getCalibrationScale(0),
                               compass.getCalibrationScale(1),
                               compass.getCalibrationScale(2));
}

/// https://johnnyqian.net/blog/gps-locator.html

// 将角度转换为弧度
static double toRadians(double degrees) { return degrees * PI / 180.0; }

// 计算两点之间的方位角
double Compass::calculateBearing(double lat1, double lon1, double lat2, double lon2) {
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

  if (lon2 > lon1) {  // 右半球
    if (lat2 > lat1)  // 第一象限
      result = x;
    else if (lat2 < lat1)  // 第四象限
      result = PI - x;
    else
      result = PI / 2;       // x轴正方向
  } else if (lon2 < lon1) {  // 左半球
    if (lat2 > lat1)         // 第二象限
      result = 2 * PI - x;
    else if (lat2 < lat1)  // 第三象限
      result = PI + x;
    else
      result = 3 * PI / 2;  // x轴负方向
  } else {                  // 相同经度
    if (lat2 > lat1)        // y轴正方向
      result = 0;
    else if (lat2 < lat1)  // y轴负方向
      result = PI;
    else {
      fprintf(stderr, "Error: 两点不能是同一个位置！\n");
      exit(EXIT_FAILURE);
    }
  }

  return result * 180.0 / PI;
}

// 使用Haversine公式计算两点间的球面距离
double Compass::complexDistance(double lat1, double lon1, double lat2, double lon2) {
  double dLat = toRadians(lat2 - lat1);
  double dLon = toRadians(lon2 - lon1);

  double havLat = sin(dLat / 2);
  double havLon = sin(dLon / 2);

  double a = havLat * havLat +
             cos(toRadians(lat1)) * cos(toRadians(lat2)) * havLon * havLon;

  return 2 * EARTH_RADIUS * atan2(sqrt(a), sqrt(1 - a));
}

double Compass::simplifiedDistance(double lat1, double lon1, double lat2, double lon2) {
  double avgLat = toRadians(lat1 + lat2) / 2.0;
  double disLat = EARTH_RADIUS * cos(avgLat) * toRadians(lon1 - lon2);
  double disLon = EARTH_RADIUS * toRadians(lat1 - lat2);

  return sqrt(disLat * disLat + disLon * disLon);
}

/**
 * @brief 获取当前方位角
 */

int Compass::getAzimuth() {
  if (!compassAvailable) return 0;
  compass.read();
  return compass.getAzimuth();
}

bool Compass::isCompassAvailable() { return compassAvailable; }