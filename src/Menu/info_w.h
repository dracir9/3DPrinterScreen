
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
    const uint8_t heatbed = random(0,2);
    const uint8_t tools = random(1,6);
    const uint8_t fans = random(0, min(8-tools-heatbed, 4));
    const uint8_t items = heatbed + tools + fans;
    const float cellAdv;
    const uint16_t cellW;
    int nextP = 0;
};

#endif
