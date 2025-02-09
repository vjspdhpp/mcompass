#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <FS.h>
#include <FastLED.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <esp_event.h>
#include <esp_wifi.h>

#include "common.h"
#include "func.h"
#include "macro_def.h"
#include "utils.h"

using namespace mcompass;

static AsyncWebServer server(80);
const char *PARAM_MESSAGE = "message";
const char *TAG = "WEBServer";

// 是否有客户端进行连接, 1分钟没有客户端连接关闭Server
static bool clientConnected = false;
// 网页服务工作状态
static bool serverEnable = false;
static Context *ctx = nullptr;
static size_t tick = 0;

static void notFound(AsyncWebServerRequest *request) {
  clientConnected = true;
  request->send(404, "text/plain", "Not found");
}

static void apis(void) {
  // 获取STA模式下本机IP
  server.on("/ip", HTTP_GET, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    request->send(200, "text/plain", WiFi.localIP().toString());
  });

  // 重启设备
  server.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "Bye");
    delay(3000);
    esp_restart();
  });

  // 获取设备信息
  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    ctx->deviceState = State::SERVER_INFO;
    String json = "{\"buildDate\":\"" + String(__DATE__) +
                  "\",\"buildTime\":\"" + String(__TIME__) +
                  "\",\"buildVersion\":\"" + String(BUILD_VERSION) +
                  "\",\"gitBranch\":\"" + String(GIT_BRANCH) +
                  "\",\"gpsStatus\":\"" + (ctx->detectGPS ? "1" : "0") +
                  "\",\"model\":\"" + (ctx->model == Model::LITE ? "0" : "1") +
                  "\",\"sensorStatus\":\"" + (ctx->hasSensor ? "1" : "0") +
                  "\",\"gitCommit\":\"" + String(GIT_COMMIT) + "\"}";
    request->send(200, "text/json", json);
  });

  // 获取目标出生点
  server.on("/spawn", HTTP_GET, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    ctx->deviceState = State::SERVER_SPAWN;
    Location location = ctx->targetLoc;
    request->send(200, "text/json",
                  "{\"latitude\":\"" + String(location.latitude, 6) +
                      "\",\"longitude\":\"" + String(location.longitude, 6) +
                      "\"}");
  });

  // 设置目标出生点
  server.on("/spawn", HTTP_POST, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    if (request->hasParam("latitude") && request->hasParam("longitude")) {
      float latitude = request->getParam("latitude")->value().toFloat();
      float longitude = request->getParam("longitude")->value().toFloat();
      Location location;
      location.latitude = latitude;
      location.longitude = longitude;
      if (gps::isValidGPSLocation(location)) {
        preference::saveHomeLocation(location);
        request->send(200);
        return;
      }
      request->send(400);
    }
  });

  // 设置指针颜色
  server.on("/pointColors", HTTP_POST, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    ESP_LOGE(TAG, "pointColors!!!!!!!");
    PointerColor pointColor = ctx->color;
    if (request->hasParam("southColor")) {
      String color = request->getParam("southColor")->value();
      ctx->deviceState = State::SERVER_COLORS;
      char *endptr;
      int hexRgb = strtol(color.c_str() + 1, &endptr, 16);
      // 检查解析是否成功
      if (endptr == color.c_str() + 1) {
        request->send(400, "text/plain", "Failed to parse southColor value.");
        return;
      }
      ESP_LOGI(TAG, "setColor to %06X\n", hexRgb);
      pointColor.southColor = hexRgb;
    } else {
      ESP_LOGE(TAG, "not found southColor");
    }
    if (request->hasParam("spawnColor")) {
      String color = request->getParam("spawnColor")->value();
      ctx->deviceState = State::SERVER_COLORS;
      char *endptr;
      int hexRgb = strtol(color.c_str() + 1, &endptr, 16);
      // 检查解析是否成功
      if (endptr == color.c_str() + 1) {
        request->send(400, "text/plain", "Failed to parse spawnColor value.");
        return;
      }
      ESP_LOGI(TAG, "setColor to %06X\n", hexRgb);
      pointColor.spawnColor = hexRgb;
    } else {
      ESP_LOGE(TAG, "not found spawnColor");
    }
    preference::savePointerColor(pointColor);
    request->send(200);
  });

  // 获取指针颜色
  server.on("/pointColors", HTTP_GET, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    PointerColor pointColor = ctx->color;
    std::string spawnColor = utils::toHexString(pointColor.spawnColor);
    std::string southColor = utils::toHexString(pointColor.southColor);
    request->send(200, "text/json",
                  "{\"spawnColor\":\"" + String(spawnColor.c_str()) +
                      "\",\"southColor\":\"" + String(southColor.c_str()) +
                      "\"}");
  });

  // 获取亮度
  server.on("/brightness", HTTP_GET, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    ctx->deviceState = State::SERVER_SPAWN;
    uint8_t brightness = ctx->brightness;
    String response = "{\"brightness\":" + String(brightness) + "}";
    request->send(200, "text/json", response);
  });

  // 设置亮度
  server.on("/brightness", HTTP_POST, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    if (request->hasParam("brightness")) {
      // 获取 brightness 参数的值（String 类型）
      String brightnessStr = request->getParam("brightness")->value();

      // 将 String 转换为整数
      int brightnessInt = brightnessStr.toInt();

      // 检查 brightness 是否在 0 到 255 范围内
      if (brightnessInt >= 0 && brightnessInt <= 255) {
        // 转换为 uint8_t 类型
        uint8_t brightness = static_cast<uint8_t>(brightnessInt);

        // 更新设备状态和亮度
        ctx->deviceState = State::SERVER_COLORS;
        preference::setBrightness(brightness);
        ESP_LOGI(TAG, "set brightness to %d", brightness);
        pixel::setBrightness(brightness);
        // 返回成功响应
        request->send(200);
      } else {
        // 如果 brightness 不在有效范围内，返回 400 错误
        request->send(400, "text/plain",
                      "Brightness must be between 0 and 255");
      }
    } else {
      // 如果请求中没有 brightness 参数，返回 400 错误
      request->send(400, "text/plain", "Brightness parameter is missing");
    }
  });

  // 设置罗盘显示指定方位角度
  server.on("/setAzimuth", HTTP_POST, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    if (request->hasParam("azimuth")) {
      float azimuth = request->getParam("azimuth")->value().toFloat();
      ctx->deviceState = State::GAME_COMPASS;
      pixel::showByAzimuth(azimuth);
      request->send(200);
    }
  });

  // 获取WiFi配置
  server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    ctx->deviceState = State::SERVER_WIFI;
    request->send(200, "text/json",
                  "{\"ssid\":\"" + ctx->ssid + "\",\"password\":\"" +
                      ctx->password + "\"}");
  });

  // 设置WiFi配置
  server.on("/wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    if (request->hasParam("ssid") && request->hasParam("password")) {
      String ssid = request->getParam("ssid")->value();
      String password = request->getParam("password")->value();
      preference::setWiFiCredentials(ssid, password);
      // 重启后配置生效
      request->send(200);
    }
  });

  // 设置高级配置
  server.on("/advancedConfig", HTTP_POST, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    if (request->hasParam("serverMode")) {
      String serverMode = request->getParam("serverMode")->value();
      ServerMode mode =
          (serverMode == "1") ? ServerMode::BLE : ServerMode::WIFI;
      preference::setServerMode(mode);
    }
    if (request->hasParam("model")) {
      String model = request->getParam("model")->value();
      Model compassModel = model == "0" ? Model::LITE : Model::GPS;
      preference::setCustomDeviceModel(compassModel);
    }
    // 重启后配置生效
    request->send(200);
  });

  // 获取高级配置
  server.on("/advancedConfig", HTTP_GET, [](AsyncWebServerRequest *request) {
    Model model = ctx->model;
    String modelStr = model == Model::LITE ? "0" : "1";
    ServerMode serverMode = ctx->serverMode;
    String serverModeStr = serverMode == ServerMode::BLE ? "1" : "0";
    request->send(200, "text/json",
                  "{\"model\":\"" + modelStr + "\",\"serverMode\":\"" +
                      serverModeStr + "\"}");
  });

  //////////////////////////// 旧API ////////////////////////////
  // 兼容性保留setWiFi
  server.on("/setWiFi", HTTP_POST, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    if (request->hasParam("ssid") && request->hasParam("password")) {
      String ssid = request->getParam("ssid")->value();
      String password = request->getParam("password")->value();
      preference::setWiFiCredentials(ssid, password);
      // 重启后配置生效
      request->send(200);
    }
  });
  // 所有LED显示指定颜色
  server.on("/setColor", HTTP_POST, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    if (request->hasParam("color")) {
      String color = request->getParam("color")->value();
      ctx->deviceState = State::SERVER_COLORS;
      char *endptr;
      int hexRgb = strtol(color.c_str() + 1, &endptr, 16);
      // 检查解析是否成功
      if (endptr == color.c_str() + 1) {
        request->send(400, "text/plain", "Failed to parse color value.");
        return;
      }
      ESP_LOGI(TAG, "setColor to %06X\n", hexRgb);
      pixel::showSolid(hexRgb);
      request->send(200);
    }
  });
  // 设置指针显示指定帧
  server.on("/setIndex", HTTP_POST, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    if (request->hasParam("index")) {
      int index = request->getParam("index")->value().toInt();
      if (index < 0 || index > MAX_FRAME_INDEX) {
        request->send(400, "text/plain", "index parameter invalid");
      }
      ctx->deviceState = State::SERVER_INDEX;
      int hexRgb = DEFAULT_POINTER_COLOR;
      if (request->getParam("color") != nullptr) {
        String color = request->getParam("color")->value();
        char *endptr;
        hexRgb = strtol(color.c_str() + 1, &endptr, 16);
        ESP_LOGI(TAG, "setIndex(%d) with color(%06X)\n", index, hexRgb);
        // 解析失败还原指针颜色
        if (endptr == color.c_str() + 1) {
          hexRgb = DEFAULT_POINTER_COLOR;
        }
      }
      pixel::setPointerColor(hexRgb);
      pixel::showFrame(index);
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Missing index parameter");
    }
  });
}

