#include "board.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nmea_parser.h"
#include "utils.h"
using namespace mcompass;

static const char *TAG = "GPS";
// GPS休眠配置表
static const SleepConfig sleepConfigs[] = {
    {10.0f, 0, true},          // 在10KM距离内，不休眠
    {50.0f, 5 * 60, false},    // 超过50KM，休眠5分钟
    {100.0f, 10 * 60, false},  // 超过100KM，休眠10分钟
    {200.0f, 15 * 60, false},  // 超过200KM，休眠15分钟
};

// GPS休眠时间
static uint32_t gpsSleepInterval = 60 * 60;  // 单位:秒

static nmea_parser_handle_t nmea_hdl = NULL;

/**
 * @brief GPS Event Handler
 *
 * @param event_handler_arg handler specific arguments
 * @param event_base event base, here is fixed to ESP_NMEA_EVENT
 * @param event_id event id
 * @param event_data event specific arguments
 */
static void gps_event_handler(void *event_handler_arg,
                              esp_event_base_t event_base, int32_t event_id,
                              void *event_data) {
  Context &context = Context::getInstance();
  gps_t *gpsParser = NULL;
  switch (event_id) {
    case GPS_UPDATE: {
      // 检测到任何串口数据,则说明GPS已经接入;
      context.setDetectGPS(true);
      gpsParser = (gps_t *)event_data;
      /* print information parsed from GPS statements */
      ESP_LOGI(TAG,
               "%d,%d/%d/%d %d:%d:%d => \r\n"
               "\t\t\t\t\t\tlatitude   = %.05f°N\r\n"
               "\t\t\t\t\t\tlongitude = %.05f°E\r\n"
               "\t\t\t\t\t\taltitude   = %.02fm\r\n"
               "\t\t\t\t\t\tspeed      = %fm/s",
               gpsParser->valid, gpsParser->date.year + YEAR_BASE,
               gpsParser->date.month, gpsParser->date.day,
               gpsParser->tim.hour + TIME_ZONE, gpsParser->tim.minute,
               gpsParser->tim.second, gpsParser->latitude, gpsParser->longitude,
               gpsParser->altitude, gpsParser->speed);
      if (gpsParser->latitude == 0.0f || gpsParser->longitude == 0.0f) {
        ESP_LOGD(TAG, "INVALID GPS DATA");
        return;
      }
      Location lastestLocation;
      lastestLocation.latitude = gpsParser->latitude;
      lastestLocation.longitude = gpsParser->longitude;

      ESP_LOGD(TAG, "Location:  %f, %f", lastestLocation.latitude,
               lastestLocation.longitude);
      // 坐标有效情况下更新本地坐标
      context.setCurrentLocation(lastestLocation);
      // 设置订阅源
      context.setSubscribeSource(Event::Source::SENSOR);
      // 计算两地距离
      auto currentLoc = context.getCurrentLocation();
      auto targetLoc = context.getSpawnLocation();
      double distance =
          utils::complexDistance(currentLoc.latitude, currentLoc.longitude,
                                 targetLoc.latitude, targetLoc.longitude);
      ESP_LOGI(TAG, "%f km to target.\n", distance);
      // 获取最接近的临界值
      float threshholdDistance = 0;
      size_t sleepConfigSize = sizeof(sleepConfigs) / sizeof(SleepConfig);
      for (int i = sleepConfigSize - 1; i >= 0; i--) {
        if (distance >= sleepConfigs[i].distanceThreshold) {
          threshholdDistance = sleepConfigs[i].distanceThreshold;
          ESP_LOGI(TAG, "use threshold %f km", threshholdDistance);
          break;
        }
      }
      float modDistance = fmod(distance, threshholdDistance);
      // 根据距离调整GPS休眠时间,
      for (int i = 0; i < sleepConfigSize; i++) {
        if (modDistance <= sleepConfigs[i].distanceThreshold) {
          gpsSleepInterval = sleepConfigs[i].sleepInterval;
          if (sleepConfigs[i].gpsPowerEn) {
            digitalWrite(GPS_EN_PIN, LOW);
          } else {
            digitalWrite(GPS_EN_PIN, HIGH);
            // 设置一个休眠定时器
            esp_timer_handle_t gpsSleepTimer;
            esp_timer_create_args_t gpsSleepTimerArgs = {
                .callback = [](void *arg) { digitalWrite(GPS_EN_PIN, LOW); },
                .arg = NULL,
            };
            ESP_ERROR_CHECK(
                esp_timer_create(&gpsSleepTimerArgs, &gpsSleepTimer));
            esp_timer_start_once(gpsSleepTimer, gpsSleepInterval * 1000000);
            ESP_LOGI(TAG, "GPS Sleep %d seconds\n", gpsSleepInterval);
          }
          break;
        }
      }
    } break;
    case GPS_UNKNOWN:
      /* print unknown statements */
      // ESP_LOGW(TAG, "Unknown statement:%s", (char *)event_data);
      break;
    default:
      break;
  }
}

void gps::init(Context *context) {
  // 配置GPS串口
  // GPSSerial.begin(9600, SERIAL_8N1, RX, TX);
  // 设置串口缓冲区大小
  // GPSSerial.setRxBufferSize(1024);
  // 启动GPS,用于GPS存在性检测
  digitalWrite(GPS_EN_PIN, LOW);

  /* NMEA parser configuration */
  nmea_parser_config_t config = NMEA_PARSER_CONFIG_DEFAULT();
  /* init NMEA parser library */
  nmea_hdl = nmea_parser_init(&config);
  /* register event handler for NMEA parser library */
  nmea_parser_add_handler(nmea_hdl, gps_event_handler, context);
  // 检测不到GPS, 关闭GPS的Timer
  esp_timer_handle_t gpsDisableTimer;
  esp_timer_create_args_t gpsDisableTimerArgs = {
      .callback =
          [](void *arg) {
            auto context = static_cast<Context *>(arg);
            if (context->getDetectGPS()) {
              ESP_LOGI(TAG, "GPS detected, skip disable");
              return;
            }
            ESP_LOGI(TAG, "No GPS detected, disable gps power");
            gps::disable();
          },
      .arg = context,
      .dispatch_method = ESP_TIMER_TASK,
      .name = "gpsDisableTimer",
      .skip_unhandled_events = true};
  ESP_ERROR_CHECK(esp_timer_create(&gpsDisableTimerArgs, &gpsDisableTimer));
  esp_timer_start_once(
      gpsDisableTimer,
      DEFAULT_GPS_DETECT_TIMEOUT * 1000000);  // 检测不到GPS, 关闭GPS的Timer
}

/**
 * @brief GPS 关闭
 */
void gps::disable() {
  /* unregister event handler */
  nmea_parser_remove_handler(nmea_hdl, gps_event_handler);
  /* deinit NMEA parser library */
  nmea_parser_deinit(nmea_hdl);
  digitalWrite(GPS_EN_PIN, HIGH);
}

bool gps::isValidGPSLocation(Location location) {
  if (location.latitude >= -90 && location.latitude <= 90 &&
      location.longitude >= -180 && location.longitude <= 180) {
    return true;
  }

  return false;
}
