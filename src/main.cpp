/*
    Have a FAT Formatted SD Card connected to the SPI port of the ESP32
    File extensions with more than 3 charecters are not supported by the SD Library
    File Names longer than 8 charecters will be truncated by the SD library, so keep filenames shorter
    index.htm is the default index (works on subfolders as well)

    Access the editor by going to http://esp32sd.local/edit

*/

#include "main.h"

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

lcdUI tft;
TouchScreen ts = TouchScreen(TOUCH_PIN_XP, TOUCH_PIN_YP, TOUCH_PIN_XM, TOUCH_PIN_YM, 300);
unsigned long startTime = 0;

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
    Serial.print("Free Heap at start: "); //390000 +
    Serial.print(ESP.getFreeHeap());
    Serial.print(" of ");
    Serial.println(ESP.getHeapSize());
    Serial.println();

    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(BLACK);
    tft.print("(0,0)");
    tft.setCursor(400, 0);
    tft.print("(400,0)");
    tft.setCursor(400, 280);
    tft.print("(400,280)");
    tft.setCursor(0, 280);
    tft.print("(0,280)");
    tft.fillCircle(25, 25, 2, GREEN);
    tft.fillCircle(400, 25, 2, BLUE);
    tft.fillCircle(400, 300, 2, RED);
    tft.fillCircle(25, 300, 2, YELLOW);

    tft.setScreen(tft.info);

    ts.enableRestore();

    //setupServer();

    /*  if (SD.begin())
    {
        tft.println("SD Card initialized.");
        Serial.println("SD Card initialized.");
        hasSD = true;
    }*/

    Serial.print("Free end setup: ");
    Serial.println(ESP.getFreeHeap());
}

bool flag = true;
bool render = true;
unsigned long cnt = 0;
unsigned long touchTime = 0;

void loop(void)
{
    if (flag && millis() % 5000 < 100)
    {
        Serial.print("Free Heap: ");
        Serial.print(ESP.getFreeHeap());
        Serial.print(" of ");
        Serial.println(ESP.getHeapSize());

        if (!tft.setScreen(tft.info))
            tft.setScreen(tft.black);
        Serial.print("FPS: ");
        Serial.println(cnt/5);
        Serial.print("Frame update time: ");
        Serial.print(tft.getUpdateTime());
        Serial.println(" us");
        cnt = 0;
        flag = false;
    }
    else if (!flag && millis() % 5000 > 500)
    {
        flag = true;
    }

    server.handleClient();

    // display touched point with colored dot
    uint16_t pixel_x, pixel_y;
    unsigned long start = micros();
    bool valid = touchMapXY(pixel_x, pixel_y);
    unsigned long end = micros();
    if (valid)
    {
        tft.fillCircle(pixel_x, pixel_y, 2, brushColor);
/*         Serial.print("X: ");
        Serial.print(pixel_x);
        Serial.print("Y: ");
        Serial.println(pixel_y); */
        touchTime = max(end-start, touchTime);
        Serial.println(touchTime);
    }

    if (Serial.available())
    {
        readPrinter();
    }

    // Keep this at the end of the loop
    if (tft.updateDisplay(60)) cnt++;
}