/**
 * @file   gcodePreview_Scr.cpp
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 22-01-2022
 * -----
 * Last Modified: 14-04-2022
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

#include "preview_Scr.h"
#include "dbg_log.h"

Preview_Scr::Preview_Scr(lcdUI* UI, tftLCD& tft, TchScr_Drv& ts):
    Screen(UI)
{
    renderEngine = GCodeRenderer::instance();
    if (renderEngine == nullptr)
        DBG_LOGE("Render engine uninitialized!");

    displayed.reset();
    tft.fillScreen(TFT_BLACK);
    tft.drawRoundRect(0, 0, 320, 320, 4, TFT_GREEN);
    tft.drawRoundRect(320, 0, 160, 50, 4, TFT_BLUE);
    tft.fillRoundRect(320, 220, 160, 50, 4, 0x0AE0);
    tft.drawRoundRect(320, 270, 160, 50, 4, TFT_ORANGE);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextDatum(CC_DATUM);
    tft.setTextPadding(0);
    tft.drawStringWr(_UI->getFile().substr(_UI->getFile().rfind('/')+1).c_str(), 400, 25, 150, 48); // Display filename
    tft.setTextColor(TFT_WHITE);
    tft.drawString("Start!", 400, 245);
    tft.drawString("Loading...", 160, 152);
    tft.drawBmpSPIFFS("/spiffs/return_48.bmp", 376, 277);

    // Setup buttons
    Button tmpBut;
    tmpBut.id = 0;
    tmpBut.xmin = 320;
    tmpBut.xmax = 480;
    tmpBut.ymin = 270;
    tmpBut.ymax = 320;
    tmpBut.enReleaseEv = true;
    
    ts.setButton(&tmpBut); // Back to Info screen
}

void Preview_Scr::update(const uint32_t deltaTime, TchScr_Drv& ts)
{
    if (!started)
    {
        if (renderEngine->begin(_UI->getFile().c_str()) == ESP_OK)
            started = true;
        else
            _UI->requestUpdate();
    }

    if (!_UI->isSDinit())
    {
        _UI->setScreen(lcdUI::Info);
    }
}

void Preview_Scr::render(tftLCD& tft)
{
    if (rendered) return;
    uint16_t* img = nullptr;
    // Main update loop is paused here
    esp_err_t ret = renderEngine->getRender(&img, pdMS_TO_TICKS(100));
    if (ret == ESP_OK)
    {
        bool oldBytes = tft.getSwapBytes();
        tft.setSwapBytes(true);
        tft.pushImage(0, 0, 320, 320, img);
        tft.setSwapBytes(oldBytes);
        tft.drawRoundRect(0, 0, 320, 320, 4, TFT_GREEN);
        rendered = true;
        drawInfo(tft);
    }
    else if (ret == ESP_FAIL)
    {
        tft.setTextDatum(CC_DATUM);
        tft.setTextFont(4);
        tft.setTextSize(1);
        tft.setTextPadding(150);
        tft.setTextColor(TFT_WHITE, TFT_RED);
        tft.drawString("Render error", 160, 160);
        rendered = true;
    }
    else
    {
        drawInfo(tft);
        _UI->requestUpdate();
    }
}

void Preview_Scr::handleTouch(const TchEvent& event)
{
    if (event.trigger == TrgSrc::RELEASE && event.id == 0)
    {
        _UI->clearFile();
        _UI->setScreen(lcdUI::FileBrowser);
    }
}

void Preview_Scr::drawInfo(tftLCD& tft)
{
    // Text settings
    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextPadding(0);
    tft.setTextColor(TFT_WHITE);
    String txt;

    const FileInfo* info = renderEngine->getInfo();

    if (info->filamentReady && !displayed[0])
    {
        tft.drawString("Filament cost: " , 328, 55);
        txt = String(info->filament, 3) + " m";
        tft.drawString(txt, 335, 71);
        displayed[0] = true;
    }

    if (info->timeReady && !displayed[1])
    {
        tft.drawString("Estimated time: " , 328, 95);
        int16_t seconds = info->printTime % 60;
        int16_t minutes = info->printTime/60 % 60;
        int16_t hours = info->printTime/3600 % 24;
        int16_t days = info->printTime/86400;
        txt = "";
        if (days > 0) txt += String(days) + "d ";
        if (hours > 0) txt += String(hours) + "h ";
        if (minutes > 0) txt += String(minutes) + "min ";
        txt += String(seconds) + "s";
        tft.drawString(txt, 335, 111);
        displayed[1] = true;
    }

    if (!rendered)
    {
        tft.setTextDatum(CC_DATUM);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(String(renderEngine->getProgress(), 0) + String("%"), 160, 168);
    }
}
