
#ifndef GCODEPREVIEW_SCR_H
#define GCODEPREVIEW_SCR_H

#include "lcdUI.h"
#include "parser.h"

class GcodePreview_Scr final : public Screen
{
public:
    GcodePreview_Scr(lcdUI* UI);

    void update(const uint32_t deltaTime) override;

    void render(tftLCD *tft) override;

private:
    bool readLine();
    bool processLine();
    void renderGCode(tftLCD *tft);
    void parseComment(const char* line);
    void raster(tftLCD *tft);

    // SD file variables
    FILE* GcodeFile;
    const uint32_t maxLineLen = 96;
    const uint32_t bufferLen = 2048;
    char* readBuffer;
    char* gCodeLine;
    char* commentLine;
    uint16_t bufPos = 0;
    uint16_t readLen = 0;

    bool readDone = false;

    // Machine state (in mm)
    bool printStarted = false;
    bool absPos = true;
    bool absEPos = true;
    Vec3 currentPos;
    Vec3 nextPos;
    Vec3 offset;
    Vec3 minPos;
    Vec3 maxPos;
    float currentE;
    float nextE;
    float offsetE;

    Vec3 camPos;

    Vec2f pos;
    Vec2f vel = Vec2f(100,100);

    uint16_t reColor = 0;
};

#endif