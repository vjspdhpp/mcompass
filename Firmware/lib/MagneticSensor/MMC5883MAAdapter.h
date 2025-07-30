#ifndef MMC5883MA_ADAPTER_H
#define MMC5883MA_ADAPTER_H

#include "MagneticSensor.h"
#include "MMC5883MACompass.h"

class MMC5883MAAdapter : public MagneticSensor {
public:
  MMC5883MAAdapter() : MagneticSensor(), _mmc5883ma(nullptr) {
    _mmc5883ma = new MMC5883MACompass();
  }

  // 析构函数
  ~MMC5883MAAdapter() override {
    if (_mmc5883ma) {
      delete _mmc5883ma;
      _mmc5883ma = nullptr;
    }
  }

  void init() override { _mmc5883ma->init(); }

  // MMC5883MA 仅支持 mode 和 odr 参数（固定 ±8G 量程）
  void setMode(byte mode, byte odr, byte /*rng*/, byte /*osr*/) override {
    _mmc5883ma->setMode(mode, odr);
  }

  void calibrate() override {
    _mmc5883ma->calibrate();
    // 更新本地校准数据
    setCalibrationOffsets(_mmc5883ma->getCalibrationOffset(0),
                          _mmc5883ma->getCalibrationOffset(1),
                          _mmc5883ma->getCalibrationOffset(2));
    setCalibrationScales(_mmc5883ma->getCalibrationScale(0),
                         _mmc5883ma->getCalibrationScale(1),
                         _mmc5883ma->getCalibrationScale(2));
  }

  void setReset() override { _mmc5883ma->setReset(); }

  void read() override {
    _mmc5883ma->read();
    _vRaw[0] = _mmc5883ma->getX();
    _vRaw[1] = _mmc5883ma->getY();
    _vRaw[2] = _mmc5883ma->getZ();
  }

  char chipID() override { return _mmc5883ma->chipID(); }

private:
  MMC5883MACompass *_mmc5883ma;

  int _get(int index) {
    if (index == 0)
      return _mmc5883ma->getX();
    if (index == 1)
      return _mmc5883ma->getY();
    if (index == 2)
      return _mmc5883ma->getZ();
    return 0;
  }
};

#endif // MMC5883MA_ADAPTER_H