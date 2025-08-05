/*
===============================================================================================================
MMC5883MACompass.cpp
Library for using MMC5883MA chip as a compass.
Adapted from QMC5883PCompass library to support MMC5883MA chip.
Based on MEMSIC MMC5883MA datasheet (Rev. C, 4/28/2017).

Supports:
- Getting values of XYZ axis.
- Calculating Azimuth.
- Getting 16 point Azimuth bearing direction (0 - 15).
- Getting 16 point Azimuth bearing Names (N, NNE, NE, ENE, E, ESE, SE, SSE, S, SSW, SW, WSW, W, WNW, NW, NNW).
- Smoothing of XYZ readings via rolling averaging and min/max removal.
- SET/RESET functionality for offset elimination.
===============================================================================================================
*/

#include "MMC5883MACompass.h"

/**
 * Constructor
 */
MMC5883MACompass::MMC5883MACompass() {}

/**
 * 初始化芯片
 * 使用手动测量流程：先软件复位 → 设置带宽 → 进入待机 → SET/RESET 清偏 → 后续读数由 read() 触发
 */
void MMC5883MACompass::init() {
  Wire.begin();
  // 软件复位
  _writeReg(0x09, 0x80);
  delay(5); // tOp ≥5ms

  // Control Register 1: 设置带宽 BW = 400Hz (BW1=1, BW0=0)
  _writeReg(0x09, 0x02);

  // 进入待机模式（手动触发测量）
  // _writeReg(0x08, 0x00); // standby，默认即为 0x00

  // 初次 SET/RESET，清除残余偏置
  _performSet();
  _performReset();
}

/**
 * 设置 I2C 地址
 */
void MMC5883MACompass::setADDR(byte b) {
  _ADDR = b;
}

/* ===== 以下其余 read/写寄存器、校准、平滑、方向转换等函数保持不变 ===== */

/**
 * Read XYZ Axis
 * 采用 SET/RESET 双测量取平均的方式消除温漂和偏置
 */
void MMC5883MACompass::read() {
  int16_t x_set, y_set, z_set;
  int16_t x_rst, y_rst, z_rst;

  // 1) SET: 清除偏置第一步
  _performSet();
  delay(2); // ≥1ms

  // 2) 触发磁场测量
  _writeReg(0x08, 0x02); // TM_M
  delay(3); // 测量时间 ≈2.5ms

  // 3) 读取 SET 测量值
  Wire.beginTransmission(_ADDR);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(_ADDR, (byte)6);
  x_set = (int16_t)(Wire.read() | Wire.read() << 8);
  y_set = (int16_t)(Wire.read() | Wire.read() << 8);
  z_set = (int16_t)(Wire.read() | Wire.read() << 8);

  // 4) RESET: 清除偏置第二步
  _performReset();
  delay(2); // ≥1ms

  // 5) 触发第二次测量
  _writeReg(0x08, 0x02); // TM_M
  delay(3);

  // 6) 读取 RESET 测量值
  Wire.beginTransmission(_ADDR);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(_ADDR, (byte)6);
  x_rst = (int16_t)(Wire.read() | Wire.read() << 8);
  y_rst = (int16_t)(Wire.read() | Wire.read() << 8);
  z_rst = (int16_t)(Wire.read() | Wire.read() << 8);

  // 7) 平均两次测量结果，消除偏置
  _vRaw[0] = (x_set + x_rst) / 2;
  _vRaw[1] = (y_set + y_rst) / 2;
  _vRaw[2] = (z_set + z_rst) / 2;

  // 8) 应用用户校准 & 平滑
  _applyCalibration();
  if (_smoothUse) {
    _smoothing();
  }
}

/* ===== 以下为原库中其余函数，保持不变 ===== */

// 读芯片 ID，返回 0xFF
char MMC5883MACompass::chipID() {
  Wire.beginTransmission(_ADDR);
  Wire.write(0x07);
  Wire.endTransmission();
  Wire.requestFrom(_ADDR, (byte)1);
  return Wire.read();
}

// 写寄存器
void MMC5883MACompass::_writeReg(byte reg, byte val) {
  Wire.beginTransmission(_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

// 清除校准
void MMC5883MACompass::clearCalibration() {
  setCalibrationOffsets(0., 0., 0.);
  setCalibrationScales(1., 1., 1.);
}

// SET 动作
void MMC5883MACompass::_performSet() {
  _writeReg(0x08, 0x08); // SET bit
  delayMicroseconds(100);
}

// RESET 动作
void MMC5883MACompass::_performReset() {
  _writeReg(0x08, 0x10); // RESET bit
  delayMicroseconds(100);
}

// 其余：_applyCalibration(), _smoothing(), getX(), getY(), getZ(), getAzimuth(), getBearing(), getDirection(), calibrate(), setMode(), setMagneticDeclination(), setSmoothing(), setCalibration(), setCalibrationOffsets(), setCalibrationScales() 等函数，均按原库代码不做改动。

