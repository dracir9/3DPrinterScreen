/**
 * @file   control_Scr.cpp
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 10-04-2023
 * -----
 * Last Modified: 16-04-2023
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
    tft.setTextDatum(BL_DATUM);
    tft.drawString("(0,0)", 2, 318);

    tft.setTextDatum(BR_DATUM);
    tft.drawString("(250,0)", 318, 318);

    tft.setTextDatum(TL_DATUM);
    tft.drawString("(0,210)", 2, 2);

    tft.setTextDatum(TR_DATUM);
    tft.drawString("(250,210)", 318, 2);

    // Joystick
    tft.drawRoundRect(324, 53 , 50, 50, 4, TFT_RED);
    tft.drawRoundRect(377, 0  , 50, 50, 4, TFT_GREEN);
    tft.drawRoundRect(377, 53 , 50, 50, 4, TFT_WHITE);
    tft.drawRoundRect(377, 106, 50, 50, 4, TFT_GREEN);
    tft.drawRoundRect(430, 0  , 50, 50, 4, TFT_BLUE);
    tft.drawRoundRect(430, 53 , 50, 50, 4, TFT_RED);
    tft.drawRoundRect(430, 106, 50, 50, 4, TFT_BLUE);
    tft.drawBmpSPIFFS("/spiffs/home_24.bmp", 390, 66);


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

    tmpBut.id = 1;
    tmpBut.xmin = 0;
    tmpBut.xmax = 320;
    tmpBut.ymin = 0;
    tmpBut.ymax = 320;
    tmpBut.enPressEv = true;
    tmpBut.enReleaseEv = false;

    ts.setButton(&tmpBut); // Move area

    tmpBut.id = 2;
    tmpBut.xmin = 377;
    tmpBut.xmax = 427;
    tmpBut.ymin = 53;
    tmpBut.ymax = 103;
    tmpBut.enPressEv = true;
    tmpBut.enHoldEv = true;

    ts.setButton(&tmpBut); // Home axis

    tmpBut.id = 3;
    tmpBut.xmin = 324;
    tmpBut.xmax = 374;
    tmpBut.ymin = 53;
    tmpBut.ymax = 103;
    tmpBut.enPressEv = true;
    tmpBut.enHoldEv = true;

    ts.setButton(&tmpBut); // Move X-

    tmpBut.id = 4;
    tmpBut.xmin = 430;
    tmpBut.xmax = 480;
    tmpBut.ymin = 53;
    tmpBut.ymax = 103;
    tmpBut.enPressEv = true;
    tmpBut.enHoldEv = true;

    ts.setButton(&tmpBut); // Move X+

    tmpBut.id = 5;
    tmpBut.xmin = 377;
    tmpBut.xmax = 427;
    tmpBut.ymin = 106;
    tmpBut.ymax = 156;
    tmpBut.enPressEv = true;
    tmpBut.enHoldEv = true;

    ts.setButton(&tmpBut); // Move Y-

    tmpBut.id = 6;
    tmpBut.xmin = 377;
    tmpBut.xmax = 427;
    tmpBut.ymin = 0;
    tmpBut.ymax = 50;
    tmpBut.enPressEv = true;
    tmpBut.enHoldEv = true;

    ts.setButton(&tmpBut); // Move Y+

    tmpBut.id = 7;
    tmpBut.xmin = 430;
    tmpBut.xmax = 480;
    tmpBut.ymin = 106;
    tmpBut.ymax = 156;
    tmpBut.enPressEv = true;
    tmpBut.enHoldEv = true;

    ts.setButton(&tmpBut); // Move Z-

    tmpBut.id = 8;
    tmpBut.xmin = 430;
    tmpBut.xmax = 480;
    tmpBut.ymin = 0;
    tmpBut.ymax = 50;
    tmpBut.enPressEv = true;
    tmpBut.enHoldEv = true;

    ts.setButton(&tmpBut); // Move Z+
}

void Control_Scr::update(uint32_t deltaTime, TchScr_Drv &ts)
{
}

void Control_Scr::render(tftLCD &tft)
{
    if (xnPressed)
    {
        tft.fillRoundRect(325, 54, 48, 48, 4, TFT_RED);
    }
    else
    {
        tft.fillRoundRect(325, 54, 48, 48, 4, TFT_BLACK);
    }

    if (xpPressed)
    {
        tft.fillRoundRect(431, 54, 48, 48, 4, TFT_RED);
    }
    else
    {
        tft.fillRoundRect(431, 54, 48, 48, 4, TFT_BLACK);
    }

    if (ynPressed)
    {
        tft.fillRoundRect(378, 107, 48, 48, 4, TFT_GREEN);
    }
    else
    {
        tft.fillRoundRect(378, 107, 48, 48, 4, TFT_BLACK);
    }

    if (ypPressed)
    {
        tft.fillRoundRect(378, 1, 48, 48, 4, TFT_GREEN);
    }
    else
    {
        tft.fillRoundRect(378, 1, 48, 48, 4, TFT_BLACK);
    }
    
    if (znPressed)
    {
        tft.fillRoundRect(431, 107, 48, 48, 4, TFT_BLUE);
    }
    else
    {
        tft.fillRoundRect(431, 107, 48, 48, 4, TFT_BLACK);
    }

    if (zpPressed)
    {
        tft.fillRoundRect(431, 1, 48, 48, 4, TFT_BLUE);
    }
    else
    {
        tft.fillRoundRect(431, 1, 48, 48, 4, TFT_BLACK);
    }
}

void Control_Scr::handleTouch(const TchEvent& event)
{
    if (event.trigger == TrgSrc::PRESS)
    {
        switch (event.id)
        {
        case 1:
            _tchPos = event.pos;
            break;

        case 3:
            xnPressed = true;
            break;

        case 4:
            xpPressed = true;
            break;

        case 5:
            ynPressed = true;
            break;

        case 6:
            ypPressed = true;
            break;

        case 7:
            znPressed = true;
            break;

        case 8:
            zpPressed = true;
            break;

        default:
            break;
        }
        _UI->requestUpdate();
    }
    else if (event.trigger == TrgSrc::HOLD_END)
    {
        switch (event.id)
        {
        case 0:
            _UI->setScreen(lcdUI::INFO_SCR);
            break;

        case 1:
            _tchPos = event.pos;
            break;

        case 3:
            xnPressed = false;
            break;

        case 4:
            xpPressed = false;
            break;

        case 5:
            ynPressed = false;
            break;

        case 6:
            ypPressed = false;
            break;

        case 7:
            znPressed = false;
            break;

        case 8:
            zpPressed = false;
            break;

        default:
            break;
        }
        _UI->requestUpdate();
    }
    else if (event.trigger == TrgSrc::RELEASE)
    {
        if (event.id == 0)
            _UI->setScreen(lcdUI::INFO_SCR);
    }
}
