#ifndef MMC5883MA_Compass
#define MMC5883MA_Compass

#include <Arduino.h>
#include <Wire.h>

class MMC5883MACompass {
public:
    MMC5883MACompass();

    // 基本接口
    void init();
    void read();

    // Adapter 需要的接口
    void setSet();
    void setReset();
    char chipID();
    int getX();
    int getY();
    int getZ();
    int getAzimuth();
    byte getBearing(int azimuth);
    void getDirection(char* myArray, int azimuth);
    void calibrate();
    void setMode(byte mode, byte odr);

    // 校准 & 平滑控制
    void setCalibration(int x_min, int x_max, int y_min, int y_max, int z_min, int z_max);
    void setCalibrationOffsets(float x_offset, float y_offset, float z_offset);
    void setCalibrationScales(float x_scale, float y_scale, float z_scale);
    float getCalibrationOffset(uint8_t index);
    float getCalibrationScale(uint8_t index);
    void clearCalibration();

    // 其它可选设置
    void setMagneticDeclination(int degrees, uint8_t minutes);
    void setSmoothing(byte steps, bool adv);

private:
    // 底层寄存器读写
    void _writeReg(byte reg, byte val);
    // 偏置消除
    void _performSet();
    void _performReset();
    // 校准 & 平滑内部实现
    void _applyCalibration();
    void _smoothing();

    // I2C 地址
    byte _ADDR = 0x30;

    // 原始和校准后的数据
    int _vRaw[3]        = {0,0,0};
    int _vCalibrated[3] = {0,0,0};

    // 校准参数
    float _offset[3]    = {0,0,0};
    float _scale[3]     = {1,1,1};

    // 平滑参数
    bool  _smoothUse      = false;
    byte  _smoothSteps    = 5;
    bool  _smoothAdvanced = false;
    int   _vHistory[10][3];
    int   _vScan          = 0;
    long  _vTotals[3]     = {0,0,0};

    // 磁偏角
    float _magneticDeclinationDegrees = 0;

    // 16向位标记
    const char _bearings[16][3] = {
        {'N',' ',' '},{'N','N','E'},{' ','N','E'},{'E','N','E'},
        {' ',' ','E'},{'E','S','E'},{' ','S','E'},{'S','S','E'},
        {' ',' ','S'},{'S','S','W'},{' ','S','W'},{'W','S','W'},
        {' ',' ','W'},{'W','N','W'},{' ','N','W'},{'N','N','W'}
    };
};

#endif
