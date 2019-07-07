#ifndef WIDGETS_H
#define WIDGETS_H

#define DEBUG_LINES

#include <Arduino.h>
#include <Adafruit_ImageReader.h>
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

    virtual vector2<int16_t> getSize(tftLCD *tft) = 0;
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
    canvas(bool updt=true);
    ~canvas();

    virtual vector2<int16_t> getSize(tftLCD *tft);
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
    verticalBox(uint8_t elem, bool updt = true);
    ~verticalBox();

    virtual vector2<int16_t> getSize(tftLCD *tft);
    void render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h);
    bool attachComponent(widget *chld);
};

/**************************************************************************
    Arrange widgets horizontally
**************************************************************************/
class horizontalBox : public widget
{
protected:
    uint8_t elNum = 1;
    widget **child = NULL;

public:
    horizontalBox(uint8_t elem, bool updt = true);
    ~horizontalBox();

    virtual vector2<int16_t> getSize(tftLCD *tft);
    void render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h);
    bool attachComponent(widget *chld);
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

    virtual vector2<int16_t> getSize(tftLCD *tft);
    void render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h);
    bool attachComponent(widget *chld);
};

/**************************************************************************
    Draw text
**************************************************************************/
class textBox : public widget
{
protected:
    String *text;
    fillMode arrange = fillMode::CenterCenter;
    uint8_t padding = 8;
    uint8_t size = 1;
    uint16_t txtcolor = TFT_WHITE;
    uint16_t bgcolor = TFT_BLACK;
    const GFXfont *font = NULL;

public:
    textBox(String *txt, fillMode arr = fillMode::CenterCenter, uint8_t padding = 8, uint16_t color = TFT_WHITE,
                                            const GFXfont *font = NULL, uint8_t textSize = 2, bool updt = true):
    widget(updt), text(txt), arrange(arr), padding(padding), size(textSize), txtcolor(color), font(font) {}

    virtual vector2<int16_t> getSize(tftLCD *tft);
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
