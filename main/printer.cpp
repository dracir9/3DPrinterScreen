/**
 * @file   printer.cpp
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 28-04-2022
 * -----
 * Last Modified: 07-06-2022
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

#include "printer.h"
#include "dbg_log.h"
#include <cstring>
#include <freertos/FreeRTOS.h>

Printer::Printer(const uart_port_t uartNum):
    uartNum(uartNum)
{
    // Initialize comunication
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(uartNum, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(uartNum, 21, 22, -1, -1));

    // Setup UART buffered IO with event queue
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, uartBufferSize, uartBufferSize, RxQueueLen, &uartRxQueue, 0));

    // Flags
    readyFlag = xSemaphoreCreateBinary();
    if (readyFlag == nullptr)
    {
        DBG_EARLY_LOGE("Failed to create card detect semaphore");
        esp_restart();
    }

    // Queues
    uartTxQueue = xQueueCreate(TxQueueLen, sizeof(TxEvent));
    if (uartTxQueue == nullptr)
    {
        DBG_EARLY_LOGE("Failed to create queues!");
        esp_restart();
    }

    // Create tasks
    xTaskCreate(serialRxTask, "UART receiver task", 4096, this, 1, &uartRxTask);
    if (uartRxTask == nullptr)
    {
        DBG_EARLY_LOGE("Failed to create serial receiver task");
        esp_restart();
    }

    xTaskCreate(serialTxTask, "UART transmitter task", 4096, this, 1, &uartTxTask);
    if (uartTxTask == nullptr)
    {
        DBG_EARLY_LOGE("Failed to create serial transmitter task");
        esp_restart();
    }
}

Printer::~Printer()
{
}

void Printer::serialRxTask(void* arg)
{
    DBG_LOGI("Starting UART receiver task");
    Printer* CNC = static_cast<Printer*>(arg);
    uart_event_t event;
    size_t linePtr = 0;
    char* dtmp = (char*) malloc(CNC->maxLineLen);
    char* line = (char*) malloc(CNC->maxLineLen);
    if (dtmp == nullptr)
    {
        DBG_LOGE("Unable to allocate buffers");
        esp_restart();
    }

    while (CNC)
    {
        if(xQueueReceive(CNC->uartRxQueue, (void *)&event, portMAX_DELAY) != pdTRUE)
            continue;

        switch(event.type)
        {
        // Data received
        case UART_DATA:
        {
            size_t len = 0;
            while ((len = uart_read_bytes(CNC->uartNum, dtmp, CNC->maxLineLen, 0)) > 0)
            {
                for (size_t i = 0; i < len; i++)
                {
                    if (dtmp[i] < 32)
                    {
                        if (linePtr == 0) continue;
                        line[linePtr] = '\0';
                        CNC->parseSerial(line, linePtr);
                        linePtr = 0;
                    }
                    else
                        line[linePtr++] = dtmp[i];
                        
                    event.size--;
                }
            }
            break;
        }
        //Event of HW FIFO overflow detected
        case UART_FIFO_OVF:
            DBG_LOGW("HW fifo overflow");
            uart_flush_input(CNC->uartNum);
            xQueueReset(CNC->uartRxQueue);
            break;
        //Event of UART ring buffer full
        case UART_BUFFER_FULL:
            DBG_LOGW("Ring buffer full");
            uart_flush_input(CNC->uartNum);
            xQueueReset(CNC->uartRxQueue);
            break;
        //Event of UART RX break detected
        case UART_BREAK:
            DBG_LOGI("uart rx break");
            break;
        //Event of UART parity check error
        case UART_PARITY_ERR:
            DBG_LOGI("uart parity error");
            break;
        //Event of UART frame error
        case UART_FRAME_ERR:
            DBG_LOGI("uart frame error");
            break;
        //UART_PATTERN_DET
        case UART_PATTERN_DET: // Not used, should never be called
            DBG_LOGI("Pattern detected");
            break;
        //Others
        default:
            DBG_LOGI("uart event type: %d", event.type);
            break;
        }
    }

    vTaskDelete(nullptr);
}

void Printer::serialTxTask(void* arg)
{
    DBG_LOGI("Starting UART transmitter task");
    Printer* CNC = static_cast<Printer*>(arg);
    TxEvent event;

    vTaskDelay(pdMS_TO_TICKS(500));
    while (!CNC->initialized)
    {
        uart_write_bytes(CNC->uartNum, "M115\n", 5);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    while (CNC)
    {
        xSemaphoreTake(CNC->readyFlag, portMAX_DELAY);

        if (xQueueReceive(CNC->uartTxQueue, (void *)&event, portMAX_DELAY) != pdTRUE)
            continue;

        /**
         * M503 Report settings
         * M115 Firmware info
         * M155 Autoreport temperatures
         */
        switch (event)
        {
        case GCODE_LINE:
            break;

        case GET_TEMP:
            break;

        case GET_POS:
            break;

        case SET_POS:
            break;

        case SET_TEMP:
            break;

        case AUTOTEMP_EN:
            uart_write_bytes(CNC->uartNum, "M155 S1\n", 8);
            break;

        case AUTOTEMP_DIS:
            break;

        case AUTOPOS_EN:
            uart_write_bytes(CNC->uartNum, "M154 S1\n", 8);
            break;

        case AUTOPOS_DIS:
            break;

        case FW_INFO:
            uart_write_bytes(CNC->uartNum, "M115\n", 5);
            break;

        case GET_SETTINGS:
            uart_write_bytes(CNC->uartNum, "M503\n", 5);
            break;

        default:
            break;
        }
    }

    vTaskDelete(nullptr);
}

