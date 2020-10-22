
#ifndef LCD_UI_H
#define LCD_UI_H

#include <Arduino.h>
#include "tftLCD.h"
#include "SD.h"
#include "widgets.h"
#include "Menu/info_w.h"
#include "Menu/black_w.h"
#include "Menu/fileBrowser_Scr.h"
#include "Menu/gcodePreview_Scr.h"

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
    bool updateDisplay(uint8_t fps);
    bool setScreen(menu idx);
    uint32_t getUpdateTime() const;
    bool initSD();
    bool checkSD() const;
    
    tftLCD tft;

    char selectedFile[96];

private:
    Screen* base = nullptr;

    menu menuid = menu::black;
    int64_t nextRender = 0;
    int64_t lastRender = 0;
    int64_t nextCheck = 0;
    unsigned long updateTime = 0;

    bool hasSD = false;

    Screen* updateObjects(menu id);
};

#endif
