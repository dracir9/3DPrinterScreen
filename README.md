# 3D Printer Screen
3D Printer Screen is a 3d printer controller project based arround ESP32. The purpose of this project is to add wireless connectivity to the printer and simplify its operation through LCD and touchscreen interfaces.

**_Early development_**

## Hardware Prerequisites
- ![This custom board](hardware/3D_Printer_Screen_V2.pdf) or another with similar wiring similar
- TFT LCD display compatible with [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) library and SPI communication
- ESP32 Wrover-e Rev 3 with at least 4MB of SPIRAM
- EFM8BB10F8G microcontroller

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
