#include "event.h"
using namespace Event;

const char* Event::SourceToString(Source source) {
  switch (source) {
    case Source::BUTTON:
      return "Button";
    case Source::GPS:
      return "GPS";
    case Source::SENSOR:
      return "Sensor";
    case Source::WEB_SERVER:
      return "WebServer";
    case Source::BLE:
      return "BLE";
    case Source::OTHER:
      return "Other";
    default:
      return "Unknown Source";
  }
}

const char* Event::TypeToString(Type type) {
  switch (type) {
    case Type::AZIMUTH:
      return "Azimuth";
    case Type::MARQUEE:
      return "Marquee";
    default:
      return "Unknown EventType";
  }
}