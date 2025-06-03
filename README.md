# A Real-World Minecraft Compass

[Youtube](https://www.youtube.com/watch?v=OetinqewrzU&t=18s)
[bilibili](https://www.bilibili.com/video/BV1cfBzYnE2k/)

- [English](README.md)
- [中文](README.zh-CN.md)

![Index](./Doc/public/MCompass.png)

Front|Bottom
-|-
![Front](./Doc/public/FrontPCB.png)|![Bottom](./Doc/public/BottomPCB.png)

## Disclaimer
* **All Minecraft game assets are copyrighted by Microsoft**, Therefore, this project does not provide in-game compass image assets.
    * The panel files only include the outline, with pixel blocks pre-drawn.
    * The original compass model extraction images are not provided.
    * The copyright of the Standard Galactic Alphabet font used on the PCB silkscreen is unclear. If you use this project for commercial purposes, ** please remove the Standard Galactic Alphabet from the PCB backside**

## Compilation Instructions

### For Non-Compilers
The repository includes GitHub Actions.You can download pre-built firmware from the latest successful "Build Firmware Workflow" run under[Actions](https://github.com/chaosgoo/mcompass/actions)

Four files are available at the bottom of the workflow page. Choose one based on your needs—they differ only in default configurations. WiFi/Bluetooth modes can be switched later via web/BLE.

File Name|Description
-|-
mcompass-639b762-LITE-BLE.bin | Standard version, BLE mode (configure via Mini Program)
mcompass-639b762-GPS-BLE.bin | GPS version, BLE mode (configure via Mini Program)
mcompass-639b762-LITE-WIFI.bin | Standard version, WiFi mode (configure via web)
mcompass-639b762-GPS-WIFI.bin | GPS version, WiFi mode (configure via web)

After downloading, extract **mcompass.bin** and flash it to an **ESP32C3** using **Flash Download Tool**(select USB mode).

The firmware merges `bootload.bin`, `partitions.bin`, `firmware.bin`, and `littlefs.bin`. Flash at address 0x0 with default settings (SPI SPEED: 40MHz; SPI MODE: DIO).

### Manual Compilation
The firmware is built on PlatformIO (Arduino framework). Dependencies are stored locally in the **lib** folder. Install PlatformIO separately.

The `Firmware/assets` folder includes `extract_pixels.py`, which processes compass{id}.bmp (10x5 images) to generate LED layout headers.

#### Web Server Compilation
The WiFi mode’s web server uses Next.js. Install Node.js, navigate to the Server folder, and run:
```bash
npm i  
npm run build  
```

Copy the generated `Server/out` contents to `Firmware/data`. Use `Firmware/assets/compass_web_data.py` to compress web resources (reduces flash usage).
Finally, use PlatformIO’s **Build Filesystem Image** and **Upload Filesystem Image** to deploy the server files.

## Features

### Bluetooth Mode
Since the Bluetooth mode relies on WeChat Mini Program for configuration, and most users may not use WeChat, this section will not be translated for now. If needed, please let me know and I'll provide the translation.
![mini_program](./Doc/public/mini_program.jpg)

### Web Server Mode
* On first boot, if no spawn point is configured, a hotspot named The Lost Compass will be created. Connect to it and visit:http://esp32.local or http://192.168.4.1 (fallback IP).
    * If WiFi credentials are saved, the device will attempt to reconnect. If failed, the hotspot reappears.
    * Quick-tap the button 4 times to display the current IP address.

### Button Actions
* 4 quick presses: Shows IP (Web Server mode).
* 6 quick presses: Enters sensor calibration (follow on-screen 8-figure motion).
* 8 quick presses: Factory reset (clears all settings).
* Long press:
    * In "Spawn Point" mode (with GPS signal): Sets current location as new spawn point.
    * In "Compass" mode: Switches to Nether mode (needle spins randomly).

`Note: GPS requires open outdoor environments. Without signal, the needle will spin erratically.`

## Materials
* PCB: 1.0mm thickness, black solder mask (JLCPCB).
* Panel: 1.0mm semi-transparent black acrylic (JLCPCB Shopping), glued post-production.
* Diffuser: PET LGT075J (glued).
* LED: WS2812B 0807
* Battery: 213455 500mAh.
* Hardware: M2×3×3.2 knurled nut, M2×4 hex screw.
* GPS: ATGM336H 5N71 module + antenna (13.1mm × 15.7mm).

## 3D Model
[MakerWorld](https://makerworld.com.cn/zh/models/667420#profileId-611642)
![MakerWorld CN](./Doc/public/makerworldcn.jpg)

## PCB & Panel Files
Temporarily using the JLCEDA Chinese version link. I will upload it to the JLCPCB English site as soon as possible.
[PCB Project & Panel Files](https://oshwhub.com/chaosgoo/wcompass)

## References
[使用GPS坐标来计算距离和方位角 by 蓝色的飘漂](https://johnnyqian.net/blog/gps-locator.html)


## Changelog
See [update.md](./Doc/update.md)