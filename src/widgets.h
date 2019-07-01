#ifndef WIDGETS_H
#define WIDGETS_H

#include <Arduino.h>
//#include <Adafruit_GFX.h>
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

    virtual void getSize(tftLCD *tft, int16_t &w, int16_t &h) = 0;
    virtual void render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h) = 0;
};

/**************************************************************************
    Base frame of the UI
**************************************************************************/
class canvas : public widget
{
protected:
    widget *child = NULL;

public:
    canvas(bool updt=true):
    widget(updt){}

    void getSize(tftLCD *tft, int16_t &w, int16_t &h);
    void render(tftLCD *tft, int16_t x=0, int16_t y=0, int16_t w=0, int16_t h=0);
    void attachComponent(widget *chld);
};

/**************************************************************************
    Arrange widgets vertically
**************************************************************************/
class verticalBox : public widget
{
protected:
    uint8_t elNum = 1;
    widget **child;

public:
    verticalBox(uint8_t elem, bool updt = true):
    widget(updt), elNum(elem), child(new widget*[elem])
    {
        for (uint8_t i = 0; i < elNum; i++)
        {
            child[i] = NULL;
        }
    }
    ~verticalBox()
    {
        delete child;
    }

    void getSize(tftLCD *tft, int16_t &w, int16_t &h);
    void render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h);
    //bool attachComponent(widget *chld, uint8_t idx);
    bool attachComponent(widget *chld);
};

class textBox : public widget
{
protected:
    String text;
    fillMode arrange;
    uint8_t padding;
    uint8_t size;
    uint16_t color;
    GFXfont *font;

public:
    textBox(String txt, fillMode arr, uint8_t padding, uint16_t color, uint8_t textSize = 2, bool updt = true):
    widget(updt), text(txt), arrange(arr), padding(padding), size(textSize), color(color) {}

    void getSize(tftLCD *tft, int16_t &w, int16_t &h);
    void render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h);
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
