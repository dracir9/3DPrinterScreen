/**
 * @file   info_w.cpp
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 22-01-2022
 * -----
 * Last Modified: 15-06-2022
 * Modified By: Ricard Bitriá Ribes
 * -----
 * @copyright (c) 2022 Ricard Bitriá Ribes
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "info_Scr.h"

Info_Scr::Info_Scr(lcdUI* UI, tftLCD& tft, TchScr_Drv& ts):
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

    // Setup buttons
    Button tmpBut;
    tmpBut.id = 0;
    tmpBut.xmin = 0;
    tmpBut.xmax = 160;
    tmpBut.ymin = 256;
    tmpBut.ymax = 320;
    tmpBut.enReleaseEv = true;

    ts.setButton(&tmpBut); // SD card button

    tmpBut.id = 1;
    tmpBut.xmin = 160;
    tmpBut.xmax = 320;
    tmpBut.enHoldEv = true;
    tmpBut.holdTime = 100;

    ts.setButton(&tmpBut); // Move button

    tmpBut.id = 2;
    tmpBut.xmin = 320;
    tmpBut.xmax = 480;
    tmpBut.enHoldEv = false;

    ts.setButton(&tmpBut); // Config button
}

void Info_Scr::update(uint32_t deltaTime, TchScr_Drv& ts)
{
    
}

void Info_Scr::render(tftLCD& tft)
{
    _UI->requestUpdate();
    if (millis() < nextP) return;
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextDatum(CC_DATUM);
    tft.setTextPadding(cellW - 4);

    // Draw temperatures
    tft.setTextColor(TFT_WHITE, TFT_DARKCYAN);
    for (uint8_t i = 0; i < items; i++)
    {
        if (i < heatbed)
        {
            tft.drawString(String(Printer::instance()->getBedTemp(), 1) + "`C", 72+cellW/2+cellAdv*i, 70);
            tft.drawString(String(Printer::instance()->getTarBedTemp(), 0) + "`C", 72+cellW/2+cellAdv*i, 103);
        }
        else if (i < tools+heatbed)
        {
            uint8_t k = i - heatbed;
            tft.drawString(String(Printer::instance()->getToolTemp(k), 1) + "`C", 72+cellW/2+cellAdv*i, 70);
            tft.drawString(String(Printer::instance()->getTarToolTemp(k), 0) + "`C", 72+cellW/2+cellAdv*i, 103);
        }
        else
        {
            tft.drawString(String(random(0, 100)) + "%", 72+cellW/2+cellAdv*i, 70);
            tft.drawString(String(random(0, 100)) + "%", 72+cellW/2+cellAdv*i, 103);
        }
    }

    // Draw positions
    Vec3f pos;
    Printer::instance()->getPosition(&pos);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextPadding(115);
    tft.drawString("X: " + String(pos.x), 60, 137);
    tft.drawString("Y: " + String(pos.y), 180, 137);
    tft.drawString("Z: " + String(pos.z), 300, 137);
    tft.drawString("Fr: " + String(random(70, 150)) + "%", 420, 137);
    
    nextP = millis() + 1000;
}

void Info_Scr::handleTouch(const TchEvent& event)
{
    if (event.trigger != TrgSrc::RELEASE) return;
    if (event.id == 0)
    {
        if (_UI->isSDinit()) _UI->setScreen(lcdUI::FileBrowser);
    }
    else if (event.id == 1)
    {
        /* code */
    }
    else if (event.id == 2)
    {
        _UI->setScreen(lcdUI::Config);
    }
}
