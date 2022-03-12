/**
 * @file   GCodeRenderer.cpp
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 07-12-2021
 * -----
 * Last Modified: 12-03-2022
 * Modified By: Ricard Bitriá Ribes
 * -----
 * @copyright (c) 2021 Ricard Bitriá Ribes
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

#include "GCodeRenderer.h"
#include <cmath>
#include <cstring>
//#include "esp_system.h"

template<class T, class U>
void swap_coord(T &a, U &b)
{
    U tmp = b;
    b = a;
    a = tmp;
}

GCodeRenderer::GCache::GCache()
{
    buffer = (char*)malloc(cacheLen);
    if (buffer == nullptr)
        DBG_LOGE("Failed to allocate cache file in PSRAM");

    reset();
}

GCodeRenderer::GCache::~GCache()
{
    if (buffer)
        free(buffer);
}

void GCodeRenderer::GCache::nextChunk()
{
    chunk.setChunk(&buffer[size]);
    size += GChunk::headBytes() + GBlock::headBytes();
    chunk.reset();
}

void GCodeRenderer::GCache::reset()
{
    size = GChunk::headBytes() + GBlock::headBytes();
    readPtr = GChunk::headBytes() + GBlock::headBytes();
    lastReadPtr = 0;
    nextStop = 0;
    chunk.setChunk(buffer);
    chunk.reset();
}

void GCodeRenderer::GCache::rewind()
{
    readPtr = GChunk::headBytes() + GBlock::headBytes();
    lastReadPtr = 0;
    chunk.setChunk(buffer);
    chunk.rewind();
    nextStop = chunk.getSize() + GChunk::headBytes();
}

int16_t GCodeRenderer::GCache::addPoint(Vec3f &vec, Vec3f &oldV)
{
    uint16_t numBytes = 0;
    // Ensure enough space in chunk
    // Worst case: A new table + a full new position is written (4+12 = 16 bytes)
    if (chunk.getSize() >= bufferLen - (GBlock::minBytes() - 1))
    {
        nextChunk();
        numBytes = GChunk::headBytes();
        oldV = Vec3f();
    }
    uint16_t newBytes = chunk.addPoint(vec, oldV);
    size += newBytes;
    numBytes += newBytes;

    return numBytes;
}

size_t GCodeRenderer::GCache::write(int32_t maxSize, FILE* file)
{
    int32_t cnt = 0;
    TickType_t timeout = xTaskGetTickCount() + pdMS_TO_TICKS(4000);
    if (size < maxSize) maxSize = size;
    while (cnt < maxSize)
    {
        int32_t numBytes = std::min(bufferLen, maxSize - cnt);
        if (fwrite(&buffer[cnt], 1, numBytes, file) != numBytes)
            return cnt;// Write error
        cnt += numBytes;
        
        // Allow some time for lower priority tasks
        if (xTaskGetTickCount() > timeout)
        {
            vTaskDelay(10);
            timeout += pdMS_TO_TICKS(4000);
        }
    }
    return cnt;
}

size_t GCodeRenderer::GCache::read(void* buff, FILE* file)
{
    readPtr = GChunk::headBytes() + GBlock::headBytes();
    lastReadPtr = 0;
    chunk.rewind();
    size_t numBytes = chunk.read(buff, file);
    nextStop = numBytes + GChunk::headBytes();
    return numBytes;
}

int16_t GCodeRenderer::GCache::readPoint(Vec3f &oldP)
{
    if (readPtr >= nextStop)
    {
        if (readPtr >= size)
            return INT16_MAX;
        
        chunk.setChunk(&buffer[readPtr]);
        chunk.rewind();
        readPtr += GChunk::headBytes() + GBlock::headBytes();
        nextStop += chunk.getSize() + GChunk::headBytes();
    }

    int16_t numBytes = chunk.readPoint(oldP);
    if (numBytes < 0)
    {
        numBytes = -1;
    }
    else
    {
        readPtr += numBytes;
        DBG_LOGV("P(%.3f, %.3f, %.3f) - %d/%d", oldP.x, oldP.y, oldP.z, readPtr, nextStop);
        numBytes = readPtr - lastReadPtr;
        lastReadPtr = readPtr;
    }       
    
    return numBytes;
}

void GCodeRenderer::GChunk::nextBlock()
{
    *chunkSize += GBlock::headBytes();
    block = (GBlock*)&block->data[block->size];
    block->reset();
}

void GCodeRenderer::GChunk::reset()
{
    idx = 0;
    mask = 1;
    *chunkSize = GBlock::headBytes();
    block->reset();
}

void GCodeRenderer::GChunk::rewind()
{
    idx = 0;
    mask = 1;
}

int16_t GCodeRenderer::GChunk::addPoint(Vec3f &vec, Vec3f &oldV)
{
    uint16_t numBytes = block->addPoint(vec, oldV, idx);
    *chunkSize += numBytes;

    if (idx >= GBlock::maxSize()/3)
    {
        nextBlock();
        numBytes += GBlock::headBytes();
        idx = 0;
    }

    return numBytes;
}

int16_t GCodeRenderer::GChunk::readPoint(Vec3f &oldP)
{
    int16_t numBytes = 0;
    for (uint8_t j = 0; j < 3; j++) // For each vector component
    {
        if (block->table & mask) // If exists
        {
            oldP[j] = block->data[idx++];
            numBytes += sizeof(block->data[0]);
        }

        mask <<= 1;
    }
    if (mask >= (1 << GBlock::maxSize()))
    {
        if (block->size != idx)
        {
            DBG_LOGE("Block size: %d, idx: %d", block->size, idx);
            numBytes = -1;
        }
        else
        {
            numBytes += GBlock::headBytes();
        }
        block = (GBlock*)&block->data[idx];
        mask = 1;
        idx = 0;
    }
    return numBytes;
}

size_t GCodeRenderer::GChunk::read(void* buff, FILE* file)
{
    setChunk(buff);
    fread(chunkSize, sizeof(chunkSize[0]), 1, file);
    if (getSize() < 0 || getSize() >= bufferLen)
    {
        DBG_LOGE("Invalid chunk size (Max %dB, got %dB)", bufferLen, getSize());
        return 0;
    }
    
    return fread(block, 1, getSize(), file);
}

void GCodeRenderer::GChunk::setChunk(void* ptr)
{
    chunkSize = (int16_t*)ptr;
    block = (GBlock*)&chunkSize[1];
}

void GCodeRenderer::GBlock::reset()
{
    table = 0;
    size = 0;
}

uint16_t GCodeRenderer::GBlock::addPoint(Vec3f &vec, Vec3f &oldV, uint8_t &i)
{
    uint16_t numBytes = 0;

    for (uint8_t j = 0; j < 3; j++)
    {
        if (vec[j] != oldV[j])
        {
            //         table[3]       |        table[2]       |       table[1]      |    table[0]
            // 31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10 9 8|7 6 5 4 3 2 1 0
            // 0  0  0  0  0  0  0  0 |z  y  x  z  y  x  z  y |x  z  y  x  z  y  x z|y x z y x z y x
            data[size++] = vec[j];
            numBytes += sizeof(data[0]);
            table |= 1 << (j + i*3);
        }
    }
    i++;

    return numBytes;
}

// Needed for linker errors
bool GCodeRenderer::_init;
GCodeRenderer GCodeRenderer::_instance;
uint8_t GCodeRenderer::eState = STOP;
uint8_t GCodeRenderer::isReady = 0;
constexpr int32_t GCodeRenderer::bufferLen;

GCodeRenderer::GCodeRenderer()
{   
    // Create queues
    threadQueue = xQueueCreate(rQueueLen, sizeof(JobData));
    thrdRetQueue = xQueueCreate(rQueueLen, sizeof(JobData));
    vectorQueue = xQueueCreate(vecQueueLen, sizeof(VectorData));
    vectRetQueue = xQueueCreate(vecQueueLen, sizeof(VectorData));
    readyFlag = xSemaphoreCreateBinary();

    if (threadQueue == nullptr || 
        thrdRetQueue == nullptr || 
        vectorQueue == nullptr ||
        vectRetQueue == nullptr ||
        readyFlag == nullptr)
    {
        DBG_EARLY_LOGE("Failed to create queues!");
        esp_restart();
    }

    outImg = (uint16_t*)calloc(320*320, sizeof(int16_t));

    xTaskCreatePinnedToCore(threadTask, "Worker task", 2560, NULL, 1, &worker, 1);
    vTaskDelay(100);

    xTaskCreatePinnedToCore(assemblerTask, "Assembler task", 3072, NULL, 1, &assembler, 0);

    xTaskCreate(mainTask, "Main task", 3072, NULL, 2, &main);

    DBG_EARLY_LOGI("Gcode renderer initialized");
    _init = true;
}

GCodeRenderer::~GCodeRenderer()
{
    JobData job;
    JobData chunk;
    
    // Clear queues
    xQueueReset(threadQueue);
    xQueueReset(thrdRetQueue);
    xQueueReset(vectorQueue);
    xQueueReset(vectRetQueue);
    
    printMinStack();

    vTaskDelete(worker);
    vTaskDelete(assembler);
    DBG_LOGI("Render task deleted");
    vTaskDelete(main);

    if (outImg)
        free(outImg);
}

void GCodeRenderer::mainTask(void* arg)
{
    DBG_LOGI("Renderer main task started");
    JobData endJobKey;
    endJobKey.size = -1;
    int64_t start = 0LL;
    int64_t half = 0LL;
    int64_t endT = 0LL;
    for( ;; )
    {
        switch (eState)
        {
        case STOP:
            vTaskSuspend(NULL);
            break;

        case READY:
            isReady |= 1;
            vTaskDelay(10);
            break;
            
        case INIT:
            instance()->init();
            half = 0LL;
            endT = 0LL;
            start = esp_timer_get_time();
            break;

        case PRE_PROCESS:
            if (instance()->readFile() != ESP_OK) eState = ERROR;
            if (eState == PRE_PROCESS) // Normal program flow
            {
                // Signal the worker to stop working
                DBG_LOGD("End threads");
                xQueueSend(instance()->threadQueue, &endJobKey, portMAX_DELAY);
                while (eState == PRE_PROCESS) vTaskSuspend(NULL); // Wait for the assembler to finish
            }
            else // Something went wrong
                instance()->stopTasks();
            
            break;

        case RENDER:
            half = esp_timer_get_time();
            while (eState == RENDER) vTaskSuspend(NULL); // Wait for the assembler to finish
            break;

        case END:
            endT = esp_timer_get_time();
            if (start > 0)
            {
                DBG_LOGI("Read time: %d ms", (unsigned int)(endT-start)/1000);
                DBG_LOGI("Render time: %d ms", (unsigned int)(endT-half)/1000);
            }
            else
                DBG_LOGI("Render time: %d ms", (unsigned int)(endT-half)/1000);

            instance()->progress = 100.0f;
            xSemaphoreGive(instance()->readyFlag);
            eState = STOP;
            break;

        case ERROR:
            DBG_LOGE("Error rendering G Code");
            while (eState == ERROR)
            {
                vTaskSuspend(instance()->worker);
                vTaskSuspend(instance()->assembler);
                vTaskSuspend(NULL);
            }
            break;
        }
    }

    vTaskDelete(nullptr);
}

void GCodeRenderer::threadTask(void* arg)
{
    DBG_LOGI("Worker started");
    VectorData endJobKey;
    endJobKey.size = -1;

    for ( ;; )
    {
        switch (eState)
        {
        case PRE_PROCESS:
            instance()->processGcode();
            // Signal end of G Code processing
            if (eState == PRE_PROCESS)
            {
                DBG_LOGD("End asm");
                xQueueSend(instance()->vectorQueue, &endJobKey, portMAX_DELAY);
                while (eState == PRE_PROCESS) vTaskSuspend(NULL); // Wait for the assembler to finish
            }
            break;

        case RENDER:
        {
            esp_err_t ret = instance()->readTmp();
            if(ret != ESP_OK)
            {
                if (ret == ESP_FAIL) eState = ERROR;
                else eState = INIT;
                instance()->stopTasks();
            }
            
            // Signal end of preprocessing
            if (eState == RENDER)
            {
                DBG_LOGD("End asm");
                xQueueSend(instance()->vectorQueue, &endJobKey, portMAX_DELAY);
                while (eState == RENDER) vTaskSuspend(NULL); // Wait for the assembler to finish
            }
            break;
        }
        case STOP:
            vTaskSuspend(NULL);
            break;

        default:
            isReady |= 2;
            vTaskDelay(10);
            break;
        }
    }

    DBG_LOGW("Delete worker");
    vTaskDelete(NULL);
}

void GCodeRenderer::assemblerTask(void* arg)
{
    DBG_LOGI("Assembler started");

    for ( ;; )
    {
        switch (eState)
        {
        case PRE_PROCESS:
            if (instance()->generatePath() != ESP_OK) eState = ERROR;
            if (eState == PRE_PROCESS)
                eState = RENDER;
            vTaskResume(instance()->main);
            vTaskResume(instance()->worker);

            break;

        case RENDER:
            if (instance()->renderMesh() != ESP_OK) eState = ERROR;
            if (eState == RENDER)
                eState = END;
            vTaskResume(instance()->main);
            vTaskResume(instance()->worker);

            break;

        case STOP:
            vTaskSuspend(NULL);
            break;

        default:
            isReady |= 4;
            vTaskDelay(10);
            break;
        }
    }

    vTaskDelete(NULL);
}

esp_err_t GCodeRenderer::readFile()
{
    JobData job;
    int16_t deleted = 0;

    rfile = fopen(filePath.c_str(), "r");
    DBG_LOGE_AND_RETURN_IF(rfile == nullptr, ESP_FAIL,
        "Error opening file \"%s\"", filePath.c_str());

    setvbuf(rfile, rBuffer, _IOFBF, bufferLen);

    // Get File size
    fseek(rfile, 0, SEEK_END);
    float filesize = 50.0f/ftell(rfile);
    rewind(rfile);

    #if CONFIG_DBG_LOG_LEVEL >= DBG_LOG_DEBUG
    int32_t percent = 0;
    #endif

    // Checking
    DBG_LOGW_IF(uxQueueSpacesAvailable(thrdRetQueue) != 0, "thrdRetQueue not full");
    DBG_LOGW_IF(uxQueueMessagesWaiting(threadQueue) != 0, "threadQueue not empty");

    DBG_LOGD("Start reading G Code");

    while (eState == PRE_PROCESS)
    {
        xQueueReceive(thrdRetQueue, &job, portMAX_DELAY);

        memcpy(job.data, tmpBuffer, deleted);

        int16_t readLen = fread(job.data + deleted, 1, bufferLen - 1 - deleted, rfile) + deleted;
        if (readLen == 0)   // End of file or error encountered
        {
            xQueueSend(thrdRetQueue, &job, portMAX_DELAY);
            break;
        }

        deleted = 0;

        if (!feof(rfile))
        {
            char* end = job.data + readLen;
            DBG_LOGV("end:%d\n", *(end-1));

            while (*(--end) >= 32)
                deleted++;

            memcpy(tmpBuffer, ++end, deleted);
        }

        progress = ftell(rfile)*filesize;
        #if CONFIG_DBG_LOG_LEVEL >= DBG_LOG_DEBUG
        if (percent != (int32_t)progress)
        {
            DBG_LOGD("Read %d %%", percent);
            percent = progress;
        }
        #endif
        
        job.size = readLen - deleted; // Job ID
        job.data[job.size] = '\0';
        xQueueSend(threadQueue, &job, portMAX_DELAY);
    }

    if (ferror(rfile))
    {
        DBG_LOGE("Error reading file \"%s\"", filePath.c_str());
        fclose(rfile);
        rfile = nullptr;
        return ESP_FAIL;
    }

    DBG_LOGD("File read!");
    fclose(rfile);
    rfile = nullptr;
    return ESP_OK;
}

esp_err_t GCodeRenderer::readTmp()
{
    DBG_LOGD("Start reading tmp");

    int32_t filesize;

    #if CONFIG_DBG_LOG_LEVEL >= DBG_LOG_DEBUG
    int32_t percent = 0;
    #endif
    VectorData moveBuffer;

    // Load from RAM or stored file
    if (isTmpOnRam)
    {
        tmpCache.rewind();
        filesize = tmpCache.getSize();
        DBG_LOGD("Tmp size: %.2fkB", filesize/1024.0f);

        camPos = tmpCache.camPos;
    }
    else
    {
        DBG_LOGD("Load tmp from SDcard");
        rfile = fopen(tmpPath.c_str(), "rb");
    
        DBG_LOGE_AND_RETURN_IF(rfile == nullptr, ESP_FAIL,
            "Error opening file \"%s\"", tmpPath.c_str());

        setvbuf(rfile, rBuffer, _IOFBF, bufferLen);

        // Get File size
        fseek(rfile, 0, SEEK_END);
        filesize = ftell(rfile);
        rewind(rfile);

        // Error in tmp file
        if (filesize == 0)
        {
            DBG_LOGW("Empty tmp file (%s), regenerating", tmpPath.c_str());
            fclose(rfile);
            rfile = nullptr;
            remove(tmpPath.c_str());
            return ESP_ERR_INVALID_SIZE;
        }
        
        // Read header
        time_t timestamp;
        fread(&timestamp, sizeof(time_t), 1, rfile);

        struct stat st;
        stat(filePath.c_str(), &st);
        if (timestamp != st.st_mtime)
        {
            DBG_LOGI("Outdated tmp file (%s), regenerating", tmpPath.c_str());
            fclose(rfile);
            rfile = nullptr;
            return ESP_ERR_INVALID_VERSION;
        }

        fread(&camPos, sizeof(Vec3f), 1, rfile);
    }

    float sizeFraction = 50.0f/filesize;
    
    DBG_LOGD("CamPos(%.3f, %.3f, %.3f)", camPos.x, camPos.y, camPos.z);
    projMat = Mat4::Translation(-camPos) * Mat4(rotMat) * Mat4::Projection(2.0f, 2.0f, near);

    // Checking
    DBG_LOGW_IF(uxQueueSpacesAvailable(vectRetQueue) != 0, "vectRetQueue not full");
    DBG_LOGW_IF(uxQueueMessagesWaiting(vectorQueue) != 0, "vectorQueue not empty");

    Vec3f point = Vec3f();
    int32_t filePtr = 0;
    while (eState == RENDER && filePtr < filesize)
    {
        if (isTmpOnRam)
        {
            progress = 50.0f + filePtr*sizeFraction;
        }
        else
        {
            char n[5];
            size_t size = tmpCache.read(n, rfile);
            if (size == 0 || size != tmpCache.chunk.getSize()) // End of file or error encountered
                break;

            progress = 50.0f + ftell(rfile)*sizeFraction;
        }

        #if CONFIG_DBG_LOG_LEVEL >= DBG_LOG_DEBUG
        if (percent != (int32_t)progress)
        {
            percent = progress;
            DBG_LOGD("Read %d %%", percent);
        }
        #endif

        int32_t chunkSize = tmpCache.chunk.getSize() + GChunk::headBytes();
        while (chunkSize > 0) // While data available
        {
            int16_t size = tmpCache.readPoint(point);
            if (size < 0)
            {
                if (isTmpOnRam)
                {
                    isTmpOnRam = false;
                }
                else
                {
                    fclose(rfile);
                    rfile = nullptr;
                }
                DBG_LOGE("Cache error");
                return ESP_FAIL;
            }
            chunkSize -= size;
            filePtr += size;

            // Ensure enough space
            if (moveBuffer.size >= vecBufferLen) 
            {
                xQueueSend(vectorQueue, &moveBuffer, portMAX_DELAY);
                moveBuffer.size = -1; // Request new buffer
            }
            if (moveBuffer.size == -1) // Get new buffer
            {
                xQueueReceive(vectRetQueue, &moveBuffer, portMAX_DELAY);
                moveBuffer.size = 0;
            }

            moveBuffer.data[moveBuffer.size++] = point;
        }
    }
    
    // Send last movements if any
    if (moveBuffer.size > 0)
        xQueueSend(vectorQueue, &moveBuffer, portMAX_DELAY);

    if (!isTmpOnRam)
    {
        // Check for errors
        if (ferror(rfile))
        {
            DBG_LOGE("Error reading file");
            fclose(rfile);
            rfile = nullptr;
            return ESP_FAIL;
        }

        // Close file
        fclose(rfile);
        rfile = nullptr;
    }
    isTmpOnRam = false;

    DBG_LOGD("Tmp read!");
    return ESP_OK;
}

void GCodeRenderer::processGcode()
{
    PrinterState printState;
    JobData job;
    VectorData moveBuffer;
    moveBuffer.size = -1;
    Vec3f lastPoint = Vec3f();
    isShell = true;

    // Checking
    DBG_LOGW_IF(uxQueueSpacesAvailable(vectRetQueue) != 0, "vectRetQueue not full");
    DBG_LOGW_IF(uxQueueMessagesWaiting(vectorQueue) != 0, "vectorQueue not empty");

    DBG_LOGD("Worker started processing");

    TickType_t timeout = xTaskGetTickCount() + pdMS_TO_TICKS(2000);
    while (xQueueReceive(threadQueue, &job, portMAX_DELAY) && job.size >= 0)
    {
        DBG_LOGV("New job");

        char* buffPtr = job.data;

        while (parseGcode(buffPtr, printState) >= 0)
        {
            if (isShell) // Ignore infill
            {
                // Ensure enough space
                // At worst 3 Vec3f will go into the buffer
                if (moveBuffer.size >= vecBufferLen-2) 
                {
                    xQueueSend(vectorQueue, &moveBuffer, portMAX_DELAY);
                    moveBuffer.size = -1; // Request new buffer
                }
                if (moveBuffer.size == -1) // Get new buffer
                {
                    xQueueReceive(vectRetQueue, &moveBuffer, portMAX_DELAY);
                    moveBuffer.size = 0;
                }
                if (printState.nextE > printState.currentE && printState.currentPos != printState.nextPos)
                {
                    // Send movement
                    if (lastPoint != printState.currentPos) // Is this line not connected to the last one?
                    {
                        moveBuffer.data[moveBuffer.size++] = lastPoint; // Create a chain break
                        moveBuffer.data[moveBuffer.size++] = printState.currentPos; // Send new starting position
                    }
                    moveBuffer.data[moveBuffer.size++] = printState.nextPos;
                    lastPoint = printState.nextPos;
                }
            }

            printState.currentPos = printState.nextPos; // Update position
            printState.currentE = printState.nextE;
        }

        xQueueSend(thrdRetQueue, &job, portMAX_DELAY);  // Return buffer

        // Allow some time for lower priority tasks
        if (xTaskGetTickCount() > timeout)
        {
            vTaskDelay(10);
            timeout += pdMS_TO_TICKS(2000);
        }
    }
    // Send remaining movements
    if (moveBuffer.size > 0)
        xQueueSend(vectorQueue, &moveBuffer, portMAX_DELAY);
    
    DBG_LOGD("All jobs processed");
}

esp_err_t GCodeRenderer::generatePath()
{
    VectorData moveBuffer;
    Vec3f lastPos = Vec3f();
    Boundary limits = {
        .Xmax = Vec3f(0.0f, 0.0f, -infinityf()),
        .Xmin = Vec3f(0.0f, 0.0f, infinityf()),
        .Ymax = Vec3f(0.0f, 0.0f, -infinityf()),
        .Ymin = Vec3f(0.0f, 0.0f, infinityf())
    };
    
    isTmpOnRam = true;
    tmpCache.reset();

    rotMat = Mat3::RotationZ(camTheta)*Mat3::RotationX(camPhi);

    DBG_LOGD("Start path");
    int32_t j = 1;

    while (xQueueReceive(vectorQueue, &moveBuffer, portMAX_DELAY) && moveBuffer.size >= 0) // New positions
    {
        for (; j < moveBuffer.size; j++)
        {
            // Tmp file too large, store on SD card
            if (tmpCache.getSize() >= cacheLen - GBlock::minBytes() - 1)
            {
                DBG_LOGD("Move tmp to SD (%dB)", tmpCache.getSize());
                isTmpOnRam = false;
                if (wfile == nullptr) // Initialize file
                {
                    wfile = fopen(tmpPath.c_str(), "wb");
                    DBG_LOGE_AND_RETURN_IF(wfile == nullptr, ESP_FAIL,
                        "Error opening tmp file for write (%s)", tmpPath.c_str());

                    setvbuf(wfile, wBuffer, _IOFBF, bufferLen);
                    
                    struct stat st;
                    stat(filePath.c_str(), &st);
                    
                    // Write header
                    fwrite(&st.st_mtime, sizeof(time_t), 1, wfile);
                    fwrite(&camPos, sizeof(Vec3f), 1, wfile);   // Wrong camera position, just to leave a gap
                }

                if (tmpCache.write(tmpCache.getSize(), wfile) != tmpCache.getSize())
                { // Write error
                    DBG_LOGE("Error writting file");
                    fclose(wfile);
                    wfile = nullptr;
                    return ESP_FAIL;
                }

                // Reset variables
                tmpCache.reset();
            }

            tmpCache.addPoint(moveBuffer.data[j], lastPos);
            DBG_LOGV("P(%.3f, %.3f, %.3f)", moveBuffer.data[j].x, moveBuffer.data[j].y, moveBuffer.data[j].z);
            checkCamPos(moveBuffer.data[j], limits);
            lastPos = moveBuffer.data[j];
        }
        xQueueSend(vectRetQueue, &moveBuffer, portMAX_DELAY);
        j = 0;
    }
    DBG_LOGD("Cache size: %d", tmpCache.getSize());
    
    // Calculate final camera position
    float dx1 = limits.Xmax.x*0.5f - limits.Xmax.y*0.375f;
    float dx2 = limits.Xmin.x*0.5f + limits.Xmin.y*0.375f;
    float dy1 = limits.Ymax.x*0.5f - limits.Ymax.y*0.375f;
    float dy2 = limits.Ymin.x*0.5f + limits.Ymin.y*0.375f;
    float dz1 = dx2*4.0f/3.0f - dx1*4.0f/3.0f;
    float dz2 = dy2*4.0f/3.0f - dy1*4.0f/3.0f;

    // Calculate position and revert camera rotation
    Vec3f camP = Vec3f(dx1 + dx2, dy1 + dy2, fminf(dz1, dz2) - 1.0f);
    camP *= !rotMat;

    esp_err_t result = ESP_OK;
    if (wfile != nullptr)
    {
        tmpCache.write(tmpCache.getSize(), wfile);
        
        fseek(wfile, sizeof(time_t), SEEK_SET);
        fwrite(&camP, sizeof(Vec3f), 1, wfile);

        result = ferror(wfile);
        fclose(wfile);
        wfile = nullptr;
    }

    // Write final camera position
    DBG_LOGD("Ymax (%.3f, %.3f)", limits.Ymax.x, limits.Ymax.y);
    DBG_LOGD("Ymin (%.3f, %.3f)", limits.Ymin.x, limits.Ymin.y);
    DBG_LOGD("Cam Pos: (%.3f, %.3f, %.3f)", camP.x, camP.y, camP.z);
    tmpCache.camPos = camP;

    DBG_LOGD_IF(result == ESP_OK, "Preprocess done");
    return result;
}

esp_err_t GCodeRenderer::renderMesh()
{
    VectorData moveBuffer;
    Vec3f start = Vec3f(infinityf(), 0.0f, 0.0f);
    Vec3f end = Vec3f();
    bool draw = false;

    float* zbuf = (float*)calloc(320*320, sizeof(float));

    if (zbuf == nullptr)
    { 
        DBG_LOGE("Failed to allocate Z buffer");
        return ESP_ERR_NO_MEM;
    }

    // Clear buffers
    memset(zbuf, 0x7F, 320*320*sizeof(float));
    memset(outImg, 0, 320*320*sizeof(int16_t));

    DBG_LOGD("Start drawing");
    while (xQueueReceive(vectorQueue, &moveBuffer, portMAX_DELAY) && moveBuffer.size >= 0)
    {
        DBG_LOGV("New job");
        draw = false;
        for (int32_t i = 0; i < moveBuffer.size; i++)
        {
            end = moveBuffer.data[i];
            if (end != start)
            {
                if (draw) projectLine(start, end, zbuf);
                start = end;
                draw = true;
            }
            else
                draw = false;
        }
        xQueueSend(vectRetQueue, &moveBuffer, portMAX_DELAY);
    }
    DBG_LOGD("Rendered");
    if (eState != RENDER) return ESP_OK;

    DBG_LOGD("Save image");

    esp_err_t result = ESP_OK;
/*     wfile = fopen(imgPath.c_str(), "wb");
    DBG_LOGE_AND_RETURN_IF(wfile == nullptr, false,
        "Error opening img file (%s)", imgPath.c_str());

    setvbuf(wfile, wBuffer, _IOFBF, bufferLen);

    if (fwrite(outImg, 2, 320*320, wfile) != 320*320)
        result = ESP_FAIL;

    fclose(wfile);
    wfile = nullptr; */

    free(zbuf);

    DBG_LOGD_IF(result == ESP_OK, "Saved");
    return result;
}

