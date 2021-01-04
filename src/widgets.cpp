
#include "widgets.h"

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

void Screen::handleTouch(touchEvent event, Vec2h pos)
{
    return;
}
