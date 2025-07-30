#ifndef QMC5883P_ADAPTER_H
#define QMC5883P_ADAPTER_H

#include "MagneticSensor.h"
#include "QMC5883PCompass.h"
class QMC5883PAdapter : public MagneticSensor {
public:
  QMC5883PAdapter() : MagneticSensor(), _qmc5883p(nullptr) {
    _qmc5883p = new QMC5883PCompass();
  }

  // 析构函数
  ~QMC5883PAdapter() override {
    if (_qmc5883p) {
      delete _qmc5883p;
      _qmc5883p = nullptr;
    }
  }

  void init() override { _qmc5883p->init(); }

  void setMode(byte mode, byte odr, byte rng, byte osr) override {
    _qmc5883p->setMode(mode, odr, rng, osr);
  }

  void calibrate() override {
    _qmc5883p->calibrate();
    // update local calibration data
    setCalibrationOffsets(_qmc5883p->getCalibrationOffset(0),
                          _qmc5883p->getCalibrationOffset(1),
                          _qmc5883p->getCalibrationOffset(2));
    setCalibrationScales(_qmc5883p->getCalibrationScale(0),
                         _qmc5883p->getCalibrationScale(1),
                         _qmc5883p->getCalibrationScale(2));
  }

  void setReset() override { _qmc5883p->setReset(); }

  void read() override {
    _qmc5883p->read();
    _vRaw[0] = _qmc5883p->getX();
    _vRaw[1] = _qmc5883p->getY();
    _vRaw[2] = _qmc5883p->getZ();
  }

  char chipID() override { return _qmc5883p->chipID(); }

private:
  QMC5883PCompass *_qmc5883p;

  int _get(int index) {
    if (index == 0)
      return _qmc5883p->getX();
    if (index == 1)
      return _qmc5883p->getY();
    if (index == 2)
      return _qmc5883p->getZ();
    return 0;
  }
};

#endif // QMC5883P_ADAPTER_H