static const float power_of_ten[] = {
    1e0f,  1e1f,  1e2f,  1e3f,  1e4f,  1e5f,  1e6f,  1e7f,  1e8f,  1e9f,  1e10f};
inline float getFloat(char* p, char** endPtr)
{
    bool negative = false;
    // Skip spaces
    while (*p == ' ') p++;

    if (*p == '-') {
        ++p;
        negative = true;
        if (!NUMERIC(*p)) { // a negative sign must be followed by an integer
        return 0.0f;
        }
    }

    // Get integer part
    uint32_t integer = 0;
    if (*p == '0') { // 0 cannot be followed by an integer
        p++;
        if (NUMERIC(*p)) {
        return 0.0f;
        }
        integer = 0;
    } else {
        if (!(NUMERIC(*p))) // must start with an integer
            return 0.0f;

        integer = *p++ - '0';

        while (NUMERIC(*p)) {
            integer = integer*10 + (*p++ - '0');
        }
    }

    // Get farctional part if any
    int32_t exponent = 1;
    int32_t fraction = 0;
    if (*p == '.') {
        p++;
        if (NUMERIC(*p)) {
            fraction = *p++ - '0';
            exponent *= 10;
        } else {
            return 0.0f;
        }

        while (NUMERIC(*p)) {
            fraction = fraction*10 + (*p++ - '0');
            exponent *= 10;
        }
    }
    
    // Compose float
    float out = (float)fraction;
    out /= exponent;
    out += (float)integer;

    // Get exponent if any
    if (('e' == *p) || ('E' == *p)) {
        p++;
        bool neg_exp = false;
        if ('-' == *p) {
            neg_exp = true;
            ++p;
        } else if ('+' == *p) {
            ++p;
        }

        if (!NUMERIC(*p))
            return 0.0f;

        exponent = *p++ - '0';

        if (NUMERIC(*p)) {
            exponent = exponent*10 + (*p++ - '0');
        }

        while (NUMERIC(*p)) {
            if (exponent < 10) { // we need to check for overflows
                exponent = exponent*10 + (*p++ - '0');
            }
            else
                break; // Unsupported exponent
            ++p;
        }

        if (neg_exp)
            exponent = -exponent;
        
        if (-10 <= exponent && exponent <= 10)
        {
            if (exponent < 0)
                out /= power_of_ten[-exponent];
            else
                out *= power_of_ten[exponent];
        }
        else // Treat large exponents as infinity. This should never happen in this application
        {
            if (exponent < 0)
                out = infinityf();
            else
                out = -infinityf();
        }
    }

    if (negative)
        out = -out;

    *endPtr = p;

    return out;
}

