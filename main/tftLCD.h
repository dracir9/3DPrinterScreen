
#ifndef TFTLCD_H
#define TFTLCD_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "Free_Fonts.h"
#include "Vector.h"

class tftLCD : public TFT_eSPI
{
public:
    void drawStringWr(const char *str, int32_t x, int32_t y, uint16_t w, uint16_t h);

    void drawBmpSPIFFS(const char *filename, int16_t x, int16_t y);
};

class tftSprite : public tftLCD, public TFT_eSprite
{
public:
    tftSprite(TFT_eSPI* tft): TFT_eSprite(tft){}    
};

#endif
