
#ifndef LCD_UI_H
#define LCD_UI_H

#include <Arduino.h>
#include <SD_MMC.h>
#include "tftLCD.h"
#include "TchScr_Drv.h"
#include "widgets.h"
#include "Menu/info_w.h"
#include "Menu/black_w.h"
#include "Menu/fileBrowser_Scr.h"
#include "Menu/gcodePreview_Scr.h"

//**************************************************************************
//*  lcdUI class
//*  Screen and user input managing
//**************************************************************************
class lcdUI
{
public:
    lcdUI();
    ~lcdUI();

    enum menu : uint8_t
    {
        black=0,
        Info,
        main,
        settings,
        FileBrowser,
        control,
        GcodePreview
    };

    // Functions
/**
 * Initialize render and touch screen tasks
 * 
 * @param       fps Maximum frames per second
 * @return      True if successfully initiated
 */
    bool begin(const uint8_t fps = 30);
    bool updateDisplay();
    void processTouch();
    void setScreen(const menu idx);
    uint32_t getUpdateTime() const;
    bool initSD();
    void endSD();
    bool checkSD() const;
    uint8_t getFrameTime() const { return frameTime; }
    bool setFile(const std::string& file);
    const std::string& getFile() const { return selectedFile; }
    void clearFile() { selectedFile.clear(); }

    friend void loop();

private:
    static void renderUITask(void* arg);
    static void handleTouchTask(void* arg);
    static void cardDetectTask(void* arg);
    static void touchISRhandle(void* arg);
    static void cardISRhandle(void* arg);

    tftLCD tft;
    Screen* base = nullptr;
    TchScr_Drv touchScreen;
    TchCalib calib = {
        .dx = 480,
        .rx_min = 80,
        .rx_max = 820,
        .dy = 320,
        .ry_min = 100,
        .ry_max = 730
    };

    bool booted = false;
    bool hasSD = false;
    bool prevPressed;

    uint8_t frameTime = 33;         // Minimum frame time
    xTaskHandle renderTask = nullptr;
    xTaskHandle touchTask = nullptr;
    xTaskHandle cardTask = nullptr;
    SemaphoreHandle_t SPIMutex;
    SemaphoreHandle_t touchFlag;
    SemaphoreHandle_t cardFlag;

    menu menuID = menu::black;
    menu newMenuID;
    int64_t lastRender = 0;
    unsigned long updateTime = 0;
    Vec2h Tpos;
    std::string selectedFile;

    bool updateObjects();
};

#endif
