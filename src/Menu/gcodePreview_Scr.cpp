
#include "gcodePreview_Scr.h"

GcodePreview_Scr::GcodePreview_Scr(lcdUI* UI)
{
    _UI = UI;
    _UI->tft.fillScreen(TFT_BLACK);
    _UI->tft.drawRect(0, 0, 320, 320, TFT_GREEN);
    _UI->tft.drawRect(32, 32, 256, 256, TFT_OLIVE);
}

bool GcodePreview_Scr::readLine()
{
    bool commentMode = false;
    *commentLine = '\0';
    uint8_t j = 0;
    for (int16_t i = 0; i < maxLineLen; i++)
    {
        if (bufPos >= readLen) // Reached end of read buffer
        {
            readLen = fread(readBuffer, 1, bufferLen, GcodeFile);
            if (readLen <= 0)
            {
                return false;
            }
            else if (ferror(GcodeFile))
            {
                ESP_LOGE("GcodePreview_Scr", "Error reading file \"%s\"", _UI->selectedFile.c_str());              
            }
            bufPos = 0;
        }
        if (readBuffer[bufPos] <= 0 || readBuffer[bufPos] == '\n' || readBuffer[bufPos] <= '\r') // End of line encountered
        {
            gCodeLine[i] = '\0';    // Ensure null-terminated string
            commentLine[j] = '\0';
            if (i == 0) // If empty line keep reading
            {
                i = -1;
                bufPos++;
                continue;
            }
            return true;
        }
        else if (readBuffer[bufPos] == ';')
        {
            commentMode = true;
            gCodeLine[i] = '\0';
        }

        if (commentMode)
            commentLine[j++] = readBuffer[bufPos];
        else
            gCodeLine[i] = readBuffer[bufPos]; // Fill line buffer

        bufPos++;
    }

    ESP_LOGE("GcodePreview_Scr", "Very long line in file \"%s\". ABORT!", _UI->selectedFile.c_str());
    return false;
}

bool GcodePreview_Scr::processLine()
{

    // Parse comments for usefull information
    if (strlen(commentLine) > 1);
        parseComment(commentLine);

    // Should be safe to parse
    if (strlen(gCodeLine) > 1)
        parser.parse(gCodeLine);
    else
        return false;

    switch (parser.command_letter)
    {
    case 'G':
        switch (parser.codenum)
        {
        case 0: case 1:
            if (parser.seen('X')){
                if(absPos) nextPos.x = offset.x + parser.value_float()*1000;
                else nextPos.x = currentPos.x + parser.value_float()*1000;
                //printf("Found X-> %f\n", parser.value_float());
            }
            if (parser.seen('Y')){
                if(absPos) nextPos.y = offset.y + parser.value_float()*1000;
                else nextPos.y = currentPos.y + parser.value_float()*1000;
                //printf("Found Y-> %f\n", parser.value_float());
            }
            if (parser.seen('Z')){
                if(absPos) nextPos.z = offset.z + parser.value_float()*1000;
                else nextPos.z = currentPos.z + parser.value_float()*1000;
                //printf("Found Z-> %f\n", parser.value_float());
            }
            if (parser.seen('E')){
                if(absEPos) nextE = offsetE + parser.value_float();
                else nextE = currentE + parser.value_float();
                //printf("Found E-> %f\n", parser.value_float());
            }
            break;

        case 28:
            nextPos = Vec3();
            nextE = 0.0f;
            break;

        case 90:
            absPos = true;
            absEPos = true;
            break;

        case 91:
            absPos = false;
            absEPos = false;
            break;

        case 92:
            if (parser.seen('X')){
                offset.x = currentPos.x - parser.value_float()*1000;
            }
            if (parser.seen('Y')){
                offset.y = currentPos.y - parser.value_float()*1000;
            }
            if (parser.seen('Z')){
                offset.z = currentPos.z - parser.value_float()*1000;
            }
            if (parser.seen('E')){
                offsetE = currentE - parser.value_float();
            }
            break;
        
        default:
            break;
        }
        break;

    case 'M':
        switch (parser.codenum)
        {
        case 82:
            absEPos = true;
            break;

        case 83:
            absEPos = false;
            break;
        
        default:
            break;
        }
        break;
    
    default:
        break;
    }
    return true;
}

