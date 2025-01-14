#include <NimBLEDevice.h>

#include "func.h"
#include "macro_def.h"

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
#define WEB_SERVER_CHARACHTERISTIC_UUID \
  (uint16_t)0xfa03  // 启用服务器API和网页服务

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

static const char *TAG = "Bluetooth";

static std::vector<std::string> split(std::string &s,
                                      const std::string &delimiter) {
  std::vector<std::string> tokens;
  size_t pos = 0;
  std::string token;
  while ((pos = s.find(delimiter)) != std::string::npos) {
    token = s.substr(0, pos);
    tokens.push_back(token);
    s.erase(0, pos + delimiter.length());
  }
  tokens.push_back(s);

  return tokens;
}

/**  None of these are required as they will be handled by the library with
 *defaults. **
 **                       Remove as you see fit for your needs */
class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override {
    ESP_LOGI(TAG, "Client address: %s\n", connInfo.getAddress().toString());
    pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 100);
  }

  void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo,
                    int reason) override {
    ESP_LOGI(TAG, "Client disconnected - start advertising\n");
    NimBLEDevice::startAdvertising();
  }

  void onMTUChange(uint16_t MTU, NimBLEConnInfo &connInfo) override {
    ESP_LOGI(TAG, "MTU updated: %u for connection ID: %u\n", MTU,
             connInfo.getConnHandle());
  }

} serverCallbacks;

/** Handler class for characteristic actions */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
  void onRead(NimBLECharacteristic *pCharacteristic,
              NimBLEConnInfo &connInfo) override {
    std::string characteristic = "Unknown";
    if (pCharacteristic->getUUID().equals(
            NimBLEUUID(SPAWN_CHARACTERISTIC_UUID))) {
      characteristic = "Spawn";
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(COLOR_CHARACTERISITC_UUID))) {
      characteristic = "Color";
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(AZIMUTH_CHARACHERSITC_UUID))) {
      characteristic = "Azimuth";
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(INFO_CHARACTERISTIC_UUID))) {
      characteristic = "Info";
    }
    ESP_LOGI(TAG, "%s onRead, value: %s", characteristic,
             pCharacteristic->getValue().c_str());
  }

  void onWrite(NimBLECharacteristic *pCharacteristic,
               NimBLEConnInfo &connInfo) override {
    if (pCharacteristic->getUUID().equals(
            NimBLEUUID(SPAWN_CHARACTERISTIC_UUID))) {
      // 获取写入的数据
      std::string value = pCharacteristic->getValue();
      ESP_LOGI(TAG, "Spawn onWrite, Received data:%s ", value.c_str());
      // 解析经度和纬度
      float longitude = 0.0f;
      float latitude = 0.0f;
      bool parseSuccess = false;
      size_t commaIndex = value.find(',');
      if (commaIndex != std::string::npos) {
        std::string longitudeStr = value.substr(0, commaIndex);
        std::string latitudeStr = value.substr(commaIndex + 1);
        try {
          longitude = std::stof(longitudeStr);
          latitude = std::stof(latitudeStr);
          parseSuccess = true;
        } catch (const std::invalid_argument &e) {
          ESP_LOGE(TAG, "Error: Invalid number format");
        } catch (const std::out_of_range &e) {
          ESP_LOGE(TAG, "Error: Number out of range");
        }
      } else {
        ESP_LOGE(TAG, "Error: Invalid format, expected 'xxx,yyy'");
      }

      // 打印经度和纬度
      if (parseSuccess) {
        ESP_LOGI(TAG, "Save Longitude: %.6f, Latitude:%.6f", longitude,
                 latitude);
        Location location = {
            .latitude = latitude,
            .longitude = longitude,
        };
        Preference::saveHomeLocation(location);
      }
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(COLOR_CHARACTERISITC_UUID))) {
      std::string value = pCharacteristic->getValue();
      ESP_LOGI(TAG, "Color onWrite, Received data: %s", value.c_str());
      std::vector<std::string> colors = split(value, ",");
      if (colors.size() == 1) {
        NeedleColor color;
        Preference::getNeedleColor(color);
        char *endptr;
        int southColor = strtol(colors[0].c_str(), &endptr, 16);
        if (endptr == colors[0].c_str() + 1) {
          color.southColor = southColor;
        }
        Preference::saveNeedleColor(color);
      } else if (colors.size() >= 2) {
        char *endptr;
        int southColor = strtol(colors[0].c_str(), &endptr, 16);
        NeedleColor color;
        Preference::getNeedleColor(color);
        if (endptr == colors[0].c_str()) {
          ESP_LOGE(TAG, "Failed to parse southColor value");
        } else {
          color.southColor = southColor;
        }
        int spawnColor = strtol(colors[1].c_str(), &endptr, 16);
        if (endptr == colors[1].c_str()) {
          ESP_LOGE(TAG, "Failed to parse spawnColor value");
        } else {
          color.spawnColor = spawnColor;
        }
        Preference::saveNeedleColor(color);
      } else {
        ESP_LOGE(TAG, "Failed to parse NeedleColor value");
      }
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(VIRTUAL_LOCATION_CHARACHTERISTIC_UUID))) {
      std::string value = pCharacteristic->getValue();
      ESP_LOGI(TAG, "Virtual Location onWrite, Received data: %s",
               value.c_str());
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(VIRTUAL_AZIMUTH_CHARACHTERISTIC_UUID))) {
      std::string value = pCharacteristic->getValue();
      ESP_LOGI(TAG, "Virtual Azimuth onWrite, Received data: %s",
               value.c_str());
    }
  }

  /**
   *  The value returned in code is the NimBLE host return code.
   */
  void onStatus(NimBLECharacteristic *pCharacteristic, int code) override {
    ESP_LOGI(TAG, "Notification/Indication return code: %d, %s\n", code,
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

    ESP_LOGI(TAG, "%s\n", str.c_str());
  }
} chrCallbacks;

