
#include "widgets.h"
#include "dbg_log.h"

/********************************************************************************
    Screen 
********************************************************************************/
Screen::Screen(lcdUI* UI)
{
    DBG_LOGV("Create Screen\n");
    _UI = UI;
}

Screen::~Screen()
{
    DBG_LOGV("Delete Screen\n");
}

void Screen::handleTouch(const TchEvent& event)
{
    return;
}
