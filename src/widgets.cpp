
#include "widgets.h"



/* Canvas */
void canvas::getSize(tftLCD *tft, int16_t &w, int16_t &h)
{
    w=h=0;
}

void canvas::render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h)
{
    if (w <= 0) w = tft->width()-x;
    if (h <= 0) h = tft->height()-y;
    child->render(tft, x, y, w, h);
}

void canvas::attachComponent(widget *chld)
{
    child=chld;
}

void canvas::clear()
{
    if(child) delete child;
}

/* Vertical Box */
void verticalBox::getSize(tftLCD *tft, int16_t &w, int16_t &h)
{
    int16_t x = 0, y = 0;
    w = h = 0;
    bool fillX = false, fillY = false;
    for (uint8_t i = 0; i < elNum; i++)
    {
        if(!child[i]) continue;
        child[i]->getSize(tft, x, y);
        if (x == 0)
        {
            w = 0;
            fillX = true;
        } else if (!fillX)
        {
            if (x > 0)
            {
                if (w < 0)
                {
                    w = 0;
                    fillX = true;
                } else if (x > w)
                {
                    w = x;
                }
            }else if (x < 0)
            {
                if (w > 0)
                {
                    w = 0;
                    fillX = true;
                } else if (x < w)
                {
                    w = x;
                }
            }
        }
        if (y == 0)
        {
            h = 0;
        }
        else if (!fillY)
        {
            if (y > 0 && h < 0)
            {
                h = 0;
                fillY = true;
            }else if (y < 0 && h > 0)
            {
                h = 0;
                fillY = true;
            } else
            {
                h += y;
            }
        }
    }
}

bool verticalBox::attachComponent(widget *chld)
{
    for (uint8_t i = 0; i < elNum; i++)
    {
        if (!child[i])
        {
            child[i] = chld;
            return true;
        }
    }
    return false;
}

void verticalBox::render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h)
{
    if (!update && init) return;
    int16_t wi = 0;
    int16_t he = 0;
    uint16_t resHeight = 0;
    uint8_t fillNum = 0;
    for (uint8_t i = 0; i < elNum; i++)
    {
        if(child[i])
        {
            child[i]->getSize(tft, wi, he);
            he == 0 ? fillNum++ : resHeight += abs(he) ;
        }
    }

    int16_t a,b,c,d;
    a=c=d=0;
    b=y;
    for (uint8_t i = 0; i < elNum; i++)
    {
        if(child[i])
        {
            child[i]->getSize(tft, wi, he);
            if (wi==0)
            {
                a = x;
                c = w;
            } else if (wi > 0)
            {
                a = x;
                c = wi;
            } else
            {
                a = x + w + wi;
                c = -wi;
            }

            if (he == 0)
            {
                d = (h-resHeight)/fillNum;
            } else if (he > 0)
            {
                d = he;
            } else
            {
                d = -he;
                if (!fillNum)
                {
                    b = h - resHeight + b;
                    fillNum = 1;
                }
            }
            child[i]->render(tft, a, b, c, d);
            b += d;
        }
    }
    init = true;
}

void textBox::getSize(tftLCD *tft, int16_t &w, int16_t &h)
{
    int16_t wi, he;
    tft->setTextSize(size);
    tft->setFont(font);
    tft->getTextBounds(text, &wi, &he);
    switch (arrange)
    {
    case fillMode::TopLeft:
        w=wi+2*padding;
        h=he+2*padding;
        break;

    case fillMode::TopCenter:
        w=0;
        h=he+2*padding;
        break;

    case fillMode::TopRight:
        w=-wi-2*padding;
        h=he+2*padding;
        break;

    case fillMode::CenterLeft:
        w=wi+2*padding;
        h=0;
        break;
    
    case fillMode::CenterCenter:
        w=0;
        h=0;
        break;

    case fillMode::CenterRight:
        w=-wi-2*padding;
        h=0;
        break;

    case fillMode::BotLeft:
        w=wi+2*padding;
        h=-he-2*padding;
        break;

    case fillMode::BotCenter:
        w=0;
        h=-he-2*padding;
        break;

    case fillMode::BotRight:
        w=-wi-2*padding;
        h=-he-2*padding;
        break;

    default:
        break;
    }
}
void textBox::render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h)
{
    if (!update && init) return;
    int16_t w1, h1;
    tft->setTextSize(size);
    tft->setTextColor(color);
    tft->setFont(font);
    tft->getTextBounds(text, &w1, &h1);
    tft->drawRect(x, y, w, h, TFT_RED);
    tft->drawRect(x+(w-w1)/2, y+(h-h1)/2, w1, h1, TFT_BLUE);
    tft->setCursor(x+w/2, y+h/2);
    tft->printCenter(text);
    init = true;
}
