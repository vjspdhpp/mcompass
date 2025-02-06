#include <Preferences.h>

#include "func.h"
#include "macro_def.h"

#define PREFERENCE_NAME "mcompass"  // 配置文件名称

#define LATITUDE_KEY "latitude"       // 纬度Key
#define LONGTITUDE_KEY "longitude"    // 经度Key
#define SPAWN_COLOR_KEY "spawnColor"  // 出生针颜色
#define SOUTH_COLOR_KEY "southColor"  // 指南针颜色
#define SERVER_MODE_KEY "serverMode"  // 配置模式
#define WIFI_SSID_KEY "SSID"          // WiFi账号
#define WIFI_PWD_KEY "PWD"            // WiFi账号
#define BRIGHTNESS_KEY "brightness"   // 亮度

static const char *TAG = "Preference";
void Preference::saveHomeLocation(Location location) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  preferences.putFloat(LATITUDE_KEY, location.latitude);
  preferences.putFloat(LONGTITUDE_KEY, location.longitude);
  preferences.end();
}

void Preference::getHomeLocation(Location &location) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  if (!preferences.isKey(LATITUDE_KEY) || !preferences.isKey(LONGTITUDE_KEY)) {
    ESP_LOGE(TAG, "!LATITUDE_KEY !LONGTITUDE_KEY");
    preferences.end();
    return;
  }
  location.latitude = preferences.getFloat(LATITUDE_KEY, 0);
  location.longitude = preferences.getFloat(LONGTITUDE_KEY, 0);
  preferences.end();
}

void Preference::savePointerColor(PointerColor color) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  preferences.putInt(SPAWN_COLOR_KEY, color.spawnColor);
  preferences.putInt(SOUTH_COLOR_KEY, color.southColor);
  preferences.end();
  ESP_LOGE(TAG, "spawnColor=%x southColor=%x", color.spawnColor,
           color.southColor);
}

void Preference::getPointerColor(PointerColor &color) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  if (!preferences.isKey(SPAWN_COLOR_KEY) ||
      !preferences.isKey(SOUTH_COLOR_KEY)) {
    ESP_LOGE(TAG, "!SPAWN_COLOR_KEY !SOUTH_COLOR_KEY");
    color.spawnColor = DEFAULT_POINTER_COLOR;
    color.southColor = DEFAULT_POINTER_COLOR;
    preferences.end();
    return;
  }
  color.spawnColor = preferences.getInt(SPAWN_COLOR_KEY, DEFAULT_POINTER_COLOR);
  color.southColor = preferences.getInt(SOUTH_COLOR_KEY, DEFAULT_POINTER_COLOR);
  ESP_LOGE(TAG, "spawnColor=%x southColor=%x", color.spawnColor,
           color.southColor);
  preferences.end();
}

void Preference::setWebServerConfig(bool enableBLE) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  preferences.putBool(SERVER_MODE_KEY, enableBLE);
  preferences.end();
}

void Preference::getWebServerConfig(bool &enableBLE) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  if (!preferences.isKey(SERVER_MODE_KEY)) {
    preferences.end();
    enableBLE = DEFAULT_SERVER_MODE;
    return;
  }
  enableBLE = preferences.getBool(SERVER_MODE_KEY, DEFAULT_SERVER_MODE);
  preferences.end();
}

void Preference::setBrightness(uint8_t brightness) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  preferences.putUChar(BRIGHTNESS_KEY, brightness);
  preferences.end();
}

void Preference::getBrightness(uint8_t &setBrightness) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  if (!preferences.isKey(BRIGHTNESS_KEY)) {
    preferences.end();
    return;
  }
  setBrightness = preferences.getUChar(BRIGHTNESS_KEY, 64);
  ESP_LOGE(TAG, "preferences.getUChar(BRIGHTNESS_KEY)=%d", setBrightness);
  preferences.end();
}

void Preference::setWiFiCredentials(String ssid, String password) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  preferences.putString(WIFI_SSID_KEY, ssid);
  preferences.putString(WIFI_PWD_KEY, password);
  preferences.end();
}

void Preference::getWiFiCredentials(String &ssid, String &password) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  if (!preferences.isKey(WIFI_SSID_KEY) && !preferences.isKey(WIFI_PWD_KEY)) {
    preferences.end();
    return;
  }

  ssid = preferences.getString(WIFI_SSID_KEY);
  password = preferences.getString(WIFI_PWD_KEY);
  preferences.end();
}