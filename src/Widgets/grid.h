
#ifndef GRID_H
#define GRID_H

#include <Arduino.h>
#include "widgets.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "grid"

template<uint8_t COL, uint8_t ROW>
class grid : public widget
{
protected:
    widget *child[COL * ROW];

    uint8_t space = 8;
    int16_t cellW = 25;
    int16_t cellH = 25;

public:
    grid(bool updt = true);
    ~grid();

    vector2<int16_t> getSize(tftLCD *tft);
    void render(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h);
};

/********************************************************************************
    Grid implementation
********************************************************************************/

template<uint8_t COL, uint8_t ROW>
grid<COL, ROW>::grid(bool updt = true):
    widget(updt)
{
    for (uint8_t i = 0; i < COL*ROW; i++)
    {
        child[i] = NULL;
    }
}

template<uint8_t COL, uint8_t ROW>
grid<COL, ROW>::~grid()
{
    ESP_LOGV(TAG, "Delete verticalBox\n");
}

template<uint8_t COL, uint8_t ROW>
vector2<int16_t> grid<COL, ROW>::getSize(tftLCD *tft)
{
    return vector2<int16_t>();
}

#endif