void Printer::parseSerial(char* str, const size_t len)
{
    TxEvent event;
    if (strcmp(str, "ok") == 0)
    {
        xSemaphoreGive(readyFlag);
    }
    else if (strncmp(str, "echo:;", 6) == 0)
    {
        return;
    }
    else if (strncmp(str, "echo: ", 6) == 0)
    {
        if (strncmp(&str[6], " G2", 3) == 0 && str[10] == ' ')
        {
            if (str[9] == '1')
            {
                isMetric = true;
                DBG_LOGD("Normal units");
            }
            else if (str[9] == '0')
            {
                isMetric = false;
                DBG_LOGD("R******d units");
            }
        }
        else if (strncmp(&str[6], " M92 ", 5) == 0)
        {
            char* endChr = &str[5];
            while (endChr != 0)
            {
                if (*endChr == 'X')
                    stpsPerUnit.x = strtof(endChr, &endChr);
                else if (*endChr == 'Y')
                    stpsPerUnit.y = strtof(endChr, &endChr);
                else if (*endChr == 'Z')
                    stpsPerUnit.z = strtof(endChr, &endChr);
                else if (*endChr == 'E')
                    stpsPerUnit.x = strtof(endChr, &endChr);
                else
                    endChr++;
            }
        }
        else if (strncmp(&str[6], " M200 S", 7) == 0)
        {
            if (str[13] == '1')
                volumetricEn = true;
            else if (str[13] == '0')
                volumetricEn = false;
        }
        else if (strncmp(&str[6], " M149 ", 6) == 0 && str[13] == ' ')
        {
            tempUnit = str[12];
            DBG_LOGD("Teperature unit: %c", tempUnit);
        }
        else
            printf(">|%s\n", str);
    }
    else if (strncmp(str, "Cap:", 4) == 0)
    {
        if (strncmp(&str[4], "AUTOREPORT_POS:1", 16) == 0)
        {
            event = AUTOPOS_EN;
            //xQueueSend(uartTxQueue, &event, portMAX_DELAY);
        }
        else if (strncmp(&str[4], "AUTOREPORT_TEMP:1", 17) == 0)
        {
            event = AUTOTEMP_EN;
            //xQueueSend(uartTxQueue, &event, portMAX_DELAY);
        }
        else
            printf(">|%s\n", str);
    }
    else if (strcmp(str, "start") == 0 || strncmp(str, "FIRMWARE_NAME:Marlin", 20) == 0)
    {
        DBG_LOGI("Printer initialized");
        initialized = true;

        if (str[0] == 'F')
        {
            event = GET_SETTINGS;
        }
        else
        {
            event = FW_INFO;
            xSemaphoreGive(readyFlag);
        }
        xQueueSend(uartTxQueue, &event, portMAX_DELAY);
    }
    else if (strncmp(str, "Marlin ", 7) == 0)
    {
        fwVersion = str;
        DBG_LOGD("FW version captured");
    }
    else
        printf(">|%s\n", str);
}
