# 制作

## 源码
[Github](https://github.com/chaosgoo/mcompass)

## 模型
[MakerWorld CN](https://makerworld.com.cn/zh/models/667420#profileId-611642)
![MakerWorld CN](./public/makerworldcn.jpg)

## PCB

[PCB工程文件和面板](https://oshwhub.com/chaosgoo/wcompass)

![Front](./public/FrontPCB.png)


![Bottom](./public/BottomPCB.png)


## 材料说明
* PCB板厚选择1.0mm, 黑色阻焊, 嘉立创制作;
* 面板立创商城制作, 参数选择1.0mm 半透黑色亚克力强遮光,无需背胶;
* 匀光材料选择PET LGT075J, 无需背胶;
* 电池参数 213455 500MAH
* 滚花螺母 M2*3*3.2
* 螺丝 M2*4 内六角
* GPS ATGM336H 5N71模块+天线, 尺寸13.1mmx15.7mm


## 指针颜色及其含义
颜色|含义
-|-
白色指针|启动中, 这时候按按钮可以进入校准模式
红色指针|罗盘模式, 暂时无法区分是执行地点还是南方
黄色指针|热点模式
绿色指针|正在连接WiFi

## Q&A
### 2024年12月2日

**Q**: GPS定位需要多久

**A**: 根据淘宝介绍页面和实测, 首次定位需要30秒+

**Q**: 根据[项目原理图](https://oshwhub.com/chaosgoo/wcompass)的灯珠手册描述, 工作电压应为3.5V~5.5V, 而原理图中实际使用LDO 3.3V输出作为电源

**A**: 设计失误, 从实际工作情况来看, 还没有表现出什么严重问题. 如果后面有第二版计划,会修复这里的设计.
