#include "MMC5883MACompass.h"
#include <math.h>

/**
 * Constructor
 */
MMC5883MACompass::MMC5883MACompass() {}

/**
 * 初始化：软件复位 + 带宽设置 + 首次 SET/RESET 清偏
 */
void MMC5883MACompass::init() {
  Wire.begin();
  _writeReg(0x09, 0x80);   // 软件复位
  delay(5);
  _writeReg(0x09, 0x02);   // Control Reg1: BW=400Hz
  _performSet();
  _performReset();
}

/**
 * 读数据流程：SET→测量→读→RESET→测量→读→平均→校准→平滑
 */
void MMC5883MACompass::read() {
  int16_t x_set, y_set, z_set;
  int16_t x_rst, y_rst, z_rst;

  // SET
  _performSet();
  delay(2);
  _writeReg(0x08, 0x02);   // TM_M
  delay(3);
  Wire.beginTransmission(_ADDR);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(_ADDR, (byte)6);
  x_set = (int16_t)(Wire.read() | Wire.read() << 8);
  y_set = (int16_t)(Wire.read() | Wire.read() << 8);
  z_set = (int16_t)(Wire.read() | Wire.read() << 8);

  // RESET
  _performReset();
  delay(2);
  _writeReg(0x08, 0x02);   // TM_M
  delay(3);
  Wire.beginTransmission(_ADDR);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(_ADDR, (byte)6);
  x_rst = (int16_t)(Wire.read() | Wire.read() << 8);
  y_rst = (int16_t)(Wire.read() | Wire.read() << 8);
  z_rst = (int16_t)(Wire.read() | Wire.read() << 8);

  // 平均
  _vRaw[0] = (x_set + x_rst) / 2;
  _vRaw[1] = (y_set + y_rst) / 2;
  _vRaw[2] = (z_set + z_rst) / 2;

  // 校准 & 平滑
  _applyCalibration();
  if (_smoothUse) _smoothing();
}

/**
 * Adapter 需要的接口
 */
void MMC5883MACompass::setSet()   { _performSet(); }
void MMC5883MACompass::setReset() { _performReset(); }

char MMC5883MACompass::chipID() {
  Wire.beginTransmission(_ADDR);
  Wire.write(0x07);
  Wire.endTransmission();
  Wire.requestFrom(_ADDR, (byte)1);
  return Wire.available() ? Wire.read() : 0;
}

int MMC5883MACompass::getX()       { return _vCalibrated[0]; }
int MMC5883MACompass::getY()       { return _vCalibrated[1]; }
int MMC5883MACompass::getZ()       { return _vCalibrated[2]; }

void MMC5883MACompass::calibrate() {
  // 如需自动标定，可在此实现。留空则采用外部手动 setCalibration().
}

void MMC5883MACompass::setMode(byte mode, byte odr) {
  // mode → Control Reg2(0x0A)，odr → Control Reg1(0x09)
  _writeReg(0x09, odr);
  _writeReg(0x0A, mode);
}

/**
 * 校准相关
 */
void MMC5883MACompass::setCalibration(int x_min, int x_max, int y_min, int y_max, int z_min, int z_max) {
  _offset[0] = (x_max + x_min) / 2.0f;
  _offset[1] = (y_max + y_min) / 2.0f;
  _offset[2] = (z_max + z_min) / 2.0f;
  float spanX = (x_max - x_min) / 2.0f;
  float spanY = (y_max - y_min) / 2.0f;
  float spanZ = (z_max - z_min) / 2.0f;
  _scale[0] = spanX ? (1.0f / spanX) : 1.0f;
  _scale[1] = spanY ? (1.0f / spanY) : 1.0f;
  _scale[2] = spanZ ? (1.0f / spanZ) : 1.0f;
}

void MMC5883MACompass::setCalibrationOffsets(float x_offset, float y_offset, float z_offset) {
  _offset[0] = x_offset;  _offset[1] = y_offset;  _offset[2] = z_offset;
}

void MMC5883MACompass::setCalibrationScales(float x_scale, float y_scale, float z_scale) {
  _scale[0] = x_scale;  _scale[1] = y_scale;  _scale[2] = z_scale;
}

float MMC5883MACompass::getCalibrationOffset(uint8_t i) {
  return (i < 3) ? _offset[i] : 0;
}
float MMC5883MACompass::getCalibrationScale(uint8_t i) {
  return (i < 3) ? _scale[i] : 1;
}

void MMC5883MACompass::clearCalibration() {
  _offset[0]=_offset[1]=_offset[2]=0;
  _scale [0]=_scale [1]=_scale [2]=1;
}

/**
 * 其它可选设置
 */
void MMC5883MACompass::setMagneticDeclination(int deg, uint8_t min) {
  _magneticDeclinationDegrees = deg + min/60.0f;
}

void MMC5883MACompass::setSmoothing(byte steps, bool adv) {
  _smoothSteps    = min(steps, (byte)10);
  _smoothAdvanced = adv;
  _smoothUse      = (_smoothSteps>1);
  _vScan = 0;
  memset(_vHistory, 0, sizeof(_vHistory));
  _vTotals[0]=_vTotals[1]=_vTotals[2]=0;
}

/**
 * 底层读写与偏置消除
 */
void MMC5883MACompass::_writeReg(byte reg, byte val) {
  Wire.beginTransmission(_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

void MMC5883MACompass::_performSet() {
  _writeReg(0x08, 0x08);
  delayMicroseconds(100);
}

void MMC5883MACompass::_performReset() {
  _writeReg(0x08, 0x10);
  delayMicroseconds(100);
}

/**
 * 应用校准 & 计算 _vCalibrated
 */
void MMC5883MACompass::_applyCalibration() {
  for (int i=0; i<3; i++) {
    float adj = _vRaw[i] - _offset[i];
    _vCalibrated[i] = int(adj * _scale[i]);
  }
}

/**
 * 简单滚动平均平滑
 */
void MMC5883MACompass::_smoothing() {
  _vScan = (_vScan + 1) % _smoothSteps;
  _vTotals[0] -= _vHistory[_vScan][0];
  _vTotals[1] -= _vHistory[_vScan][1];
  _vTotals[2] -= _vHistory[_vScan][2];

  _vHistory[_vScan][0] = _vCalibrated[0];
  _vHistory[_vScan][1] = _vCalibrated[1];
  _vHistory[_vScan][2] = _vCalibrated[2];

  _vTotals[0] += _vHistory[_vScan][0];
  _vTotals[1] += _vHistory[_vScan][1];
  _vTotals[2] += _vHistory[_vScan][2];

  _vCalibrated[0] = _vTotals[0] / _smoothSteps;
  _vCalibrated[1] = _vTotals[1] / _smoothSteps;
  _vCalibrated[2] = _vTotals[2] / _smoothSteps;
}

/**
 * 方向计算
 */
int MMC5883MACompass::getAzimuth() {
  float x = _vCalibrated[0], y = _vCalibrated[1];
  float h = atan2(y, x) * 180.0f / M_PI + _magneticDeclinationDegrees;
  if (h<0)    h += 360;
  if (h>=360) h -= 360;
  return int(h);
}

byte MMC5883MACompass::getBearing(int az) {
  return (byte)((az + 11) / 22) & 0x0F;
}

void MMC5883MACompass::getDirection(char* a, int az) {
  byte i = getBearing(az);
  a[0]=_bearings[i][0];  a[1]=_bearings[i][1];  a[2]=_bearings[i][2];
}