int8_t GCodeRenderer::parseGcode(char* &p, PrinterState &state)
{
    char* lineStart = p;
    // Skip spaces
    while (*p == ' ') ++p;

    // Skip line number field: N[-0-9]
    if (*p == 'N' && NUMERIC_SIGNED(p[1]))
    {
        //strtol(++p, &p, 10);   // Check line number

        p += 2;                  // skip N[-0-9]
        while (NUMERIC(*p)) ++p; // skip numbers
        while (*p == ' ')   ++p; // skip spaces
    }

    // Get the command letter, which must be G, M, or T
    const char letter = *p++;
    uint16_t codenum = 0;

    // Only G and M commands are parsed
    if (letter == 'G' || letter == 'M')
    {
        // Skip spaces to get the numeric part
        while (*p == ' ') p++;

        // Bail if there's no command code number
        if (!NUMERIC(*p)) return 0;

        // Get the code number - integer digits only
        // strtol() is slower
        do { codenum *= 10, codenum += *p++ - '0'; } while (NUMERIC(*p));
        // Skip all spaces to get to the first argument, or nul
        while (*p == ' ') p++;
    }
    else if (letter == ';')
    {
        parseComment(p);
        while (*p > 31) p++; // Find end of line
        while (*p == '\r' || *p == '\n') p++; // Handle "\r\n"
        return 0; // Gcode comment
    }
    else if (letter == '\0')
        return -1;  // End of buffer
    else
    {
        while (*p > 31) p++; // Find end of line
        while (*p == '\r' || *p == '\n') p++; // Handle "\r\n"
        return 0;
    }

    // The command parameters (if any) start here, for sure!
    switch (letter)
    {
    case 'G':
        switch (codenum)
        {
        case 0: case 1:
        {
            while (*p > 31 && *p != ';') // Get the next parameter. A control character or a semicolon ends the loop
            {
                while (*p == ' ') p++;           // Skip spaces between parameters & values
                const char param = *p++;
                if (param == 'X') {
                    if(state.absPos) state.nextPos.x = state.offset.x + getFloat(p, &p);
                    else state.nextPos.x = state.currentPos.x + getFloat(p, &p);
                    DBG_LOGV("X%f", state.nextPos.x);
                }
                if (param == 'Y') {
                    if(state.absPos) state.nextPos.y = state.offset.y + getFloat(p, &p);
                    else state.nextPos.y = state.currentPos.y + getFloat(p, &p);
                    DBG_LOGV("Y%f", state.nextPos.y);
                }
                if (param == 'Z') {
                    if(state.absPos) state.nextPos.z = state.offset.z + getFloat(p, &p);
                    else state.nextPos.z = state.currentPos.z + getFloat(p, &p);
                    DBG_LOGV("Z%f", state.nextPos.z);
                }
                if (param == 'E') {
                    if(state.absEPos) state.nextE = state.offsetE + getFloat(p, &p);
                    else state.nextE = state.currentE + getFloat(p, &p);
                    DBG_LOGV("E%f", state.nextE);
                }
            }
            break;
        }
        case 28:
            state.nextPos = state.offset;
            state.nextE = 0.0f;
            break;

        case 90:
            state.absPos = true;
            state.absEPos = true;
            break;

        case 91:
            state.absPos = false;
            state.absEPos = false;
            break;

        case 92:
        {
            while (*p > 31 && *p != ';') // Get the next parameter. A control character or a semicolon ends the loop
            {
                while (*p == ' ') p++;           // Skip spaces between parameters & values
                const char param = *p++;
                if (param == 'X') {
                    state.offset.x = state.currentPos.x - getFloat(p, &p);
                }
                if (param == 'Y') {
                    state.offset.y = state.currentPos.y - getFloat(p, &p);
                }
                if (param == 'Z') {
                    state.offset.z = state.currentPos.z - getFloat(p, &p);
                }
                if (param == 'E') {
                    state.offsetE = state.currentE - getFloat(p, &p);
                }
            }
            break;
        }
        }
        break;

    case 'M':
        switch (codenum)
        {
        case 82:
            state.absEPos = true;
            break;

        case 83:
            state.absEPos = false;
            break;
        }
        break;
    }
    while (*p > 31) p++; // Find end of line
    while (*p == '\r' || *p == '\n') p++; // Handle "\r\n"
    // p should point to a new G code line or the end of file
    return p-lineStart;
}

