
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
void tftLCD::drawCharBg(vector2<int16_t> pos, uint8_t c, uint16_t color, uint16_t bg, vector2<uint8_t> size, vector2<int16_t> *start, vector2<int16_t> dim)
{
    int16_t minx = 0, miny = 0, maxx = 5, maxy = 8;
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
        int16_t  w  = pgm_read_byte(&glyph->width),
                 h  = pgm_read_byte(&glyph->height);
        int16_t  xo = (int8_t)pgm_read_byte(&glyph->xOffset),
                 yo = (int8_t)pgm_read_byte(&glyph->yOffset);
        uint8_t  bits = 0,
                 oldbits = pgm_read_byte(&bitmap[bo]),
                 bit = 0,
                 oldbit = 0;
        uint16_t oldbo = bo;
        w *= size.x;
        h *= size.y;
        xo *= size.x;
        yo *= size.y;
        maxx =  w, maxy = h;

        if (dim.x != 0) 
        {
            if ((*start).x < 0) (*start).x = 0;
            minx = (*start).x - pos.x - xo;
            maxx = dim.x + minx;
        }
        else if ((*start).x >= 0)
        {
            minx = (*start).x - pos.x - xo;
        }
        
        if (dim.y != 0)
        {
            if ((*start).y < 0) (*start).y = 0;
            miny = (*start).y - pos.y - yo;
            maxy = dim.y + miny;
        }
        else if ((*start).y >= 0)
        {
            miny = (*start).y - pos.y - yo;
        }
        dim.x = maxx-minx;
        dim.y = maxy-miny;

        if (miny < 0)
        {
            fillRect((*start).x, (*start).y, dim.x, min((int)dim.y, -miny), bg);
        }
        if (maxy > h)
        {
            fillRect((*start).x, max((int)(*start).y, h + pos.y + yo), dim.x, min(maxy-h, (int)dim.y), bg);
            dim.y -= maxy-h;
        }
        dim.y -= max(0, -miny);
        if (dim.y < 0) dim.y = 0;
        if (minx < 0)
        {
            fillRect((*start).x, max((int)(*start).y, pos.y+yo), min((int)dim.x, -minx), dim.y, bg);
        }
        if (maxx > w)
        {
            fillRect(max((int)(*start).x, w + pos.x + xo), max((int)(*start).y, pos.y+yo), min(maxx-w, (int)dim.x), dim.y, bg);
        }
        
        startWrite();
        for(uint16_t yy = 0; yy < h; yy++) // Start drawing
        {
            for(uint16_t xx = 0; xx < w; xx++)
            {
                if (xx % size.x == 0)
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
                            drawFastHLine(pos.x+xo+xx, pos.y+yo+yy, size.x, color);
                            xx += size.x-1;
                        }
                    }
                    else if (bg != color &&  xx >= minx && xx < maxx && yy >= miny && yy < maxy)
                    {
                        writePixel(pos.x+xo+xx, pos.y+yo+yy, bg);
                    }
                    bits <<= 1;
                }
                else if (xx >= minx && xx < maxx && yy >= miny && yy < maxy)
                {
                    writePixel(pos.x+xo+xx, pos.y+yo+yy, bg);
                }
            }
            if (yy % size.y == (size.y-1))
            {
                oldbit = bit;
                oldbo = bo;
                oldbits = bits;
            }
            else
            {
                bit = oldbit;
                bo = oldbo;
                bits = oldbits;
            }
        }
        endWrite();
        (*start).x = pos.x+xo+maxx;
    } // End classic vs custom font
}

size_t tftLCD::writeBg(uint8_t c, vector2<int16_t> *pos, vector2<int16_t> dim)
{
    if(!gfxFont) 
    { // 'Classic' built-in font
        if(c == '\n') {                        // Newline?
            cursor_x  = 0;                     // Reset x to zero,
            cursor_y += textsize_y * 8;        // advance y one line
        } else if(c != '\r') {                 // Ignore carriage returns
            if(wrap && ((cursor_x + textsize_x * 6) > _width)) { // Off right?
                cursor_x  = 0;                 // Reset x to zero,
                cursor_y += textsize_y * 8;    // advance y one line
            }
            drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x, textsize_y);
            cursor_x += textsize_x * 6;          // Advance x one char
        }
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
                    drawCharBg(vector2<int16_t>(cursor_x, cursor_y), c, textcolor, textbgcolor, vector2<uint8_t>(textsize_x, textsize_y), pos, dim);
                }
                cursor_x += (uint8_t)pgm_read_byte(&glyph->xAdvance) * (int16_t)textsize_x;
            }
        }
    }
    return 1;
}

