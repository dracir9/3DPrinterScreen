
#ifndef GCODEPREVIEW_SCR_H
#define GCODEPREVIEW_SCR_H

#include "lcdUI.h"
#include "parser.h"

#define MAX_LINE_LEN 96
#define BUFFER_LEN 2048

class GcodePreview_Scr final : public Screen
{
public:
    GcodePreview_Scr(lcdUI* UI);

    void update(uint32_t deltaTime) override;

    void render(tftLCD *tft) override;

private:
    bool readLine();
    bool processComand();
    void renderGCode(tftLCD *tft);

    // SD file variables
    FILE* GcodeFile;
    char* RBuffer;
    char* GcodeLine;
    uint16_t bufPos = 0;
    uint16_t readLen = 0;

    bool readDone = false;

    // Machine state
    bool absPos = true;
    bool absEPos = true;
    Vector3<int32_t> currentPos;
    Vector3<int32_t> nextPos;
    Vector3<int32_t> offset;
    float currentE;
    float nextE;
    float offsetE;

    Vector2<float> pos;
    Vector2<float> vel = Vector2<float>(100,100);

    uint16_t reColor = 0;
};

#endif