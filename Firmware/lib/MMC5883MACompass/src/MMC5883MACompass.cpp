#include "MMC5883MACompass.h"

/* ==================== 构造 & 初始化 ==================== */
MMC5883MACompass::MMC5883MACompass() {}

void MMC5883MACompass::init() {
    Wire.begin();

    /* 软件复位 */
    _writeReg(REG_CTRL1, 0x80);
    delay(3);

    /* 带宽：200 Hz（0x04），其余默认 */
    _writeReg(REG_CTRL1, 0x04);

    clearCalibration();

    /* 先做一次 SET+RESET 退磁 */
    _performSet();
    _performReset();
}

/* ==================== 公共接口 ==================== */
void MMC5883MACompass::setADDR(byte b){ _ADDR=b; }

char MMC5883MACompass::chipID(){
    Wire.beginTransmission(_ADDR);
    Wire.write(REG_ID);
    if (Wire.endTransmission(false)) return 0;
    Wire.requestFrom(_ADDR,(byte)1);
    return Wire.read();               /* 应为 0x0C */
}

void MMC5883MACompass::setMode(byte mode, byte odr){
    _writeReg(REG_CTRL1, odr & 0x0C);             /* BW */
    _writeReg(REG_CTRL0, mode & 0x03);            /* 0:Suspend 1:Cont 2:Single */
}

void MMC5883MACompass::setMagneticDeclination(int d,uint8_t m){
    _decl = d + m/60.0f;
}

void MMC5883MACompass::setReset(){
    _writeReg(REG_CTRL1,0x80);
    delay(3);
}

void MMC5883MACompass::setSmoothing(byte n,bool adv){
    _useSmooth=true;
    _smoothN = n<2?2:(n>10?10:n);
    _smoothAdv=adv;
}

/* ==================== 读数主函数 ==================== */
void MMC5883MACompass::read(){
    int16_t A[3],B[3];

    /* ① SET & 测量 A */
    _performSet();
    _triggerMeasure();
    while(!(_readStatus()&0x01));       // 等待 Meas_Done
    _burstRead(A);

    /* ② RESET & 测量 B */
    _performReset();
    _triggerMeasure();
    while(!(_readStatus()&0x01));
    _burstRead(B);

    /* ③ (A-B)/2 去零偏 */
    for(int i=0;i<3;++i) _raw[i]=(int)(A[i]-B[i])/2;

    _applyCalibration();
    if(_useSmooth) _smoothing();
}

/* ==================== 私有 I²C ==================== */
void MMC5883MACompass::_writeReg(byte r,byte v){
    Wire.beginTransmission(_ADDR);
    Wire.write(r); Wire.write(v);
    Wire.endTransmission();
}

uint8_t MMC5883MACompass::_readStatus(){
    Wire.beginTransmission(_ADDR);
    Wire.write(REG_STATUS);
    Wire.endTransmission(false);
    Wire.requestFrom(_ADDR,(byte)1);
    return Wire.read();
}

void MMC5883MACompass::_burstRead(int16_t v[3]){
    Wire.beginTransmission(_ADDR);
    Wire.write(REG_DATA);
    Wire.endTransmission(false);
    Wire.requestFrom(_ADDR,(byte)6);

    for(int i=0;i<3;++i){
        uint16_t lo=Wire.read(),hi=Wire.read();
        v[i]=int16_t((hi<<8)|lo) - 32768;      /* 减偏移 */
    }
}

void MMC5883MACompass::_triggerMeasure(){
    _writeReg(REG_CTRL0, CTRL0_TM_M);          // TM_M=1
}

void MMC5883MACompass::_performSet (){
    _writeReg(REG_CTRL0, CTRL0_SET );
    delayMicroseconds(50);
}
void MMC5883MACompass::_performReset(){
    _writeReg(REG_CTRL0, CTRL0_RST );
    delayMicroseconds(50);
}

/* ==================== 校准 & 滤波 ==================== */
void MMC5883MACompass::calibrate(){
    clearCalibration();
    long minv[3]={ 65000, 65000, 65000 };
    long maxv[3]={-65000,-65000,-65000 };
    unsigned long t0=millis();
    while(millis()-t0<10000){
        read();
        for(int i=0;i<3;++i){
            if(_raw[i]<minv[i]) minv[i]=_raw[i];
            if(_raw[i]>maxv[i]) maxv[i]=_raw[i];
        }
        delay(50);
    }
    setCalibration(minv[0],maxv[0], minv[1],maxv[1], minv[2],maxv[2]);
}

void MMC5883MACompass::setCalibration(int xmin,int xmax,int ymin,int ymax,int zmin,int zmax){
    setCalibrationOffsets((xmin+xmax)/2.0f,
                          (ymin+ymax)/2.0f,
                          (zmin+zmax)/2.0f);
    float dx=(xmax-xmin)/2.0f, dy=(ymax-ymin)/2.0f, dz=(zmax-zmin)/2.0f;
    float avg=(dx+dy+dz)/3.0f;
    setCalibrationScales(avg/dx, avg/dy, avg/dz);
}
void MMC5883MACompass::setCalibrationOffsets(float x,float y,float z){
    _off[0]=x; _off[1]=y; _off[2]=z;
}
void MMC5883MACompass::setCalibrationScales(float xs,float ys,float zs){
    _scale[0]=xs; _scale[1]=ys; _scale[2]=zs;
}
float MMC5883MACompass::getCalibrationOffset(uint8_t i){return _off[i];}
float MMC5883MACompass::getCalibrationScale (uint8_t i){return _scale[i];}
void  MMC5883MACompass::clearCalibration(){
    setCalibrationOffsets(0,0,0);
    setCalibrationScales (1,1,1);
}

/* ---------- 滑动平均 ---------- */
void MMC5883MACompass::_smoothing(){
    if(_scan>=_smoothN) _scan=0;
    for(int i=0;i<3;++i){
        _tot[i]-=_hist[_scan][i];
        _hist[_scan][i]=_cal[i];
        _tot[i]+=_hist[_scan][i];
        if(_smoothAdv){
            int min=_hist[0][i], max=min;
            for(int j=1;j<_smoothN;++j){
                min=_hist[j][i]<min?_hist[j][i]:min;
                max=_hist[j][i]>max?_hist[j][i]:max;
            }
            _smooth[i]=(_tot[i]-min-max)/(_smoothN-2);
        }else{
            _smooth[i]=_tot[i]/_smoothN;
        }
    }
    ++_scan;
}

/* ---------- 校准应用 ---------- */
void MMC5883MACompass::_applyCalibration(){
    for(int i=0;i<3;++i)
        _cal[i]=(_raw[i]-_off[i])*_scale[i];
}

/* ==================== 输出接口 ==================== */
int MMC5883MACompass::_get(int i){
    return _useSmooth? _smooth[i] : _cal[i];
}
int MMC5883MACompass::getX(){ return _get(0); }
int MMC5883MACompass::getY(){ return _get(1); }
int MMC5883MACompass::getZ(){ return _get(2); }

int MMC5883MACompass::getAzimuth(){
    float h = atan2(getY(),getX())*180.0f/PI + _decl;
    if(h<0) h+=360;
    return int(h+0.5f)%360;
}
byte MMC5883MACompass::getBearing(int az){
    return byte(((az+11)/22)%16);
}
void MMC5883MACompass::getDirection(char*buf,int az){
    byte b=getBearing(az);
    buf[0]=_bearingName[b][0];
    buf[1]=_bearingName[b][1];
    buf[2]=_bearingName[b][2];
}
