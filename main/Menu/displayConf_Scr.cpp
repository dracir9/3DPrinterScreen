/**
 * @file   displayConf_Scr.cpp
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 11-04-2022
 * -----
 * Last Modified: 24-08-2023
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
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextDatum(CL_DATUM);
    tft.setTextPadding(0);
    tft.drawString("Display Brightness:", 10, 75);
    tft.drawRoundRect(0, 51, 239, 48, 4, TFT_OLIVE);
    sliderX = _UI->getBrightness()*190/255;
    tft.drawRoundRect(241, 51, 239, 48, 4, TFT_OLIVE);

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
    tmpBut.enHoldTickEv = true;
    tmpBut.enReleaseEv = false;
    tmpBut.holdTime = 0;
    tmpBut.xmin = 261;
    tmpBut.xmax = 461;
    tmpBut.ymin = 50;
    tmpBut.ymax = 100;

    ts.setButton(&tmpBut); // Brightness slider
}

void DisplayConf_Scr::update(uint32_t deltaTime, TchScr_Drv& ts)
{
    _UI->setBrightness(sliderX*255/190);
}

void DisplayConf_Scr::render(tftLCD& tft)
{
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextDatum(CR_DATUM);
    tft.setTextPadding(40);
    tft.drawString(String(sliderX*100/190) + String(" %"), 229, 75);

    tft.fillRect(261, 55, 200, 40, TFT_BLACK);
    tft.fillRect(266, 74, 190, 2, TFT_WHITE);
    tft.fillRoundRect(261+sliderX, 55, 10, 40, 4, TFT_WHITE);
}

void DisplayConf_Scr::handleTouch(const TchEvent& event)
{
    if (event.trigger == TrgSrc::RELEASE)
    {
        if (event.id == 0)
        {
            _UI->setScreen(lcdUI::CONFIG_MENU_SCR);
        }
    }
    else if (event.trigger == TrgSrc::HOLD_TICK)
    {
        sliderX = event.pos.x - 266;
        if (sliderX > 190)
        {
            sliderX = 190;
        }
        else if (sliderX < 0)
        {
            sliderX = 0;
        }
        _UI->requestUpdate();
    }
}
