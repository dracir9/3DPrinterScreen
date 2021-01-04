
#ifndef BLACK_W_H
#define BLACK_W_H

#include "lcdUI.h"

class Black_W final: public Screen
{
public:
    Black_W(lcdUI* UI);

    void update(uint32_t deltaTime) override;
    void render(tftLCD *tft) override;

private:
};

#endif
