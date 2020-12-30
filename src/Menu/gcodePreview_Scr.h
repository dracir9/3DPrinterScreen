
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

    void handleTouch(const Screen::touchEvent event, const Vec2h pos) override;

private:
    bool readLine();
    bool processLine();
    void renderGCode(tftLCD *tft);
    void parseComment(const char* line);
    void raster(tftLCD *tft);

    // SD file variables
    FILE* GcodeFile;
    static constexpr uint32_t maxLineLen = 96;
    static constexpr uint32_t bufferLen = 2048;
    char* readBuffer;
    char* gCodeLine;
    char* commentLine;
    uint16_t bufPos = 0;
    uint16_t readLen = 0;

    bool readDone = false;
    int64_t eTime = 0;

    // Machine state (in um)
    bool draw = false;
    bool absPos = true;
    bool absEPos = true;
    Vec3 currentPos;
    Vec3 nextPos;
    Vec3 offset;
    Vec3 minPos;
    Vec3 maxPos;
    float currentE = 0.0f;
    float nextE = 0.0f;
    float offsetE = 0.0f;

    // Camera settings
    int32_t near = 200;
    Vec3 camPos;
    Vec3f light = Vec3f(1.0f, 0.0f, 0.0f);
};

#endif