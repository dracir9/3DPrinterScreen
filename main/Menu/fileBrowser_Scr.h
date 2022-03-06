/**
 * @file   fileBrowser_Scr.h
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 22-01-2022
 * -----
 * Last Modified: 06-03-2022
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

#ifndef FILE_BROWSER_SCR_H
#define FILE_BROWSER_SCR_H

#include "stdio.h"
#include "../lcdUI.h"
#include "dirent.h"

class FileBrowser_Scr final: public Screen
{
public:
    FileBrowser_Scr(lcdUI* UI, tftLCD& tft, TchScr_Drv& ts);

    void update(const uint32_t deltaTime, TchScr_Drv& ts) override;

    void render(tftLCD& tft) override;

    void handleTouch(const TchEvent& event) override;

private:
    esp_err_t removeDir(const char* path);
    void loadPage();
    void renderPage(tftLCD& tft);
    bool isPageLoaded();
    void setPageLoaded();
    void updatePath(const std::string &newPath, const bool relativePath);
    bool isHidden(const char * name);
    bool isGcode(const std::string &file);
    void sendFile(const std::string &file);

    std::string path = "/sdcard";
    std::string dirList[8];
    uint8_t fileDepth = 0;
    uint8_t isDir = 0;
    uint8_t numFilePages = 0;
    uint8_t filePage = 0;
    uint8_t pageLoaded = 1;
    bool pageRendered = false;
    uint8_t hiddenFiles[32] = {0};
};

#endif