
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
    #ifdef DEBUG_MODE
    Serial.println("Created canvas");
    #endif
}

canvas::~canvas()
{
    #ifdef DEBUG_MODE
    Serial.println("Delete canvas");
    #endif

    delete child;
    Serial.println("Deleted child");
    child = NULL;
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

/**************************************************************************/
/*!
    @brief  Set all "child" variables to NULL
*/
/**************************************************************************/
void canvas::nullify()
{
    child->nullify();
    child = NULL;
}

/********************************************************************************
    Box 
********************************************************************************/

void box::nullify()
{
    child->nullify();
    child = NULL;
}
