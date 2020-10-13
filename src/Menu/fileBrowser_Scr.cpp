
#include "fileBrowser_Scr.h"

FileBrowser_Scr::FileBrowser_Scr(tftLCD* tft)
{
    tft->fillScreen(TFT_BLACK);
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
    pos += vel * ((float)deltaTime/1000000.0f);
    if((pos.x > 474.0f && vel.x > 0) || (pos.x < 5.0f && vel.x < 0))
    {
        vel.x = -vel.x;
    }

    if((pos.y > 314.0f && vel.y > 0) || (pos.y < 5.0f && vel.y < 0))
    {
        vel.y = -vel.y;
    }

    reColor++;
}

void FileBrowser_Scr::render(tftLCD *tft)
{
    tft->fillRect(pos.x-8, pos.y-8, 16, 16, TFT_BLACK);
    tft->fillCircle(pos.x, pos.y, 5, reColor);
}