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
    virtual vector2<int16_t> getSize(tftLCD *tft) = 0;
    virtual void render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h) = 0;
};

/**************************************************************************
    Base frame of the UI
**************************************************************************/
class canvas //: public widget
{
protected:
    widget *child = NULL;

public:
    canvas(bool updt=true);
    virtual ~canvas();

    void render(tftLCD *tft);
    void attachComponent(widget *chld);
    virtual void update() = 0;
};

/**************************************************************************
    Box
**************************************************************************/
class box : public widget
{
protected:
    widget *child = NULL;
    uint16_t bgColor = TFT_BLACK;
    uint8_t padding = 0;

public:
    box(bool updt = true);
    ~box();

    vector2<int16_t> getSize(tftLCD *tft);
    void render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h);
    bool attachComponent(widget *chld);
};

/* class grid : public widget
{
public:
    int16_t gridX;//=118;
    int16_t gridY;//=40;
    const uint8_t col;//=3;
    const uint8_t row;//=3;
    uint8_t space;//=8;
    int16_t cellW;//=(width()-gridX-(e-1)*8)/e;
    int16_t cellH;//☺=25;

    grid(uint8_t c, uint8_t r, bool updt = true):
    widget(updt), col(c), row(r){}

    void getTLPoint(int16_t *x, int16_t *y, uint8_t c, uint8_t r) const;
    void getCenterPoint(int16_t *x, int16_t *y, uint8_t c, uint8_t r) const;
}; */

#endif
