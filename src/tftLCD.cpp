
#include "tftLCD.h"
#include <Adafruit_GFX.cpp>

void tftLCD::getTextBounds(const char *str, int16_t *w, int16_t *h)
{
    uint8_t c; // Current character
    int16_t x=0, y=0, minx = _width, miny = _height, maxx = -1, maxy = -1;
    while ((c = *str++))
    {
        charBounds(c, &x, &y, &minx, &miny, &maxx, &maxy);
    }
    if (maxx >= minx)
    {
        *w  = maxx - minx + 1;
    }
    if (maxy >= miny)
    {
        *h  = maxy - miny + 1;
    }
}

void tftLCD::getTextBounds(const String &str, int16_t *w, int16_t *h)
{
    if(str.length() != 0)
    {
        getTextBounds(const_cast<char*>(str.c_str()), w, h);
    }
}

/**************************************************************************/
/*!
    @brief  Print a string centered at cursor location
    @param  String to print
*/
/**************************************************************************/
void tftLCD::printCenter(const String &str)
{
    int16_t x,y;
    uint16_t w, h;
    int16_t center = getCursorX();
    getTextBounds(str, 0, 0, &x, &y, &w, &h);
    setCursor(getCursorX()-w/2-x, getCursorY()-h/2-y);
    if (str.indexOf('\n') == -1)
    {
        print(str);
    } else
    {
        String buf;
        int a = 0;
        int b = str.indexOf('\n');
        while (a < str.length())
        {
            buf = str.substring(a, b)+'\n';
            getTextBounds(buf, 0, 0, &x, &y, &w, &h);
            setCursor(center-w/2-x, getCursorY());
            print(buf);
            a = b+1;
            b = str.indexOf('\n', a);
            if (b == -1) b = str.length();
        }
    }
}

void tftLCD::printCenter(const char *str)
{
    printCenter(String(str));
}
