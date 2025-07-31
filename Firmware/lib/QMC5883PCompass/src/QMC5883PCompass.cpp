/*
 * QMC5883PCompass.cpp
 * 修正：
 *  - 方位角采用“正北0°、顺时针”为正：heading = atan2(X, Y)
 *  - 新增 getDialAngle() = 360° - heading，供“转表盘、针固定”式UI使用
 *  - 保留轴重映射能力
 */

#include "QMC5883PCompass.h"
#include "Arduino.h"
#include <Wire.h>

QMC5883PCompass::QMC5883PCompass() {}

/** 初始化：开启 I2C，写入基础寄存器 */
void QMC5883PCompass::init() {
  Wire.begin();

  // 定义XYZ符号/方向寄存器（数据手册示例）。若完全靠软件重映射，也可忽略。
  _writeReg(0x29, 0x06);

  // 0x0B：量程与 Set/Reset (RNG=8G(0x08) + Set/Reset On(0x01))
  _writeReg(0x0B, 0x08 | 0x01);

  // 0x0A：模式/ODR/OSR1 (连续模式0x03 + 200Hz 0x0C + OSR1=8 0x00)
  _writeReg(0x0A, 0x03 | 0x0C | 0x00);
}

void QMC5883PCompass::setADDR(byte b) { _ADDR = b; }

/** 设置寄存器工具 */
void QMC5883PCompass::_writeReg(byte r, byte v) {
  Wire.beginTransmission(_ADDR);
  Wire.write(r);
  Wire.write(v);
  Wire.endTransmission();
}

/** 芯片ID（QMC5883P：0x00） */
char QMC5883PCompass::chipID() {
  Wire.beginTransmission(_ADDR);
  Wire.write(0x00);
  (void)Wire.endTransmission();
  Wire.requestFrom(_ADDR, (byte)1);
  char id = 0;
  if (Wire.available()) id = Wire.read();
  return id;
}

/** 模式设置（保持原接口语义） */
void QMC5883PCompass::setMode(byte mode, byte odr, byte rng, byte osr) {
  byte regA_val = mode | odr | osr;   // 0x0A: MODE/ODR/OSR1
  byte regB_val = rng | 0x01;         // 0x0B: RNG + Set/Reset On
  _writeReg(0x0B, regB_val);
  _writeReg(0x0A, regA_val);
}

/** 地磁偏角（度+分） */
void QMC5883PCompass::setMagneticDeclination(int degrees, uint8_t minutes) {
  _magneticDeclinationDegrees = (float)degrees + (float)minutes / 60.0f;
}

/** 软复位（寄存器法） */
void QMC5883PCompass::setReset() {
  _writeReg(0x0B, 0x80); // SOFT_RST
}

/** 平滑设置 */
void QMC5883PCompass::setSmoothing(byte steps, bool adv) {
  _smoothUse = true;
  _smoothSteps = (steps < 2) ? 2 : ((steps > 10) ? 10 : steps);
  _smoothAdvanced = adv;
}

/** 一键标定（旋转 10 秒） */
void QMC5883PCompass::calibrate() {
  clearCalibration();
  long range[3][2] = {{ 65000,-65000},{ 65000,-65000},{ 65000,-65000}};

  read(); // 触发一次
  long x = getX(), y = getY(), z = getZ();
  range[0][0] = range[0][1] = x;
  range[1][0] = range[1][1] = y;
  range[2][0] = range[2][1] = z;

  unsigned long t0 = millis();
  while (millis() - t0 < 10000) {
    read();
    x = getX(); y = getY(); z = getZ();
    if (x < range[0][0]) range[0][0] = x;
    if (x > range[0][1]) range[0][1] = x;
    if (y < range[1][0]) range[1][0] = y;
    if (y > range[1][1]) range[1][1] = y;
    if (z < range[2][0]) range[2][0] = z;
    if (z > range[2][1]) range[2][1] = z;
  }

  setCalibration(range[0][0], range[0][1],
                 range[1][0], range[1][1],
                 range[2][0], range[2][1]);
}

void QMC5883PCompass::setCalibration(int x_min, int x_max,
                                     int y_min, int y_max,
                                     int z_min, int z_max) {
  setCalibrationOffsets((x_min + x_max) / 2.0f,
                        (y_min + y_max) / 2.0f,
                        (z_min + z_max) / 2.0f);

  float x_delta = (x_max - x_min) / 2.0f;
  float y_delta = (y_max - y_min) / 2.0f;
  float z_delta = (z_max - z_min) / 2.0f;
  float avg     = (x_delta + y_delta + z_delta) / 3.0f;

  setCalibrationScales(avg / x_delta, avg / y_delta, avg / z_delta);
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
  setCalibrationOffsets(0.f, 0.f, 0.f);
  setCalibrationScales(1.f, 1.f, 1.f);
}

