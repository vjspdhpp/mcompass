#ifndef QMC5883P_COMPASS_H
#define QMC5883P_COMPASS_H

#include <Arduino.h>
#include <Wire.h>

/* ===========================================================================================
   QMC5883P 三轴磁力计驱动（头文件）
   关键特性：
     · 支持设置量程 / 采样率 / OSR / 地磁偏角
     · 支持滚动平均或高级平滑
     · 支持六点硬铁校准、偏移/比例校正
     · 输出 0-359° 方位角、16 方位索引、三字母方位字符串
   =========================================================================================*/
class QMC5883PCompass
{
public:
    /* ---------- 构造 & 初始化 ---------- */
    QMC5883PCompass();
    void  init();                                 // 默认连续测量 200 Hz、8 G

    /* ---------- 寄存器相关 ---------- */
    void  setADDR(byte addr = 0x2C);              // 改 I²C 地址（默认 0x2C）
    void  setMode(byte mode, byte odr, byte rng,
                  byte osr);                      // 快速自定义寄存器 0x0A/0x0B
    void  setReset();                             // 软复位

    /* ---------- 地磁偏角 ---------- */
    void  setMagneticDeclination(int deg,
                                 uint8_t min = 0);

    /* ---------- 平滑（滚动平均/高级平滑） ---------- */
    void  setSmoothing(byte steps = 5,
                       bool adv = false);

    /* ---------- 读取接口 ---------- */
    void  read();                                 // 更新原始值（必须先调）
    int   getX();
    int   getY();
    int   getZ();
    int   getAzimuth();                           // 0-359°
    byte  getBearing(int azimuth);                // 0-15
    void  getDirection(char *buf, int azimuth);   // 三字符，如 “ N ”

    /* ---------- 校准（硬铁 / 软铁） ---------- */
    void  calibrate();                            // 自动扫描 30 s
    void  setCalibration(int x_min, int x_max,
                         int y_min, int y_max,
                         int z_min, int z_max);
    void  setCalibrationOffsets(float x_off,
                                float y_off,
                                float z_off);
    void  setCalibrationScales (float x_scl,
                                float y_scl,
                                float z_scl);
    float getCalibrationOffset(uint8_t idx);      // 0:X 1:Y 2:Z
    float getCalibrationScale (uint8_t idx);      // 0:X 1:Y 2:Z
    void  clearCalibration();

    /* ---------- 芯片 ID ---------- */
    char  chipID();                               // 读取 0x0D

private:
    /* --- 私有工具函数 --- */
    void  _writeReg(byte reg, byte val);
    int   _get(int idx);
    void  _applyCalibration();
    void  _smoothing();

    /* --- 常量 / 参数 --- */
    byte  _ADDR           = 0x2C;                 // I²C 地址
    float _magneticDeclinationDegrees = 0.0f;

    /* --- 平滑相关 --- */
    bool  _smoothUse      = false;
    bool  _smoothAdvanced = false;
    byte  _smoothSteps    = 5;
    int   _vHistory[10][3];
    int   _vScan          = 0;
    long  _vTotals[3]     = {0, 0, 0};
    int   _vSmooth[3]     = {0, 0, 0};

    /* --- 原始 / 校准数据缓存 --- */
    int   _vRaw[3]        = {0, 0, 0};
    float _offset[3]      = {0.0f, 0.0f, 0.0f};
    float _scale[3]       = {1.0f, 1.0f, 1.0f};
    int   _vCalibrated[3] = {0, 0, 0};

    /* --- 16 方位字符表 --- */
    const char _bearings[16][3] = {
        {' ', ' ', 'N'},  {'N', 'N', 'E'},  {' ', 'N', 'E'},  {'E', 'N', 'E'},
        {' ', ' ', 'E'},  {'E', 'S', 'E'},  {' ', 'S', 'E'},  {'S', 'S', 'E'},
        {' ', ' ', 'S'},  {'S', 'S', 'W'},  {' ', 'S', 'W'},  {'W', 'S', 'W'},
        {' ', ' ', 'W'},  {'W', 'N', 'W'},  {' ', 'N', 'W'},  {'N', 'N', 'W'},
    };
};

#endif /* QMC5883P_COMPASS_H */
