#ifndef MMC5883MA_Compass
#define MMC5883MA_Compass

#include "Arduino.h"
#include <Wire.h>

class MMC5883MACompass {
public:
    MMC5883MACompass();

    /* 初始化与配置 --------------------------------------------------- */
    void init();                                            // I²C+软复位
    void setADDR(byte b);                                   // 若需改地址
    void setMode(byte mode, byte odr = 0x04);               // 持续/单次模式
    void setMagneticDeclination(int deg, uint8_t min = 0);  // 磁偏角
    void setSmoothing(byte steps = 5, bool adv = false);    // 滑动平均
    void setReset();                                        // 软件复位

    /* 校准 ----------------------------------------------------------- */
    void calibrate();                                       // 8 字旋转 10 s
    void setCalibration(int xmin,int xmax,int ymin,int ymax,
                        int zmin,int zmax);
    void setCalibrationOffsets(float x,float y,float z);
    void setCalibrationScales (float xs,float ys,float zs);
    float getCalibrationOffset(uint8_t idx);
    float getCalibrationScale (uint8_t idx);
    void  clearCalibration();

    /* 数据读取 ------------------------------------------------------- */
    void read();                                            // 更新内部缓冲
    int  getX();
    int  getY();
    int  getZ();
    int  getAzimuth();                                      // 0-359°
    byte getBearing(int azimuth);                           // 0-15
    void getDirection(char* out, int azimuth);              // N / ENE…

    /* 调试 ----------------------------------------------------------- */
    char chipID();                                          // 0x0C

private:
    /* I²C 帮手 */
    void _writeReg(byte reg, byte val);
    uint8_t _readStatus();
    void _burstRead(int16_t v[3]);
    void _triggerMeasure();                                 // TM_M=1
    void _performSet();
    void _performReset();

    /* 内部工具 */
    int  _get(int idx);
    void _applyCalibration();
    void _smoothing();

    /* 常量与状态 */
    static constexpr byte REG_DATA   = 0x00;      // 数据首址
    static constexpr byte REG_STATUS = 0x07;
    static constexpr byte REG_CTRL0  = 0x08;
    static constexpr byte REG_CTRL1  = 0x09;
    static constexpr byte REG_CTRL2  = 0x0A;
    static constexpr byte REG_ID     = 0x2F;

    static constexpr byte CTRL0_TM_M = 0x01;
    static constexpr byte CTRL0_SET  = 0x08;
    static constexpr byte CTRL0_RST  = 0x10;

    byte  _ADDR = 0x30;          /* 默认 I²C 地址 */
    float _decl = 0;             /* 磁偏角 */
    bool  _useSmooth = false;
    byte  _smoothN   = 5;
    bool  _smoothAdv = false;

    int   _raw [3]   = {0};
    int   _cal [3]   = {0};
    int   _smooth[3] = {0};
    long  _tot  [3]  = {0};
    int   _hist [10][3] {};
    byte  _scan = 0;
    float _off  [3] = {0,0,0};
    float _scale[3] = {1,1,1};

    static constexpr char _bearingName[16][3] = {
        {' ',' ','N'},{'N','N','E'},{' ','N','E'},{'E','N','E'},
        {' ',' ','E'},{'E','S','E'},{' ','S','E'},{'S','S','E'},
        {' ',' ','S'},{'S','S','W'},{' ','S','W'},{'W','S','W'},
        {' ',' ','W'},{'W','N','W'},{' ','N','W'},{'N','N','W'}
    };
};

#endif
