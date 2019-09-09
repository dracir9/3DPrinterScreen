
#ifndef LCD_UI_H
#define LCD_UI_H

#include <Arduino.h>
#include "tftLCD.h"
#include "widgets.h"
#include "Menu/info_w.h"
#include "Menu/black_w.h"

class lcdUI : public tftLCD
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
        SDmenu,
        control
    };

    canvas* base = NULL;

    // Functions
    bool updateDisplay(uint8_t fps);
    bool setScreen(menu idx);
    uint32_t getUpdateTime();
    
private:
    uint8_t state = 255;
    menu menuid = menu::black;
    bool rendered = false;
    unsigned long updateTime = 0;

    canvas* updateObjects(menu id);
    /* void drawInfo(bool init);
    void drawBlack(bool init); */
};

#endif
