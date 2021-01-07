
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
    
    _UI->tft.setTextColor(TFT_BLACK);
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
    _UI->tft.fillRoundRect(0, 73, 66, 25, 4, TFT_NAVY);
    _UI->tft.setTextColor(TFT_WHITE);
    _UI->tft.drawString("Current", 33, 85);
    _UI->tft.fillRoundRect(0, 106, 66, 25, 4, TFT_NAVY);
    _UI->tft.drawString("Target", 33, 118);
    _UI->tft.drawRoundRect(0, 40, 480, 92, 4, TFT_GREEN);

    _UI->tft.drawRoundRect(0, 140, 480, 25, 4, TFT_GREEN);

    _UI->tft.drawString("Time:", 40, 202);
    _UI->tft.drawString("--D --:--:--", 40, 218);
    _UI->tft.drawString("Remaining:", 440, 202);
    _UI->tft.drawString("--D --:--:--", 440, 218);
    _UI->tft.drawRoundRect(0, 173, 480, 75, 4, TFT_GREEN);

    _UI->tft.drawBmpSPIFFS("/spiffs/SDcard_64.bmp", 45, 256);
    _UI->tft.drawRoundRect(0, 256, 155, 64, 4, TFT_ORANGE);
    _UI->tft.drawBmpSPIFFS("/spiffs/Move_48.bmp", 216, 264);
    _UI->tft.drawRoundRect(163, 256, 154, 64, 4, TFT_ORANGE);
    _UI->tft.drawBmpSPIFFS("/spiffs/Settings_48.bmp", 379, 264);
    _UI->tft.drawRoundRect(325, 256, 155, 64, 4, TFT_ORANGE);
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
        tft->drawString(String(random(250)) + "`C", 139+138*i, 85);
        tft->drawString(String(random(70)) + "`C", 139+138*i, 118);
    }
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    //tft->fillRoundRect(1, 101, tft->width()-2, 23, 4, TFT_BLACK);
    tft->drawString("X: " + String(random(250)), 60, 152);
    tft->drawString("Y: " + String(random(210)), 180, 152);
    tft->drawString("Z: " + String(random(200)), 300, 152);
    tft->drawString("Fr: " + String(random(150)) + "%", 420, 152);
    
    nextP = millis() + 1000;
}

void Info_W::handleTouch(const touchEvent event, const Vec2h pos)
{
    if (event == press)
    {
        if (pos.y > 256)
        {
            if (pos.x < 160)
            {
                if (_UI->checkSD()) _UI->setScreen(lcdUI::FileBrowser);
            }
            else if (pos.x < 320)
            {

            }
            else
            {

            }
        }
    }
}
