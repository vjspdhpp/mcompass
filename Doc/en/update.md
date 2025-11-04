# Changelog

## December 7, 2024



* Added distance-based GPS power management strategy: GPS will be turned off when sufficiently far from target to save power

* Fixed issue where spawn point coordinates only had 2 decimal places of precision

## December 16, 2024



* Fixed incorrect reading of target location configuration

## December 29, 2024



* Added reboot interface

* Added pointer color parameter to setIndex interface

* Modified web access startup strategy:


  * Starts unconditionally but will shut down the service and hotspot if no connection is made within 2 minutes

* Added active detection of GPS module presence; adjusted initial startup behavior to point to target location or south accordingly

* Added data validity checks when processing interface data

* Added Bluetooth library source code:


  * Related functions not yet developed

## February 9, 2025



* Project refactoring

* Optimized LittleFS web file size:


  * Enabled GZIP compression

* Added pointer color interfaces:


  * Set spawn point needle color

  * Set compass needle color

* Added brightness control

* Added model differentiation:


  * Standard version: points to south on first startup

  * GPS version: points to spawn point on first startup

* Improved NIMBLE logic:


  * Added Bluetooth server mode

* Screen now supports character display

## April 1, 2025



* Modified Bluetooth device name

* Released mini-program QR code

## June 11, 2025



* Fixed issue where SSID was not "The Lost Compass"

* Fixed data source switching problem in subscription

## July 30, 2025



* Added automatic geomagnetic sensor model recognition

## September 17, 2025



* Updated platform version and board type

* Added multiple geomagnetic sensor detection attempts on startup to improve fault tolerance

* Added pointer overshoot effect to mimic game behavior:


  * When rotated quickly, the pointer will slightly overshoot the actual value, similar to mechanical pointers in the real world

> （注：文档部分内容可能由 AI 生成）