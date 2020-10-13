
#ifndef GCODEPREVIEW_SCR_H
#define GCODEPREVIEW_SCR_H

#include "lcdUI.h"

class lcdUI;

class GcodePreview_Scr final : public Screen
{
public:
    GcodePreview_Scr(lcdUI* UI);

    void update(uint32_t deltaTime);

    void render(tftLCD *tft);

private:
    File GcodeFile;

    Vector2<float> pos;
    Vector2<float> vel = Vector2<float>(100,100);

    uint16_t reColor = 0;
};

#endif