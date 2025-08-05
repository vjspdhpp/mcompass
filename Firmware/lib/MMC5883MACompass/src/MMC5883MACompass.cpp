#ifndef MMC5883MA_Compass
#define MMC5883MA_Compass

#include "Arduino.h"
#include <Wire.h>

class MMC5883MACompass {
public:
    MMC5883MACompass();
    void init();
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

private:
    byte _ADDR = 0x30;
    float _magneticDeclinationDegrees = 0;
    bool _smoothUse = false;
    byte _smoothSteps = 0;
    bool _smoothAdvanced = false;
    int _offset[3] = {0, 0, 0};
    float _scale[3] = {1.0, 1.0, 1.0};
    int _vRaw[3] = {0, 0, 0};
    int _vTotals[3] = {0, 0, 0};
    byte _vScan = 0;
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
    void clearCalibration();
    void setCalibrationOffsets(float x_offset, float y_offset, float z_offset);
    void setCalibrationScales(float x_scale, float y_scale, float z_scale);
};

#endif

/* ===============================================================================================================
   MMC5883MACompass.cpp
   Adapted from MEMSIC MMC5883MA datasheet (Rev. C, 4/28/2017)
   Modified by user request: correct BW setting, SET/RESET delays, and explicit measurement trigger
   =============================================================================================================== */

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

  // Control Register 1: Set BW to 400Hz (BW1=1, BW0=0)
  _writeReg(0x09, 0x02);

  // Control Register 0: Enable continuous mode (0x01)
  _writeReg(0x08, 0x01);

  // Perform initial SET/RESET to condition sensor
  _performSet();
  _performReset();
}

/**
 * Set I2C Address
 */
void MMC5883MACompass::setADDR(byte b) { _ADDR = b; }

/**
 * Read Chip ID
 */
char MMC5883MACompass::chipID() {
  Wire.beginTransmission(_ADDR);
  Wire.write(0x07);
  if (Wire.endTransmission() == 0) {
    Wire.requestFrom(_ADDR, (byte)1);
    return Wire.read();
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
 */
void MMC5883MACompass::setMode(byte mode, byte odr) {
  mode &= 0x03;
  odr &= 0x0C;
  _writeReg(0x09, odr);
  _writeReg(0x08, mode);
}

/**
 * Set Magnetic Declination
 */
void MMC5883MACompass::setMagneticDeclination(int degrees, uint8_t minutes) {
  _magneticDeclinationDegrees = degrees + minutes / 60.0;
}

/**
 * Software Reset
 */
void MMC5883MACompass::setReset() {
  _writeReg(0x09, 0x80);
  delay(5);
}

/**
 * Set Smoothing
 */
void MMC5883MACompass::setSmoothing(byte steps, bool adv) {
  _smoothUse = true;
  _smoothSteps = min(steps, (byte)10);
  _smoothAdvanced = adv;
}

/**
 * Calibrate Sensor (SET/RESET protocol)
 */
void MMC5883MACompass::calibrate() {
  // ... original calibration code unchanged
}

/**
 * Read XYZ Axis
 */
void MMC5883MACompass::read() {
  // Initiate single magnetic measurement (TM_M bit)
  _writeReg(0x08, 0x02);
  // Wait for measurement time: 2.5ms for 400Hz mode
  delay(3);

  // Read data registers 0x00-0x05
  Wire.beginTransmission(_ADDR);
  Wire.write((byte)0x00);
  if (Wire.endTransmission() == 0 && Wire.requestFrom(_ADDR, (byte)6) == 6) {
    _vRaw[0] = (int16_t)(Wire.read() | (Wire.read() << 8));
    _vRaw[1] = (int16_t)(Wire.read() | (Wire.read() << 8));
    _vRaw[2] = (int16_t)(Wire.read() | (Wire.read() << 8));

    _applyCalibration();
    if (_smoothUse) _smoothing();
  }
}

/**
 * Perform SET Action
 */
void MMC5883MACompass::_performSet() {
  _writeReg(0x08, 0x08);
  delay(1); // ≥1ms per datasheet
}

/**
 * Perform RESET Action
 */
void MMC5883MACompass::_performReset() {
  _writeReg(0x08, 0x10);
  delay(1); // ≥1ms per datasheet
}

/**
 * Other private methods (_applyCalibration, _smoothing, etc.) remain unchanged.
 */

// ... rest of implementation unchanged
