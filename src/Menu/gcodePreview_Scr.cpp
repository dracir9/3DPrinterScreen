
#include "gcodePreview_Scr.h"

GcodePreview_Scr::GcodePreview_Scr(lcdUI* UI)
{
    UI->tft.fillScreen(TFT_BLACK);
    GcodeFile = UI-> selectedFile;
}

bool GcodePreview_Scr::readLine()
{
    bool commentMode;
    uint16_t len;

    do {
        commentMode = false;
        len = 0;
        int c;
        // Get one line
        while(len < MAX_LINE_LEN - 1)
        {
            c = GcodeFile.read();
            if(c <= 0 || c  == '\r' || c == '\n') {
                break;
            }
            else if(c == ';'){  // If there is a comment just end the string
                commentMode = true;
            }

            if (!commentMode) GcodeLine[len++] = (char) c;
        }

        if (len >= MAX_LINE_LEN - 1) // 
        {
            ESP_LOGE("GcodePreview_Scr", "Very long line in file \"%s\". ABORT!", GcodeFile.name());
            return false;
        }
        if (c < 0 && GcodeFile.available())
        {
            ESP_LOGE("GcodePreview_Scr", "Error reading file \"%s\"", GcodeFile.name());
            return false;
        }
    } while(!len && GcodeFile.available());

    GcodeLine[len] = '\0'; // End line
    return true;
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
                printf("Found X-> %f\n", parser.value_float());
            }
            if (parser.seen('Y')){
                printf("Found Y-> %f\n", parser.value_float());
            }
            if (parser.seen('Z')){
                printf("Found Z-> %f\n", parser.value_float());
            }
            if (parser.seen('E')){
                printf("Found E-> %f\n", parser.value_float());
            }
            break;
        
        default:
            break;
        }
        break;

    case 'M':
        switch (parser.codenum)
        {
        case 32:
            if (parser.seen('B')){
                printf("Found B-> %f\n", parser.value_float());
            }
            if (parser.seen('S')){
                printf("Found S-> %f\n", parser.value_float());
            }
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

void GcodePreview_Scr::update(uint32_t deltaTime)
{
    int line = 0;
    while (GcodeFile.available() && !readDone)
    {
        if (!readLine()) break;
        
        processComand();

        printf("Line: %d >>%s<<\n", line, GcodeLine);
        line++;
    }
    fflush(stdout);
    readDone = true;
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
    tft->fillRect(pos.x-8, pos.y-8, 16, 16, TFT_BLACK);
    tft->fillCircle(pos.x, pos.y, 5, reColor);
}
