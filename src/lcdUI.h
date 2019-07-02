
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
        info=1,
        main,
        settings,
        SDmenu,
        control
    };
private:
    uint8_t state=0;

public:
    canvas base;

    // Functions
    void updateDisplay(uint8_t id);
    
private:
    void drawInfo(bool init);
};

#endif
