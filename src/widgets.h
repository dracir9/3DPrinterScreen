#pragma once

#ifndef WIDGETS_H
#define WIDGETS_H

#include "tftLCD.h"

class lcdUI;

/**************************************************************************
    Base frame of the UI
**************************************************************************/
class Screen
{
protected:
    lcdUI *_UI;

public:
    enum touchEvent : uint8_t
    {
        press=0,
        release,
        hold
    };
    
    Screen(lcdUI* UI);
    virtual ~Screen();

    virtual void render(tftLCD& tft) = 0;
    virtual void update(const uint32_t deltaTime) = 0;
    virtual void handleTouch(const touchEvent event, const Vec2h pos);
};

#endif
