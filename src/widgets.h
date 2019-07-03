#ifndef WIDGETS_H
#define WIDGETS_H

#include <Arduino.h>
//#include <Adafruit>
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
    ~canvas(){ delete child;}

    void getSize(tftLCD *tft, int16_t &w, int16_t &h);
    void render(tftLCD *tft, int16_t x=0, int16_t y=0, int16_t w=0, int16_t h=0);
    void attachComponent(widget *chld);
    void clear();
};

/**************************************************************************
    Arrange widgets vertically
**************************************************************************/
class verticalBox : public widget
{
protected:
    uint8_t elNum = 1;
    widget **child = NULL;

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
        if (child)
        {
            for (uint8_t i = 0; i < elNum; i++)
            {
                if(child[i]) delete child[i];
                child[i] = NULL;
            }
            delete[] child;
            child = NULL;
        } 
    }

    void getSize(tftLCD *tft, int16_t &w, int16_t &h);
    void render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h);
    bool attachComponent(widget *chld);
};

class textBox : public widget
{
protected:
    String text;
    fillMode arrange = fillMode::CenterCenter;
    uint8_t padding = 8;
    uint8_t size = 1;
    uint16_t color = TFT_WHITE;
    const GFXfont *font = NULL;

public:
    textBox(String txt, fillMode arr = fillMode::CenterCenter, uint8_t padding = 8, uint16_t color = TFT_WHITE,
                                            const GFXfont *font = NULL, uint8_t textSize = 2, bool updt = true):
    widget(updt), text(txt), arrange(arr), padding(padding), size(textSize), color(color), font(font) {}

    void getSize(tftLCD *tft, int16_t &w, int16_t &h);
    void render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h);
    void updateText(String txt);
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
