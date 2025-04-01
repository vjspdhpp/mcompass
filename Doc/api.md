---
outline: deep
---

### Minecraft Compass Web Server API 文档

by [DoraemonMiku](https://github.com/DoraemonMiku)

---

## **获取设备IP**

### **路径:** `/ip`

- **方法:** `GET`
- **描述:** 返回设备的当前IP地址。

### **请求参数:** 无

### **响应结果:**

- **状态码:** `200 OK`
- **类型:** `text/plain`
- **内容:** 返回设备IP地址。

### **示例响应:**

```
192.168.1.10
```

---
## **重启**

### **路径:** `/restart`

- **方法:** `POST`
- **描述:** 重启设备

### **请求参数:** 无

### **响应结果:**

- **状态码:** `200 OK`
- **类型:** `text/plain`
- **内容:** 无

## **设置帧索引** [调试接口]

### **路径:** `/setIndex`

- **方法:** `POST`
- **描述:** 设置当前显示的帧索引（调试用）。

### **请求参数:**

| 参数名     | 类型    | 必填  | 描述       |
| ------- | ----- | --- | -------- |
| `index` | `Int` | 是   | 要显示的帧索引（0-最大帧索引） |
| `color` | `String` | 否   | 可选颜色值（默认使用当前颜色） |

### **响应结果:**

- **成功:**
  - **状态码:** `200 OK`
  - **内容:** `OK`
- **失败:**
  - **状态码:** `400 Bad Request`
  - **内容:** `Missing index parameter`

### **示例请求:**

```
POST /setIndex
Content-Type: application/x-www-form-urlencoded

index=3
```

### **示例响应:**

```
OK
```

---

## **获取设备信息**

### **路径:** `/info`

- **方法:** `GET`
- **描述:** 获取设备固件的构建信息，包括版本号和Git信息。

### **请求参数:** 无

### **响应结果:**

- **状态码:** `200 OK`
- **类型:** `application/json`
- **内容:** JSON格式设备信息。

### **响应字段说明:**

| 字段名            | 类型       | 描述         |
| -------------- | -------- | ---------- |
| `buildDate`    | `String` | 固件构建日期。    |
| `buildTime`    | `String` | 固件构建时间。    |
| `buildVersion` | `String` | 固件版本号。      |
| `gitBranch`    | `String` | Git分支名。       |
| `gitCommit`    | `String` | Git提交的哈希值。 |
| `model`        | `String` | 设备型号。  GPS版本为1，标准版为0 |
| `gpsStatus`    | `String` | GPS状态。 检测到了为1，否则为0 |
| `sensorStatus` | `String` | 传感器状态。 初始化成功为1，否则为0 |

### **示例响应:**

```json
{
  "buildDate": "Nov 27 2024",
  "buildTime": "10:30:00",
  "buildVersion": "1.0.0",
  "gitBranch": "main",
  "gitCommit": "abc123",
  "model": "1",
  "gpsStatus": "1",
  "sensorStatus": "1"
}
```

---

## **获取WiFi信息**

### **路径:** `/wifi`

- **方法:** `GET`
- **描述:** 获取设备当前保存的WiFi信息。

### **请求参数:** 无

### **响应结果:**

- **状态码:** `200 OK`
- **类型:** `application/json`
- **内容:** WiFi信息。

### **响应字段说明:**

| 字段名        | 类型       | 描述         |
| ---------- | -------- | ---------- |
| `ssid`     | `String` | WiFi网络的名称。 |
| `password` | `String` | WiFi网络的密码。 |

### **示例响应:**

```json
{
  "ssid": "MyNetwork",
  "password": "password123"
}
```

---

## **获取出生点信息**

### **路径:** `/spawn`

- **方法:** `GET`
- **描述:** 获取设备当前存储的出生点（经纬度）。

### **请求参数:** 无

### **响应结果:**

- **状态码:** `200 OK`
- **类型:** `application/json`
- **内容:** 经纬度信息。

### **响应字段说明:**

| 字段名         | 类型      | 描述        |
| ----------- | ------- | --------- |
| `latitude`  | `Float` | 当前设置的纬度值。 |
| `longitude` | `Float` | 当前设置的经度值。 |

### **示例响应:**

```json
{
  "latitude": 37.7749,
  "longitude": -122.4194
}
```

---

## **设置出生点**

### **路径:** `/spawn`

- **方法:** `POST`
- **描述:** 设置设备的出生点（经纬度）。

### **请求参数:**

| 参数名         | 类型      | 必填  | 描述       |
| ----------- | ------- | --- | -------- |
| `latitude`  | `Float` | 是   | 要设置的纬度值。 |
| `longitude` | `Float` | 是   | 要设置的经度值。 |

### **响应结果:**

- **状态码:** `200 OK`

### **示例请求:**

```
POST /spawn
Content-Type: application/x-www-form-urlencoded

latitude=37.7749&longitude=-122.4194
```

### **示例响应:**

```
OK
```

---

## **设置指针颜色**

### **路径:** `/pointColors`

- **方法:** `POST`
- **描述:** 设置指针颜色

### **请求参数:**

| 参数名     | 类型       | 必填  | 描述                          |
| ------- | -------- | --- | --------------------------- |
| `spawnColor` | `String` | 否   | 出生针颜色 颜色的十六进制RGB值（例如: `#FF5252`）。 |
| `southColor` | `String` | 否   | 指南针颜色 颜色的十六进制RGB值（例如: `#FF5252`）。 |

### **响应结果:**

- **状态码:** `200 OK`

### **示例请求:**

```
POST /pointColors
Content-Type: application/x-www-form-urlencoded

spawnColor=#FF5252&southColor=#FF5252
```

## **获取指针颜色**

### **路径:** `/pointColors`

- **方法:** `GET`
- **描述:** 获取指针颜色

### **请求参数:**


### **响应结果:**

- **状态码:** `200 OK`
- **类型:** `application/json`
- **内容:** 指针颜色。

### **响应字段说明:**

| 字段名        | 类型       | 描述         |
| ---------- | -------- | ---------- |
| `spawnColor` | `String` | 出生针颜色。 |
| `southColor` | `String` | 指南针颜色。 |

### **示例请求:**

```
GET /pointColors
Content-Type: application/x-www-form-urlencoded

spawnColor=#FF5252&southColor=#FF5252
```

---

## **设置方位角**

### **路径:** `/setAzimuth`

- **方法:** `POST`
- **描述:** 设置设备的当前方位角（单位为度）。

### **请求参数:**

| 参数名       | 类型      | 必填  | 描述            |
| --------- | ------- | --- | ------------- |
| `azimuth` | `Float` | 是   | 方位角值（0~360°）。 |

### **响应结果:**

- **状态码:** `200 OK`

### **示例请求:**

```
POST /setAzimuth
Content-Type: application/x-www-form-urlencoded

azimuth=180.0
```

---

## **设置WiFi** [兼容保留]

### **路径:** `/setWiFi`

- **方法:** `POST`
- **注意:** 此接口为旧版兼容保留，建议使用新的`/wifi`接口

### **请求参数:**

| 参数名        | 类型       | 必填  | 描述        |
| ---------- | -------- | --- | --------- |
| `ssid`     | `String` | 是   | WiFi网络名称。 |
| `password` | `String` | 是   | WiFi网络密码。 |

### **响应结果:**

- **状态码:** `200 OK`

### **行为说明:**

- 提交WiFi设置后，设备会自动重启。

### **示例请求:**

```
POST /setWiFi
Content-Type: application/x-www-form-urlencoded

ssid=MyNetwork&password=password123
```

---

## **亮度控制**

### **路径:** `/brightness`

- **方法:** `GET`
- **描述:** 获取当前LED亮度值

### **响应结果:**

- **状态码:** `200 OK`
- **类型:** `application/json`
- **内容:** 亮度值（0-255）

### **示例响应:**
```json
{"brightness": 128}
```

---

### **路径:** `/brightness`

- **方法:** `POST`
- **描述:** 设置LED亮度

### **请求参数:**

| 参数名         | 类型      | 必填  | 描述       |
| ----------- | ------- | --- | -------- |
| `brightness`| `Int`   | 是   | 亮度值（0-255） |

### **响应结果:**
- **成功:** `200 OK`
- **无效值:** `400 Bad Request` (亮度超出范围)

---

## **高级配置**

### **路径:** `/advancedConfig`

- **方法:** `GET`
- **描述:** 获取高级配置信息

### **响应字段说明:**
| 字段名         | 类型      | 描述                  |
| ----------- | ------- | ------------------- |
| `model`     | `String`| 设备型号（0: 标准版, 1: GPS版） |
| `serverMode`| `String`| 服务模式（0: WiFi, 1: BLE） |

### **示例响应:**
```json
{"model": "1", "serverMode": "0"}
```

---

### **路径:** `/advancedConfig`

- **方法:** `POST`
- **描述:** 设置高级配置

### **请求参数:**
| 参数名         | 类型      | 必填  | 描述                  |
| ----------- | ------- | --- | ------------------- |
| `serverMode`| `String`| 否   | 服务模式（"0": WiFi, "1": BLE） |
| `model`     | `String`| 否   | 设备型号（"0": 标准版, "1": GPS版） |

### **响应结果:**
- **成功:** `200 OK`

---

## **未找到的路径**

- **描述:** 对于未定义的接口，返回404错误。
- **响应结果:**
  - **状态码:** `404 Not Found`
  - **内容:** `Not found`

---

### 注意事项

- 使用`/setWiFi`接口会导致设备重启，请确保参数正确。
- 所有POST请求需要使用`application/x-www-form-urlencoded`格式提交参数。
