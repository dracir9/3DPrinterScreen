/**
 * @file   printer.cpp
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 28-04-2022
 * -----
 * Last Modified: 08-05-2022
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

Printer::Printer(uint8_t tools, const uart_port_t uartNum):
    toolheads(tools), uartNum(uartNum)
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

    // Create arrays
    actualTemp = (float*)calloc(sizeof(float), tools);

    //
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
    char* dtmp = (char*) malloc(CNC->uartBufferSize);
    if (dtmp == nullptr)
    {
        DBG_LOGE("Unable to allocate buffers");
        esp_restart();
    }

    while (CNC)
    {
        if(xQueueReceive(CNC->uartRxQueue, (void *)&event, portMAX_DELAY))
        {
            switch(event.type)
            {
                // Data received
                case UART_DATA:
                    uart_read_bytes(CNC->uartNum, dtmp, event.size, portMAX_DELAY);
                    dtmp[event.size] = '\0';
                    CNC->parseSerial(dtmp, event.size);
                    break;
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
        else
        {
            DBG_LOGI("Send");
            uart_write_bytes(CNC->uartNum, "P", 2);
        }
    }

    vTaskDelete(nullptr);
}

void Printer::serialTxTask(void* arg)
{
    DBG_LOGI("Starting UART transmitter task");
    Printer* CNC = static_cast<Printer*>(arg);

    while (CNC)
    {
        xSemaphoreTake(CNC->readyFlag, portMAX_DELAY);
    }

    vTaskDelete(nullptr);
}

void Printer::parseSerial(const char* str, const size_t len)
{
    printf(">>%s\n", str);
    if (len == 2)
    {
        if (str[0] == 'O' && str[1] == 'K')
        {
            xSemaphoreGive(readyFlag);
        }
    }
}
