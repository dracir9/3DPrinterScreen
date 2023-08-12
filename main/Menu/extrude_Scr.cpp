/**
 * @file   control_Scr.cpp
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 10-04-2023
 * -----
 * Last Modified: 12-08-2023
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

#include "extrude_Scr.h"
#include "dbg_log.h"


Extrude_Scr::Extrude_Scr(lcdUI * UI, tftLCD & tft, TchScr_Drv & ts):
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
    tft.drawBmpSPIFFS("/spiffs/extruder_48.bmp", 277, 264);
    tft.drawRoundRect(244, 256, 114, 64, 4, TFT_ORANGE);
    tft.drawBmpSPIFFS("/spiffs/term_48.bmp", 399, 264);
    tft.drawRoundRect(366, 256, 114, 64, 4, TFT_ORANGE);

    // Extruders
    int32_t extruders = _printer->getToolNum();

    if (extruders == 0)
        extruders = 2;
    
    float spacing = (480.0f - 50.0f) / (extruders + 1);

    
    for (int32_t i = 0; i < extruders; i++)
    {
        tft.drawRoundRect((i+1)*spacing - 40, 0, 80, 80, 4, TFT_WHITE);
        tft.drawBmpSPIFFS("/spiffs/arrowT_48.bmp", (i+1)*spacing - 20, 16);
        tft.drawBmpSPIFFS("/spiffs/extruder_48.bmp", (i+1)*spacing - 40, 104);
        tft.setTextDatum(CC_DATUM);
        tft.setTextColor(TFT_BLACK);
        tft.drawString(String(i+1), (i+1)*spacing-15, 118);
        tft.setTextDatum(CL_DATUM);
        tft.setTextColor(TFT_WHITE);
        tft.drawString("000.0", (i+1)*spacing+5, 145);
        tft.drawRoundRect((i+1)*spacing - 40, 170, 80, 80, 4, TFT_WHITE);
        tft.drawBmpSPIFFS("/spiffs/arrowB_48.bmp", (i+1)*spacing - 20, 186);
    }

    // Step Size
    tft.setTextDatum(CC_DATUM);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("Step", 455, 120);

    tft.setTextFont(4);
    tft.drawRoundRect(430, 51, 50, 50, 4, TFT_CYAN);
    tft.drawString("+", 456, 76);
    tft.drawRoundRect(430, 155, 50, 50, 4, TFT_CYAN);
    tft.drawString("-", 456, 180);

    Button tmpBut;
    tmpBut.id = 0;
    tmpBut.xmin = 0;
    tmpBut.xmax = 114;
    tmpBut.ymin = 256;
    tmpBut.ymax = 320;
    tmpBut.enReleaseEv = true;
    
    ts.setButton(&tmpBut); // Back to Info screen

    tmpBut.id = 1;
    tmpBut.xmin = 122;
    tmpBut.xmax = 236;
    tmpBut.ymin = 256;
    tmpBut.ymax = 320;
    tmpBut.enReleaseEv = true;

    ts.setButton(&tmpBut); // Move control

    tmpBut.id = 2;
    tmpBut.xmin = 366;
    tmpBut.xmax = 480;
    tmpBut.ymin = 256;
    tmpBut.ymax = 320;
    tmpBut.enReleaseEv = true;

    ts.setButton(&tmpBut); // Temperature control

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
}

void Extrude_Scr::update(uint32_t deltaTime, TchScr_Drv &ts)
{
    esp_err_t ret = ESP_OK;

    if (ret != ESP_OK)
    {
        DBG_LOGE("Failed to move the toolhead: %s", esp_err_to_name(ret));
    }
}

void Extrude_Scr::render(tftLCD &tft)
{
    tft.setTextSize(1);
    tft.setTextDatum(CC_DATUM);

    tft.setTextFont(2);
    tft.setTextPadding(50);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(String(stepSize, 1), 455, 136);
}

void Extrude_Scr::handleTouch(const TchEvent& event)
{
    if (event.trigger == TrgSrc::PRESS)
    {
        _UI->requestUpdate();
    }
    else if (event.trigger == TrgSrc::HOLD_END || event.trigger == TrgSrc::RELEASE)
    {
        switch (event.id)
        {
        case 9:
            if (stepSize == 1.0f)
                stepSize = 0.1f;
            else if (stepSize == 5.0f)
                stepSize = 1.0f;
            else if (stepSize == 10.0f)
                stepSize = 5.0f;
            break;

        case 10:
            if (stepSize == 0.1f)
                stepSize = 1.0f;
            else if (stepSize == 1.0f)
                stepSize = 5.0f;
            else if (stepSize == 5.0f)
                stepSize = 10.0f;
            break;
        
        default:
            break;
        }

        if (event.trigger == TrgSrc::RELEASE)
        {
            switch (event.id)
            {
            case 0:
                _UI->setScreen(lcdUI::INFO_SCR);
                break;
            
            case 1:
                _UI->setScreen(lcdUI::CONTROL_SCR);
                break;

            case 2:
                // TODO: Temperature screen
                break;

            default:
                break;
            }
        }
        _UI->requestUpdate();
    }
}
