/**
 * @file   control_Scr.cpp
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 10-04-2023
 * -----
 * Last Modified: 10-04-2023
 * Modified By: Ricard Bitriá Ribes
 * -----
 * @copyright (c) 2023 Ricard Bitriá Ribes
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

#include "control_Scr.h"


Control_Scr::Control_Scr(lcdUI * UI, tftLCD & tft, TchScr_Drv & ts):
    Screen(UI)
{
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextPadding(0);
    tft.setTextDatum(CC_DATUM);
    
    tft.setTextColor(TFT_WHITE);

    // Move area
    tft.drawRoundRect(0, 0, 320, 320, 4, TFT_GREEN);
    tft.drawString("Press to move", 160, 160);

    // Joystick
    tft.drawRoundRect(365, 40, 35, 35, 4, TFT_RED);
    tft.drawRoundRect(405, 00, 35, 35, 4, TFT_GREEN);
    tft.drawRoundRect(405, 40, 35, 35, 4, TFT_WHITE);
    tft.drawRoundRect(405, 80, 35, 35, 4, TFT_GREEN);
    tft.drawRoundRect(445, 00, 35, 35, 4, TFT_BLUE);
    tft.drawRoundRect(445, 40, 35, 35, 4, TFT_RED);
    tft.drawRoundRect(445, 80, 35, 35, 4, TFT_BLUE);
    tft.drawBmpSPIFFS("/spiffs/home_24.bmp", 410, 45);

    tft.drawRoundRect(325, 0, 35, 264, 4, TFT_BLUE);

    // Return button
    tft.drawRoundRect(320, 270, 160, 50, 4, TFT_ORANGE);
    tft.drawBmpSPIFFS("/spiffs/return_48.bmp", 376, 277);

    Button tmpBut;
    tmpBut.id = 0;
    tmpBut.xmin = 320;
    tmpBut.xmax = 480;
    tmpBut.ymin = 270;
    tmpBut.ymax = 320;
    tmpBut.enReleaseEv = true;
    
    ts.setButton(&tmpBut); // Back to Info screen
}

void Control_Scr::update(uint32_t deltaTime, TchScr_Drv &ts)
{
}

void Control_Scr::render(tftLCD &tft)
{
}

void Control_Scr::handleTouch(const TchEvent& event)
{
    if (event.trigger == TrgSrc::RELEASE && event.id == 0)
    {
        _UI->setScreen(lcdUI::INFO_SCR);
    }
}
