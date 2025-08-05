// 文件: MMC5883MACompass.cpp
#include "MMC5883MACompass.h"
#include <Wire.h>
#include <Arduino.h>

MMC5883MACompass::MMC5883MACompass() {}

void MMC5883MACompass::init() {
    Wire.begin();
    _writeReg(0x09, 0x80);  // SW reset
    delay(5);               // ≥5ms
    _writeReg(0x09, 0x02);  // BW = 400Hz
    _writeReg(0x08, 0x01);  // Continuous mode
    _performSet();
    _performReset();
}

void MMC5883MACompass::setReset() {
    _writeReg(0x09, 0x80);
    delay(5);
}

void MMC5883MACompass::setADDR(byte b) {
    _ADDR = b;
}

void MMC5883MACompass::setMode(byte mode, byte odr) {
    mode &= 0x03;
    odr  &= 0x0C;
    _writeReg(0x09, odr);
    _writeReg(0x08, mode);
}

void MMC5883MACompass::setMagneticDeclination(int degrees, uint8_t minutes) {
    _magneticDeclinationDegrees = degrees + minutes / 60.0;
}

void MMC5883MACompass::setSmoothing(byte steps, bool adv) {
    _smoothUse      = true;
    _smoothSteps    = min(steps, (byte)10);
    _smoothAdvanced = adv;
}

void MMC5883MACompass::calibrate() {
    // 保留原校准实现
    // 用户可根据需求填写基于SET/RESET的校准流程
}

char MMC5883MACompass::chipID() {
    Wire.beginTransmission(_ADDR);
    Wire.write(0x07);
    if (Wire.endTransmission() == 0) {
        Wire.requestFrom(_ADDR, (byte)1);
        return Wire.read();
    }
    return 0;
}

void MMC5883MACompass::read() {
    _writeReg(0x08, 0x02);  // Trigger single measurement
    delay(3);               // ≥2.5ms

    Wire.beginTransmission(_ADDR);
    Wire.write((byte)0x00);
    if (Wire.endTransmission() == 0 && Wire.requestFrom(_ADDR, (byte)6) == 6) {
        _vRaw[0] = (int16_t)(Wire.read() | (Wire.read() << 8));
        _vRaw[1] = (int16_t)(Wire.read() | (Wire.read() << 8));
        _vRaw[2] = (int16_t)(Wire.read() | (Wire.read() << 8));
        _performReset();      // Reset bridge
        _applyCalibration();
        if (_smoothUse) _smoothing();
    }
}

void MMC5883MACompass::_writeReg(byte r, byte v) {
    Wire.beginTransmission(_ADDR);
    Wire.write(r);
    Wire.write(v);
    Wire.endTransmission();
}

void MMC5883MACompass::_performSet() {
    _writeReg(0x08, 0x08);
    delay(1);               // ≥1ms
}

void MMC5883MACompass::_performReset() {
    _writeReg(0x08, 0x10);
    delay(1);               // ≥1ms
}

void MMC5883MACompass::_applyCalibration() {
    _vCalibrated[0] = (_vRaw[0] - (int)_offset[0]) * _scale[0];
    _vCalibrated[1] = (_vRaw[1] - (int)_offset[1]) * _scale[1];
    _vCalibrated[2] = (_vRaw[2] - (int)_offset[2]) * _scale[2];
}

void MMC5883MACompass::_smoothing() {
    byte max = 0, min = 0;
    if (_vScan >= _smoothSteps) _vScan = 0;
    for (int i = 0; i < 3; i++) _vHistory[_vScan][i] = _vCalibrated[i];
    _vScan++;

    memset(_vTotals, 0, sizeof(_vTotals));
    for (int j = 0; j < _smoothSteps; j++) {
        for (int i = 0; i < 3; i++) _vTotals[i] += _vHistory[j][i];
    }

    for (int i = 0; i < 3; i++) {
        max = min = 0;
        for (byte j = 1; j < _smoothSteps; j++) {
            if (_vHistory[j][i] > _vHistory[max][i]) max = j;
            if (_vHistory[j][i] < _vHistory[min][i]) min = j;
        }
        if (_smoothAdvanced && _smoothSteps > 2) {
            _vSmooth[i] = (_vTotals[i] - _vHistory[max][i] - _vHistory[min][i]) / (_smoothSteps - 2);
        } else {
            _vSmooth[i] = _vTotals[i] / _smoothSteps;
        }
    }
}
