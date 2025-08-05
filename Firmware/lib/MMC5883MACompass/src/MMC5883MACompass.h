// 文件: MMC5883MACompass.h
#ifndef MMC5883MA_COMPASS_H
#define MMC5883MA_COMPASS_H

#include "Arduino.h"
#include <Wire.h>

class MMC5883MACompass {
public:
    MMC5883MACompass();
    void init();
    void setReset();
    void setADDR(byte b);
    void setMode(byte mode, byte odr);
    void setMagneticDeclination(int degrees, uint8_t minutes);
    void setSmoothing(byte steps, bool adv);
    void calibrate();
    void setCalibration(int x_min, int x_max, int y_min, int y_max, int z_min, int z_max);
    void read();
    char chipID();
    int getAzimuth();
    byte getBearing(int azimuth);
    void getDirection(char *myArray, int azimuth);
    float getCalibrationOffset(byte idx);
    float getCalibrationScale(byte idx);
    int getX();
    int getY();
    int getZ();

private:
    byte _ADDR = 0x30;
    float _magneticDeclinationDegrees = 0;
    bool _smoothUse = false;
    byte _smoothSteps = 5;
    bool _smoothAdvanced = false;
    int _vRaw[3] = {0, 0, 0};
    int _vHistory[10][3];
    int _vScan = 0;
    long _vTotals[3] = {0, 0, 0};
    int _vSmooth[3] = {0, 0, 0};
    float _offset[3] = {0.0, 0.0, 0.0};
    float _scale[3]  = {1.0, 1.0, 1.0};
    int _vCalibrated[3] = {0, 0, 0};
    const char _bearings[16][3];
    void _writeReg(byte reg, byte val);
    void _performSet();
    void _performReset();
    void _applyCalibration();
    void _smoothing();
};

extern const char MMC5883MACompass::_bearings[16][3];
#endif // MMC5883MA_COMPASS_H
