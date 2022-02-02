/**
 * @file   lcdUI.cpp
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 22-01-2022
 * -----
 * Last Modified: 02-02-2022
 * Modified By: Ricard Bitriá Ribes
 * -----
 * @copyright (c) 2022 Ricard Bitriá Ribes
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "lcdUI.h"
#include "dbg_log.h"
#include <SD_MMC.h>
#include "Menu/info_w.h"
#include "Menu/black_w.h"
#include "Menu/fileBrowser_Scr.h"
#include "Menu/gcodePreview_Scr.h"

bool lcdUI::init;
lcdUI lcdUI::_instance;
constexpr TchCalib lcdUI::calib;

lcdUI::lcdUI() :
    touchScreen(I2C_NUM_0)
{
    init = true;
}

lcdUI::~lcdUI()
{
    vTaskDelete(cardTaskH);
    vTaskDelete(touchTaskH);
    vTaskDelete(updateTaskH);
    vSemaphoreDelete(cardFlag);
    vSemaphoreDelete(updateFlag);
    delete base;
}

lcdUI* lcdUI::instance()
{
    return init ? &_instance : nullptr;
}

void IRAM_ATTR lcdUI::cardISRhandle(void* arg)
{
    BaseType_t xHigherPriorityTaskWoken = false;

    lcdUI* UI = static_cast<lcdUI*>(arg);
    if (UI)
        xSemaphoreGiveFromISR(UI->cardFlag, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR();
}

void lcdUI::cardDetectTask(void* arg)
{
    DBG_LOGI("Starting card detect task");

    lcdUI* UI = static_cast<lcdUI*>(arg);
    gpio_config_t cardEmpty = {
        .pin_bit_mask = GPIO_SEL_27,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config_t cardInserted = {
        .pin_bit_mask = GPIO_SEL_27,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    bool connected = false;
    
    while (UI)
    {
        bool prevRead = gpio_get_level(GPIO_NUM_34);
        if (prevRead && connected)
        {
            gpio_config(&cardEmpty);

            DBG_LOGD("Card disconnected");
            UI->endSD();
            connected = false;
        }
        else if (!prevRead && !connected)
        {
            gpio_config(&cardInserted);
            gpio_set_level(GPIO_NUM_27, HIGH);

            DBG_LOGD("Card connected");
            connected = UI->initSD();
        }
        xSemaphoreTake(UI->cardFlag, portMAX_DELAY);
    }
    
    vTaskDelete(nullptr);
}

void lcdUI::touchTask(void* arg)
{
    DBG_LOGI("Starting touch task");
    lcdUI* UI = static_cast<lcdUI*>(arg);

    esp_err_t err;
    err = UI->touchScreen.begin(I2C_MODE_MASTER, GPIO_NUM_32, GPIO_NUM_33, 0x22);
    if (err != ESP_OK)
    {
        DBG_LOGE("Error initializing touch driver");
    }

    err = UI->touchScreen.setCalibration(&UI->calib);
    if (err != ESP_OK)
    {
        DBG_LOGE("Error initializing touch driver");
    }

    err = UI->touchScreen.setThresholds(400, 10000);
    if (err != ESP_OK)
    {
        DBG_LOGE("Error initializing touch driver");
    }

    err = UI->touchScreen.setNotifications(true, false, true);
    if (err != ESP_OK)
    {
        DBG_LOGE("Error initializing touch driver");
    }

    while (UI) UI->processTouch();

    vTaskDelete(nullptr);
}

void lcdUI::updateTask(void* arg)
{
    DBG_LOGI("Starting render task");
    lcdUI* UI = static_cast<lcdUI*>(arg);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrameTime = pdMS_TO_TICKS(100);

    while (UI)
    {
        esp_err_t ret = UI->updateDisplay();
        if (ret != ESP_OK)
        {
            DBG_LOGE("Update display failed");
            break;
        }

        xSemaphoreTake(UI->updateFlag, portMAX_DELAY);
        vTaskDelayUntil(&xLastWakeTime, xFrameTime);
        xLastWakeTime = xTaskGetTickCount();
    }
    vTaskDelete(nullptr);
}

esp_err_t lcdUI::begin()
{
    if (booted) return false;

    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(TFT_RED);

    // Crete semaphores
    cardFlag = xSemaphoreCreateBinary();
    if (cardFlag == nullptr)
    {
        DBG_LOGE("Failed to create card detect semaphore");
        return false;
    }

    updateFlag = xSemaphoreCreateBinary();
    if (updateFlag == nullptr)
    {
        DBG_LOGE("Failed to create update flag semaphore");
        return false;
    }

    // Create tasks
    xTaskCreate(touchTask, "touch task", 4096, this, 1, &touchTaskH);
    if (touchTaskH == nullptr)
    {
        DBG_LOGE("Failed to create touch task");
        return false;
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    xTaskCreate(cardDetectTask, "cardTask", 4096, this, 0, &cardTaskH);
    if (cardTaskH == nullptr)
    {
        DBG_LOGE("Failed to create card detect task");
        return false;
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    xTaskCreate(updateTask, "Render task", 4096, this, 2, &updateTaskH);
    if (updateTaskH == nullptr)
    {
        DBG_LOGE("Failed to create render task");
        return false;
    }
    vTaskDelay(pdMS_TO_TICKS(100)); // Allow some time for the task to start

    // Configure interrupts
    gpio_set_intr_type(GPIO_NUM_34, GPIO_INTR_ANYEDGE);
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);
    gpio_isr_handler_add(GPIO_NUM_34, cardISRhandle, this);

    booted = true;
    return ESP_OK;
}

void lcdUI::setScreen(const menu screen)
{
    newMenuID = screen;
    xSemaphoreGive(updateFlag);
}

bool lcdUI::initSD()
{
    if (!SDinit && SD_MMC.begin("/sdcard"))
    {
        DBG_LOGD("SD Card initialized");
        SDinit = true;
    }
    return SDinit;
}

void lcdUI::endSD()
{
    if (SDinit)
        SD_MMC.end();
    
    SDinit = false;
}

bool lcdUI::isSDinit() const
{
    return SDinit;
}

esp_err_t lcdUI::setFile(const std::string& file)
{
    if (access(file.c_str(), F_OK) == 0)
        selectedFile = file;
    else
        return ESP_ERR_NOT_FOUND;
    return ESP_OK;
}

void lcdUI::processTouch()
{
    TchEvent event;
    touchScreen.getEvent(&event, portMAX_DELAY);

    if (event.event == TrgSrc::IDLE) // Touch event
    {
        #ifdef DEBUG_TOUCH
        if (event == Screen::press) tft.fillCircle(Tpos.x, Tpos.y, 2, TFT_YELLOW);
        else if (event == Screen::hold) tft.fillCircle(Tpos.x, Tpos.y, 2, TFT_MAGENTA);
        else if (event == Screen::release) tft.fillCircle(Tpos.x, Tpos.y, 2, TFT_CYAN);
        #endif

        Screen::touchEvent legacyEvent = Screen::press;
        if (!base) return;
        base->handleTouch(legacyEvent, event.point);
    }
    else // Button event
    {
        
    }
}

esp_err_t lcdUI::updateDisplay()
{
    int64_t deltaTime = esp_timer_get_time() - lastRender;
    lastRender = esp_timer_get_time();

    esp_err_t ret = updateObjects(); // Update to the latest screen
    if (ret != ESP_OK) return ret;

    base->update(deltaTime);    // Update logic

    vTaskDelay(2); // Allow some time for other tasks

    base->render(tft);         // Render frame
    
    updateTime = esp_timer_get_time() - lastRender;

    return ESP_OK;
}

esp_err_t lcdUI::updateObjects()
{
    menu localMenu = newMenuID; // Local copy so that it remains unchanged
    if (menuID == localMenu) return ESP_OK;
    DBG_LOGD("Change screen to ID: %d\n", localMenu);

    delete base;

    switch (localMenu)
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

    if (base == nullptr)
        return ESP_ERR_NO_MEM;

    menuID = localMenu;
    return ESP_OK;
}

void lcdUI::requestUpdate()
{
    xSemaphoreGive(updateFlag);
}
