#pragma once
#include "common.h"
#include "macro_def.h"

namespace mcompass {
class Context;
namespace pixel {

/**
 * @brief LED初始化
 */
void init(mcompass::Context *context);
/**
 * @brief 启动动画
 */
void bootAnimation(void (*callbackFn)() = nullptr);

/**
 * @brief 丢失方位
 */
void lostBearing();
/**
 * @brief 地狱, 无GPS信号
 */
void theNether();
/**
 * @brief 显示帧
 * @param index 帧索引
 * @param overrideColor 重载颜色, 用来覆盖指针颜色, 默认红色指针
 */
void showFrame(int index);
/**
 * @brief 显示方位角
 * @param azimuth 方位角 范围应当是0~360
 */
void showByAzimuth(float azimuth);
/**
 * @brief 根据方位角显示帧
 * @param bearing 方位角
 * @param azimuth 当前罗盘方位角
 */
void showFrameByBearing(float bearing, int azimuth);
/**
 * @brief 根据位置显示帧
 * @param latitudeA 目标位置纬度
 * @param longitudeA 目标位置经度
 * @param latitudeB 当前位置纬度
 * @param longitudeB 当前位置经度
 * @param azimuth 当前罗盘方位角
 */
void showFrameByLocation(float latA, float lonA, float latB, float lonB,
                         int azimuth);
/**
 * @brief 热点
 */
void showHotspot();

/**
 * @brief 显示纯色
 * @param color 颜色
 */
void showSolid(int color);
/**
 * @brief 连接WiFi
 */
void showConnectingWifi();
/**
 * @brief 服务器颜色
 */
void showServerColors();
/**
 * @brief 服务器WiFi
 */
void showServerWifi();
/**
 * @brief 服务器生成
 */
void showServerSpawn();
/**
 * @brief 服务器信息
 */
void showServerInfo();
/**
 * @brief 绘制字符
 */
void drawChar(char c, int startX, int startY, uint32_t color);
/**
 * @brief 设置亮度
 */
void setBrightness(uint8_t brightness);

/**
 * @brief 设置指针颜色
 */
void setPointerColor(uint32_t pointColor);

/**
 * @brief 倒计时
 */
void counterDown(int seconds);

/**
 * @brief 清除显示
 */
void clear();

/**
 * @brief 刷新显示
 */
void show();

} // namespace pixel
} // namespace mcompass
