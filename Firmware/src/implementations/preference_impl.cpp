#include <Preferences.h>

#include "func.h"
#include "macro_def.h"

static const char *TAG = "Preference";
void Preference::saveHomeLocation(Location location) {
  Preferences preferences;
  preferences.begin("mcompass", false);
  preferences.putFloat("latitude", location.latitude);
  preferences.putFloat("longitude", location.longitude);
  preferences.end();
}

void Preference::getHomeLocation(Location &location) {
  Preferences preferences;
  preferences.begin("mcompass", false);
  if (!preferences.isKey("latitude") || !preferences.isKey("longitude")) {
    preferences.end();
    return;
  }
  location.latitude = preferences.getFloat("latitude", 0);
  location.longitude = preferences.getFloat("longitude", 0);
  preferences.end();
}

void Preference::saveNeedleColor(NeedleColor color) {
  Preferences preferences;
  preferences.begin("mcompass", false);
  preferences.putInt("spawnColor", color.spawnColor);
  preferences.putInt("southColor", color.southColor);
  preferences.end();
  ESP_LOGE(TAG, "spawnColor=%x southColor=%x", color.spawnColor,
           color.southColor);
}

void Preference::getNeedleColor(NeedleColor &color) {
  Preferences preferences;
  preferences.begin("mcompass", false);
  if (!preferences.isKey("spawnColor") || !preferences.isKey("southColor")) {
    ESP_LOGE(TAG, "!spawnColor !southColor");
    color.spawnColor = DEFAULT_NEEDLE_COLOR;
    color.southColor = DEFAULT_NEEDLE_COLOR;
    preferences.end();
    return;
  }
  color.spawnColor = preferences.getInt("spawnColor", DEFAULT_NEEDLE_COLOR);
  color.southColor = preferences.getInt("southColor", DEFAULT_NEEDLE_COLOR);
  ESP_LOGE(TAG, "spawnColor=%x southColor=%x", color.spawnColor,
           color.southColor);
  preferences.end();
}

void Preference::setWebServerConfig(bool enable) {
  Preferences preferences;
  preferences.begin("mcompass", false);
  preferences.putBool("enableWebServer", enable);
  preferences.end();
}

void Preference::getWebServerConfig(bool &enable) {
  Preferences preferences;
  preferences.begin("mcompass", false);
  if (!preferences.isKey("enableWebServer")) {
    preferences.end();
    enable = false;
    return;
  }
  enable = preferences.getBool("enableWebServer", false);
  preferences.end();
}