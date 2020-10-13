
#ifndef FILE_BROWSER_SCR_H
#define FILE_BROWSER_SCR_H

#include "widgets.h"

class FileBrowser_Scr final: public Screen
{
public:
    FileBrowser_Scr(tftLCD* tft);

    void update(uint32_t deltaTime);

    void render(tftLCD *tft);

private:
    Vector2<float> pos;
    Vector2<float> vel = Vector2<float>(100,100);

    uint16_t reColor = 0;
};

#endif