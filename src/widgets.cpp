
#include "widgets.h"

vector2<int16_t> arrangeSize(vector2<int16_t> size, fillMode arrange)
{
    size.x = abs(size.x);
    size.y = abs(size.y);
    switch (arrange)
    {
    case fillMode::TopLeft:
        break;

    case fillMode::TopCenter:
        size.x = 0;
        break;

    case fillMode::TopRight:
        size.x *= -1;
        break;

    case fillMode::CenterLeft:
        size.y = 0;
        break;
    
    case fillMode::CenterCenter:
        size.x = 0;
        size.y = 0;
        break;

    case fillMode::CenterRight:
        size.x *= -1;
        size.y = 0;
        break;

    case fillMode::BotLeft:
        size.y *= -1;
        break;

    case fillMode::BotCenter:
        size.x = 0;
        size.y *= -1;
        break;

    case fillMode::BotRight:
        size.x *= -1;
        size.y *= -1;
        break;

    default:
        break;
    }

    return size;
}

/********************************************************************************
    Canvas 
********************************************************************************/
canvas::canvas(bool updt):
    widget(updt)
{
}

canvas::~canvas()
{
    delete child;
}

vector2<int16_t> canvas::getSize(tftLCD *tft)
{
    return vector2<int16_t>();
}

void canvas::render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h)
{
#ifdef DEBUG_MODE
    Serial.println("Canvas render start");
#endif

    if (w <= 0) w = tft->width()-x;
    if (h <= 0) h = tft->height()-y;
    if (child) child->render(tft, x, y, w, h);

#ifdef DEBUG_MODE
    Serial.println("Canvas render end");
#endif
}

void canvas::attachComponent(widget *chld)
{
    child=chld;
}

void canvas::clear()
{
    if(child)
    {
        delete child;
    }
    child = NULL;
}

/********************************************************************************
    Vertical Box 
********************************************************************************/
verticalBox::verticalBox(uint8_t elem, bool updt):
    widget(updt), elNum(elem), child(new widget*[elem])
{
    for (uint8_t i = 0; i < elNum; i++)
    {
        child[i] = NULL;
    }
}

verticalBox::~verticalBox()
{
    if (child)
    {
        for (uint8_t i = 0; i < elNum; i++)
        {
            if(child[i]) delete child[i];
            child[i] = NULL;
        }
        delete[] child;
        child = NULL;
    } 
}

vector2<int16_t> verticalBox::getSize(tftLCD *tft)
{
    vector2<int16_t> size0;
    bool fillX = false, fillY = false;
    for (uint8_t i = 0; i < elNum; i++)
    {
        if(!child[i]) continue;
        vector2<int16_t> size1 = child[i]->getSize(tft);
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
#ifdef DEBUG_MODE
    Serial.println("vertical Box render start");
#endif
    if (!update && init) return;
    vector2<int16_t> size;
    uint16_t resHeight = 0;
    uint8_t fillNum = 0;
    for (uint8_t i = 0; i < elNum; i++)
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
    for (uint8_t i = 0; i < elNum; i++)
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
                    b = h - resHeight + b;
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

    init = true;

#ifdef DEBUG_MODE
    Serial.println("vertical Box render end");
#endif
}

/********************************************************************************
    Horizontal Box 
********************************************************************************/



/********************************************************************************
    Text Box 
********************************************************************************/
vector2<int16_t> textBox::getSize(tftLCD *tft)
{
#ifdef DEBUG_MODE
    Serial.println("text Box get Size start");
#endif

    tft->setTextSize(size);
    if (font) tft->setFreeFont(font);
    else tft->setTextFont(GLCD);
    vector2<int16_t> size = tft->getTextBounds(*text);
    size.x = max(paddingX, size.x);
    size.y = max(paddingY, size.y);
    return arrangeSize(size, arrange);
 

#ifdef DEBUG_MODE
    Serial.println("text Box get Size end");
#endif
}

void textBox::render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h)
{
#ifdef DEBUG_MODE
    Serial.println("text Box render start");
#endif

    if (!update && init) return; // Render only once?

    vector2<int16_t> dim = tft->getTextBounds(*text);
    dim.x = max(paddingX, dim.x);
    dim.y = max(paddingY, dim.y);
    tft->img.setColorDepth(1);
    tft->img.createSprite(dim.x, dim.y);

    tft->img.setTextSize(size);
    tft->img.setTextColor(txtcolor);
    if (font) tft->img.setFreeFont(font);
    else tft->img.setTextFont(GLCD);
    tft->img.setCursor(dim.x/2, dim.y/2);
    tft->img.printCenter(*text);

    tft->img.setBitmapColor(txtcolor, bgcolor);
    tft->img.pushSprite(x+(w-dim.x)/2, y+(h-dim.y)/2);

    tft->img.deleteSprite();

#ifdef DEBUG_LINES
    vector2<int16_t> size = tft->getTextBounds(*text);
    tft->drawRect(x, y, w, h, TFT_RED);
    tft->drawRect(x+(w-size.x)/2, y+(h-size.y)/2, size.x, size.y, TFT_BLUE);
#endif

    init = true;

#ifdef DEBUG_MODE
    Serial.println("text Box render end");
#endif
}
