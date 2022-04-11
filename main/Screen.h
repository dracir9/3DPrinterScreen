#pragma once

#ifndef WIDGETS_H
#define WIDGETS_H

#include "tftLCD.h"
#include "TchScr_Drv.h"

class lcdUI;

/**************************************************************************
    Base frame of the UI
**************************************************************************/
class Screen
{
protected:
    lcdUI *_UI;

public:
    Screen(lcdUI* UI);
    virtual ~Screen();

    virtual void render(tftLCD& tft) = 0;
    virtual void update(const uint32_t deltaTime, TchScr_Drv& ts) = 0;
    virtual void handleTouch(const TchEvent& event);
};

#endif
