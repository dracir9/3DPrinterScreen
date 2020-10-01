
#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <Arduino.h>
#include "widgets.h"
#include "Ulitity/utility.h"

/**************************************************************************
    Draw text
**************************************************************************/
class textBox : public widget
{
protected:
    String *text;
    fillMode arrange = fillMode::CenterCenter;
    int16_t paddingX = 8;
    int16_t paddingY = 8;
    uint8_t size = 1;
    uint16_t txtcolor = TFT_WHITE;
    uint16_t bgcolor = TFT_MAROON;
    const GFXfont *font = NULL;

public:
    textBox(String *txt, fillMode arr = fillMode::CenterCenter, uint16_t color = TFT_WHITE,
                                            const GFXfont *font = NULL, uint8_t textSize = 2, bool updt = true):
    widget(updt), text(txt), arrange(arr), size(textSize), txtcolor(color), font(font) {}

    vector2<int16_t> getSize(tftLCD *tft) const;
    void draw(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h) const;
};

#endif