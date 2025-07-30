/*
===============================================================================================================
MMC5883MACompass.cpp
Library for using MMC5883MA chip as a compass.
Adapted from QMC5883PCompass library to support MMC5883MA chip.
Based on MEMSIC MMC5883MA datasheet (Rev. C, 4/28/2017).

Supports:
- Getting values of XYZ axis.
- Calculating Azimuth.
- Getting 16 point Azimuth bearing direction (0 - 15).
- Getting 16 point Azimuth bearing Names (N, NNE, NE, ENE, E, ESE, SE, SSE, S, SSW, SW, WSW, W, WNW, NW, NNW).
- Smoothing of XYZ readings via rolling averaging and min/max removal.
- SET/RESET functionality for offset elimination.

===============================================================================================================
v1.0 - July 30, 2025
Adapted for MMC5883MA by Grok
===============================================================================================================
FROM MEMSIC MMC5883MA Datasheet
-----------------------------------------------
-- REGISTER 0x08 (Control Register 0) --
  SET (0x08): Initiate SET action
  RESET (0x10): Initiate RESET action
  CM (0x01): Continuous measurement mode
  TM (0x02): Take measurement

-- REGISTER 0x09 (Control Register 1) --
  BW (Bandwidth):
    00: 100Hz, 10ms
    01: 200Hz, 5ms
    10: 400Hz, 2.5ms
    11: 600Hz, 1.6ms
  X-inhibit, Y-inhibit, Z-inhibit: Disable axis measurement
  SW_RST (0x80): Software reset

-- REGISTER 0x07 --
  Chip ID: 0xFF
-----------------------------------------------
*/

#include "MMC5883MACompass.h"
#include <Wire.h>
#include <Arduino.h>

MMC5883MACompass::MMC5883MACompass() {}

/**
 * Initialize Chip
 * Configures I2C, sets continuous mode, 400Hz ODR, and performs initial SET/RESET.
 */
void MMC5883MACompass::init() {
  Wire.begin();
  // Perform software reset
  _writeReg(0x09, 0x80);
  delay(5); // Wait for reset (5ms per datasheet)

  // Control Register 1: Set BW to 400Hz (0x10)
  _writeReg(0x09, 0x10);

  // Control Register 0: Enable continuous mode (0x01)
  _writeReg(0x08, 0x01);

  // Perform initial SET/RESET
  _performSet();
  _performReset();
}

/**
 * Set I2C Address
 */
void MMC5883MACompass::setADDR(byte b) { _ADDR = b; }

/**
 * Read Chip ID
 * Returns 0xFF for MMC5883MA
 */
char MMC5883MACompass::chipID() {
  Wire.beginTransmission(_ADDR);
  Wire.write(0x07); // Chip ID register
  int err = Wire.endTransmission();
  if (!err) {
    Wire.requestFrom(_ADDR, (byte)1);
    char buf[1] = {0};
    Wire.readBytes(buf, 1);
    return buf[0];
  }
  return 0;
}

/**
 * Write to Register
 */
void MMC5883MACompass::_writeReg(byte r, byte v) {
  Wire.beginTransmission(_ADDR);
  Wire.write(r);
  Wire.write(v);
  Wire.endTransmission();
}

/**
 * Set Mode and Output Data Rate
 * mode: 0x00 (Suspend), 0x01 (Continuous), 0x02 (Single)
 * odr: 0x00 (100Hz), 0x04 (200Hz), 0x08 (400Hz), 0x0C (600Hz)
 */
void MMC5883MACompass::setMode(byte mode, byte odr) {
  // Ensure valid mode and odr
  mode &= 0x03; // Mask to valid mode bits
  odr &= 0x0C;  // Mask to valid ODR bits
  _writeReg(0x09, odr); // Set BW in Control Register 1
  _writeReg(0x08, mode); // Set mode in Control Register 0
}

/**
 * Set Magnetic Declination
 */
void MMC5883MACompass::setMagneticDeclination(int degrees, uint8_t minutes) {
  _magneticDeclinationDegrees = (float)degrees + (float)minutes / 60.0;
}

