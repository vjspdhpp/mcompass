#pragma once

#include <string>
#include <vector>

namespace mcompass {
    enum class WorkType;
    enum class SensorModel;
};

namespace utils {

/**
 * @brief 将RGB颜色转换为16进制字符串
 * @param spawnColor 颜色值
 * @return 16进制字符串
 */
std::string toHexString(int spawnColor);

/**
 * @brief 将16进制字符串转换为RGB颜色
 * @param hexColor 16进制字符串
 * @return 颜色值
 */
int fromHexString(const std::string &hexColor);

/**
 * @brief 判断是否插入了USB
 * @return 是否插入了USB
 */
bool isPluggedUSB(void);

/**
 * @brief 分割字符串到vector
 */
std::vector<std::string> split(std::string &s, const std::string &delimiter);

/**
 * @brief 计算方位角
 * @param latA 目标位置纬度
 * @param lonA 目标位置经度
 * @param latB 当前位置纬度
 * @param lonB 当前位置经度
 * @return 方位角
 */
double calculateBearing(double lat1, double lon1, double lat2, double lon2);
double complexDistance(double lat1, double lon1, double lat2, double lon2);

double simplifiedDistance(double lat1, double lon1, double lat2, double lon2);

std::string workType2Str(mcompass::WorkType type);
std::string sensorModel2Str(mcompass::SensorModel model);
}  // namespace utils