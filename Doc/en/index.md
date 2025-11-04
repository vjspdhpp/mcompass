---
# https://vitepress.dev/reference/default-theme-home-page
layout: home

hero:
  name: "MCompass"
  text: "A Real-Life Minecraft Compass"
  tagline: Maybe we can find our way home before the battery runs out
  actions:
    - theme: brand
      text: Project Description
      link: ./introduction
    - theme: alt
      text: Making Guide
      link: ./make
    - theme: alt
      text: API Overview
      link: ./api
    - theme: alt
      text: Changelog
      link: ./update
    - theme: alt
      text: Support This Project
      link: ./donate


features:
  - title: Open Source
    details: Provides all files needed for replication except game textures.
    link: https://github.com/chaosgoo/mcompass.git
  - title: Based on ESP32-C3
    link: https://github.com/chaosgoo/mcompass/tree/main/Firmware
    details: To use up my inventory
  - title: Game Linkage
    details: Synchronizes with in-game data using MOD
    link: https://github.com/chaosgoo/mcompass/tree/main/Mod
---