void GCodeRenderer::parseComment(const char* str)
{
    // Only tested with Ultimaker Cura

    if (strncmp(str, "TYPE:", 5) == 0)
    {
        if (strncmp(&str[5], "FILL", 4) == 0)
            isShell = false;
        else
            isShell = true;
    }
    else if (strncmp(str, "Filament used:", 14) == 0)
    {
        info.filament = strtof(&str[14], nullptr);
        info.filamentReady = true;
    }
    else if (strncmp(str, "TIME:", 5) == 0)
    {
        info.printTime = strtoul(&str[5], NULL, 10);
        info.timeReady = true;
    }
}

inline void GCodeRenderer::checkCamPos(const Vec3f &u, Boundary &limit)
{
    // Apply rotation
    Vec3f p = u*rotMat;

    float marg =  p.x - p.z * 0.75;
    if (marg > limit.Xmax.z)
    {
        limit.Xmax.x = p.x;
        limit.Xmax.y = p.z;
        limit.Xmax.z = marg;
    }

    marg = p.x + p.z * 0.75;
    if (marg < limit.Xmin.z)
    {
        limit.Xmin.x = p.x;
        limit.Xmin.y = p.z;
        limit.Xmin.z = marg;
    }

    marg =  p.y - p.z * 0.75;
    if (marg > limit.Ymax.z)
    {
        limit.Ymax.x = p.y;
        limit.Ymax.y = p.z;
        limit.Ymax.z = marg;
    }

    marg = p.y + p.z * 0.75;
    if (marg < limit.Ymin.z)
    {
        limit.Ymin.x = p.y;
        limit.Ymin.y = p.z;
        limit.Ymin.z = marg;
    }
}

