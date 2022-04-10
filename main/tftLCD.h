
#ifndef TFTLCD_H
#define TFTLCD_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "Free_Fonts.h"
#include "Vector.h"

class tftSprite : public TFT_eSprite
{
public:
    tftSprite(TFT_eSPI* tft): TFT_eSprite(tft){}    

    void printCenter(const String &str);
    void printCenter(const char *str);
};

class tftLCD : public TFT_eSPI
{
public:
    void drawCharBg(Vec2h pos, uint8_t c, uint16_t color, uint16_t bg, uint8_t size, Vec2h *start, Vec2h dim);
    size_t writeBg(uint8_t c, Vec2h *pos, Vec2h dim);
    void printBg(const String &str);
    void printBg(const String &str, Vector2<uint8_t> pad);
    void printBg(const String &str, uint8_t pad);
    void printBg(const String &str, Vec2h pos, Vector2<uint16_t> dim);
    
    void charBounds(char c, int16_t *x, int16_t *y, int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy);
    Vec2h getTextBounds(const char *str);
    Vec2h getTextBounds(const String &str);
    Vec2h getTextBounds(const char *str, int16_t *x, int16_t *y);
    Vec2h getTextBounds(const String &str, int16_t *x, int16_t *y);

    void printCenter(const String &str);
    void printCenter(const char *str);

    void drawStringWr(const char *str, int32_t x, int32_t y, uint16_t w, uint16_t h);

    void drawBmpSPIFFS(const char *filename, int16_t x, int16_t y);

    tftSprite img = tftSprite(this);
};

#endif