void GcodePreview_Scr::renderGCode(tftLCD *tft)
{
    if (readDone) return;

    for (uint8_t i = 0; i < 32; i++)
    {
        tft->fillRect(320, i*10, 160, 10, i << 11);
        tft->drawLine(470, i*10, 480, i*10, TFT_CYAN);
    }

    // Setup memory buffers
    readBuffer = (char*)malloc(bufferLen);
    if (readBuffer == NULL)
    {
        ESP_LOGE("GcodePreview_Scr", "Could not allocate memory for read buffer");
        readDone = true;
        return;
    }

    gCodeLine = (char*)malloc(maxLineLen);
    if (gCodeLine == NULL)
    {
        ESP_LOGE("GcodePreview_Scr", "Could not allocate memory for line buffer");
        readDone = true;
        return;
    }

    commentLine = (char*)malloc(maxLineLen);
    if (gCodeLine == NULL)
    {
        ESP_LOGE("GcodePreview_Scr", "Could not allocate memory for comment buffer");
        readDone = true;
        return;
    }

    uint16_t* Zbuffer = (uint16_t*) calloc(320*160, sizeof(uint16_t));
    if (Zbuffer == NULL)
    {
        ESP_LOGE("GcodePreview_Scr", "Could not allocate memory for Z buffer");
        readDone = true;
        return;
    }

    // Open G-Code
    GcodeFile = fopen(_UI->selectedFile.c_str(), "r");
    if (GcodeFile == NULL)
    {
        ESP_LOGE("GcodePreview_Scr", "Failed to open file \"%s\"", _UI->selectedFile.c_str());
        readDone = true;
        return;
    }
    
    eTime = esp_timer_get_time();
    TIC
    // Start reading
    while (!feof(GcodeFile))
    {
        if (!readLine()) break;

        if (processLine() && printStarted && nextE > currentE && !(currentPos == nextPos))
        {
            // Transform points to camera coordinates
            Vec3 p1(currentPos.x - camPos.x, camPos.z - currentPos.z, currentPos.y - camPos.y);
            Vec3 p2(nextPos.x - camPos.x, camPos.z - nextPos.z, nextPos.y - camPos.y);

            if(p1.z > 0 && p2.z > 0)
            {
                // Apply perspective transformation
                Vec3 d1(p1.x*near / p1.z, p1.y*near / p1.z, p1.z);
                Vec3 d2(p2.x*near / p2.z, p2.y*near / p2.z, p2.z);

                Vec3f dir = p2-p1;
                dir.Normalize();
                uint32_t color = 23 * abs(dir*light) + 8;
                
                tft->drawLine(160 + d1.x, 160 + d1.y, 160 + d2.x, 160 + d2.y, color << 11);
            }
            else
            {
                if (p1.z <= 0)
                    ESP_LOGE("GcodePreview_Scr", "p1.Z(%d) <= 0!", p1.z);

                if (p2.z <= 0)
                    ESP_LOGE("GcodePreview_Scr", "p2.Z(%d) <= 0!", p2.z);
            }
        }

        currentPos = nextPos;
        currentE = nextE;
        if (esp_timer_get_time() - eTime > 1000000)
        {
            vTaskDelay(2);
            eTime = esp_timer_get_time();
        }
    }
    TOC

    // Close file
    fclose(GcodeFile);
    free(readBuffer);
    free(gCodeLine);
    free(commentLine);
    free(Zbuffer);

    readDone = true;
}

void GcodePreview_Scr::parseComment(const char* line)
{
    char *p;
    if ((p = strstr(line, "MINX:")))
    {
        minPos.x = strtof(&p[5], nullptr)*1000;
    }
    else if ((p = strstr(line, "MAXX:")))
    {
        maxPos.x = strtof(&p[5], nullptr)*1000;
    }
    else if ((p = strstr(line, "MINY:")))
    {
        minPos.y = strtof(&p[5], nullptr)*1000;
    }
    else if ((p = strstr(line, "MAXY:")))
    {
        maxPos.y = strtof(&p[5], nullptr)*1000;
    }
    else if ((p = strstr(line, "MINZ:")))
    {
        minPos.z = strtof(&p[5], nullptr)*1000;
    }
    else if ((p = strstr(line, "MAXZ:")))
    {
        maxPos.z = strtof(&p[5], nullptr)*1000;
    }
    else if ((p = strstr(line, "Generated")))
    {
        int32_t x = (maxPos.x + minPos.x) / 2;
        int32_t z = (maxPos.z + minPos.z) / 2;
        int32_t y1 = (160 * minPos.y - (maxPos.x - minPos.x)*near/2) / 160;
        int32_t y2 = (160 * minPos.y - (maxPos.z - minPos.z)*near/2) / 160;
        camPos = Vec3(x, min(y1, y2), z);
        ESP_LOGD("GcodePreview_Scr", "camPos(%d, %d, %d)", camPos.x, camPos.y, camPos.z);
    }
    else if ((p = strstr(line, "LAYER:0")))
    {
        printStarted = true;
    }
    else
    {
        ESP_LOGV("GcodePreview_Scr", "Unseen comment: %s\n", line);
    }
}

void GcodePreview_Scr::update(const uint32_t deltaTime)
{
    
}

void GcodePreview_Scr::render(tftLCD *tft)
{
    renderGCode(tft);
    //raster(tft);
    //tft->fillRect(pos.x-8, pos.y-8, 16, 16, TFT_BLACK);
    //tft->fillCircle(pos.x, pos.y, 5, reColor);
}

void GcodePreview_Scr::raster(tftLCD *tft)
{
    //tft->fillRect(0, 0, 320, 320, TFT_BLACK);

    camPos = Vec3(0, -86, 100);
    int32_t cube[8][3] = {{-50,-50, 50},{-50,-50,150},{-50,50,50},{50,-50,50},{50,50,50},{-50,50,150},{50,-50,150},{50,50,150}};
    int32_t line[12][2] = {{0,1},{0,2},{0,3},{1,5},{1,6},{2,4},{2,5},{3,4},{3,6},{4,7},{5,7},{6,7}};

    int32_t px1;
    int32_t py1;
    int32_t px2;
    int32_t py2;

    for (uint8_t i = 0; i < 12; i++)
    {
        // Transform points to camera coordinates
        Vec3 p1(cube[line[i][0]][0] - camPos.x, camPos.z - cube[line[i][0]][2], cube[line[i][0]][1] - camPos.y);
        Vec3 p2(cube[line[i][1]][0] - camPos.x, camPos.z - cube[line[i][1]][2], cube[line[i][1]][1] - camPos.y);

        // Apply perspective transformation
        px1 = p1.x*near / p1.z;
        py1 = p1.y*near / p1.z;
        px2 = p2.x*near / p2.z;
        py2 = p2.y*near / p2.z;

        tft->drawLine(160+px1, 160+py1, 160+px2, 160+py2, TFT_CYAN);
        //printf("%d:From (%d, %d) to (%d, %d)\n", i, px1, py1, px2, py2);
    }
}
