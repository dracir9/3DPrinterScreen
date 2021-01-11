
#include "lcdUI.h"

void renderUITask(void* arg)
{
    ESP_LOGD(__FILE__, "Starting render task");
    fflush(stdout);
    lcdUI* UI = (lcdUI*)arg;
    TickType_t xLastWakeTime;
    const TickType_t xFrameTime = UI? UI->frameTime : 200;

    while (UI && UI->updateDisplay())
    {
        xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, xFrameTime);
    }
    vTaskDelete(NULL);
}

void handleTouchTask(void* arg)
{
    ESP_LOGD(__FILE__, "Starting touch task");
    fflush(stdout);
    lcdUI* UI = (lcdUI*)arg;

    while (UI && UI->processTouch())
    {
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    vTaskDelete(NULL);
}

bool lcdUI::begin(uint8_t fps)
{
    if (booted) return false;

    frameTime = max(configTICK_RATE_HZ/fps, 2);

    tft.begin();
    tft.setRotation(1);

    ts.enableRestore();

    SPIMutex = xSemaphoreCreateMutex();
    if (SPIMutex == NULL)
    {
        ESP_LOGE(__FILE__, "Failed to create SPI Mutex");
        return false;
    }
    xTaskCreate(renderUITask, "Render task", 4096, this, 3, &renderTask);
    if (renderTask == NULL)
    {
        ESP_LOGE(__FILE__, "Failed to create render task");
        return false;
    }
    delay(100); // Allow some time for the task to start
    xTaskCreate(handleTouchTask, "touch task", 4096, this, 2, &touchTask);
    if (touchTask == NULL)
    {
        ESP_LOGE(__FILE__, "Failed to create touch task");
        return false;
    }
    delay(100);

    booted = true;
    return true;
}

lcdUI::lcdUI()
{
    menuID = menu::Info;
    setScreen(menu::black);
}

lcdUI::~lcdUI()
{
    vTaskDelete(renderTask);
    vTaskDelete(touchTask);
    delete base;
}

/***************************************************************************
 * @brief      Update and render the LCD display
 * @return     True if successfully updated
 **************************************************************************/
bool lcdUI::updateDisplay()
{
    uint32_t deltaTime = esp_timer_get_time() - lastRender;
    lastRender = esp_timer_get_time();

    if (!updateObjects()) return false; // Update to the latest screen

    base->update(deltaTime);    // Update logic

    vTaskDelay(2); // Allow some time for other tasks

    xSemaphoreTake(SPIMutex, portMAX_DELAY);
    base->render(tft);         // Render frame
    PRINT_SCR(tft);
    xSemaphoreGive(SPIMutex);
    
    updateTime = esp_timer_get_time()-lastRender;

    if (esp_timer_get_time() > nextCheck)
    {
        initSD();
        nextCheck += 5000000LL;
    }
    return true;
}

bool lcdUI::processTouch()
{
    xSemaphoreTake(SPIMutex, portMAX_DELAY);
    TSPoint p = ts.getPoint(tft.height(), tft.width());
    xSemaphoreGive(SPIMutex);

    bool pressed = false;
    Screen::touchEvent event;
    if (p.z > MIN_PRESSURE && p.z < MAX_PRESSURE)
    {
        pressed = true;
        Tpos.x = p.y;
        Tpos.y = tft.height() - p.x;
    }
    
    if (!prevPressed && pressed) event = Screen::press;
    else if (prevPressed && pressed) event = Screen::hold;
    else if (prevPressed && !pressed) event = Screen::relase;
    else return true;   // Idle touch. "prevPressed" and "pressed" are already equal, is safe to return

    prevPressed = pressed;

    #ifdef DEBUG_TOUCH
    if (event == Screen::press) tft.fillCircle(Tpos.x, Tpos.y, 2, TFT_YELLOW);
    else if (event == Screen::hold) tft.fillCircle(Tpos.x, Tpos.y, 2, TFT_MAGENTA);
    else if (event == Screen::relase) tft.fillCircle(Tpos.x, Tpos.y, 2, TFT_CYAN);
    #endif

    if (!base) return false;
    base->handleTouch(event, Tpos);
    return true;
}

void lcdUI::setScreen(menu idx)
{
    newMenuID = idx;
}

bool lcdUI::updateObjects()
{
    if (menuID == newMenuID) return true;
    ESP_LOGD(__FILE__, "Change screen to ID: %d\n", newMenuID);

    delete base;

    xSemaphoreTake(SPIMutex, portMAX_DELAY);
    switch (newMenuID)
    {
        case menu::black:
            base = new Black_W(this, tft);
            break;
        case menu::Info:
            base = new Info_W(this, tft);
            break;
        case menu::main:
            break;
        case menu::FileBrowser:
            base = new FileBrowser_Scr(this, tft);
            break;
        case menu::settings:
            break;
        case menu::control:
            break;
        case menu::GcodePreview:
            base = new GcodePreview_Scr(this, tft);
            break;
        default:
            base = nullptr;
    }
    xSemaphoreGive(SPIMutex);

    if(base)
    {
        menuID = newMenuID;
        return true;
    }
    return false;
}

uint32_t lcdUI::getUpdateTime() const
{
    return updateTime;
}

bool lcdUI::initSD()
{
    if (!hasSD && SD.begin(5, SPI, 40000000U, "/sdcard"))
    {
        ESP_LOGD(__FILE__, "SD Card initialized.");
        hasSD = true;
    }
    return hasSD;
}

bool lcdUI::checkSD() const
{
    return hasSD;
}