inline void GCodeRenderer::projectLine(const Vec3f &u, const Vec3f &v, float* zBuffer)
{
    DBG_LOGV("(%.3f, %.3f, %.3f) -> (%.3f, %.3f, %.3f)", u.x, u.y, u.z, v.x, v.y, v.z);
    Vec4f d1(u);
    Vec4f d2(v);

    // Apply perspective transformation
    d1 *= projMat;
    d2 *= projMat;

    d1.Homogenize();
    d2.Homogenize();
    
    // Remap to screen
    d1.x += scrOff.x;
    d1.y += scrOff.y;
    d2.x += scrOff.x;
    d2.y += scrOff.y;

    // Calculate illumination
    Vec3f dir = u-v;
    dir.Normalize();

    uint16_t alpha = fabsf(dir*lightDir)*191.0f + 64.0f;
    uint16_t r = alpha;
    uint16_t g = alpha;
    uint16_t b = alpha;
    uint16_t color = ((uint16_t)(r & 0xF8)<<8) | ((uint16_t)(g & 0xFC)<<3) | (b >> 3);

    // Render line
    drawLine(d1, d2, color, zBuffer);
}

inline void GCodeRenderer::drawLine(const Vec3f &u, const Vec3f &v, uint16_t color, float* zBuffer)
{
    Vec3f diff = v - u;
    Vec3f point = u;
    uint32_t cnt = 0;
    Pixel pix;
    pix.color = color;

    if (fabsf(diff.x) >= fabsf(diff.y))
        cnt = fabsf(diff.x);
    else
        cnt = fabsf(diff.y);

    cnt++;
    if (cnt > 1)
        diff /= (float)cnt;

    for (uint32_t i = 0; i <= cnt; i++)
    {
        pix.x = point.x;
        pix.y = point.y;
        pix.z = point.z;
        putPixel(pix, zBuffer);

        point += diff;
    }
}

