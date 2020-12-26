
#ifndef HORIZONTALBOX_H
#define HORIZONTALBOX_H

#include <Arduino.h>
#include "Widgets/verticalBox.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "horizontalBox"

/**************************************************************************
    Arrange widgets horizontally
**************************************************************************/

template<uint8_t NUM>
class horizontalBox : public verticalBox<NUM>
{
public:
    horizontalBox(bool updt = true);

    Vec2h getSize(tftLCD *tft);
    void draw(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h) const;
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
Vec2h horizontalBox<NUM>::getSize(tftLCD *tft)
{
    Vec2h size0;
    bool fillX = false, fillY = false;
    for (uint8_t i = 0; i < NUM; i++)
    {
        if(!verticalBox<NUM>::child[i]) continue;
        Vec2h size1 = verticalBox<NUM>::child[i]->getSize(tft);
        if (size1.y == 0) // Calculate Y
        {
            size0.y = 0;
            fillY = true;
        } else if (!fillY)
        {
            if (size1.y > 0)
            {
                if (size0.y < 0)
                {
                    size0.y = 0;
                    fillY = true;
                } else if (size1.y > size0.y)
                {
                    size0.y = size1.y;
                }
            }else if (size1.y < 0)
            {
                if (size0.y > 0)
                {
                    size0.y = 0;
                    fillY = true;
                } else if (size1.y < size0.y)
                {
                    size0.y = size1.y;
                }
            }
        }
        if (size1.x == 0) // Calculate X
        {
            size0.x = 0;
            fillX = true;
        }
        else if (!fillX)
        {
            if (size1.x > 0 && size0.x < 0)
            {
                size0.x = 0;
                fillX = true;
            }else if (size1.x < 0 && size0.x > 0)
            {
                size0.x = 0;
                fillX = true;
            } else
            {
                size0.x += size1.x;
            }
        }
    }
    return size0;
}

template<uint8_t NUM>
void horizontalBox<NUM>::draw(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h) const
{
    ESP_LOGV(TAG, "horizontal Box render start\n");

    Vec2h size;
    uint16_t resWidth = 0;
    uint8_t fillNum = 0;
    for (uint8_t i = 0; i < NUM; i++)
    {
        if(verticalBox<NUM>::child[i])
        {
            size = verticalBox<NUM>::child[i]->getSize(tft);
            size.x == 0 ? fillNum++ : resWidth += abs(size.x);
        }
    }

    int16_t x0, y0, w0, h0;
    x0 = x;
    y0 = w0 = h0 = 0;
    for (uint8_t i = 0; i < NUM; i++)
    {
        if(verticalBox<NUM>::child[i])
        {
            size = verticalBox<NUM>::child[i]->getSize(tft);
            if (size.x == 0)
            {
                w0 = (w-resWidth)/fillNum;
                
            } else if (size.x > 0)
            {
                w0 = size.x;
            } else
            {
                w0 = -size.x;
                if(fillNum == 0)
                {
                    x0 = w - resWidth + x0; // x0 += w - resWidth
                    fillNum = 1;
                }
            }

            if (size.y == 0)
            {
                y0 = y;
                h0 = h;
            } else if (size.y > 0)
            {
                y0 = y;
                h0 = size.y;
            } else
            {
                y0 = y + h + size.y;
                h0 = -size.y;
            }
            verticalBox<NUM>::child[i]->render(tft, x0, y0, w0, h0);
            x0 += w0;
        }
    }

    #ifdef DEBUG_LINES
        size = verticalBox<NUM>::getSize(tft);
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
        tft->drawRect(x, y, tft->width(), abs(size.y), TFT_GREEN);
    #endif

    ESP_LOGV(TAG, "vertical Box render end\n");
}
#endif