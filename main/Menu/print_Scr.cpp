/**
 * @file   print_Scr.cpp
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 18-06-2022
 * -----
 * Last Modified: 22-06-2022
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

#include "print_Scr.h"


Print_Scr::Print_Scr(lcdUI* UI, tftLCD& tft, TchScr_Drv& ts):
    Screen(UI)
{
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextPadding(0);
    tft.setTextDatum(CC_DATUM);
    
    tft.drawRoundRect(0, 0, 250, 250, 4, TFT_GREEN);

    uint8_t e = 1;
    tft.setTextColor(TFT_BLACK);
    for (uint8_t i = 0; i < items; i++)
    {
        tft.fillRoundRect(256 + cellAdv*i, 48, cellW, 25, 4, TFT_DARKCYAN);
        tft.fillRoundRect(256 + cellAdv*i, 81, cellW, 25, 4, TFT_DARKCYAN);
        if (i < heatbed)
        {
            tft.drawBmpSPIFFS("/spiffs/heatbed_32.bmp", 256+(cellW-32)/2+cellAdv*i, 8);
        }
        else if (i < tools+heatbed)
        {
            tft.drawBmpSPIFFS("/spiffs/extruder_32.bmp", 256+(cellW-32)/2+cellAdv*i, 8);
            tft.drawString(String(e++), 256+cellW/2+cellAdv*i, 17);
        }
        else
        {
            tft.drawBmpSPIFFS("/spiffs/fan_32.bmp", 256+(cellW-32)/2+cellAdv*i, 8);
        }
    }

    tft.setTextColor(TFT_WHITE);
    // Position
    tft.drawRoundRect(256, 114, 224, 40, 4, TFT_GREEN);

    // Time remaining
    tft.drawString("Time:", 312, 192);
    tft.drawString("--D --:--:--", 312, 208);
    tft.drawString("Remaining:", 424, 192);
    tft.drawString("--D --:--:--", 424, 208);
    tft.drawRoundRect(256, 162, 224, 88, 4, TFT_GREEN);

    // Bottom menu
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

    ts.setButton(&tmpBut); // Move button

    tmpBut.id = 2;
    tmpBut.xmin = 320;
    tmpBut.xmax = 480;

    ts.setButton(&tmpBut); // Config button
}

void Print_Scr::update(const uint32_t deltaTime, TchScr_Drv& ts)
{
    if (!_UI->isSDinit())
    {
        _UI->setScreen(lcdUI::Info);
    }
}

void Print_Scr::render(tftLCD& tft)
{
    _UI->requestUpdate();

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
            tft.drawString(String(Printer::instance()->getBedTemp(), 0) + "`C", 256+cellW/2+cellAdv*i, 60);
            tft.drawString(String(Printer::instance()->getTarBedTemp(), 0) + "`C", 256+cellW/2+cellAdv*i, 93);
        }
        else if (i < tools+heatbed)
        {
            uint8_t k = i - heatbed;
            tft.drawString(String(Printer::instance()->getToolTemp(k), 0) + "`C", 256+cellW/2+cellAdv*i, 60);
            tft.drawString(String(Printer::instance()->getTarToolTemp(k), 0) + "`C", 256+cellW/2+cellAdv*i, 93);
        }
        else
        {
            tft.drawString(String(random(0, 100)) + "%", 256+cellW/2+cellAdv*i, 60);
            tft.drawString(String(random(0, 100)) + "%", 256+cellW/2+cellAdv*i, 93);
        }
    }

    // Draw positions
    Vec3f pos;
    Printer::instance()->getPosition(&pos);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextPadding(56);
    tft.drawString("X:", 284, 127);
    tft.drawString(String(pos.x), 284, 143);
    tft.drawString("Y:", 340, 127);
    tft.drawString(String(pos.y), 340, 143);
    tft.drawString("Z:", 396, 127);
    tft.drawString(String(pos.z), 396, 143);
    tft.drawString("Fr:", 451, 127);
    tft.drawString(String(random(70, 150)) + "%", 451, 143);
}

void Print_Scr::handleTouch(const TchEvent& event)
{
    return;
}
