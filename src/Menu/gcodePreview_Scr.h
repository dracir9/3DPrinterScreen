
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
    bool processComand();
    void renderGCode(tftLCD *tft);
    void parseComment(const char* line);

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

    // Machine state
    bool absPos = true;
    bool absEPos = true;
    Vector3<int32_t> currentPos;
    Vector3<int32_t> nextPos;
    Vector3<int32_t> offset;
    Vector3<int32_t> minPos;
    Vector3<int32_t> maxPos;
    float currentE;
    float nextE;
    float offsetE;

    Vector2<float> pos;
    Vector2<float> vel = Vector2<float>(100,100);

    uint16_t reColor = 0;
};

#endif