#ifndef MMC5883MA_COMPASS_H
#define MMC5883MA_COMPASS_H

#include <Arduino.h>
#include <Wire.h>

class MMC5883MACompass {
public:
    MMC5883MACompass();

    /* ---- 基本控制 ---- */
    void   init();
    void   setADDR(uint8_t addr);
    char   chipID();                             // 读 0x2F，期望 0x0C
    void   setMagneticDeclination(int deg, uint8_t min = 0);
    void   setSmoothing(uint8_t steps = 5, bool advanced = false);
    void   setCalibration(int x_min, int x_max,
                          int y_min, int y_max,
                          int z_min, int z_max);
    void   clearCalibration();

    /* ---- 读数接口 ---- */
    void   read();                               // 带 SET/RESET 的一次完整读取
    int    getX();
    int    getY();
    int    getZ();
    int    getAzimuth();                         // 0-359°
    uint8_t getBearing(int azimuth);             // 0-15
    void   getDirection(char dir[3], int azimuth);

private:
    /* ---- 低层工具 ---- */
    void   _writeReg(uint8_t reg, uint8_t val);
    bool   _singleMeasurement(int16_t& x, int16_t& y, int16_t& z);
    void   _performSet();
    void   _performReset();
    void   _applyCalibration();
    void   _smoothing();
    int    _get(int idx);

    /* ---- 成员 ---- */
    uint8_t _ADDR = 0x30;                        // 默认 I²C 地址
    int32_t _vRaw[3]        {0, 0, 0};
    int32_t _vCalibrated[3] {0, 0, 0};
    float   _offset[3]      {0.f, 0.f, 0.f};
    float   _scale[3]       {1.f, 1.f, 1.f};

    /* 平滑 */
    bool    _smoothUse      = false;
    bool    _smoothAdv      = false;
    uint8_t _smoothSteps    = 5;
    int32_t _vHist[10][3]   {};
    int32_t _vTotals[3]     {0, 0, 0};
    uint8_t _vScan          = 0;
    int32_t _vSmooth[3]     {0, 0, 0};

    /* 其它 */
    float   _declinationDeg = 0.f;
    static const char _bearings[16][3];
};

#endif
