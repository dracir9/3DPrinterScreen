/**
 * @file   printer.h
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 28-04-2022
 * -----
 * Last Modified: 30-04-2022
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

#ifndef PRINTER_H
#define PRINTER_H

#include "stdint.h"
#include "driver/uart.h"

class Printer
{
private:
    static constexpr int uartBufferSize = 1024 * 2;
    const uint8_t toolheads;
    const uart_port_t uartNum;

    QueueHandle_t uartQueue;
    TaskHandle_t uartTask;
    
    float* tempE;

    static void serialComTask(void* arg);
public:
    Printer(const uint8_t tools, const uart_port_t uartNum);
    ~Printer();
};

#endif // PRINTER_H
