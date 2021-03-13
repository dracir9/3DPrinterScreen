
#ifndef GCODEPREVIEW_SCR_H
#define GCODEPREVIEW_SCR_H

#include "lcdUI.h"
#include <bitset>
#include "parser.h"
#include "Ulitity/utility.h"

class GcodePreview_Scr final : public Screen
{
public:
    GcodePreview_Scr(lcdUI* UI, tftLCD& tft);
    ~GcodePreview_Scr();

    void update(const uint32_t deltaTime) override;

    void render(tftLCD& tft) override;

    void handleTouch(const touchEvent event, const Vec2h pos) override;

private:
    bool readLine();
    bool processLine();
    bool initRender();
    void renderGCode(tftLCD& tft);
    void parseComment(const char* line);
    void drawLineZbuf(tftLCD& tft, Vec3f u, Vec3f v, const uint32_t color);
    void drawPixelZbuf(tftLCD& tft, Vec3f p, const uint32_t color);
    void drawInfo(tftLCD& tft);

    // SD file variables
    FILE* GcodeFile = nullptr;
    static constexpr uint32_t maxLineLen = 96;
    static constexpr uint32_t bufferLen = 2048;
    char* readBuffer = nullptr;
    char* gCodeLine = nullptr;
    char* commentLine = nullptr;
    float* Zbuffer = nullptr;
    uint16_t bufPos = 0;
    uint16_t readLen = 0;

    // Status and info
    uint8_t readState = 0;
    uint32_t lines = 0;
    int64_t eTime = 0;
    int64_t maxWorkTime = 0;
    std::bitset<2> displayed;
    int32_t printTime = 0;
    float filament = 0.0f;
    TFT_eSprite* img;

    // Machine state (in um)
    bool draw = false;
    bool absPos = true;
    bool absEPos = true;
    Vec3f currentPos;
    Vec3f nextPos;
    Vec3f offset;
    Vec3f minPos;
    Vec3f maxPos;
    float currentE = 0.0f;
    float nextE = 0.0f;
    float offsetE = 0.0f;

    // Camera settings
    static constexpr int32_t near = 200;
    Vec3f camPos;
    const Vec3f light = Vec3f(1.0f, 0.0f, 0.0f);
    int32_t zCmin = 0;
    int32_t zCmax = 0;
    uint32_t minidx = 160*320;
};

#endif