
#include "gcodePreview_Scr.h"

GcodePreview_Scr::GcodePreview_Scr(lcdUI* UI)
{
    _UI = UI;
    _UI->tft.fillScreen(TFT_BLACK);
    _UI->tft.drawRect(0, 0, 320, 320, TFT_GREEN);
    _UI->tft.drawRect(32, 32, 256, 256, TFT_OLIVE);
    _UI->tft.drawRect(320, 270, 160, 50, TFT_ORANGE);
    _UI->tft.setTextFont(2);
    _UI->tft.setTextDatum(CC_DATUM);
    _UI->tft.setTextPadding(0);
    _UI->tft.drawString("Return", 400, 295);
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
        if (readBuffer[bufPos] <= 0 || readBuffer[bufPos] == '\r' || readBuffer[bufPos] == '\n') // End of line encountered
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
    // Parse comments for useful information
    if (strlen(commentLine) > 5)
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

    Zbuffer = (uint16_t*) calloc(320*160, sizeof(uint16_t));
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
    
    uint32_t lines = 0;
    eTime = esp_timer_get_time();
    TIC
    // Start reading
    while (!feof(GcodeFile) && ! readDone)
    {
        if (!readLine()) break;

        if (processLine() && draw && nextE > currentE && !(currentPos == nextPos))
        {
            // Transform points to camera coordinates
            Vec3 p1(currentPos.x - camPos.x, camPos.z - currentPos.z, currentPos.y - camPos.y);
            Vec3 p2(nextPos.x - camPos.x, camPos.z - nextPos.z, nextPos.y - camPos.y);

            if(p1.z > 0 && p2.z > 0)
            {
                // Apply perspective transformation
                Vec3 d1(p1.x*near / p1.z, p1.y*near / p1.z, p1.z);
                Vec3 d2(p2.x*near / p2.z, p2.y*near / p2.z, p2.z);

                if (!(d1 == d2))
                {
                    Vec3f dir = p2-p1;
                    dir.Normalize();
                    uint32_t color = (uint32_t)(23 * abs(dir*light) + 8) << 11;
                
                    tft->drawLine(160 + d1.x, 160 + d1.y, 160 + d2.x, 160 + d2.y, color);
                    lines++;
                }
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
        if (esp_timer_get_time() - eTime > 1000000) // Allow some time for other tasks
        {
            vTaskDelay(2);
            eTime = esp_timer_get_time();
        }
    }
    TOC
    printf("Total Lines: %d\n", lines);

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
    else if (strstr(line, "Generated"))
    {
        int32_t x = (maxPos.x + minPos.x) / 2;
        int32_t z = (maxPos.z + minPos.z) / 2;
        int32_t y1 = (160 * minPos.y - (maxPos.x - minPos.x)*near/2) / 160;
        int32_t y2 = (160 * minPos.y - (maxPos.z - minPos.z)*near/2) / 160;
        camPos = Vec3(x, min(y1, y2), z);
        ESP_LOGD("GcodePreview_Scr", "camPos(%d, %d, %d)", camPos.x, camPos.y, camPos.z);
    }
    else if (strstr(line, "LAYER:0"))
    {
        draw = true;
    }
    else if ((p = strstr(line, "TYPE:")))
    {
        if (strstr(line, "FILL") || strstr(line, "INNER"))
            draw = false;
        else
            draw = true;
    }
    else
    {
        ESP_LOGV("GcodePreview_Scr", "Unseen comment: %s\n", line);
    }
}

void GcodePreview_Scr::drawLineZbuf(tftLCD *tft, Vec3 u, Vec3 v, const uint32_t color)
{
    // Other
    int32_t dx = abs(v.x - u.x),
    dy = abs(v.y - u.y);

    int32_t* i;     // Major iterator
    int32_t* j;     // Minor iterator
    int32_t end;    // End point
    int32_t step;

    if (dx >= dy) {
        if (u.x > v.x) swap_coord(u, v);
        i = &u.x;
        j = &u.y;
        end = v.x;
        step = (u.y > v.y)? -1 : 1;
    }
    else{
        if (u.y > v.y) swap_coord(u, v);
        swap_coord(dx, dy);
        i = &u.y;
        j = &u.x;
        end = v.y;
        step = (u.x > v.x)? -1 : 1;
    }

    int32_t err = dx >> 1;
    int32_t zi = u.z, xi = *i;

    for (; *i <= end; (*i)++) {
        drawPixelZbuf(tft, u, color);
        err -= dy;
        if (err < 0) {
            err += dx;
            *j += step;
        }
        u.z = map(*i, xi, end, zi, v.z);
        //u.z = zi + dz*(xi-*i)/dx;
    }
}

void GcodePreview_Scr::drawPixelZbuf(tftLCD *tft, Vec3 p, const uint32_t color)
{
    //tft->drawPixel(p.x, p.y, color);
    tft->drawPixel(p.x, p.z, TFT_YELLOW);
}

void GcodePreview_Scr::update(const uint32_t deltaTime)
{
    angle += omega*deltaTime/1000000;
}

void GcodePreview_Scr::render(tftLCD *tft)
{
    renderGCode(tft);
    drawLineZbuf(tft, Vec3(240,160,160), Vec3(240 + 100*cos(angle), 160 + 100*sin(angle), 160 + 100*sin(angle)), TFT_WHITE);
    //raster(tft);
    //tft->fillRect(pos.x-8, pos.y-8, 16, 16, TFT_BLACK);
    //tft->fillCircle(pos.x, pos.y, 5, reColor);
}

void GcodePreview_Scr::handleTouch(const Screen::touchEvent event, const Vec2h pos)
{
    if (event == press && pos.x > 320)
    {
        if(pos.y > 270)
        {
            readDone = true;
            _UI->selectedFile.clear();
            _UI->setScreen(lcdUI::FileBrowser);
        }
    }
}

void GcodePreview_Scr::raster(tftLCD *tft)
{
    //tft->fillRect(0, 0, 320, 320, TFT_BLACK);

    int32_t cube[8][3] = {{minPos.x, minPos.y, minPos.z},{minPos.x, minPos.y, maxPos.z},{minPos.x, maxPos.y, minPos.z},{maxPos.x, minPos.y, minPos.z},
                        {maxPos.x, maxPos.y, minPos.z},{minPos.x, maxPos.y, maxPos.z},{maxPos.x, minPos.y, maxPos.z},{maxPos.x, maxPos.y, maxPos.z}};
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
