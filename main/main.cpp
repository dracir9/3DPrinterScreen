/**
 * @file   main.c
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 21-01-2022
 * -----
 * Last Modified: 10-04-2023
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

#include <cstdio>
#include <Arduino.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "dbg_log.h"
#include "esp_spiffs.h"
#include "lcdUI.h"


extern "C" void app_main(void)
{
    initArduino();

    esp_reset_reason_t reason = esp_reset_reason();
    if (reason == ESP_RST_PANIC ||
        reason == ESP_RST_INT_WDT ||
        reason == ESP_RST_TASK_WDT ||
        reason == ESP_RST_WDT ||
        reason == ESP_RST_BROWNOUT)
    {
        DBG_LOGE("System unexpected reboot! (%d)", reason);
        while (true) vTaskDelay(100);
    }

    DBG_LOGI("System ready!");

    DBG_LOGD("Free Heap at start: %d of %d\n", esp_get_free_internal_heap_size(), ESP.getHeapSize());

    // INITIALIZE SPIFFS STORAGE
    DBG_LOGI("Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = false
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            DBG_LOGE("Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            DBG_LOGE("Failed to find SPIFFS partition");
        } else {
            DBG_LOGE("Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    // INITIALIZE UI
    ret = lcdUI::instance()->begin();
    if (ret != ESP_OK)
    {
        DBG_LOGE("Failed to initialize UI. Rebooting NOW!");
        esp_restart();
    }

    lcdUI::instance()->setScreen(lcdUI::INFO_SCR);

    DBG_LOGD("Free Heap after init: %d of %d", esp_get_free_internal_heap_size(), ESP.getHeapSize());
    DBG_LOGD("Max continuous segment: %d", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    while (1)
    {
        vTaskDelay(1000);
    }
}
