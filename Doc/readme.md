---
nextPage: /api
---
# 一个现实世界中的Minecraft罗盘
![Index](./public/MCompass.png)


## 声明
* **Minecraft游戏素材版权均归微软所有**, 所以本项目不提供游戏中罗盘的图片素材
    * 面板文件像素块已绘制好, 可直接在下单立创面板订单
    * 罗盘取模原始图片恕不提供
    * PCB背面丝印的标准银河字母字体版权不明确. 如果有任何借此项目进行商业的行为, **请删除背面标准银河字母**

## 编译说明

### 不会编译
仓库已添加Github Actions, 可以直接点击[Actions](https://github.com/chaosgoo/mcompass/actions)找到最近一次的"Build Firmware Workflow"构建成功的记录,

点击后的页面下方有四个文件, 按需下载一个即可, 除了默认配置不同外, 其余没有区别;
使用WiFi配置还是蓝牙配置, 后续仍可通过网页/蓝牙切换.
名称|描述
-|-
mcompass-639b762-LITE-BLE.bin |标准版, 蓝牙模式, 使用小程序配置
mcompass-639b762-GPS-BLE.bin | GPS版, 蓝牙模式, 使用小程序配置
mcompass-639b762-LITE-WIFI.bin | 标准版, WiFi模式, 使用网页配置
mcompass-639b762-GPS-WIFI.bin | GPS版, WiFi模式, 使用网页配置

下载后解压文件得到**mcompass.bin**文件使用**Flash Download Tool**选择ESP32C3->USB下载固件.

固件已将`bootload.bin, partitions.bin, firmware.bin, littlefs.bin`合并, 直接烧录到地址0x0即可, 其余参数保持默认, SPI SPEED:40Mhz;SPI MODE:DIO

### 手动编译
固件使用PlatformIO平台Arduino框架编写, 已将依赖库迁移至本地lib文件夹下.
PlatformIO的安装方式请自行搜索;

`Firmware/assets`文件夹内附带了一个**extract_pixels.py**脚本, 会检测`Firmware/assets`文件夹内`compass{id}.bmp`的10x5图像, 根据LED灯珠排布取模,生成对应的头文件.

#### 网页资源编译
WiFi模式自带的服务端使用next.js开发, 安装好node.js后,进入Server文件夹执行`npm i`安装所需依赖.

执行`npm run build`后构建网页服务所需文件,拷贝生成的`Server/out`文件夹内容到`Firmware/data`文件夹下, 此时可以使用`Firmware/assets/compass_web_data.py`进一步压缩网页资源,以减少flash占用,并显著提高页面打开的成功率.
最后使用PlatformIO自带的`Build Filesystem Image`和`Upload Filesystem Image`指令上传服务器文件到设备.

## 功能说明

### 蓝牙后台模式
![mini_program](./public/mini_program.jpg)

启动后1分钟内可以通过**罗盘控制台**小程序搜索并连接, 使用**罗盘控制台**时候要确保授予小程序必须权限(蓝牙, 选择地点),别忘记开启蓝牙.
在小程序中可以自定义指针的颜色,校准传感器, 地图选择目标地点

### 网页后台模式
* 首次启动检测出生点配置, 如果没有配置出生点, 则会创建一个名为**The Lost Compass**的热点, 连接该热点后打开浏览器输入[esp32.local](http://esp32.local)进入后台, 如果无法打开网页, 也可以尝试使用ESP32C3热点的默认网关地址[192.168.4.1](http://192.168.4.1),配置完成后重启装置生效;
    * 如果你在后台配置了WiFi地址, 那么下次启动会尝试连接WiFi, 如果连接失败, 会再次创建热点, 供你重新配置;
    * 不管是热点模式还是连接到WiFi, 快速点按按钮四次,会显示当前设备的IP地址;

### 按钮
* 连续快速按下按钮四次
    * WEB服务器模式下,会显示当前设备的IP地址;
* 连续快速按下按钮六次, 会进入传感器校准模式, 此时会显示数字倒计时,倒计时结束,拿起罗盘在控制画8字,并尽可能让装置在各个方向上旋转;
* 连续快速按下按钮八次, 会进入出厂设置, 会清空所有配置, 恢复出厂设置;
* 长按按钮
    * 出生针模式下在有GPS信号时, 长按按钮可以设置当前地点为新的出生点;
    * 指南针模式下, 长按按钮会切换到nether模式,指针会开始乱转;

*注意: 在室外开阔环境下才能够有GPS信号, 没有GPS信号指针会乱转.*

## 参考资料
[使用GPS坐标来计算距离和方位角 by 蓝色的飘漂](https://johnnyqian.net/blog/gps-locator.html)
