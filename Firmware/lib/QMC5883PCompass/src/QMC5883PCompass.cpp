/*
 * QMC5883PCompass.cpp – 添加“轴重映射/取反”以适配板级坐标
 */

#include "QMC5883PCompass.h"
#include "Arduino.h"
#include <Wire.h>

QMC5883PCompass::QMC5883PCompass() {}

/**
 * INIT：初始化
 */
void QMC5883PCompass::init() {
  Wire.begin();

  // 可选：根据数据手册的应用示例，设定“XYZ 符号/方向”寄存器
  // 若后续采用软件重映射，这里保持 0x06 作为缺省即可
  _writeReg(0x29, 0x06);

  // 0x0B: 量程=8G(0x08) + Set/Reset on(0x01)
  _writeReg(0x0B, 0x08 | 0x01);

  // 0x0A: 连续模式(0x03) + ODR=200Hz(0x0C) + OSR1=8(0x00)
  _writeReg(0x0A, 0x03 | 0x0C | 0x00);
}

void QMC5883PCompass::setADDR(byte b) { _ADDR = b; }

/** 读取芯片ID（QMC5883P 在 0x00） */
char QMC5883PCompass::chipID() {
  Wire.beginTransmission(_ADDR);
  Wire.write(0x00);
  int err = Wire.endTransmission();
  Wire.requestFrom(_ADDR, (byte)1);
  char buf[1] = {0};
  Wire.readBytes(buf, 1);
  return buf[0];
}

void QMC5883PCompass::_writeReg(byte r, byte v) {
  Wire.beginTransmission(_ADDR);
  Wire.write(r);
  Wire.write(v);
  Wire.endTransmission();
}

/** 设置模式（保持你原有语义） */
void QMC5883PCompass::setMode(byte mode, byte odr, byte rng, byte osr) {
  byte regA_val = mode | odr | osr;
  byte regB_val = rng | 0x01; // SET/RESET on
  _writeReg(0x0B, regB_val);
  _writeReg(0x0A, regA_val);
}

void QMC5883PCompass::setMagneticDeclination(int degrees, uint8_t minutes) {
  _magneticDeclinationDegrees = (float)degrees + (float)minutes / 60.0;
}

void QMC5883PCompass::setReset() { _writeReg(0x0B, 0x80); }

void QMC5883PCompass::setSmoothing(byte steps, bool adv) {
  _smoothUse = true;
  _smoothSteps = (steps > 10) ? 10 : steps;
  _smoothAdvanced = adv ? true : false;
}

void QMC5883PCompass::calibrate() {
  clearCalibration();
  long calibrationData[3][2] = {{65000, -65000}, {65000, -65000}, {65000, -65000}};

  // 预热
  read();
  long x = calibrationData[0][0] = calibrationData[0][1] = getX();
  long y = calibrationData[1][0] = calibrationData[1][1] = getY();
  long z = calibrationData[2][0] = calibrationData[2][1] = getZ();

  unsigned long startTime = millis();
  while ((millis() - startTime) < 10000) {
    read();
    x = getX(); y = getY(); z = getZ();
    if (x < calibrationData[0][0]) calibrationData[0][0] = x;
    if (x > calibrationData[0][1]) calibrationData[0][1] = x;
    if (y < calibrationData[1][0]) calibrationData[1][0] = y;
    if (y > calibrationData[1][1]) calibrationData[1][1] = y;
    if (z < calibrationData[2][0]) calibrationData[2][0] = z;
    if (z > calibrationData[2][1]) calibrationData[2][1] = z;
  }

  setCalibration(calibrationData[0][0], calibrationData[0][1],
                 calibrationData[1][0], calibrationData[1][1],
                 calibrationData[2][0], calibrationData[2][1]);
}

void QMC5883PCompass::setCalibration(int x_min, int x_max, int y_min, int y_max,
                                     int z_min, int z_max) {
  setCalibrationOffsets((x_min + x_max) / 2.0, (y_min + y_max) / 2.0, (z_min + z_max) / 2.0);

  float x_avg_delta = (x_max - x_min) / 2.0;
  float y_avg_delta = (y_max - y_min) / 2.0;
  float z_avg_delta = (z_max - z_min) / 2.0;
  float avg_delta   = (x_avg_delta + y_avg_delta + z_avg_delta) / 3.0;

  setCalibrationScales(avg_delta / x_avg_delta, avg_delta / y_avg_delta, avg_delta / z_avg_delta);
}

void QMC5883PCompass::setCalibrationOffsets(float x_offset, float y_offset, float z_offset) {
  _offset[0] = x_offset; _offset[1] = y_offset; _offset[2] = z_offset;
}

void QMC5883PCompass::setCalibrationScales(float x_scale, float y_scale, float z_scale) {
  _scale[0] = x_scale; _scale[1] = y_scale; _scale[2] = z_scale;
}

