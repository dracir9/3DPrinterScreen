
#ifndef FILE_BROWSER_SCR_H
#define FILE_BROWSER_SCR_H

#include "lcdUI.h"

class lcdUI;

class FileBrowser_Scr final: public Screen
{
public:
    FileBrowser_Scr(lcdUI* UI);

    void update(uint32_t deltaTime) override;

    void render(tftLCD *tft) override;

    void handleTouch(Screen::touchEvent event, Vector2<int16_t> pos) override;

private:

    void printDirectory(File dir, int numTabs);

    lcdUI* _UI;

    bool draw = false;
    Vector2<int16_t> cursor;
};

#endif