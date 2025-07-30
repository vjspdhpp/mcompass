/*
===============================================================================================================
QMC5883PCompass.cpp
Library for using QMC5883P series chip boards as a compass.
This is a modified version of the QMC5883LCompass library to support the
QMC5883P chip.

Learn more at [https://github.com/mprograms/QMC5883LCompass]

Supports:

- Getting values of XYZ axis.
- Calculating Azimuth.
- Getting 16 point Azimuth bearing direction (0 - 15).
- Getting 16 point Azimuth bearing Names (N, NNE, NE, ENE, E, ESE, SE, SSE, S,
SSW, SW, WSW, W, WNW, NW, NNW)
- Smoothing of XYZ readings via rolling averaging and min / max removal.
- Optional chipset modes (see below)

===============================================================================================================

v1.0 - June 13, 2019
Written by MPrograms
Github: [https://github.com/mprograms/]


Release under the GNU General Public License v3
[https://www.gnu.org/licenses/gpl-3.0.en.html]

===============================================================================================================

FROM QST QMC5883P Datasheet
-----------------------------------------------
-- REGISTER 0x0A --
 MODE CONTROL (MODE)
        Suspend			0x00 (0b00)
        Normal		    0x01 (0b01)
        Single          0x02 (0b10)
        Continuous		0x03 (0b11)

OUTPUT DATA RATE (ODR)
        10Hz        	0x00 (0b0000)
        50Hz        	0x04 (0b0100)
        100Hz       	0x08 (0b1000)
        200Hz       	0x0C (0b1100)

OVER SAMPLE RATIO 1 (OSR1)
        8         	    0x00 (0b000000)
        4         	    0x10 (0b010000)
        2         	    0x20 (0b100000)
        1          	    0x30 (0b110000)

-- REGISTER 0x0B --
FULL SCALE (RNG)
        2G          	0x0C (0b1100)
        8G          	0x08 (0b1000)
        12G             0x04 (0b0100)
        30G             0x00 (0b0000)

SET/RESET MODE (SET/RESET MODE)
        Set and reset on    0x01 (0b01)
        Set only on         0x02 (0b10)
        Set and reset off   0x00 (0b00) or 0x03 (0b11)

*/

#include "QMC5883PCompass.h"
#include "Arduino.h"
#include <Wire.h>

QMC5883PCompass::QMC5883PCompass() {}

/**
        INIT
        Initialize Chip - This needs to be called in the sketch setup()
function. This sequence is changed for the QMC5883P chip.
**/
void QMC5883PCompass::init() {
  Wire.begin();

  // Register 0B: Set Range to 8G (0x08) and SET/RESET mode to 'on' (0x01)
  _writeReg(0x0B, 0x08 | 0x01);

  // Register 0A: Set Mode to Continuous (0x03), ODR to 200Hz (0x0C), OSR1 to 8
  // (0x00)
  _writeReg(0x0A, 0x03 | 0x0C | 0x00);
}

/**
        SET ADDRESS
        Set the I2C Address of the chip. This needs to be called in the sketch
setup() function.
**/
void QMC5883PCompass::setADDR(byte b) { _ADDR = b; }

/**
 * Read Chip ID, should be 0x80 for QMC5883P.
 * Changed register from 0x0D to 0x00.
 */
char QMC5883PCompass::chipID() {
  Wire.beginTransmission(_ADDR);
  Wire.write(0x00); // Chip ID is at address 0x00 for QMC5883P
  int err = Wire.endTransmission();
  Wire.requestFrom(_ADDR, (byte)1);
  char buf[1] = {0};
  Wire.readBytes(buf, 1);
  return buf[0];
}

/**
        REGISTER
        Write the register to the chip. (No change in logic)
**/
void QMC5883PCompass::_writeReg(byte r, byte v) {
  Wire.beginTransmission(_ADDR);
  Wire.write(r);
  Wire.write(v);
  Wire.endTransmission();
}

