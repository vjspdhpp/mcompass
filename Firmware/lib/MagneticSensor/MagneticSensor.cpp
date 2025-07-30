#include "MagneticSensor.h"

// 基类中 getAzimuth 的通用实现
int MagneticSensor::getAzimuth() {
    _applyCalibration(); // 确保数据已校准

    // 计算水平方向的磁场矢量
    float X_calibrated = (float)_vCalibrated[0];
    float Y_calibrated = (float)_vCalibrated[1];

    // 计算方位角 (atan2 返回弧度)
    float azimuthRadians = atan2(Y_calibrated, X_calibrated);

    // 转换为度数
    int azimuthDegrees = (int)(azimuthRadians * 180.0 / PI);

    // 调整到 0-360 度范围
    azimuthDegrees += (int)_magneticDeclinationDegrees; // 应用磁偏角
    if (azimuthDegrees < 0) azimuthDegrees += 360;
    if (azimuthDegrees >= 360) azimuthDegrees -= 360;

    return azimuthDegrees;
}

// 基类中 getBearing 的通用实现
byte MagneticSensor::getBearing(int azimuth) {
    // 将 0-360 度映射到 0-15 的象限
    return (byte)(azimuth / 22.5 + 0.5); // +0.5 用于四舍五入
}

// 基类中 getDirection 的通用实现
void MagneticSensor::getDirection(char* myArray, int azimuth) {
    byte index = getBearing(azimuth);
    myArray[0] = _bearings[index][0];
    myArray[1] = _bearings[index][1];
    myArray[2] = _bearings[index][2];
    myArray[3] = '\0'; // 确保字符串以空字符结尾
}

// 基类中 _smoothing 的通用实现
void MagneticSensor::_smoothing() {
    if (!_smoothUse) return;

    for (int i = 0; i < 3; i++) {
        _vTotals[i] -= _vHistory[_vScan][i]; // 减去最旧的值
        _vHistory[_vScan][i] = _vRaw[i];     // 存入新值
        _vTotals[i] += _vRaw[i];             // 加上新值
        _vSmooth[i] = _vTotals[i] / _smoothSteps; // 计算平均值
    }

    _vScan++;
    if (_vScan >= _smoothSteps) {
        _vScan = 0; // 循环使用历史缓冲区
    }
}

// 基类中 _applyCalibration 的通用实现
void MagneticSensor::_applyCalibration() {
    // 如果使用了平滑，则对平滑后的数据进行校准；否则对原始数据进行校准
    int* dataToCalibrate = _smoothUse ? _vSmooth : _vRaw;

    _vCalibrated[0] = (int)((dataToCalibrate[0] - _offset[0]) * _scale[0]);
    _vCalibrated[1] = (int)((dataToCalibrate[1] - _offset[1]) * _scale[1]);
    _vCalibrated[2] = (int)((dataToCalibrate[2] - _offset[2]) * _scale[2]);
}