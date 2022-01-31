
#include "gcodePreview_Scr.h"

char GcodePreview_Scr::fileBuffer[];

GcodePreview_Scr::GcodePreview_Scr(lcdUI* UI, tftLCD& tft):
    Screen(UI), img(&tft.img)
{
    displayed.set();
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
}

GcodePreview_Scr::~GcodePreview_Scr()
{
    if (Zbuffer)
        free(Zbuffer);
    if (GcodeFile)
        fclose(GcodeFile);
    if (readBuffer)
        free(readBuffer);
    if (gCodeLine)
        free(gCodeLine);
    if (commentLine)
        free(commentLine);
}

bool GcodePreview_Scr::readLine()
{
    bool commentMode = false;
    *commentLine = '\0';
    uint8_t j = 0;
    for (int16_t i = 0; i < maxLineLen; i++, bufPos++)
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
                ESP_LOGE("GcodePreview_Scr", "Error reading file \"%s\"", _UI->getFile().c_str());
                return false;
            }
            bufPos = 0;
        }

        char inChar = readBuffer[bufPos];
        if (inChar < 32) // End of line encountered
        {
            gCodeLine[i] = '\0';    // Ensure null-terminated string
            commentLine[j] = '\0';
            if (i == 0 && j == 0) // If empty line keep reading
            {
                i = -1;
                continue;
            }
            return true;
        }
        else if (inChar == ';')
        {
            commentMode = true;
            gCodeLine[i] = '\0';
        }

        if (commentMode)
            commentLine[j++] = inChar;
        else
            gCodeLine[i] = inChar; // Fill line buffer
    }

    ESP_LOGE("GcodePreview_Scr", "Very long line in file \"%s\". ABORT!", _UI->getFile().c_str());
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
                if(absPos) nextPos.x = offset.x + parser.value_float();
                else nextPos.x = currentPos.x + parser.value_float();
                //printf("Found X-> %f\n", parser.value_float());
            }
            if (parser.seen('Y')){
                if(absPos) nextPos.y = offset.y + parser.value_float();
                else nextPos.y = currentPos.y + parser.value_float();
                //printf("Found Y-> %f\n", parser.value_float());
            }
            if (parser.seen('Z')){
                if(absPos) nextPos.z = offset.z + parser.value_float();
                else nextPos.z = currentPos.z + parser.value_float();
                //printf("Found Z-> %f\n", parser.value_float());
            }
            if (parser.seen('E')){
                if(absEPos) nextE = offsetE + parser.value_float();
                else nextE = currentE + parser.value_float();
                //printf("Found E-> %f\n", parser.value_float());
            }
            break;

        case 28:
            nextPos = Vec3f();
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
                offset.x = currentPos.x - parser.value_float();
            }
            if (parser.seen('Y')){
                offset.y = currentPos.y - parser.value_float();
            }
            if (parser.seen('Z')){
                offset.z = currentPos.z - parser.value_float();
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

bool GcodePreview_Scr::initRender()
{
    // Setup memory buffers
    readBuffer = (char*)malloc(bufferLen);
    if (readBuffer == NULL)
    {
        ESP_LOGE("GcodePreview_Scr", "Could not allocate memory for read buffer");
        goto init_fail;
    }

    gCodeLine = (char*)malloc(maxLineLen);
    if (gCodeLine == NULL)
    {
        ESP_LOGE("GcodePreview_Scr", "Could not allocate memory for line buffer");
        goto init_fail;
    }

    commentLine = (char*)malloc(maxLineLen);
    if (gCodeLine == NULL)
    {
        ESP_LOGE("GcodePreview_Scr", "Could not allocate memory for comment buffer");
        goto init_fail;
    }

    // ZBuffer maps to only half of the screen to save RAM.
    Zbuffer = (float*) ps_calloc(320*320, sizeof(float));
    if (Zbuffer == NULL)
    {
        ESP_LOGE("GcodePreview_Scr", "Could not allocate memory for Z buffer");
        goto init_fail;
    }
    memset(Zbuffer, 0x7F, 320*320*sizeof(float));

    // Create sprite
    img->setColorDepth(16);
    if (img->createSprite(320, 320) == NULL)
    {
        ESP_LOGE("GcodePreview_Scr", "Failed to create sprite");
        goto init_fail;
    }
    img->fillSprite(TFT_BLACK);
    img->drawRoundRect(0, 0, 320, 320, 4, TFT_GREEN);

    // Open G-Code
    GcodeFile = fopen(_UI->getFile().c_str(), "r");
    if (GcodeFile == NULL)
    {
        ESP_LOGE("GcodePreview_Scr", "Failed to open file \"%s\"", _UI->getFile().c_str());
        goto init_fail;
    }

    setvbuf(GcodeFile, fileBuffer, _IOFBF, fileBufLen);

    fseek(GcodeFile, 0, SEEK_END);
    filesize = ftell(GcodeFile);
    rewind(GcodeFile);

    return true;

    init_fail:
    readState = 255;
    return false;
}

void GcodePreview_Scr::renderGCode(tftLCD& tft)
{
    switch (readState)
    {
    case 255:
        return;

    case 0:
        if (initRender())
        {
            readState = 1;
            TIC
        }
        break;

    case 1:
    {
        eTime = esp_timer_get_time();
        // Start reading
        while (esp_timer_get_time() - eTime < maxWorkTime && readLine() && readState == 1)
        {
            if (processLine() && draw && nextE > currentE && !(currentPos == nextPos))
            {
                // Transform points to camera coordinates
                Vec3f p1(currentPos.x - camPos.x, camPos.z - currentPos.z, currentPos.y - camPos.y);
                Vec3f p2(nextPos.x - camPos.x, camPos.z - nextPos.z, nextPos.y - camPos.y);

                // Apply perspective transformation
                float invZ1 = 1.0f/p1.z;
                float invZ2 = 1.0f/p2.z;
                Vec3f d1(p1.x*near*invZ1, p1.y*near*invZ1, p1.z);
                Vec3f d2(p2.x*near*invZ2, p2.y*near*invZ2, p2.z);

                if (!(d1 == d2))
                {
                    Vec3f dir = p2-p1;
                    dir.Normalize();
                    uint32_t color = (uint32_t)(23 * abs(dir*light) + 8) << 11;
                
                    static const Vec3f scrOff(160.0f, 160.0f, 0.0f);
                    drawLineZbuf(tft, scrOff + d1, scrOff + d2, color);
                    lines++;
                }
            }

            currentPos = nextPos;
            currentE = nextE;
        }

        tft.setTextPadding(0);
        tft.setTextDatum(CC_DATUM);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(String(ftell(GcodeFile)*100/filesize) + String("%"), 160, 168);

        if (feof(GcodeFile)) readState = 2;
        break;
    }
    case 2:
        TOC
        ESP_LOGD(__FILE__, "Total Lines: %d", lines);
        img->pushSprite(0, 0);
        // Close file
        fclose(GcodeFile);
        free(readBuffer);
        free(gCodeLine);
        free(commentLine);
        free(Zbuffer);
        GcodeFile = nullptr;
        readBuffer = nullptr;
        gCodeLine = nullptr;
        commentLine = nullptr;
        Zbuffer = nullptr;
        img->deleteSprite();

        readState = 255;
        break;

    default:
        ESP_LOGE(__FILE__, "Unexpected state reached!");
        readState = 255;
        break;
    }
}

void GcodePreview_Scr::parseComment(const char* line)
{
    // Only tested with Ultimaker Cura
    char *p;
    if ((p = strstr(line, "MINX:")))
    {
        minPos.x = strtof(&p[5], nullptr);
    }
    else if ((p = strstr(line, "MAXX:")))
    {
        maxPos.x = strtof(&p[5], nullptr);
    }
    else if ((p = strstr(line, "MINY:")))
    {
        minPos.y = strtof(&p[5], nullptr);
    }
    else if ((p = strstr(line, "MAXY:")))
    {
        maxPos.y = strtof(&p[5], nullptr);
    }
    else if ((p = strstr(line, "MINZ:")))
    {
        minPos.z = strtof(&p[5], nullptr);
    }
    else if ((p = strstr(line, "MAXZ:")))
    {
        maxPos.z = strtof(&p[5], nullptr);
    }
    else if (strstr(line, "Generated"))
    {
        int32_t x = (maxPos.x + minPos.x) / 2;
        int32_t z = (maxPos.z + minPos.z) / 2;
        int32_t y1 = (160 * minPos.y - (maxPos.x - minPos.x)*near/2) / 160;
        int32_t y2 = (160 * minPos.y - (maxPos.z - minPos.z)*near/2) / 160;
        camPos = Vec3f(x, min(y1, y2), z);
        ESP_LOGD("GcodePreview_Scr", "camPos(%f, %f, %f)", camPos.x, camPos.y, camPos.z);
        zCmin = minPos.y - camPos.y;
        zCmax = maxPos.y - camPos.y;
    }
    else if (strstr(line, "LAYER:0"))
    {
        draw = true;
    }
    else if (strstr(line, "TYPE:"))
    {
        if (strstr(line, "FILL") || strstr(line, "INNER"))
            draw = false;
        else
            draw = true;
    }
    else if ((p = strstr(line, "Filament")))
    {
        filament = strtof(&p[15], nullptr);
        displayed[0] = false;
    }
    else if ((p = strstr(line, "TIME:")))
    {
        printTime = atoi(&p[5]);
        displayed[1] = false;
    }
    else
    {
        ESP_LOGV("GcodePreview_Scr", "Unseen comment: %s", line);
    }
}

void GcodePreview_Scr::drawLineZbuf(tftLCD& tft, Vec3f u, Vec3f v, const uint32_t color)
{
    // Other
    float dx = abs(v.x - u.x),
            dy = abs(v.y - u.y);

    float* i;     // Major iterator
    float* j;     // Minor iterator


    if (dx >= dy) {
        if (u.x > v.x) swap_coord(u, v);
        i = &u.x;
        j = &u.y;
    }
    else {
        if (u.y > v.y) swap_coord(u, v);
        swap_coord(dx, dy);
        i = &u.y;
        j = &u.x;
    }
    if (dx == 0.0f) dx = 1.0f;

    int32_t end = dx;
    dx = 1.0f/dx;
    float dz = (v.z - u.z)*dx;
    dy = (v.y - u.y)*dx;

    //printf("U(%f, %f, %f)  V(%f, %f, %f)\n", u.x, u.y, u.z, v.x, v.y, v.z);
    for (; end >= 0; end--) {
        //printf("(%f, %f, %f)\n", u.x, u.y, u.z);
        drawPixelZbuf(tft, u, color);
        *i += 1.0f;
        *j += dy;
        u.z += dz;
    }
}

void GcodePreview_Scr::drawPixelZbuf(tftLCD& tft, Vec3f p, const uint32_t color)
{
    int32_t x = p.x,
            y = p.y;
    uint32_t i = x + y*320.0f;

    if (p.z < Zbuffer[i])
    {
        Zbuffer[i] = p.z;
        img->drawPixel(x, y, color);
    }
}

void GcodePreview_Scr::update(const uint32_t deltaTime)
{
    if (!_UI->isSDinit())
    {
        _UI->setScreen(lcdUI::Info);
    }
}

void GcodePreview_Scr::render(tftLCD& tft)
{
    renderGCode(tft);
    drawInfo(tft);
}

void GcodePreview_Scr::handleTouch(const touchEvent event, const Vec2h pos)
{
    if (event == press && pos.x > 320)
    {
        if(pos.y > 270)
        {
            readState = 255;
            _UI->clearFile();
            _UI->setScreen(lcdUI::FileBrowser);
        }
    }
}

void GcodePreview_Scr::drawInfo(tftLCD& tft)
{
    if (displayed.all()) return;
    // Text settings
    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextPadding(0);
    tft.setTextColor(TFT_WHITE);
    String txt;

    if (!displayed[0])
    {
        tft.drawString("Filament cost: " , 328, 55);
        txt = String(filament, 3) + " m";
        tft.drawString(txt, 335, 71);
        displayed[0] = true;
    }

    if (!displayed[1])
    {
        tft.drawString("Estimated time: " , 328, 95);
        uint8_t seconds = printTime % 60;
        uint8_t minutes = printTime/60 % 60;
        uint8_t hours = printTime/3600 % 24;
        uint16_t days = printTime/86400;
        txt = "";
        if (days > 0) txt += String(days) + "d ";
        if (hours > 0) txt += String(hours) + "h ";
        if (minutes > 0) txt += String(minutes) + "min ";
        txt += String(seconds) + "s";
        tft.drawString(txt, 335, 111);
        displayed[1] = true;
    }
}
