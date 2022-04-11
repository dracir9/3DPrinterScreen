/**
 * @file   display_conf_Scr.cpp
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

#include "displayConf_Scr.h"
DisplayConf_Scr::DisplayConf_Scr(lcdUI* UI, tftLCD& tft, TchScr_Drv& ts):
    Screen(UI)
{
    tft.fillScreen(TFT_BLACK);

    // Bottom menu
    tft.drawBmpSPIFFS("/spiffs/return_48.bmp", 53, 272);
    tft.drawRoundRect(0, 256, 155, 64, 4, TFT_ORANGE);

    // Slider
    tft.drawRoundRect(0, 51, 239, 48, 4, TFT_OLIVE);

    Button tmpBut;
    tmpBut.id = 0;
    tmpBut.xmin = 0;
    tmpBut.xmax = 160;
    tmpBut.ymin = 256;
    tmpBut.ymax = 320;
    tmpBut.enReleaseEv = true;
    
    ts.setButton(&tmpBut); // Back to Config screen

    tmpBut.id = 1;
    tmpBut.enHoldEv = true;
    tmpBut.enReleaseEv = false;
    tmpBut.holdTime = 0;
    tmpBut.xmin = 0;
    tmpBut.xmax = 240;
    tmpBut.ymin = 50;
    tmpBut.ymax = 100;

    ts.setButton(&tmpBut); // Brightness slider
    ts.setNotifications(true, true, true);
}

void DisplayConf_Scr::update(uint32_t deltaTime, TchScr_Drv& ts)
{
    
}

void DisplayConf_Scr::render(tftLCD& tft)
{
    if (engaged)
    {
        tft.fillRect(20, 55, 200, 40, TFT_BLACK);
        tft.fillRoundRect(sliderX, 55, 10, 40, 4, TFT_WHITE);
    }
}

void DisplayConf_Scr::handleTouch(const TchEvent& event)
{
    if (event.trigger == TrgSrc::RELEASE)
    {
        if (event.id == 0)
        {
            _UI->setScreen(lcdUI::Config);
        }
    }
    else if (event.trigger == TrgSrc::HOLD_STRT)
    {
        engaged = true;
    }
    else if (event.trigger == TrgSrc::HOLD_END)
    {
        engaged = false;
    }
    else if (event.trigger == TrgSrc::IDLE)
    {
        sliderX = event.pos.x;
        if (sliderX > 210)
        {
            sliderX = 210;
        }
        else if (sliderX < 20)
        {
            sliderX = 20;
        }
        _UI->requestUpdate();
    }
}
