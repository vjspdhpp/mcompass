---
outline: deep
---
# Minecraft Compass Web Server API Documentation

by [DoraemonMiku](https://github.com/DoraemonMiku)

The API documentation below provides detailed information about all available endpoints for the Minecraft Compass device. Use the sidebar navigation to quickly jump between sections.

## Get Device IP

Retrieves the current IP address of the device.

::: details Endpoint Information
**Path:**`ip`
**Method:**`GET`
**Authentication:**None
:::

### Request

No parameters required.

### Response

**Status Code:**`200 OK`

**Content Type:**`text/plain`

192.168.1.10


## Restart Device

Triggers an immediate device restart.

::: details Endpoint Information

**Path:**`/restart`

**Method:**`POST`

**Authentication:**None

:::

### Request

No parameters required.

### Response

**Status Code:**`200 OK`

**Content Type:**`text/plain` &#x20;

(Empty response body)

## Set Frame Index [Debug Interface]

Sets the currently displayed frame index (for debugging purposes only).

::: details Endpoint Information

**Path:**`/setIndex`

**Method:**`POST`

**Authentication:**None

:::

### Request Parameters

| Parameter | Type     | Required | Description                                  |
|-----------|----------|----------|----------------------------------------------|
| `index`   | `Int`    | ✅ Yes   | Frame index to display (0 - max frame index) |
| `color`   | `String` | ❌ No    | Optional color value (hex format, e.g., `#FF0000`). Uses current color by default. |

### Example Request

POST /setIndex

Content-Type: application/x-www-form-urlencoded

index=3\&color=#FF0000


### Response


**Success:**
  `200 OK` with response body `OK`
**Failure:**
  `400 Bad Request` with response body `Missing index parameter`
## Get Device Information

Retrieves firmware build details and hardware status information.
::: details Endpoint Information
**Path:**`/info`
**Method:**`GET`
**Authentication:**None
  :::

### Request

No parameters required.

### Response

**Status Code:**`200 OK`
**Content Type:**`application/json`

#### Response Fields

| Field          | Type     | Description                                                         |
| -------------- | -------- | ------------------------------------------------------------------- |
| `buildDate`    | `String` | Firmware build date (e.g., "Nov 27 2024")                           |
| `buildTime`    | `String` | Firmware build time (e.g., "10:30:00")                              |
| `buildVersion` | `String` | Firmware version number (e.g., "1.0.0")                             |
| `gitBranch`    | `String` | Git branch name (e.g., "main")                                      |
| `gitCommit`    | `String` | Git commit hash (e.g., "abc123")                                    |
| `model`        | `String` | Device model identifier: `1` (GPS version) / `0` (Standard version) |
| `gpsStatus`    | `String` | GPS module status: `1` (detected) / `0` (not detected)              |
| `sensorStatus` | `String` | Geomagnetic sensor status: `1` (initialized) / `0` (failed)         |

#### Example Response

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

## Get WiFi Information

Retrieves the currently stored WiFi network credentials.

::: details Endpoint Information

**Path:**`/wifi`

**Method:**`GET`

**Authentication:**None

  :::

### Request

No parameters required.

### Response



**Status Code:**`200 OK`

**Content Type:**`application/json`

```json
{
"ssid": "MyNetwork",
"password": "password123"
}
```


## Get Spawn Point Information

Retrieves the currently stored spawn point coordinates (latitude and longitude).

::: details Endpoint Information


**Path:**`/spawn`

**Method:**`GET`

**Authentication:**None

  :::

### Request

No parameters required.

### Response



**Status Code:**`200 OK`

**Content Type:**`application/json`

#### Response Fields



| Field       | Type    | Description                   |
| ----------- | ------- | ----------------------------- |
| `latitude`  | `Float` | Currently set latitude value  |
| `longitude` | `Float` | Currently set longitude value |

#### Example Response

```json
{
"latitude": 37.7749,
"longitude": -122.4194
}
```

## Set Spawn Point

Configures the device's target spawn point coordinates.

::: details Endpoint Information



**Path:**`/spawn`

**Method:**`POST`

**Authentication:**None

  :::

### Request Parameters



| Parameter   | Type    | Required | Description                        |
| ----------- | ------- | -------- | ---------------------------------- |
| `latitude`  | `Float` | ✅ Yes    | Target latitude (e.g., 37.7749)    |
| `longitude` | `Float` | ✅ Yes    | Target longitude (e.g., -122.4194) |

### Example Request




POST /spawn

Content-Type: application/x-www-form-urlencoded

latitude=37.7749\&longitude=-122.4194


### Response



**Status Code:**`200 OK`

## Set Pointer Colors

Configures the colors for the spawn point needle and compass needle.

::: details Endpoint Information



**Path:**`/pointColors`

**Method:**`POST`

**Authentication:**None

  :::

### Request Parameters

| Parameter    | Type     | Required | Description                                               |
| ------------ | -------- | -------- | --------------------------------------------------------- |
| `spawnColor` | `String` | ❌ No     | Spawn point needle color (hex RGB value, e.g., `#FF5252`) |
| `southColor` | `String` | ❌ No     | Compass needle color (hex RGB value, e.g., `#FF5252`)     |


### Example Request


POST /pointColors

Content-Type: application/x-www-form-urlencoded

spawnColor=#FF5252\&southColor=#4CAF50


### Response



**Status Code:**`200 OK`

## Get Pointer Colors

Retrieves the current colors for the spawn point needle and compass needle.

::: details Endpoint Information



**Path:**`/pointColors`

**Method:**`GET`

**Authentication:**None

  :::

### Request

No parameters required.

### Response



**Status Code:**`200 OK`

**Content Type:**`application/json`

#### Response Fields



| Field        | Type     | Description                  |
| ------------ | -------- | ---------------------------- |
| `spawnColor` | `String` | Current spawn needle color   |
| `southColor` | `String` | Current compass needle color |

#### Example Response


```json
{
"spawnColor": "#FF5252",
"southColor": "#4CAF50"
}
```

## Set Azimuth

Sets the device's current azimuth angle (direction reference).

::: details Endpoint Information



**Path:**`/setAzimuth`

**Method:**`POST`

**Authentication:**None

  :::

### Request Parameters



| Parameter | Type    | Required | Description                   |
| --------- | ------- | -------- | ----------------------------- |
| `azimuth` | `Float` | ✅ Yes    | Azimuth angle value (0\~360°) |

### Example Request


POST /setAzimuth

Content-Type: application/x-www-form-urlencoded

azimuth=180.0


### Response



**Status Code:**`200 OK`

## Set WiFi [Compatibility Reserved]

Configures WiFi network credentials (legacy interface, retained for compatibility).

::: details Endpoint Information



**Path:**`/setWiFi`

**Method:**`POST`

**Note:*This is a legacy interface. Use the `/wifi` endpoint for new implementations.

  :::

### Request Parameters



| Parameter  | Type     | Required | Description           |
| ---------- | -------- | -------- | --------------------- |
| `ssid`     | `String` | ✅ Yes    | WiFi network name     |
| `password` | `String` | ✅ Yes    | WiFi network password |

### Behavior Notes

Submitting this request will automatically restart the device to apply changes.

### Example Request




POST /setWiFi

Content-Type: application/x-www-form-urlencoded

ssid=MyNetwork\&password=password123


### Response



**Status Code:**`200 OK`

## Brightness Control

### Get Current Brightness

Retrieves the current LED brightness level.

::: details Endpoint Information



**Path:**`/brightness`

**Method:**`GET`

**Authentication:**None

  :::

#### Response



**Status Code:**`200 OK`

**Content Type:**`application/json`



```json
{"brightness": 128}
```

### Set Brightness

Configures the LED brightness level.

::: details Endpoint Information



**Path:**`/brightness`

**Method:**`POST`

**Authentication:**None

  :::

#### Request Parameters



| Parameter    | Type  | Required | Description              |
| ------------ | ----- | -------- | ------------------------ |
| `brightness` | `Int` | ✅ Yes    | Brightness level (0-255) |

#### Example Request




POST /brightness

Content-Type: application/x-www-form-urlencoded

brightness=200


#### Response



**Success:*`200 OK`

**Invalid Value:*`400 Bad Request` (if value is outside 0-255 range)

## Advanced Configuration

### Get Advanced Settings

Retrieves advanced device configuration parameters.

::: details Endpoint Information



**Path:**`/advancedConfig`

**Method:**`GET`

**Authentication:**None

  :::

#### Response



**Status Code:**`200 OK`

**Content Type:**`application/json`

#### Response Fields



| Field        | Type     | Description                              |
| ------------ | -------- | ---------------------------------------- |
| `model`      | `String` | Device model: `0` (Standard) / `1` (GPS) |
| `serverMode` | `String` | Service mode: `0` (WiFi) / `1` (BLE)     |

#### Example Response



```json
{"model": "1", "serverMode": "0"}
```

### Update Advanced Settings

Modifies advanced device configuration parameters.

::: details Endpoint Information



**Path:**`/advancedConfig`

**Method:**`POST`

**Authentication:**None

  :::

#### Request Parameters



| Parameter    | Type     | Required | Description                                  |
| ------------ | -------- | -------- | -------------------------------------------- |
| `serverMode` | `String` | ❌ No     | Service mode: `"0"` (WiFi) / `"1"` (BLE)     |
| `model`      | `String` | ❌ No     | Device model: `"0"` (Standard) / `"1"` (GPS) |

#### Example Request




POST /advancedConfig

Content-Type: application/x-www-form-urlencoded

serverMode=1\&model=1


#### Response



**Status Code:**`200 OK`

## Error Handling

For undefined endpoints or invalid requests:



| Scenario                | Status Code       | Response Content  |
| ----------------------- | ----------------- | ----------------- |
| Undefined path          | `404 Not Found`   | `Not found`       |
| Invalid parameter value | `400 Bad Request` | Error description |

### General Notes



All POST requests must use `application/x-www-form-urlencoded` content type.

The `/setWiFi` endpoint triggers an automatic device restart.

Debug interfaces (e.g., `/setIndex`) are intended for development/testing only.

Firmware automatically validates all input parameters for range and format.
