#include <OneButton.h>

#include "board.h"

using namespace mcompass;
static const char* TAG = "Button";

static OneButton buttonInstance(CALIBRATE_PIN, true);

void button::init(Context* context) {
  ESP_LOGI(TAG, "Button init %p", context);
  auto ctx = context;
  /////////////////////// 初始化按钮 ///////////////////////
  // 单击事件
  buttonInstance.attachClick(
      [](void* ctx) {
        ESP_LOGI(TAG, "Button clicked");
        auto context = static_cast<Context*>(ctx);
        auto deviceState = context->getDeviceState();
        auto workType = context->getWorkType();
        switch (deviceState) {
          case State::COMPASS: {
            // 切换罗盘工作类型
            ESP_LOGI(TAG, "Toggle WorkType to %s",
                     workType == WorkType::SPAWN ? "SPAWN" : "SOUTH");
            context->toggleWorkType();

            break;
          }

          default:
            break;
        }
      },
      ctx);
  // 多次点击
  buttonInstance.attachMultiClick(
      [](void* ctx) {
        ESP_LOGI(TAG, "Button multi clicked");
        auto context = static_cast<Context*>(ctx);
        auto deviceState = context->getDeviceState();
        if (deviceState != State::COMPASS) return;
        int n = buttonInstance.getNumberClicks();
        if (n <= 5) return;
        ESP_LOGW(TAG, "Factory Reset!!!");
        // 恢复出厂设置
        // 倒计时3秒
        pixel::counterDown(3);
        preference::factoryReset();
        // 重启
        esp_restart();
      },
      ctx);
  // 长按事件
  buttonInstance.attachLongPressStart(
      [](void* ctx) {
        ESP_LOGI(TAG, "Button long pressed");
        auto context = static_cast<Context*>(ctx);
        auto deviceState = context->getDeviceState();
        auto workType = context->getWorkType();
        auto currentLoc = context->getCurrentLocation();
        switch (deviceState) {
          // COMPASS模式下响应长按事件
          case State::COMPASS: {
            // 出生针模式下， 长按设置新的出生点
            if (workType == WorkType::SPAWN) {
              // 检查GPS坐标是否有效
              if (gps::isValidGPSLocation(currentLoc)) {
                preference::saveSpawnLocation(currentLoc);
                context->setSpawnLocation(currentLoc);
                ESP_LOGI(TAG, "Set New Home Location to {%.2f,%.2f}",
                         currentLoc.latitude, currentLoc.longitude);
              } else {
                ESP_LOGW(TAG, "Can't set home, invalid GPS data.");
              }
            } else if (workType == WorkType::SOUTH) {
              // 指南针模式下， 长按切换数据源到nether
              ESP_LOGI(TAG, "Switch Data Source to Nether");
              context->setSubscribeSource(Event::Source::NETHER);
            }
            break;
          }
          default:
            break;
        }
      },
      ctx);
}

void button::tick() { buttonInstance.tick(); }