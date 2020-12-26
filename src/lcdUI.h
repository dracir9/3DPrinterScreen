
#ifndef LCD_UI_H
#define LCD_UI_H

#include <Arduino.h>
#include <TouchScreen.h>
#include "tftLCD.h"
#include "SD.h"
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
        info,
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
 * @param       upsD Maximum display updates per second
 * @param       upsT Maximum touch reads per second
 * @return      True if successfully initiated
 */
    bool begin(uint8_t upsD = 30);
    bool updateDisplay();
    bool processTouch();
    void setScreen(menu idx);
    uint32_t getUpdateTime() const;
    bool initSD();
    bool checkSD() const;

    friend void renderUITask(void* arg);
    friend void handleTouchTask(void* arg);
    friend void loop();
    
    tftLCD tft;

    std::string selectedFile;

private:
    Screen* base = nullptr;
    TouchScreen ts = TouchScreen(TOUCH_PIN_XP, TOUCH_PIN_YP, TOUCH_PIN_XM, TOUCH_PIN_YM, 300);

    bool booted = false;
    bool hasSD = false;
    bool prevPressed;

    uint8_t fps = 30;
    xTaskHandle renderTask;
    xTaskHandle touchTask;
    SemaphoreHandle_t SPIMutex;

    menu menuID = menu::black;
    menu newMenuID;
    int64_t lastRender = 0;
    int64_t nextCheck = 0;
    unsigned long updateTime = 0;
    Vector2<int16_t> Tpos;

    bool updateObjects();
};

#endif
