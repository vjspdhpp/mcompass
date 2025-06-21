#include <Preferences.h>

#include "board.h"
#include "macro_def.h"
using namespace mcompass;

static const char *TAG = "Preference";
static Context *ctx;

void preference::init(Context *context) {
  ctx = context;

  ServerMode tempServerMode;
  PointerColor tempPointerColor;
  uint8_t tempBrightness;
  Location tempSpawnLocation;
  String tempSsid;
  String tempPassword;
  Model tempDeviceModel;

  preference::getServerMode(tempServerMode);
  preference::getPointerColor(tempPointerColor);
  preference::getBrightness(tempBrightness);
  preference::getSpawnLocation(tempSpawnLocation);
  preference::getWiFiCredentials(tempSsid, tempPassword);
  preference::getCustomDeviceModel(tempDeviceModel);

  ctx->setServerMode(tempServerMode);
  ctx->setColor(tempPointerColor);
  ctx->setBrightness(tempBrightness);
  ctx->setSpawnLocation(tempSpawnLocation);
  ctx->setSsid(tempSsid);
  ctx->setPassword(tempPassword);
  ctx->setModel(tempDeviceModel);
  ctx->setWorkType(tempDeviceModel == Model::GPS ? WorkType::SPAWN
                                                 : WorkType::SOUTH);
}

void preference::saveSpawnLocation(Location location) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  preferences.putFloat(LATITUDE_KEY, location.latitude);
  preferences.putFloat(LONGTITUDE_KEY, location.longitude);
  preferences.end();
}

void preference::getSpawnLocation(Location &location) {
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

void preference::savePointerColor(PointerColor color) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  preferences.putInt(SPAWN_COLOR_KEY, color.spawnColor);
  preferences.putInt(SOUTH_COLOR_KEY, color.southColor);
  preferences.end();
  ESP_LOGE(TAG, "spawnColor=%x southColor=%x", color.spawnColor,
           color.southColor);
}

void preference::getPointerColor(PointerColor &color) {
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

void preference::setServerMode(ServerMode serverMode) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  preferences.putInt(SERVER_MODE_KEY, static_cast<int>(serverMode));
  preferences.end();
}

void preference::getServerMode(ServerMode &serverMode) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  if (!preferences.isKey(SERVER_MODE_KEY)) {
    preferences.end();
    serverMode = DEFAULT_SERVER_MODE;
    return;
  }
  serverMode = static_cast<ServerMode>(preferences.getInt(
      SERVER_MODE_KEY, static_cast<int>(DEFAULT_SERVER_MODE)));
  preferences.end();
}

void preference::setBrightness(uint8_t brightness) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  preferences.putUChar(BRIGHTNESS_KEY, brightness);
  preferences.end();
}

void preference::getBrightness(uint8_t &brightness) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  if (!preferences.isKey(BRIGHTNESS_KEY)) {
    preferences.end();
    return;
  }
  brightness = preferences.getUChar(BRIGHTNESS_KEY, 64);
  ESP_LOGE(TAG, "preferences.getUChar(BRIGHTNESS_KEY)=%d", brightness);
  preferences.end();
}

void preference::setWiFiCredentials(String ssid, String password) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  preferences.putString(WIFI_SSID_KEY, ssid);
  preferences.putString(WIFI_PWD_KEY, password);
  preferences.end();
}

void preference::getWiFiCredentials(String &ssid, String &password) {
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

void preference::factoryReset() {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  preferences.clear();
  preferences.end();
}

void preference::setCustomDeviceModel(Model model) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  preferences.putInt(MODEL_KEY, static_cast<int>(model));
  preferences.end();
}

void preference::getCustomDeviceModel(Model &model) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  if (!preferences.isKey(MODEL_KEY)) {
    model = DEFAULT_MODEL;
    preferences.end();
    return;
  }
  model = static_cast<Model>(
      preferences.getInt(MODEL_KEY, static_cast<int>(DEFAULT_MODEL)));
  preferences.end();
}

/**
 * @brief 设置校准数据
 */
void preference::setCalibration(preference::CalibrationData data) {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  preferences.putBytes(CALIBRATION_KEY, &data, sizeof(preference::CalibrationData));
  preferences.end();
}

/**
 * @brief 获取校准数据
 */
preference::CalibrationData preference::getCalibration() {
  Preferences preferences;
  preferences.begin(PREFERENCE_NAME, false);
  preference::CalibrationData data = {0}; 
  if (!preferences.isKey(CALIBRATION_KEY)) {
    ESP_LOGE(TAG, "!CALIBRATION_KEY");
    preferences.end();
    return data;
  }
  preferences.getBytes(CALIBRATION_KEY, &data, sizeof(preference::CalibrationData));
  preferences.end();
  return data;
}