#ifndef QMC5883L_ADAPTER_H
#define QMC5883L_ADAPTER_H

#include "MagneticSensor.h"
#include "QMC5883LCompass.h"

class QMC5883LAdapter : public MagneticSensor {
public:
  QMC5883LAdapter() : MagneticSensor(), _qmc5883l(nullptr) {
    _qmc5883l = new QMC5883LCompass();
  }

  ~QMC5883LAdapter() override {
    if (_qmc5883l) {
      delete _qmc5883l;
      _qmc5883l = nullptr;
    }
  }

  void init() override { _qmc5883l->init(); }

  void setMode(byte mode, byte odr, byte rng, byte osr) override {
    _qmc5883l->setMode(mode, odr, rng, osr);
  }

  void calibrate() override {
    _qmc5883l->calibrate();
    // update local calibration data
    setCalibrationOffsets(_qmc5883l->getCalibrationOffset(0),
                          _qmc5883l->getCalibrationOffset(1),
                          _qmc5883l->getCalibrationOffset(2));
    setCalibrationScales(_qmc5883l->getCalibrationScale(0),
                         _qmc5883l->getCalibrationScale(1),
                         _qmc5883l->getCalibrationScale(2));
  }

  void setReset() override { _qmc5883l->setReset(); }

  void read() override {
    _qmc5883l->read();
    _vRaw[0] = _qmc5883l->getX();
    _vRaw[1] = _qmc5883l->getY();
    _vRaw[2] = _qmc5883l->getZ();
  }

  char chipID() override { return _qmc5883l->chipID(); }

private:
  QMC5883LCompass *_qmc5883l;
  int _get(int index) {
    if (index == 0)
      return _qmc5883l->getX();
    if (index == 1)
      return _qmc5883l->getY();
    if (index == 2)
      return _qmc5883l->getZ();
    return 0;
  }
};

#endif // QMC5883L_ADAPTER_H