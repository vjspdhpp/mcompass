#include "MMC5883MACompass.h"

static inline int16_t u16_to_s16(uint16_t u) { return int16_t(u - 32768); }

/* ───────────────────────── 公共接口 ───────────────────────── */

MMC5883MACompass::MMC5883MACompass() {}

void MMC5883MACompass::init() {
    Wire.begin();

    /* 软件复位，等待 5 ms */
    _writeReg(0x09, 0x80);
    delay(5);

    /* 带宽 400 Hz（BW1:0 = 10）→ 2.5 ms / 转换 */
    _writeReg(0x09, 0x10);

    /* 关闭连续测量，采用单次触发模式（0x0A CM_Freq = 0000） */
    _writeReg(0x0A, 0x00);

    /* 初次 SET/RESET */
    _performSet();
    _performReset();
}

void MMC5883MACompass::setADDR(uint8_t addr) { _ADDR = addr; }

char MMC5883MACompass::chipID() {
    Wire.beginTransmission(_ADDR);
    Wire.write(0x2F);
    if (Wire.endTransmission() == 0 && Wire.requestFrom(_ADDR, uint8_t(1)) == 1)
        return Wire.read();
    return 0;
}

void MMC5883MACompass::setMagneticDeclination(int deg, uint8_t min) {
    _declinationDeg = float(deg) + float(min) / 60.f;
}

void MMC5883MACompass::setSmoothing(uint8_t steps, bool adv) {
    _smoothUse  = true;
    _smoothAdv  = adv;
    _smoothSteps = steps < 2 ? 2 : (steps > 10 ? 10 : steps);
}

void MMC5883MACompass::clearCalibration() {
    _offset[0] = _offset[1] = _offset[2] = 0.f;
    _scale [0] = _scale [1] = _scale [2] = 1.f;
}

void MMC5883MACompass::setCalibration(int xmin, int xmax,
                                      int ymin, int ymax,
                                      int zmin, int zmax) {
    _offset[0] = (xmin + xmax) / 2.f;
    _offset[1] = (ymin + ymax) / 2.f;
    _offset[2] = (zmin + zmax) / 2.f;

    float x_delta = (xmax - xmin) / 2.f;
    float y_delta = (ymax - ymin) / 2.f;
    float z_delta = (zmax - zmin) / 2.f;
    float avg     = (x_delta + y_delta + z_delta) / 3.f;

    _scale[0] = avg / x_delta;
    _scale[1] = avg / y_delta;
    _scale[2] = avg / z_delta;
}

/* 读取一次：SET → 测量 → RESET → 测量 → 去偏 & 校正 */
void MMC5883MACompass::read() {
    int16_t xs, ys, zs, xr, yr, zr;

    _performSet();
    if (!_singleMeasurement(xs, ys, zs)) return;

    _performReset();
    if (!_singleMeasurement(xr, yr, zr)) return;

    _vRaw[0] = (xs - xr) / 2;
    _vRaw[1] = (ys - yr) / 2;
    _vRaw[2] = (zs - zr) / 2;

    _applyCalibration();
    if (_smoothUse) _smoothing();
}

int MMC5883MACompass::getX() { return _get(0); }
int MMC5883MACompass::getY() { return _get(1); }
int MMC5883MACompass::getZ() { return _get(2); }

int MMC5883MACompass::getAzimuth() {
    float hdg = atan2(float(getY()), float(getX())) * 180.f / PI;
    hdg += _declinationDeg;
    if (hdg < 0)   hdg += 360.f;
    if (hdg >= 360.f) hdg -= 360.f;
    return int(hdg);
}

uint8_t MMC5883MACompass::getBearing(int az) {
    return uint8_t((az + 11.25) / 22.5) & 0x0F;
}

void MMC5883MACompass::getDirection(char dir[3], int az) {
    uint8_t b = getBearing(az);
    dir[0] = _bearings[b][0];
    dir[1] = _bearings[b][1];
    dir[2] = _bearings[b][2];
}

