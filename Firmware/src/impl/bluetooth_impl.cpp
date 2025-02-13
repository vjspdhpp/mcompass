#include <NimBLEDevice.h>

#include "board.h"
#include "macro_def.h"
#include "utils.h"

static NimBLEServer *pServer;
static TaskHandle_t gloopTaskHandle = NULL;
static const char *TAG = "Bluetooth";
// 是否有客户端进行连接, 1分钟没有客户端连接关闭Server
static bool clientConnected = false;
// 服务工作状态
static bool serverEnable = false;

using namespace mcompass;

/**  None of these are required as they will be handled by the library with
 *defaults. **
 **                       Remove as you see fit for your needs */
class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override {
    ESP_LOGI(TAG, "Client address: %s\n", connInfo.getAddress().toString());
    pServer->updateConnParams(connInfo.getConnHandle(), 80, 100, 4, 200);
    clientConnected = true;
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
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(WEB_SERVER_CHARACHTERISTIC_UUID))) {
      characteristic = "Web Server";
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(BRIGHTNESS_CHARACHTERISTIC_UUID))) {
      characteristic = "Brightness";
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
        preference::saveSpawnLocation(location);
      }
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(COLOR_CHARACTERISITC_UUID))) {
      std::string value = pCharacteristic->getValue();
      ESP_LOGI(TAG, "Color onWrite, Received data: %s", value.c_str());
      std::vector<std::string> colors = utils::split(value, ",");
      if (colors.size() == 1) {
        PointerColor color;
        preference::getPointerColor(color);
        char *endptr;
        int southColor = strtol(colors[0].c_str(), &endptr, 16);
        if (endptr == colors[0].c_str() + 1) {
          color.southColor = southColor;
        }
        preference::savePointerColor(color);
      } else if (colors.size() >= 2) {
        char *endptr;
        int southColor = strtol(colors[0].c_str(), &endptr, 16);
        PointerColor color;
        preference::getPointerColor(color);
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
        preference::savePointerColor(color);
      } else {
        ESP_LOGE(TAG, "Failed to parse PointerColor value");
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
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(REBOOT_CHARACTERISTIC_UUID))) {
      std::string value = pCharacteristic->getValue();
      ESP_LOGI(TAG, "Reboot onWrite, Received data: %s", value.c_str());
      delay(1000);
      esp_restart();
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(WEB_SERVER_CHARACHTERISTIC_UUID))) {
      // uint8_t value = pCharacteristic->getValue().data;
      // ESP_LOGI(TAG, "WebServer onWrite, Received data: %s", value.c_str());
      // preference::setBrightness(value);
    } else if (pCharacteristic->getUUID().equals(
                   NimBLEUUID(BRIGHTNESS_CHARACHTERISTIC_UUID))) {
      std::string value = pCharacteristic->getValue();
      ESP_LOGI(TAG, "Brightness onWrite, Received data: %d", value[0]);
      if (value.length() == 1) {
        uint8_t brightness = static_cast<uint8_t>(value[0]);
        // 打印亮度值
        preference::setBrightness(brightness);
        pixel::setBrightness(brightness);
      } else {
        ESP_LOGE(TAG, "Error: Invalid brightness value length");
      }
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

static void ble_azimuth_dispatcher(void *handler_arg, esp_event_base_t base,
                                   int32_t id, void *event_data) {
  static uint32_t last_update = 0;
  if (millis() - last_update < 1000) return;
  last_update = millis();
  Event::Body *evt = (Event::Body *)event_data;
  switch (evt->type) {
    case Event::Type::AZIMUTH: {
      if (pServer->getConnectedCount() > 0) {
        // 校验刷新频率, 限制帧率1Hz
        ESP_LOGI(TAG, "Notify Azimuth: %d", evt->azimuth.angle);
        NimBLEService *pSvc =
            pServer->getServiceByUUID(NimBLEUUID(BASE_SERVICE_UUID));
        if (pSvc) {
          NimBLECharacteristic *pChr = pSvc->getCharacteristic(
              NimBLEUUID(AZIMUTH_CHARACHERSITC_UUID), 0);
          pChr->setValue(sensor::getAzimuth());
          if (pChr) {
            pChr->notify();
          }
        }
      }
    } break;
  }
}

void ble_server::init(Context *context) {
  NimBLEDevice::init("NimBLE");
  NimBLEDevice::setSecurityAuth(BLE_SM_PAIR_AUTHREQ_SC);
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(&serverCallbacks);
  // 基础Service
  NimBLEService *baseService =
      pServer->createService(NimBLEUUID(BASE_SERVICE_UUID));
  // 指针颜色
  NimBLECharacteristic *colorChar = baseService->createCharacteristic(
      NimBLEUUID(COLOR_CHARACTERISITC_UUID),
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  PointerColor color;
  preference::getPointerColor(color);
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
  preference::getSpawnLocation(location);
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
  // 请求校准
  NimBLECharacteristic *calibrateChar = baseService->createCharacteristic(
      NimBLEUUID(CALIBRATE_CHARACTERISTIC_UUID), NIMBLE_PROPERTY::WRITE);
  calibrateChar->setValue(INFO_JSON);
  calibrateChar->setCallbacks(&chrCallbacks);

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
  // 服务器模式
  NimBLECharacteristic *serverModeChar = advancedService->createCharacteristic(
      NimBLEUUID(WEB_SERVER_CHARACHTERISTIC_UUID), NIMBLE_PROPERTY::WRITE);
  serverModeChar->setValue(context->getServerMode());
  serverModeChar->setCallbacks(&chrCallbacks);

  // 亮度
  NimBLECharacteristic *brightnessChar = advancedService->createCharacteristic(
      NimBLEUUID(BRIGHTNESS_CHARACHTERISTIC_UUID),
      NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);
  uint8_t brightness = context->getBrightness();
  brightnessChar->setValue(brightness);
  brightnessChar->setCallbacks(&chrCallbacks);

  baseService->start();
  advancedService->start();

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->setName("Lenovo");
  pAdvertising->addServiceUUID(baseService->getUUID());
  pAdvertising->addServiceUUID(advancedService->getUUID());
  pAdvertising->enableScanResponse(true);
  pAdvertising->start();
  serverEnable = true;
  Serial.printf("Advertising Started\n");
  esp_event_handler_register_with(context->getEventLoop(), MCOMPASS_EVENT, 0,
                                  ble_azimuth_dispatcher, NULL);
  // 定时器, 用于关闭蓝牙
  esp_timer_handle_t deinitTimer;
  esp_timer_create_args_t timerConfig = {
      .callback =
          [](void *) {
            if (pServer->getConnectedCount() == 0) {
              ESP_LOGI(TAG, "No client connected, deinit");
              Context &context = Context::getInstance();
              // 如果型号是GPS, 没有设置过目标地址, 也不会关闭蓝牙
              if (context.isGPSModel() &&
                  !gps::isValidGPSLocation(context.getSpawnLocation())) {
                ESP_LOGI(TAG, "Spawn Location is not set, skip deinit");
                return;
              }
              ble_server::deinit(&context);
            } else {
              ESP_LOGI(TAG, "Client connected, skip deinit");
            }
          },
      .arg = nullptr,
      .dispatch_method = ESP_TIMER_TASK,
      .name = "ble_deinit_timer",
      .skip_unhandled_events = true};

  esp_timer_create(&timerConfig, &deinitTimer);
  esp_timer_start_once(deinitTimer, DEFAULT_SERVER_TIMEOUT * 1000000);
}

void ble_server::deinit(Context *context) {
  if (!serverEnable || clientConnected) {
    return;
  }
  ESP_LOGW(TAG, "deinit");
  esp_event_handler_unregister_with(context->getEventLoop(), MCOMPASS_EVENT, 0,
                                    ble_azimuth_dispatcher);
  NimBLEDevice::deinit(false);
  esp_bt_controller_disable();
  serverEnable = false;
}