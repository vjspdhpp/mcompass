#include "MMC5883MACompass.h"

/**
 * Constructor
 */
MMC5883MACompass::MMC5883MACompass() {}

/**
 * 初始化芯片
 */
void MMC5883MACompass::init() {
  Wire.begin();
  _writeReg(0x09, 0x80);   // 软件复位
  delay(5);
  _writeReg(0x09, 0x02);   // 带宽设置 BW=400Hz
  _performSet();           // 初次 SET
  _performReset();         // 初次 RESET
}

/**
 * 读数：SET→测量→读→RESET→测量→读→平均→校准→平滑
 */
void MMC5883MACompass::read() {
  int16_t x_set, y_set, z_set;
  int16_t x_rst, y_rst, z_rst;

  _performSet();
  delay(2);
  _writeReg(0x08, 0x02);
  delay(3);
  Wire.beginTransmission(_ADDR);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(_ADDR, (byte)6);
  x_set = (int16_t)(Wire.read() | Wire.read() << 8);
  y_set = (int16_t)(Wire.read() | Wire.read() << 8);
  z_set = (int16_t)(Wire.read() | Wire.read() << 8);

  _performReset();
  delay(2);
  _writeReg(0x08, 0x02);
  delay(3);
  Wire.beginTransmission(_ADDR);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(_ADDR, (byte)6);
  x_rst = (int16_t)(Wire.read() | Wire.read() << 8);
  y_rst = (int16_t)(Wire.read() | Wire.read() << 8);
  z_rst = (int16_t)(Wire.read() | Wire.read() << 8);

  _vRaw[0] = (x_set + x_rst) / 2;
  _vRaw[1] = (y_set + y_rst) / 2;
  _vRaw[2] = (z_set + z_rst) / 2;

  _applyCalibration();
  if (_smoothUse) _smoothing();
}

/**
 * Adapter 需要的公开接口：直接调用内部 SET/RESET
 */
void MMC5883MACompass::setSet() {
  _performSet();
}
void MMC5883MACompass::setReset() {
  _performReset();
}

// 其余函数保持不变——写寄存器、校准、平滑、方向计算等

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

// ... _applyCalibration(), _smoothing(), getX(), getY(), getZ(), getAzimuth(), getBearing(), getDirection(), calibrate(), setMode(), setMagneticDeclination(), setSmoothing(), setCalibrationOffsets(), setCalibrationScales(), clearCalibration() 等
