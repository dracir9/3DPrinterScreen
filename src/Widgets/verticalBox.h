
#ifndef BOX_H
#define BOX_H

#include <Arduino.h>
#include "widgets.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "verticalBox"

/**************************************************************************
    Arrange widgets vertically
**************************************************************************/

template<uint8_t NUM>
class verticalBox : public widget
{
protected:
    widget *child[NUM];

public:
    verticalBox(bool updt = true);
    ~verticalBox();

    virtual Vec2h getSize(tftLCD *tft) const;
    virtual void draw(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h) const;
    bool attachComponent(widget *chld);
};

/********************************************************************************
    Vertical Box implementation
********************************************************************************/

template<uint8_t NUM>
verticalBox<NUM>::verticalBox(bool updt):
    widget(updt)
{
    for (uint8_t i = 0; i < NUM; i++)
    {
        child[i] = NULL;
    }
}

template<uint8_t NUM>
verticalBox<NUM>::~verticalBox()
{
    ESP_LOGV(TAG, "Delete verticalBox\n");
}

template<uint8_t NUM>
Vec2h verticalBox<NUM>::getSize(tftLCD *tft) const
{
    Vec2h size0;
    bool fillX = false, fillY = false;
    for (uint8_t i = 0; i < NUM; i++)
    {
        if(!child[i]) continue;
        Vec2h size1 = child[i]->getSize(tft);
        if (size1.x == 0)
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
        if (size1.y == 0)
        {
            size0.y = 0;
            fillY = true;
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
bool verticalBox<NUM>::attachComponent(widget *chld)
{
    for (uint8_t i = 0; i < NUM; i++)
    {
        if (!child[i])
        {
            child[i] = chld;
            return true;
        }
    }
    return false;
}

template<uint8_t NUM>
void verticalBox<NUM>::draw(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h) const
{
    ESP_LOGV(TAG, "verticalBox render start\n");

    Vec2h size;
    uint16_t resHeight = 0;
    uint8_t fillNum = 0;
    for (uint8_t i = 0; i < NUM; i++)
    {
        if(child[i])
        {
            size = child[i]->getSize(tft);
            size.y == 0 ? fillNum++ : resHeight += abs(size.y) ;
        }
    }

    int16_t a,b,c,d;
    a=c=d=0;
    b=y;
    for (uint8_t i = 0; i < NUM; i++)
    {
        if(child[i])
        {
            size = child[i]->getSize(tft);
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
                    b = h - resHeight + b; // b += h-resHeight
                    fillNum = 1;
                }
            }
            child[i]->render(tft, a, b, c, d);
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

    ESP_LOGV(TAG, "verticalBox render end\n");
}

#endif