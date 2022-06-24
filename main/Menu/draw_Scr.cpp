/**
 * @file   draw_Scr.cpp
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 24-06-2022
 * -----
 * Last Modified: 24-06-2022
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

#include "draw_Scr.h"


Draw_Scr::Draw_Scr(lcdUI* UI, tftLCD& tft, TchScr_Drv& ts):
    Screen(UI)
{
    tft.drawRoundRect(0, 0, 480, 270, 4, TFT_GREEN);
    tft.drawRect(0, 270, 50, 50, TFT_ORANGE);

    // Setup buttons
    Button tmpBut;
    tmpBut.id = 0;
    tmpBut.xmin = 0;
    tmpBut.xmax = 50;
    tmpBut.ymin = 270;
    tmpBut.ymax = 320;
    tmpBut.enReleaseEv = true;

    ts.setButton(&tmpBut); // Back buttoon

    tmpBut.id = 1;
    tmpBut.xmin = 0;
    tmpBut.xmax = 480;
    tmpBut.ymin = 0;
    tmpBut.ymax = 270;
    tmpBut.enHoldEv = true;
    tmpBut.enHoldTickEv = true;
    tmpBut.enReleaseEv = false;

    ts.setButton(&tmpBut); // Draw area
}

void Draw_Scr::update(const uint32_t deltaTime, TchScr_Drv& ts)
{

}

void Draw_Scr::render(tftLCD& tft)
{
    _UI->requestUpdate();

    if (touched)
    {
        tft.fillCircle(point.x, point.y, 1, TFT_WHITE);
        touched = false;
    }
}

void Draw_Scr::handleTouch(const TchEvent& event)
{
    if (event.trigger == TrgSrc::HOLD_TICK && event.id == 1)
    {
        touched = true;
        point = event.pos;
    }
    else if (event.trigger == TrgSrc::RELEASE)
    {
        if (event.id == 0)
        {
            _UI->setScreen(lcdUI::Info);
        }
    }
}