/** 读取一次（芯片XYZ -> 板级XYZ -> 校准 -> 平滑） */
void QMC5883PCompass::read() {
  // QMC5883P：数据从 0x01 开始（X LSB..Z MSB）
  Wire.beginTransmission(_ADDR);
  Wire.write(0x01);
  if (Wire.endTransmission()) return;

  Wire.requestFrom(_ADDR, (byte)6);
  if (Wire.available() < 6) return;

  // 芯片原始坐标
  _vRaw[0] = (int)(int16_t)(Wire.read() | (Wire.read() << 8)); // chip X
  _vRaw[1] = (int)(int16_t)(Wire.read() | (Wire.read() << 8)); // chip Y
  _vRaw[2] = (int)(int16_t)(Wire.read() | (Wire.read() << 8)); // chip Z

  // --- 轴重映射：得到“板级坐标” ---
  _vMapped[0] = _map_sgn[0] * _vRaw[_map_src[0]]; // board X
  _vMapped[1] = _map_sgn[1] * _vRaw[_map_src[1]]; // board Y
  _vMapped[2] = _map_sgn[2] * _vRaw[_map_src[2]]; // board Z

  // 应用校准（板级坐标）
  _applyCalibration();

  // 可选平滑
  if (_smoothUse) _smoothing();
}

/** 应用硬铁/软铁校准 */
void QMC5883PCompass::_applyCalibration() {
  _vCalibrated[0] = (int)((_vMapped[0] - _offset[0]) * _scale[0]);
  _vCalibrated[1] = (int)((_vMapped[1] - _offset[1]) * _scale[1]);
  _vCalibrated[2] = (int)((_vMapped[2] - _offset[2]) * _scale[2]);
}

/** 平滑滤波（支持“剔除极值”的高级模式） */
void QMC5883PCompass::_smoothing() {
  if (_vScan > _smoothSteps - 1) _vScan = 0;

  for (int i = 0; i < 3; i++) {
    if (_vTotals[i] != 0) _vTotals[i] -= _vHistory[_vScan][i];
    _vHistory[_vScan][i] = _vCalibrated[i];
    _vTotals[i] += _vHistory[_vScan][i];

    if (_smoothAdvanced && _smoothSteps >= 3) {
      byte maxIdx = 0, minIdx = 0;
      for (int j = 1; j < _smoothSteps; j++) {
        if (_vHistory[j][i] > _vHistory[maxIdx][i]) maxIdx = j;
        if (_vHistory[j][i] < _vHistory[minIdx][i]) minIdx = j;
      }
      int sum = _vTotals[i] - (_vHistory[maxIdx][i] + _vHistory[minIdx][i]);
      _vSmooth[i] = sum / (int)(_smoothSteps - 2);
    } else {
      _vSmooth[i] = _vTotals[i] / (int)_smoothSteps;
    }
  }
  _vScan++;
}

int QMC5883PCompass::getX() { return _get(0); }
int QMC5883PCompass::getY() { return _get(1); }
int QMC5883PCompass::getZ() { return _get(2); }

int QMC5883PCompass::_get(int i) {
  return _smoothUse ? _vSmooth[i] : _vCalibrated[i];
}

/** 返回导航角：正北0°、顺时针（含磁偏角校正） */
int QMC5883PCompass::getAzimuth() {
  // 关键修正：heading = atan2(X, Y)
  float heading = atan2((float)getX(), (float)getY()) * 180.0f / PI;

  // 加上磁偏角
  heading += _magneticDeclinationDegrees;

  // 归一化到[0,360)
  while (heading <   0.0f) heading += 360.0f;
  while (heading >= 360.0f) heading -= 360.0f;

  return (int)heading;
}

/** 表盘角：用于“转表盘、针固定”式UI（表盘需转的角度） */
int QMC5883PCompass::getDialAngle() {
  int az = getAzimuth();
  int dial = 360 - az;
  if (dial >= 360) dial -= 360;
  return dial;
}

byte QMC5883PCompass::getBearing(int azimuth) {
  float a = (azimuth + 11.25f) / 22.5f;
  return ((int)a) & 0x0F;
}

void QMC5883PCompass::getDirection(char *myArray, int azimuth) {
  int d = getBearing(azimuth);
  myArray[0] = _bearings[d][0];
  myArray[1] = _bearings[d][1];
  myArray[2] = _bearings[d][2];
}

/** 轴重映射设置：把“板级 X/Y/Z”映射到“芯片某轴”，并指定正负 */
void QMC5883PCompass::setAxisRemap(uint8_t bx_src, int8_t bx_sgn,
                                    uint8_t by_src, int8_t by_sgn,
                                    uint8_t bz_src, int8_t bz_sgn) {
  _map_src[0] = bx_src; _map_sgn[0] = (bx_sgn >= 0) ? +1 : -1;
  _map_src[1] = by_src; _map_sgn[1] = (by_sgn >= 0) ? +1 : -1;
  _map_src[2] = bz_src; _map_sgn[2] = (bz_sgn >= 0) ? +1 : -1;
}
