
#include "textBox.h"

/********************************************************************************
    Text Box 
********************************************************************************/
vector2<int16_t> textBox::getSize(tftLCD *tft) const
{
#ifdef DEBUG_MODE
    Serial.println("text Box get Size start");
#endif

    tft->setTextSize(size);
    if (font) tft->setFreeFont(font);
    else tft->setTextFont(GLCD);
    vector2<int16_t> size = tft->getTextBounds(*text);
    size.x = max(paddingX, size.x);
    size.y = max(paddingY, size.y);
    return arrangeSize(size, arrange);
 

#ifdef DEBUG_MODE
    Serial.println("text Box get Size end");
#endif
}

void textBox::draw(tftLCD *tft, int16_t x, int16_t y, int16_t w, int16_t h) const
{
#ifdef DEBUG_MODE
    Serial.println("text Box render start");
#endif
    vector2<int16_t> dim = tft->getTextBounds(*text);
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
    vector2<int16_t> size = tft->getTextBounds(*text);
    tft->drawRect(x, y, w, h, TFT_RED);
    tft->drawRect(x+(w-size.x)/2, y+(h-size.y)/2, size.x, size.y, TFT_BLUE);
#endif

#ifdef DEBUG_MODE
    Serial.println("text Box render end");
#endif
}
