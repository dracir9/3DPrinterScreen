
#include "fileBrowser_Scr.h"

FileBrowser_Scr::FileBrowser_Scr(lcdUI* UI)
{
    UI->tft.fillScreen(TFT_BLACK);
    _UI = UI;
/*     tft->drawRect(0, 0, 480, 70, TFT_RED);
    tft->setCursor(240, 35);
    tft->setTextFont(2);
    tft->setTextSize(2);
    tft->setTextDatum(CC_DATUM);
    tft->print("Test text");
    tft->drawRect(0, 70, 250, 250, TFT_GREEN);
    tft->drawRect(250, 70, 230, 70, TFT_BLUE); */
}

void FileBrowser_Scr::update(uint32_t deltaTime)
{
    if (_UI->checkSD() && SD.exists("/test.gcode"))
    {
        _UI->selectedFile = SD.open("/test.gcode");
        _UI->setScreen(_UI->GcodePreview);
    }
}

void FileBrowser_Scr::render(tftLCD *tft)
{
    tft->setCursor(240, 160);
    tft->setTextSize(2);
    if (_UI->checkSD())
    {
        tft->print("SD connected!  ");
        if (!SD.exists("/test.gcode")) tft->print("\ntest.gcode not found :(");
    }
    else
    {
        tft->print("SD not found :(");
    }
}