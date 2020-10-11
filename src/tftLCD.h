
#ifndef TFTLCD_H
#define TFTLCD_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Free_Fonts.h>
#include "Vector.h"
#include "Configuration.h"

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
    void drawCharBg(Vector2<int16_t> pos, uint8_t c, uint16_t color, uint16_t bg, uint8_t size, Vector2<int16_t> *start, Vector2<int16_t> dim);
    size_t writeBg(uint8_t c, Vector2<int16_t> *pos, Vector2<int16_t> dim);
    void printBg(const String &str);
    void printBg(const String &str, Vector2<uint8_t> pad);
    void printBg(const String &str, uint8_t pad);
    void printBg(const String &str, Vector2<int16_t> pos, Vector2<uint16_t> dim);
    
    void charBounds(char c, int16_t *x, int16_t *y, int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy);
    Vector2<int16_t> getTextBounds(const char *str);
    Vector2<int16_t> getTextBounds(const String &str);
    Vector2<int16_t> getTextBounds(const char *str, int16_t *x, int16_t *y);
    Vector2<int16_t> getTextBounds(const String &str, int16_t *x, int16_t *y);

    void printCenter(const String &str);
    void printCenter(const char *str);

    tftSprite img = tftSprite(this);
};

#endif
