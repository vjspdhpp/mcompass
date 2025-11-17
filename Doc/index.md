---
# https://vitepress.dev/reference/default-theme-home-page
layout: home

hero:
  name: "MCompass"
  text: "一个现实世界中的Minecraft罗盘"
  tagline: 或许在没有电池耗尽之前,我们能够靠它找到回家的路
  actions:
    - theme: brand
      text: 项目描述
      link: /introduction
    - theme: brand
      text: 物料清单
      link: /bom
    - theme: alt
      text: 制作
      link: /make
    - theme: alt
      text: API一览
      link: /api
    - theme: alt
      text: 更新记录
      link: /update
    - theme: alt
      text: 支持本项目
      link: /donate


features:
  - title: 开源
    details: 提供了复刻所需的除游戏贴图以外所有文件.
    link: https://github.com/chaosgoo/mcompass.git
  - title: 基于ESP32-C3
    link: https://github.com/chaosgoo/mcompass/tree/main/Firmware
    details: 为了消耗我的库存
  - title: 游戏进行联动
    details: 使用MOD和游戏内数据进行同步
    link: https://github.com/chaosgoo/mcompass/tree/main/Mod
---
