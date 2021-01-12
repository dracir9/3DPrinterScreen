
#include "utility.h"

#ifdef TIMER_U
    int64_t Timer::startTime;

    void Timer::tic()
    {
        startTime = esp_timer_get_time();
    }

    const int64_t Timer::toc()
    {
        int64_t eTime = esp_timer_get_time()-startTime;
        ESP_LOGD("Utility", "\nElapsed time: %lu\n", (unsigned long)eTime);
        return eTime;
    }
#endif

#ifdef SCREEN_SERVER_U
    bool ScreenServer::screenServer(TFT_eSPI& tft)
    {
        // With no filename the screenshot will be saved with a default name e.g. tft_screen_#.xxx
        // where # is a number 0-9 and xxx is a file type specified below
        return screenServer(DEFAULT_FILENAME, tft);
    }

    bool ScreenServer::screenServer(String filename, TFT_eSPI& tft)
    {
        uint32_t clearTime;
        uint32_t lastCmdTime = millis();     // Initialise start of command time-out

        // Check serial buffer
        if (Serial.available() > 0) {
            // Read the command byte
            uint8_t cmd = Serial.read();
            // If it is 'S' (start command) then clear the serial buffer for 100ms and stop waiting
            if ( cmd == 'S' ) {
                // Precautionary receive buffer garbage flush for 50ms
                clearTime = millis() + 50;
                while ( millis() < clearTime && Serial.read() >= 0) delay(1);

                lastCmdTime = millis(); // Set last received command time

                // Send screen size etc using a simple header with delimiters for client checks
                sendParameters(filename, tft);
            }
            else
                return false;
        }
        else
            return false;

        uint8_t color[3 * NPIXELS]; // RGB and 565 format color buffer for N pixels

        // Send all the pixels on the whole screen
        for ( uint32_t y = 0; y < tft.height(); y++)
        {
            // Increment x by NPIXELS as we send NPIXELS for every byte received
            for ( uint32_t x = 0; x < tft.width(); x += NPIXELS)
            {
                yield();

                // Wait here for serial data to arrive or a time-out elapses
                while ( Serial.available() == 0 )
                {
                    if ( millis() > lastCmdTime + PIXEL_TIMEOUT) return false;
                    delay(1);
                }

                // Serial data must be available to get here, read 1 byte and
                // respond with N pixels, i.e. N x 3 RGB bytes or N x 2 565 format bytes
                if ( Serial.read() == 'X' ) {
                    // X command byte means abort, so clear the buffer and return
                    clearTime = millis() + 50;
                    while ( millis() < clearTime && Serial.read() >= 0) yield();
                    return false;
                }
                // Save arrival time of the read command (for later time-out check)
                lastCmdTime = millis();

            #if defined BITS_PER_PIXEL && BITS_PER_PIXEL >= 24 && NPIXELS > 1
                // Fetch N RGB pixels from x,y and put in buffer
                tft.readRectRGB(x, y, NPIXELS, 1, color);
                // Send buffer to client
                Serial.write(color, 3 * NPIXELS); // Write all pixels in the buffer
            #else
                // Fetch N 565 format pixels from x,y and put in buffer
                #if NPIXELS > 1
                    tft.readRect(x, y, NPIXELS, 1, (uint16_t *)color);
                #else
                    uint16_t c = tft.readPixel(x, y);
                    color[0] = c>>8;
                    color[1] = c & 0xFF;  // Swap bytes
                #endif
                // Send buffer to client
                Serial.write(color, 2 * NPIXELS); // Write all pixels in the buffer
            #endif
            }
        }

        Serial.flush(); // Make sure all pixel bytes have been despatched

        return true;
    }

    void ScreenServer::sendParameters(String filename, TFT_eSPI& tft)
    {
        Serial.write('W'); // Width
        Serial.write(tft.width()  >> 8);
        Serial.write(tft.width()  & 0xFF);

        Serial.write('H'); // Height
        Serial.write(tft.height() >> 8);
        Serial.write(tft.height() & 0xFF);

        Serial.write('Y'); // Bits per pixel (16 or 24)
        if (NPIXELS > 1) Serial.write(BITS_PER_PIXEL);
        else Serial.write(16); // readPixel() only provides 16 bit values

        Serial.write('?'); // Filename next
        Serial.print(filename);

        Serial.write('.'); // End of filename marker

        Serial.write(FILE_EXT); // Filename extension identifier

        Serial.write(*FILE_TYPE); // First character defines file type j,b,p,t
    }
#endif
