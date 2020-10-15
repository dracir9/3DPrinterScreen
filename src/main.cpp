/*
    Have a FAT Formatted SD Card connected to the SPI port of the ESP32
    File extensions with more than 3 charecters are not supported by the SD Library
    File Names longer than 8 charecters will be truncated by the SD library, so keep filenames shorter
    index.htm is the default index (works on subfolders as well)

    Access the editor by going to http://esp32sd.local/edit

*/

#include "main.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "main"

//############################################################
//  CONFIGURATION
//############################################################

// Touch Screen
#define MINPRESSURE 500
#define MAXPRESSURE 6400
#define TOUCH_GRID_X 3
#define TOUCH_GRID_Y 4
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
uint16_t brushColor = YELLOW;

//###########################################################
//  GLOBAL VARIALBES
//###########################################################

lcdUI UI;
TouchScreen ts = TouchScreen(TOUCH_PIN_XP, TOUCH_PIN_YP, TOUCH_PIN_XM, TOUCH_PIN_YM, 300);

//###########################################################
//  FUNCTIONS
//###########################################################
bool touchMapXY(uint16_t &x, uint16_t &y)
{
    TSPoint p = ts.getPoint();

    if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
    {
        x = map(p.y, 0, 4095, 0, 480);
        y = map(p.x, 0, 4095, 320, 0);
        return true;
    }
    return false;
}

void setup(void)
{
    Serial.begin(115200);
    printf("Free Heap at start: %d of %d\n", ESP.getFreeHeap(), ESP.getHeapSize());

    UI.tft.begin();
    UI.tft.setRotation(1);
    UI.tft.fillScreen(BLACK);
    UI.tft.print("(0,0)");
    UI.tft.setCursor(400, 0);
    UI.tft.print("(400,0)");
    UI.tft.setCursor(400, 280);
    UI.tft.print("(400,280)");
    UI.tft.setCursor(0, 280);
    UI.tft.print("(0,280)");
    UI.tft.fillCircle(25, 25, 2, GREEN);
    UI.tft.fillCircle(400, 25, 2, BLUE);
    UI.tft.fillCircle(400, 300, 2, RED);
    UI.tft.fillCircle(25, 300, 2, YELLOW);

    UI.setScreen(UI.FileBrowser);

    ts.enableRestore();

    //setupServer();

    ESP_LOGD(TAG, "Free end setup: %d\n", ESP.getFreeHeap());
}

bool flag = true;
unsigned long cnt = 0;
unsigned long touchTime = 0;

void loop(void)
{
    if (flag && millis() % 5000 < 100)
    {
        ESP_LOGD(TAG ,"Free Heap: %d of %d", ESP.getFreeHeap(), ESP.getHeapSize());

        //if (!UI.setScreen(UI.info))
        //    UI.setScreen(UI.FileBrowser);
        ESP_LOGD(TAG, "FPS: %lu\nFrame update time: %d µs", cnt/5, UI.getUpdateTime());
        cnt = 0;
        flag = false;
    }
    else if (!flag && millis() % 5000 > 500)
    {
        flag = true;
    }

    // display touched point with colored dot
    uint16_t pixel_x, pixel_y;
    unsigned long start = micros();
    bool valid = touchMapXY(pixel_x, pixel_y);
    unsigned long end = micros();
    if (valid)
    {
        UI.tft.fillCircle(pixel_x, pixel_y, 2, brushColor);
        //printf("X: %d, Y: %d\n", pixel_x, pixel_y);
        touchTime = max(end-start, touchTime);
        printf("%lu\n", touchTime);
    }

    if (Serial.available())
    {
        readPrinter();
    }

    // Keep this at the end of the loop
    if (UI.updateDisplay(60)) cnt++;
}