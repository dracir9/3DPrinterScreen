
#ifndef LCD_UI_H
#define LCD_UI_H

#include <Arduino.h>
#include "tftLCD.h"
#include "widgets.h"
#include "Fonts/FreeMono12pt7b.h"

class lcdUI : public tftLCD
{

    // Variables
public:
    enum menu : uint8_t
    {
        black=0,
        info,
        main,
        settings,
        SDmenu,
        control
    };
private:
    menu menuid = menu::black;
    uint8_t state = 255;
    bool rendered = false;

public:
    canvas base;

    // Functions
    bool updateDisplay(uint8_t fps);
    bool setScreen(menu idx);
    
private:
    bool updateObjects(menu id, bool init);
    void drawInfo(bool init);
    void drawBlack(bool init);
};

#endif
