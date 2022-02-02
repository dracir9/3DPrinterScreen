
#include "info_w.h"

Info_W::Info_W(lcdUI* UI, tftLCD& tft):
    Screen(UI), cellAdv(416.0f/(items)), cellW(cellAdv-8)
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextPadding(0);
    tft.setTextDatum(CC_DATUM);
    
    tft.setTextColor(TFT_BLACK);
    uint8_t e = 1;
    for (uint8_t i = 0; i < items; i++)
    {
        tft.fillRoundRect(72 + cellAdv*i, 58, cellW, 25, 4, TFT_DARKCYAN);
        tft.fillRoundRect(72 + cellAdv*i, 91, cellW, 25, 4, TFT_DARKCYAN);
        if (i < heatbed)
        {
            tft.drawBmpSPIFFS("/spiffs/heatbed_32.bmp", 72+(cellW-32)/2+cellAdv*i, 13);
        }
        else if (i < tools+heatbed)
        {
            tft.drawBmpSPIFFS("/spiffs/extruder_32.bmp", 72+(cellW-32)/2+cellAdv*i, 13);
            if (tools > 1) tft.drawString(String(e++), 72+cellW/2+cellAdv*i, 22);
        }
        else
        {
            tft.drawBmpSPIFFS("/spiffs/fan_32.bmp", 72+(cellW-32)/2+cellAdv*i, 13);
        }
    }
    tft.fillRoundRect(0, 58, 64, 25, 4, TFT_NAVY);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("Current", 32, 70);
    tft.fillRoundRect(0, 91, 64, 25, 4, TFT_NAVY);
    tft.drawString("Target", 32, 103);
    tft.drawRoundRect(0, 0, 480, 117, 4, TFT_GREEN);

    tft.drawRoundRect(0, 125, 480, 25, 4, TFT_GREEN);

    tft.drawString("Time:", 40, 195);
    tft.drawString("--D --:--:--", 40, 211);
    tft.drawString("Remaining:", 440, 195);
    tft.drawString("--D --:--:--", 440, 211);
    tft.drawRoundRect(0, 158, 480, 90, 4, TFT_GREEN);

    tft.drawBmpSPIFFS("/spiffs/SDcard_64.bmp", 45, 264);
    tft.drawRoundRect(0, 256, 155, 64, 4, TFT_ORANGE);
    tft.drawBmpSPIFFS("/spiffs/move_48.bmp", 216, 264);
    tft.drawRoundRect(163, 256, 154, 64, 4, TFT_ORANGE);
    tft.drawBmpSPIFFS("/spiffs/settings_48.bmp", 378, 264);
    tft.drawRoundRect(325, 256, 155, 64, 4, TFT_ORANGE);
}

void Info_W::update(uint32_t deltaTime)
{
    
}

void Info_W::render(tftLCD& tft)
{
    if (millis() < nextP) return;
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextDatum(CC_DATUM);
    tft.setTextPadding(40);

    // Draw temperatures
    tft.setTextColor(TFT_WHITE, TFT_DARKCYAN);
    for (uint8_t i = 0; i < items; i++)
    {
        if (i < heatbed)
        {
            tft.drawString(String(random(50, 70)) + "`C", 72+cellW/2+cellAdv*i, 70);
            tft.drawString(String(random(50, 70)) + "`C", 72+cellW/2+cellAdv*i, 103);
        }
        else if (i < tools+heatbed)
        {
            tft.drawString(String(random(180, 250)) + "`C", 72+cellW/2+cellAdv*i, 70);
            tft.drawString(String(random(180, 250)) + "`C", 72+cellW/2+cellAdv*i, 103);
        }
        else
        {
            tft.drawString(String(random(0, 100)) + "%", 72+cellW/2+cellAdv*i, 70);
            tft.drawString(String(random(0, 100)) + "%", 72+cellW/2+cellAdv*i, 103);
        }
    }

    // Draw positions
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextPadding(115);
    tft.drawString("X: " + String(random(-100, 500)), 60, 137);
    tft.drawString("Y: " + String(random(-100, 500)), 180, 137);
    tft.drawString("Z: " + String(random(-100, 500)), 300, 137);
    tft.drawString("Fr: " + String(random(70, 150)) + "%", 420, 137);
    
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
                if (_UI->isSDinit()) _UI->setScreen(lcdUI::FileBrowser);
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
