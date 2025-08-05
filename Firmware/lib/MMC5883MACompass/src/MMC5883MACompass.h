// 文件: MMC5883MACompass.h
#ifndef MMC5883MA_COMPASS_H
#define MMC5883MA_COMPASS_H

#include "Arduino.h"
#include <Wire.h>

class MMC5883MACompass {
public:
    MMC5883MACompass();

    /**
     * 初始化传感器
     */
    void init();

    /** 软件复位 */
    void setReset();

    /** 设置I2C地址 */
    void setADDR(byte b);

    /** 设置模式和输出数据率 */
    void setMode(byte mode, byte odr);

    /** 设置磁偏角 */
    void setMagneticDeclination(int degrees, uint8_t minutes);

    /** 设置平滑参数 */
    void setSmoothing(byte steps, bool adv);

    /** 校准 (用户可实现SET/RESET校准流程) */
    void calibrate();

    /** 设置校准值 */
    void setCalibration(int x_min, int x_max, int y_min, int y_max, int z_min, int z_max);

    /** 触发测量并读取 */
    void read();

    /** 读取芯片ID */
    char chipID();

    /** 获取方位角 */
    int getAzimuth();

    /** 罗盘方向索引 */
    byte getBearing(int azimuth);

    /** 获取方向文字 */
    void getDirection(char *myArray, int azimuth);

    /** 访问校准偏移 */
    float getCalibrationOffset(byte idx);

    /** 访问校准比例 */
    float getCalibrationScale(byte idx);

    /** 获取X轴数值 */
    int getX();
    /** 获取Y轴数值 */
    int getY();
    /** 获取Z轴数值 */
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
    const char _bearings[16][3] = {
        {' ', ' ', 'N'}, {' ', 'N', 'N'}, {'N', 'N', 'E'}, {' ', 'N', 'E'},
        {'E', 'N', 'E'}, {' ', 'E', ' '}, {'E', 'S', 'E'}, {' ', 'S', 'E'},
        {'S', 'S', 'E'}, {' ', ' ', 'S'}, {'S', 'S', 'W'}, {' ', 'S', 'W'},
        {'W', 'S', 'W'}, {' ', ' ', 'W'}, {'W', 'N', 'W'}, {' ', 'N', 'W'}
    };

    void _writeReg(byte reg, byte val);
    void _performSet();
    void _performReset();
    void _applyCalibration();
    void _smoothing();
};

#endif // MMC5883MA_COMPASS_H