/**
        CHIP MODE
        Set the chip mode.
        This function is rewritten for QMC5883P as settings are in two
registers. Note: The values for rng and osr parameters are different from the
QMC5883L version. See the comment block at the top of this file. This function
also hard-codes SET/RESET mode to 'on'.
**/
void QMC5883PCompass::setMode(byte mode, byte odr, byte rng, byte osr) {
  byte regA_val = mode | odr | osr;
  byte regB_val = rng | 0x01; // SET/RESET mode 'on'

  _writeReg(0x0B, regB_val);
  _writeReg(0x0A, regA_val);
}

/**
 * Define the magnetic declination for accurate degrees. (No change in logic)
 * https://www.magnetic-declination.com/
 */
void QMC5883PCompass::setMagneticDeclination(int degrees, uint8_t minutes) {
  _magneticDeclinationDegrees = (float)degrees + (float)minutes / 60.0;
}

/**
        RESET
        Reset the chip.
        Changed register from 0x0A to 0x0B.
**/
void QMC5883PCompass::setReset() { _writeReg(0x0B, 0x80); }

// 1 = Basic 2 = Advanced. (No change in logic)
void QMC5883PCompass::setSmoothing(byte steps, bool adv) {
  _smoothUse = true;
  _smoothSteps = (steps > 10) ? 10 : steps;
  _smoothAdvanced = (adv == true) ? true : false;
}

// (No change in logic)
void QMC5883PCompass::calibrate() {
  clearCalibration();
  Serial.println("Calibrating QMC5883P...clearCalibration");
  long calibrationData[3][2] = {
      {65000, -65000}, {65000, -65000}, {65000, -65000}};

  // Prime the values
  read();
  long x = calibrationData[0][0] = calibrationData[0][1] = getX();
  long y = calibrationData[1][0] = calibrationData[1][1] = getY();
  long z = calibrationData[2][0] = calibrationData[2][1] = getZ();

  unsigned long startTime = millis();

  while ((millis() - startTime) < 10000) {
    read();

    x = getX();
    y = getY();
    z = getZ();

    if (x < calibrationData[0][0]) {
      calibrationData[0][0] = x;
    }
    if (x > calibrationData[0][1]) {
      calibrationData[0][1] = x;
    }

    if (y < calibrationData[1][0]) {
      calibrationData[1][0] = y;
    }
    if (y > calibrationData[1][1]) {
      calibrationData[1][1] = y;
    }

    if (z < calibrationData[2][0]) {
      calibrationData[2][0] = z;
    }
    if (z > calibrationData[2][1]) {
      calibrationData[2][1] = z;
    }
  }

  setCalibration(calibrationData[0][0], calibrationData[0][1],
                 calibrationData[1][0], calibrationData[1][1],
                 calibrationData[2][0], calibrationData[2][1]);
}

/**
    SET CALIBRATION (No change in logic)
**/
void QMC5883PCompass::setCalibration(int x_min, int x_max, int y_min, int y_max,
                                     int z_min, int z_max) {
  setCalibrationOffsets((x_min + x_max) / 2.0, (y_min + y_max) / 2.0,
                        (z_min + z_max) / 2.0);

  float x_avg_delta = (x_max - x_min) / 2.0;
  float y_avg_delta = (y_max - y_min) / 2.0;
  float z_avg_delta = (z_max - z_min) / 2.0;

  float avg_delta = (x_avg_delta + y_avg_delta + z_avg_delta) / 3.0;

  setCalibrationScales(avg_delta / x_avg_delta, avg_delta / y_avg_delta,
                       avg_delta / z_avg_delta);
}

// (No change in logic)
void QMC5883PCompass::setCalibrationOffsets(float x_offset, float y_offset,
                                            float z_offset) {
  _offset[0] = x_offset;
  _offset[1] = y_offset;
  _offset[2] = z_offset;
  Serial.print("Calibration Offsets: ");
  Serial.print("X: ");
  Serial.print(_offset[0]);
  Serial.print(", Y: ");
  Serial.print(_offset[1]);
  Serial.print(", Z: ");
  Serial.println(_offset[2]);
  Serial.flush(); // Ensure all data is sent before proceeding
}

