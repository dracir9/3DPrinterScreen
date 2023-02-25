/**
 * @file   printer.h
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 28-04-2022
 * -----
 * Last Modified: 25-02-2023
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
#include "Vector.h"
#include <string>

enum TxEvent : uint8_t
{
    GCODE_LINE,
    GET_TEMP,
    GET_POS,
    SET_POS,
    SET_TEMP,
    SET_FEEDRATE,
    AUTOTEMP_EN,
    AUTOTEMP_DIS,
    AUTOPOS_EN,
    AUTOPOS_DIS,
    FW_INFO,
    GET_SETTINGS,
};

enum PState : uint8_t
{
    OFFLINE,
    READ_CNFG,
    INIT,
    READY,
    PRINTING
};

class Printer
{
private:
    static bool _init;
    static Printer _instance;

    static constexpr size_t uartBufferSize = 1024;
    static const uart_port_t uartNum = UART_NUM_1;

    static constexpr size_t RxQueueLen = 10;
    static constexpr size_t TxQueueLen = 10;
    static constexpr size_t maxLineLen = 96;
    QueueHandle_t uartRxQueue;
    QueueHandle_t uartTxQueue;

    TaskHandle_t uartRxTask;
    TaskHandle_t uartTxTask;

    PState state = OFFLINE;
    bool isMetric = true;
    bool volumetricEn = false;
    char tempUnit = 'C';
    SemaphoreHandle_t readyFlag;

    std::string fwVersion;

    uint8_t toolheads = 1;
    uint8_t activeTool = 0;
    
    float bedTemp;
    float tarBedTemp;
    float* currentTemp = nullptr;
    float* targetTemp = nullptr;

    Vec3f pos;
    float* pos_E = nullptr;

    Vec3f offset;
    float* offset_E = nullptr;

    Vec3f stpsPerUnit;
    float* stpsPerUnit_E = nullptr;

    Vec3f maxFeedrate;
    float* maxFeedrate_E = nullptr;

    Vec3f maxAccel;
    float* maxAccel_E = nullptr;

    Vec3f* hotendPID;

    uint8_t feedrate = 100;

    std::string filePath;
    FILE* file = nullptr;

    static void serialRxTask(void* arg);
    static void serialTxTask(void* arg);

    void sendLine();
    void parseSerial(char* str, const size_t len);
    esp_err_t parseTemp(char* str);
    esp_err_t parsePos(char* str);
    void allocateFields();
    void cleanFields();

    Printer();

public:
    ~Printer();
    static Printer* instance();

    esp_err_t sendFile(std::string path);

    uint8_t getToolNum() const;
    PState getState() const;
    float getBedTemp() const;
    float getTarBedTemp() const;
    float getToolTemp(uint8_t tool) const;
    float getTarToolTemp(uint8_t tool) const;
    esp_err_t getPosition(Vec3f* vec) const;
    esp_err_t getExtruderPos(float* pos, uint8_t tool) const;
    float getFeedrate() const;
};

#endif // PRINTER_H
