
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
    void drawCharBg(vector2<int16_t> pos, uint8_t c, uint16_t color, uint16_t bg, vector2<uint8_t> size, vector2<int16_t> *start, vector2<int16_t> dim);
    size_t writeBg(uint8_t c, vector2<int16_t> *pos, vector2<int16_t> dim);
    void printBg(const String &str);
    void printBg(const String &str, vector2<uint8_t> pad);
    void printBg(const String &str, uint8_t pad);
    void printBg(const String &str, vector2<int16_t> pos, vector2<uint16_t> dim);

    using Adafruit_GFX::getTextBounds;
    vector2<int16_t> getTextBounds(const char *str);
    vector2<int16_t> getTextBounds(const String &str);

    void printCenter(const String &str);
    void printCenter(const char *str);
};

#endif
