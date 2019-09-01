
#ifndef HORIZONTALBOX_H
#define HORIZONTALBOX_H

#include <Arduino.h>
#include "Widgets/verticalBox.h"

/**************************************************************************
    Arrange widgets horizontally
**************************************************************************/

template<uint8_t NUM>
class horizontalBox : public verticalBox<NUM>
{
public:
    horizontalBox(bool updt = true);

    vector2<int16_t> getSize(tftLCD *tft);
    void render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h);
};

/********************************************************************************
    Horizontal Box implementation
********************************************************************************/

template<uint8_t NUM>
horizontalBox<NUM>::horizontalBox(bool updt):
    verticalBox<NUM>(updt)
{

}

template<uint8_t NUM>
vector2<int16_t> horizontalBox<NUM>::getSize(tftLCD *tft)
{
    vector2<int16_t> size0;
    bool fillX = false, fillY = false;
    for (uint8_t i = 0; i < NUM; i++)
    {
        if(!verticalBox<NUM>::child[i]) continue;
        vector2<int16_t> size1 = verticalBox<NUM>::child[i]->getSize(tft);
        if (size1.x == 0) // Calculate X
        {
            size0.x = 0;
            fillX = true;
        } else if (!fillX)
        {
            if (size1.x > 0)
            {
                if (size0.x < 0)
                {
                    size0.x = 0;
                    fillX = true;
                } else if (size1.x > size0.x)
                {
                    size0.x = size1.x;
                }
            }else if (size1.x < 0)
            {
                if (size0.x > 0)
                {
                    size0.x = 0;
                    fillX = true;
                } else if (size1.x < size0.x)
                {
                    size0.x = size1.x;
                }
            }
        }
        if (size1.y == 0) // Calculate Y
        {
            size0.y = 0;
        }
        else if (!fillY)
        {
            if (size1.y > 0 && size0.y < 0)
            {
                size0.y = 0;
                fillY = true;
            }else if (size1.y < 0 && size0.y > 0)
            {
                size0.y = 0;
                fillY = true;
            } else
            {
                size0.y += size1.y;
            }
        }
    }
    return size0;
}

template<uint8_t NUM>
void horizontalBox<NUM>::render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h)
{
    #ifdef DEBUG_MODE
    Serial.println("horizontal Box render start");
    #endif
    if (!verticalBox<NUM>::update && verticalBox<NUM>::init) return;
    vector2<int16_t> size;
    uint16_t resHeight = 0;
    uint8_t fillNum = 0;
    for (uint8_t i = 0; i < NUM; i++)
    {
        if(verticalBox<NUM>::child[i])
        {
            size = verticalBox<NUM>::child[i]->getSize(tft);
            size.y == 0 ? fillNum++ : resHeight += abs(size.y) ;
        }
    }

    int16_t a,b,c,d;
    a=c=d=0;
    b=y;
    for (uint8_t i = 0; i < NUM; i++)
    {
        if(verticalBox<NUM>::child[i])
        {
            size = verticalBox<NUM>::child[i]->getSize(tft);
            if (size.x == 0)
            {
                a = x;
                c = w;
            } else if (size.x > 0)
            {
                a = x;
                c = size.x;
            } else
            {
                a = x + w + size.x;
                c = -size.x;
            }

            if (size.y == 0)
            {
                d = (h-resHeight)/fillNum;
            } else if (size.y > 0)
            {
                d = size.y;
            } else
            {
                d = -size.y;
                if (!fillNum)
                {
                    b = h - resHeight + b;
                    fillNum = 1;
                }
            }
            verticalBox<NUM>::child[i]->render(tft, a, b, c, d);
            b += d;
        }
    }

    #ifdef DEBUG_LINES
        size = getSize(tft);
        if (size.x == 0)
        {
            size.x = w;
        } else if (size.x < 0)
        {
            x += w + size.x;
        }
        if (size.y == 0)
        {
            size.y = h;
        } else if (size.y < 0)
        {
            y += h + size.y;
        }
        tft->drawRect(x, y, abs(size.x), abs(size.y), TFT_GREEN);
    #endif

    verticalBox<NUM>::init = true;

    #ifdef DEBUG_MODE
    Serial.println("vertical Box render end");
    #endif
}

#endif