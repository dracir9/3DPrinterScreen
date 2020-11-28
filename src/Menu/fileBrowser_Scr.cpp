
#include "fileBrowser_Scr.h"

FileBrowser_Scr::FileBrowser_Scr(lcdUI* UI)
{
    UI->tft.fillScreen(TFT_BLACK);
    _UI = UI;
    _UI->tft.drawRect(0, 0, 480, 70, TFT_RED);
    
    _UI->tft.setTextDatum(CC_DATUM);
    _UI->tft.setTextFont(4);
    _UI->tft.drawString("Test text", 240, 35);
    _UI->tft.setTextFont(2);
}

void FileBrowser_Scr::update(const uint32_t deltaTime)
{
    if (_UI->checkSD())
    {
        if (!init)
        {
            printDirectory(SD.open("/"), 0);

            printf("SPIFFS:\n");
            printDirectory(SPIFFS.open("/"), 0);
            /* if (SD.exists("/test.gcode"))
            {
                strcpy(_UI->selectedFile, "/sd/test.gcode");
                _UI->setScreen(_UI->GcodePreview);
            } */
            init = true;
        }

        loadPage();
    }
}

void FileBrowser_Scr::render(tftLCD *tft)
{
    if (_UI->checkSD())
    {
        renderPage(tft);
    }
    else
    {
        tft->setTextDatum(CC_DATUM);
        tft->drawString("SD not found :(", 240, 195);
    }
}

bool FileBrowser_Scr::isPageLoaded()
{
    return pageLoaded == filePage;
}

bool FileBrowser_Scr::isPageRendered()
{
    return pageRendered == pageLoaded;
}

void FileBrowser_Scr::setPageLoaded()
{
    pageLoaded = filePage;
}

void FileBrowser_Scr::setPageRendered()
{
    pageRendered = pageLoaded;
}

void FileBrowser_Scr::updatePath(const char* newPath, const bool relativePath)
{
    if (relativePath)
    {
        char* dot = strstr(newPath, "/..");
        if (dot == NULL)
        {
            strcat(path, "/");
            strcat(path, newPath);
        } else
        {
            dot += 3;
            char* slash = strrchr(path, '/');
            if (slash == NULL || (*dot < '0' && *dot > '9'))
            {
                ESP_LOGE("fileBrowser", "Invalid path!");
                return;
            }

            for (uint8_t i = 0; i < (*dot-'0'); i++)
            {
                if (strcmp(path, "/sdcard") == 0) break;
                slash = strrchr(path, '/');
                if (slash == NULL) return;
                *slash = '\0';
            }
        }
    } else
    {
        strcpy(path, newPath);
    }
    ESP_LOGD("fileBrowser", "New path: %s", path);
    numFilePages = 0;       // Mark as new folder for reading
    filePage++;             // Trigger page load
    pageRendered++;         // Trigger page render
}

void FileBrowser_Scr::handleTouch(const touchEvent event, const Vector2<int16_t> pos)
{
    if (event == press)
    {
        if (pos.y >= 70 && pos.y < 120)
        {
            if (pos.x < 50) updatePath("/sdcard", false);       // Return Home
            else if (pos.x < 120) updatePath("/..3", true);
            else if (pos.x < 190) updatePath("/..2", true);
            else if (pos.x < 260) updatePath("/..1", true);
            else if (pos.x < 330) filePage = 0;
            else if (pos.x < 380 && filePage > 0) filePage--;
            else if (pos.x > 430 && filePage < numFilePages-1) filePage++;
        }
        else if (pos.y >= 120)
        {
            uint8_t idx = (pos.y - 120) / 50;
            if (pos.x >= 240) idx += 4;
            ESP_LOGD("Touch", "IDX: %d", idx);
            if ((isDir & (1 << idx)) > 0) updatePath(dirList[idx], true);
        }
    }
}

void FileBrowser_Scr::printDirectory(File dir, int numTabs)
{
    while (true)
    {
        File entry =  dir.openNextFile();

        if (!entry) break; // no more files

        for (uint8_t i = 0; i < numTabs; i++)
        {
            Serial.print('\t');
        }

        // strrchar() gets the last occurence of '/' get only the name of the file
        Serial.print(strrchr(entry.name(), '/'));

        if (entry.isDirectory())
        {
            Serial.println("/");
            printDirectory(entry, numTabs + 1);
        }
        else
        {
            // files have sizes, directories do not
            Serial.print("\t\t");
            Serial.println(entry.size(), DEC);
        }
        entry.close();
    }
}

