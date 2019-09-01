
#ifndef HORIZONTALBOX_H
#define HORIZONTALBOX_H

#include <Arduino.h>
#include "Widgets/verticalBox.h"

/**************************************************************************
    Arrange widgets horizontally
**************************************************************************/

class horizontalBox : public verticalBox
{
public:
    horizontalBox(bool updt = true);

    vector2<int16_t> getSize(tftLCD *tft);
    void render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h);
};

#endif