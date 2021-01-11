
#ifndef INFO_W_H
#define INFO_W_H

#include "lcdUI.h"

class Info_W final: public Screen
{
public:
    Info_W(lcdUI* UI, tftLCD& tft);

    void update(uint32_t deltaTime) override;
    void render(tftLCD& tft) override;
    void handleTouch(const touchEvent event, const Vec2h pos) override;

private:
    const uint8_t tools = 3;
    int nextP = 0;
};

#endif
