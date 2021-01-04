
#include "widgets.h"


Vec2h arrangeSize(Vec2h size, fillMode arrange)
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
    Widget 
********************************************************************************/
void widget::render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h)
{
    if (!update && init) return; // Render only once?
    draw(tft, x, y, w, h);
    init = true;
}

/********************************************************************************
    Screen 
********************************************************************************/
Screen::Screen()
{
    ESP_LOGV(__FILE__, "Created Screen\n");
}

Screen::~Screen()
{
    ESP_LOGV(__FILE__, "Deleted Screen\n");
}

void Screen::render(tftLCD *tft)
{
    ESP_LOGV(__FILE__, "Screen render start\n");

    if (child) child->render(tft, 0, 0, tft->width(), tft->height());

    ESP_LOGV(__FILE__, "Screen render end\n");
}

void Screen::attachToRoot(widget *chld)
{
    child=chld;
}

void Screen::handleTouch(touchEvent event, Vec2h pos)
{
    return;
}