/**
 * Software Reset
 */
void MMC5883MACompass::setReset() {
  _writeReg(0x09, 0x80);
  delay(5); // Wait for reset
}

/**
 * Set Smoothing
 */
void MMC5883MACompass::setSmoothing(byte steps, bool adv) {
  _smoothUse = true;
  _smoothSteps = (steps > 10) ? 10 : steps;
  _smoothAdvanced = adv;
}

/**
 * Calibrate Sensor
 * Uses SET/RESET protocol to eliminate offset (Page 12 of datasheet)
 */
void MMC5883MACompass::calibrate() {
  clearCalibration();
  int calibrationData[3][2] = {{32767, -32768}, {32767, -32768}, {32767, -32768}};

  // Prime the values
  read();
  int x = calibrationData[0][0] = calibrationData[0][1] = getX();
  int y = calibrationData[1][0] = calibrationData[1][1] = getY();
  int z = calibrationData[2][0] = calibrationData[2][1] = getZ();

  unsigned long startTime = millis();
  while ((millis() - startTime) < 10000) {
    _performSet();
    read();
    int x_set = getX();
    int y_set = getY();
    int z_set = getZ();

    _performReset();
    read();
    int x_reset = getX();
    int y_reset = getY();
    int z_reset = getZ();

    // Average SET and RESET measurements to remove offset
    x = (x_set + x_reset) / 2;
    y = (y_set + y_reset) / 2;
    z = (z_set + z_reset) / 2;

    if (x < calibrationData[0][0]) calibrationData[0][0] = x;
    if (x > calibrationData[0][1]) calibrationData[0][1] = x;
    if (y < calibrationData[1][0]) calibrationData[1][0] = y;
    if (y > calibrationData[1][1]) calibrationData[1][1] = y;
    if (z < calibrationData[2][0]) calibrationData[2][0] = z;
    if (z > calibrationData[2][1]) calibrationData[2][1] = z;

    delay(10); // Allow sensor to stabilize
  }

  setCalibration(calibrationData[0][0], calibrationData[0][1],
                 calibrationData[1][0], calibrationData[1][1],
                 calibrationData[2][0], calibrationData[2][1]);
}

/**
 * Set Calibration
 */
void MMC5883MACompass::setCalibration(int x_min, int x_max, int y_min, int y_max, int z_min, int z_max) {
  setCalibrationOffsets((x_min + x_max) / 2.0, (y_min + y_max) / 2.0, (z_min + z_max) / 2.0);

  float x_avg_delta = (x_max - x_min) / 2.0;
  float y_avg_delta = (y_max - y_min) / 2.0;
  float z_avg_delta = (z_max - z_min) / 2.0;

  float avg_delta = (x_avg_delta + y_avg_delta + z_avg_delta) / 3.0;

  setCalibrationScales(avg_delta / x_avg_delta, avg_delta / y_avg_delta, avg_delta / z_avg_delta);
}

/**
 * Set Calibration Offsets
 */
void MMC5883MACompass::setCalibrationOffsets(float x_offset, float y_offset, float z_offset) {
  _offset[0] = x_offset;
  _offset[1] = y_offset;
  _offset[2] = z_offset;
}

/**
 * Set Calibration Scales
 */
void MMC5883MACompass::setCalibrationScales(float x_scale, float y_scale, float z_scale) {
  _scale[0] = x_scale;
  _scale[1] = y_scale;
  _scale[2] = z_scale;
}

/**
 * Get Calibration Offset
 */
float MMC5883MACompass::getCalibrationOffset(uint8_t index) {
  return _offset[index];
}

/**
 * Get Calibration Scale
 */
float MMC5883MACompass::getCalibrationScale(uint8_t index) {
  return _scale[index];
}

/**
 * Clear Calibration
 */
void MMC5883MACompass::clearCalibration() {
  setCalibrationOffsets(0., 0., 0.);
  setCalibrationScales(1., 1., 1.);
}

/**
 * Read XYZ Axis
 */
