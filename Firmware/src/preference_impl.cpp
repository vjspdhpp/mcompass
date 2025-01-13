#include <Preferences.h>

#include "func.h"

void saveHomeLocation(Location location) {
  Preferences preferences;
  preferences.begin("mcompass", false);
  preferences.putFloat("latitude", location.latitude);
  preferences.putFloat("longitude", location.longitude);
  preferences.end();
}

void getHomeLocation(Location &location) {
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

void saveNeedleColor(NeedleColor color) {
  Preferences preferences;
  preferences.begin("mcompass", false);
  preferences.begin("mcompass", false);
  preferences.putInt("spawnColor", color.spawnColor);
  preferences.putInt("southColor", color.southColor);
  preferences.end();
}

void getNeedleColor(NeedleColor &color) {
  Preferences preferences;
  preferences.begin("mcompass", false);
  if (!preferences.isKey("spawnColor") || !preferences.isKey("southColor")) {
    color.spawnColor = 0xff0000;
    color.southColor = 0xff0000;
    preferences.end();
    return;
  }
  color.spawnColor = preferences.getInt("spawnColor", 0xff0000);
  color.southColor = preferences.getInt("southColor", 0xff0000);
  preferences.end();
}