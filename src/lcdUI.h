
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
    uint8_t id = 0;
    uint8_t state = 255;

public:
    canvas base;

    // Functions
    void updateDisplay();
    bool setScreen(menu idx);
    
private:
    void drawInfo(bool init);
    void drawBlack(bool init);
};

#endif
