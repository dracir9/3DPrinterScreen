
#ifndef INFO_W_H
#define INFO_W_H

#include "widgets.h"

class Info_W final: public Screen
{
public:
    Info_W(lcdUI* UI);

    void update(uint32_t deltaTime) override;
    void render(tftLCD *tft) override;

private:
};

#endif
