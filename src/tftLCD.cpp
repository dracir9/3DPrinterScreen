
#include "tftLCD.h"
#include <pgmspace.h>

#ifdef LOAD_GFXFF
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
#endif

/*####################################################
    tft Sprite class
####################################################*/

/**************************************************************************/
/*!
    @brief  Print a string centered at cursor location (supports multy-line text)
    @param  String to print
*/
/**************************************************************************/
void tftSprite::printCenter(const String &str)
{
    uint8_t h = 1;
    for (unsigned int i = 0; i < str.length(); i++)
    {
        if(str[i] == '\n') h++;
    }
    h *= fontHeight();
    cursor_y -= h/2;
    int16_t center = cursor_x;
    setTextDatum(TC_DATUM);
    if (str.indexOf('\n') == -1)
    {
        drawString(str, cursor_x, cursor_y);
    }
    else
    {
        int a = 0;
        int b = str.indexOf('\n');
        while (a < str.length())
        {
            drawString(str.substring(a, b), center, cursor_y);
            print('\n');
            a = b+1;
            b = str.indexOf('\n', a);
            if (b == -1) b = str.length();
        }
    }
}

void tftSprite::printCenter(const char *str)
{
    printCenter(String(str));
}

/*####################################################
    tft LCD class
####################################################*/

/**************************************************************************/
/*!
    @brief    Draw single character over any rectangle without flickering (Only free fonts supported yet)
            May be slower than using sprites but less RAM intesive.
    @param    pos   Cursor position (2D vector)
    @param    c     Character to draw
    @param    color Text color
    @param    bg    Rectangle color
    @param    size  Text size
    @param    start Upper Left corner of the rectagle (2D vector)
    @param    dim   Width and height of the rectangle (2D vector)
*/
/**************************************************************************/
void tftLCD::drawCharBg(Vec2h pos, uint8_t c, uint16_t color, uint16_t bg, uint8_t size, Vec2h *start, Vec2h dim)
{
    #ifdef LOAD_GFXFF
    int16_t minx = 0, miny = 0, maxx = 5, maxy = 8;
    if(gfxFont) // 'Classic' built-in font
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
        if (size > 1)
        {
            w *= size;
            h *= size;
            xo *= size;
            yo *= size;
        }
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
                if (xx % size == 0)
                {
                    if(!(bit++ & 7))
                    {
                        bits = pgm_read_byte(&bitmap[bo++]);
                    }
                    if(bits & 0x80)
                    {
                        if(size == 1)
                        {
                            drawPixel(pos.x+xo+xx, pos.y+yo+yy, color);
                        }
                        else
                        {
                            drawFastHLine(pos.x+xo+xx, pos.y+yo+yy, size, color);
                            xx += size-1;
                        }
                    }
                    else if (bg != color &&  xx >= minx && xx < maxx && yy >= miny && yy < maxy)
                    {
                        drawPixel(pos.x+xo+xx, pos.y+yo+yy, bg);
                    }
                    bits <<= 1;
                }
                else if (xx >= minx && xx < maxx && yy >= miny && yy < maxy)
                {
                    drawPixel(pos.x+xo+xx, pos.y+yo+yy, bg);
                }
            }
            if (yy % size == (size-1))
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
    else
    #endif
    {
        if((pos.x >= _width)            || // Clip right
           (pos.y >= _height)           || // Clip bottom
           ((pos.x + 6 * size - 1) < 0) || // Clip left
           ((pos.y + 8 * size - 1) < 0))   // Clip top
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
                    if(size == 1)
                        drawPixel(pos.x+i, pos.y+j, color);
                    else
                        fillRect(pos.x+i*size, pos.y+j*size, size, size, color);
                }
                else if(bg != color)
                {
                    if(size == 1)
                        drawPixel(pos.x+i, pos.y+j, bg);
                    else
                        fillRect(pos.x+i*size, pos.y+j*size, size, size, bg);
                }
            }
        }
        if(bg != color)
        { // If opaque, draw vertical line for last column
            if(size == 1) drawFastVLine(pos.x+5, pos.y, 8, bg);
            else          fillRect(pos.x+5*size, pos.y, size, 8*size, bg);
        }
        endWrite();
    }   
}

