/**
 * @file   printer.h
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 28-04-2022
 * -----
 * Last Modified: 12-08-2023
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

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include "driver/uart.h"
#include "Vector.h"

enum TxEvent : uint8_t
{
    UART_SEND_CMD,
    UART_SEND_LINE
};

struct TxPacket
{
    TxEvent id;
    uint16_t len;
    char data[];
};

enum PState : uint8_t
{
    OFFLINE,
    READ_CNFG,
    INIT,
    READY,
    PRINTING
};

class Printer final
{
private:
    static bool _init;
    static Printer _instance;

    static constexpr size_t uartBufferSize = 1024;
    static const uart_port_t uartNum = UART_NUM_1;

    static constexpr size_t RxQueueLen = 10;
    static constexpr size_t maxLineLen = 96;
    static constexpr size_t TxBuffLen = 256;
    QueueHandle_t uartQueue;
    RingbufHandle_t uartTxBuffer;

    TaskHandle_t uartRxTask;

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
    std::vector<float> currentTemp;
    std::vector<float> targetTemp;

    Vec3f pos;
    std::vector<float> pos_E;

    Vec3f offset;
    std::vector<float> offset_E;

    Vec3f stpsPerUnit;
    std::vector<float> stpsPerUnit_E;

    Vec3f maxFeedrate;
    std::vector<float> maxFeedrate_E;

    Vec3f maxAccel;
    std::vector<float> maxAccel_E;

    std::vector<Vec3f> hotendPID;

    uint8_t feedrate = 100;

    std::string filePath;
    std::ifstream file;

    static void serialRxTask(void* arg);

    void sendLine();
    void parseSerial(char* str, const size_t len);
    esp_err_t parseTemp(char* str);
    esp_err_t parsePos(char* str);
    void allocateFields();

    esp_err_t sendCommand(const char* cmd);
    esp_err_t sendTxEvent(TxEvent event);
    esp_err_t getSettings();
    esp_err_t getFwInfo();

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

    esp_err_t setAutoReportPos(bool enable);
    esp_err_t setAutoReportTemp(bool enable);

    esp_err_t move(float x, float y, float z, bool isRelative);
    esp_err_t homeAxis(bool homeX = true, bool homeY = true, bool homeZ = true);

    esp_err_t extrude(float e, bool isRelative);
};

#endif // PRINTER_H
