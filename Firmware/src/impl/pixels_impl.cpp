#include <FastLED.h>

#include "board.h"
#include "compass_frames.h"
#include "context.h"
#include "font.h"
#include "utils.h"

using namespace mcompass;

static CRGB leds[NUM_LEDS];

static const char *TAG = "PIXEL";

static uint32_t pColor = DEFAULT_POINTER_COLOR;

// 屏幕布局定义
const uint8_t mask[5][10] = {{0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
                             {0, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                             {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                             {0, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                             {0, 0, 1, 1, 1, 1, 1, 1, 0, 0}};

// 坐标到LED索引的映射表（预计算）
int ledMap[5][10] = {{-1, -1, 8, 9, 18, 19, 28, 29, 37, 38},
                     {-1, 1, 7, 10, 17, 20, 27, 30, 36, 39},
                     {0, 2, 6, 11, 16, 21, 26, 31, 35, 40},
                     {-1, 3, 5, 12, 15, 22, 25, 32, 34, 41},
                     {-1, -1, 4, 13, 14, 23, 24, 33, -1, -1}};

// 将物理坐标转换为LED索引
int getLedIndex(uint8_t row, uint8_t col) {
  if (row >= 5 || col >= 10)
    return -1;
  return ledMap[row][col];
}

// 绘制像素
void drawPixel(uint8_t row, uint8_t col, uint32_t color) {
  int index = getLedIndex(row, col);
  if (index >= 0 && index <= NUM_LEDS) {
    leds[index] = color;
  }
}

// 显示字符（支持滚动）
void pixel::drawChar(char c, int startX, int startY, uint32_t color) {
  uint8_t charIndex = 0;

  // 字符映射（可扩展）
  if (c >= '0' && c <= '9') {
    charIndex = c - '0';
  } else if (c >= 'a' && c <= 'z') {
    charIndex = c - 'a' + 10;
  } else {
    return;
  }

  // 每个字符占3列
  for (int charCol = 0; charCol < 3; charCol++) {
    int screenCol = startX + charCol; // 计算屏幕上的列坐标
    // if (screenCol < 0 || screenCol >= 10) {
    //   ESP_LOGW(TAG, "skip screenCol < 0 || screenCol >= 10 %d,", screenCol);
    //   continue;  // 列越界跳过
    // }
    for (int charRow = 0; charRow < 5; charRow++) {
      int screenRow = startY + charRow; // 计算屏幕上的行坐标
      // if (screenRow < 0 || screenRow >= 5) {
      //   ESP_LOGW(TAG, "skip screenRow < 0 || screenRow >= 5 %d,", screenRow);
      //   continue;  // 行越界跳过
      // }

      // 检查mask和字库数据
      if (mask[screenRow][screenCol]) {
        bool on = (font3x5[charIndex][charRow] >> (2 - charCol)) &
                  1; // 获取字库像素值
        if (on) {
          drawPixel(screenRow, screenCol, color); // 绘制有效像素
        }
      }
    }
  }
}

void pixel::init(Context *context) {
  uint8_t brightness = 64;
  preference::getBrightness(brightness);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);
  ESP_LOGI(TAG, "set brightness %d", brightness);
}

void pixel::bootAnimation(void (*callbackFn)()) {
  for (int i = 0; i < 59; i++) {
    showFrame(i % MAX_FRAME_INDEX);
    unsigned long start = millis();
    if (callbackFn) {
      callbackFn();
    }
    unsigned long cost = millis() - start;
    // if (cost < 50) {
    //   delay(50 - cost);
    // }
    delay(50);
  }
}

void pixel::theNether() {
  // 当前帧索引
  static int curIndex = 0;
  // 目标帧索引
  static int targetIndex = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = frames[curIndex][i];
  }
  FastLED.show();
  if (curIndex == targetIndex) {
    targetIndex = random(0, MAX_FRAME_INDEX);
  } else {
    if (curIndex < targetIndex) {
      curIndex += 1;
    } else {
      curIndex -= 1;
    }
  }
  if (curIndex < 0) {
    curIndex = MAX_FRAME_INDEX;
  }
  if (curIndex > MAX_FRAME_INDEX) {
    curIndex = 0;
  }
}

void pixel::showFrame(int index) {
  if (index > MAX_FRAME_INDEX || index < 0) {
    return;
  }
  // Serial.printf("showFrame: relative index=%f,", index);
  static uint32_t color = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = frames[index][i] == DEFAULT_POINTER_COLOR ||
                      frames[index][i] == 0xcb1a1a ||
                      frames[index][i] == 0xbe1515
                  ? pColor
                  : frames[index][i];
  }
  FastLED.show();
}

void pixel::showByAzimuth(float azimuth) {
  if (azimuth < 0 || azimuth > 360) {
    // 不响应不合法的方位角
    return;
  }

  // 原始素材中指针的方位角不是均匀分布的
  // 去除高度重复帧后, 得到27帧不同的图像, 其中第N张图像对应
  // 1 正上
  // 8 正右
  // 14 正下
  // 21 正右
  int index = 0;
  // 较小的间距
  float step_small = 90.0 / 7.0;
  // 间距(90°~180°)
  float step = 90.0 / 6.0;
  if (azimuth < step_small * 6) {
    // 90.0 / (8 - 1)
    index = static_cast<int>(azimuth / step_small);
  } else if (azimuth < 90 + step * 5) {
    // 90.0 / (14 - 8)
    index = 7 + static_cast<int>((azimuth - 90.0) / step);
  } else if (azimuth < 180 + step_small * 6) {
    // 90.0 / (21 - 14)
    index = 13 + static_cast<int>((azimuth - 180.0) / step_small);
  } else {
    // 90.0 / (28 - 21)
    index = 20 + static_cast<int>((azimuth - 270.0) / step_small);
  }

  // 限制边界
  index = min(MAX_FRAME_INDEX, index);
  index = max(0, index);
  showFrame(index);
}

void pixel::showFrameByBearing(float bearing, int azimuth) {
  // 当前方位角对应的索引
  int aIndex = (int)(azimuth / 360.0 * MAX_FRAME_INDEX);
  // 目标方位角对应的索引
  int bIndex = (int)(bearing / 360.0 * MAX_FRAME_INDEX);
  // 计算差值
  float degree = azimuth - bearing;
  if (degree < 0) {
    degree += 360;
  }
  showByAzimuth(degree);
}

void pixel::showFrameByLocation(float latA, float lonA, float latB, float lonB,
                                int azimuth) {
  float bearing = utils::calculateBearing(latA, lonA, latB, lonB);

  // 由于我们的0度定义为正南方, 而calculateBearing是以正北方为0度计算的
  // 所以需要对这个结果进行调整
  bearing = 180.0f - bearing;
  if (bearing < 0) {
    bearing += 360.0f;
  }
  ESP_LOGI(TAG, "showFrameByLocation: bearing=%f, azimuth=%d", bearing,
           azimuth);
  showFrameByBearing(bearing, azimuth);
}

void pixel::showSolid(int color) {
  fill_solid(leds, NUM_LEDS, CRGB(color));
  FastLED.show();
}

static void showBouncing(int color) {
  FastLED.clear();
  static int indexes[] = {0, 2, 6, 11, 16, 21, 26, 31, 35, 40};
  static int dir = 1;
  static int index = 0;

  if (index >= 9) {
    dir = -1;
  } else if (index <= 0) {
    dir = 1;
  }

  for (int i = 0; i < 10; i++) {
    // Calculate distance from current index (0-9)
    float distance = abs(i - index);
    // Use gaussian/normal distribution formula
    float brightness = 255 * exp(-(distance * distance) /
                                 (2 * 1.5)); // sigma=1.5 controls spread

    leds[indexes[i]] = color;
    // Apply calculated brightness
    if (distance > 0) {
      leds[indexes[i]].fadeToBlackBy(255 - (int)brightness);
    }
  }
  index += dir;
  FastLED.show();
}

void pixel::showServerWifi() {
  showBouncing(CRGB::Green);
  delay(100);
}

void pixel::showServerSpawn() {
  showBouncing(CRGB::Blue);
  delay(100);
}

void pixel::showServerInfo() {
  showBouncing(CRGB::Red);
  delay(100);
}

void pixel::setBrightness(uint8_t brightness) {
  FastLED.setBrightness(brightness);
}

void pixel::setPointerColor(uint32_t pointColor) { pColor = pointColor; }

void pixel::counterDown(int seconds) {
  for (int i = seconds; i >= 0; i--) {
    FastLED.clear();
    ESP_LOGI(TAG, "counterDown: %d", i);
    drawChar('0' + i, 4, 0, CRGB::Red);
    FastLED.show();
    delay(1000);
  }
}

void pixel::clear() { FastLED.clear(); }

void pixel::show() { FastLED.show(); }