// (No change in logic)
void QMC5883PCompass::setCalibrationScales(float x_scale, float y_scale,
                                           float z_scale) {
  _scale[0] = x_scale;
  _scale[1] = y_scale;
  _scale[2] = z_scale;
  Serial.print("Calibration Scales: ");
  Serial.print("X: ");
  Serial.print(_scale[0]);
  Serial.print(", Y: ");
  Serial.print(_scale[1]);
  Serial.print(", Z: ");
  Serial.println(_scale[2]);
  Serial.flush(); // Ensure all data is sent before proceeding
}

// (No change in logic)
float QMC5883PCompass::getCalibrationOffset(uint8_t index) {
  return _offset[index];
}

// (No change in logic)
float QMC5883PCompass::getCalibrationScale(uint8_t index) {
  return _scale[index];
}

// (No change in logic)
void QMC5883PCompass::clearCalibration() {
  setCalibrationOffsets(0., 0., 0.);
  setCalibrationScales(1., 1., 1.);
}

/**
        READ
        Read the XYZ axis and save the values in an array.
        Changed data start register from 0x00 to 0x01.
**/
void QMC5883PCompass::read() {
  Wire.beginTransmission(_ADDR);
  Wire.write(0x01); // Set read pointer to 0x01 (X_LSB) for QMC5883P
  int err = Wire.endTransmission();
  if (!err) {
    Wire.requestFrom(_ADDR, (byte)6);
    _vRaw[0] = (int)(int16_t)(Wire.read() | Wire.read() << 8);
    _vRaw[1] = (int)(int16_t)(Wire.read() | Wire.read() << 8);
    _vRaw[2] = (int)(int16_t)(Wire.read() | Wire.read() << 8);

    _applyCalibration();

    if (_smoothUse) {
      _smoothing();
    }
  }
}

/**
    APPLY CALIBRATION (No change in logic)
**/
void QMC5883PCompass::_applyCalibration() {
  _vCalibrated[0] = (_vRaw[0] - _offset[0]) * _scale[0];
  _vCalibrated[1] = (_vRaw[1] - _offset[1]) * _scale[1];
  _vCalibrated[2] = (_vRaw[2] - _offset[2]) * _scale[2];
}

/**
        SMOOTH OUTPUT (No change in logic)
**/
void QMC5883PCompass::_smoothing() {
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

      _vSmooth[i] = (_vTotals[i] - (_vHistory[max][i] + _vHistory[min][i])) /
                    (_smoothSteps - 2);
    } else {
      _vSmooth[i] = _vTotals[i] / _smoothSteps;
    }
  }

  _vScan++;
}

/**
        GET X AXIS (No change in logic)
**/
int QMC5883PCompass::getX() { return _get(0); }

/**
        GET Y AXIS (No change in logic)
**/
int QMC5883PCompass::getY() { return _get(1); }

/**
        GET Z AXIS (No change in logic)
**/
int QMC5883PCompass::getZ() { return _get(2); }

/**
        GET SENSOR AXIS READING (No change in logic)
**/
int QMC5883PCompass::_get(int i) {
  if (_smoothUse)
    return _vSmooth[i];

  return _vCalibrated[i];
}

/**
        GET AZIMUTH (No change in logic)
**/
int QMC5883PCompass::getAzimuth() {
  float heading = atan2(getY(), getX()) * 180.0 / PI;
  heading += _magneticDeclinationDegrees;
  return (int)heading % 360;
}

/**
        GET BEARING (No change in logic, small fix for negative azimuth)
*/
byte QMC5883PCompass::getBearing(int azimuth) {
  float a = (azimuth + 11.25) / 22.5;
  byte bearing = (int)a % 16;
  return bearing;
}

/**
        GET DIRECTION (No change in logic)
*/
void QMC5883PCompass::getDirection(char *myArray, int azimuth) {
  int d = getBearing(azimuth);
  myArray[0] = _bearings[d][0];
  myArray[1] = _bearings[d][1];
  myArray[2] = _bearings[d][2];
}
