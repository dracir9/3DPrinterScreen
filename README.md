# 3D Printer Screen
3D Printer Screen is a 3d printer controller project based arround ESP32. The purpose of this project is to add wireless connectivity to the printer and simplify its operation through LCD and touchscreen interfaces.

**_Early development_**

## Hardware Prerequisites
- [ESP Wrover Kit v4.1](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-wrover-kit.html)
- TFT compatible with [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) library and SPI communication
- XPT2046 touch controller
- Pinout:
  * TFT_DC          -> 0
  * TFT_CS          -> 5
  * TFT_RST         -> 22
  * TFT_MISO        -> 19
  * TFT_MOSI        -> 23
  * TFT_SCLK        -> 18
  * TFT_BackLight   -> 25
  * Touch_IRQ       -> 27
  * Touch_CS        -> 26
  * SD Card_CD      -> 21

## Current features
 - [x] SD card browser
 - [x] Gcode preview
 
## Screenshots
#### Main screen
![info](/images/screenshots/screenshot_2021_1_18_3_1_7.png)
#### SD card browser
![browser](/images/screenshots/screenshot_2021_1_15_23_4_5.png)
![browser](/images/screenshots/screenshot_2021_1_15_23_4_56.png)
![browser](/images/screenshots/screenshot_2021_1_15_23_4_27.png)
![browser](/images/screenshots/screenshot_2021_1_15_23_4_46.png)
#### Gcode preview
![preview](/images/screenshots/screenshot_2021_1_18_18_55_15.png) Based on [this design](https://www.thingiverse.com/thing:2508515)
![preview](/images/screenshots/screenshot_2021_1_15_23_5_20.png) Based on [this design](https://www.thingiverse.com/thing:2914080)
![preview](/images/screenshots/screenshot_2021_1_15_23_6_17.png) Based on [this design](https://www.thingiverse.com/thing:826836)
![preview](/images/screenshots/screenshot_2021_1_15_23_7_13.png) Based on [this design](https://www.thingiverse.com/thing:1657791)
 
## Planned features
 - [ ] Movement, temperature, settings, file selection, etc. menus
 - [ ] SD card printing
 - [ ] WiFi printing
