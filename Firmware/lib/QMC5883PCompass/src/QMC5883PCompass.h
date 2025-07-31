#ifndef QMC5883P_Compass
#define QMC5883P_Compass

#include "Arduino.h"
#include "Wire.h"

class QMC5883PCompass{
public:
    QMC5883PCompass();

    // 初始化与配置
    void init();
    void setADDR(byte b);
    void setMode(byte mode, byte odr, byte rng, byte osr);
    void setMagneticDeclination(int degrees, uint8_t minutes);
    void setSmoothing(byte steps, bool adv);
    void setReset();
    char chipID();

    // 读数（按板级坐标返回）
    void read();
    int  getX();
    int  getY();
    int  getZ();

    // 方位角（单位：度）
    // getAzimuth(): 以“正北为0°，顺时针增加”的导航角
    int  getAzimuth();
    // getDialAngle(): 用于“转表盘、针固定”的UI，返回应旋转的表盘角度=360°-Azimuth
    int  getDialAngle();

    // 文字方位
    byte getBearing(int azimuth);
    void getDirection(char* myArray, int azimuth);

    // 标定（针对“板级坐标”）
    void  calibrate();
    void  setCalibration(int x_min, int x_max, int y_min, int y_max, int z_min, int z_max);
    void  setCalibrationOffsets(float x_offset, float y_offset, float z_offset);
    void  setCalibrationScales(float x_scale, float y_scale, float z_scale);
    float getCalibrationOffset(uint8_t index);
    float getCalibrationScale(uint8_t index);
    void  clearCalibration();

    // --- 轴重映射（板级X/Y/Z来自芯片轴，并可取反）---
    enum Axis : uint8_t { AXIS_X = 0, AXIS_Y = 1, AXIS_Z = 2 };
    void setAxisRemap(uint8_t boardX_src, int8_t boardX_sign,
                      uint8_t boardY_src, int8_t boardY_sign,
                      uint8_t boardZ_src, int8_t boardZ_sign);

private:
    // I2C
    void _writeReg(byte reg, byte val);

    // 内部流程
    int  _get(int index);
    void _smoothing();
    void _applyCalibration();

    // 配置
    byte  _ADDR = 0x2C;                 // QMC5883P 默认 I2C 地址
    float _magneticDeclinationDegrees = 0; // 磁偏角（度）

    // 原始芯片读数（芯片坐标）
    int _vRaw[3] = {0,0,0};
    // 轴重映射后的“板级坐标”读数
    int _vMapped[3] = {0,0,0};

    // 平滑
    bool _smoothUse      = false;
    byte _smoothSteps    = 5;           // 2..10
    bool _smoothAdvanced = false;
    int  _vHistory[10][3];
    int  _vScan = 0;
    long _vTotals[3]  = {0,0,0};
    int  _vSmooth[3]  = {0,0,0};

    // 校准（针对“板级坐标”）
    float _offset[3] = {0.f,0.f,0.f};
    float _scale[3]  = {1.f,1.f,1.f};
    int   _vCalibrated[3];

    // 轴重映射配置（默认：板=芯片，正号）
    uint8_t _map_src[3] = {AXIS_X, AXIS_Y, AXIS_Z}; // board X/Y/Z 来自哪个芯片轴
    int8_t  _map_sgn[3] = {+1, +1, +1};            // 符号

    // 16方位字符
    const char _bearings[16][3] =  {
        {' ', ' ', 'N'},
        {'N', 'N', 'E'},
        {' ', 'N', 'E'},
        {'E', 'N', 'E'},
        {' ', ' ', 'E'},
        {'E', 'S', 'E'},
        {' ', 'S', 'E'},
        {'S', 'S', 'E'},
        {' ', ' ', 'S'},
        {'S', 'S', 'W'},
        {' ', 'S', 'W'},
        {'W', 'S', 'W'},
        {' ', ' ', 'W'},
        {'W', 'N', 'W'},
        {' ', 'N', 'W'},
        {'N', 'N', 'W'},
    };
};

#endif