void FileBrowser_Scr::renderPage(tftLCD *tft)
{
    if (isPageRendered()) return;

    // Draw controls
    tft->fillRect(330, 70, 50, 50, pageLoaded == 0 ? TFT_BLACK : TFT_CYAN);
    tft->fillRect(430, 70, 50, 50, pageLoaded == numFilePages-1 ? TFT_BLACK : TFT_CYAN);
    tft->drawRect(0, 70, 50, 50, TFT_ORANGE);
    tft->drawRect(50, 70, 70, 50, TFT_ORANGE);
    tft->setViewport(50, 70, 70, 50);
    tft->setCursor(5,0);
    tft->print("abcdefghijklmnop");
    tft->resetViewport();
    tft->drawRect(120, 70, 70, 50, TFT_ORANGE);
    tft->drawRect(190, 70, 70, 50, TFT_ORANGE);
    tft->drawRect(260, 70, 70, 50, TFT_ORANGE);

    tft->setTextDatum(CC_DATUM);
    tft->setTextPadding(48);
    tft->drawString("SD", 25, 95);
    char tmp[15];
    sprintf(tmp, "%d/%d", filePage+1, numFilePages);
    tft->drawString(tmp, 405, 95);

    // Draw file table
    tft->setTextDatum(CL_DATUM);
    uint8_t k = 0;
    for (uint8_t i = 0; i < 2; i++)
    {
        for (uint8_t j = 0; j < 4; j++)
        {
            if (strlen(dirList[k]) == 0)
            {
                tft->fillRect(240*i, 120 + 50*j, 240, 50, TFT_BLACK);   // Clear unused slots
                k++;
                continue;
            }
            tft->drawRect(240*i, 120 + 50*j, 240, 50, TFT_OLIVE);       // Draw grid
            tft->setTextPadding(202);                                   // Set pading to clear old text
            tft->drawString(dirList[k], 10 + 240*i, 145 + 50*j);        // Write file name
            if ((isDir & 1<<k) > 0)
            {
                tft->drawBmpSPIFFS("/spiffs/folder_24.bmp", 212 + 240*i, 133 + 50*j);   // Draw folder icon
            }
            else
            {
                tft->drawBmpSPIFFS("/spiffs/file_24.bmp", 212 + 240*i, 133 + 50*j);     // Draw file icon
            }
            k++;
        }
    }
    setPageRendered();
}

void FileBrowser_Scr::loadPage()
{
    if (isPageLoaded()) return;
    struct dirent *entry;
    DIR * dir = opendir(path);
    if (!dir)
    {
        ESP_LOGE("fileBrowser", "Error opening folder %s", path);
        return;
    }

    if (numFilePages == 0) // First directory read
    {
        uint16_t hidden = 0;
        while ((entry = readdir(dir)) && numFilePages <= 255)
        {
            if (isHidden(entry->d_name))
            {
                hidden++;
                continue;
            }
            numFilePages++;      // Count number of files

            if (numFilePages % 8 == 0) hiddenFiles[numFilePages / 8] = hidden;
        }
        numFilePages /= 8;                                  // Translate to screen pages
        numFilePages++;
        ESP_LOGD("fileBrowser", "File Pages: %d", numFilePages);

        rewinddir(dir);
        filePage = 0;
    }

    seekdir(dir, filePage*8 + hiddenFiles[filePage]);
    uint8_t i = 0;
    while (i < 8)
    {
        entry = readdir(dir);
        if (!entry) break;                                  // No more files

        if (isHidden(entry->d_name)) continue;
        
        strncpy(dirList[i], entry->d_name, maxFilenameLen-1); // Save file name and cut to size
        dirList[i][maxFilenameLen-1] = '\0';
        
        if (entry->d_type == DT_DIR) isDir |= 1<<i;
        i++;
    }

    closedir(dir);

    for ( ; i < 8; i++)
    {
        dirList[i][0] = '\0';
    }
    setPageLoaded();
}

bool FileBrowser_Scr::isHidden(const char *name)
{
    //const char *lookupTable[1] = {"System Volume Information"};
    if (strncmp(name, "System Volume Information", 256) == 0) return true;

    // May add extra system files here

    return false;
}
