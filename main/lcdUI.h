/**
 * @file   lcdUI.h
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 22-01-2022
 * -----
 * Last Modified: 11-04-2022
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

#include "tftLCD.h"
#include "Screen.h"
#include "TchScr_Drv.h"

class lcdUI
{
public:
    enum menu : uint8_t
    {
        none,
        black,
        Info,
        main,
        Config,
        FileBrowser,
        control,
        GcodePreview
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
    Screen* base = nullptr;
    TchScr_Drv touchScreen;

    bool booted = false;
    bool SDinit = false;

    xTaskHandle updateTaskH = nullptr;
    xTaskHandle touchTaskH = nullptr;
    xTaskHandle cardTaskH = nullptr;
    SemaphoreHandle_t touchMutex;

    menu menuID = menu::none;
    menu newMenuID = menu::black;
    int64_t lastRender = 0;
    int64_t updateTime = 0;

    std::string selectedFile;
    
    lcdUI(/* args */);
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
    void setScreen(const menu screen);
    void endSD();
    bool isSDinit() const;
    esp_err_t setFile(const std::string& file);
    const std::string& getFile() const { return selectedFile; }
    void clearFile() { selectedFile.clear(); }
    void requestUpdate();
};

#endif