float QMC5883PCompass::getCalibrationOffset(uint8_t index) { return _offset[index]; }
float QMC5883PCompass::getCalibrationScale(uint8_t index)  { return _scale[index]; }

void QMC5883PCompass::clearCalibration() {
  setCalibrationOffsets(0., 0., 0.);
  setCalibrationScales(1., 1., 1.);
}

/**
 * READ：读芯片原始 XYZ，然后做“板级坐标重映射”与校准/平滑
 */
void QMC5883PCompass::read() {
  Wire.beginTransmission(_ADDR);
  Wire.write(0x01); // QMC5883P: X_LSB 从 0x01 开始
  int err = Wire.endTransmission();
  if (!err) {
    Wire.requestFrom(_ADDR, (byte)6);
    // 芯片原始坐标
    _vRaw[0] = (int)(int16_t)(Wire.read() | Wire.read() << 8); // chip X
    _vRaw[1] = (int)(int16_t)(Wire.read() | Wire.read() << 8); // chip Y
    _vRaw[2] = (int)(int16_t)(Wire.read() | Wire.read() << 8); // chip Z

    // --- 轴重映射：得到“板级坐标” ---
    _vMapped[0] = _map_sgn[0] * _vRaw[_map_src[0]]; // board X
    _vMapped[1] = _map_sgn[1] * _vRaw[_map_src[1]]; // board Y
    _vMapped[2] = _map_sgn[2] * _vRaw[_map_src[2]]; // board Z

    // 校准应用基于“板级坐标”
    _applyCalibration();

    if (_smoothUse) _smoothing();
  }
}

void QMC5883PCompass::_applyCalibration() {
  _vCalibrated[0] = (_vMapped[0] - _offset[0]) * _scale[0];
  _vCalibrated[1] = (_vMapped[1] - _offset[1]) * _scale[1];
  _vCalibrated[2] = (_vMapped[2] - _offset[2]) * _scale[2];
}

void QMC5883PCompass::_smoothing() {
  byte max = 0, min = 0;
  if (_vScan > _smoothSteps - 1) _vScan = 0;

  for (int i = 0; i < 3; i++) {
    if (_vTotals[i] != 0) _vTotals[i] -= _vHistory[_vScan][i];
    _vHistory[_vScan][i] = _vCalibrated[i];
    _vTotals[i] += _vHistory[_vScan][i];

    if (_smoothAdvanced) {
      max = 0; for (int j = 0; j < _smoothSteps - 1; j++) max = (_vHistory[j][i] > _vHistory[max][i]) ? j : max;
      min = 0; for (int k = 0; k < _smoothSteps - 1; k++) min = (_vHistory[k][i] < _vHistory[min][i]) ? k : min;
      _vSmooth[i] = (_vTotals[i] - (_vHistory[max][i] + _vHistory[min][i])) / (_smoothSteps - 2);
    } else {
      _vSmooth[i] = _vTotals[i] / _smoothSteps;
    }
  }
  _vScan++;
}

int QMC5883PCompass::getX() { return _get(0); }
int QMC5883PCompass::getY() { return _get(1); }
int QMC5883PCompass::getZ() { return _get(2); }

int QMC5883PCompass::_get(int i) {
  if (_smoothUse) return _vSmooth[i];
  return _vCalibrated[i];
}

int QMC5883PCompass::getAzimuth() {
  float heading = atan2(getY(), getX()) * 180.0 / PI; // 以“板级 X/Y”为基准
  heading += _magneticDeclinationDegrees;
  // 归一到 0~359
  while (heading < 0) heading += 360.0;
  while (heading >= 360.0) heading -= 360.0;
  return (int)heading;
}

byte QMC5883PCompass::getBearing(int azimuth) {
  float a = (azimuth + 11.25) / 22.5;
  byte bearing = (int)a % 16;
  return bearing;
}

void QMC5883PCompass::getDirection(char *myArray, int azimuth) {
  int d = getBearing(azimuth);
  myArray[0] = _bearings[d][0];
  myArray[1] = _bearings[d][1];
  myArray[2] = _bearings[d][2];
}

// --- 新增：设置轴重映射 ---
void QMC5883PCompass::setAxisRemap(uint8_t bx_src, int8_t bx_sgn,
                                    uint8_t by_src, int8_t by_sgn,
                                    uint8_t bz_src, int8_t bz_sgn) {
  _map_src[0] = bx_src; _map_sgn[0] = (bx_sgn >= 0) ? +1 : -1;
  _map_src[1] = by_src; _map_sgn[1] = (by_sgn >= 0) ? +1 : -1;
  _map_src[2] = bz_src; _map_sgn[2] = (bz_sgn >= 0) ? +1 : -1;
}