/**************************************************************************/
/*!
    @brief    Draw single character over any rectangle without flickering (Only free fonts supported yet)
            May be slower than using sprites but less RAM intesive.
    @param    c     Character to draw
    @param    pos   Upper Left corner of the rectagle (2D vector)
    @param    dim   Width and height of the rectangle (2D vector)
*/
/**************************************************************************/
size_t tftLCD::writeBg(uint8_t c, Vec2h *pos, Vec2h dim)
{
    #ifdef LOAD_GFXFF
    if(gfxFont)
    {
        if(c == '\n')
        {
            cursor_x  = 0;
            cursor_y += (int16_t)textsize *
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
                    if(textwrapX && ((cursor_x + textsize * (xo + w)) > _width))
                    {
                        cursor_x  = 0;
                        cursor_y += (int16_t)textsize *
                          (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
                    }
                    drawCharBg(Vec2h(cursor_x, cursor_y), c, textcolor, textbgcolor, textsize, pos, dim);
                }
                cursor_x += (uint8_t)pgm_read_byte(&glyph->xAdvance) * (int16_t)textsize;
            }
        }
    }
    else // Custom font
    #endif
    { // 'Classic' built-in font
        if(c == '\n') {                        // Newline?
            cursor_x  = 0;                     // Reset x to zero,
            cursor_y += textsize * 8;        // advance y one line
        } else if(c != '\r') {                 // Ignore carriage returns
            if(textwrapX && ((cursor_x + textsize * 6) > _width)) { // Off right?
                cursor_x  = 0;                 // Reset x to zero,
                cursor_y += textsize * 8;    // advance y one line
            }
            drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
            cursor_x += textsize * 6;          // Advance x one char
        }
    }
    return 1;
}

/**************************************************************************/
/*!
    @brief    Draw string with filled background without flickering (Support multy-line)
            May be slower than using sprites but less RAM intesive.
    @param    str   String to draw
*/
/**************************************************************************/
void tftLCD::printBg(const String &str)
{
    printBg(str, Vector2<uint8_t>());
}

/**************************************************************************/
/*!
    @brief    Draw string with filled background without flickering (Support multy-line)
            May be slower than using sprites but less RAM intesive.
    @param    str   String to draw
    @param    pad   Border width arround text
*/
/**************************************************************************/
void tftLCD::printBg(const String &str, uint8_t pad)
{
    printBg(str, Vector2<uint8_t>(pad, pad));
}

