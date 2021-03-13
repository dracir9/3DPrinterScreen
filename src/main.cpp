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

//###########################################################
//  GLOBAL VARIALBES
//###########################################################

lcdUI UI;
int32_t prevHeap;

//###########################################################
//  FUNCTIONS
//###########################################################
void setup(void)
{
    Serial.begin(115200);
    printf("Free Heap at start: %d of %d\n", ESP.getFreeHeap(), ESP.getHeapSize());

    if (!SPIFFS.begin(true))
    {
        ESP_LOGE(__FILE__, "Failed to initialize SPIFFS. Rebooting NOW!");
        esp_restart();
    }

    if (!UI.begin())
    {
        ESP_LOGE(__FILE__, "Failed to initialize UI. Rebooting NOW!");
        esp_restart();
    }

    UI.setScreen(UI.Info);

    prevHeap = ESP.getFreeHeap();
    ESP_LOGD(__FILE__, "Free end setup: %d", prevHeap);
}

bool flag = true;
void loop(void)
{
    if (flag && millis() % 5000 < 100)
    {
        if (abs(prevHeap - (int32_t)ESP.getFreeHeap()) > 100)
        {
            prevHeap = ESP.getFreeHeap();
            ESP_LOGD(__FILE__ ,"Free Heap: %d of %d", prevHeap, ESP.getHeapSize());
        }
        if (UI.getUpdateTime() > 2004)
            ESP_LOGD(__FILE__, "Frame update time: %d µs", UI.getUpdateTime());
        if (uxTaskGetStackHighWaterMark(UI.renderTask) < 512)
            ESP_LOGD(__FILE__, "Min stack render: %d", uxTaskGetStackHighWaterMark(UI.renderTask));
        if (uxTaskGetStackHighWaterMark(UI.touchTask) < 512)
            ESP_LOGD(__FILE__, "Min stack touch: %d", uxTaskGetStackHighWaterMark(UI.touchTask));

        flag = false;
    }
    else if (!flag && millis() % 5000 > 500)
    {
        flag = true;
    }
    vTaskDelay(100);
}