/**
 * @file   lcdUI.cpp
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 22-01-2022
 * -----
 * Last Modified: 25-07-2023
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
#include "driver/ledc.h"
#include "esp_vfs_fat.h"
#include "Menu/info_Scr.h"
#include "Menu/black_Scr.h"
#include "Menu/fileBrowser_Scr.h"
#include "Menu/preview_Scr.h"
#include "Menu/config_Scr.h"
#include "Menu/displayConf_Scr.h"
#include "Menu/print_Scr.h"
#include "Menu/draw_Scr.h"
#include "Menu/control_Scr.h"
#include "Menu/extrude_Scr.h"

bool lcdUI::init;
lcdUI lcdUI::_instance;
constexpr TchCalib lcdUI::calib;
constexpr char lcdUI::mount_point[];

lcdUI::lcdUI() :
    touchScreen(UART_NUM_2)
{
    init = true;
}

lcdUI::~lcdUI()
{
    vTaskDelete(cardTaskH);
    vTaskDelete(touchTaskH);
    vTaskDelete(updateTaskH);
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

    gpio_config_t detectConf = {
        .pin_bit_mask = BIT64(sd_cd_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config_t ledConf = {
        .pin_bit_mask = BIT64(sd_led_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    bool connected = false;

    gpio_config(&detectConf);
    gpio_config(&ledConf);
    
    while (UI)
    {
        bool prevRead = gpio_get_level(sd_cd_pin);
        if (prevRead && connected)
        {
            gpio_set_level(sd_led_pin, LOW);

            DBG_LOGD("Card disconnected");
            UI->endSD();
            connected = false;
            UI->requestUpdate();
        }
        else if (!prevRead && !connected)
        {
            gpio_set_level(sd_led_pin, HIGH);

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
    err = UI->touchScreen.begin(tch_tx_pin, tch_rx_pin);
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

    xTaskCreate(updateTask, "Render task", 4096, this, 2, &updateTaskH);
    if (updateTaskH == nullptr)
    {
        DBG_LOGE("Failed to create render task");
        return ESP_FAIL;
    }
    vTaskDelay(pdMS_TO_TICKS(100)); // Allow some time for the task to start

    xTaskCreate(cardDetectTask, "cardTask", 4096, this, 0, &cardTaskH);
    if (cardTaskH == nullptr)
    {
        DBG_LOGE("Failed to create card detect task");
        return ESP_FAIL;
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    // Configure interrupts
    gpio_set_intr_type(sd_cd_pin, GPIO_INTR_ANYEDGE);
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);
    gpio_isr_handler_add(sd_cd_pin, cardISRhandle, this);

    // Configure backlight PWM
    ledc_timer_config_t ledcTimer = {
        .speed_mode         = LEDC_LOW_SPEED_MODE,
        .duty_resolution    = LEDC_TIMER_8_BIT,
        .timer_num          = LEDC_TIMER_0,
        .freq_hz            = 10000,
        .clk_cfg            = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&ledcTimer));

    ledc_channel_config_t ledcChannel = {};
        ledcChannel.gpio_num   = lcd_bl_pin;
        ledcChannel.speed_mode = LEDC_LOW_SPEED_MODE;
        ledcChannel.channel    = LEDC_CHANNEL_0;
        ledcChannel.intr_type  = LEDC_INTR_DISABLE;
        ledcChannel.timer_sel  = LEDC_TIMER_0;
        ledcChannel.duty       = 255; // Set duty to 100%
        ledcChannel.hpoint     = 0;

    ESP_ERROR_CHECK(ledc_channel_config(&ledcChannel));

    booted = true;
    return ESP_OK;
}

void lcdUI::setScreen(const ScreenType screen)
{
    newMenuID = screen;
    xTaskNotifyGive(updateTaskH);
}

bool lcdUI::initSD()
{
    if (!SDinit)
    {
        // Init SD card
        esp_vfs_fat_sdmmc_mount_config_t mount_config = {
            .format_if_mount_failed = false,
            .max_files = 5,
            .allocation_unit_size = 16 * 1024
        };

        sdmmc_host_t host = SDMMC_HOST_DEFAULT();
        host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;

        sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

        // Set bus width to use:
        slot_config.width = 4;

        slot_config.clk = sd_clk_pin;
        slot_config.cmd = sd_cmd_pin;
        slot_config.d0 = sd_D0_pin;
        slot_config.d1 = sd_D1_pin;
        slot_config.d2 = sd_D2_pin;
        slot_config.d3 = sd_D3_pin;
        slot_config.cd = sd_cd_pin;

        DBG_LOGD("Mounting filesystem");
        esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

        if (ret != ESP_OK) {
            if (ret == ESP_FAIL) {
                DBG_LOGE("Failed to mount filesystem");
            }
            else if (ret == ESP_ERR_NOT_FOUND)
            {
                DBG_LOGE("Failed to initialize the card, card disconnected");
            } else {
                DBG_LOGE("Failed to initialize the card (%s). ", esp_err_to_name(ret));
            }
            return false;
        }
        DBG_LOGD("SD Card initialized");
        SDinit = true;
    }
    return SDinit;
}

void lcdUI::endSD()
{
    GCodeRenderer::instance()->stop();
    if (SDinit)
    {
        // Card disconnected, unmount partition and disable SDMMC peripheral
        esp_vfs_fat_sdcard_unmount(mount_point, card);
    }
    
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
    // Local copy to prevent external modifications while creating the new screen
    // (newMenuID may be changed from other threads)
    ScreenType localMenu = newMenuID;
    if (menuID == localMenu) return ESP_OK;
    DBG_LOGD("Change screen to ID: %d", localMenu);

    tft.fillScreen(TFT_BLACK);
    Button clearBtn;
    clearBtn.id = 31;
    touchScreen.setButton(&clearBtn, portMAX_DELAY);

    switch (localMenu)
    {
        case ScreenType::BLACK_SCR:
            base = std::make_unique<Black_Scr>(this, tft);
            break;
        case ScreenType::INFO_SCR:
            base = std::make_unique<Info_Scr>(this, tft, touchScreen);
            break;
        case ScreenType::MAIN_SCR:
            break;
        case ScreenType::FILE_BROWSER_SRC:
            base = std::make_unique<FileBrowser_Scr>(this, tft, touchScreen);
            break;
        case ScreenType::CONFIG_MENU_SCR:
            base = std::make_unique<Config_Scr>(this, tft, touchScreen);
            break;
        case ScreenType::CONTROL_SCR:
            base = std::make_unique<Control_Scr>(this, tft, touchScreen);
            break;
        case ScreenType::EXTRUDE_SCR:
            base = std::make_unique<Extrude_Scr>(this, tft, touchScreen);
            break;
        case ScreenType::GCODE_PREVIEW_SCR:
            base = std::make_unique<Preview_Scr>(this, tft, touchScreen);
            break;
        case ScreenType::DISPLAY_CONF:
            base = std::make_unique<DisplayConf_Scr>(this, tft, touchScreen);
            break;
        case ScreenType::PRINT_SCR:
            base = std::make_unique<Print_Scr>(this, tft, touchScreen);
            break;
        case ScreenType::DRAW_SCR:
            base = std::make_unique<Draw_Scr>(this, tft, touchScreen);
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
