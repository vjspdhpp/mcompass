#include "QMC5883LCompass.h"
#include <Wire.h>
#include <math.h>

/* ── MMC5883MA 寄存器 ─────────────────────────── */
constexpr byte REG_DATA_X_L = 0x00;   // 连续 6 字节 X_L…Z_H
constexpr byte REG_STATUS   = 0x07;   // bit0 = Data Ready
constexpr byte REG_CTRL0    = 0x08;   // TM_M / SET / RESET
constexpr byte REG_CTRL2    = 0x0A;   // Soft-reset
constexpr byte REG_PRODUCT  = 0x2F;   // 0x0C

/* 控制位 */
constexpr byte CTRL0_TM_M   = 0x02;   // ← 改为 bit1
constexpr byte CTRL0_SET    = 0x08;
constexpr byte CTRL0_RESET  = 0x10;

/* ─────────────────────────────────────────────── */
QMC5883LCompass::QMC5883LCompass() {}

/* 初始化 */
void QMC5883LCompass::init() {
    Wire.begin();
    // 软件复位
    _writeReg(REG_CTRL2, 0x80);
    delay(2);

    // 首次 SET/RESET 消除偏置
    _writeReg(REG_CTRL0, CTRL0_SET);
    delayMicroseconds(50);
    _writeReg(REG_CTRL0, CTRL0_RESET);
    delayMicroseconds(50);

    clearCalibration();
}

/* 读一次数据，双采样平均去偏 */
bool QMC5883LCompass::read() {
    int16_t A[3], B[3];

    // ① SET
    _writeReg(REG_CTRL0, CTRL0_SET);
    delayMicroseconds(50);

    // ② 启动测量 A
    _writeReg(REG_CTRL0, CTRL0_TM_M);
    while (!(_readStatus() & 0x01));  // 等待 Data Ready
    _burstRead(A);

    // ③ RESET
    _writeReg(REG_CTRL0, CTRL0_RESET);
    delayMicroseconds(50);

    // ④ 启动测量 B
    _writeReg(REG_CTRL0, CTRL0_TM_M);
    while (!(_readStatus() & 0x01));
    _burstRead(B);

    // ⑤ (A + B)/2
    for (int i = 0; i < 3; ++i)
        _vRaw[i] = int((A[i] + B[i]) / 2);

    _applyCalibration();
    if (_useSmooth) _smoothing();
    return true;
}

/* ── I²C 工具 ───────────────────────────────── */
void QMC5883LCompass::_writeReg(byte reg, byte val){
    Wire.beginTransmission(_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

uint8_t QMC5883LCompass::_readStatus(){
    Wire.beginTransmission(_ADDR);
    Wire.write(REG_STATUS);
    Wire.endTransmission(false);
    Wire.requestFrom(_ADDR, (byte)1);
    return Wire.read();
}

/* 直接按两字节拼成 int16_t，不要减掉 32768 */
void QMC5883LCompass::_burstRead(int16_t v[3]){
    Wire.beginTransmission(_ADDR);
    Wire.write(REG_DATA_X_L);
    Wire.endTransmission(false);
    Wire.requestFrom(_ADDR, (byte)6);
    for (int i = 0; i < 3; ++i) {
        uint16_t lo = Wire.read();
        uint16_t hi = Wire.read();
        v[i] = int16_t((hi << 8) | lo);
    }
}

/* ── 校准与滤波（保持不变，这里略去） ───────────────────────── */
void QMC5883LCompass::setMagneticDeclination(int d, uint8_t m){
    _magDeclDeg = d + m / 60.0f;
}
// … 省略其余 calibrate(), setCalibration*(), _applyCalibration(), _smoothing(), getX/Y/Z/Azimuth/Bearing/Direction 等函数 …
