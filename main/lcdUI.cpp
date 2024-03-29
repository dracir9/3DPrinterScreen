/**
 * @file   lcdUI.cpp
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 22-01-2022
 * -----
 * Last Modified: 15-04-2022
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
#include "driver/ledc.h"
#include "Menu/info_Scr.h"
#include "Menu/black_Scr.h"
#include "Menu/fileBrowser_Scr.h"
#include "Menu/preview_Scr.h"
#include "Menu/config_Scr.h"
#include "Menu/displayConf_Scr.h"

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
        vTaskNotifyGiveFromISR(UI->cardTaskH, &xHigherPriorityTaskWoken)

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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
            UI->requestUpdate();
        }
        else if (!prevRead && !connected)
        {
            gpio_config(&cardInserted);
            gpio_set_level(GPIO_NUM_27, HIGH);

            DBG_LOGD("Card connected");
            connected = UI->initSD();
            UI->requestUpdate();
        }
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
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

    err = UI->touchScreen.setNotifications(false, true, true);
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
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        vTaskDelayUntil(&xLastWakeTime, xFrameTime);
        xLastWakeTime = xTaskGetTickCount();
    }
    vTaskDelete(nullptr);
}

esp_err_t lcdUI::begin()
{
    if (booted) return ESP_OK;

    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_RED);

    // Crete semaphores
    touchMutex = xSemaphoreCreateMutex();
    if (touchMutex == nullptr)
    {
        DBG_LOGE("Failed to create touch mutex");
        return ESP_FAIL;
    }

    // Create tasks
    xTaskCreate(touchTask, "touch task", 4096, this, 1, &touchTaskH);
    if (touchTaskH == nullptr)
    {
        DBG_LOGE("Failed to create touch task");
        return ESP_FAIL;
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    xTaskCreate(cardDetectTask, "cardTask", 4096, this, 0, &cardTaskH);
    if (cardTaskH == nullptr)
    {
        DBG_LOGE("Failed to create card detect task");
        return ESP_FAIL;
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    xTaskCreate(updateTask, "Render task", 4096, this, 2, &updateTaskH);
    if (updateTaskH == nullptr)
    {
        DBG_LOGE("Failed to create render task");
        return ESP_FAIL;
    }
    vTaskDelay(pdMS_TO_TICKS(100)); // Allow some time for the task to start

    // Configure interrupts
    gpio_set_intr_type(GPIO_NUM_34, GPIO_INTR_ANYEDGE);
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);
    gpio_isr_handler_add(GPIO_NUM_34, cardISRhandle, this);

    // Configure backlight PWM
    ledc_timer_config_t ledcTimer = {
        .speed_mode         = LEDC_LOW_SPEED_MODE,
        .duty_resolution    = LEDC_TIMER_8_BIT,
        .timer_num          = LEDC_TIMER_0,
        .freq_hz            = 10000,
        .clk_cfg            = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&ledcTimer));

    ledc_channel_config_t ledcChannel = {
        .gpio_num   = 26,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 255, // Set duty to 100%
        .hpoint     = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&ledcChannel));

    booted = true;
    return ESP_OK;
}

void lcdUI::setScreen(const menu screen)
{
    newMenuID = screen;
    xTaskNotifyGive(updateTaskH);
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
    GCodeRenderer::instance()->stop();
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
    {
        DBG_LOGD("Set file %s", file.c_str());
        selectedFile = file;
    }
    else
        return ESP_ERR_NOT_FOUND;
    return ESP_OK;
}

void lcdUI::processTouch()
{
    TchEvent event;
    if (touchScreen.getEvent(&event, portMAX_DELAY) != ESP_OK) return;

    #ifdef CONFIG_TOUCH_DBG
    if (event.trigger == TrgSrc::PRESS) tft.fillCircle(event.pos.x, event.pos.y, 1, TFT_YELLOW);
    else if (event.trigger == TrgSrc::HOLD_STRT) tft.fillCircle(event.pos.x, event.pos.y, 1, TFT_MAGENTA);
    else if (event.trigger == TrgSrc::HOLD_TICK) tft.fillCircle(event.pos.x, event.pos.y, 1, TFT_RED);
    else if (event.trigger == TrgSrc::HOLD_END) tft.fillCircle(event.pos.x, event.pos.y, 1, TFT_ORANGE);
    else if (event.trigger == TrgSrc::RELEASE) tft.fillCircle(event.pos.x, event.pos.y, 1, TFT_CYAN);
    #endif

    if (!base) return;
    xSemaphoreTake(touchMutex, portMAX_DELAY);
    base->handleTouch(event);
    xSemaphoreGive(touchMutex);
}

esp_err_t lcdUI::updateDisplay()
{
    int64_t deltaTime = esp_timer_get_time() - lastRender;
    lastRender = esp_timer_get_time();

    xSemaphoreTake(touchMutex, portMAX_DELAY);
    esp_err_t ret = updateObjects(); // Update to the latest screen
    xSemaphoreGive(touchMutex);
    if (ret != ESP_OK) return ret;

    base->update(deltaTime, touchScreen);    // Update logic

    vTaskDelay(2); // Allow some time for other tasks

    base->render(tft);         // Render frame
    
    updateTime = esp_timer_get_time() - lastRender;

    return ESP_OK;
}

esp_err_t lcdUI::updateObjects()
{
    menu localMenu = newMenuID; // Local copy so that it remains unchanged
    if (menuID == localMenu) return ESP_OK;
    DBG_LOGD("Change screen to ID: %d", localMenu);

    tft.fillScreen(TFT_BLACK);
    Button clearBtn;
    clearBtn.id = 31;
    touchScreen.setButton(&clearBtn, portMAX_DELAY);

    delete base;
    base = nullptr;

    switch (localMenu)
    {
        case menu::black:
            base = new Black_Scr(this, tft);
            break;
        case menu::Info:
            base = new Info_Scr(this, tft, touchScreen);
            break;
        case menu::main:
            break;
        case menu::FileBrowser:
            base = new FileBrowser_Scr(this, tft, touchScreen);
            break;
        case menu::Config:
            base = new Config_Scr(this, tft, touchScreen);
            break;
        case menu::control:
            break;
        case menu::GcodePreview:
            base = new Preview_Scr(this, tft, touchScreen);
            break;
        case menu::DisplayConf:
            base = new DisplayConf_Scr(this, tft, touchScreen);
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
    xTaskNotifyGive(updateTaskH);
}

esp_err_t lcdUI::setBrightness(uint32_t duty)
{
    if (duty > 255)
        duty = 255;
    
    esp_err_t ret = ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    if (ret != ESP_OK)
        return ret;

    // Update duty
    ret = ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    if (ret != ESP_OK)
        return ret;

    return ESP_OK;
}

uint32_t lcdUI::getBrightness()
{
    return ledc_get_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}
