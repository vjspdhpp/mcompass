#ifndef QMC5883P_COMPASS_H
#define QMC5883P_COMPASS_H

#include <Arduino.h>
#include <Wire.h>

class QMC5883PCompass {
public:
    QMC5883PCompass();
    void  init();
    void  setADDR(byte b);
    void  setMode(byte mode, byte odr, byte rng, byte osr);
    void  setMagneticDeclination(int degrees, uint8_t minutes);
    void  setSmoothing(byte steps, bool adv);

    /* --- 校准相关 --- */
    void  calibrate();
    void  setCalibration(int x_min, int x_max,
                         int y_min, int y_max,
                         int z_min, int z_max);
    void  setCalibrationOffsets(float x_offset, float y_offset, float z_offset);
    void  setCalibrationScales (float x_scale, float y_scale, float z_scale);
    float getCalibrationOffset(uint8_t index);
    float getCalibrationScale (uint8_t index);
    void  clearCalibration();

    void  setReset();

    /* --- 读数接口 --- */
    void  read();
    int   getX();
    int   getY();
    int   getZ();
    int   getAzimuth();                // 0-359°
    byte  getBearing(int azimuth);     // 16 方位索引
    void  getDirection(char* buf, int azimuth);
    char  chipID();

private:
    void  _writeReg(byte reg, byte val);
    int   _get(int index);
    void  _smoothing();
    void  _applyCalibration();

    /* --- 内部变量 --- */
    float _magneticDeclinationDegrees = 0.0f;
    bool  _smoothUse      = false;
    byte  _smoothSteps    = 5;
    bool  _smoothAdvanced = false;

    byte  _ADDR           = 0x2C;          // QMC5883P 默认 I2C 地址
    int   _vRaw[3]        = {0, 0, 0};
    int   _vHistory[10][3];
    int   _vScan          = 0;
    long  _vTotals[3]     = {0, 0, 0};
    int   _vSmooth[3]     = {0, 0, 0};

    float _offset[3]      = {0.0f, 0.0f, 0.0f};
    float _scale[3]       = {1.0f, 1.0f, 1.0f};
    int   _vCalibrated[3];

    /* 16 方位字符表 */
    const char _bearings[16][3] = {
        {' ', ' ', 'N'},  {'N', 'N', 'E'},  {' ', 'N', 'E'},  {'E', 'N', 'E'},
        {' ', ' ', 'E'},  {'E', 'S', 'E'},  {' ', 'S', 'E'},  {'S', 'S', 'E'},
        {' ', ' ', 'S'},  {'S', 'S', 'W'},  {' ', 'S', 'W'},  {'W', 'S', 'W'},
        {' ', ' ', 'W'},  {'W', 'N', 'W'},  {' ', 'N', 'W'},  {'N', 'N', 'W'},
    };
};

#endif /* QMC5883P_COMPASS_H */