void CompassBLE::initBleServer() {
  NimBLEDevice::init("NimBLE");
  NimBLEDevice::setSecurityAuth(BLE_SM_PAIR_AUTHREQ_SC);
  NimBLEDevice::setPower(3);
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(&serverCallbacks);
  // 基础Service
  NimBLEService *baseService =
      pServer->createService(NimBLEUUID(BASE_SERVICE_UUID));
  // 指针颜色
  NimBLECharacteristic *colorChar = baseService->createCharacteristic(
      NimBLEUUID(COLOR_CHARACTERISITC_UUID),
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  NeedleColor color;
  Preference::getNeedleColor(color);
  char colorBuffer[24] = {0};
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
  Preference::getHomeLocation(location);
  char locationBuffer[24] = {0};
  sprintf(locationBuffer, "%.6f,%.6f", location.latitude, location.longitude);
  spawnChar->setValue(locationBuffer);
  spawnChar->setCallbacks(&chrCallbacks);
  // 设备信息
  NimBLECharacteristic *infoChar = baseService->createCharacteristic(
      NimBLEUUID(INFO_CHARACTERISTIC_UUID),
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  infoChar->setValue(INFO_JSON);
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

void CompassBLE::bleLoop() {
  /** Loop here and send notifications to connected peers */
  delay(2000);
  if (pServer->getConnectedCount()) {
    NimBLEService *pSvc =
        pServer->getServiceByUUID(NimBLEUUID(BASE_SERVICE_UUID));
    if (pSvc) {
      NimBLECharacteristic *pChr =
          pSvc->getCharacteristic(NimBLEUUID(AZIMUTH_CHARACHERSITC_UUID), 0);
      pChr->setValue(Compass::getAzimuth());
      if (pChr) {
        pChr->notify();
      }
    }
  }
}