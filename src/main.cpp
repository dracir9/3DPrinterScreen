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

//###########################################################
//  GLOBAL VARIALBES
//###########################################################

lcdUI UI;

//###########################################################
//  FUNCTIONS
//###########################################################
void setup(void)
{
    Serial.begin(115200);
    printf("Free Heap at start: %d of %d\n", ESP.getFreeHeap(), ESP.getHeapSize());

    if (!SPIFFS.begin(true))
    {
        ESP_LOGE(TAG, "Failed to initialize SPIFFS. Rebooting NOW!");
        esp_restart();
    }

    if (!UI.begin())
    {
        ESP_LOGE(TAG, "Failed to initialize UI. Rebooting NOW!");
        esp_restart();
    }

    UI.setScreen(UI.FileBrowser);

    ESP_LOGD(TAG, "Free end setup: %d\n", ESP.getFreeHeap());
}

bool flag = true;
void loop(void)
{
    if (flag && millis() % 5000 < 100)
    {
        ESP_LOGD(TAG ,"Free Heap: %d of %d", ESP.getFreeHeap(), ESP.getHeapSize());
        ESP_LOGD(TAG, "Frame update time: %d µs", UI.getUpdateTime());
        ESP_LOGD(TAG, "Min stack render: %d", uxTaskGetStackHighWaterMark(UI.renderTask));
        ESP_LOGD(TAG, "Min stack touch: %d", uxTaskGetStackHighWaterMark(UI.touchTask));

        flag = false;
    }
    else if (!flag && millis() % 5000 > 500)
    {
        flag = true;
    }
}