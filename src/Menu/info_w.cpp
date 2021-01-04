
#include "info_w.h"

Info_W::Info_W(lcdUI* UI):
    Screen(UI)
{
    ESP_LOGV(__FILE__, "Create info\n");
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
    const uint8_t e=3;

    int16_t x = 0;
    int16_t y = 0;
    uint16_t w = 56;
    uint16_t h = 16;

    tft->setTextFont(0);
    tft->setTextSize(2);
    //tft->getTextBounds(String("Current"), 0, 0, &x, &y, &w, &h);
    const grid temp(w+18, 40, tft->width()-w-18, 91, e, 3, 8);
    temp.getTLPoint(&x, &y, 0, 3);
    const grid position(0, y, tft->width(), temp.cellH, 4, 1, 8);
    position.getTLPoint(&x, &y, 0, 1);
    const grid info(0, y, tft->width(), tft->height()-y, 4, 2, 8);

   
    // First draw
    if (init)
    {
        // Header
        tft->fillRect(0, 0, 480, 32, TFT_RED);
        tft->setTextDatum(TL_DATUM);
        tft->setTextColor(TFT_WHITE, TFT_RED);
        tft->drawString("Hello world!", 5, 10);
        // End Header

        // Temperature header
        tft->setTextColor(TFT_BLACK);
        for (uint8_t i = 0; i < e; i++)
        {
            temp.getTLPoint(&x, &y, i, 0);
            tft->fillRoundRect(x, y, temp.cellW, temp.cellH, 4, TFT_WHITE);
            temp.getCenterPoint(&x, &y, i, 0);
            tft->setTextDatum(CC_DATUM);
            if(i==0)
            {
                tft->drawString("HB", x, y);
            }
            else
            {
                tft->drawString("E", x, y);
            }
        }
        // End Temperature header

        // Temperatures
        temp.getTLPoint(&x, &y, 0, 1);
        tft->fillRoundRect(0, y, temp.gridX-temp.space, temp.cellH, 4, TFT_NAVY);
        temp.getCenterPoint(&x, &y, 0, 1);
        tft->setCursor(5+w/2, y);
        tft->setTextColor(TFT_WHITE);
        tft->printCenter(String("Current"));


        temp.getTLPoint(&x, &y, 0, 2);
        tft->fillRoundRect(0, y, temp.gridX-temp.space, temp.cellH, 4, TFT_NAVY);
        temp.getCenterPoint(&x, &y, 0, 2);
        tft->setCursor(5+w/2, y);
        tft->printCenter(String("Target"));
                
        temp.getTLPoint(&x, &y, 0, 0);
        tft->drawRoundRect(0, y, tft->width(), 92, 4, TFT_GREEN);
        // End Temperatures

        // Position
        position.getTLPoint(&x, &y, 0, 0);
        tft->drawRoundRect(0, y, tft->width(), position.cellH, 4, TFT_GREEN);
        info.getTLPoint(&x, &y, 0, 0);
        tft->drawRoundRect(0, y, tft->width(), tft->height()-y, 4, TFT_GREEN);
        tft->setTextSize(3);
        info.getCenterPoint(&x, &y, 0, 0);
        tft->setCursor(x, y);
        tft->printCenter("SD");
        tft->setTextSize(2);
        for (uint8_t c = 0; c < 4; c++)
        {
            for (uint8_t r = 0; r < 4; r++)
            {
                info.getTLPoint(&x, &y, c, r);
                tft->drawRect(x, y, info.cellW, info.cellH, TFT_RED);
            }
        }
        init = false;
    }


    // Draw update
    if (millis()%1000 == 0)
    {
        for (uint8_t i = 0; i < e; i++)
        {
            temp.getTLPoint(&x, &y, i, 1);
            tft->fillRoundRect(x, y, temp.cellW, temp.cellH, 4, TFT_DARKCYAN);
            temp.getCenterPoint(&x, &y, i, 1);
            tft->setCursor(x,y);
            tft->printCenter(String(random(250))+String(char(247)));
            temp.getTLPoint(&x, &y, i, 2);
            tft->fillRoundRect(x, y, temp.cellW, temp.cellH, 4, TFT_DARKCYAN);
            temp.getCenterPoint(&x, &y, i, 2);
            tft->setCursor(x,y);
            tft->printCenter(String(random(70))+String(char(247)));
        }
        
        position.getTLPoint(&x, &y, 0, 0);
        tft->fillRoundRect(1, y+1, tft->width()-2, position.cellH-2, 4, TFT_BLACK);
        position.getCenterPoint(&x, &y, 0, 0);
        tft->setCursor(x,y);
        tft->printCenter(String("X:") + String(random(250)));
        position.getCenterPoint(&x, &y, 1, 0);
        tft->setCursor(x,y);
        tft->printCenter(String("Y:") + String(random(210)));
        position.getCenterPoint(&x, &y, 2, 0);
        tft->setCursor(x,y);
        tft->printCenter(String("Z:") + String(random(200)));
        position.getCenterPoint(&x, &y, 3, 0);
        tft->setCursor(x,y);
        tft->printCenter(String("F_R:") + String(random(150)) + String("%"));
    }
}