inline void GCodeRenderer::putPixel(const Pixel pix, float* zBuffer)
{
    if (pix.x < 0 || pix.x >= 320 || pix.y < 0 || pix.y >= 320)
        return;

    uint32_t id = pix.x + pix.y*320;

    if (pix.z < zBuffer[id])
    {
        zBuffer[id] = pix.z;
        outImg[id] = pix.color;
    }
}

void GCodeRenderer::generateFilenames()
{
    tmpPath = filePath;
    tmpPath.insert(7, "/.cache");
    imgPath = tmpPath;
    size_t dotPos = tmpPath.rfind(".");
    tmpPath.replace(dotPos, std::string::npos, ".bin");
    imgPath.replace(dotPos, std::string::npos, ".img");
}

void GCodeRenderer::stopTasks()
{
    DBG_LOGD("Stop tasks");
    JobData job;
    job.size = -1;

    // Stop worker
    xQueueSendToFront(threadQueue, &job, portMAX_DELAY);

    // Stop assembler
    xQueueSendToFront(vectorQueue, &job, portMAX_DELAY);

    vTaskResume(worker);
    vTaskResume(assembler);
    vTaskResume(main);

    DBG_LOGD("Stopped");
}

void GCodeRenderer::init()
{
    vTaskResume(worker);
    vTaskResume(assembler);

    // Wait for all tasks to be idle
    DBG_LOGD("Wait for tasks");
    isReady = 0;
    uint8_t timeout = UINT8_MAX;
    while (isReady != 0x06 && timeout > 0) 
    {
        timeout--;
        vTaskDelay(1);
    }

    assert(xQueueReset(threadQueue) == pdPASS);
    assert(xQueueReset(thrdRetQueue) == pdPASS);
    assert(xQueueReset(vectorQueue) == pdPASS);
    assert(xQueueReset(vectRetQueue) == pdPASS);

    DBG_LOGD("Reset buffers");
    JobData job;
    // Send buffers to be available
    for (uint8_t i = 0; i < rQueueLen; i++)
    {
        job.data = readBuffers[i];
        xQueueSend(thrdRetQueue, &job, 0);
    }

    for (uint8_t i = 0; i < vecQueueLen; i++)
    {
        job.data = vectorBuffer[i];
        xQueueSend(vectRetQueue, &job, 0);
    }

    if (rfile != nullptr)
    {
        DBG_LOGW("Read file not closed!");
        fclose(rfile);
        rfile = nullptr;
    }
    if (wfile != nullptr)
    {
        DBG_LOGW("Write file not closed!");
        fclose(wfile);
        wfile = nullptr;
    }

    struct stat sb;

    if (stat("/sdcard/.cache", &sb) != 0 || !S_ISDIR(sb.st_mode))
    {
        if (mkdir("/sdcard/.cache", 0777) != 0)
            DBG_LOGE("Error creating cache folder");
    }

    if (access(tmpPath.c_str(), F_OK) == 0)
    {
        DBG_LOGD("Begin rendering");
        eState = RENDER;
    }
    else
    {
        DBG_LOGD("Begin preprocessing");
        eState = PRE_PROCESS;
    }
}

GCodeRenderer* GCodeRenderer::instance()
{
    return _init ? &_instance : nullptr;
}

esp_err_t GCodeRenderer::begin(std::string file)
{
    if (eState != STOP) return ESP_ERR_INVALID_STATE;

    xSemaphoreTake(readyFlag, 0); // clear ready flag
    progress = 0.0f;
    filePath = file;

    info.filamentReady = false;
    info.timeReady = false;

    generateFilenames();
    eState = INIT;
    vTaskResume(instance()->main);
    DBG_LOGI("Render task started");

    return ESP_OK;
}

esp_err_t GCodeRenderer::getRender(uint16_t** outPtr, TickType_t timeout)
{
    // TODO: Return error image on error state
    if (xSemaphoreTake(readyFlag, timeout) != pdTRUE)
        return ESP_ERR_TIMEOUT;

    *outPtr = outImg;
    return ESP_OK;
}

void GCodeRenderer::printMinStack()
{
    DBG_LOGI("Min stack worker: %dB", uxTaskGetStackHighWaterMark(worker));
    DBG_LOGI("Min stack Main: %dB", uxTaskGetStackHighWaterMark(main));
    DBG_LOGI("Min stack assembler: %dB", uxTaskGetStackHighWaterMark(assembler));
}
