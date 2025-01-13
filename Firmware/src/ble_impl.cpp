#include <NimBLEDevice.h>

#include "func.h"

/* 基础配置 */
#define BASE_SERVICE_UUID (uint16_t)0xf900
#define COLOR_CHARACTERISITC_UUID (uint16_t)0xf901   // 指针颜色
#define AZIMUTH_CHARACHERSITC_UUID (uint16_t)0xf902  // 方位角
#define SPAWN_CHARACTERISTIC_UUID (uint16_t)0xf903   // 出生点信息
#define INFO_CHARACTERISTIC_UUID (uint16_t)0xf904    // 设备信息

/** 高级配置  */
#define ADVANCED_SERVICE_UUID (uint16_t)0xfa00
#define VIRTUAL_LOCATION_CHARACHTERISTIC_UUID (uint16_t)0xfa01  // 虚拟坐标
#define VIRTUAL_AZIMUTH_CHARACHTERISTIC_UUID (uint16_t)0xfa02  // 虚拟方位角

// uuid2string
#define UUID2String(uuid)                                                 \
  ((uuid) == COLOR_CHARACTERISITC_UUID               ? "COLOR"            \
   : (uuid) == AZIMUTH_CHARACHERSITC_UUID            ? "AZIMUTH"          \
   : (uuid) == SPAWN_CHARACTERISTIC_UUID             ? "SPAWN"            \
   : (uuid) == INFO_CHARACTERISTIC_UUID              ? "INFO"             \
   : (uuid) == VIRTUAL_LOCATION_CHARACHTERISTIC_UUID ? "VIRTUAL_LOCATION" \
   : (uuid) == VIRTUAL_AZIMUTH_CHARACHTERISTIC_UUID  ? "VIRTUAL_AZIMUTH"  \
                                                     : "UNKNOWN_UUID")

extern NimBLEServer *pServer;

static String generateInfoJson() {
  String buildDate = __DATE__;
  String buildTime = __TIME__;
#ifdef BUILD_VERSION
  String buildVersion = BUILD_VERSION;
#else
  String buildVersion = "UNKNOWN";
#endif
  String gitBranch = "UNKNOWN";
#ifdef GIT_BRANCH
  gitBranch = GIT_BRANCH;
#endif
  String gitCommit = "UNKNOWN";
#ifdef GIT_COMMIT
  gitCommit = GIT_COMMIT;
#endif
  return String("{\"buildDate\":\"" + buildDate + "\",\"buildTime\":\"" +
                buildTime + "\",\"buildVersion\":\"" + buildVersion +
                "\",\"gitBranch\":\"" + gitBranch + "\",\"gitCommit\":\"" +
                gitCommit + "\"}");
}
float lat = 0.0f;
float lon = 0.0f;
/** Handler class for characteristic actions */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
  void onRead(NimBLECharacteristic *pCharacteristic,
              NimBLEConnInfo &connInfo) override {
    if (pCharacteristic->getUUID().equals(
            NimBLEUUID(SPAWN_CHARACTERISTIC_UUID))) {
      Serial.print("SPAWN: onRead");
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(COLOR_CHARACTERISITC_UUID))) {
      Serial.print("COLOR: onRead");
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(AZIMUTH_CHARACHERSITC_UUID))) {
      Serial.print("AZIMUTH: onRead");
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(INFO_CHARACTERISTIC_UUID))) {
      Serial.print("INFO: onRead");
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(VIRTUAL_LOCATION_CHARACHTERISTIC_UUID))) {
      Serial.print("VIRTUAL: onRead");
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(VIRTUAL_AZIMUTH_CHARACHTERISTIC_UUID))) {
      Serial.print("VIRTUAL_AZIMUTH: onRead");
    } else {
      Serial.print("UNKNOWN: onRead");
    }
    Serial.printf(", value: %s\n", pCharacteristic->getValue().c_str());
  }

  void onWrite(NimBLECharacteristic *pCharacteristic,
               NimBLEConnInfo &connInfo) override {
    if (pCharacteristic->getUUID().equals(
            NimBLEUUID(SPAWN_CHARACTERISTIC_UUID))) {
      Serial.print("SPAWN: onWrite - ");

      // 获取写入的数据
      std::string value = pCharacteristic->getValue();
      Serial.println("Received data: " + String(value.c_str()));

      // 解析经度和纬度
      float longitude = 0.0f;
      float latitude = 0.0f;
      bool parseSuccess = false;

      // 查找逗号分隔符
      size_t commaIndex = value.find(',');
      if (commaIndex != std::string::npos) {
        // 提取经度部分
        std::string longitudeStr = value.substr(0, commaIndex);
        // 提取纬度部分
        std::string latitudeStr = value.substr(commaIndex + 1);

        // 将字符串转换为浮点数
        try {
          longitude = std::stof(longitudeStr);
          latitude = std::stof(latitudeStr);
          parseSuccess = true;
        } catch (const std::invalid_argument &e) {
          Serial.println("Error: Invalid number format");
        } catch (const std::out_of_range &e) {
          Serial.println("Error: Number out of range");
        }
      } else {
        Serial.println("Error: Invalid format, expected 'xxx,yyy'");
      }

      // 如果解析成功，打印经度和纬度
      if (parseSuccess) {
        Serial.print("Longitude: ");
        Serial.println(longitude, 6);  // 打印 6 位小数
        Serial.print("Latitude: ");
        Serial.println(latitude, 6);  // 打印 6 位小数
      }
      Location location = {
          .latitude = latitude,
          .longitude = longitude,
      };
      saveHomeLocation(location);
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(COLOR_CHARACTERISITC_UUID))) {
      Serial.print("COLOR: onWrite");
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(AZIMUTH_CHARACHERSITC_UUID))) {
      Serial.print("AZIMUTH: onWrite");
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(INFO_CHARACTERISTIC_UUID))) {
      Serial.print("INFO: onWrite");
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(VIRTUAL_LOCATION_CHARACHTERISTIC_UUID))) {
      Serial.print("VIRTUAL: onWrite");
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(VIRTUAL_AZIMUTH_CHARACHTERISTIC_UUID))) {
      Serial.print("VIRTUAL: onWrite");
    } else {
      Serial.print("UNKNOWN: onWrite");
    }
    Serial.printf(", value: %s\n", pCharacteristic->getValue().c_str());
  }

  /**
   *  The value returned in code is the NimBLE host return code.
   */
  void onStatus(NimBLECharacteristic *pCharacteristic, int code) override {
    Serial.printf("Notification/Indication return code: %d, %s\n", code,
                  NimBLEUtils::returnCodeToString(code));
  }

  /** Peer subscribed to notifications/indications */
  void onSubscribe(NimBLECharacteristic *pCharacteristic,
                   NimBLEConnInfo &connInfo, uint16_t subValue) override {
    std::string str = "Client ID: ";
    str += connInfo.getConnHandle();
    str += " Address: ";
    str += connInfo.getAddress().toString();
    if (subValue == 0) {
      str += " Unsubscribed to ";
    } else if (subValue == 1) {
      str += " Subscribed to notifications for ";
    } else if (subValue == 2) {
      str += " Subscribed to indications for ";
    } else if (subValue == 3) {
      str += " Subscribed to notifications and indications for ";
    }
    str += std::string(pCharacteristic->getUUID());

    Serial.printf("%s\n", str.c_str());
  }
} chrCallbacks;

