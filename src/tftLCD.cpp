
#include "tftLCD.h"
#include <Adafruit_GFX.cpp>

void tftLCD::charBounds(char c, int16_t *x, int16_t *y)
{
    if(gfxFont)
    {
        if(c == '\n') 
        { // Newline?
            *x = 0;    // Reset x to zero, advance y by one line
            *y += textsize * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
        } else if(c != '\r')
        { // Not a carriage return; is normal char
            uint8_t first = pgm_read_byte(&gfxFont->first),
                    last  = pgm_read_byte(&gfxFont->last);
            if((c >= first) && (c <= last)) // Char present in this font?
            {
                GFXglyph *glyph = &(((GFXglyph *)pgm_read_pointer(
                &gfxFont->glyph))[c - first]);
                uint8_t gw = pgm_read_byte(&glyph->width),
                        xa = pgm_read_byte(&glyph->xAdvance);
                int8_t  xo = pgm_read_byte(&glyph->xOffset);
                if(wrap && ((*x+(((int16_t)xo+gw)*textsize)) > _width))
                {
                    *x  = 0; // Reset x to zero, advance y by one line
                    *y += textsize * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
                }
                *x += xa * textsize;
            }
        }
    }
    else
    {
        if(c == '\n') // Newline?
        {
            *x = 0;                        // Reset x to zero,
            *y += textsize * 8;             // advance y one line
        } else if(c != '\r') // Normal char; ignore carriage returns
        {
            if(wrap && ((*x + textsize * 6) > _width)) // Off right?
            {
                *x  = 0;                    // Reset x to zero,
                *y += textsize * 8;         // advance y one line
            }
            *x += textsize * 6;
        }
    }
}

void tftLCD::getTextBounds(const char *str, int16_t *w, int16_t *h)
{
    uint8_t c; // Current character
    w=h=0;
    while ((c = *str++))
    {
        charBounds(c, w, h);
    }
    *w -= textsize;
    //*h -= textsize;
}

void tftLCD::getTextBounds(const String &str, int16_t *w, int16_t *h)
{
    if(str.length() != 0)
    {
        getTextBounds(const_cast<char*>(str.c_str()), w, h);
    }
}