void tftLCD::printBg(const String &str)
{
    if (str.indexOf('\n') == -1)
    {
        int16_t x = 0, y = 0;
        uint16_t w = 0, h = 0;
        getTextBounds(str, 0, 0, &x, &y, &w, &h);
        vector2<int16_t> pos(x + cursor_x, y + cursor_y);
        for (unsigned int i = 0; i < str.length(); i++)
        {
            writeBg(str[i], &pos, vector2<int16_t>(0, h));
        }
    }
    else
    {
        String buf;
        int a = 0;
        int b = str.indexOf('\n');
        while (a < str.length())
        {
            buf = str.substring(a, b);
            printBg(buf);
            print('\n');
            a = b+1;
            b = str.indexOf('\n', a);
            if (b == -1) b = str.length();
        }
    }
}

void tftLCD::printBg(const String &str, uint8_t pad)
{
    printBg(str, vector2<uint8_t>(pad, pad));
}

void tftLCD::printBg(const String &str, vector2<uint8_t> pad)
{
/*     int16_t x = 0, y = 0;
    uint16_t w = 0, h = 0;
    getTextBounds(str, 0, 0, &x, &y, &w, &h); */
    if (str.indexOf('\n') == -1)
    {
        int16_t x = 0, y = 0;
        uint16_t w = 0, h = 0;
        getTextBounds(str, 0, 0, &x, &y, &w, &h);
        vector2<int16_t> pos(x + cursor_x - pad.x, y + cursor_y - pad.y);
        h += 2*pad.y;
        for (unsigned int i = 0; i < str.length(); i++)
        {
            writeBg(str[i], &pos, vector2<int16_t>(0, h));
        }
        fillRect(pos.x, pos.y, pad.x, h, textbgcolor);
    }
    else
    {
        String buf;
        int a = 0;
        int b = str.indexOf('\n');
        while (a < str.length())
        {
            buf = str.substring(a, b);
            printBg(buf);
            print('\n');
            a = b+1;
            b = str.indexOf('\n', a);
            if (b == -1) b = str.length();
        }
    }
}

void tftLCD::printBg(const String &str, vector2<int16_t> pos, vector2<uint16_t> dim)
{

    //int16_t x = cursor_x, y = cursor_y;
    int16_t x = 0, y = 0;
    uint16_t w = 0, h = 0;
    vector2<int16_t> start = pos;
    vector2<int16_t> end(0, 0);

    for (unsigned int i = 0; i < str.length(); i++)
    {        
        getTextBounds(String(str[i]), cursor_x, cursor_y, &x, &y, &w, &h);
        //charBounds(str[i], &x, &y, &minx, &miny, &maxx, &maxy);
        drawRect(x, y, w, h, TFT_CYAN);
        if (x < pos.x + dim.x && x+w > pos.x && y < pos.y + dim.y && y+h > pos.y)
        {
            if (pos.x+dim.x < x+w) end.x = x-pos.x+dim.x;
            if (pos.y+dim.y < y+h) end.y = y-pos.y+dim.y;
            writeBg('l', &start, end);
        }
        else
        {
            write(str[i]);
        }
    }
    //fillRect(pos.x, pos.y, pad.x, h, textbgcolor);
}

/**************************************************************************/
/*!
    @brief  Return a vector with the width and height of the text
    @param  String to evaluate
*/
/**************************************************************************/
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
            printBg(str);
        }
        else
        {
            print(str);
        }
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
            if (gfxFont && (textcolor != textbgcolor))
            {
                printBg(buf);
            }
            else
            {
                print(buf);
            }
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
