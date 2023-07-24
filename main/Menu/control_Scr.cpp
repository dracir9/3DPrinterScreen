/**
 * @file   control_Scr.cpp
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 10-04-2023
 * -----
 * Last Modified: 24-07-2023
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
#include "dbg_log.h"


Control_Scr::Control_Scr(lcdUI * UI, tftLCD & tft, TchScr_Drv & ts):
    Screen(UI)
{
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextPadding(0);
    tft.setTextDatum(CC_DATUM);
    
    tft.setTextColor(TFT_WHITE);

    // Bottom buttons
    tft.drawBmpSPIFFS("/spiffs/return_48.bmp", 33, 272);
    tft.drawRoundRect(0, 256, 114, 64, 4, TFT_ORANGE);
    tft.drawBmpSPIFFS("/spiffs/move_48.bmp", 155, 264);
    tft.drawRoundRect(122, 256, 114, 64, 4, TFT_ORANGE);
    tft.drawBmpSPIFFS("/spiffs/settings_48.bmp", 378, 264);
    tft.drawRoundRect(244, 256, 114, 64, 4, TFT_ORANGE);
    tft.drawRoundRect(366, 256, 114, 64, 4, TFT_ORANGE);

    // Joystick
    tft.drawRoundRect(0, 85 , 80, 80, 4, TFT_RED);
    tft.drawRoundRect(85, 0  , 80, 80, 4, TFT_GREEN);
    tft.drawRoundRect(85, 85 , 80, 80, 4, TFT_WHITE);
    tft.drawRoundRect(85, 170, 80, 80, 4, TFT_GREEN);
    tft.drawRoundRect(170, 0  , 80, 80, 4, TFT_BLUE);
    tft.drawRoundRect(170, 85 , 80, 80, 4, TFT_RED);
    tft.drawRoundRect(170, 170, 80, 80, 4, TFT_BLUE);
    tft.drawBmpSPIFFS("/spiffs/home_24.bmp", 113, 113);

    // Step Size
    tft.setTextDatum(CC_DATUM);
    tft.drawString("XY Step", 403, 156);
    tft.drawString("Z Step", 403, 209);

    tft.setTextFont(4);
    tft.drawRoundRect(324, 148, 50, 50, 4, TFT_CYAN);
    tft.drawString("-", 350, 174);
    tft.drawRoundRect(430, 148, 50, 50, 4, TFT_CYAN);
    tft.drawString("+", 456, 174);

    tft.drawRoundRect(324, 201 , 50, 50, 4, TFT_CYAN);
    tft.drawString("-", 350, 227);
    tft.drawRoundRect(430, 201, 50, 50, 4, TFT_CYAN);
    tft.drawString("+", 456, 227);

    Button tmpBut;
    tmpBut.id = 0;
    tmpBut.xmin = 0;
    tmpBut.xmax = 114;
    tmpBut.ymin = 256;
    tmpBut.ymax = 320;
    tmpBut.enReleaseEv = true;
    
    ts.setButton(&tmpBut); // Back to Info screen

    tmpBut.id = 2;
    tmpBut.xmin = 85;
    tmpBut.xmax = 165;
    tmpBut.ymin = 85;
    tmpBut.ymax = 165;
    tmpBut.enPressEv = true;
    tmpBut.enHoldEv = true;
    tmpBut.enReleaseEv = true;

    ts.setButton(&tmpBut); // Home all axis

    tmpBut.id = 3;
    tmpBut.xmin = 0;
    tmpBut.xmax = 80;
    tmpBut.ymin = 85;
    tmpBut.ymax = 165;
    tmpBut.enPressEv = true;
    tmpBut.enHoldEv = true;
    tmpBut.enReleaseEv = true;

    ts.setButton(&tmpBut); // Move X-

    tmpBut.id = 4;
    tmpBut.xmin = 170;
    tmpBut.xmax = 250;
    tmpBut.ymin = 85;
    tmpBut.ymax = 165;
    tmpBut.enPressEv = true;
    tmpBut.enHoldEv = true;
    tmpBut.enReleaseEv = true;

    ts.setButton(&tmpBut); // Move X+

    tmpBut.id = 5;
    tmpBut.xmin = 85;
    tmpBut.xmax = 165;
    tmpBut.ymin = 170;
    tmpBut.ymax = 250;
    tmpBut.enPressEv = true;
    tmpBut.enHoldEv = true;
    tmpBut.enReleaseEv = true;

    ts.setButton(&tmpBut); // Move Y-

    tmpBut.id = 6;
    tmpBut.xmin = 85;
    tmpBut.xmax = 165;
    tmpBut.ymin = 0;
    tmpBut.ymax = 80;
    tmpBut.enPressEv = true;
    tmpBut.enHoldEv = true;
    tmpBut.enReleaseEv = true;

    ts.setButton(&tmpBut); // Move Y+

    tmpBut.id = 7;
    tmpBut.xmin = 170;
    tmpBut.xmax = 250;
    tmpBut.ymin = 170;
    tmpBut.ymax = 250;
    tmpBut.enPressEv = true;
    tmpBut.enHoldEv = true;
    tmpBut.enReleaseEv = true;

    ts.setButton(&tmpBut); // Move Z-

    tmpBut.id = 8;
    tmpBut.xmin = 170;
    tmpBut.xmax = 250;
    tmpBut.ymin = 0;
    tmpBut.ymax = 80;
    tmpBut.enPressEv = true;
    tmpBut.enHoldEv = true;
    tmpBut.enReleaseEv = true;

    ts.setButton(&tmpBut); // Move Z+

    tmpBut.id = 9;
    tmpBut.xmin = 324;
    tmpBut.xmax = 374;
    tmpBut.ymin = 164;
    tmpBut.ymax = 214;
    tmpBut.enPressEv = false;
    tmpBut.enHoldEv = false;
    tmpBut.enReleaseEv = true;

    ts.setButton(&tmpBut); // Step -

    tmpBut.id = 10;
    tmpBut.xmin = 430;
    tmpBut.xmax = 480;
    tmpBut.ymin = 164;
    tmpBut.ymax = 214;
    tmpBut.enPressEv = false;
    tmpBut.enHoldEv = false;
    tmpBut.enReleaseEv = true;

    ts.setButton(&tmpBut); // Step +

    tmpBut.id = 11;
    tmpBut.xmin = 324;
    tmpBut.xmax = 374;
    tmpBut.ymin = 217;
    tmpBut.ymax = 257;
    tmpBut.enPressEv = false;
    tmpBut.enHoldEv = false;
    tmpBut.enReleaseEv = true;

    ts.setButton(&tmpBut); // Z Step -

    tmpBut.id = 12;
    tmpBut.xmin = 430;
    tmpBut.xmax = 480;
    tmpBut.ymin = 217;
    tmpBut.ymax = 257;
    tmpBut.enPressEv = false;
    tmpBut.enHoldEv = false;
    tmpBut.enReleaseEv = true;

    ts.setButton(&tmpBut); // Z Step +
}

void Control_Scr::update(uint32_t deltaTime, TchScr_Drv &ts)
{
    esp_err_t ret = ESP_OK;
    if (xnPressed)
    {
        ret = _printer->move(-stepSize, 0, 0, true);
    }
    else if (xpPressed)
    {
        ret = _printer->move(stepSize, 0, 0, true);
    }
    else if (ynPressed)
    {
        ret = _printer->move(0, -stepSize, 0, true);
    }
    else if (ypPressed)
    {
        ret = _printer->move(0, stepSize, 0, true);
    }
    else if (znPressed)
    {
        ret = _printer->move(0, 0, -zStepSize, true);
    }
    else if (zpPressed)
    {
        ret = _printer->move(0, 0, zStepSize, true);
    }

    if (ret != ESP_OK)
    {
        DBG_LOGE("Failed to move the toolhead: %s", esp_err_to_name(ret));
    }
}

void Control_Scr::render(tftLCD &tft)
{
    if (xnPressed)
    {
        tft.fillRoundRect(1, 86, 78, 78, 4, TFT_RED);
    }
    else
    {
        tft.fillRoundRect(1, 86, 78, 78, 4, TFT_BLACK);
    }

    if (xpPressed)
    {
        tft.fillRoundRect(171, 86, 78, 78, 4, TFT_RED);
    }
    else
    {
        tft.fillRoundRect(171, 86, 78, 78, 4, TFT_BLACK);
    }

    if (ynPressed)
    {
        tft.fillRoundRect(86, 171, 78, 78, 4, TFT_GREEN);
    }
    else
    {
        tft.fillRoundRect(86, 171, 78, 78, 4, TFT_BLACK);
    }

    if (ypPressed)
    {
        tft.fillRoundRect(86, 1, 78, 78, 4, TFT_GREEN);
    }
    else
    {
        tft.fillRoundRect(86, 1, 78, 78, 4, TFT_BLACK);
    }
    
    if (znPressed)
    {
        tft.fillRoundRect(171, 171, 78, 78, 4, TFT_BLUE);
    }
    else
    {
        tft.fillRoundRect(171, 171, 78, 78, 4, TFT_BLACK);
    }

    if (zpPressed)
    {
        tft.fillRoundRect(171, 1, 78, 78, 4, TFT_BLUE);
    }
    else
    {
        tft.fillRoundRect(171, 1, 78, 78, 4, TFT_BLACK);
    }

    tft.setTextFont(4);
    tft.setTextSize(1);
    tft.setTextPadding(0);
    tft.setTextDatum(CC_DATUM);
    tft.drawString("X-", 40, 125);
    tft.drawString("X+", 210, 125);
    tft.drawString("Y-", 125, 210);
    tft.drawString("Y+", 125, 40);
    tft.drawString("Z-", 210, 210);
    tft.drawString("Z+", 210, 40);

    tft.setTextFont(2);
    tft.setTextPadding(50);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(String(stepSize, 1), 403, 174);
    if (zStepSize < 10.0f)
        tft.drawString(String(zStepSize, 3), 403, 227);
    else
        tft.drawString(String(zStepSize, 1), 403, 227);
}

void Control_Scr::handleTouch(const TchEvent& event)
{
    if (event.trigger == TrgSrc::PRESS)
    {
        switch (event.id)
        {
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
            return;
        }
        _UI->requestUpdate();
    }
    else if (event.trigger == TrgSrc::HOLD_END || event.trigger == TrgSrc::RELEASE)
    {
        switch (event.id)
        {
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

        switch (event.id)
        {
        case 0:
            if (event.trigger == TrgSrc::RELEASE)
                _UI->setScreen(lcdUI::INFO_SCR);
            break;

        case 9:
            if (stepSize == 5.0f)
                stepSize = 1.0f;
            else if (stepSize == 10.0f)
                stepSize = 5.0f;
            else if (stepSize == 50.0f)
                stepSize = 10.0f;
            else if (stepSize == 100.0f)
                stepSize = 50.0f;
            break;

        case 10:
            if (stepSize == 1.0f)
                stepSize = 5.0f;
            else if (stepSize == 5.0f)
                stepSize = 10.0f;
            else if (stepSize == 10.0f)
                stepSize = 50.0f;
            else if (stepSize == 50.0f)
                stepSize = 100.0f;
            break;
            break;

        case 11:
            if (zStepSize == 0.01f)
                zStepSize = 0.001f;
            else if (zStepSize == 0.1f)
                zStepSize = 0.01f;
            else if (zStepSize == 1.0f)
                zStepSize = 0.1f;
            else if (zStepSize == 5.0f)
                zStepSize = 1.0f;
            else if (zStepSize == 10.0f)
                zStepSize = 5.0f;
            else if (zStepSize == 50.0f)
                zStepSize = 10.0f;
            else if (zStepSize == 100.0f)
                zStepSize = 50.0f;
            break;

        case 12:
            if (zStepSize == 0.001f)
                zStepSize = 0.01f;
            else if (zStepSize == 0.01f)
                zStepSize = 0.1f;
            else if (zStepSize == 0.1f)
                zStepSize = 1.0f;
            else if (zStepSize == 1.0f)
                zStepSize = 5.0f;
            else if (zStepSize == 5.0f)
                zStepSize = 10.0f;
            else if (zStepSize == 10.0f)
                zStepSize = 50.0f;
            else if (zStepSize == 50.0f)
                zStepSize = 100.0f;
            break;
        
        default:
            break;
        }
        _UI->requestUpdate();
    }
}