void MMC5883MACompass::read() {
  _performSet(); // Perform SET before measurement
  Wire.beginTransmission(_ADDR);
  Wire.write(0x00); // Data output register
  int err = Wire.endTransmission();
  if (!err) {
    Wire.requestFrom(_ADDR, (byte)6);
    _vRaw[0] = (int)(int16_t)(Wire.read() | Wire.read() << 8);
    _vRaw[1] = (int)(int16_t)(Wire.read() | Wire.read() << 8);
    _vRaw[2] = (int)(int16_t)(Wire.read() | Wire.read() << 8);
    _performReset(); // Perform RESET after measurement
    _applyCalibration();
    if (_smoothUse) {
      _smoothing();
    }
  }
}

/**
 * Perform SET Action
 */
void MMC5883MACompass::_performSet() {
  _writeReg(0x08, 0x08); // SET bit
  delayMicroseconds(100); // Short delay for SET action
}

/**
 * Perform RESET Action
 */
void MMC5883MACompass::_performReset() {
  _writeReg(0x08, 0x10); // RESET bit
  delayMicroseconds(100); // Short delay for RESET action
}

/**
 * Apply Calibration
 */
void MMC5883MACompass::_applyCalibration() {
  _vCalibrated[0] = (_vRaw[0] - _offset[0]) * _scale[0];
  _vCalibrated[1] = (_vRaw[1] - _offset[1]) * _scale[1];
  _vCalibrated[2] = (_vRaw[2] - _offset[2]) * _scale[2];
}

/**
 * Smooth Output
 */
void MMC5883MACompass::_smoothing() {
  byte max = 0;
  byte min = 0;

  if (_vScan > _smoothSteps - 1) {
    _vScan = 0;
  }

  for (int i = 0; i < 3; i++) {
    if (_vTotals[i] != 0) {
      _vTotals[i] = _vTotals[i] - _vHistory[_vScan][i];
    }
    _vHistory[_vScan][i] = _vCalibrated[i];
    _vTotals[i] = _vTotals[i] + _vHistory[_vScan][i];

    if (_smoothAdvanced) {
      max = 0;
      for (int j = 0; j < _smoothSteps - 1; j++) {
        max = (_vHistory[j][i] > _vHistory[max][i]) ? j : max;
      }

      min = 0;
      for (int k = 0; k < _smoothSteps - 1; k++) {
        min = (_vHistory[k][i] < _vHistory[min][i]) ? k : min;
      }

      _vSmooth[i] = (_vTotals[i] - (_vHistory[max][i] + _vHistory[min][i])) / (_smoothSteps - 2);
    } else {
      _vSmooth[i] = _vTotals[i] / _smoothSteps;
    }
  }

  _vScan++;
}

/**
 * Get X Axis
 */
int MMC5883MACompass::getX() { return _get(0); }

/**
 * Get Y Axis
 */
int MMC5883MACompass::getY() { return _get(1); }

/**
 * Get Z Axis
 */
int MMC5883MACompass::getZ() { return _get(2); }

/**
 * Get Sensor Axis Reading
 */
int MMC5883MACompass::_get(int i) {
  if (_smoothUse)
    return _vSmooth[i];
  return _vCalibrated[i];
}

/**
 * Get Azimuth
 */
int MMC5883MACompass::getAzimuth() {
  float heading = atan2(getY(), getX()) * 180.0 / PI;
  heading += _magneticDeclinationDegrees;
  if (heading < 0) heading += 360.0;
  return (int)heading % 360;
}

/**
 * Get Bearing
 */
byte MMC5883MACompass::getBearing(int azimuth) {
  float a = (azimuth + 11.25) / 22.5;
  byte bearing = (int)a % 16;
  return bearing;
}

/**
 * Get Direction
 */
void MMC5883MACompass::getDirection(char *myArray, int azimuth) {
  int d = getBearing(azimuth);
  myArray[0] = _bearings[d][0];
  myArray[1] = _bearings[d][1];
  myArray[2] = _bearings[d][2];
}