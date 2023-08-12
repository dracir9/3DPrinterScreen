/**
 * @file   lcdUI.h
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 22-01-2022
 * -----
 * Last Modified: 12-08-2023
 * Modified By: Ricard Bitriá Ribes
 * -----
 * @copyright (c) 2022 Ricard Bitriá Ribes
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef LCD_UI_H
#define LCD_UI_H

#include <string>
#include <memory>
#include "driver/sdmmc_host.h"
#include "tftLCD.h"
#include "Screen.h"
#include "TchScr_Drv.h"
#include "printer.h"


/**
 * ADD NEW SCREEN
 *  1. Create source and header files under Menu folder
 *  2. Create new class named xxx_Scr that inherits Screen class publicly
 *  3. Override the update and render methods
 *  4. If necessary override the handleTouch method as well
 *  5. Include the new header in lcdUI.cpp
 *  6. Add screen name to ScreenType enum
 *  7. Add a new case in the updateObjects method in the lcdUI class
 */

class lcdUI
{
public:
    enum ScreenType : uint8_t
    {
        NONE,
        BLACK_SCR,
        INFO_SCR,
        MAIN_SCR,
        CONFIG_MENU_SCR,
        FILE_BROWSER_SRC,
        CONTROL_SCR,
        EXTRUDE_SCR,
        HEATERS_SCR,
        GCODE_PREVIEW_SCR,
        DISPLAY_CONF,
        PRINT_SCR,
        DRAW_SCR
    };

private:
    static lcdUI _instance;
    static bool init;
    
    static constexpr TchCalib calib = {
        .dx = 480,
        .rx_min = 900,
        .rx_max = 80,
        .dy = 320,
        .ry_min = 830,
        .ry_max = 100
    };

    tftLCD tft;
    std::unique_ptr<Screen> base;
    TchScr_Drv touchScreen;

    static constexpr gpio_num_t sd_cd_pin = GPIO_NUM_38;
    static constexpr gpio_num_t sd_led_pin = GPIO_NUM_41;
    static constexpr gpio_num_t sd_D0_pin = GPIO_NUM_47;
    static constexpr gpio_num_t sd_D1_pin = GPIO_NUM_48;
    static constexpr gpio_num_t sd_D2_pin = GPIO_NUM_14;
    static constexpr gpio_num_t sd_D3_pin = GPIO_NUM_21;
    static constexpr gpio_num_t sd_clk_pin = GPIO_NUM_45;
    static constexpr gpio_num_t sd_cmd_pin = GPIO_NUM_0;
    static constexpr gpio_num_t tch_tx_pin = GPIO_NUM_40;
    static constexpr gpio_num_t tch_rx_pin = GPIO_NUM_39;
    static constexpr gpio_num_t lcd_bl_pin = GPIO_NUM_46;

    bool booted = false;
    bool SDinit = false;

    static constexpr char mount_point[] = "/sdcard";
    sdmmc_card_t *card;

    xTaskHandle updateTaskH = nullptr;
    xTaskHandle touchTaskH = nullptr;
    xTaskHandle cardTaskH = nullptr;
    SemaphoreHandle_t touchMutex;

    ScreenType menuID = ScreenType::NONE;
    ScreenType newMenuID = ScreenType::BLACK_SCR;
    int64_t lastRender = 0;
    int64_t updateTime = 0;

    std::string selectedFile;
    
    lcdUI();
    ~lcdUI();

    static void updateTask(void* arg);
    static void touchTask(void* arg);
    static void cardDetectTask(void* arg);
    static void cardISRhandle(void* arg);

    bool initSD();
    void processTouch();
    esp_err_t updateDisplay();
    esp_err_t updateObjects();
public:

    static lcdUI* instance();
    esp_err_t begin();
    void setScreen(const ScreenType screen);
    void endSD();
    bool isSDinit() const;
    esp_err_t setFile(const std::string& file);
    const std::string& getFile() const { return selectedFile; }
    void clearFile() { selectedFile.clear(); }
    void requestUpdate();
    esp_err_t setBrightness(uint32_t duty);
    uint32_t getBrightness();
};

#endif
