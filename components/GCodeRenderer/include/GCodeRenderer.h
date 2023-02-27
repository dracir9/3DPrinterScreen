/**
 * @file   GCodeRenderer.h
 * @author Ricard Bitriá Ribes (https://github.com/dracir9)
 * Created Date: 07-12-2021
 * -----
 * Last Modified: 26-02-2023
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

#ifndef GCODERENDERER_H
#define GCODERENDERER_H

#include <sys/stat.h>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "Vector.h"
#include "dbg_log.h"
#include "Matrix.h"

struct FileInfo
{
    bool filamentReady:1;
    bool timeReady:1;
    Vec3f camPos;
    time_t timestamp;
    float filament = 0.0f;
    uint32_t printTime = 0;
};

class GCodeRenderer
{
public:
    constexpr static int32_t cacheLen = 7500000;
    constexpr static uint8_t rQueueLen = 2;
    constexpr static int32_t bufferLen = 4096;
    constexpr static uint8_t vecQueueLen = 2;
    constexpr static int32_t vecBufferLen = 350;
    constexpr static int32_t maxLineLen = 96;

private:
    struct Pixel
    {
        int16_t x;
        int16_t y;
        uint16_t color;
        float z;
    };

    struct JobData
    {
        int32_t size = -1;
        char* data = nullptr;
    };

    struct VectorData
    {
        int32_t size = -1;
        Vec3f* data = nullptr;
    };

    struct PrinterState
    {
        bool absPos = true;
        bool absEPos = true;
        Vec3f currentPos = Vec3f();
        Vec3f nextPos = Vec3f();
        Vec3f offset = Vec3f();
        float currentE = 0.0f;
        float nextE = 0.0f;
        float offsetE = 0.0f;
    };

    /**
     * Block contains
     *  · Header:
     *      - Table of the stored vector components
     *      - Size: number of float values stored
     *  · Data:
     *      The actual float values corresponding to 3D vector components (X,Y,Z)
     */
    struct GBlock
    {
        uint32_t table : 24;
        uint32_t size : 8;
        float data[];

        constexpr static int32_t headBytes() { return sizeof(uint32_t); }
        constexpr static int32_t minBytes() { return sizeof(uint32_t) + sizeof(float)*3; }
        constexpr static int32_t maxSize() { return 24; }
        void reset();
        uint16_t addPoint(Vec3f &vec, Vec3f &oldV, uint8_t &i);
    };

    /**
     * Chunk contains
     *  · Header:
     *      - Size: number of bytes occupied by the chunk
     *  · block: A bunch of blocks
     */
    struct GChunk
    {
        constexpr static int32_t headBytes() { return sizeof(chunkSize[0]); }
        void nextBlock();
        void reset();
        void rewind();
        int16_t addPoint(Vec3f &vec, Vec3f &oldV);
        int16_t readPoint(Vec3f &oldP);

        /**
         * @brief Load chunk from file
         * 
         * @param buff Memory buffer to store the chunk. MUST BE OF AT LEAST 'bufferLen' BYTES!
         * @param file File struct where the chunk is stored
         * @return size_t Return the number of bytes read. Returns 0 on error.
         */
        size_t read(void* buff, FILE* file);
        void setChunk(void* ptr);
        int16_t getSize() const { return *chunkSize; }

    private:
        uint8_t idx = 0;
        uint32_t mask = 1;
        int16_t* chunkSize;
        GBlock* block;
    };

    /**
     * Cache contains
     *  · size: number of bytes stored in cache
     *  · camPos: Calculated camera position
     *  · chunk: the current active chunk being written
     */
    struct GCache
    {
        Vec3f camPos;
        GChunk chunk;

        GCache();
        ~GCache();
        void nextChunk();
        void reset();
        void rewind();

        /**
         * @brief Add point to cache
         * 
         * @param vec   New point
         * @param oldV  Previous point
         * @return int16_t Returns number of bytes written. Returns -1 on error
         */
        int16_t addPoint(Vec3f &vec, Vec3f &oldV);
        size_t write(int32_t maxSize, FILE* file);

        /**
         * @brief Load cache from file. Only one chunk is read
         * 
         * @param buff Memory buffer to store the cache. MUST BE OF AT LEAST 'bufferLen' BYTES!
         * @param file File struct where the cache is stored
         * @return size_t Return the number of bytes read. Returns 0 on error.
         */
        size_t read(void* buff, FILE* file);
        int16_t readPoint(Vec3f &oldP);

        int32_t getSize() const { return size; }

    private:
        char* buffer;
        int32_t size = 0;
        int32_t readPtr = 0;
        int32_t nextStop = 0;
        int32_t lastReadPtr = 0;
    };

    struct Boundary
    {
        Vec3f Xmax;
        Vec3f Xmin;
        Vec3f Ymax;
        Vec3f Ymin;
    };

    enum renderState
    {
        STOP,   // Suspended state
        READY,   // Task ready
        INIT,   // Initialize G Code rendering
        PRE_PROCESS, // PreProcess step
        RENDER, // Render step
        END,     // Finalize G Code rendering
        ERROR
    };

    static bool _init;
    static GCodeRenderer _instance;

    static uint8_t eState;
    static uint8_t isReady; // Bitmap [0: main task, 1: worker, 2: assembler]
    float progress = 0.0f;

    GCache tmpCache;
    bool isTmpOnRam = false;
    uint16_t* outImg = nullptr;
    float* zbuf = nullptr;

    FILE* rfile;
    FILE* wfile;
    char readBuffers[rQueueLen][bufferLen];  // Buffers where data to be processed is stored
    char vectorBuffer[vecQueueLen][vecBufferLen*sizeof(Vec3f)];    // Buffer for movement vectors and pixel data
    char tmpBuffer[maxLineLen];   // Temporal buffer to hold incomplete lines when reading gcode
    char rBuffer[bufferLen];   // System buffer for fread
    char wBuffer[bufferLen];   // System buffer for fwrite
    std::string filePath;
    std::string tmpPath;
    std::string imgPath;

    Vec3f camPos;
    static constexpr int32_t near = 200;
    static constexpr Vec3f lightDir = Vec3f(1.0f, 0.0f, 0.0f);
    static constexpr Vec3f scrOff = Vec3f(160.0f, 160.0f, 0.0f);
    static constexpr float camTheta = M_PI/6.0f;
    static constexpr float camPhi = M_PI*2.0f/3.0f;
    Mat3 rotMat;
    Mat4 projMat;

    FileInfo info;
    bool isShell = true;

    QueueHandle_t threadQueue;
    QueueHandle_t thrdRetQueue;
    QueueHandle_t vectorQueue;
    QueueHandle_t vectRetQueue;
    SemaphoreHandle_t readyFlag;

    TaskHandle_t main;
    TaskHandle_t worker;
    TaskHandle_t assembler;

    GCodeRenderer();
    ~GCodeRenderer();

    static void mainTask(void* arg);
    static void threadTask(void* arg);
    static void assemblerTask(void* arg);

    // Stage 1
    esp_err_t readFile();
    esp_err_t readTmp();

    // Stage 2
    void processGcode();

    // Stage 3
    esp_err_t generatePath();
    esp_err_t renderMesh();

    // Helpers
    int8_t parseGcode(char* &p, PrinterState &state);
    void parseComment(const char* str);
    void checkCamPos(const Vec3f &u, Boundary &limit);

    // Rendering
    void projectLine(const Vec3f &u, const Vec3f &v);
    void drawLine(const Vec3f &u, const Vec3f &v, uint16_t color);
    void putPixel(const Pixel pix);

    // General functions
    void generateFilenames();
    esp_err_t loadImg();
    void init();
    void stopTasks();
    esp_err_t waitIdle();
    
public:
    esp_err_t begin(std::string file);
    void stop();
    esp_err_t getRender(uint16_t** outPtr, TickType_t timeout);
    static GCodeRenderer* instance();
    void printMinStack();
    float getProgress() { return progress; };
    const FileInfo* getInfo() { return &info; };
};

#endif // GCODERENDERER_H
