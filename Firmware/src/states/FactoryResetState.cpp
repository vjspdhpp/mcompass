#include "states/FactoryResetState.h" // 用于状态切换
#include "context.h"
#include "pixel_def.h"
#include "preference_def.h"

using namespace mcompass;
void FactoryResetState::onEnter(Context &context) {

};
void FactoryResetState::onExit(Context &context) {

};
void FactoryResetState::handleEvent(Context &context, Event::Body *evt) {
  auto deviceState = context.getDeviceState();
  context.setDeviceState(State::INFO);
  ESP_LOGW(getName(), "Factory Reset!!!");
  // 恢复出厂设置
  // 倒计时3秒
  pixel::counterDown(3);
  preference::factoryReset();
  // 重启
  esp_restart();
};