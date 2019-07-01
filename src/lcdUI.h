
#ifndef LCD_UI_H
#define LCD_UI_H

#include <Arduino.h>
#include "tftLCD.h"
#include "widgets.h"

class grid
{
public:
    const int16_t gridX;//=118;
    const int16_t gridY;//=40;
    const uint8_t col;//=3;
    const uint8_t row;//=3;
    const uint8_t space;//=8;
    const int16_t cellW;//=(width()-gridX-(e-1)*8)/e;
    const int16_t cellH;//=25;

    grid(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t c, uint8_t r, uint8_t s):
    gridX(x), gridY(y), col(c), row(r), space(s),
    cellW((w-(c-1)*s)/c),
    cellH((h-(r-1)*s)/r)
    {}

    void getTLPoint(int16_t *x, int16_t *y, uint8_t c, uint8_t r) const;
    void getCenterPoint(int16_t *x, int16_t *y, uint8_t c, uint8_t r) const;
};

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
    template <class text>
    void printCenter(text s);

    void drawInfo(bool init);
};

#endif
