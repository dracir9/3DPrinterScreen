
#include "lcdUI.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "lcdUI"

void renderUITask(void* arg)
{
    ESP_LOGD(TAG, "Starting render task");
    fflush(stdout);
    lcdUI* UI = (lcdUI*)arg;
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = configTICK_RATE_HZ/(UI? UI->fps : 5);

    while ( UI )
    {
        xLastWakeTime = xTaskGetTickCount();
        UI->updateDisplay();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
    vTaskDelete(NULL);
}

void handleTouchTask(void* arg)
{
    ESP_LOGD(TAG, "Starting touch task");
    fflush(stdout);
    lcdUI* UI = (lcdUI*)arg;

    while ( UI )
    {
        UI->processTouch();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    vTaskDelete(NULL);
}

bool lcdUI::begin(uint8_t upsD)
{
    if (booted) return false;

    fps = upsD;

    tft.begin();
    tft.setRotation(1);

    ts.enableRestore();

    SPIMutex = xSemaphoreCreateMutex();
    if (SPIMutex == NULL)
    {
        ESP_LOGE(TAG, "Failed to create SPI Mutex");
        return false;
    }
    xTaskCreate(renderUITask, "Render task", 4096, this, 3, &renderTask);
    if (renderTask == NULL)
    {
        ESP_LOGE(TAG, "Failed to create render task");
        return false;
    }
    delay(100); // Allow some time for the task to start
    xTaskCreate(handleTouchTask, "touch task", 4096, this, 2, &touchTask);
    if (touchTask == NULL)
    {
        ESP_LOGE(TAG, "Failed to create touch task");
        return false;
    }
    delay(100);

    booted = true;
    return true;
}

lcdUI::lcdUI()
{
    menuID = menu::info;
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
    base->render(&tft);         // Render frame
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
    ESP_LOGD(TAG, "change to idx %d!\n", newMenuID);

    delete base;

    xSemaphoreTake(SPIMutex, portMAX_DELAY);
    switch (newMenuID)
    {
        case menu::black:
            base = new Black_W(this);
            break;
        case menu::info:
            base = new Info_W();
            break;
        case menu::main:
            break;
        case menu::FileBrowser:
            base = new FileBrowser_Scr(this);
            break;
        case menu::settings:
            break;
        case menu::control:
            break;
        case menu::GcodePreview:
            base = new GcodePreview_Scr(this);
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
        ESP_LOGD(TAG, "SD Card initialized.");
        hasSD = true;
    }
    return hasSD;
}

bool lcdUI::checkSD() const
{
    return hasSD;
}

/**************************************************************************/
/************************************************************************
//    @brief  Draw the info screen
//    @param  Should initialize the screen?
*/
/**************************************************************************/
/* void lcdUI::drawInfo(bool init)
{
     static uint32_t cnt;
    const uint8_t e=3;

    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;

    setTextSize(2);
    getTextBounds(String("Current"), 0, 0, &x, &y, &w, &h);
    const grid temp(w+18, 40, width()-w-18, 91, e, 3, 8);
    temp.getTLPoint(&x, &y, 0, 3);
    const grid position(0, y, width(), temp.cellH, 4, 1, 8);
    position.getTLPoint(&x, &y, 0, 1);
    const grid info(0, y, width(), height()-y, 4, 2, 8);

   
    // First draw
    if (init)
    {
        //fillScreen(TFT_WHITE);
        // Header
        fillRect(0,0,HEIGHT,32,TFT_RED);
        setCursor(5,10);
        println("Hello world!");
        // End Header

        // Temperature header
        setTextColor(TFT_BLACK);
        for (uint8_t i = 0; i < e; i++)
        {
            temp.getTLPoint(&x, &y, i, 0);
            fillRoundRect(x, y, temp.cellW, temp.cellH, 4, TFT_WHITE);
            temp.getCenterPoint(&x, &y, i, 0);
            setCursor(x,y);
            if(i==0)
            {
                printCenter("HB");
            }
            else
            {
                printCenter("E"+String(i));
            }
        }
        // End Temperature header

        // Temperatures
        temp.getTLPoint(&x, &y, 0, 1);
        fillRoundRect(0, y, temp.gridX-temp.space, temp.cellH, 4, TFT_NAVY);
        temp.getCenterPoint(&x, &y, 0, 1);
        setCursor(5+w/2, y);
        setTextColor(TFT_WHITE);
        printCenter(String("Current"));


        temp.getTLPoint(&x, &y, 0, 2);
        fillRoundRect(0, y, temp.gridX-temp.space, temp.cellH, 4, TFT_NAVY);
        temp.getCenterPoint(&x, &y, 0, 2);
        setCursor(5+w/2, y);
        printCenter(String("Target"));
                
        temp.getTLPoint(&x, &y, 0, 0);
        drawRoundRect(0, y, width(), 92, 4, TFT_GREEN);
        // End Temperatures

        // Position
        position.getTLPoint(&x, &y, 0, 0);
        drawRoundRect(0, y, width(), position.cellH, 4, TFT_GREEN);
        info.getTLPoint(&x, &y, 0, 0);
        drawRoundRect(0, y, width(), height()-y, 4, TFT_GREEN);
        setTextSize(3);
        info.getCenterPoint(&x, &y, 0, 0);
        setCursor(x, y);
        printCenter("SD");
        setTextSize(2);
        for (uint8_t c = 0; c < 4; c++)
        {
            for (uint8_t r = 0; r < 4; r++)
            {
                info.getTLPoint(&x, &y, c, r);
                drawRect(x, y, info.cellW, info.cellH, TFT_RED);
            }
        }

        cnt=0;
    }


    // Draw update
    if (millis()%1000 == 0)
    {
        for (uint8_t i = 0; i < e; i++)
        {
            temp.getTLPoint(&x, &y, i, 1);
            fillRoundRect(x, y, temp.cellW, temp.cellH, 4, TFT_DARKCYAN);
            temp.getCenterPoint(&x, &y, i, 1);
            setCursor(x,y);
            printCenter(String(random(250))+String(char(247)));
            temp.getTLPoint(&x, &y, i, 2);
            fillRoundRect(x, y, temp.cellW, temp.cellH, 4, TFT_DARKCYAN);
            temp.getCenterPoint(&x, &y, i, 2);
            setCursor(x,y);
            printCenter(String(random(70))+String(char(247)));
        }
        
        position.getTLPoint(&x, &y, 0, 0);
        fillRoundRect(1, y+1, width()-2, position.cellH-2, 4, TFT_BLACK);
        position.getCenterPoint(&x, &y, 0, 0);
        setCursor(x,y);
        printCenter(String("X:") + String(random(250)));
        position.getCenterPoint(&x, &y, 1, 0);
        setCursor(x,y);
        printCenter(String("Y:") + String(random(210)));
        position.getCenterPoint(&x, &y, 2, 0);
        setCursor(x,y);
        printCenter(String("Z:") + String(random(200)));
        position.getCenterPoint(&x, &y, 3, 0);
        setCursor(x,y);
        printCenter(String("F_R:") + String(random(150)) + String("%"));
    }
    if (cnt != millis()/100)
    {
        cnt=millis()/100;
        setCursor(5,26);
        fillRect(5,getCursorY(),46,15,TFT_BLUE);
        print(cnt);
    } 
} */
