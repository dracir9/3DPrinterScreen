
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
    tft->setCursor(300, 43);
    tft->setTextFont(2);
    if (_UI->checkSD())
    {
        tft->println("SD connected!");
        renderPage(tft);
    }
    else
    {
        tft->print("SD not found :(");
    }
}

void FileBrowser_Scr::handleTouch(const touchEvent event, const Vector2<int16_t> pos)
{
    if (event == press)
    {
        if (pos.y > 70 && pos.y < 120)
        {
            if (pos.x > 280 && pos.x < 330 && filePage > 0) filePage--;
            if (pos.x > 430 && filePage < numFilePages-1) filePage++;
        }
    }
}

void FileBrowser_Scr::printDirectory(File dir, int numTabs)
{
    while (true)
    {
        File entry =  dir.openNextFile();

        if (! entry)
        {
            // no more files
            break;
        }

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
    if (pageRendered == pageLoaded) return;

    // Draw controls
    tft->setTextDatum(CL_DATUM);
    tft->setTextPadding(270);
    tft->drawString(path, 10, 95);
    tft->fillRect(280, 70, 50, 50, pageLoaded == 0 ? TFT_DARKCYAN : TFT_CYAN);
    tft->fillRect(430, 70, 50, 50, pageLoaded == numFilePages-1 ? TFT_DARKCYAN : TFT_CYAN);
    tft->setTextDatum(CC_DATUM);
    char tmp[15];
    sprintf(tmp, "Page %d of %d", filePage+1, numFilePages);
    tft->setTextPadding(100);
    tft->drawString(tmp, 380, 95);

    // Draw file table
    tft->setTextDatum(CL_DATUM);
    uint8_t k = 0;
    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 2; j++)
        {
            if (strlen(dirList[k]) == 0)
            {
                tft->fillRect(240*j, 120 + 50*i, 240, 50, TFT_BLACK);   // Clear unused slots
                k++;
                continue;
            }
            tft->drawRect(240*j, 120 + 50*i, 240, 50, TFT_OLIVE);       // Draw grid
            tft->setTextPadding(202);                                   // Set pading to clear old text
            tft->drawString(dirList[k], 10 + 240*j, 145 + 50*i);
            if ((isDir & 1<<k) > 0)
            {
                tft->drawBmpSPIFFS("/spiffs/folder_24.bmp", 212 + 240*j, 133 + 50*i);
            }
            else
            {
                tft->drawBmpSPIFFS("/spiffs/file_24.bmp", 212 + 240*j, 133 + 50*i);
            }
            k++;
        }
    }
    pageRendered = pageLoaded;
}

void FileBrowser_Scr::loadPage()
{
    if (pageLoaded == filePage) return;
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
    for ( ; i < 8; i++)
    {
        dirList[i][0] = '\0';
    }
    pageLoaded = filePage;
}

bool FileBrowser_Scr::isHidden(const char *name)
{
    //const char *lookupTable[1] = {"System Volume Information"};
    if (strncmp(name, "System Volume Information", 256) == 0) return true;

    // May add extra system files here

    return false;
}
