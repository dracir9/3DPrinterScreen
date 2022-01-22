/**
 * @file   main.c
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 21-01-2022
 * -----
 * Last Modified: 22-01-2022
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

#include <stdio.h>
#include <Arduino.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "lcdUI.h"

gpio_config_t input_conf = {
    .pin_bit_mask = GPIO_SEL_34 | GPIO_SEL_27,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};

lcdUI UI;

extern "C" void app_main(void)
{
    gpio_config(&input_conf);
    initArduino();

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("System ready!\n");

    printf("Free Heap at start: %d of %d\n", esp_get_free_heap_size(), ESP.getHeapSize());

    // INITIALIZE SPIFFS STORAGE
    ESP_LOGI(__FILE__, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = false
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(__FILE__, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(__FILE__, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(__FILE__, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    // INITIALIZE UI
    if (!UI.begin())
    {
        ESP_LOGE(__FILE__, "Failed to initialize UI. Rebooting NOW!");
        esp_restart();
    }

    UI.setScreen(UI.Info);

    while (1)
    {

    }
}
