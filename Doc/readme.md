---
nextPage: /api
---
# 一个现实世界中的Minecraft罗盘
![Index](./public/MCompass.png)


## 声明
* **Minecraft游戏素材版权均归微软所有**, 所以本项目不提供游戏中罗盘的图片素材
    * 面板文件仅提供外形, 像素块已绘制好
    * 罗盘取模原始图片恕不提供
    * PCB背面丝印的标准银河字母字体版权不明确. 如果有任何借此项目进行商业的行为, **请删除背面标准银河字母**

## 编译说明

### 不会编译
仓库已添加Github Actions, 可以直接点击[Actions](https://github.com/chaosgoo/mcompass/actions)找到最近一次的"Build Firmware Workflow"构建成功的记录,

点击后的页面下方有个类似于"mcompass-639b762"的文件, 下载后解压文件得到**mcompass.bin**文件使用**Flash Download Tool**选择ESP32C3->USB下载固件.

固件已将`bootload.bin, partitions.bin, firmware.bin, littlefs.bin`合并, 直接烧录到地址0x0即可, 其余参数保持默认, SPI SPEED:40Mhz;SPI MODE:DIO

### 手动编译
固件使用PlatformIO平台Arduino框架编写, 已将依赖库迁移至本地lib文件夹下.
PlatformIO的安装方式请自行搜索;

`Firmware/assets`文件夹内附带了一个**extract_pixels.py**脚本, 会检测`Firmware/assets`文件夹内`compass{id}.bmp`的10x5图像, 根据LED灯珠排布取模,生成对应的头文件.

服务端程序使用next.js. 安装好node.js后,进入Server文件夹执行`npm i`安装依赖.

执行`npm run build`拷贝生成的`Server/out`文件夹内容到`Firmware/data`文件夹下,使用PlatformIO自带的`Build Filesystem Image`和`Upload Filesystem Image`指令上传到设备.

## 功能说明
* 首次插上电脑启动会创建一个`The Lost Compass`的热点, 连接后打开浏览器输入[esp32.local](http://esp32.local), 输入WiFi和密码进行配网,配网完成后装置会重启;
* 当罗盘开机后会启动后台服务, 使用[esp32.local](http://esp32.local)访问后台;
    * 如果启动后3秒内没有连接到配置的WiFi, 则会主动创建一个`Your Compass`的热点, 连接该热点后使用网关IP()一般为[192.168.4.1](http://192.168.4.1)访问后台
    * 一分钟内没有人访问启动的后台服务则会关闭热点.和网页服务
* 开机后显示白色指针时按住按钮会进入校准模式(校准时候会观察到指针持续10秒卡住), 请举着罗盘飞来飞去, 尽可能多的让罗盘在各个方向旋转.
* 在室外开阔环境下才能够有GPS信号, 没有GPS信号指针会乱转的.
* 在有GPS信号的情况下, 长按按钮可以设置当前地点为新的出生点.
* 只支持2.4G的WiFi
* 连接WiFi时候长按按钮可以清楚已有的WiFi信息, 回到第一步重新配网.
* 指南针模式下长按会切换模式, 该模式下指针抖动, 就像是游戏里处于下界一样, 再次长按会切换会指南针模式
    * 这时单击按钮切换到GPS模式, 并且GPS没有信号的话,会导致不好区分当前模式

## 指针颜色及其含义
颜色|含义
-|-
白色指针|启动中, 这时候按按钮可以进入校准模式
红色指针|罗盘模式, 暂时无法区分是指向地点还是南方
黄色指针|热点模式
绿色指针|正在连接WiFi


## 参考资料
[使用GPS坐标来计算距离和方位角 by 蓝色的飘漂](https://johnnyqian.net/blog/gps-locator.html)
