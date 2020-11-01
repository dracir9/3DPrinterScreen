
#include "gcodePreview_Scr.h"

#define SCALE 5
#define MOVEX 400
#define MOVEY 400

GcodePreview_Scr::GcodePreview_Scr(lcdUI* UI)
{
    _UI = UI;
    UI->tft.fillScreen(TFT_BLACK);
}

bool GcodePreview_Scr::readLine()
{
    bool commentMode = false;
    for (int16_t i = 0; i < MAX_LINE_LEN; i++)
    {
        if (bufPos >= readLen)
        {
            readLen = fread(RBuffer, 1, BUFFER_LEN, GcodeFile);
            if (readLen <= 0)
            {
                return false;
            }
            else if (ferror(GcodeFile))
            {
                ESP_LOGE("GcodePreview_Scr", "Error reading file \"%s\"", _UI->selectedFile);              
            }
            bufPos = 0;
        }
        if (RBuffer[bufPos] <= 0 || RBuffer[bufPos] == '\n' || RBuffer[bufPos] <= '\r')
        {
            GcodeLine[i] = '\0';
            if (i == 0) // If empty line keep reading
            {
                i = -1;
                bufPos++;
                continue;
            }
            return true;
        }
        else if (RBuffer[bufPos] == ';')
        {
            commentMode = true;
            GcodeLine[i] = '\0';
        }

        if (!commentMode) GcodeLine[i] = RBuffer[bufPos];

        bufPos++;
    }

    ESP_LOGE("GcodePreview_Scr", "Very long line in file \"%s\". ABORT!", _UI->selectedFile);
    return false;
}

bool GcodePreview_Scr::processComand()
{
    // Should be safe to parse
    parser.parse(GcodeLine);

    switch (parser.command_letter)
    {
    case 'G':
        switch (parser.codenum)
        {
        case 0: case 1:
            if (parser.seen('X')){
                if(absPos) nextPos.x = offset.x + parser.value_float()*SCALE;
                else nextPos.x = currentPos.x + parser.value_float()*SCALE;
                //printf("Found X-> %f\n", parser.value_float());
            }
            if (parser.seen('Y')){
                if(absPos) nextPos.y = offset.y + parser.value_float()*SCALE;
                else nextPos.y = currentPos.y + parser.value_float()*SCALE;
                //printf("Found Y-> %f\n", parser.value_float());
            }
            if (parser.seen('Z')){
                if(absPos) nextPos.z = offset.z + parser.value_float()*SCALE;
                else nextPos.z = currentPos.z + parser.value_float()*SCALE;
                //printf("Found Z-> %f\n", parser.value_float());
            }
            if (parser.seen('E')){
                if(absEPos) nextE = offsetE + parser.value_float()*SCALE;
                else nextE = currentE + parser.value_float()*SCALE;
                //printf("Found E-> %f\n", parser.value_float());
            }
            break;

        case 28:
            nextPos = Vector3<int32_t>();
            nextE = 0;
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
                offset.x = currentPos.x;
            }
            if (parser.seen('Y')){
                offset.y = currentPos.y;
            }
            if (parser.seen('Z')){
                offset.z = currentPos.z;
            }
            if (parser.seen('E')){
                offsetE = currentE;
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
    int line = 0;

    // Setup memory buffers
    RBuffer = (char*)malloc(BUFFER_LEN);
    if (RBuffer == NULL)
    {
        ESP_LOGE("GcodePreview_Scr", "Could not allocate memory for Read Buffer");
        readDone = true;
        return;
    }

    GcodeLine = (char*)malloc(MAX_LINE_LEN);
    if (GcodeLine == NULL)
    {
        ESP_LOGE("GcodePreview_Scr", "Could not allocate memory for line Buffer");
        readDone = true;
        return;
    }

    // Open G-Code
    GcodeFile = fopen(_UI->selectedFile, "r");
    if (GcodeFile == NULL)
    {
        ESP_LOGE("GcodePreview_Scr", "Failed to open file \"%s\"", _UI->selectedFile);
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
            tft->drawLine(currentPos.x-MOVEX, currentPos.y-MOVEY, nextPos.x-MOVEX, nextPos.y-MOVEY, TFT_RED);
            //printf("Line from (%d, %d) to (%d, %d)\n", currentPos.x, currentPos.y, nextPos.x, nextPos.y);
        }

        currentPos = nextPos;
        currentE = nextE;
        reColor += 10;

        //printf("Line: %d >>%s<<\n", line, GcodeLine);
        line++;
    }

    TOC
    // Close file
    fclose(GcodeFile);
    free(RBuffer);
    free(GcodeLine);

    readDone = true;
}

void GcodePreview_Scr::update(uint32_t deltaTime)
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
