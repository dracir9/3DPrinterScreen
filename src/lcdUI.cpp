
#include "lcdUI.h"

void IRAM_ATTR lcdUI::touchISRhandle(void* arg)
{
    BaseType_t xHigherPriorityTaskWoken = false;

    lcdUI* UI = static_cast<lcdUI*>(arg);
    if (UI)
        xSemaphoreGiveFromISR(UI->touchFlag, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR();
}

void IRAM_ATTR lcdUI::cardISRhandle(void* arg)
{
    BaseType_t xHigherPriorityTaskWoken = false;

    lcdUI* UI = static_cast<lcdUI*>(arg);
    if (UI)
        xSemaphoreGiveFromISR(UI->cardFlag, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR();
}

void lcdUI::renderUITask(void* arg)
{
    ESP_LOGD(__FILE__, "Starting render task");
    lcdUI* UI = static_cast<lcdUI*>(arg);
    TickType_t xLastWakeTime = xTaskGetTickCount();;
    const TickType_t xFrameTime = UI? UI->frameTime : 200;

    while (UI && UI->updateDisplay())
    {
        vTaskDelayUntil(&xLastWakeTime, xFrameTime);
        xLastWakeTime = xTaskGetTickCount();
    }
    vTaskDelete(nullptr);
}

void lcdUI::handleTouchTask(void* arg)
{
    ESP_LOGD(__FILE__, "Starting touch task");
    lcdUI* UI = static_cast<lcdUI*>(arg);

    while (UI && UI->processTouch())
    {
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    vTaskDelete(nullptr);
}

void lcdUI::cardDetectTask(void* arg)
{
    ESP_LOGD(__FILE__, "Starting card detect task");
    lcdUI* UI = static_cast<lcdUI*>(arg);
    bool connected = false;
    while (UI)
    {
        bool prevRead = digitalRead(CD_PIN);
        if (prevRead && connected)
        {
            ESP_LOGD(__FILE__, "Card disconnected");
            UI->endSD();
            connected = false;
        }
        else if (!prevRead && !connected)
        {
            ESP_LOGD(__FILE__, "Card connected");
            connected = UI->initSD();
        }
        xSemaphoreTake(UI->cardFlag, portMAX_DELAY);
    }
    
    vTaskDelete(nullptr);
}

bool lcdUI::begin(const uint8_t fps)
{
    if (booted) return false;

    frameTime = max(configTICK_RATE_HZ/fps, 2);

    tft.begin();
    tft.setRotation(1);
    uint16_t calibrationData[5] = CALIBRATION;
    tft.setTouch(calibrationData);

    pinMode(CD_PIN, INPUT_PULLUP);
    pinMode(T_IRQ_PIN, INPUT_PULLUP);

    // Crete semaphores
    SPIMutex = xSemaphoreCreateMutex();
    if (SPIMutex == nullptr)
    {
        ESP_LOGE(__FILE__, "Failed to create SPI Mutex");
        return false;
    }
    touchFlag = xSemaphoreCreateBinary();
    if (touchFlag == nullptr)
    {
        ESP_LOGE(__FILE__, "Failed to create touch semaphore");
        return false;
    }
    cardFlag = xSemaphoreCreateBinary();
    if (cardFlag == nullptr)
    {
        ESP_LOGE(__FILE__, "Failed to create card detect semaphore");
        return false;
    }

    // Create tasks
    xTaskCreate(renderUITask, "Render task", 4096, this, 3, &renderTask);
    if (renderTask == nullptr)
    {
        ESP_LOGE(__FILE__, "Failed to create render task");
        return false;
    }
    delay(100); // Allow some time for the task to start
    xTaskCreate(handleTouchTask, "touch task", 4096, this, 2, &touchTask);
    if (touchTask == nullptr)
    {
        ESP_LOGE(__FILE__, "Failed to create touch task");
        return false;
    }
    delay(100);
    xTaskCreate(cardDetectTask, "touch task", 2048, this, 1, &cardTask);
    if (cardTask == nullptr)
    {
        ESP_LOGE(__FILE__, "Failed to create card detect task");
        return false;
    }
    delay(100);

    // Configure interrupts
    gpio_set_intr_type((gpio_num_t)CD_PIN, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type((gpio_num_t)T_IRQ_PIN, GPIO_INTR_NEGEDGE);

    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);

    gpio_isr_handler_add((gpio_num_t) CD_PIN, cardISRhandle, this);
    gpio_isr_handler_add((gpio_num_t) T_IRQ_PIN, touchISRhandle, this);

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

    return true;
}

bool lcdUI::processTouch()
{
    gpio_set_intr_type((gpio_num_t)T_IRQ_PIN, GPIO_INTR_DISABLE);
    Vector2<uint16_t> p;
    bool pressed = false;
    Screen::touchEvent event;

    xSemaphoreTake(SPIMutex, portMAX_DELAY);
    if (tft.getTouch(&p.x, &p.y, MIN_PRESSURE))
    {
        pressed = true;
        Tpos = p;
    }
    xSemaphoreGive(SPIMutex);

    gpio_set_intr_type((gpio_num_t)T_IRQ_PIN, GPIO_INTR_NEGEDGE);
    
    if (!prevPressed && pressed) event = Screen::press;
    else if (prevPressed && pressed) event = Screen::hold;
    else if (prevPressed && !pressed) event = Screen::release;
    else
    {
        xSemaphoreTake(touchFlag, portMAX_DELAY);
        return true;   // No touch. "prevPressed" and "pressed" are already equal, is safe to return
    }

    prevPressed = pressed;

    #ifdef DEBUG_TOUCH
    xSemaphoreTake(SPIMutex, portMAX_DELAY);
    if (event == Screen::press) tft.fillCircle(Tpos.x, Tpos.y, 2, TFT_YELLOW);
    else if (event == Screen::hold) tft.fillCircle(Tpos.x, Tpos.y, 2, TFT_MAGENTA);
    else if (event == Screen::release) tft.fillCircle(Tpos.x, Tpos.y, 2, TFT_CYAN);
    xSemaphoreGive(SPIMutex);
    #endif

    if (!base) return false;
    base->handleTouch(event, Tpos);
    if (event == Screen::release)
        xSemaphoreTake(touchFlag, portMAX_DELAY);
    return true;
}

void lcdUI::setScreen(const menu idx)
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
    if (!hasSD && SD_MMC.begin("/sdcard"))
    {
        ESP_LOGD(__FILE__, "SD Card initialized.");
        hasSD = true;
    }
    return hasSD;
}

void lcdUI::endSD()
{
    if (hasSD)
        SD_MMC.end();
    
    hasSD = false;
}

bool lcdUI::checkSD() const
{
    return hasSD;
}

bool lcdUI::setFile(const std::string& file)
{
    if (access(file.c_str(), F_OK) == 0)
        selectedFile = file;
    else
        return false;
    return true;
}
