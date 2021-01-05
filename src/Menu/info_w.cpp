
#include "info_w.h"

Info_W::Info_W(lcdUI* UI):
    Screen(UI)
{
    _UI->tft.setTextFont(2);
    _UI->tft.setTextSize(1);
    _UI->tft.setTextPadding(0);

    _UI->tft.fillRect(0, 0, 480, 32, TFT_RED);
    _UI->tft.setTextDatum(CC_DATUM);
    _UI->tft.setTextColor(TFT_WHITE);
    _UI->tft.drawString("Hello world!", 240, 16);
    
    _UI->tft.fillRoundRect(0, 73, 66, 25, 4, TFT_NAVY);
    _UI->tft.setTextColor(TFT_WHITE);
    _UI->tft.drawString("Current", 33, 85);
    _UI->tft.fillRoundRect(0, 106, 66, 25, 4, TFT_NAVY);
    _UI->tft.drawString("Target", 33, 118);
    _UI->tft.setTextColor(TFT_BLACK);
    _UI->tft.setTextDatum(CC_DATUM);
    for (uint8_t i = 0; i < tools; i++)
    {
        _UI->tft.fillRoundRect(74 + 138*i, 40, 130, 25, 4, TFT_WHITE);
        _UI->tft.fillRoundRect(74 + 138*i, 73, 130, 25, 4, TFT_DARKCYAN);
        _UI->tft.fillRoundRect(74 + 138*i, 106, 130, 25, 4, TFT_DARKCYAN);
        if(i==0)
        {
            _UI->tft.drawString("HB", 138, 52);
        }
        else
        {
            _UI->tft.drawString("E" + String(i), 138+138*i, 52);
        }
    }
    _UI->tft.drawRoundRect(0, 40, 480, 92, 4, TFT_GREEN);
    _UI->tft.drawRoundRect(0, 140, 480, 25, 4, TFT_GREEN);

    _UI->tft.drawRoundRect(0, 173, 480, 89, 4, TFT_GREEN);
    _UI->tft.drawRoundRect(0, 270, 155, 50, 4, TFT_ORANGE);
    _UI->tft.drawRoundRect(163, 270, 154, 50, 4, TFT_ORANGE);
    _UI->tft.drawRoundRect(325, 270, 155, 50, 4, TFT_ORANGE);
}

/**************************************************************************/
/*!
    @brief  Return Top-Left coordinates of a cell (Not constrained tot the actual size of the grid)
    @param  x    Where the X coordinate will be stored
    @param  y    Where the Y coordinate will be stored
    @param  c    Column index
    @param  r    Row index
*/
/**************************************************************************/
void grid::getTLPoint(int16_t *x, int16_t *y, uint8_t c, uint8_t r) const
{
    *x = gridX + cellW*c + space*c;
    *y = gridY + cellH*r + space*r;
}

/**************************************************************************/
/*!
    @brief  Return center coordinates of a cell (Not constrained tot the actual size of the grid)
    @param  x    Where the X coordinate will be stored
    @param  x    Where the Y coordinate will be stored
    @param  c    Column index
    @param  r    Row index
*/
/**************************************************************************/
void grid::getCenterPoint(int16_t *x, int16_t *y, uint8_t c, uint8_t r) const
{
    *x = gridX + cellW*c + space*c + cellW/2;
    *y = gridY + cellH*r + space*r + cellH/2;
}

void Info_W::update(uint32_t deltaTime)
{
    
}

void Info_W::render(tftLCD *tft)
{
    if (millis() < nextP) return;
    tft->setTextFont(2);
    tft->setTextSize(1);
    tft->setTextDatum(CC_DATUM);
    tft->setTextPadding(100);

    // Draw update
    tft->setTextColor(TFT_WHITE, TFT_DARKCYAN);
    for (uint8_t i = 0; i < tools; i++)
    {
        tft->drawString(String(random(250)), 139+138*i, 85);
        tft->drawString(String(random(70)), 139+138*i, 118);
    }
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    //tft->fillRoundRect(1, 101, tft->width()-2, 23, 4, TFT_BLACK);
    tft->drawString("X: " + String(random(250)), 60, 152);
    tft->drawString("Y: " + String(random(210)), 180, 152);
    tft->drawString("Z: " + String(random(200)), 300, 152);
    tft->drawString("Fr: " + String(random(150)) + "%", 420, 152);
    
    nextP = millis() + 1000;
}
