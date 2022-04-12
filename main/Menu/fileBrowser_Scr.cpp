/**
 * @file   fileBrowser_Scr.cpp
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 22-01-2022
 * -----
 * Last Modified: 12-04-2022
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

#include "fileBrowser_Scr.h"
#include "dbg_log.h"

FileBrowser_Scr::FileBrowser_Scr(lcdUI* UI, tftLCD& tft, TchScr_Drv& ts):
    Screen(UI)
{
    tft.fillScreen(TFT_BLACK);

    // SD Home
    tft.drawRoundRect(0, 0, 50, 50, 4, TFT_ORANGE);
    tft.drawBmpSPIFFS("/spiffs/home_24.bmp", 13, 13);

    tft.drawBmpSPIFFS("/spiffs/return_48.bmp", 53, 272);
    tft.drawRoundRect(0, 256, 155, 64, 4, TFT_ORANGE);
    tft.drawRoundRect(288, 256, 66, 64, 4, TFT_CYAN);

    // Setup buttons
    Button tmpBut;
    tmpBut.id = 0;
    tmpBut.xmin = 0;
    tmpBut.xmax = 160;
    tmpBut.ymin = 256;
    tmpBut.ymax = 320;
    tmpBut.enReleaseEv = true;
    
    ts.setButton(&tmpBut); // Back to Info screen

    tmpBut.id = 1;
    tmpBut.xmin = 163;
    tmpBut.xmax = 280;
    tmpBut.ymin = 256;
    tmpBut.ymax = 320;

    ts.setButton(&tmpBut); // Prev page

    tmpBut.id = 2;
    tmpBut.xmin = 362;
    tmpBut.xmax = 480;
    tmpBut.ymin = 256;
    tmpBut.ymax = 320;
    
    ts.setButton(&tmpBut); // Next page

    for (int i = 0; i < 2; i++) // File buttons
    {
        for (int j = 0; j < 4; j++)
        {
            tmpBut.id = j + i*4 + 3;
            tmpBut.xmin = i * 240;
            tmpBut.xmax = i * 240 + 240;
            tmpBut.ymin = 50 * j + 50;
            tmpBut.ymax = 50 * j + 100;
            ts.setButton(&tmpBut);
        }
    }
    
    tmpBut.id = 11;
    tmpBut.xmin = 0;
    tmpBut.xmax = 50;
    tmpBut.ymin = 0;
    tmpBut.ymax = 50;
    tmpBut.enHoldEv = true;
    tmpBut.holdTime = 100;

    ts.setButton(&tmpBut); // Return to SD root
    tmpBut.enHoldEv = false;

    for (int i = 0; i < 5; i++) // Folder navigation
    {
        tmpBut.id = i + 12;
        tmpBut.xmin = 50 + 86*i;
        tmpBut.xmax = 136 + 86*i;
        tmpBut.ymin = 0;
        tmpBut.ymax = 50;
        ts.setButton(&tmpBut);
    }
}

void FileBrowser_Scr::update(const uint32_t deltaTime, TchScr_Drv& ts)
{
    if (_UI->isSDinit())
    {
        loadPage();
    }
    else
    {
        _UI->setScreen(lcdUI::Info);
    }
}

void FileBrowser_Scr::render(tftLCD& tft)
{
    renderPage(tft);
}

bool FileBrowser_Scr::isPageLoaded()
{
    return pageLoaded == filePage;
}

void FileBrowser_Scr::setPageLoaded()
{
    pageLoaded = filePage;
    pageRendered = false;
}

void FileBrowser_Scr::updatePath(const std::string &newPath, const bool relativePath)
{
    if (relativePath)
    {
        size_t dot = newPath.find("/..");
        if (dot == -1)
        {
            path += "/" + newPath;
        } 
        else
        {
            dot += 3;
            size_t slash = path.rfind('/');
            if (slash == -1 || (newPath[dot] < '0' && newPath[dot] > '9'))
            {
                DBG_LOGE("Invalid path!");
                return;
            }

            for (uint8_t i = newPath[dot]-'0'; i > 0; i--)
            {
                if (path.compare("/sdcard") == 0) break;
                path.erase(slash);
                slash = path.rfind('/');
                if (slash == -1) return;
            }
        }
    } else
    {
        path = newPath;
    }
    DBG_LOGD("New path: %s", path.c_str());
    numFilePages = 0;       // Mark as new folder for reading
    filePage++;             // Trigger page load
}

void FileBrowser_Scr::handleTouch(const TchEvent& event)
{
    if (event.trigger == TrgSrc::HOLD_STRT && event.id == 11)
    {
        if (removeDir("/sdcard/.cache") == ESP_OK)
            DBG_LOGI("Cache deleted!");
        else
            DBG_LOGI("Failed to delete cache");
        return;
    }

    if (event.trigger != TrgSrc::RELEASE) return;

    bool update = true;

    if (event.id == 0) // Back to Main menu
    {
        _UI->setScreen(lcdUI::Info);
    }
    else if (event.id == 1 && filePage > 0) // Prev page
    {
        filePage--;
    }
    else if (event.id == 2 && filePage < numFilePages-1) // Next page
    {
        filePage++;
    }
    else if (event.id > 2 && event.id <= 10) // Open file
    {
        uint8_t idx = event.id - 3;
        DBG_LOGV("IDX: %d", idx);
        if ((isDir & (1 << idx)) > 0) updatePath(dirList[idx], true);
        else if (isGcode(dirList[idx]))
            sendFile(dirList[idx]);
        else
            update = false;
    }
    else if (event.id == 11) // Return to root
    {
        updatePath("/sdcard", false);
    }
    else if (event.id > 11 && event.id <= 16) // Navigation bar
    {
        int8_t k = fileDepth > 5 ? 5 : fileDepth;
        k -= event.id - 12;
        switch (k)
        {
        case 1:
            filePage = 0;
            break;

        case 2:
            updatePath("/..1", true);
            break;

        case 3:
            updatePath("/..2", true);
            break;

        case 4:
            updatePath("/..3", true);
            break;

        case 5:
            updatePath("/..4", true);
            break;

        default:
            update = false;
            break;
        }
    }
    else
    {
        update = false;
    }
    
    if (update)
        _UI->requestUpdate();
}

esp_err_t FileBrowser_Scr::removeDir(const char* path)
{
    static char fullName[128];
    DIR *dp = opendir (path);

    if (dp != NULL)
    {
        struct dirent *ep;
        while ((ep = readdir (dp)))
        {
            if (ep->d_type == DT_DIR)
            {
                if (removeDir(ep->d_name) != ESP_OK)
                {
                    closedir (dp);
                    return ESP_FAIL;
                }
            }
            else
            {
                strcpy(fullName, path);
                strcat(fullName, "/");
                strcat(fullName, ep->d_name);
                DBG_LOGD("%s", fullName);
                if (remove(fullName) != 0)
                {
                    closedir (dp);
                    return ESP_FAIL;
                }
            }
        }

        closedir (dp);
        remove(path);
    }
    else
    {
        DBG_LOGE("Couldn't open the directory");
        return ESP_FAIL;
    }
    return ESP_OK;
}

void FileBrowser_Scr::renderPage(tftLCD& tft)
{
    if (pageRendered) return;

    // Draw controls
    // Page +
    if (pageLoaded == 0)
    {
        tft.fillRect(163, 256, 117, 64, TFT_BLACK);
    }
    else
    {
        tft.drawBmpSPIFFS("/spiffs/arrowL_48.bmp", 197, 268);
        tft.drawRoundRect(163, 256, 117, 64, 4, TFT_ORANGE);
    }

    // Page -
    if (pageLoaded == numFilePages-1)
    {
        tft.fillRect(362, 256, 118, 64, TFT_BLACK);
    }
    else
    {
        tft.drawBmpSPIFFS("/spiffs/arrowR_48.bmp", 397, 268);
        tft.drawRoundRect(362, 256, 118, 64, 4, TFT_ORANGE);
    }
    
    // Folder path names
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(2);
    tft.setTextPadding(0);
    tft.setTextDatum(CC_DATUM);

    uint8_t idx = path.length()-1;
    for (uint8_t i = 5; i > 0; i--) // For each of the last 5 folders
    {
        if (i <= fileDepth)
        {
            tft.fillRect(86*i - 35, 1, 84, 48, TFT_BLACK);
            tft.drawRoundRect(86*i - 36, 0, 86, 50, 4, TFT_ORANGE);
            uint8_t len = 0;
            for (; idx > 0; idx--, len++)
            {
                if (path[idx] == '/') break;
            }

            std::string folderName = path.substr(idx-- +1, len);
            tft.drawStringWr(folderName.c_str(), 7 + 86*i, 25, 80, 48);
        }
        else
        {
            tft.fillRect(86*i - 36, 0, 86, 50, TFT_BLACK);
        }
    }
    
    tft.setTextPadding(48);
    tft.drawString("Page", 321, 280);
    tft.drawString(String(pageLoaded+1) + " of " + String(numFilePages), 321, 296);

    // Draw file table
    tft.setTextDatum(CL_DATUM);
    tft.setTextPadding(202);                                   // Set pading to clear old text
    uint8_t k = 0;
    for (uint8_t i = 0; i < 2; i++)
    {
        for (uint8_t j = 0; j < 4; j++, k++)
        {
            if (dirList[k].length() == 0)
            {
                tft.fillRect(241*i, 51 + 50*j, 239, 48, TFT_BLACK);   // Clear unused slots
                continue;
            }
            tft.fillRect(10 + 240*i, 52 + 50*j, 200, 46, TFT_BLACK);
     
            tft.drawStringWr(dirList[k].c_str(), 10 + 240*i, 75 + 50*j, 200, 48);       // Write file name

            if ((isDir & 1<<k) > 0)
                tft.drawBmpSPIFFS("/spiffs/folder_24.bmp", 212 + 240*i, 63 + 50*j);     // Draw folder icon
            else if (isGcode(dirList[k]))
                tft.drawBmpSPIFFS("/spiffs/gcode_24.bmp", 212 + 240*i, 63 + 50*j);      // Draw gcode icon
            else
                tft.drawBmpSPIFFS("/spiffs/file_24.bmp", 212 + 240*i, 63 + 50*j);       // Draw file icon
            
            tft.drawRoundRect(241*i, 51 + 50*j, 239, 48, 4, TFT_OLIVE);       // Draw grid
        }
    }
    pageRendered = true;
}

void FileBrowser_Scr::loadPage()
{
    if (isPageLoaded()) return;
    struct dirent *entry;
    DIR * dir = opendir(path.c_str());
    if (!dir)
    {
        DBG_LOGE("Error opening folder %s", path.c_str());
        return;
    }

    if (numFilePages == 0) // First directory read
    {
        uint16_t hidden = 0;
        while ((entry = readdir(dir)) && numFilePages < 255)
        {
            if (isHidden(entry->d_name))
            {
                hidden++;
                continue;
            }
            numFilePages++;      // Count number of files

            if (numFilePages % 8 == 0) hiddenFiles[numFilePages / 8] = hidden;
        }

        if (numFilePages != 0)
            numFilePages = (numFilePages - 1) / 8;          // Translate to screen pages
        numFilePages++;
        DBG_LOGD("File Pages: %d", numFilePages);

        fileDepth = 0;
        for (uint8_t i = 1; i < path.length(); i++)
        {
            if (path[i] == '/') fileDepth++;
        }

        rewinddir(dir);
        filePage = 0;
    }

    isDir = 0;
    seekdir(dir, filePage*8 + hiddenFiles[filePage]);
    uint8_t i = 0;
    while (i < 8)
    {
        entry = readdir(dir);
        if (!entry) break;                                  // No more files

        if (isHidden(entry->d_name)) continue;
        
        dirList[i] = entry->d_name;                         // Save file name
        
        if (entry->d_type == DT_DIR) isDir |= 1<<i;
        i++;
    }

    closedir(dir);

    for ( ; i < 8; i++)
    {
        dirList[i].clear();
    }
    setPageLoaded();
}

bool FileBrowser_Scr::isHidden(const char *name)
{
    //const char *lookupTable[1] = {"System Volume Information"};
    if (strncmp(name, "System Volume Information", 25) == 0 ||
        strncmp(name, ".cache", 6) == 0)
        return true;

    // May add extra system files here

    return false;
}

bool FileBrowser_Scr::isGcode(const std::string &file)
{
    size_t dot = file.rfind('.');
    if (dot != -1)
    {
        dot++;
        if (file.compare(dot, 5, "g") == 0 || file.compare(dot, 5, "gco") == 0 || file.compare(dot, 5, "gcode") == 0)
            return true;
    }
    return false;
}

void FileBrowser_Scr::sendFile(const std::string &file)
{
    if (_UI->setFile(path + "/" + file) == ESP_OK)
        _UI->setScreen(lcdUI::GcodePreview);
}
