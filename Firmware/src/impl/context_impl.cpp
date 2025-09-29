#include "context.h"

#include "IState.h"
#include "utils.h"

using namespace mcompass;

void Context::setModel(Model model) { this->model = model; }

Model Context::getModel() const { return model; }

bool Context::isModel(const Model model) { return this->model == model; }

bool Context::isGPSModel() { return this->isModel(mcompass::Model::GPS); }

State Context::getDeviceState() const { return deviceState; }
void Context::setDeviceState(State state) { deviceState = state; }

State Context::getLastDeviceState() const { return lastDeviceState; }
void Context::setLastDeviceState(State state) { lastDeviceState = state; }

WorkType Context::getWorkType() const { return workType; }
void Context::setWorkType(WorkType wt) { workType = wt; }
void Context::toggleWorkType() {
  setWorkType(workType == mcompass::WorkType::SPAWN
                  ? mcompass::WorkType::SOUTH
                  : mcompass::WorkType::SPAWN);
}

bool Context::getDetectGPS() const { return detectGPS; }
void Context::setDetectGPS(bool detect) { detectGPS = detect; }

SensorModel Context::getSensorModel() const { return sensorModel; }
void Context::setSensorModel(SensorModel sm) { sensorModel = sm; }

bool Context::getHasSensor() const { return hasSensor; }
void Context::setHasSensor(bool sensor) { hasSensor = sensor; }

PointerColor Context::getColor() const { return color; }
void Context::setColor(PointerColor c) { color = c; }

Location Context::getCurrentLocation() const { return currentLoc; }
void Context::setCurrentLocation(const Location &loc) { currentLoc = loc; }

Location Context::getSpawnLocation() const { return spawnLocation; }
void Context::setSpawnLocation(const Location &loc) { spawnLocation = loc; }

ServerMode Context::getServerMode() const { return serverMode; }
void Context::setServerMode(ServerMode mode) { serverMode = mode; }

uint8_t Context::getBrightness() const { return brightness; }
void Context::setBrightness(uint8_t bright) { brightness = bright; }

String Context::getSsid() const { return ssid; }
void Context::setSsid(const String &id) { ssid = id; }

String Context::getPassword() const { return password; }
void Context::setPassword(const String &pass) { password = pass; }

Event::Source Context::getSubscribeSource() const { return subscribeSource; }
void Context::setSubscribeSource(Event::Source src) { subscribeSource = src; }

int Context::getAzimuth() const { return azimuth; }
void Context::setAzimuth(int azi) {
  setLastAzimuth(azimuth);
  azimuth = azi;
}

int Context::getLastAzimuth() const { return lastAzimuth; }
void Context::setLastAzimuth(int azi) { lastAzimuth = azi; }

esp_event_loop_handle_t Context::getEventLoop() { return eventLoop; }

void Context::setEventLoop(esp_event_loop_handle_t loop) { eventLoop = loop; }

void Context::logSelf(char *buffer) {
  sprintf(buffer,
          "{\n "
          "ServerMode:%d\n PointerColor{spawnColor:0x%x,southColor:0x%x}\n "
          "Brightness:%d\n "
          "SpawnLocation:{latitude:%.2f,longitude:%.2f}\n WiFi:%s\n Model:%s\n "
          "HasSensor:%d\n Sensor Model:%s\n "
          "detectGPS:%d\n "
          "}",
          this->getServerMode(), this->getColor().spawnColor,
          this->getColor().southColor, this->getBrightness(),
          this->getSpawnLocation().latitude, this->getSpawnLocation().longitude,
          this->getSsid(), this->getModel() == Model::GPS ? "GPS" : "LITE",
          this->getHasSensor(),
          utils::sensorModel2Str(this->getSensorModel()).c_str(),
          this->getDetectGPS());
}

void Context::setIsGPSFixed(bool isFixed) { isGPSFixed = isFixed; }
bool Context::getIsGPSFixed() const { return isGPSFixed; }

void Context::setState(IState *newState) {
  if (m_currentState != nullptr) {
    ESP_LOGI("Context", "Exiting state: %s", m_currentState->getName());
    m_currentState->onExit(*this);
    delete m_currentState; // 删除旧状态对象
  }

  m_currentState = newState;

  if (m_currentState != nullptr) {
    ESP_LOGI("Context", "Entering state: %s", m_currentState->getName());
    m_currentState->onEnter(*this);
  }
}

IState *Context::getCurrentState() { return m_currentState; }


