
#include "tftLCD.h"
#include <pgmspace.h>

/*####################################################
    tft LCD class
####################################################*/

/**
 * @brief Draw string wrapped
 * 
 * @param str   String to draw
 * @param x     String X position
 * @param y     String Y position
 * @param w     Max string width
 * @param h     Max string height
 */
void tftLCD::drawStringWr(const char *str, int32_t x, int32_t y, uint16_t w, uint16_t h)
{
    const uint16_t cheight = fontHeight();
    const int16_t lines = min(textWidth(str)/w, h/cheight-1);
    
    uint16_t len = strlen(str);
    uint16_t cnt = 0;
    const uint16_t maxChar = w/3;
    char buffer[maxChar+1];
    for (int16_t i = 0; i <= lines; i++)
    {
        uint8_t charN = maxChar;
        strncpy(buffer, &str[cnt], maxChar);
        buffer[maxChar] = '\0';

        while (textWidth(buffer) > w)
            buffer[--charN] = '\0';
            
        drawString(buffer, x, y - cheight/2*lines + i*cheight);

        cnt += charN;
        if (cnt > len) break; // All characters read
    }
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t SPIFFS_read16(FILE *f) {
    uint16_t result;
    ((uint8_t *)&result)[0] = fgetc(f); // LSB
    ((uint8_t *)&result)[1] = fgetc(f); // MSB
    return result;
}

uint32_t SPIFFS_read32(FILE *f) {
    uint32_t result;
    ((uint8_t *)&result)[0] = fgetc(f); // LSB
    ((uint8_t *)&result)[1] = fgetc(f);
    ((uint8_t *)&result)[2] = fgetc(f);
    ((uint8_t *)&result)[3] = fgetc(f); // MSB
    return result;
}

void tftLCD::drawBmpSPIFFS(const char *filename, int16_t x, int16_t y)
{
    if ((x >= _width) || (y >= _height)) return;

    // Open requested file on SPIFFS
    FILE *bmpFS = fopen(filename, "r");

    if (!bmpFS)
    {
        ESP_LOGE(__FILE__, "File \"%s\" not found!", filename);
        return;
    }

    uint32_t seekOffset;
    uint16_t w, h, row;

    if (SPIFFS_read16(bmpFS) == 0x4D42)
    {
        SPIFFS_read32(bmpFS);
        SPIFFS_read32(bmpFS);
        seekOffset = SPIFFS_read32(bmpFS);
        SPIFFS_read32(bmpFS);
        w = SPIFFS_read32(bmpFS);
        h = SPIFFS_read32(bmpFS);

        if (SPIFFS_read16(bmpFS) == 1 && SPIFFS_read16(bmpFS) == 16 && SPIFFS_read32(bmpFS) == 3)
        {
            y += h - 1;

            bool oldSwapBytes = getSwapBytes();
            setSwapBytes(true);
            fseek(bmpFS, seekOffset, SEEK_SET);
            uint8_t lineBuffer[w * 2 + ((w & 1) << 1)];

            for (row = 0; row < h; row++)
            {
                fread(lineBuffer, 1, sizeof(lineBuffer), bmpFS);

                // Push the pixel row to screen, pushImage will crop the line if needed
                // y is decremented as the BMP image is drawn bottom up
                pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
            }
            setSwapBytes(oldSwapBytes);
        }
        else
            ESP_LOGE(__FILE__, "BMP format not recognized.");
    }
    fclose(bmpFS);
}
