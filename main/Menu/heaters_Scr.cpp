/**
 * @file   heaters_Scr.cpp
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 12-08-2023
 * -----
 * Last Modified: 13-08-2023
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

#include "heaters_Scr.h"
#include "dbg_log.h"


Heaters_Scr::Heaters_Scr(lcdUI * UI, tftLCD & tft, TchScr_Drv & ts):
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

    // Step Size
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
    tmpBut.xmin = 244;
    tmpBut.xmax = 358;
    tmpBut.ymin = 256;
    tmpBut.ymax = 320;
    tmpBut.enReleaseEv = true;

    ts.setButton(&tmpBut); // Extruder control

    tmpBut.id = 3;
    tmpBut.xmin = 430;
    tmpBut.xmax = 480;
    tmpBut.ymin = 51;
    tmpBut.ymax = 101;
    tmpBut.enReleaseEv = true;

    ts.setButton(&tmpBut); // Step -

    tmpBut.id = 4;
    tmpBut.xmin = 430;
    tmpBut.xmax = 480;
    tmpBut.ymin = 155;
    tmpBut.ymax = 205;
    tmpBut.enReleaseEv = true;

    ts.setButton(&tmpBut); // Step +

    // Extruders
    numTools = _printer->getToolNum();

    if (numTools == 0)
        numTools = 2;

    float spacing = ((480.0f - 50.0f) - (numTools+1)*buttSize) / (numTools + 2);
    
    tft.setTextFont(2);
    for (int32_t i = 0; i <= numTools; i++)
    {
        if (i < numTools)
        {        
            tft.drawRoundRect(spacing*(i+1) + buttSize*i, 0, buttSize, buttSize, 4, TFT_WHITE);
            tft.drawBmpSPIFFS("/spiffs/arrowT_48.bmp", spacing*(i+1) + buttSize*i +20, 16);

            tft.drawBmpSPIFFS("/spiffs/extruder_32.bmp", spacing*(i+1) + buttSize*i +24, 112);
            tft.setTextColor(TFT_BLACK);
            tft.drawString(String(i+1), spacing*(i+1) + buttSize*i +40, 120);
            tft.setTextColor(TFT_WHITE);
            tft.drawString("000.0", spacing*(i+1) + buttSize*i +40, 155);

            tft.drawRoundRect(spacing*(i+1) + buttSize*i, 170, buttSize, buttSize, 4, TFT_WHITE);
            tft.drawBmpSPIFFS("/spiffs/arrowB_48.bmp", spacing*(i+1) + buttSize*i +20, 186);
        }
        else
        {
            tft.drawRoundRect(spacing*(i+1) + buttSize*i, 0, buttSize, buttSize, 4, TFT_WHITE);
            tft.drawBmpSPIFFS("/spiffs/arrowT_48.bmp", spacing*(i+1) + buttSize*i +20, 16);

            tft.drawBmpSPIFFS("/spiffs/heatbed_32.bmp", spacing*(i+1) + buttSize*i +24, 112);
            tft.setTextColor(TFT_WHITE);

            tft.drawString("000.0", spacing*(i+1) + buttSize*i +40, 155);

            tft.drawRoundRect(spacing*(i+1) + buttSize*i, 170, buttSize, buttSize, 4, TFT_WHITE);
            tft.drawBmpSPIFFS("/spiffs/arrowB_48.bmp", spacing*(i+1) + buttSize*i +20, 186);
        }

        tmpBut.id = 5+i*2;
        tmpBut.xmin = spacing*(i+1) + buttSize*i;
        tmpBut.xmax = spacing*(i+1) + buttSize*i + buttSize;
        tmpBut.ymin = 0;
        tmpBut.ymax = 80;
        tmpBut.enReleaseEv = true;

        ts.setButton(&tmpBut); // Step -

        tmpBut.id = 6+i*2;
        tmpBut.xmin = spacing*(i+1) + buttSize*i;
        tmpBut.xmax = spacing*(i+1) + buttSize*i + buttSize;
        tmpBut.ymin = 170;
        tmpBut.ymax = 250;
        tmpBut.enReleaseEv = true;

        ts.setButton(&tmpBut); // Step +
    }
}

void Heaters_Scr::update(uint32_t deltaTime, TchScr_Drv &ts)
{
    esp_err_t ret = ESP_OK;

    if (ret != ESP_OK)
    {
        DBG_LOGE("Failed to move the toolhead: %s", esp_err_to_name(ret));
    }
}

void Heaters_Scr::render(tftLCD &tft)
{
    tft.setTextSize(1);
    tft.setTextDatum(CC_DATUM);

    tft.setTextFont(2);
    tft.setTextPadding(50);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(String(stepSize, 1), 455, 136);

    float spacing = ((480.0f - 50.0f) - (numTools+1)*buttSize) / (numTools + 2);

    String tempTxt;
    float temperature;
    for (int32_t i = 0; i <= numTools; i++)
    {
        if (i < numTools)
        {            
            tempTxt = String(_printer->getTarToolTemp(i), 0);
            temperature = _printer->getTarToolTemp(i);
        }
        else
        {
            tempTxt = String(_printer->getTarBedTemp(), 0);
            temperature = _printer->getTarBedTemp();
        }
        
        tempTxt += ' ';
        tempTxt += _printer->getTempUnit();

        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(tempTxt, spacing*(i+1) + buttSize*i +40, 155);

        if (temperature < 150.0f)
            tft.setTextColor(TFT_CYAN, TFT_BLACK);
        else if (temperature < 200.0f)
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
        else
            tft.setTextColor(TFT_ORANGE, TFT_BLACK);

        tempTxt = String(temperature, 0);
        tempTxt += ' ';
        tempTxt += _printer->getTempUnit();
        tft.drawString(tempTxt, spacing*(i+1) + buttSize*i + 40, 100);
    }
}

void Heaters_Scr::handleTouch(const TchEvent& event)
{
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
            _UI->setScreen(lcdUI::EXTRUDE_SCR);
            break;
            
        case 3:
            if (stepSize == 1.0f)
                stepSize = 5.0f;
            else if (stepSize == 5.0f)
                stepSize = 10.0f;
            else if (stepSize == 10.0f)
                stepSize = 50.0f;
            break;

        case 4:
            if (stepSize == 5.0f)
                stepSize = 1.0f;
            else if (stepSize == 10.0f)
                stepSize = 5.0f;
            else if (stepSize == 50.0f)
                stepSize = 10.0f;
            break;
        
        default:
        {
            uint8_t tool = (event.id - 5)/2;
            esp_err_t ret = ESP_OK;
            float temp;
            if (tool < numTools)
            {
                if ((event.id-5) % 2 == 0)
                    temp = _printer->getTarToolTemp(tool) + stepSize;
                else
                    temp = _printer->getTarToolTemp(tool) - stepSize;

                ret = _printer->setToolTemp(tool, temp);
                
                DBG_LOGE_IF(ret != ESP_OK, "Failed to set tool %o target temperature to %f", tool, temp);
            }
            else if (tool == numTools)
            {
                if ((event.id-5) % 2 == 0)
                    temp = _printer->getTarBedTemp() + stepSize;
                else
                    temp = _printer->getTarBedTemp() - stepSize;

                ret = _printer->setBedTemp(temp);

                DBG_LOGE_IF(ret != ESP_OK, "Failed to set bed target temperature to %f", temp);
            }
            break;
        }
        }
        _UI->requestUpdate();
    }
}
