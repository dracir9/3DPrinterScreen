
#ifndef LCD_UI_H
#define LCD_UI_H

#include <Arduino.h>
#include <SD_MMC.h>
#include "tftLCD.h"
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
    bool processTouch();
    void setScreen(const menu idx);
    uint32_t getUpdateTime() const;
    bool initSD();
    bool checkSD() const;
    uint8_t getFrameTime() const { return frameTime; }
    bool setFile(const std::string& file);
    const std::string& getFile() const { return selectedFile; }
    void clearFile() { selectedFile.clear(); }

    friend void renderUITask(void* arg);
    friend void handleTouchTask(void* arg);
    friend void loop();

private:
    tftLCD tft;
    Screen* base = nullptr;

    bool booted = false;
    bool hasSD = false;
    bool prevPressed;

    uint8_t frameTime = 33;         // Minimum frame time
    xTaskHandle renderTask = nullptr;
    xTaskHandle touchTask = nullptr;
    SemaphoreHandle_t SPIMutex;

    menu menuID = menu::black;
    menu newMenuID;
    int64_t lastRender = 0;
    int64_t nextCheck = 0;
    unsigned long updateTime = 0;
    Vec2h Tpos;
    std::string selectedFile;

    bool updateObjects();
};

#endif
