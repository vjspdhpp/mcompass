#include "states/CalibratingState.h" // 用于状态切换
#include "context.h"
#include "pixel_def.h"
#include "sensor_def.h"

using namespace mcompass;

void CalibratingState::onEnter(Context &context) {

};
void CalibratingState::onExit(Context &context) {

};
void CalibratingState::handleEvent(Context &context, Event::Body *evt) {
  auto deviceState = context.getDeviceState();
  ESP_LOGI(getName(), "deviceState=%d", deviceState);
  context.setDeviceState(State::INFO);
  // 倒计时3秒
  pixel::counterDown(3);
  String text = String("Claibrate");

  xTaskCreate(
      [](void *ctx) {
        auto context = static_cast<Context *>(ctx);
        ESP_LOGI("", "Calibrate Start");
        sensor::calibrate();
        context->setDeviceState(State::COMPASS);
        ESP_LOGI("", "Calibrate Done.");
        vTaskDelete(NULL);
      },
      "calibrate", 8192, &context, configMAX_PRIORITIES - 1, NULL);

  int x = 0;
  size_t length = text.length();
  int xLimit = -length * 4;
  while (context.getDeviceState() == State::INFO) {
    pixel::clear();
    for (size_t i = 0; i < length; i++) {
      pixel::drawChar(text.charAt(i), x + i * 4, 0, 0x00ff00);
    }
    pixel::show();
    delay(100);
    x--;
    if (x < xLimit) {
      x = 10;
    }
  }
  ESP_LOGI(getName(), "Exit Info State");

  esp_restart();
};