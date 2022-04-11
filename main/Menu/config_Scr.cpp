/**
 * @file   config_Scr.cpp
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 11-04-2022
 * -----
 * Last Modified: 11-04-2022
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

#include "config_Scr.h"

Config_Scr::Config_Scr(lcdUI* UI, tftLCD& tft, TchScr_Drv& ts):
    Screen(UI)
{
    tft.fillScreen(TFT_BLACK);

    tft.drawBmpSPIFFS("/spiffs/return_48.bmp", 53, 272);
    tft.drawRoundRect(0, 256, 155, 64, 4, TFT_ORANGE);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextDatum(CL_DATUM);
    tft.setTextPadding(202);
    tft.drawString("Brightness", 10, 75);
    tft.drawRoundRect(0, 51, 239, 48, 4, TFT_OLIVE);

    Button tmpBut;
    tmpBut.id = 0;
    tmpBut.xmin = 0;
    tmpBut.xmax = 160;
    tmpBut.ymin = 256;
    tmpBut.ymax = 320;
    tmpBut.enReleaseEv = true;
    
    ts.setButton(&tmpBut); // Back to Info screen

    for (int i = 0; i < 2; i++) // Menu buttons
    {
        for (int j = 0; j < 4; j++)
        {
            tmpBut.id = j + i*4 + 1;
            tmpBut.xmin = i * 240;
            tmpBut.xmax = i * 240 + 240;
            tmpBut.ymin = 50 * j + 50;
            tmpBut.ymax = 50 * j + 100;
            ts.setButton(&tmpBut);
        }
    }
}

void Config_Scr::update(uint32_t deltaTime, TchScr_Drv& ts)
{
    
}

void Config_Scr::render(tftLCD& tft)
{
    
}

void Config_Scr::handleTouch(const TchEvent& event)
{
    if (event.trigger != TrgSrc::RELEASE) return;
    if (event.id == 0)
    {
        _UI->setScreen(lcdUI::Info);
    }
    else if(event.id == 1)
    {

    }
}

