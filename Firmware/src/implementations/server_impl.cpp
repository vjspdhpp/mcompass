
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <FS.h>
#include <FastLED.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <soc/usb_serial_jtag_reg.h>

#include "common.h"
#include "func.h"
#include "macro_def.h"

static AsyncWebServer server(80);
const char *PARAM_MESSAGE = "message";
const char *TAG = "WEBServer";

// 是否有客户端进行连接, 1分钟没有客户端连接关闭Server
bool clientConnected = false;
// 网页服务工作状态
bool serverEnable = false;
static Context *ctx = nullptr;
static int isPluggedUSB(void) {
  uint32_t *aa = (uint32_t *)USB_SERIAL_JTAG_FRAM_NUM_REG;
  uint32_t first = *aa;
  vTaskDelay(pdMS_TO_TICKS(10));
  return (int)(*aa - first);
}

static void notFound(AsyncWebServerRequest *request) {
  clientConnected = true;
  request->send(404, "text/plain", "Not found");
}

static void apis(void) {
  server.on("/ip", HTTP_GET, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    request->send(200, "text/plain", WiFi.localIP().toString());
  });

  server.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "Bye");
    delay(3000);
    esp_restart();
  });

  server.on("/setIndex", HTTP_POST, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    if (request->hasParam("index")) {
      int index = request->getParam("index")->value().toInt();
      if (index < 0 || index > MAX_FRAME_INDEX) {
        request->send(400, "text/plain", "index parameter invalid");
      }
      ctx->deviceState = STATE_SERVER_INDEX;
      int hexRgb = DEFAULT_NEEDLE_COLOR;
      if (request->getParam("color") != nullptr) {
        String color = request->getParam("color")->value();
        char *endptr;
        hexRgb = strtol(color.c_str() + 1, &endptr, 16);
        ESP_LOGI(TAG, "setIndex(%d) with color(%06X)\n", index, hexRgb);
        // 解析失败还原指针颜色
        if (endptr == color.c_str() + 1) {
          hexRgb = DEFAULT_NEEDLE_COLOR;
        }
      }
      Pixel::showFrame(index, hexRgb);
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Missing index parameter");
    }
  });
  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    ctx->deviceState = STATE_SERVER_INFO;
    request->send(200, "text/json", INFO_JSON);
  });

  server.on("/spawn", HTTP_GET, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    ctx->deviceState = STATE_SERVER_SPAWN;
    Location location = {
        .latitude = 0.0f,
        .longitude = 0.0f,
    };
    Preference::getHomeLocation(location);
    request->send(200, "text/json",
                  "{\"latitude\":\"" + String(location.latitude, 6) +
                      "\",\"longitude\":\"" + String(location.longitude, 6) +
                      "\"}");
  });
  server.on("/setColor", HTTP_POST, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    if (request->hasParam("color")) {
      String color = request->getParam("color")->value();
      ctx->deviceState = STATE_SERVER_COLORS;
      char *endptr;
      int hexRgb = strtol(color.c_str() + 1, &endptr, 16);
      // 检查解析是否成功
      if (endptr == color.c_str() + 1) {
        request->send(400, "text/plain", "Failed to parse color value.");
        return;
      }
      ESP_LOGI(TAG, "setColor to %06X\n", hexRgb);
      Pixel::showSolid(hexRgb);
      request->send(200);
    }
  });

  server.on("/setAzimuth", HTTP_POST, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    if (request->hasParam("azimuth")) {
      float azimuth = request->getParam("azimuth")->value().toFloat();
      ctx->deviceState = STATE_GAME_COMPASS;
      Pixel::showFrameByAzimuth(azimuth);
      request->send(200);
    }
  });

  server.on("/spawn", HTTP_POST, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    if (request->hasParam("latitude") && request->hasParam("longitude")) {
      float latitude = request->getParam("latitude")->value().toFloat();
      float longitude = request->getParam("longitude")->value().toFloat();
      if (latitude >= -90 && latitude <= 90 && longitude >= -180 &&
          longitude <= 180) {
        // 坐标不合法, 拒绝修改
        ESP_LOGE(TAG, "Spawn Location Error\n");
        request->send(400);
      }
      Location location = {.latitude = latitude, .longitude = longitude};
      Preference::saveHomeLocation(location);
      request->send(200);
    }
  });
  server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    ctx->deviceState = STATE_SERVER_WIFI;
    String ssid = WiFi.SSID();
    String password = WiFi.psk();
    request->send(200, "text/json",
                  "{\"ssid\":\"" + ssid + "\",\"password\":\"\"}");
  });
  server.on("/setWiFi", HTTP_POST, [](AsyncWebServerRequest *request) {
    clientConnected = true;
    if (request->hasParam("ssid") && request->hasParam("password")) {
      String ssid = request->getParam("ssid")->value();
      String password = request->getParam("password")->value();
      wifi_config_t config;
      strncpy(reinterpret_cast<char *>(config.sta.ssid), ssid.c_str(),
              ssid.length());
      strncpy(reinterpret_cast<char *>(config.sta.password), password.c_str(),
              password.length());
      esp_wifi_set_config(WIFI_IF_STA, &config);
      ESP_LOGD(TAG, "setWiFi: %s, ******\n", ssid.c_str());
      request->send(200);
      delay(3000);
      esp_restart();
    }
  });
}

void CompassServer::localHotspot(const char *ssid) {
  ESP_LOGI(TAG, "Starting local hotspot");
  WiFi.softAP(ssid, "");
  ESP_LOGI(TAG, "Local hotspot started");
}

void CompassServer::stopHotspot() {
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
  server.serveStatic("/", LittleFS, "/").setDefaultFile(defaultFile);
  server.onNotFound(notFound);
  server.begin();
  ESP_LOGI(TAG, "Server launched");
  apis();
}

void CompassServer::init(Context *context) {
  ctx = context;
  ESP_LOGI(TAG, "Setting up server");
  // 获取储存的WiFi配置
  wifi_config_t config;
  esp_wifi_get_config(WIFI_IF_STA, &config);

  LittleFS.begin(false, "/littlefs", 32);
  // 没有WiFi配置无条件开启热点
  if (strlen(reinterpret_cast<const char *>(config.sta.ssid)) == 0) {
    ctx->deviceState = STATE_HOTSPOT;
    ESP_LOGI(TAG, "No WiFi credentials found");
    localHotspot();
    launchServer("default.html");
    return;
  }
  ESP_LOGI(TAG, "Connecting to %s\n", config.sta.ssid);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(reinterpret_cast<const char *>(config.sta.ssid),
             reinterpret_cast<const char *>(config.sta.password));
  ctx->deviceState = STATE_CONNECT_WIFI;
  unsigned long begin = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (millis() - begin > DEFAULT_WIFI_CONNECT_TIME) {
      break;
    }
  }
  ctx->deviceState = STATE_COMPASS;
  // 仍然未能连接到WiFi, 开启本地热点showFrameByAzimuth
  // 此时热点名称 Your Compass
  if (WiFi.status() != WL_CONNECTED) {
    localHotspot("Your Compass");
  }
  ESP_LOGI(TAG, "IP Address: %s", WiFi.localIP());
  launchServer("index.html");
  MDNS.addService("http", "tcp", 80);
  serverEnable = true;
}

bool CompassServer::shouldStopServer() {
  return !clientConnected && WiFi.status() != WL_CONNECTED;
}

void CompassServer::endWebServer() {
  if (serverEnable) {
    ESP_LOGI(TAG, "endWebServer");
    server.end();
    serverEnable = false;
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  }
}
