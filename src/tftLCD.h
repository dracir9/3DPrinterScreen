
#ifndef TFTLCD_H
#define TFTLCD_H

#include <Arduino.h>
#include <MCUFRIEND_kbv.h>

template <class T>
struct vector2
{
    vector2():
        x(0), y(0){}
    vector2(T x0, T y0):
        x(x0), y(y0){}

    T x,y;

    bool operator==(vector2 v1);
    bool operator!=(vector2 v1);
    vector2 operator+(vector2 v1);
    vector2 operator-(vector2 v1);
    vector2 operator*(size_t num);
};

class tftLCD : public MCUFRIEND_kbv
{
public:
    void drawCharCF(vector2<int16_t> pos, unsigned char c, uint16_t color, uint16_t bg, vector2<uint8_t> size);
    size_t writeCF(uint8_t c);

    using Adafruit_GFX::getTextBounds;
    vector2<int16_t> getTextBounds(const char *str);
    vector2<int16_t> getTextBounds(const String &str);

    void printCenter(const String &str);
    void printCenter(const char *str);
};

#endif
