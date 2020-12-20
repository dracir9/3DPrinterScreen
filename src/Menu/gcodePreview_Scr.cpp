
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
            gCodeLine[i] = '\0';
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

bool GcodePreview_Scr::processComand()
{

    // Parse comments for usefull information
    parseComment(commentLine);

    // Should be safe to parse
    parser.parse(gCodeLine);

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
            nextPos = Vector3<int32_t>();
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

    // Open G-Code
    GcodeFile = fopen(_UI->selectedFile.c_str(), "r");
    if (GcodeFile == NULL)
    {
        ESP_LOGE("GcodePreview_Scr", "Failed to open file \"%s\"", _UI->selectedFile.c_str());
        readDone = true;
        return;
    }

    TIC
    // Start reading
    while (!feof(GcodeFile))
    {
        if (!readLine()) break;

        processComand();

        if (nextE > currentE)
        {
            float scale = 256.0f / max(maxPos.x-minPos.x, maxPos.y-minPos.y);
            Vector3<int32_t> startVec = (currentPos - minPos)*scale;
            Vector3<int32_t> endVec = (nextPos - minPos)*scale;
            
            tft->drawLine(32 + startVec.x, 32 + startVec.y, 32 + endVec.x, 32 + endVec.y, TFT_RED);
            //printf("Line from (%d, %d) to (%d, %d)\n", currentPos.x, currentPos.y, nextPos.x, nextPos.y);
        }

        currentPos = nextPos;
        currentE = nextE;
        reColor += 10;
    }

    TOC
    // Close file
    fclose(GcodeFile);
    free(readBuffer);
    free(gCodeLine);
    free(commentLine);

    readDone = true;
}

void GcodePreview_Scr::parseComment(const char* line)
{
    char *p;
    p = strstr(line, "MINX:");
    if (p)
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
}

void GcodePreview_Scr::update(const uint32_t deltaTime)
{
    pos += vel * ((float)deltaTime/1000000.0f);
    if((pos.x > 474.0f && vel.x > 0) || (pos.x < 5.0f && vel.x < 0))
    {
        vel.x = -vel.x;
    }

    if((pos.y > 314.0f && vel.y > 0) || (pos.y < 5.0f && vel.y < 0))
    {
        vel.y = -vel.y;
    }

    reColor++;
}

void GcodePreview_Scr::render(tftLCD *tft)
{
    renderGCode(tft);
    //tft->fillRect(pos.x-8, pos.y-8, 16, 16, TFT_BLACK);
    //tft->fillCircle(pos.x, pos.y, 5, reColor);
}
