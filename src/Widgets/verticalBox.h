
#ifndef BOX_H
#define BOX_H

#include <Arduino.h>
#include "widgets.h"

/**************************************************************************
    Arrange widgets vertically
**************************************************************************/

class verticalBox : public widget
{
protected:
    uint8_t elNum = 1;
    widget **child = NULL;

public:
    verticalBox(uint8_t elem, bool updt = true);
    ~verticalBox();

    virtual vector2<int16_t> getSize(tftLCD *tft);
    virtual void render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h);
    void nullify();
    bool attachComponent(widget *chld);
};

#endif