void web_server::startHotspot(const char *ssid) {
  ESP_LOGI(TAG, "Starting local hotspot");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, "");
  // 注册 Wi-Fi 事件处理函数
  // ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
  //                                            &wifi_event_handler, NULL));
  // ESP_LOGI(TAG, "Local hotspot started");
}

void web_server::stopHotspot() {
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
}

static void launchServer(const char *defaultFile) {
  if (!MDNS.begin("esp32")) {  // Set the hostname to "esp32.local"
    ESP_LOGE(TAG, "Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  ESP_LOGI(TAG, "Launching server");
  apis();
  server.serveStatic("/", LittleFS, "/").setDefaultFile(defaultFile);
  server.onNotFound(notFound);
  server.begin();
  ESP_LOGI(TAG, "Server launched");
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
    wifi_event_ap_staconnected_t *event =
        (wifi_event_ap_staconnected_t *)event_data;
    ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac),
             event->aid);
    clientConnected = true;
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_AP_STADISCONNECTED) {
    wifi_event_ap_stadisconnected_t *event =
        (wifi_event_ap_stadisconnected_t *)event_data;
    ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac),
             event->aid);
    // 设备断开连接,重置tick
    clientConnected = false;
    tick = 0;
  }
}

void web_server::init(Context *context) {
  ctx = context;
  ESP_LOGI(TAG, "Setting up server");
  // 获取储存的WiFi配置
  String ssid, password;
  preference::getWiFiCredentials(ssid, password);
  LittleFS.begin(false, "/littlefs", 32);
  // 没有WiFi配置无条件开启热点
  if (ssid.length() == 0) {
    ctx->deviceState = State::HOTSPOT;
    ESP_LOGI(TAG, "No WiFi credentials found");
    startHotspot();
    launchServer("index.html");
    return;
  }
  ESP_LOGI(TAG, "Connecting to %s\n", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(false);
  WiFi.begin(ssid, password);
  ctx->deviceState = State::CONNECT_WIFI;
  unsigned long begin = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (millis() - begin > DEFAULT_WIFI_CONNECT_TIME) {
      break;
    }
  }
  ctx->deviceState = State::COMPASS;
  // 仍然未能连接到WiFi, 开启本地热点showFrameByAzimuth
  // 此时热点名称 Your Compass
  if (WiFi.status() != WL_CONNECTED) {
    startHotspot("Your Compass");
  }

  ESP_LOGI(TAG, "IP Address: %s", WiFi.localIP().toString());
  launchServer("index.html");
  MDNS.addService("http", "tcp", 80);
  serverEnable = true;
}

bool web_server::shouldStop() {
  return !clientConnected && WiFi.status() != WL_CONNECTED;
}

void web_server::endServer() {
  tick++;
  // 超过指定tick检测是否需要关闭
  if (tick < DEFAULT_SERVER_TICK_COUNT) {
    return;
  }
  if (!serverEnable || !web_server::shouldStop()) {
    return;
  }
  ESP_LOGW(TAG, "endWebServer");
  server.end();
  serverEnable = false;
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}
