#pragma once

#ifndef WIDGETS_H
#define WIDGETS_H

#include <Arduino.h>
#include "Configuration.h"
#include "tftLCD.h"

enum fillMode : int8_t
{
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    CenterCenter,
    CenterRight,
    BotLeft,
    BotCenter,
    BotRight
};

vector2<int16_t> arrangeSize(vector2<int16_t> size, fillMode arrange);

/**************************************************************************
    Base class for UI ogjects
**************************************************************************/
class widget
{
protected:
    bool update = true;
    bool init = false;

public:
    widget(bool updt):
    update(updt)
    {}
    virtual ~widget() = default;
    virtual vector2<int16_t> getSize(tftLCD *tft) const = 0;
    void render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h);
    virtual void draw(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h) const = 0;
};

/**************************************************************************
    Base frame of the UI
**************************************************************************/
class canvas
{
protected:
    widget *child = NULL;

public:
    canvas(bool updt=true);
    virtual ~canvas();

    void render(tftLCD *tft);
    void attachToRoot(widget *chld);
    virtual void update() = 0;
};

#endif
