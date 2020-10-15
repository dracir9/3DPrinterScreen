
#ifndef FILE_BROWSER_SCR_H
#define FILE_BROWSER_SCR_H

#include "lcdUI.h"

class lcdUI;

class FileBrowser_Scr final: public Screen
{
public:
    FileBrowser_Scr(lcdUI* UI);

    void update(uint32_t deltaTime);

    void render(tftLCD *tft);

    void printDirectory(File dir, int numTabs);

private:
    lcdUI* _UI;
};

#endif