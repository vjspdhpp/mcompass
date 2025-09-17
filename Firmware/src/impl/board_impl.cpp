#include <Arduino.h>
#include <esp_log.h>
#include <esp_wifi.h>

#include "board.h"
#include "context.h"
#include "event.h"
#include "states/CompassState.h"

using namespace mcompass;
static const char *TAG = "BOARD";
static uint32_t last_time = 0;
Context &context = Context::getInstance();

float g_interpolated_azimuth = 0.0f; // 模拟指针的当前位置
float g_azimuth_velocity = 0.0f;     // 模拟指针的当前速度

// 弹簧刚度 (k): 越大, "拉力"越强, 反应越快, 也越容易超调
const float spring_constant = 60.0f;
// 阻尼系数 (c): 越大, "摩擦力"越大, 摆动越快停止。
//   - 如果太小: 会一直抖动
//   - 如果太大: 会缓慢接近目标, 不会超调 (过阻尼)
const float damping_coefficient = 6.0f;
// 质量 (m): 越大, "惯性"越大, 反应越慢, 越不容易被拉动
const float mass = 1.0f;

// 时间步长 (dt): 1.0 / 60Hz
const float dt = 1.0f / 60.0f;

static void setupContext() {
  preference::init(&context);
  // 根据设备型号设置默认订阅源
  if (context.isGPSModel()) {
    context.setSubscribeSource(Event::Source::NETHER);
  } else {
    context.setSubscribeSource(Event::Source::SENSOR);
  }
}

// 校准检测
void calibrateCheck() {
  if (digitalRead(CALIBRATE_PIN) == LOW) {
    Context::getInstance().setDeviceState(State::CALIBRATE);
  }
}

void board::init() {
  Serial.begin(115200);
  delay(1000);
  ESP_LOGI(TAG, "Board init %p", &context);
  // 初始化上下文
  setupContext();
  // 设置引脚模式
  pinMode(CALIBRATE_PIN, INPUT_PULLUP);
  pinMode(GPS_EN_PIN, OUTPUT);
  // 关闭GPS电源
  digitalWrite(GPS_EN_PIN, HIGH);
  // 初始化串口
  Serial.begin(115200);
  // 初始化LED
  pixel::init(&context);
  // 初始化按钮
  button::init(&context);
  // 初始化罗盘传感器
  sensor::init(&context);
  // 如果传感器初始化失败,则直接返回
  if (!context.getHasSensor()) {
    return;
  }
  // GPS型号才需要初始化GPS
  if (context.isGPSModel()) {
    gps::init(&context);
  }
  /////////////////////// 根据服务器模式初始化 ///////////////////////
  context.getServerMode() == ServerMode::BLE ? ble_server::init(&context)
                                             : web_server::init(&context);
  char buffer[256];
  context.logSelf(buffer);
  ESP_LOGI(TAG, "Context: %s", buffer);

  /////////////////////// 创建按钮定时器 ///////////////////////
  esp_timer_handle_t timer;
  esp_timer_create_args_t timer_args = {
      .callback = [](void *) { button::tick(); },
      .arg = nullptr,
      .dispatch_method = ESP_TIMER_TASK,
      .name = "button_timer",
      .skip_unhandled_events = true};
  esp_timer_create(&timer_args, &timer);
  esp_timer_start_periodic(timer, 10000); // 10ms = 10000us
  /////////////////////// 创建传感器定时器 ///////////////////////
  esp_timer_handle_t sensor_timer;
  esp_timer_create_args_t sensor_timer_args = {
      .callback =
          [](void *) {
            auto target_azimuth = sensor::getAzimuth();

            // 2. 计算 "目标" 与 "当前" 之间的最短角度差 (位移 x)
            float difference = target_azimuth - g_interpolated_azimuth;

            // 确保我们总是走最短路径
            if (difference > 180.0f) {
              difference -= 360.0f;
            } else if (difference < -180.0f) {
              difference += 360.0f;
            }
            // 现在 'difference' 是指针需要转动的最短角度, 比如 -10 度或 +20 度

            // 3. 计算弹簧力 (F_spring = k * x)
            // "拉"向目标的力
            float spring_force = spring_constant * difference;

            // 4. 计算阻尼力 (F_damping = -c * v)
            // 与当前运动方向相反的 "摩擦" 力
            float damping_force = -damping_coefficient * g_azimuth_velocity;

            // 5. 计算总受力并得出加速度 (F_total = m * a  =>  a = F_total / m)
            float acceleration = (spring_force + damping_force) / mass;

            // 6. 更新速度 (v = v_0 + a * dt) (简单的欧拉积分)
            g_azimuth_velocity += acceleration * dt;

            // 7. 更新位置 (p = p_0 + v * dt)
            g_interpolated_azimuth += g_azimuth_velocity * dt;

            // 8. 将角度标准化到 [0, 360) 范围内
            g_interpolated_azimuth = fmod(g_interpolated_azimuth, 360.0f);
            if (g_interpolated_azimuth < 0.0f) {
              g_interpolated_azimuth += 360.0f;
            }

            // 9. 使用这个新的、插值后的 "弹性" 角度
            Event::Body event;
            event.type = Event::Type::AZIMUTH;
            event.source = Event::Source::SENSOR;
            // 使用插值后的值, 而不是传感器的原始值
            event.azimuth.angle = g_interpolated_azimuth;
            esp_event_post_to(context.getEventLoop(), MCOMPASS_EVENT, 0, &event,
                              sizeof(event), 0);
          },
      .arg = nullptr,
      .dispatch_method = ESP_TIMER_TASK,
      .name = "sensor_timer",
      .skip_unhandled_events = true};
  esp_timer_create(&sensor_timer_args, &sensor_timer);
  esp_timer_start_periodic(sensor_timer, 16667); // 16.667ms
  /////////////////////// 创建Nether数据源定时器 ///////////////////////
  esp_timer_handle_t nether_timer;
  esp_timer_create_args_t nether_timer_args = {
      .callback =
          [](void *) {
            static int targetIndex = random(0, MAX_FRAME_INDEX); // 随机目标索引
            static int currentIndex = 0;                         // 当前索引
            const int step = 1;                                  // 每次移动步长

            // 逼近目标索引
            if (currentIndex != targetIndex) {
              if (currentIndex < targetIndex) {
                currentIndex += step;
              } else {
                currentIndex -= step;
              }
            } else {
              // 到达目标后生成新随机索引
              targetIndex = random(0, MAX_FRAME_INDEX);
            }

            // 处理索引越界
            currentIndex = (currentIndex + MAX_FRAME_INDEX) % MAX_FRAME_INDEX;

            // 根据索引计算方位角（均匀分布）
            int azimuth = (currentIndex * 360) / MAX_FRAME_INDEX;

            // ESP_LOGI(TAG, "NETHER currentIndex=%d, targetIndex=%d,
            // azimuth=%d",
            //          currentIndex, targetIndex, azimuth);

            // 发送方位角事件
            Event::Body event;
            event.type = Event::Type::AZIMUTH;
            event.source = Event::Source::NETHER;
            event.azimuth.angle = azimuth;
            esp_event_post_to(context.getEventLoop(), MCOMPASS_EVENT, 0, &event,
                              sizeof(event), 0);
          },
      .arg = nullptr,
      .dispatch_method = ESP_TIMER_TASK,
      .name = "nether_timer",
      .skip_unhandled_events = true};
  esp_timer_create(&nether_timer_args, &nether_timer);
  esp_timer_start_periodic(nether_timer, 50000); // 50ms

  context.setState(new CompassState());
}