
#include "textBox.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "textBox"

/********************************************************************************
    Text Box 
********************************************************************************/
Vec2h textBox::getSize(tftLCD *tft) const
{
    ESP_LOGV(TAG, "text Box getSize start\n");

    tft->setTextSize(size);
    if (font) tft->setFreeFont(font);
    else tft->setTextFont(GLCD);
    Vec2h size = tft->getTextBounds(*text);
    size.x = max(paddingX, size.x);
    size.y = max(paddingY, size.y);
    return arrangeSize(size, arrange);
 
    ESP_LOGV(TAG, "textBox getSize end\n");
}

void textBox::draw(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h) const
{
    ESP_LOGV(TAG, "text Box render start\n");

    Vec2h dim = tft->getTextBounds(*text);
    dim.x = max(paddingX, dim.x);
    dim.y = max(paddingY, dim.y);
    tft->img.setColorDepth(1);
    tft->img.createSprite(dim.x, dim.y);

    tft->img.setTextSize(size);
    tft->img.setTextColor(txtcolor);
    if (font) tft->img.setFreeFont(font);
    else tft->img.setTextFont(GLCD);
    tft->img.setCursor(dim.x/2, dim.y/2);
    tft->img.printCenter(*text);

    tft->img.setBitmapColor(txtcolor, bgcolor);
    tft->img.pushSprite(x+(w-dim.x)/2, y+(h-dim.y)/2);

    tft->img.deleteSprite();

#ifdef DEBUG_LINES
    Vec2h size = tft->getTextBounds(*text);
    tft->drawRect(x, y, w, h, TFT_RED);
    tft->drawRect(x+(w-size.x)/2, y+(h-size.y)/2, size.x, size.y, TFT_BLUE);
#endif

    ESP_LOGV(TAG, "textBox render end\n");
}
