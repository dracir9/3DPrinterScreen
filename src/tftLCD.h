
#ifndef TFTLCD_H
#define TFTLCD_H

#include <Arduino.h>
#include <MCUFRIEND_kbv.h>

class tftLCD : public MCUFRIEND_kbv
{
public:
    using Adafruit_GFX::charBounds;
    void charBounds(char c, int16_t *x, int16_t *y);
    using Adafruit_GFX::getTextBounds;
    void getTextBounds(const char *str, int16_t *w, int16_t *h);
    void getTextBounds(const String &str, int16_t *w, int16_t *h);
};

#endif