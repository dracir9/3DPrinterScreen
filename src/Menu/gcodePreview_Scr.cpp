
#include "gcodePreview_Scr.h"

GcodePreview_Scr::GcodePreview_Scr(lcdUI* UI)
{
    UI->tft.fillScreen(TFT_BLACK);
    GcodeFile = UI-> selectedFile;
}

void GcodePreview_Scr::update(uint32_t deltaTime)
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

void GcodePreview_Scr::render(tftLCD *tft)
{
    tft->fillRect(pos.x-8, pos.y-8, 16, 16, TFT_BLACK);
    tft->fillCircle(pos.x, pos.y, 5, reColor);
}
