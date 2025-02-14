# 一个现实世界中的Minecraft罗盘
![Index](./Doc/public/MCompass.png)

Front|Bottom
-|-
![Front](./Doc/public/FrontPCB.png)|![Bottom](./Doc/public/BottomPCB.png)

## 声明
* **Minecraft游戏素材版权均归微软所有**, 所以本项目不提供游戏中罗盘的图片素材
    * 面板文件仅提供外形, 像素块已绘制好
    * 罗盘取模原始图片恕不提供
    * PCB背面丝印的标准银河字母字体版权不明确. 如果有任何借此项目进行商业的行为, **请删除背面标准银河字母**

## 编译说明

### 不会编译
仓库已添加Github Actions, 可以直接点击[Actions](https://github.com/chaosgoo/mcompass/actions)找到最近一次的"Build Firmware Workflow"构建成功的记录,

点击后的页面下方有四个文件, 按需下载一个即可, 除了默认配置不同外, 其余没有区别;
均可通过网页/蓝牙切换.
名称|描述
-|-
mcompass-639b762-LITE-BLE.bin |标准版, 蓝牙服务器模式
mcompass-639b762-GPS-BLE.bin | GPS版, 蓝牙服务器模式
mcompass-639b762-LITE-WIFI.bin | 标准版, WiFi模式
mcompass-639b762-GPS-WIFI.bin | GPS版, WiFi模式

下载后解压文件得到**mcompass.bin**文件使用**Flash Download Tool**选择ESP32C3->USB下载固件.

固件已将`bootload.bin, partitions.bin, firmware.bin, littlefs.bin`合并, 直接烧录到地址0x0即可, 其余参数保持默认, SPI SPEED:40Mhz;SPI MODE:DIO

### 手动编译
固件使用PlatformIO平台Arduino框架编写, 已将依赖库迁移至本地lib文件夹下.
PlatformIO的安装方式请自行搜索;

`Firmware/assets`文件夹内附带了一个**extract_pixels.py**脚本, 会检测`Firmware/assets`文件夹内`compass{id}.bmp`的10x5图像, 根据LED灯珠排布取模,生成对应的头文件.

服务端程序使用next.js. 安装好node.js后,进入Server文件夹执行`npm i`安装依赖.

执行`npm run build`拷贝生成的`Server/out`文件夹内容到`Firmware/data`文件夹下,使用PlatformIO自带的`Build Filesystem Image`和`Upload Filesystem Image`指令上传服务器文件到设备.

## 功能说明

### 网页后台模式
* 首次启动检测出生点配置, 如果没有配置出生点, 则会创建一个名为**The Lost Compass**的热点, 连接该热点后打开浏览器输入[esp32.local](http://esp32.local)进入后台, 如果无法打开网页, 也可以尝试使用ESP32C3热点的默认网关地址[192.168.4.1](http://192.168.4.1),配置完成后重启装置生效;
    * 如果你在后台配置了WiFi地址, 那么下次启动会尝试连接WiFi, 如果连接失败, 会再次创建热点供你配置;
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

## 材料说明
* PCB板厚选择1.0mm, 黑色阻焊, 嘉立创制作;
* 面板立创商城制作, 参数选择1.0mm 半透黑色亚克力强遮光,无需背胶;
* 匀光材料选择PET LGT075J, 无需背胶;
* 电池参数 213455 500MAH
* 滚花螺母 M2*3*3.2
* 螺丝 M2*4 内六角
* GPS ATGM336H 5N71模块+天线, 尺寸13.1mmx15.7mm

## 模型
[MakerWorld](https://makerworld.com.cn/zh/models/667420#profileId-611642)
![MakerWorld CN](./Doc/public/makerworldcn.jpg)

## PCB工程及面板文件
[PCB工程文件和面板](https://oshwhub.com/chaosgoo/wcompass)

## 参考资料
[使用GPS坐标来计算距离和方位角 by 蓝色的飘漂](https://johnnyqian.net/blog/gps-locator.html)


## 更新记录
见[update.md](./Doc/update.md)