/**************************************************************************/
/*!
    @brief    Draw string with filled background without flickering (Support multy-line)
            May be slower than using sprites but less RAM intesive.
    @param    str   String to draw
    @param    pad   Border width arround text in X and Y directions (2D vector)
*/
/**************************************************************************/
void tftLCD::printBg(const String &str, Vector2<uint8_t> pad)
{
    if (str.indexOf('\n') == -1)
    {
        int16_t x = 0, y = 0;
        Vec2h size = getTextBounds(str, &x, &y);
        Vec2h pos(x + cursor_x - pad.x, y + cursor_y - pad.y);
        size.y += 2*pad.y;
        for (unsigned int i = 0; i < str.length(); i++)
        {
            writeBg(str[i], &pos, Vec2h(0, size.y));
        }
        fillRect(pos.x, pos.y, pad.x, size.y, textbgcolor);
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

/**************************************************************************/
/*!
    @brief    Draw string over any rectangle without flickering (Support multy-line) ########## TO DO: -Add multy line, -Fix some single line issues
            May be slower than using sprites but less RAM intesive.
    @param    str   String to draw
    @param    pos   Upper Left corner of the rectagle (2D vector)
    @param    dim   Width and height of the rectangle (2D vector)
*/
/**************************************************************************/
void tftLCD::printBg(const String &str, Vec2h pos, Vector2<uint16_t> dim)
{
    int16_t x = 0, y = 0;
    Vec2h start = pos;
    Vec2h end(0, 0);

    for (unsigned int i = 0; i < str.length(); i++)
    {        
        Vec2h size = getTextBounds(String(str[i]), &x, &y);
        x += cursor_x;
        y += cursor_y;
        //charBounds(str[i], &x, &y, &minx, &miny, &maxx, &maxy);

        #ifdef DEBUG_LINES  // Debugging
        drawRect(x, y, size.x, size.y, TFT_CYAN); 
        #endif

        if (x < pos.x + dim.x && x+size.x > pos.x && y < pos.y + dim.y && y+size.y > pos.y) // Check for character and rectangle overlap
        {
            if (pos.x+dim.x < x+size.x || i == str.length()-1) end.x = pos.x+dim.x-start.x;
            if (pos.y+dim.y < y+size.y) end.y = pos.y+dim.y-start.y;
            writeBg(str[i], &start, end);
        }
        else
        {
            write(str[i]);
        }
    }
}

/**************************************************************************/
/*!
    @brief    Helper to determine size of a character with current font/size.
       Broke this out as it's used by both the PROGMEM- and RAM-resident getTextBounds() functions.
    @param    c     The ascii character in question
    @param    x     Pointer to x location of character
    @param    y     Pointer to y location of character
    @param    minx  Minimum clipping value for X
    @param    miny  Minimum clipping value for Y
    @param    maxx  Maximum clipping value for X
    @param    maxy  Maximum clipping value for Y
*/
/**************************************************************************/
#if defined(LOAD_GFXFF) || defined(LOAD_GLCD)
void tftLCD::charBounds(char c, int16_t *x, int16_t *y, int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy)
{
    #ifdef LOAD_GFXFF
    if(gfxFont)
    {
        if(c == '\n')
        { // Newline?
            *x  = 0;    // Reset x to zero, advance y by one line
            *y += textsize * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
        }
        else if(c != '\r')
        { // Not a carriage return; is normal char
            uint8_t first = pgm_read_byte(&gfxFont->first),
                    last  = pgm_read_byte(&gfxFont->last);
            if((c >= first) && (c <= last)) { // Char present in this font?
                GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c - first);
                uint8_t gw = pgm_read_byte(&glyph->width),
                        gh = pgm_read_byte(&glyph->height),
                        xa = pgm_read_byte(&glyph->xAdvance);
                int8_t  xo = pgm_read_byte(&glyph->xOffset),
                        yo = pgm_read_byte(&glyph->yOffset);
                if(textwrapX && ((*x+(((int16_t)xo+gw)*textsize)) > _width))
                {
                    *x  = 0; // Reset x to zero, advance y by one line
                    *y += textsize * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
                }
                int16_t tsx = (int16_t)textsize,
                        tsy = (int16_t)textsize,
                        x1 = *x + xo * tsx,
                        y1 = *y + yo * tsy,
                        x2 = x1 + gw * tsx - 1,
                        y2 = y1 + gh * tsy - 1;
                if(x1 < *minx) *minx = x1;
                if(y1 < *miny) *miny = y1;
                if(x2 > *maxx) *maxx = x2;
                if(y2 > *maxy) *maxy = y2;
                *x += xa * tsx;
            }
        }

    }
    else
    #endif
    { // Default font

        if(c == '\n')
        {                     // Newline?
            *x  = 0;                        // Reset x to zero,
            *y += textsize * 8;           // advance y one line
            // min/max x/y unchaged -- that waits for next 'normal' character
        } else if(c != '\r')
        {  // Normal char; ignore carriage returns
            if(textwrapX && ((*x + textsize * 6) > _width))
            { // Off right?
                *x  = 0;                    // Reset x to zero,
                *y += textsize * 8;       // advance y one line
            }
            int x2 = *x + textsize * 6 - 1, // Lower-right pixel of char
                y2 = *y + textsize * 8 - 1;
            if(x2 > *maxx) *maxx = x2;      // Track max x, y
            if(y2 > *maxy) *maxy = y2;
            if(*x < *minx) *minx = *x;      // Track min x, y
            if(*y < *miny) *miny = *y;
            *x += textsize * 6;             // Advance x one char
        }
    }
}
#endif

/**************************************************************************/
/*!
    @brief  Return a vector with the width and height of a text
    @param  str     String to evaluate
*/
/**************************************************************************/
Vec2h tftLCD::getTextBounds(const String &str)
{
    int b = str.indexOf('\n');
    if (b == -1) // Single line
    {
        return Vec2h(textWidth(str), fontHeight());
    }
    else // Multy-line
    {
        int16_t w = 0, h = 0;
        String buf;
        int a = 0;
        while (a < str.length())
        {
            buf = str.substring(a, b)+'\n';
            w = max(textWidth(buf), w);         // Find the widest line
            a = b+1;
            b = str.indexOf('\n', a);
            if (b == -1) b = str.length();
            h++;                                // number of lines
        }
        h *= fontHeight();                      // Total height
        return Vec2h(w,h);
    }
    return Vec2h();
}

Vec2h tftLCD::getTextBounds(const String &str, int16_t *x, int16_t *y)
{
    if(str.length() != 0)
    {
        return getTextBounds(str.c_str(), x, y);
    }
    return Vec2h();
}

Vec2h tftLCD::getTextBounds(const char *str)
{
    ESP_LOGV(__FILE__, "getTextBounds start\n");

    return getTextBounds(String(str));
}

/**************************************************************************/
/*!
    @brief  Return the bounding box of a text
    @param  str     String to evaluate
    @param  x       X offset from cursor X position
    @param  y       Y offset from cursor Y position
*/
/**************************************************************************/
Vec2h tftLCD::getTextBounds(const char *str, int16_t *x, int16_t *y)
{
    if (textfont != 1)
    {
        return getTextBounds(str);
    }
    #if defined(LOAD_GFXFF) || defined(LOAD_GLCD)
        uint8_t c; // Current character

        *x = 0;
        *y = 0;
        Vec2h size = Vec2h();

        int16_t minx = _width, miny = _height, maxx = -1, maxy = -1;

        while((c = *str++))
            charBounds(c, x, y, &minx, &miny, &maxx, &maxy);

        if(maxx >= minx) {
            *x = minx;
            size.x  = maxx - minx + 1;
        }
        if(maxy >= miny) {
            *y = miny;
            size.y  = maxy - miny + 1;
        }
        return size;
    #else
        return Vec2h();
    #endif
}

/**************************************************************************/
/*!
    @brief  Print a string centered at cursor location (supports multy-line text)
    @param  String to print
*/
/**************************************************************************/
void tftLCD::printCenter(const String &str)
{
    int16_t x,y;
    int16_t center;

    center = cursor_x; // Save central X position for later
    Vec2h size = getTextBounds(str, &x, &y); // Get bounding box
    int b = str.indexOf('\n');
    if (b == -1) // Single line text
    {
        setCursor(getCursorX()-size.x/2-x, getCursorY()-size.y/2-y);
        printBg(str);

    }
    else // Multy-line text
    {
        String buf;
        cursor_y -= size.y/2+y;
        int a = 0;
        while (a < str.length()) // handle each line at once
        {
            buf = str.substring(a, b)+'\n';
            size = getTextBounds(buf, &x, &y);
            cursor_x = center-size.x/2-x;
            printBg(buf);
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

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t SPIFFS_read16(FILE *f) {
    uint16_t result;
    ((uint8_t *)&result)[0] = fgetc(f); // LSB
    ((uint8_t *)&result)[1] = fgetc(f); // MSB
    return result;
}

uint32_t SPIFFS_read32(FILE *f) {
    uint32_t result;
    ((uint8_t *)&result)[0] = fgetc(f); // LSB
    ((uint8_t *)&result)[1] = fgetc(f);
    ((uint8_t *)&result)[2] = fgetc(f);
    ((uint8_t *)&result)[3] = fgetc(f); // MSB
    return result;
}

void tftLCD::drawBmpSPIFFS(const char *filename, int16_t x, int16_t y)
{
    if ((x >= _width) || (y >= _height)) return;

    // Open requested file on SPIFFS
    FILE *bmpFS = fopen(filename, "r");

    if (!bmpFS)
    {
        ESP_LOGE(__FILE__, "File \"%s\" not found!", filename);
        return;
    }

    uint32_t seekOffset;
    uint16_t w, h, row;

    if (SPIFFS_read16(bmpFS) == 0x4D42)
    {
        SPIFFS_read32(bmpFS);
        SPIFFS_read32(bmpFS);
        seekOffset = SPIFFS_read32(bmpFS);
        SPIFFS_read32(bmpFS);
        w = SPIFFS_read32(bmpFS);
        h = SPIFFS_read32(bmpFS);

        if (SPIFFS_read16(bmpFS) == 1 && SPIFFS_read16(bmpFS) == 16 && SPIFFS_read32(bmpFS) == 3)
        {
            y += h - 1;

            bool oldSwapBytes = getSwapBytes();
            setSwapBytes(true);
            fseek(bmpFS, seekOffset, SEEK_SET);
            uint8_t lineBuffer[w * 2 + ((w & 1) << 1)];

            for (row = 0; row < h; row++)
            {
                fread(lineBuffer, 1, sizeof(lineBuffer), bmpFS);

                // Push the pixel row to screen, pushImage will crop the line if needed
                // y is decremented as the BMP image is drawn bottom up
                pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
            }
            setSwapBytes(oldSwapBytes);
        }
        else
            ESP_LOGE(__FILE__, "BMP format not recognized.");
    }
    fclose(bmpFS);
}
