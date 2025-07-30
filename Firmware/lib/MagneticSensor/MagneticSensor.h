#ifndef MAGNETIC_SENSOR_H
#define MAGNETIC_SENSOR_H

#include "Arduino.h"
#include "Wire.h"

class MagneticSensor {
public:
  MagneticSensor() {}

  virtual ~MagneticSensor() = default;

  // --- 传感器初始化与配置 ---
  virtual void init() = 0;
  virtual void setMode(byte mode, byte odr, byte rng, byte osr) = 0;
  virtual void setMagneticDeclination(int degrees, uint8_t minutes) {
    _magneticDeclinationDegrees = degrees + (float)minutes / 60.0;
  }
  virtual void setSmoothing(byte steps, bool adv) {
    _smoothUse = true;
    _smoothSteps = steps;
    _smoothAdvanced = adv;
  }
  virtual void clearSmoothing() { _smoothUse = false; }

  // --- 校准相关 ---
  virtual void calibrate() = 0;
  virtual void setCalibration(int x_min, int x_max, int y_min, int y_max,
                              int z_min, int z_max) {
    _offset[0] = (float)(x_min + x_max) / 2.0;
    _offset[1] = (float)(y_min + y_max) / 2.0;
    _offset[2] = (float)(z_min + z_max) / 2.0;
    _scale[0] = (float)2.0 / (x_max - x_min);
    _scale[1] = (float)2.0 / (y_max - y_min);
    _scale[2] = (float)2.0 / (z_max - z_min);
  }
  virtual void setCalibrationOffsets(float x_offset, float y_offset,
                                     float z_offset) {
    _offset[0] = x_offset;
    _offset[1] = y_offset;
    _offset[2] = z_offset;
  }
  virtual void setCalibrationScales(float x_scale, float y_scale,
                                    float z_scale) {
    _scale[0] = x_scale;
    _scale[1] = y_scale;
    _scale[2] = z_scale;
  }
  virtual float getCalibrationOffset(uint8_t index) {
    if (index < 3)
      return _offset[index];
    return 0.0;
  }
  virtual float getCalibrationScale(uint8_t index) {
    if (index < 3)
      return _scale[index];
    return 0.0;
  }
  virtual void clearCalibration() {
    _offset[0] = _offset[1] = _offset[2] = 0.0;
    _scale[0] = _scale[1] = _scale[2] = 1.0;
  }

  // --- 数据读取与处理 ---
  virtual void setReset() = 0;
  virtual void read() = 0;
  virtual int getX() {
    _applyCalibration();
    return _vCalibrated[0];
  }
  virtual int getY() {
    _applyCalibration();
    return _vCalibrated[1];
  }
  virtual int getZ() {
    _applyCalibration();
    return _vCalibrated[2];
  }
  virtual int getAzimuth();
  virtual byte getBearing(int azimuth);
  virtual void getDirection(char *myArray, int azimuth);
  virtual char chipID() = 0;

protected:

  float _magneticDeclinationDegrees = 0;
  bool _smoothUse = false;
  byte _smoothSteps = 5;
  bool _smoothAdvanced = false;
  int _vRaw[3] = {0, 0, 0};
  int _vHistory[10][3];
  int _vScan = 0;
  long _vTotals[3] = {0, 0, 0};
  int _vSmooth[3] = {0, 0, 0};
  float _offset[3] = {0., 0., 0.};
  float _scale[3] = {1., 1., 1.};
  int _vCalibrated[3];

  virtual void _smoothing();
  virtual void _applyCalibration();

  const char _bearings[16][3] = {
      {' ', ' ', 'N'}, {'N', 'N', 'E'}, {' ', 'N', 'E'}, {'E', 'N', 'E'},
      {' ', ' ', 'E'}, {'E', 'S', 'E'}, {' ', 'S', 'E'}, {'S', 'S', 'E'},
      {' ', ' ', 'S'}, {'S', 'S', 'W'}, {' ', 'S', 'W'}, {'W', 'S', 'W'},
      {' ', ' ', 'W'}, {'W', 'N', 'W'}, {' ', 'N', 'W'}, {'N', 'N', 'W'},
  };
};

#endif // MAGNETIC_SENSOR_H