/* ───────────────────────── 私有实现 ───────────────────────── */

void MMC5883MACompass::_writeReg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

/* 单次触发并读取原始 XYZ（已去 32768） */
bool MMC5883MACompass::_singleMeasurement(int16_t& x, int16_t& y, int16_t& z) {
    /* 触发 TM_M=1 */
    _writeReg(0x08, 0x01);

    /* 等待 Meas_M_Done=1，最多 10 ms */
    for (uint8_t i = 0; i < 20; ++i) {
        Wire.beginTransmission(_ADDR);
        Wire.write(0x07);
        Wire.endTransmission();
        if (Wire.requestFrom(_ADDR, uint8_t(1)) == 1) {
            if (Wire.read() & 0x01) break;
        }
        delay(1);
    }

    /* 读取 6 字节 (Low→High) */
    Wire.beginTransmission(_ADDR);
    Wire.write(0x00);
    Wire.endTransmission();
    if (Wire.requestFrom(_ADDR, uint8_t(6)) != 6) return false;

    uint8_t xl = Wire.read(), xh = Wire.read();
    uint8_t yl = Wire.read(), yh = Wire.read();
    uint8_t zl = Wire.read(), zh = Wire.read();

    x = u16_to_s16(uint16_t(xh) << 8 | xl);
    y = u16_to_s16(uint16_t(yh) << 8 | yl);
    z = u16_to_s16(uint16_t(zh) << 8 | zl);
    return true;
}

void MMC5883MACompass::_performSet()   { _writeReg(0x08, 0x08); delay(1); }
void MMC5883MACompass::_performReset() { _writeReg(0x08, 0x10); delay(1); }

void MMC5883MACompass::_applyCalibration() {
    for (uint8_t i = 0; i < 3; ++i)
        _vCalibrated[i] = int((_vRaw[i] - _offset[i]) * _scale[i]);
}

void MMC5883MACompass::_smoothing() {
    if (_vScan >= _smoothSteps) _vScan = 0;

    for (uint8_t i = 0; i < 3; ++i) {
        _vTotals[i] -= _vHist[_vScan][i];
        _vHist[_vScan][i] = _vCalibrated[i];
        _vTotals[i] += _vHist[_vScan][i];

        if (_smoothAdv) {
            /* 去掉一极值一极小值再平均 */
            int maxIdx = 0, minIdx = 0;
            for (uint8_t j = 1; j < _smoothSteps; ++j) {
                if (_vHist[j][i] > _vHist[maxIdx][i]) maxIdx = j;
                if (_vHist[j][i] < _vHist[minIdx][i]) minIdx = j;
            }
            _vSmooth[i] =
                (_vTotals[i] - (_vHist[maxIdx][i] + _vHist[minIdx][i])) /
                int(_smoothSteps - 2);
        } else {
            _vSmooth[i] = _vTotals[i] / int(_smoothSteps);
        }
    }
    ++_vScan;
}

int MMC5883MACompass::_get(int idx) {
    return _smoothUse ? _vSmooth[idx] : _vCalibrated[idx];
}

/* ───────────────────────── 方位字符表 ───────────────────────── */
const char MMC5883MACompass::_bearings[16][3] = {
    {' ', ' ', 'N'}, {'N', 'N', 'E'}, {' ', 'N', 'E'}, {'E', 'N', 'E'},
    {' ', ' ', 'E'}, {'E', 'S', 'E'}, {' ', 'S', 'E'}, {'S', 'S', 'E'},
    {' ', ' ', 'S'}, {'S', 'S', 'W'}, {' ', 'S', 'W'}, {'W', 'S', 'W'},
    {' ', ' ', 'W'}, {'W', 'N', 'W'}, {' ', 'N', 'W'}, {'N', 'N', 'W'}
};
