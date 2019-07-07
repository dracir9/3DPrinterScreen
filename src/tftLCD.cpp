
#include "tftLCD.h"
#include "glcdfont.c"
#include <pgmspace.h>

inline GFXglyph * pgm_read_glyph_ptr(const GFXfont *gfxFont, uint8_t c)
{
    #ifdef __AVR__
        return &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
    #else
        // expression in __AVR__ section may generate "dereferencing type-punned pointer will break strict-aliasing rules" warning
        // In fact, on other platforms (such as STM32) there is no need to do this pointer magic as program memory may be read in a usual way
        // So expression may be simplified
        return gfxFont->glyph + c;
    #endif //__AVR__
}

inline uint8_t * pgm_read_bitmap_ptr(const GFXfont *gfxFont)
{
    #ifdef __AVR__
        return (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);
    #else
        // expression in __AVR__ section generates "dereferencing type-punned pointer will break strict-aliasing rules" warning
        // In fact, on other platforms (such as STM32) there is no need to do this pointer magic as program memory may be read in a usual way
        // So expression may be simplified
        return gfxFont->bitmap;
    #endif //__AVR__
}

/*****************************************************
    Vector temp
*****************************************************/
template <class T>
bool vector2<T>::operator==(vector2 v1)
{
    return ((v1.x == x) && (v1.y == y));
}

template <class T>
bool vector2<T>::operator!=(vector2 v1)
{
    return ((v1.x != x) || (v1.y != y));
}

template <class T>
vector2<T> vector2<T>::operator+(vector2 v1)
{
    return vector2(v1.x + x, v1.y + y);
}

template <class T>
vector2<T> vector2<T>::operator-(vector2 v1)
{
    return vector2(v1.x - x, v1.y - y);
}

template <class T>
vector2<T> vector2<T>::operator*(size_t num)
{
    return vector2(x * num, y * num);
}

/*****************************************************
    tft LCD class
*****************************************************/
void tftLCD::drawCharCF(vector2<int16_t> pos, unsigned char c, uint16_t color, uint16_t bg, vector2<uint8_t> size)
{
    if(!gfxFont) // 'Classic' built-in font
    {
        if((pos.x >= _width)            || // Clip right
           (pos.y >= _height)           || // Clip bottom
           ((pos.x + 6 * size.x - 1) < 0) || // Clip left
           ((pos.y + 8 * size.y - 1) < 0))   // Clip top
            return;

        if(!_cp437 && (c >= 176)) c++; // Handle 'classic' charset behavior

        startWrite();
        for(int8_t i=0; i<5; i++ )
        { // Char bitmap = 5 columns
            uint8_t line = pgm_read_byte(&font[c * 5 + i]);
            for(int8_t j=0; j<8; j++, line >>= 1)
            {
                if(line & 1)
                {
                    if(size.x == 1 && size.y == 1)
                        writePixel(pos.x+i, pos.y+j, color);
                    else
                        writeFillRect(pos.x+i*size.x, pos.y+j*size.y, size.x, size.y, color);
                }
                else if(bg != color)
                {
                    if(size.x == 1 && size.y == 1)
                        writePixel(pos.x+i, pos.y+j, bg);
                    else
                        writeFillRect(pos.x+i*size.x, pos.y+j*size.y, size.x, size.y, bg);
                }
            }
        }
        if(bg != color)
        { // If opaque, draw vertical line for last column
            if(size.x == 1 && size.y == 1) writeFastVLine(pos.x+5, pos.y, 8, bg);
            else          writeFillRect(pos.x+5*size.x, pos.y, size.x, 8*size.y, bg);
        }
        endWrite();
    }
    else
    { // Custom font
        c -= (uint8_t)pgm_read_byte(&gfxFont->first);
        GFXglyph *glyph  = pgm_read_glyph_ptr(gfxFont, c);
        uint8_t  *bitmap = pgm_read_bitmap_ptr(gfxFont);

        uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
        uint8_t  w  = pgm_read_byte(&glyph->width),
                 h  = pgm_read_byte(&glyph->height);
        int8_t   xo = pgm_read_byte(&glyph->xOffset),
                 yo = pgm_read_byte(&glyph->yOffset);
        uint8_t  xx, yy, bits = 0, bit = 0;
        int16_t  xo16 = 0, yo16 = 0;

        if(size.x > 1 || size.y > 1)
        {
            xo16 = xo;
            yo16 = yo;
        }
        startWrite();
        for(yy=0; yy<h; yy++)
        {
            for(xx=0; xx<w; xx++)
            {
                if(!(bit++ & 7))
                {
                    bits = pgm_read_byte(&bitmap[bo++]);
                }
                if(bits & 0x80)
                {
                    if(size.x == 1 && size.y == 1)
                    {
                        writePixel(pos.x+xo+xx, pos.y+yo+yy, color);
                    }
                    else
                    {
                        writeFillRect(pos.x+(xo16+xx)*size.x, pos.y+(yo16+yy)*size.y,
                          size.x, size.y, color);
                    }
                }
                bits <<= 1;
            }
        }
        endWrite();
    } // End classic vs custom font
}

size_t tftLCD::writeCF(uint8_t c)
{
    if(!gfxFont) 
    { // 'Classic' built-in font
        return 0;
    }
    else // Custom font
    {
        if(c == '\n')
        {
            cursor_x  = 0;
            cursor_y += (int16_t)textsize_y *
                        (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
        } else if(c != '\r')
        {
            uint8_t first = pgm_read_byte(&gfxFont->first);
            if((c >= first) && (c <= (uint8_t)pgm_read_byte(&gfxFont->last)))
            {
                GFXglyph *glyph  = pgm_read_glyph_ptr(gfxFont, c - first);
                uint8_t   w     = pgm_read_byte(&glyph->width),
                          h     = pgm_read_byte(&glyph->height);
                if((w > 0) && (h > 0))
                { // Is there an associated bitmap?
                    int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset); // sic
                    if(wrap && ((cursor_x + textsize_x * (xo + w)) > _width))
                    {
                        cursor_x  = 0;
                        cursor_y += (int16_t)textsize_y *
                          (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
                    }
                    drawCharCF(vector2<int16_t>(cursor_x, cursor_y), c, textcolor, textbgcolor, vector2<uint8_t>(textsize_x, textsize_y));
                }
                cursor_x += (uint8_t)pgm_read_byte(&glyph->xAdvance) * (int16_t)textsize_x;
            }
        }
    }
    return 1;
}

vector2<int16_t> tftLCD::getTextBounds(const char *str)
{
    uint8_t c; // Current character
    int16_t x=0, y=0, minx = _width, miny = _height, maxx = -1, maxy = -1;
    vector2<int16_t> size;
    while ((c = *str++))
    {
        charBounds(c, &x, &y, &minx, &miny, &maxx, &maxy);
    }
    if (maxx >= minx)
    {
        size.x  = maxx - minx + 1;
    }
    if (maxy >= miny)
    {
        size.y  = maxy - miny + 1;
    }
    return size;
}

vector2<int16_t> tftLCD::getTextBounds(const String &str)
{
    if(str.length() != 0)
    {
        return getTextBounds(const_cast<char*>(str.c_str()));
    }
    return vector2<int16_t>();
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
        if (gfxFont && (textcolor != textbgcolor))
        {
            fillRect(getCursorX() + x, getCursorY()+y, w, h, textbgcolor);
        }
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
            if (gfxFont && (textcolor != textbgcolor))
            {
                fillRect(center-w/2, getCursorY()+y, w, h, textbgcolor);
            }
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
