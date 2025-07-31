#ifndef QMC5883P_Compass
#define QMC5883P_Compass

#include "Arduino.h"
#include "Wire.h"

class QMC5883PCompass{
public:
    QMC5883PCompass();
    void init();
    void setADDR(byte b);
    void setMode(byte mode, byte odr, byte rng, byte osr);
    void setMagneticDeclination(int degrees, uint8_t minutes);
    void setSmoothing(byte steps, bool adv);
    void calibrate();
    void setCalibration(int x_min, int x_max, int y_min, int y_max, int z_min, int z_max);
    void setCalibrationOffsets(float x_offset, float y_offset, float z_offset);
    void setCalibrationScales(float x_scale, float y_scale, float z_scale);
    float getCalibrationOffset(uint8_t index);
    float getCalibrationScale(uint8_t index);
    void clearCalibration();
    void setReset();
    void read();
    int getX();
    int getY();
    int getZ();
    int getAzimuth();
    byte getBearing(int azimuth);
    void getDirection(char* myArray, int azimuth);
    char chipID();

    // --- 新增：轴重映射接口 ---
    enum Axis : uint8_t { AXIS_X = 0, AXIS_Y = 1, AXIS_Z = 2 };
    // 将“板级X/Y/Z”分别映射到“芯片axisSrc”，并指定符号（+1 或 -1）
    void setAxisRemap(uint8_t boardX_src, int8_t boardX_sign,
                      uint8_t boardY_src, int8_t boardY_sign,
                      uint8_t boardZ_src, int8_t boardZ_sign);

private:
    void _writeReg(byte reg,byte val);
    int _get(int index);
    void _smoothing();
    void _applyCalibration();

    float _magneticDeclinationDegrees = 0;
    bool  _smoothUse = false;
    byte  _smoothSteps = 5;
    bool  _smoothAdvanced = false;
    byte  _ADDR = 0x2C; // QMC5883P 默认地址

    // 原始芯片读数（芯片坐标）
    int   _vRaw[3] = {0,0,0};
    // 经过坐标重映射后的“板级坐标”读数（作为校准与输出的基准）
    int   _vMapped[3] = {0,0,0};

    // 平滑用
    int   _vHistory[10][3];
    int   _vScan = 0;
    long  _vTotals[3] = {0,0,0};
    int   _vSmooth[3] = {0,0,0};

    // 校准参数（针对“板级坐标”）
    float _offset[3] = {0.,0.,0.};
    float _scale[3]  = {1.,1.,1.};
    int   _vCalibrated[3];

    // 轴重映射配置（默认：板级=芯片，正号）
    uint8_t _map_src[3] = {AXIS_X, AXIS_Y, AXIS_Z}; // board X/Y/Z 来自哪个芯片轴
    int8_t  _map_sgn[3] = {+1, +1, +1};             // 对应的符号

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
