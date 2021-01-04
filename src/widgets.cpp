
#include "widgets.h"

/********************************************************************************
    Screen 
********************************************************************************/
Screen::Screen(lcdUI* UI)
{
    ESP_LOGV(__FILE__, "Created Screen\n");
    _UI = UI;
}

Screen::~Screen()
{
    ESP_LOGV(__FILE__, "Deleted Screen\n");
}

void Screen::handleTouch(touchEvent event, Vec2h pos)
{
    return;
}