void initBleServer() {
  NimBLEDevice::init("NimBLE");
  NimBLEDevice::setSecurityAuth(BLE_SM_PAIR_AUTHREQ_SC);
  NimBLEDevice::setPower(3);
  pServer = NimBLEDevice::createServer();
  // pServer->setCallbacks(&serverCallbacks);
  // 基础Service
  NimBLEService *baseService =
      pServer->createService(NimBLEUUID(BASE_SERVICE_UUID));
  // 指针颜色
  NimBLECharacteristic *colorChar = baseService->createCharacteristic(
      NimBLEUUID(COLOR_CHARACTERISITC_UUID),
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  NeedleColor color;
  getNeedleColor(color);
  char colorBuffer[24];
  sprintf(colorBuffer, "%x,%x", color.spawnColor, color.southColor);
  colorChar->setValue(colorBuffer);
  colorChar->setCallbacks(&chrCallbacks);
  // 方位角, 可读, Notify
  NimBLECharacteristic *azimuthChar = baseService->createCharacteristic(
      NimBLEUUID(AZIMUTH_CHARACHERSITC_UUID),
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  azimuthChar->setValue(0);
  azimuthChar->setCallbacks(&chrCallbacks);
  // 出生点, 可读可写
  NimBLECharacteristic *spawnChar = baseService->createCharacteristic(
      NimBLEUUID(SPAWN_CHARACTERISTIC_UUID),
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  Location location;
  getHomeLocation(location);
  char locationBuffer[24];
  sprintf(locationBuffer, "%.6f,%.6f", location.latitude, location.longitude);
  spawnChar->setValue(locationBuffer);
  spawnChar->setCallbacks(&chrCallbacks);
  // 设备信息
  NimBLECharacteristic *infoChar = baseService->createCharacteristic(
      NimBLEUUID(INFO_CHARACTERISTIC_UUID),
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  infoChar->setValue(generateInfoJson().c_str());
  infoChar->setCallbacks(&chrCallbacks);

  // 高级Service
  NimBLEService *advancedService =
      pServer->createService(NimBLEUUID(ADVANCED_SERVICE_UUID));
  // 虚拟方位角
  NimBLECharacteristic *virtualAzimuthChar =
      advancedService->createCharacteristic(
          NimBLEUUID(VIRTUAL_AZIMUTH_CHARACHTERISTIC_UUID),
          NIMBLE_PROPERTY::WRITE);
  virtualAzimuthChar->setValue(0);
  virtualAzimuthChar->setCallbacks(&chrCallbacks);
  // 虚拟坐标
  NimBLECharacteristic *virtualLocationChar =
      advancedService->createCharacteristic(
          NimBLEUUID(VIRTUAL_LOCATION_CHARACHTERISTIC_UUID),
          NIMBLE_PROPERTY::WRITE);
  virtualLocationChar->setValue(0);
  virtualLocationChar->setCallbacks(&chrCallbacks);

  baseService->start();
  advancedService->start();

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->setName("Lenovo");
  pAdvertising->addServiceUUID(baseService->getUUID());
  pAdvertising->addServiceUUID(advancedService->getUUID());
  pAdvertising->enableScanResponse(true);
  pAdvertising->start();
}

float delta = 0.0f;
void ble_loop() {
  /** Loop here and send notifications to connected peers */
  delay(2000);
  delta += 0.01f;
  if (pServer->getConnectedCount()) {
    NimBLEService *pSvc =
        pServer->getServiceByUUID(NimBLEUUID(BASE_SERVICE_UUID));
    if (pSvc) {
      NimBLECharacteristic *pChr =
          pSvc->getCharacteristic(NimBLEUUID(AZIMUTH_CHARACHERSITC_UUID), 0);
      pChr->setValue(delta);
      if (pChr) {
        pChr->notify();
      }
    }
  }
}